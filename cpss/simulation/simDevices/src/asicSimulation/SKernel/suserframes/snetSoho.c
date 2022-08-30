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
* @file snetSoho.c
*
* @brief This is a external API definition for SnetSoho module of SKernel.
*
* @version   62
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetSoho.h>
#include <asicSimulation/SKernel/smem/smemSoho.h>
#include <asicSimulation/SKernel/sohoCommon/sregSoho.h>
#include <asicSimulation/SKernel/suserframes/snetSohoEgress.h>
#include <asicSimulation/SKernel/skernel.h>
#include <common/Utils/SohoHash/smacHashSoho.h>
#include <asicSimulation/SKernel/sfdb/sfdbSoho.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#define __LOG_DROP(x) __LOG(("DROP occurred: " x))

static GT_VOID snetSohoIngressPacketProc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoHeaderProc
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoIpPriorityAssign
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoIgmpSnoop
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSoho2IgmpSnoop
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoPortMapUpdate
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoPriorityAssign
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoL2Decision
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoL2RubyDecision
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoStatCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

static GT_VOID snetSohoAtuEntryGet
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_U32 address,
    OUT SNET_SOHO_ATU_STC * atuEntryPtr
);

static GT_VOID snetSohoQcMonitorIngress
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr,
    OUT SNET_SOHO_MIRROR_STC * mirrPtr
);

static GT_VOID snetSohoQcMonitorEgress
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr,
    OUT SNET_SOHO_MIRROR_STC * mirrPtr
);

static GT_VOID snetSohoFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
);

static GT_VOID snetSohoLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT      *     devObjPtr,
    IN GT_U32                           port,
    IN GT_U32                           linkState
);

static GT_VOID snetOpalPvtCollect
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
);

#define SOHO_IS_ATU_ENTRY_STATIC(_atu_entry)     \
    ((SGT_MAC_ADDR_IS_MCST_BY_FIRST_BYTE(_atu_entry.macAddr.bytes[0])) ||  \
    (_atu_entry.entryState == 0xe) ||            \
    (_atu_entry.entryState == 0xf) )

/* Retrieve Request Format from frame data buffer */
#define RMT_TPYE(_frame_data_ptr)\
    (GT_U16)((_frame_data_ptr)[16] << 8 | (_frame_data_ptr)[17] )
/* Retrieve Request Format from frame data buffer */
#define REQ_FMT(_frame_data_ptr)\
    (GT_U16)((_frame_data_ptr)[18] << 8 | (_frame_data_ptr)[19] )

/* Retrieve Request Format from frame data buffer */
#define REQ_CDE(_frame_data_ptr)\
    (GT_U16)((_frame_data_ptr)[22] << 8 | (_frame_data_ptr)[23] )


/**
* @internal snetSohoProcessInit function
* @endinternal
*
* @brief   Init module.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetSohoProcessInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    devObjPtr->descriptorPtr =
        (void *)calloc(1, sizeof(SKERNEL_FRAME_SOHO_DESCR_STC));

    if (devObjPtr->descriptorPtr == 0)
    {
        skernelFatalError("smemSohoInit: allocation error\n");
    }

    /* initiation of internal soho function */
    devObjPtr->devFrameProcFuncPtr = snetSohoFrameProcess;
    devObjPtr->devPortLinkUpdateFuncPtr = snetSohoLinkStateNotify;
    devObjPtr->devFdbMsgProcFuncPtr = sfdbSohoMsgProcess;
    devObjPtr->devMacTblAgingProcFuncPtr = sfdbSohoMacTableAging ;
}

/**
* @internal snetSohoLinkStateNotify function
* @endinternal
*
* @brief   Notify devices database that link state changed
*/
static GT_VOID snetSohoLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 port,
    IN GT_U32 linkState
)
{
    GT_U32 regAddress,swPortRegAddr;      /* Register's address */
    GT_U32 spd;         /* speed Number */
    GT_U32  old_linkState;

    /* Port status register */
    swPortRegAddr = SWITCH_PORT0_STATUS_REG(devObjPtr,port);
    /* Link */
    smemRegFldSet(devObjPtr, swPortRegAddr, 11, 1, linkState);

    /* PHY status */

    regAddress = PHY_STATUS_REG(port);

    /* Get previous Link state */
    smemRegFldGet(devObjPtr, regAddress, 2, 1, &old_linkState);

    /* Link */
    smemRegFldSet(devObjPtr, regAddress, 2, 1, linkState);

    /* PHY port specific status */
    regAddress = PHY_PORT_STATUS_REG(port);
    /* Link */
    smemRegFldSet(devObjPtr, regAddress, 10, 1, linkState);

    /* Port Speed */
    smemRegFldGet(devObjPtr, swPortRegAddr, 8, 2, &spd);

    /* The internal PHY is for ports 0..8 but not Gige.
       in Peridot the ports 0..8 are gig ports.
     */
    if (spd == SOHO_PORT_SPEED_1000_MBPS_E &&
        (!SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType)))
    {
        return ;
    }

    if(old_linkState == linkState)
    {
        /* no change in the link status */
        return;
    }


    snetSohoDoInterrupt(devObjPtr,
        PHY_INTERRUPT_STATUS_REG(port),/*causeRegAddr*/
        PHY_INTERRUPT_ENABLE_REG(port),/*causeMaskRegAddr*/
        1<<10/* link status changed*/,
        SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType) ?
            1<<7/*bit 7 in global 1*/ :
            1<<1/*bit 1 in global 1*/);
}

/**
* @internal snetSohoSaLearning function
* @endinternal
*
* @brief   SA learning process
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
static void snetSohoSaLearning
(
    IN SKERNEL_DEVICE_OBJECT   * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoSaLearning);

    GT_STATUS status;                   /* return status */
    SNET_SOHO_ATU_STC  atuEntry;        /* ATU database entry */
    GT_U8 isEntryMc = 0, isStatic = 0;  /* ATU database entry state*/
    GT_U32 regAddress;                  /* register's address */
    GT_U32 atuAddress = 0;              /* register's address */
    GT_U32 fldValue;                    /* register's field value */
    GT_U8 saOk = 0, doSa = 0;           /* SA learn flags */
    GT_U32 port;                        /* source port id */
    GT_U8 * srcMacPtr;                  /* src mac pointer */
    GT_U8 violation = 0;                /* ATU violation flag */
    GT_U8 updateDpv = 0;                /* update DPV flag */
    GT_U32 ports;                       /* device ports number */
    SOHO_DEV_MEM_INFO * memInfoPtr;     /* device's memory pointer */
    GT_U32 trunkIdx;                    /* trunk mask index */
    GT_U32 pav;                         /* Port Association Vector for ATU learning */
    GT_U32  trunkId = 0;/* 1 based number , if 0 -- not trunk */
    GT_U32 portCtrlRegData;             /* Port Control Register's Data */
    GT_U32 pavRegData;                  /* Port Association Register's Data */
    GT_U32 portBaseVlanRegData;         /* Port Based vlan Register's Data */
    GT_U32 portAtuControlData;         /* Port Atu Control Register's Data */
    GT_U32 learnInterruptEn;
    GT_U32 refreshLock;                 /* refresh lock bit in SA learning */

    ports = devObjPtr->portsNumber;

    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    srcMacPtr = SRC_MAC_FROM_DSCR(descrPtr);
    port = descrPtr->srcPort;

    regAddress = PORT_ASSOC_VECTOR_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddress, &pavRegData);

    regAddress  = PORT_CTRL1_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddress, &portCtrlRegData);

    regAddress  = PORT_BASED_VLAN_MAP_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddress, &portBaseVlanRegData);

    regAddress  = PORT_ATU_CONTROL(devObjPtr,port);
    smemRegGet(devObjPtr, regAddress, &portAtuControlData);

    /* enable interrupt NA message :    */
    learnInterruptEn = SMEM_U32_GET_FIELD(pavRegData, 0, ports);
    LOG_FIELD_VALUE("learnInterruptEn",learnInterruptEn);

    status = sfdbSohoAtuEntryAddress(devObjPtr, srcMacPtr, descrPtr->dbNum,
                                    &atuAddress);
    /* pav */
    pav = SMEM_U32_GET_FIELD(pavRegData, 0, ports);
    LOG_FIELD_VALUE("PAV : Port Association Vector for ATU learning. ",pav);

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        /* Get load balancing vector */
        __LOG(("Get load balancing vector"));
        trunkIdx = (srcMacPtr[5] & 0x7) ^ (descrPtr->dstMacPtr[5] & 0x7);
        descrPtr->tpv = memInfoPtr->trunkMaskMem.trunkTblMem[trunkIdx] & 0x7ff;

        /* Trunk Port */
        if (descrPtr->srcTrunk)
        {
            trunkId = SMEM_U32_GET_FIELD(portCtrlRegData, 4, 4);
            LOG_FIELD_VALUE("trunk Id",trunkId);
            pav = trunkId;
        }
    }

    /* correct location for MC SA test FIGURE 16 ATU SA Processing */

    if (SGT_MAC_ADDR_IS_MCST(srcMacPtr))
    {
        return;
    }

    /* Was SA found */
    if (status == GT_OK)
    {
        __LOG(("MAC SA found \n"));
        descrPtr->saHit = 1;

        snetSohoAtuEntryGet(devObjPtr, atuAddress, &atuEntry);

        if (SOHO_IS_ATU_ENTRY_STATIC(atuEntry))
        {
            doSa = 1;
            if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {/* Iis entry 's DPV all zeros */
                if (atuEntry.atuData == 0)
                {
                   descrPtr->saNoDpv = GT_TRUE;
                }
            }
        }
        else
        {
            fldValue = SMEM_U32_GET_FIELD(pavRegData, 13, 1);
            LOG_FIELD_VALUE("Locked port",fldValue);
             /* Locked port */
            if (fldValue)
            {
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                {
                    refreshLock = SMEM_U32_GET_FIELD(pavRegData,11, 1);
                    LOG_FIELD_VALUE("refreshLock",refreshLock);
                    if (refreshLock)
                    {
                        atuEntry.entryState = 0x7;
                    }
                }
                doSa = 1;
            }
            else
            {
                updateDpv = 1;
            }
        }

        if (doSa == 1)
        {
            /* Is this port's bit set */
            if ((atuEntry.atuData & (1 << port)) == 0)
            {
                /* Ignore wrong data */
                fldValue = SMEM_U32_GET_FIELD(pavRegData, 12, 1);
                LOG_FIELD_VALUE("Ignore wrong data",fldValue);
                if (!fldValue)
                {
                    /* Set member violation */
                    memInfoPtr->macDbMem.violation = SOHO_SRC_ATU_PORT_E;
                    violation = 1;
                }
                else
                {
                    saOk = 1;
                }
            }
            else
            {
                saOk = 1;
            }
        }
    }
    else  /* entry was not found */
    {
        __LOG(("MAC SA NOT found \n"));

        /* init atuEntry  */
        memset(&atuEntry , 0 , sizeof(atuEntry));

        if (learnInterruptEn == 0)
        {
            /* the interrupts are not allowed !!! */
            violation = 0;
            /* the update of dpv is not allowed */
            updateDpv = 0;
        }
        else
        {
            fldValue = SMEM_U32_GET_FIELD(pavRegData, 13, 1);
            LOG_FIELD_VALUE("Locked port",fldValue);
            /* Locked port */
            if (fldValue)
            {
                /* Set miss violation */
                memInfoPtr->macDbMem.violation = SOHO_MISS_ATU_E;
                violation = 1;
            }


           if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
           {/* flag over limit reached address for opal plus */

               fldValue = SMEM_U32_GET_FIELD(pavRegData, 14, 1);
               LOG_FIELD_VALUE("flag over limit reached address",fldValue);
               if (fldValue)
               {/* flag over limit  address for opal plus */
                   descrPtr->overLimit = 0;
               }
           }


            status = sfdbSohoFreeBinGet(devObjPtr, srcMacPtr,
                                        descrPtr->dbNum, &atuAddress);
            /*  All entries static */
            if (status == GT_OK)
            {
                /* Set miss violation */
                memInfoPtr->macDbMem.violation = SOHO_FULL_ATU_E;
                violation = 1;
            }
            else
            {
                updateDpv = 1;
            }
        }
    }

    if (violation)
    {
        GT_U16 * wordDataPtr;

        wordDataPtr = memInfoPtr->macDbMem.violationData;
        memset(wordDataPtr, 0, 5 * sizeof(GT_U16));

        /* DBNum */
        wordDataPtr[0] = (GT_U16)descrPtr->dbNum;

        /* SPID, even if the packet is received on trunk - the incoming port is*
         *          value that should be configured                           */
        wordDataPtr[1] = (GT_U16)descrPtr->srcPort;

        /* SA set - force swapped bytes order as documented */
        wordDataPtr[2] = srcMacPtr[1] | (srcMacPtr[0] << 8);

        wordDataPtr[3] = srcMacPtr[3] | (srcMacPtr[2] << 8);

        wordDataPtr[4] = srcMacPtr[5] | (srcMacPtr[4] << 8);


        /* set ATU Violation - no matter if interrupt enabled,
        * status register is updated
        */
        snetSohoDoInterrupt(devObjPtr,
            GLB_STATUS_REG,/*causeRegAddr*/
            GLB_CTRL_REG,/*causeMaskRegAddr*/
            1<<3/*bit 3 in global 1*/,
            1<<3/*bit 3 in global 1*/);

        return;
    }


    /* Update DPV for station move */
    if (updateDpv)
    {
        fldValue = SMEM_U32_GET_FIELD(pavRegData, 13, 1);
        LOG_FIELD_VALUE("Locked port",fldValue);
         /* Locked port */
        if (fldValue == 0)
        {
            /* PAV */
            if (pav == 0)
            {
                atuEntry.entryState = 0;
            }
            else
            {
                /* Update PAV for ATU learning */
                atuEntry.entryState = 0x7;
            }
            /* Set SA and Entry_State */
            /* word 0 */
            regAddress = atuAddress;
            fldValue = (srcMacPtr[5]) |
                       (srcMacPtr[4] << 8) |
                       (srcMacPtr[3] << 16) |
                       (srcMacPtr[2] << 24);

            descrPtr->atuEntry[0] = fldValue;
            descrPtr->atuEntryAddr = regAddress;
           /*
            It should be stored only in the end of ingress pipe
            after all bridge decisions
            smemRegSet(devObjPtr, regAddress, fldValue);
            */

            /* word 1
              regAddress = atuAddress + 0x4;
            */

            /* SA bytes */
            fldValue = srcMacPtr[1] |
                       (srcMacPtr[0] << 8);

            /* Entries state */
            fldValue |= (atuEntry.entryState << 16);

            /* Destination Ports Vector */
            fldValue |= (pav << 20);

            descrPtr->atuEntry[1] = fldValue;
         /* It should be stored only in the end of ingress pipe
            after all bridge decisions
            smemRegSet(devObjPtr, regAddress, fldValue);
        */
        }
        if ((devObjPtr->deviceType == SKERNEL_RUBY) ||
            (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType)))
        {
            /* New data */
            if (descrPtr->saHit == 0)
            {
                smemRegFldGet(devObjPtr, GLB_ATU_CTRL_REG, 3, 1, &fldValue);
                LOG_FIELD_VALUE("Learn2All",fldValue);
                /* Learn2All */
                if (fldValue)
                {
                    descrPtr->saUpdate = 1;
                }
            }
        }
        saOk = 1;
    }

    if (saOk)
    {
        isEntryMc = atuEntry.entryState;

        if (isEntryMc)
        {
            /* Use DA ATU priority */
            if (descrPtr->Mgmt)
            {
                descrPtr->priorityInfo.sa_pri.useSaPriority =
                    (atuEntry.entryState == 0xE);
            }
            else
            {
                descrPtr->priorityInfo.sa_pri.useSaPriority =
                    (atuEntry.entryState == 0xF);
            }
            descrPtr->notRateLimit = (atuEntry.entryState == 0x5);
        }
        else
        if (descrPtr->Mgmt)
        {
            descrPtr->priorityInfo.sa_pri.useSaPriority =
                  (atuEntry.entryState == 0xD);
        }
        else
        if (isStatic)
        {

            if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {/* Use SA ATU priority */
            descrPtr->priorityInfo.sa_pri.useSaPriority =
                (atuEntry.entryState == 0);
            }
            else
            /* Use SA ATU priority */
            descrPtr->priorityInfo.sa_pri.useSaPriority =
                (atuEntry.entryState == 0xF);
        }
        else
        {
            descrPtr->priorityInfo.sa_pri.useSaPriority = 0;
        }
        if (descrPtr->priorityInfo.sa_pri.useSaPriority != 0)
        {
            descrPtr->priorityInfo.sa_pri.useSaPriority = 1;
            descrPtr->priorityInfo.sa_pri.saPriority = atuEntry.priority;
        }
    }
}

/**
* @internal snetSohoMacHashCalc function
* @endinternal
*
* @brief   Calculates the hash index for the mac address table
*
* @param[in] macAddrPtr               -  pointer to the first byte of MAC address.
*                                       The hash index
*/
GT_U32 snetSohoMacHashCalc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8 * macAddrPtr
)
{
    GT_U32  hushIdx;

    sohoMacHashCalc((GT_ETHERADDR *)macAddrPtr, &hushIdx);
    if (!SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        /* the SOHO-1 devices support only 10 bits of bucket index */
        /* the SOHO-2 devices support 11 bits of bucket index */

        hushIdx &= 0x3ff;/* mask with 10 bits */
    }

    return hushIdx;
}

/**
* @internal snetSohoVlanAssign function
* @endinternal
*
* @brief   Ingress VLAN processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @note 0 = Port is a member of this VLAN and frames are to egress unmodified
*       1 = Port is a member of this VLAN and frames are to egress Untagged
*       2 = Port is a member of this VLAN and frames are to egress Tagged
*       3 = Port is not a member of this VLAN
*
*/
static GT_VOID snetSohoVlanAssign
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoVlanAssign);

    GT_U32  regAddr, fldValue,fldValue2;/* register's address and field value */
    SNET_SOHO_VTU_STC vtuEntry;         /* VTU database entry */
    GT_STATUS status;                   /* return status */
    GT_U8 violation = 0;                /* ATU violation flag */
    SOHO_DEV_MEM_INFO * memInfoPtr;     /* device's memory pointer */

    /*Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    regAddr = PORT_BASED_VLAN_MAP_REG(devObjPtr,descrPtr->srcPort);
    smemRegFldGet(devObjPtr, regAddr, 12, 4, &fldValue);
    /* Start with default database */
    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        regAddr = PORT_CTRL1_REG(devObjPtr,descrPtr->srcPort);
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            LOG_FIELD_VALUE("dbNum[0..3]",fldValue);
            smemRegFldGet(devObjPtr, regAddr, 0, 8, &fldValue2);
            LOG_FIELD_VALUE("dbNum[4..11]",fldValue2);
            fldValue = fldValue | (fldValue2 << 4);
        }
        else
        {
            LOG_FIELD_VALUE("dbNum[4..7]",fldValue);
            smemRegFldGet(devObjPtr, regAddr, 0, 4, &fldValue2);
            LOG_FIELD_VALUE("dbNum[0..3]",fldValue2);
            fldValue = (fldValue <<4) | fldValue2;
        }
    }
    else
    {
        LOG_FIELD_VALUE("dbNum[4..7]",fldValue);
    }
    descrPtr->dbNum = fldValue;
    /* Start with default VTU vector */
    __LOG(("Start with default VTU vector"));
    descrPtr->vtuHit = 0;

    if (!SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 10, 2, &fldValue);
        LOG_FIELD_VALUE("802.1q",fldValue);
        /* 802.1q disabled */
        if (fldValue == 0)
        {
            return;
        }
    }
    else
    {
        descrPtr->vtuVector = 0x7ff;
    }

    __LOG(("Start vlan entry lookup for vid[%d] \n",descrPtr->vid));
    status = smemSohoVtuEntryGet(devObjPtr, descrPtr->vid, &vtuEntry);
    if (status == GT_NOT_FOUND)
    {
        __LOG(("the vlan entry was not found : vid[%d] \n",descrPtr->vid));
        descrPtr->vtuMiss = 1;
        /* Set miss violation */
        memInfoPtr->vlanDbMem.violation = SOHO_MISS_VTU_VID_E;
        violation = 1;
    }
    else
    {
        __LOG(("the vlan entry was found : vid[%d] \n",descrPtr->vid));

        /* Is SPID member */
        if ((vtuEntry.portsMap[descrPtr->srcPort] & 0x3) == 3)
        {
            __LOG(("the source port[%d] is not member in vid[%d] --> Set 'member violation' \n",
                descrPtr->srcPort,
                descrPtr->vid));
            /* Set member violation */
            memInfoPtr->vlanDbMem.violation = SOHO_SRC_VTU_PORT_E;
            violation = 1;
        }
        else
        {
            descrPtr->spOk = 1;
        }

        descrPtr->vtuHit = 1;

        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
             descrPtr->dbNum = vtuEntry.dbNum;
             /* CALCULATE SBIT VALUE */
             descrPtr->sBit = vtuEntry.sid;

             descrPtr->policyVid = vtuEntry.vidPolicy;
        }
        else
        {
            descrPtr->dbNum = vtuEntry.dbNum;
        }

        SOHO_MEMBER_TAG_2_VTU_VECTOR(vtuEntry.portsMap, descrPtr->vtuVector);

        regAddr  = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 0, 2, &fldValue);
        LOG_FIELD_VALUE("PortState",fldValue);
        /* PortState */
        if (fldValue == 0)
        {
            descrPtr->vtuVector &= ~(1 << descrPtr->srcPort);
        }
        descrPtr->priorityInfo.vtu_pri.useVtuPriority = (GT_U8)(vtuEntry.pri & 0x8);
        descrPtr->priorityInfo.vtu_pri.vtuPriority = (GT_U8)(vtuEntry.pri & 0x7);
    }

    if (violation)
    {
        GT_U16 * wordDataPtr;

        wordDataPtr = &memInfoPtr->vlanDbMem.violationData[0];
        memset(wordDataPtr, 0, 5 * sizeof(GT_U16));

        /* SPID */
        wordDataPtr[0] = (GT_U16)descrPtr->srcPort;

        /* VID */
        wordDataPtr[1] = descrPtr->vid;

        snetSohoDoInterrupt(devObjPtr,
            GLB_STATUS_REG,/*causeRegAddr*/
            GLB_CTRL_REG,/*causeMaskRegAddr*/
            1<<5/*bit 5 in global 1*/,
            1<<5/*bit 5 in global 1*/);
    }
}

/**
* @internal snetSohoFrameProcess function
* @endinternal
*
* @brief   Process the frame, get and do actions for a frame
*/
static GT_VOID snetSohoFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
)
{
    DECLARE_FUNC_NAME(snetSohoFrameProcess);

    SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr; /* pointer to frame's descriptor */
    SNET_SOHO_MIRROR_STC mirrStc;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E);

    /* save info to log */
    __LOG((SIM_LOG_IN_PACKET_STR
            "start new frame on: deviceName[%s],deviceId[%d], srcPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            srcPort));

    descrPtr = (SKERNEL_FRAME_SOHO_DESCR_STC *)devObjPtr->descriptorPtr;
    memset(descrPtr, 0, sizeof(SKERNEL_FRAME_SOHO_DESCR_STC));
    memset(&mirrStc, 0, sizeof(SNET_SOHO_MIRROR_STC));

    SIM_SOHO_LOG_PACKET_DESCR_SAVE

    descrPtr->frameBuf = bufferId;
    descrPtr->srcPort = srcPort;
    descrPtr->byteCount = (GT_U16)bufferId->actualDataSize;

    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoFrameProcess : prepare packet from network port \n");

    __LOG(("frame dump:  \n"));
    simLogSohoPacketDescrFrameDump(devObjPtr, descrPtr);

    /* Ingress packet rules for incoming packet */
    __LOG(("Ingress packet rules for incoming packet"));
    snetSohoIngressPacketProc(devObjPtr, descrPtr);

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    /* QC Monitor - Ingress and Egress */
    __LOG(("QC Monitor - Ingress \n"));
    snetSohoQcMonitorIngress(devObjPtr, descrPtr , &mirrStc);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoQcMonitorIngress \n");

    __LOG(("QC Monitor - Egress \n"));
    snetSohoQcMonitorEgress(devObjPtr, descrPtr  , &mirrStc);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    __LOG((SIM_LOG_ENDED_IN_PACKET_STR
            "ended processing frame from: deviceName[%s],deviceId[%d], srcPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            srcPort));
}

/**
* @internal snetSohoIngressPacketProc function
* @endinternal
*
* @brief   Ingress packet rules for incoming packets
*/
static GT_VOID snetSohoIngressPacketProc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    snetSohoHeaderProc(devObjPtr, descrPtr);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoHeaderProc \n");

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    snetSohoIpPriorityAssign(devObjPtr, descrPtr);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoIpPriorityAssign \n");

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    snetSohoIgmpSnoop(devObjPtr, descrPtr);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoIgmpSnoop \n");

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    snetSohoPortMapUpdate(devObjPtr, descrPtr);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoPortMapUpdate \n");

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    snetSohoPriorityAssign(devObjPtr, descrPtr);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoPriorityAssign \n");

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    snetSohoL2Decision(devObjPtr, descrPtr);
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoL2Decision \n");

    SIM_SOHO_LOG_PACKET_DESCR_SAVE
    /* protect RX MAC MIB counters from simultaneous access with management. */
    SCIB_SEM_TAKE;
    snetSohoStatCountUpdate(devObjPtr, descrPtr);
    SCIB_SEM_SIGNAL;
    SIM_SOHO_LOG_PACKET_DESCR_COMPARE("snetSohoStatCountUpdate \n");
}
/**
* @internal snetSohoDaTranslation function
* @endinternal
*
* @brief   Destination address translation process
*/
static GT_VOID snetSohoDaTranslation
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoDaTranslation);

    GT_U8   isEntryMng;                 /* Is ATU entry management */
    GT_STATUS status;                   /* return status */
    SNET_SOHO_ATU_STC  atuEntry;        /* ATU database entry */
    GT_U8 isEntryMc = 0, isStatic = 0;  /* ATU database entry state*/
    GT_U32 regAddress;                  /* register's address */
    GT_U32 fldValue;                    /* register's field value */
    GT_U32 ports;                       /* device ports number */
    SOHO_DEV_MEM_INFO * memInfoPtr;     /* device's memory pointer */
    GT_U32 atuData = 0;                 /* Destination port vector/TrunkID */
    GT_U32 trunkIdx;                    /* trunk mask index */

    ports = devObjPtr->portsNumber;
    /* Get pointer to the device memory */
    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    status = sfdbSohoAtuEntryAddress(devObjPtr, descrPtr->dstMacPtr,
                                     descrPtr->dbNum, &regAddress);
    if (status != GT_OK)
    {
        __LOG(("MAC DA NOT found \n"));
        return;
    }

    __LOG(("MAC DA found \n"));
    descrPtr->daHit = 1;
    snetSohoAtuEntryGet(devObjPtr, regAddress, &atuEntry);


    /* DA found */
    isEntryMng = ( ((SGT_MAC_ADDR_IS_MCST_BY_FIRST_BYTE(atuEntry.macAddr.bytes[0]) &&
                    ((atuEntry.entryState & 0x7) == 0x6)) ) ||
                 (((atuEntry.entryState == 0xc) ||
                   (atuEntry.entryState == 0xd)) &&
                  (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType)) ));


    atuData = atuEntry.atuData;

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        if ((atuEntry.trunk) &&
            (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
        {
            /* TRNUK MAP */
            __LOG(("access TRNUK MAP"));
            smemRegFldGet(devObjPtr, GLB2_TRUNK_MASK_REG,0,11, &trunkIdx);
            LOG_FIELD_VALUE("trunk mask",trunkIdx);
            smemRegFldGet(devObjPtr, GLB2_TRUNK_ROUT_REG,0,11, &atuData);
            LOG_FIELD_VALUE("trunk rout",atuData);
            atuData = atuData & trunkIdx;

        }
        else if (atuEntry.trunk)
        {
            trunkIdx = atuData & 0xf;
            __LOG(("mac entry on trunk[%d] \n",trunkIdx));
            atuData = memInfoPtr->trunkRouteMem.trouteTblMem[trunkIdx] & 0x7ff;
        }
    }

    if (isEntryMng)
    {
        __LOG(("mac entry is Mgmt \n"));
        descrPtr->Mgmt = 1;
    }
    else
    {
        /* Is entry MC */
        if (SGT_MAC_ADDR_IS_MCST_BY_FIRST_BYTE(atuEntry.macAddr.bytes[0]))
        {
            __LOG(("mac entry is MC \n"));
            isEntryMc = 1;
        }
        else
        /* Is entry static */
        if (atuEntry.entryState & 0xe)
        {
            __LOG(("mac entry is static \n"));
            isStatic = 1;
            descrPtr->daStatic = 1;
        }
    }
    if (isEntryMc)
    {
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            /* Use DA ATU priority */
            __LOG(("Use DA ATU priority"));
            if (descrPtr->Mgmt)
            {
                descrPtr->priorityInfo.da_pri.useDaPriority =
                    (atuEntry.entryState == 0x6);
            }
            else
            {
                descrPtr->priorityInfo.da_pri.useDaPriority = 0;
            }

                descrPtr->policyDa = (atuEntry.entryState == 0x4);

        }
        else
        {
            /* Use DA ATU priority */
            __LOG(("Use DA ATU priority"));
            if (descrPtr->Mgmt)
            {
                descrPtr->priorityInfo.da_pri.useDaPriority =
                    (atuEntry.entryState == 0xF);
            }
            else
            {
                descrPtr->priorityInfo.da_pri.useDaPriority = 0;
            }

        }
        descrPtr->notRateLimit = (atuEntry.entryState == 0x5);
    }
    else if (descrPtr->Mgmt)
    {
        descrPtr->priorityInfo.da_pri.useDaPriority =
              (atuEntry.entryState == 0xD);
    }
    if (isStatic)
    {
        /* Use DA ATU priority */
        descrPtr->priorityInfo.da_pri.useDaPriority =
            (atuEntry.entryState == 0xE);

        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            /* Use DA ATU priority */
            if (descrPtr->Mgmt)
            {
                descrPtr->priorityInfo.da_pri.useDaPriority =
                    (atuEntry.entryState == 0x6);
            }
            else
            {
                descrPtr->priorityInfo.da_pri.useDaPriority = 0;
            }

                descrPtr->policyDa = (atuEntry.entryState == 0x4);
                descrPtr->notRateLimit = (atuEntry.entryState == 0x5);

        }
    }
    else
    {
        descrPtr->priorityInfo.da_pri.useDaPriority = 0;
    }
    if (descrPtr->priorityInfo.da_pri.useDaPriority != 0)
    {
        descrPtr->priorityInfo.da_pri.daPriority =
         (GT_U8)(SMEM_U32_GET_FIELD(atuEntry.atuData, 11, 1) |
                 SMEM_U32_GET_FIELD(atuEntry.atuData, 10, 2) << 1);
        LOG_FIELD_VALUE("daPriority",descrPtr->priorityInfo.da_pri.daPriority);
    }
    if (!SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        regAddress = PORT_ASSOC_VECTOR_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddress, 15, 1, &fldValue);
        LOG_FIELD_VALUE("SPID ingress monitor",fldValue);
         /* SPID ingress monitor */
        if (fldValue)
        {
            /* Mask the SPID's bit in the SPID's PAV */
            atuData &= ~(1 << descrPtr->srcPort);
            smemRegFldGet(devObjPtr, regAddress, 0, ports, &fldValue);
            LOG_FIELD_VALUE("SPID's PAV",fldValue);
            fldValue &= ~(1 << descrPtr->srcPort);
            atuData |= fldValue;
        }
    }

    /* update the vector */
    descrPtr->destPortVector = atuData;
}

/**
* @internal snetSohoAtuEntryGet function
* @endinternal
*
* @brief   Format 64-bit ATU entry to SNET_SOHO_ATU_STC structure
*/
static GT_VOID snetSohoAtuEntryGet
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_U32 atuAddress,
    OUT SNET_SOHO_ATU_STC * atuEntryPtr
)
{
    DECLARE_FUNC_NAME(snetSohoAtuEntryGet);
    GT_U32 * atuWordPtr;                /* ATU word pointer */

    ASSERT_PTR(atuEntryPtr);

    /* Pointer to ATU word in SRAM */
    atuWordPtr = smemMemGet(devObjPtr, atuAddress);
    SOHO_MSG_2_MAC((GT_U8 *)atuWordPtr, atuEntryPtr->macAddr);
    atuEntryPtr->entryState =
        (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[1], 16, 4);
    LOG_FIELD_VALUE("ATU.entryState",atuEntryPtr->entryState);

    /* Sapphire/Ruby */
    if (devObjPtr->deviceType == SKERNEL_SAPPHIRE)
    {
        atuEntryPtr->atuData =
            (GT_U16)SMEM_U32_GET_FIELD(atuWordPtr[1], 20, 10);
        LOG_FIELD_VALUE("ATU.atuData",atuEntryPtr->atuData);
        atuEntryPtr->priority =
            (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[1], 30, 1) << 1 |
            (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[2],  0, 1);
        LOG_FIELD_VALUE("ATU.priority",atuEntryPtr->priority);
    }
    else if(SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        /* 11 bits of ports bmp / trunkId(0 based) */
        atuEntryPtr->atuData =
            (GT_U16)SMEM_U32_GET_FIELD(atuWordPtr[1], 20, 11);
        LOG_FIELD_VALUE("ATU.atuData",atuEntryPtr->atuData);

        atuEntryPtr->priority =
                (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[1], 31, 1) |      /* LSB */
                (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[2], 0, 2) << 1;   /* MSB */
        LOG_FIELD_VALUE("ATU.priority",atuEntryPtr->priority);
        atuEntryPtr->trunk =
                (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[2], 2, 1) ;
        LOG_FIELD_VALUE("ATU.trunk",atuEntryPtr->trunk);
        if ((SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
        {
            atuEntryPtr->fid =
                (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[2], 3, 1) ;
            LOG_FIELD_VALUE("ATU.fid",atuEntryPtr->fid);
        }
    }
    else
    {
        atuEntryPtr->atuData =
            (GT_U16)SMEM_U32_GET_FIELD(atuWordPtr[1], 20, 11);
        LOG_FIELD_VALUE("ATU.atuData",atuEntryPtr->atuData);
        atuEntryPtr->priority =
            (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[1], 31, 1) |      /* LSB */
            (GT_U8) SMEM_U32_GET_FIELD(atuWordPtr[2], 0, 2) << 1;   /* MSB */
        LOG_FIELD_VALUE("ATU.priority",atuEntryPtr->priority);
        atuEntryPtr->trunk = 0;

    }
}


/**
* @internal snetSohoCreateMarvellTag function
* @endinternal
*
* @brief   Get Marvell tag from descriptor
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - description pointer
*/
GT_U32 snetSohoCreateMarvellTag
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    INOUT SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoCreateMarvellTag);

    GT_U32 mrvlTag;
    GT_U32 mrvlTagCmd = 0;
    GT_U8  cpu_code=0;

    __LOG_PARAM(descrPtr->rxSnif);
    __LOG_PARAM(descrPtr->txSnif);
    __LOG_PARAM(descrPtr->pktCmd);

    if (descrPtr->rxSnif)
    {
        descrPtr->srcTrunk = 1;
        mrvlTagCmd =  TAG_CMD_TO_TARGET_SNIFFER_E;
        __LOG_PARAM(descrPtr->srcTrunk);
    }
    else
    if (descrPtr->txSnif)
    {
        mrvlTagCmd =  TAG_CMD_TO_TARGET_SNIFFER_E;
    }
    else
    if (descrPtr->pktCmd == SKERNEL_PKT_FORWARD_E)
    {
       mrvlTagCmd = TAG_CMD_FORWARD_E;
    }
    else
    if (descrPtr->pktCmd == SKERNEL_PKT_TRAP_CPU_E)
    {
       mrvlTagCmd = TAG_CMD_TO_CPU_E;
       if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
       {
           if (descrPtr->Mgmt)
           {
                if (descrPtr->arp)
                {
                    cpu_code = 0x4;
                }
                else
                {
                    cpu_code = 0x0;
                }
           }
           else if (descrPtr->daType.igmp)
           {
                cpu_code = 0x2;
           }
       }
       else
       {
           if (descrPtr->Mgmt)
           {
                cpu_code=0x2;
           }
           else if (descrPtr->daType.igmp)
           {
                cpu_code=0x6;
           }
       }

        __LOG_PARAM(cpu_code);
    }
    __LOG_PARAM(mrvlTagCmd);

    mrvlTag = mrvlTagCmd << 30;

    if (mrvlTagCmd == TAG_CMD_FORWARD_E)
    {
        __LOG_PARAM(descrPtr->srcVlanTagged);
        mrvlTag |= ((descrPtr->srcVlanTagged & 0x01) << 29);

        /* Src Dev */
        __LOG_PARAM(descrPtr->srcDevice);
        mrvlTag |= ((descrPtr->srcDevice & 0x1F) << 24);
        __LOG_PARAM(descrPtr->srcTrunk);
        if (descrPtr->srcTrunk)
        {
            mrvlTag |= (1 << 18);
            __LOG_PARAM(descrPtr->trunkId);
            /* Src_trunk */
            mrvlTag |= ((descrPtr->trunkId & 0x0F) << 19);
        }
        else
        {
            /* Src_Port */
            __LOG_PARAM(descrPtr->srcPort);
            mrvlTag |= ((descrPtr->srcPort & 0x1F) << 19);
        }
    }

    if (mrvlTagCmd == TAG_CMD_TO_CPU_E)
    {
        __LOG_PARAM(descrPtr->rmtMngmt);
        if (descrPtr->rmtMngmt)
        {
            /* Src_Tagged replace with 0! */
            mrvlTag |= ( 0x00 << 29);
            /* Src Dev */
            mrvlTag |= ((descrPtr->srcDevice & 0x1F) << 24);
            /* Second octet = 0 */
            mrvlTag |= (0  << 16);
            /* remote management bit  */
            mrvlTag |= ( 0x1 << 12);
            /*   Sequnence + tag DSA 0XF vid and pri will be added later */
        }
        else
        {
            /* Src_Tagged */
            __LOG_PARAM(descrPtr->srcVlanTagged);
            mrvlTag |= ((descrPtr->srcVlanTagged & 0x01) << 29);

            /* Src Dev */
            __LOG_PARAM(descrPtr->srcDevice);
            mrvlTag |= ((descrPtr->srcDevice & 0x1F) << 24);

            /* Src_Port */
            __LOG_PARAM(descrPtr->srcPort);
            mrvlTag |= ((descrPtr->srcPort & 0x1F) << 19);

            __LOG_PARAM(cpu_code);
            if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
            {
                /* CPU_Code[2:1] : The '>>' operator is a must in the
                                   expression '(cpu_code >> 1) & 0x3)' .
                                   Only bits 1 , 2 are saved in the 3rd
                                   Octet of the marvell tag .
                                   The first bit(0) is saved on the 4th octet.
                                   Tuvia(21-Aug-05)
                */
                mrvlTag |= ( ((cpu_code >> 1) & 0x3) << 17);
            }
            else
            {
                /* CPU_Code[3:1] */
                mrvlTag |= ( ((cpu_code >> 1) & 0x7) << 16);
            }
            /* CPU_Code[0] **** vid and pri will be added later */
            mrvlTag |= ( (cpu_code & 0x1) << 12);
        }
    }

    if (mrvlTagCmd == TAG_CMD_TO_TARGET_SNIFFER_E)
    {
        /* rx_sniff */
        __LOG_PARAM(descrPtr->rxSnif);
        mrvlTag |= ((descrPtr->rxSnif & 0x01) << 18);
    }

    if ((SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType) &&
        (descrPtr->rmtMngmt == 0)))
    {
        /* Frame's CFI */
        __LOG(("Frame's CFI"));
        __LOG_PARAM(descrPtr->yellow);
        mrvlTag |= (descrPtr->yellow & 0x1) << 16;
    }
    /* add vlan tag with vpt.
      if descrPtr->rmtMngmt is TRUE then the vid field is of the form 0xfXX
      where XX is the RMU 8 bits sequence number.
    */
    __LOG_PARAM(descrPtr->vid);
    __LOG_PARAM(descrPtr->fPri);
    mrvlTag |= ((descrPtr->vid & 0xfff) | ((descrPtr->fPri & 0x7) << 13));

    __LOG_PARAM(mrvlTag);

    return SGT_LIB_SWAP_BYTES_AND_WORDS_MAC(mrvlTag);
}

/**
* @internal snetSohoHeaderProc function
* @endinternal
*
* @brief   Header processing, DA and SA Capture and Double Tag Removal
*/
static GT_VOID snetSohoHeaderProc
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoHeaderProc);

    GT_U32 regAddress;                  /* Register's address */
    GT_U32 headerState;
    SBUF_BUF_STC * frameBufPtr;
    GT_U32  cascadedPort;
    GT_U32  dbNum=0;
    GT_U32  learnDis=0;
    GT_U32  ignoreFcs=0;
    GT_U32  vlanTable=0;
    GT_U8 * headerPtr;
    GT_U32  regAddr;                    /* Register's address */
    GT_U32* regValPtr;                  /* Register's value pointer */
    GT_U32  fldValue;                   /* Register's field value */
    GT_U32 ports;                       /* ports number */
    GT_U8  doubleTag = 0;               /* double tag flag */
    GT_U32  frameMode;

    ports = devObjPtr->portsNumber;

    regAddress  = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
    regValPtr = smemMemGet(devObjPtr, regAddress);

    /* port in header mode */
    headerState = SMEM_U32_GET_FIELD(regValPtr[0], 11, 1);
    LOG_FIELD_VALUE("ATU.headerState",headerState);
    frameBufPtr = descrPtr->frameBuf;
    headerPtr = frameBufPtr->actualDataPtr;

    /* Set byte count from actual buffer's length */
    descrPtr->byteCount = (GT_U16)frameBufPtr->actualDataSize;
    if (headerState)
    {
        /* write the header to the port based vlan register */
        if ( (headerPtr[0] != 0) && (headerPtr[1] != 0) )
        {
            __LOG(("write the header to the port based vlan register"));
            dbNum =  (frameBufPtr->actualDataPtr[0] >> 4) & 0xf;
            SMEM_U32_SET_FIELD(regValPtr[0], 12,  4, dbNum);

            learnDis = (headerPtr[0] >> 3) & 0x1;
            SMEM_U32_SET_FIELD(regValPtr[0], 11,  1, learnDis);

            if (devObjPtr->deviceType == SKERNEL_SAPPHIRE)
            {
                ignoreFcs = (headerPtr[0] >> 2) & 0x1;
                SMEM_U32_SET_FIELD(regValPtr[0], 10,  1, ignoreFcs);
                vlanTable = ( (headerPtr[0] & 0x3) << 8) | headerPtr[1];
            }
            else
            {
                vlanTable = ( (headerPtr[0] & 0x7) << 8) | headerPtr[1];
            }

            regAddr = PORT_BASED_VLAN_MAP_REG(devObjPtr,descrPtr->srcPort);
            smemRegFldSet(devObjPtr, regAddr,  0, ports, vlanTable);

            SET_LOG_FIELD_VALUE("vlanTable",vlanTable);
        }
        /* remove ingress header */
        headerPtr+=2;
        descrPtr->byteCount-=2;
    }

    /* Set destination MAC pointer */
    descrPtr->dstMacPtr = frameBufPtr->actualDataPtr;
    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        if (SGT_MAC_ADDR_IS_MGMT(descrPtr->dstMacPtr))
        {
            smemRegFldGet(devObjPtr, GLB2_MNG_REG, 3, 1, &fldValue);
            /* Rsvd2CPU */
            LOG_FIELD_VALUE("Rsvd2CPU",fldValue);
            if (fldValue)
            {
                GT_U8 daBits;
                daBits = descrPtr->dstMacPtr[5] & 0xf;
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                {
                    smemRegFldGet(devObjPtr, GLB2_MGMT_EN_REG_2X, 0, 16, &fldValue);
                }
                else
                {
                    smemRegFldGet(devObjPtr, GLB2_MGMT_EN_REG, 0, 16, &fldValue);
                }
                LOG_FIELD_VALUE("MGMT Enables",fldValue);
                if (fldValue & (1 << daBits))
                {
                    descrPtr->Mgmt = 1;
                }
            }
            if (SGT_MAC_ADDR_IS_PAUSE(descrPtr->dstMacPtr))
            {
              descrPtr->daType.pause = 1;
            }
        }
    }
    else
    {
        GT_U32 etherType, etherTypeOffset;

        /* check if the frame is igmp/mld */
        if (SGT_MAC_ADDR_IS_MLD(descrPtr->dstMacPtr))
        {
          descrPtr->daType.mld = 1;
        }
        /* ------- check for IGMP frame ------- */
        __LOG(("------- check for IGMP frame -------"));
        if (descrPtr->srcVlanTagged == 1)
        {
            etherTypeOffset = 16;
        }
        else
        {
            etherTypeOffset = 12;
        }
        etherType = descrPtr->dstMacPtr[etherTypeOffset] << 8 |
                    descrPtr->dstMacPtr[etherTypeOffset + 1];

        if(etherType == 0x0800)
        {/* if it's IP frame */
            if (SGT_MAC_ADDR_IS_IPMC(descrPtr->dstMacPtr))
            {/* if it's IP multicast frame */
                if(descrPtr->dstMacPtr[etherTypeOffset+11] == 2)
                {/* if IP protocol is IGMP */
                    descrPtr->daType.igmp = 1;
                }
            }
        }

        if (SGT_MAC_ADDR_IS_PAUSE(descrPtr->dstMacPtr))
        {
          descrPtr->daType.pause = 1;
        }
    }

    if (descrPtr->daType.pause)
    {
        if (!((headerPtr[12] == 0x88) && (headerPtr[13] == 0x08)))
        {
               descrPtr->daType.pause = 0;
        }
        else if (!((headerPtr[14] == 0x00) && (headerPtr[15] == 0x01)))
        {
               descrPtr->daType.pause = 0;
        }
        if (descrPtr->daType.pause)
        {
            descrPtr->pauseTime = headerPtr[16] & headerPtr[17];
            return;
        }
    }

    frameMode = 0;
    if(!SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        /* The packet is not pause */
        smemRegFldGet(devObjPtr, regAddress, 8, 1, &cascadedPort);
        LOG_FIELD_VALUE("cascaded Port",cascadedPort);
    }
    else
    {
        smemRegFldGet(devObjPtr, regAddress, 8, 2, &frameMode);
        LOG_FIELD_VALUE("FrameMode",frameMode);

        cascadedPort = 0;
        if(frameMode == 0)
        {
            __LOG(("port[%d] is network port\n"
                ,descrPtr->srcPort));
        }
        else
        if(frameMode == 1)
        {
            cascadedPort = 1;
            __LOG(("port[%d] is cascade port\n"
                ,descrPtr->srcPort));
        }
        else
        if(frameMode == 2)
        {
            __LOG(("port[%d] is Provider port (core port)\n"
                ,descrPtr->srcPort));
            descrPtr->isProviderPort = 1;
        }
    }

    if (cascadedPort)
    {
        descrPtr->cascade = 1;
        descrPtr->srcVlanTagged = (frameBufPtr->actualDataPtr[12] >> 5) & 0x1;
        descrPtr->qPri = (headerPtr[14] >> 6) & 0x3;
        descrPtr->marvellTagCmd = (headerPtr[12] >> 6) & 0x3;
        if (descrPtr->marvellTagCmd == TAG_CMD_FROM_CPU_E)
        {
            descrPtr->qPri = (headerPtr[14] & 0x3);
        }
        else if (descrPtr->marvellTagCmd == TAG_CMD_TO_TARGET_SNIFFER_E)
        {
            if ((devObjPtr->deviceType == SKERNEL_RUBY) ||
                (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType)))
            {
                descrPtr->rxSnif = (headerPtr[13] >> 2) & 0x1;
            }
        }
        else if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
        {
            if(descrPtr->marvellTagCmd == TAG_CMD_FORWARD_E)
            {
                descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;
                /* bit 18 in mrvl tag */
                descrPtr->srcTrunk = (headerPtr[13] >> 2) & 0x1;
                if (descrPtr->srcTrunk)
                {
                    /* bit 19-23 (5 bits) in mrvl tag --
                       but Jade/Opal support only 4 bits of trunk */
                    descrPtr->trunkId = (headerPtr[13] >> 3) & 0x0F;

                }
                else
                {
                    regAddr = PORT_CTRL1_REG(devObjPtr,descrPtr->srcPort);
                    smemRegFldGet(devObjPtr, regAddr,  14, 1, &fldValue);
                    LOG_FIELD_VALUE("Trunk port",fldValue);
                    /* Trunk port */
                    descrPtr->srcTrunk = (GT_U8)fldValue;
                    smemRegFldGet(devObjPtr, regAddr,  4, 4, &fldValue);
                    LOG_FIELD_VALUE("Trunk ID",fldValue);
                    /* Trunk ID */
                    descrPtr->trunkId = (GT_U8)fldValue;
                }
            }
        }

        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            snetOpalPvtCollect(devObjPtr, descrPtr);
        }

        return;
    }

    if ((devObjPtr->deviceType == SKERNEL_RUBY) ||
        (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType)))
    {
        descrPtr->tagVal = 0x8100;
        if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
        {
            regAddr = PORT_CTRL1_REG(devObjPtr,descrPtr->srcPort);
            smemRegFldGet(devObjPtr, regAddr,  14, 1, &fldValue);
            LOG_FIELD_VALUE("Trunk port",fldValue);
            /* Trunk port */
            descrPtr->srcTrunk = (GT_U8)fldValue;
            if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {
                smemRegFldGet(devObjPtr, regAddr,  8, 4, &fldValue);
            }
            else
            {
                smemRegFldGet(devObjPtr, regAddr,  4, 4, &fldValue);
            }
            LOG_FIELD_VALUE("Trunk ID",fldValue);
            /* Trunk ID */
            descrPtr->trunkId = (GT_U8)fldValue;
        }

        if(!SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            smemRegFldGet(devObjPtr, regAddress, 15, 1, &fldValue);
            LOG_FIELD_VALUE("Use core tag ",fldValue);


            descrPtr->isProviderPort = fldValue;
        }

        /* Use core tag */
        if (descrPtr->isProviderPort)
        {
            if(!SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {
                smemRegGet(devObjPtr, GLB_CORE_TAG_TYPE_REG, &fldValue);
                LOG_FIELD_VALUE("Global <core tag> (etherType)",fldValue);
            }
            else
            {
                regAddr = PORT_ETYPE(devObjPtr,descrPtr->srcPort);
                smemRegFldGet(devObjPtr, regAddr, 0, 16, &fldValue);
                LOG_FIELD_VALUE("(per port) <PortEType> (etherType)",fldValue);
            }

            /* Core Tag Type */
            descrPtr->tagVal = (GT_U16)fldValue;
            /* Use double tag data */
            if ((headerPtr[12] << 8 | headerPtr[13]) == descrPtr->tagVal)
            {
                descrPtr->dtPri = (headerPtr[14] >> 5) & 0x7;
                descrPtr->dtDei = (headerPtr[14] >> 4) & 0x1;
                descrPtr->dtVid = ((headerPtr[14] & 0xf) << 8) | headerPtr[15];
                descrPtr->useDt = 1;
                /* Remove bytes 13 to 16 */
                memmove (&headerPtr[12], &headerPtr[16], descrPtr->byteCount - 16);
                descrPtr->byteCount-= 4;
                descrPtr->modified = 1;
                descrPtr->tagVal = 0x8100;
            }
        }
        else
        if(!SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            regAddr = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
            smemRegFldGet(devObjPtr, regAddr,  9, 1, &fldValue);
            LOG_FIELD_VALUE("DoubleTag",fldValue);
            /* DoubleTag */
            if (fldValue)
            {
                if ((headerPtr[12] << 8 | headerPtr[13]) == descrPtr->tagVal)
                {
                    doubleTag = 1;
                }
            }
        }

        if (doubleTag)
        {
            if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
            {
                smemRegFldGet(devObjPtr, GLB2_MNG_REG, 15, 1, &fldValue);
                LOG_FIELD_VALUE("Use double tag data",fldValue);
            }
            else
            {
                fldValue = 0;
            }
            /* Use double tag data */
            if (fldValue)
            {
                descrPtr->dtPri = (headerPtr[14] >> 5) & 0x7;
                descrPtr->dtDei = (headerPtr[14] >> 4) & 0x1;
                descrPtr->dtVid = headerPtr[14] & 0xf;
                descrPtr->useDt = 1;
            }
            /* Remove bytes 13 to 16 */
            memcpy(&headerPtr[12], &headerPtr[15], descrPtr->byteCount - 15);
            descrPtr->byteCount-= 4;
            descrPtr->modified = 1;
            descrPtr->tagVal = 0x8100;
        }
    }
}

/**
* @internal snetSohoCascadeFrame function
* @endinternal
*
* @brief   Cascade Frame Port Mapping
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @retval GT_TRUE                  - Skip PORT_VEC and PRI processing
* @retval GT_FALSE                 - Perform Normal Packet Processing
*/
static GT_BOOL snetSohoCascadeFrame
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoCascadeFrame);

    GT_U32 regAddr;                     /* register's address */
    GT_U32 trgPort;                     /* target port */
    GT_U32 fieldValue;                  /* Register's field value */
    GT_BOOL retVal;                     /* Return status */
    GT_U32 mrvlTag;                     /* Marvell tag */
    GT_U32 trgDev;                      /* Target device */

    /* build the MARVELL tag */
    mrvlTag = DSA_TAG(descrPtr->frameBuf->actualDataPtr);

    retVal = GT_TRUE;

    if (descrPtr->marvellTagCmd == TAG_CMD_FROM_CPU_E)
    {
        trgDev = SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("trgDev",trgDev);
        if (trgDev == descrPtr->srcDevice)
        {
            /* Target port from Marvell Tag */
            __LOG(("Target port from Marvell Tag"));
            trgPort = SMEM_U32_GET_FIELD(mrvlTag, 19, 5);
            LOG_FIELD_VALUE("trgPort",trgPort);
            descrPtr->destPortVector = (1 << trgPort);
        }
        else
        {
            smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 12, 4, &fieldValue);
            LOG_FIELD_VALUE("CAS_PORT",fieldValue);
            /* Cascade port */
            if (fieldValue == 0xf)
            {
                descrPtr->destPortVector = (1 << trgDev);
            }
            else
            {
                trgPort = fieldValue;
                descrPtr->destPortVector = (1 << trgPort);
            }
        }
        descrPtr->Mgmt = 1;
    }
    else
    if (descrPtr->marvellTagCmd == TAG_CMD_TO_CPU_E)
    {
        descrPtr->srcDevice = (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("srcDevice",descrPtr->srcDevice);
        regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 0, 4, &fieldValue);
        LOG_FIELD_VALUE("CPU_Port",fieldValue);
        /*  CPU port */
        trgPort = fieldValue;
        descrPtr->destPortVector = (1 << trgPort);
        descrPtr->Mgmt = 1;
    }
    else
    if (descrPtr->marvellTagCmd == TAG_CMD_TO_TARGET_SNIFFER_E)
    {
        descrPtr->srcDevice = (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("srcDevice",descrPtr->srcDevice);
        descrPtr->destPortVector = 0;
        descrPtr->Mgmt = 1;
    }
    else
    {
        descrPtr->srcDevice = (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("srcDevice",descrPtr->srcDevice);
        retVal = GT_FALSE;
    }

    return retVal;
}

/**
* @internal snetSohoCascadeRubyFrame function
* @endinternal
*
* @brief   Cascade Frame Port Mapping
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @retval GT_TRUE                  - Skip PORT_VEC and PRI processing
* @retval GT_FALSE                 - Perform Normal Packet Processing
*/
static GT_BOOL snetSohoCascadeRubyFrame
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoCascadeRubyFrame);

    GT_U32      trgPort;                    /* target port */
    GT_U32      trgDev;                     /* Target device */
    GT_U32      srcDevice;                  /* Source device */
    GT_U32      regAddr;                    /* register's address */
    GT_U32      fieldValue;                 /* register's field value */
    GT_BOOL     retVal;                     /* return value */
    SOHO_DEV_MEM_INFO * memInfoPtr;         /* device's memory pointer */
    GT_U32      mrvlTag;                    /* Marvell tag */
    GT_U32      mrvlTagCpuCode;             /* Marvell tag CPU code */
    GT_U32      flowCtrl;                   /* Flow control */


    memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);

    /* build the MARVELL tag */
    mrvlTag = DSA_TAG(descrPtr->frameBuf->actualDataPtr);

    retVal = GT_TRUE;

    if (descrPtr->marvellTagCmd == TAG_CMD_TO_CPU_E)
    {
        __LOG(("Ingress DSA : 'TO_CPU' format \n"));
        descrPtr->srcDevice =
            (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("srcDevice",descrPtr->srcDevice);

        descrPtr->origSrcPortOrTrnk =
            (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 19, 5);
        LOG_FIELD_VALUE("origSrcPortOrTrnk",descrPtr->origSrcPortOrTrnk);

        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            /*Monitor Control register Offset: 0x1A or decimal 26
             the field CPU Dest - bits 4:7*/
            smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fieldValue);
            LOG_FIELD_VALUE("CPU Dest",fieldValue);
        }
        else
        {
            regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
            smemRegFldGet(devObjPtr, regAddr, 0, 4, &fieldValue);
            LOG_FIELD_VALUE("CPU_port",fieldValue);
        }
        /*  CPU port */
        trgPort = fieldValue;
        descrPtr->destPortVector = (1 << trgPort);
        if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
        {
            /* build the CPU code from 3 and 1 bits */
            mrvlTagCpuCode = (SMEM_U32_GET_FIELD(mrvlTag, 16, 3) << 1) |
                              SMEM_U32_GET_FIELD(mrvlTag, 12, 1);
            LOG_FIELD_VALUE("mrvlTagCpuCode",mrvlTagCpuCode);
            /* ARP */
            if (mrvlTagCpuCode == 0x4)
            {
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fieldValue);
                LOG_FIELD_VALUE("ARP monitor destination",fieldValue);
                /* ARP monitor destination */
                trgPort = fieldValue;
                descrPtr->destPortVector = (1 << trgPort);
            }
        }

        __LOG_PARAM(descrPtr->destPortVector);
        descrPtr->Mgmt = 1;
    }
    else
    if (descrPtr->marvellTagCmd == TAG_CMD_FROM_CPU_E)
    {
        __LOG(("Ingress DSA : 'FROM_CPU' format \n"));
        if (descrPtr->rmtMngmt)
        {
            __LOG(("remote management packet \n"));
            descrPtr->destPortVector = (1 <<  descrPtr->srcPort);
        }
        else
        {

            trgDev = SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
            LOG_FIELD_VALUE("trgDev",trgDev);
            if (trgDev == descrPtr->srcDevice)
            {
                /* Target port from Marvell Tag */
                trgPort = SMEM_U32_GET_FIELD(mrvlTag, 19, 5);
                LOG_FIELD_VALUE("trgPort",trgPort);
                descrPtr->destPortVector = (1 << trgPort);
            }
            else
            {
                if ( SKERNEL_DEVICE_FAMILY_SOHO_PLUS((devObjPtr)->deviceType))
                {/* For cascade FROM_CPU forwarding the OPAL+ does not look at this filed
                    and it behaves like OPAL when the field contain the value 0xf. */
                    fieldValue = 0xf;
                }
                else
                {
                    smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 12, 4, &fieldValue);
                    LOG_FIELD_VALUE("Cascade port",fieldValue);
                }
                 /* Cascade port */
                if (fieldValue == 0xf)
                {
                    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
                    {
                        trgPort = memInfoPtr->trgDevMem.deviceTblMem[trgDev] & 0xf;
                        __LOG_PARAM_WITH_NAME("trgPort from deviceTblMem[trgDev]",trgPort);
                        if (trgPort == 0xf)
                        {
                            descrPtr->destPortVector = 0;
                        }
                        else
                        {
                            descrPtr->destPortVector = (1 << trgPort);
                        }
                    }
                }
                else
                {
                    trgPort = fieldValue;
                    if (trgPort >= 0xb && trgPort <= 0xe)
                    {
                        descrPtr->destPortVector = 0;
                    }
                    else
                    {
                        descrPtr->destPortVector = (1 << trgPort);
                    }
                }
            }
        }
        __LOG_PARAM(descrPtr->destPortVector);
        descrPtr->Mgmt = 1;
    }
    else
    if (descrPtr->marvellTagCmd == TAG_CMD_TO_TARGET_SNIFFER_E)
    {
        __LOG(("Ingress DSA : 'TO_TARGET_SINFFER' format \n"));
        flowCtrl = GT_FALSE;

        if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
        {
            /* Flow control from marvell tag */
            flowCtrl = SMEM_U32_GET_FIELD(mrvlTag, 17, 1);
            LOG_FIELD_VALUE("(CCFC)Flow control from marvell tag",flowCtrl);
            if (flowCtrl)
            {
                srcDevice = SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
                LOG_FIELD_VALUE("srcDevice",srcDevice);
                if (srcDevice == descrPtr->srcDevice)
                {
                    /* Egress port's speed */
                    descrPtr->fcSpd = (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 10, 2);
                    LOG_FIELD_VALUE("Egress port's speed",descrPtr->fcSpd);
                    descrPtr->fcIn = 1;
                }
                else
                {
                    trgDev = SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
                    LOG_FIELD_VALUE("trgDev",trgDev);
                    trgPort = memInfoPtr->trgDevMem.deviceTblMem[trgDev] & 0xf;
                    if (trgPort == 0xf)
                    {
                        descrPtr->destPortVector = 0;
                    }
                    else
                    {
                        descrPtr->destPortVector = (1 << trgPort);
                    }
                }
            }
        }
        /* Process Rx/Tx sniffer frame */
        if (flowCtrl == GT_FALSE)
        {
            if (descrPtr->rxSnif)
            {
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 12, 4, &fieldValue);
                LOG_FIELD_VALUE("Ingress monitor destination",fieldValue);
                /* Ingress monitor destination */
                trgPort = fieldValue;
                descrPtr->Mgmt = 1;
            }
            else
            {
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 8, 4, &fieldValue);
                LOG_FIELD_VALUE("Egress monitor destination",fieldValue);
                /* Egress monitor destination */
                trgPort = fieldValue;
                if (descrPtr->daHit)
                {
                    descrPtr->Mgmt = 1;
                }
            }

            if (trgPort >= 0xb && trgPort <= 0xf)
            {
                descrPtr->destPortVector = 0;
            }
            else
            {
                descrPtr->destPortVector = (1 << trgPort);
            }
        }

        descrPtr->srcDevice =
            (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("srcDevice",descrPtr->srcDevice);
        descrPtr->origSrcPortOrTrnk =
            (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 19, 5);
        LOG_FIELD_VALUE("origSrcPortOrTrnk",descrPtr->origSrcPortOrTrnk);
    }
    else /* forward DSA tag */
    {
        __LOG(("Ingress DSA : 'FORWARD' format \n"));
        descrPtr->srcDevice =
            (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 24, 5);
        LOG_FIELD_VALUE("srcDevice",descrPtr->srcDevice);
        descrPtr->origSrcPortOrTrnk =
            (GT_U8)SMEM_U32_GET_FIELD(mrvlTag, 19, 5);
        LOG_FIELD_VALUE("origSrcPortOrTrnk",descrPtr->origSrcPortOrTrnk);

        retVal = GT_FALSE;
    }

    return retVal;
}

/**
* @internal snetSohoVtuAtuLookUp function
* @endinternal
*
* @brief   DA and SA lookups with the determined DBNum
*/
static GT_VOID snetSohoVtuAtuLookUp
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoVtuAtuLookUp);

    if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        __LOG(("collect values for PVT \n"));
        snetOpalPvtCollect(devObjPtr, descrPtr);
    }
    /* Start VTU lookup after VID is determine */
    __LOG(("Start VTU lookup after VID is determine \n"));
    snetSohoVlanAssign(devObjPtr, descrPtr);
    /* DA Translation Process */
    __LOG(("Start DA lookup : DA Translation Process \n"));
    snetSohoDaTranslation(devObjPtr, descrPtr);
    /* SA Learning Process */
    __LOG(("Start SA lookup : SA Learning Process \n"));
    snetSohoSaLearning(devObjPtr, descrPtr);
}


/**
* @internal snetOpalPvtCollect function
* @endinternal
*
* @brief   collect values for PVT
*/
static GT_VOID snetOpalPvtCollect
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    SIM_TBD_BOOKMARK
    /* the code currently not using: descrPtr->pvt ... so remove it */
#if 0
    DECLARE_FUNC_NAME(snetOpalPvtCollect);

    GT_U32 fld5BitPort;                 /* register's field first bit */
    SOHO_DEV_MEM_INFO * memInfoPtr;         /* device's memory pointer */
    SMEM_REGISTER * pvtTblMemPtr;       /* PVT table memory pointer */
    GT_32 index,offset;


    if(descrPtr->cascade)
    {
        /* Get pointer to the device memory */
        memInfoPtr = (SOHO_DEV_MEM_INFO *)(devObjPtr->deviceMemory);
        pvtTblMemPtr = memInfoPtr->pvtDbMem.pvtTblMem;

        smemRegFldGet(devObjPtr, GLB2_DEST_POLARITY_REG, 14, 1,&fld5BitPort);
        LOG_FIELD_VALUE("PVT memory mode",fld5BitPort);


       /* When this bit fld5BitPort is zero the 9 bits
          used to access the PVT memory is:
          Addr[8:4] = Source Device[4:0] or 0x1F
          Addr[3:0] = Source Port/Trunk[3:0]              */
       /* When this bit is one the 9 bits used to access the PVT memory is:
          Addr[8:5] = Source Device[3:0] or 0xF
          Addr[4:0] = Source Port/Trunk[4:0]             */

        if(fld5BitPort == 0)
        {
            offset = 4;

        }
        else
        {
            offset = 5;
        }



        if (descrPtr->srcTrunk == 1)
        {

            index = descrPtr->trunkId  | (descrPtr->srcTrunk << offset);

        }
        else
        {

            index = descrPtr->srcPort | (descrPtr->srcDevice << offset);

        }
        descrPtr->pvt = (GT_UINTPTR)&pvtTblMemPtr[index];
    }
#endif
}

/**
* @internal snetSohoIpPriorityAssign function
* @endinternal
*
* @brief   IP priority extraction
*/
static GT_VOID snetSohoIpPriorityAssign
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoIpPriorityAssign);

    GT_U32 regAddr, fldValue;           /* registers address and value */
    GT_U32 fldFirstBit;                 /* register's field first bit */
    GT_U16 etherType;                   /* frame's ethernet type */
    GT_U8 * dataPtr;                    /* pointer to frame's actual data */
    GT_U32 port;                        /* source port */
    GT_U8 byte;
    GT_BOOL vlanModeEn = GT_TRUE;       /* IEEE 802.1 mode enable */
    GT_U8   etherTypeOffset=0;
    GT_U32 portEtypeRegData;            /* Port Policy Register's Data */

    GT_U32      remoteMgmt = 0;         /* Remote Management bit  */
    GT_U32      dsa18_23;               /* Must be 0x3e Remote Management */
    GT_U32      dsa29;                  /* Must be 1 in Remote Management */
    GT_U32      dsa16;                  /* Must be 1 in Remote Management */
    GT_U32      dsa8_12;                /* Must be 0xf Remote Management */

    GT_BIT      f2Renable;              /* Frame to Register enable */
    GT_BIT      f2Rp10;                 /* port 10 bit              */
    GT_BIT      daCheck;                /* check the DA in RRU validate static */
    GT_U32      rmEnable,RMUMode;       /* RMU config fields */
    GT_U16  compareEtherType;

    dataPtr = descrPtr->frameBuf->actualDataPtr;
    port = descrPtr->srcPort;
    descrPtr->ipPriority.useIpvxPriority = 0;

    etherType = (dataPtr[12] << 8 | dataPtr[13]);

    if (descrPtr->cascade == 0)
    {
        if(descrPtr->isProviderPort)
        {
            compareEtherType = descrPtr->tagVal;
        }
        else
        {
            compareEtherType = 0x8100;
        }

        __LOG(("check if vlan tagged packet by etherType [0x%4.4x] to packet's bytes [0x%4.4x] \n",
            compareEtherType,etherType))

        if (etherType != compareEtherType)
        {
            regAddr = PORT_DFLT_VLAN_PRI_REG(devObjPtr,port);
            smemRegFldGet(devObjPtr, regAddr, 0, 12, &fldValue);
            LOG_FIELD_VALUE("Default VID (pvid)",fldValue);
            /* Default VID */
            descrPtr->vid = (GT_U16)fldValue;
            if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
            {
                if (SGT_MAC_ADDR_IS_IPMC(descrPtr->dstMacPtr))
                {/* if it's IP multicast frame */
                    etherTypeOffset = 12;
                    if(descrPtr->dstMacPtr[etherTypeOffset+11] == 2)
                    {/* if IP protocol is IGMP */
                        descrPtr->daType.igmp = 1;
                    }
                }
            }
        }
        else
        {
            if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType) == GT_FALSE)
            {
                descrPtr->srcVlanTagged = 1;
            }

            if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
            {
                regAddr = PORT_CTRL_2_REG(devObjPtr,port);
                smemRegFldGet(devObjPtr, regAddr, 10, 2, &fldValue);
                LOG_FIELD_VALUE("802.1Q mode",fldValue);
                /* 802.1Q mode */
                if (fldValue)
                {
                    descrPtr->srcVlanTagged = 1;
                    descrPtr->mcfi = (dataPtr[14] >> 4) & 0x1;
                    descrPtr->tagOut = 1;
                }
                else
                {
                    __LOG(("srcVlanTagged = %d, with port in 'untagged mode'\n", descrPtr->srcVlanTagged));
                    /* Use default Port VID for 802.1Q disabled */
                    regAddr = PORT_DFLT_VLAN_PRI_REG(devObjPtr,port);
                    smemRegFldGet(devObjPtr, regAddr, 0, 12, &fldValue);
                    LOG_FIELD_VALUE(" Default VID (pvid)",fldValue);

                    /* Default VID */
                    descrPtr->vid = (GT_U16)fldValue;
                    vlanModeEn = GT_FALSE;
                }
                if (SGT_MAC_ADDR_IS_IPMC(descrPtr->dstMacPtr))
                {/* if it's IP multicast frame */
                    etherTypeOffset = 16;
                    if(descrPtr->dstMacPtr[etherTypeOffset+11] == 2)
                    {/* if IP protocol is IGMP */
                        descrPtr->daType.igmp = 1;
                    }
                }
            }
        }
    }
    else
    {
        /* Cascade port frame */
        __LOG(("Cascade port frame"));
        descrPtr->srcVlanTagged = (dataPtr[12] >> 5) & 0x1;
        if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
        {
            descrPtr->mcfi = dataPtr[13] & 0x1;
            descrPtr->tagOut = 1;
        }
        if (descrPtr->marvellTagCmd == TAG_CMD_FORWARD_E)
        {
            descrPtr->yellow = descrPtr->mcfi;
        }

        rmEnable = 0;
        remoteMgmt = 0;
        if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
        {
            smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 8,3, &RMUMode);
            LOG_FIELD_VALUE("RMUMode",RMUMode);

            rmEnable = (RMUMode == 7) ? 0 : 1;
        }
        else
        if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 12,1, &rmEnable);
            LOG_FIELD_VALUE("rmEnable",rmEnable);
        }

        if(rmEnable)
        {
            remoteMgmt = dataPtr[13] & 0x02;
        }

        if (rmEnable && remoteMgmt == 2)
        {
            dsa18_23 = dataPtr[13] & 0xf8;
            dsa29 =   dataPtr[12] & 0x20;
            dsa16 = dataPtr[13] & 0x1;
            dsa8_12 = dataPtr[14] & 0x1f;

            smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 12,1, &f2Renable);
            LOG_FIELD_VALUE("f2Renable",f2Renable);
            smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 13,1, &f2Rp10);
            LOG_FIELD_VALUE("f2Rp10",f2Rp10);

            if ((dsa18_23 == 0xf8) && (dsa29 == 0) &&
                (dsa16 == 0)       && (dsa8_12 == 0xf) && (f2Renable) &&
                (((f2Rp10) && (port == 10)) ||((!(f2Rp10)) && (port == 9))))
            {
               descrPtr->rmtMngmt = 1;
            }
            else
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                __LOG_DROP("bad RMU format");
            }
        }
    }
    if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
    {
        regAddr = PORT_CTRL_3_REG(devObjPtr,port);
        smemRegFldGet(devObjPtr, regAddr, 0, 2, &fldValue);
        if (fldValue & 0x01)
        {
            /* Use CFI Yellow */
            descrPtr->yellow = descrPtr->mcfi;
        }
        if (fldValue & 0x02)
        {
            /* Use DEI Yellow */
            descrPtr->yellow = descrPtr->yellow || descrPtr->dtDei;
        }
    }
    else
    {
        descrPtr->yellow = descrPtr->mcfi;
    }
    LOG_FIELD_VALUE("Yellow",descrPtr->yellow);

    if (descrPtr->srcVlanTagged || descrPtr->cascade)
    {
        if (vlanModeEn == GT_TRUE)
        {
            descrPtr->vid = (dataPtr[14] & 0xf) << 8 | dataPtr[15];
            regAddr = PORT_DFLT_VLAN_PRI_REG(devObjPtr,port);

            if (descrPtr->vid == 0)
            {
                smemRegFldGet(devObjPtr, regAddr, 0, 12, &fldValue);
                LOG_FIELD_VALUE("Default VID (pvid)",fldValue);
                /* Default VID */
                descrPtr->vid = (GT_U16)fldValue;
                if (descrPtr->srcVlanTagged &&
                    SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
                {
                    descrPtr->priOnlyTag = GT_TRUE;
                }
                 if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)
                     && (descrPtr->vid == 0))
                     descrPtr->priOnlyTag = GT_TRUE;
            }
            else
            {
                smemRegFldGet(devObjPtr, regAddr, 12, 1, &fldValue);
                LOG_FIELD_VALUE("Force default VID",fldValue);
                /* Force default VID */
                if (fldValue)
                {
                    smemRegFldGet(devObjPtr, regAddr, 0, 12, &fldValue);
                    /* Default VID */
                    LOG_FIELD_VALUE("(forced)Default VID (pvid)",fldValue);
                    descrPtr->vid = (GT_U16)fldValue;
                }
            }
        }

        /* IEEE priority */
        descrPtr->fPri = (dataPtr[14] >> 5) & 0x7;
        if (descrPtr->useDt)
        {
            descrPtr->fPri = descrPtr->dtPri;
        }

        /* Map IEEE Priority Per Port to fpri */
        if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
        {
            /* Mapping involves accessing IEEE Priority map table,
             *  which is not currently initialized.
             *  So that identical 1:1 hard-coded mapping is used. */

            descrPtr->ipPriority.ieeePiority = descrPtr->fPri;
        }
        else
        {
            /* calculate base address */
            regAddr = PORT_IEEE_PRIO_REMAP_REG(devObjPtr,port);

            regAddr += ((descrPtr->fPri / 4) * 16);

            fldFirstBit = (descrPtr->fPri % 4) * 4;

            smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 3, &fldValue);

            descrPtr->fPri = (GT_U8)fldValue;

            /* Map IEEE priority to global priority */
            fldFirstBit = fldValue * 2;
            smemRegFldGet(devObjPtr, GLB_IEEE_PRI_REG, fldFirstBit, 2,
                          &fldValue);

            descrPtr->ipPriority.ieeePiority = (GT_U8)fldValue;
        }

        LOG_FIELD_VALUE("fPri",descrPtr->fPri);
        LOG_FIELD_VALUE("ieeePiority",descrPtr->ipPriority.ieeePiority);
    }

    if (descrPtr->useDt)
    {
        descrPtr->vid = descrPtr->dtVid;
        descrPtr->fPri = descrPtr->dtPri;
        if ((SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)) &&
             (descrPtr->vid == 0))
            descrPtr->priOnlyTag = GT_TRUE;
    }



    if  (descrPtr->rmtMngmt == 1)
    {/* check the DA in RMU  */
        smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 14,1, &daCheck);
        LOG_FIELD_VALUE("daCheck",daCheck);
        if  ((daCheck == 1) && (descrPtr->daStatic == 0))
        {
            descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
            __LOG_DROP("RMU DA not static");
        }
    }

    else
    {
        /* Start VTU/DA/SA Lookup */
        snetSohoVtuAtuLookUp(devObjPtr, descrPtr);
    }


    /* Assign IP Priority to Tagged packet or Cascade packet */
    if (descrPtr->srcVlanTagged || descrPtr->cascade)
    {
        etherType = (dataPtr[16] << 8 | dataPtr[17]);
        if (etherType == 0x0800)
        {
            /* Tagged IPV4 Classifying */
            byte = (dataPtr[18] >> 4) & 0xf;
            if (byte == 0x4)
            {
                descrPtr->ipPriority.useIpvxPriority = 1; /*IPV4_E;*/
                byte = (dataPtr[19] >> 2) & 0x3f;
                descrPtr->ipPriority.ipPriority = byte;
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                    descrPtr->iHl = dataPtr[14] & 0xf;
            }
        }
        else
        if (etherType == 0x86dd)
        {
            /* Tagged IPV6 Classifying */
            byte = (dataPtr[18] >> 4) & 0xf;
            if (byte == 0x6)
            {
                descrPtr->ipPriority.useIpvxPriority = 2; /*IPV6_E;*/
                byte = (dataPtr[19] >> 6) & 0x3;
                descrPtr->ipPriority.ipPriority = byte;
                byte = dataPtr[18] & 0xf;
                descrPtr->ipPriority.ipPriority |= byte << 2;
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                    descrPtr->iHl = dataPtr[14] & 0xf;
            }
        }
        else
        if (etherType == 0x0806)
        {
            if (SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr))
            {
                if (descrPtr->cascade == 0)
                {
                    descrPtr->arp = 1;
                }
            }
        }
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            if (etherType == 0x8863)
            {
                    if (descrPtr->cascade == 0)
                    {
                        descrPtr->pppOE = 1;
                    }
                }
            else if (etherType == 0x8200)
            {
                    if (descrPtr->cascade == 0)
                    {
                        descrPtr->vBas = 1;
                    }

            }

            portEtypeRegData = PORT_ETYPE(devObjPtr,port);
            smemRegFldGet(devObjPtr, portEtypeRegData, 0, 16, &fldValue);
            LOG_FIELD_VALUE("portEtype",fldValue);

            if  (etherType == fldValue)
            {

                    if (descrPtr->cascade == 0)
                    {
                        descrPtr->eType = 1;
                    }

            }
        }
    }
    else
    {
        if (etherType == 0x0800)
        {
            /* Untagged IPV4 Classifying */
            byte = (dataPtr[14] >> 4) & 0xf;
            if (byte == 0x4)
            {
                descrPtr->ipPriority.useIpvxPriority = 1; /*IPV4_E;*/
                byte = (dataPtr[15] >> 2) & 0x3f;
                descrPtr->ipPriority.ipPriority = byte;
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                    descrPtr->iHl = dataPtr[14] & 0xf;
            }
        }
        else
        if (etherType == 0x86dd)
        {
            /* Untagged IPV6 Classifying */
            byte = (dataPtr[14] >> 4) & 0xf;
            if (byte == 0x6)
            {
                descrPtr->ipPriority.useIpvxPriority = 2; /*IPV6_E;*/
                byte = (dataPtr[15] >> 6) & 0x3;
                descrPtr->ipPriority.ipPriority = byte;
                byte = dataPtr[14] & 0xf;
                descrPtr->ipPriority.ipPriority |= byte << 2;
            }
        }
        else
        if (etherType == 0x0806)
        {
            if (SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr))
            {
                descrPtr->arp = 1;
            }
        }
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            if (etherType == 0x8863)
            {
                if (SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr))
                {
                    if (descrPtr->cascade == 0)
                    {
                        descrPtr->pppOE = 1;
                    }
                }
            }
            else if (etherType == 0x8200)
            {
                if (SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr))
                {
                    if (descrPtr->cascade == 0)
                    {
                        descrPtr->vBas = 1;
                    }
                }
            }
            /*reading portEtype value register 0xf */

            portEtypeRegData = PORT_ETYPE(devObjPtr,port);
            smemRegFldGet(devObjPtr, portEtypeRegData, 0, 16, &fldValue);
            LOG_FIELD_VALUE("portEtype",fldValue);
            if  (etherType == fldValue)
            {
                    if (descrPtr->cascade == 0)
                    {
                        descrPtr->eType = 1;


                }
            }
        }
    }

    if (descrPtr->ipPriority.useIpvxPriority)
    {
        /* Remap the IP Priority according to IP_QPRI map */
        regAddr = GLB_IP_QPRI_MAP_REG +
            ((descrPtr->ipPriority.ipPriority / 8) * 16);

        fldFirstBit = ((descrPtr->ipPriority.ipPriority % 8) * 2);

        smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 2, &fldValue);
        LOG_FIELD_VALUE("ipPriority",fldValue);

        descrPtr->ipPriority.ipPriority = (GT_U8)fldValue;
    }
}

/**
* @internal snetSohoIgmpSnoop function
* @endinternal
*
* @brief   Control frame's processing
*/
static GT_VOID snetSohoIgmpSnoop
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoIgmpSnoop);

    GT_U8 * dataPtr;                    /* pointer to frame's actual data */
    GT_U32 port;                        /* source port */
    GT_U8 nextHeaderProt;               /* frame's protocol/next header */
    GT_U32 regAddr, fldValue;           /* registers address and value */
 /*   GT_U32 packetSize;                   packet size */
    GT_U32 portControl2Data;            /* Port Control 2 Register's Data */



    port = descrPtr->srcPort;

    regAddr = PORT_CTRL_2_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddr, &portControl2Data);


    dataPtr = descrPtr->frameBuf->actualDataPtr;
    port = descrPtr->srcPort;

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        /* Change the IGMP Frame check logic in Ruby2 */
        __LOG(("Change the IGMP Frame check logic in Ruby2"));
        snetSoho2IgmpSnoop(devObjPtr, descrPtr);
        return;
    }

    /* IPv4 IGMP snooping */
    if (descrPtr->ipPriority.useIpvxPriority == 1 &&
        descrPtr->daType.igmp)
    {
        if (descrPtr->srcVlanTagged)
        {
            nextHeaderProt = dataPtr[27] & 0xff;
        }
        else
        {
            nextHeaderProt = dataPtr[23] & 0xff;
        }
        if (nextHeaderProt == 0x02)
        {
            regAddr = PORT_CTRL_REG(devObjPtr,port);
            smemRegFldGet(devObjPtr, regAddr, 10, 1, &fldValue);
            LOG_FIELD_VALUE("IGMP snooping",fldValue);
            /* IGMP snooping */
            if (fldValue)
            {
                descrPtr->igmpSnoop = 1;
            }
        }
    }
    else
    /* IPv4 IGMP snooping */
    if (descrPtr->ipPriority.useIpvxPriority == 2 &&
         descrPtr->daType.mld)
    {
        if (descrPtr->srcVlanTagged)
        {
            nextHeaderProt = dataPtr[24] & 0xff;
        }
        else
        {
            nextHeaderProt = dataPtr[20] & 0xff;
        }
        if (nextHeaderProt == 0x01)
        {
            regAddr = PORT_CTRL_REG(devObjPtr,port);
            smemRegFldGet(devObjPtr, regAddr, 10, 1, &fldValue);
            LOG_FIELD_VALUE("IGMP snooping",fldValue);
            /* IGMP snooping */
            if (fldValue)
            {
                descrPtr->igmpSnoop = 1;
            }
        }
    }
}

/**
* @internal snetSoho2IgmpSnoop function
* @endinternal
*
* @brief   Ruby2 specific snooping processing
*/
static GT_VOID snetSoho2IgmpSnoop
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSoho2IgmpSnoop);

    GT_U8 * dataPtr;                    /* pointer to frame's actual data */
    GT_U32 port;                        /* source port */
    GT_U32 regAddr, fldValue;           /* registers address and value */
    GT_U8 nextHeaderProt, hdr2;         /* frame's protocol/next header */
    GT_U8 error;                        /* error flag */

    dataPtr = descrPtr->frameBuf->actualDataPtr;
    port = descrPtr->srcPort;

    /* IPv4 IGMP snooping */
    if (descrPtr->ipPriority.useIpvxPriority == 1)
    {
        if (descrPtr->srcVlanTagged)
        {
            nextHeaderProt = dataPtr[27] & 0xff;
        }
        else
        {
            nextHeaderProt = dataPtr[23] & 0xff;
        }
        if (nextHeaderProt == 0x02)
        {
            regAddr = PORT_CTRL_REG(devObjPtr,port);
            smemRegFldGet(devObjPtr, regAddr, 10, 1, &fldValue);
            LOG_FIELD_VALUE("IGMP snooping",fldValue);
            /* IGMP snooping */
            if (fldValue)
            {
                descrPtr->igmpSnoop = 1;
            }
        }
    }
    else
    /* IPv6 MLD snooping */
    if (descrPtr->ipPriority.useIpvxPriority == 2)
    {
        if (descrPtr->srcVlanTagged)
        {
            nextHeaderProt = dataPtr[24];
            hdr2 = dataPtr[64];
            error = (dataPtr[65] >> 0x7);
        }
        else
        {
            nextHeaderProt = dataPtr[20];
            hdr2 = dataPtr[60];
            error = (dataPtr[61] >> 0x7);
        }
        if (nextHeaderProt == 0x0 &&
            hdr2 == 0x3a &&
            error == 1)
        {
            regAddr = PORT_CTRL_REG(devObjPtr,port);
            smemRegFldGet(devObjPtr, regAddr, 10, 1, &fldValue);
            LOG_FIELD_VALUE("IGMP snooping",fldValue);
            /* IGMP snooping */
            if (fldValue)
            {
                descrPtr->igmpSnoop = 1;
            }
        }
    }
}

/**
* @internal snetSohoPortMapUpdate function
* @endinternal
*
* @brief   Frame Mapping Policy
*/
static GT_VOID snetSohoPortMapUpdate
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoPortMapUpdate);

    GT_U32 regAddr, fldValue;           /* registers address and value */
    GT_U8  enforcePortBaseVlan = 0;     /* enforce port based VLAN */
    GT_U32 portVector = 0;              /* destination ports vector */
    GT_U32 ports = 0;                   /* device ports number */
    GT_U16 i,ii,jj;
    GT_U32 portBaseVlanBmp;
    GT_U32 port;
    GT_U32 portCtrlRegData;             /* Port Control Register's Data */
    GT_U32 portCtrl1RegData;            /* Port Control Register's Data */
    GT_U32 portCtrl2RegData;            /* Port Control Register's Data */
    GT_U8  mgmt = 0;                    /* Management frame */
    GT_U32  myDevNum;                   /* my device number */
    GT_BOOL enableLoopBackFilter;       /* Prevent loopback filter */
    GT_U32 lastData;
    GT_U8 *dataPtr;
    GT_U32  LimitModeFlooding;

    ports = devObjPtr->portsNumber;
    port = descrPtr->srcPort;

    regAddr = PORT_CTRL_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddr, &portCtrlRegData);

    regAddr = PORT_CTRL1_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddr, &portCtrl1RegData);

    regAddr = PORT_CTRL_2_REG(devObjPtr,port);
    smemRegGet(devObjPtr, regAddr, &portCtrl2RegData);

    smemRegFldGet(devObjPtr, GLB_CTRL_2_REG, 0, 5, &myDevNum);
    LOG_FIELD_VALUE("(ownDevNum)Source Device",myDevNum);
    /* Source Device */
    descrPtr->srcDevice = (GT_U8)myDevNum;

    if (descrPtr->cascade)
    {
        if(devObjPtr->deviceType == SKERNEL_SAPPHIRE)
        {
            if(snetSohoCascadeFrame(devObjPtr, descrPtr))
            {/* if it's marvel tag frame skip port vector and priority definition */
                __LOG(("if it's marvel tag frame skip port vector and priority definition"));
                return;
            }
        }
        else
        {
            if(snetSohoCascadeRubyFrame(devObjPtr, descrPtr))
            {


                /* layer 3 identification the kind of remote command */
                if ((descrPtr->rmtMngmt) &&
                    (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
                {
                    /* get the ethertype */
                    descrPtr->remethrtype = RMT_TPYE(descrPtr->frameBuf->actualDataPtr);

                    /* get the request format */
                    descrPtr->reqFormat = REQ_FMT(descrPtr->frameBuf->actualDataPtr);

                    /* get the request code */
                    descrPtr->reqCode = REQ_CDE(descrPtr->frameBuf->actualDataPtr);


                    dataPtr = descrPtr->frameBuf->actualDataPtr;

                    switch (descrPtr->reqCode)
                    {
                        case SNET_OPAL_DUMP_MIB:
                        case SNET_OPAL_DUMP_ATU_STATE:
                        case SNET_OPAL_GETID:
                           for (ii=0; ii<8;ii++ )
                           { /* 2*(request format ,pad ,request code ,data) */

                               descrPtr->reqData[ii] =
                               dataPtr[18 + ii];
                           }
                       break;

                       case SNET_OPAL_READ_WRITE_STATE:
                           for (ii=0; ii<6;ii++ )
                           { /* 2*(request format ,request code  */
                               descrPtr->reqData[ii] =
                               dataPtr[18 + ii];
                           }


                           for (ii=0; ii<121;ii++ )
                           {
                               for (jj=0; jj<4;jj++)
                               {
                                  descrPtr->reqData[6 +4*ii +jj] =
                                  dataPtr[24 + 4*ii +jj];
                               }
                               lastData =  REQ_END_LIST(descrPtr->frameBuf->actualDataPtr,ii);
                               if (lastData == 0xffffffff)
                               { /* last data */
                                   break;
                               }
                           }

                      break;
                    }

                }

                /* if it's marvel tag frame skip port vector and priority definition */
                return;
            }
        }
        /* if we are here : we got forward DSA tag */
    }

    if (descrPtr->daHit)
    {
        LimitModeFlooding = 0;
        if (descrPtr->Mgmt)
        {
            portVector = descrPtr->destPortVector;
            mgmt = 1;
        }
        else
        {
            fldValue = SMEM_U32_GET_FIELD(portCtrl2RegData, 7, 1);
            LOG_FIELD_VALUE("MapDA : Map Using DA Hits ",fldValue);
            /* Map Using DA Hits */
            if (fldValue)
            {
                portVector = descrPtr->destPortVector;
            }
            else
            {
                __LOG(("NOTE: mac DA found but ignored (treated as flooding) \n"));
                LimitModeFlooding = 1;
            }
        }
    }
    else
    {
        LimitModeFlooding = 1;
    }

    /* Limit Mode Flooding */
    if(LimitModeFlooding)
    {
        GT_BIT isBC,isMc;
        isBC = SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr);
        isMc = SGT_MAC_ADDR_IS_MCST(descrPtr->dstMacPtr);

        if(isBC && SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {
            smemRegFldGet(devObjPtr, GLB2_MNG_REG, 12,1,&fldValue);
            LOG_FIELD_VALUE("Flood BC",fldValue);
            if(fldValue == 0)
            {
                /* consider BC like MC */
                isBC = 0;
            }
        }

        if (devObjPtr->deviceType == SKERNEL_SAPPHIRE)
        {
            /* not have filter option */
            isBC = 1;
        }

        if(isBC)
        {
            /* Flood BC */
            portVector = 0x3ff;
        }
        else
        {
            for (i = 0; i < ports; i++)
            {
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                {
                    regAddr = PORT_CTRL_REG(devObjPtr,i);
                    smemRegFldGet(devObjPtr, regAddr, 2,2,&fldValue);
                    __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                        i ,
                        "Egress Floods" ,
                        fldValue));

                    if(fldValue == 0)
                    {
                        /*Do not egress any frame with an unknown DA (unicast or multicast)*/
                        fldValue = 0;
                    }
                    else
                    if(fldValue == 1)
                    {
                        /*Do not egress any frame with an unknown multicast DA*/
                        if(isMc)
                        {
                            fldValue = 0;
                        }
                    }
                    else
                    if(fldValue == 2)
                    {
                        /*Do not egress any frame with an unknown unicast DA*/
                        if(!isMc)
                        {
                            fldValue = 0;
                        }
                    }
                    else/*fldValue == 3*/
                    {
                        /*Egress all frames with an unknown DA (unicast and multicast)*/
                    }
                }
                else
                {
                    if(isMc)
                    {
                        regAddr = PORT_CTRL_2_REG(devObjPtr,i);
                        smemRegFldGet(devObjPtr, regAddr, 6,1,&fldValue);
                        __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                            i ,
                            "Default forward only MULTICAST" ,
                            fldValue));
                    }
                    else
                    {
                        regAddr = PORT_CTRL_REG(devObjPtr,i);
                        smemRegFldGet(devObjPtr, regAddr, 2,1,&fldValue);
                        __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                            i ,
                            "Forward unknown" ,
                            fldValue));
                    }
                }

                /* Forward unknown */
                if (fldValue)
                {
                    portVector |=  1 << i;
                }

            }
        }
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        /* Is DA a MGMT DA? */
        if (descrPtr->Mgmt == 1)
        {
            /* Switch Management Register */
            smemRegFldGet(devObjPtr, GLB2_MNG_REG, 0, 3, &fldValue);
            LOG_FIELD_VALUE("MGMT Pri",fldValue);
            /* MGMT Pri */
            descrPtr->fPri = descrPtr->qPri = (GT_U8)fldValue;

            if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {
                /*Monitor Control register Offset: 0x1A or decimal 26
                 the field CPU Dest - bits 4:7*/
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fldValue);
                LOG_FIELD_VALUE("CPU Dest",fldValue);
            }
            else
            {
                /* Port control 2 register */
                fldValue = SMEM_U32_GET_FIELD(portCtrl2RegData, 0, 4);
                LOG_FIELD_VALUE("CPUport",fldValue);
            }

            /* CPUport */
            descrPtr->destPortVector = 1 << fldValue;
            return;
        }
    }

    /* VLAN table */
    regAddr = PORT_BASED_VLAN_MAP_REG(devObjPtr,port);
    smemRegFldGet(devObjPtr, regAddr, 0, ports, &portBaseVlanBmp);
    LOG_FIELD_VALUE("VLANTable: port based vlan map",portBaseVlanBmp);

    if (mgmt == 0)
    {
        if (descrPtr->igmpSnoop)
        {
            if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {
                /*Monitor Control register Offset: 0x1A or decimal 26
                 the field CPU Dest - bits 4:7*/
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fldValue);
                LOG_FIELD_VALUE("CPU Dest",fldValue);
            }
            else
            {
                /* Port control 2 register */
                fldValue = SMEM_U32_GET_FIELD(portCtrl2RegData, 0, 4);
                LOG_FIELD_VALUE("CPU port",fldValue);
            }

            /* CPU port */
            portVector = 1 << fldValue;
        }

        if (descrPtr->saUpdate &&
            devObjPtr->deviceType != SKERNEL_SAPPHIRE)
        {
            for (i = 0; i < ports; i++)
            {
                if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
                {
                    /*  Message Port */
                    fldValue = SMEM_U32_GET_FIELD(portCtrl1RegData, 15, 1);
                    __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                        i ,
                        "Message Port" ,
                        fldValue));
                }
                else
                {
                    /* Marvell Tag */
                    fldValue = SMEM_U32_GET_FIELD(portCtrlRegData, 8, 1);
                    __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                        i ,
                        "Marvell Tag" ,
                        fldValue));
                }
                /* Message Port/Marvell Tag */
                if (fldValue)
                {
                    portVector |= (1 << i);
                }
            }
        }

        if (descrPtr->vtuHit &&
            !SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
        {
            portVector &= descrPtr->vtuVector;
            fldValue = SMEM_U32_GET_FIELD(portCtrlRegData, 3, 1);
            LOG_FIELD_VALUE("Protected port",fldValue);
            /* Protected port */
            if (fldValue)
            {
                enforcePortBaseVlan = 1;
            }
        }
        else
        {
            enforcePortBaseVlan = 1;

            fldValue = SMEM_U32_GET_FIELD(portCtrlRegData, 7, 1);
            LOG_FIELD_VALUE("VLAN tunnel",fldValue);
            /* VLAN tunnel */
            if (fldValue)
            {
                if (descrPtr->daStatic == 1)
                {
                    enforcePortBaseVlan = 0;
                }
            }

            if ((SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType)))
            {/* 802.1Q MODE */
                fldValue = SMEM_U32_GET_FIELD(portCtrl2RegData, 10, 2);
                LOG_FIELD_VALUE("802.1Q MODE",fldValue);
                if (fldValue == 0)
                {  /* USE VTU */
                    fldValue = SMEM_U32_GET_FIELD(GLB2_MNG_REG, 8, 1);
                    LOG_FIELD_VALUE("USE VTU",fldValue);
                    if(fldValue)
                    {
                        descrPtr->policyVid = 0;
                    }
                }
                portVector &= descrPtr->vtuVector;


            }
        }

        if (enforcePortBaseVlan)
        {
            portVector &= portBaseVlanBmp;
            if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
            {
                /* VTU vector */
                portVector &= descrPtr->vtuVector;
                /* Load Balancing is done -- by trunk designated ports */
                portVector &= descrPtr->tpv;
            }

            if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {/* IGMP/MLD Snooping */
                fldValue = SMEM_U32_GET_FIELD(GLB_CTRL_REG, 10, 1);
                LOG_FIELD_VALUE("IGMP/MLD Snooping",fldValue);
                if ((fldValue)  && (descrPtr->igmpSnoop))
                { /* DPV vectored CPUDest */
                     fldValue = SMEM_U32_GET_FIELD(GLB_MON_CTRL_REG, 4, 4);
                     LOG_FIELD_VALUE("CPUDest",fldValue);
                     portVector &= ~(1 << fldValue);

                }
            }
        }
    }

    if (((portBaseVlanBmp >> port) & 0x1) == 0)
    {
        portVector &= ~(1 << port);
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType) &&
        descrPtr->srcTrunk)
    {
        /* don't allow traffic to return to src trunk */
        for (i = 0; i < ports; i++)
        {
            regAddr = PORT_CTRL1_REG(devObjPtr,i);
            smemRegFldGet(devObjPtr, regAddr, 14,1,&fldValue);

            __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                i ,
                "Trunk port" ,
                fldValue));
            /* Trunk port */
            if (fldValue)
            {
                if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                {
                    smemRegFldGet(devObjPtr, regAddr,  8, 4, &fldValue);
                }
                else
                {
                    smemRegFldGet(devObjPtr, regAddr,  4, 4, &fldValue);
                }
                LOG_FIELD_VALUE("trunk Id",fldValue);
                if (descrPtr->trunkId == fldValue)
                {
                    portVector &= ~(1 << i);
                }
            }
        }
    }

    if(SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        /* Loopback Filter : Global 2 offset 0x05 bit 15 */
        smemRegFldGet(devObjPtr, GLB2_MNG_REG, 15, 1, &fldValue);
        LOG_FIELD_VALUE("Loopback Filter",fldValue);
        enableLoopBackFilter =  fldValue ? GT_TRUE : GT_FALSE;
    }
    else
    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType) ||
        devObjPtr->deviceType == SKERNEL_RUBY)
    {
        enableLoopBackFilter = GT_TRUE;
    }
    else
    {
        enableLoopBackFilter = GT_FALSE;
    }

    if (enableLoopBackFilter && descrPtr->cascade)
    {
        if (descrPtr->srcDevice == myDevNum)
        {
            smemRegFldGet(devObjPtr, GLB2_MNG_REG, 14, 1, &fldValue);
            LOG_FIELD_VALUE("prevent loop",fldValue);
            /* Prevent loop */
            if (fldValue)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                __LOG_DROP("Prevent loop (ownDevNum)");
                return;
            }

            portVector &= ~(1 << descrPtr->origSrcPortOrTrnk);
        }
    }
    descrPtr->destPortVector = portVector;
}

/**
* @internal snetSohoPriorityAssign function
* @endinternal
*
* @brief   Priority Selection
*/
static GT_VOID snetSohoPriorityAssign
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoPriorityAssign);

    GT_U32 regAddr, fldValue;           /* registers address and value */
    GT_U32 port;                        /* source port */
    GT_U32 fldFirstBit;                 /* register's field first bit */
    GT_BOOL useDefaultUserPrio;         /* use port default user priority */
    GT_BIT  useTagPrio;
    GT_BIT  useIpPrio;
    GT_BIT  TagIfBoth;

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        /* If DA a MGMT, priority has been assigned */
        if (descrPtr->Mgmt)
        {
            __LOG(("If DA a MGMT, priority has been assigned"));
            return;
        }
    }

    useDefaultUserPrio = GT_FALSE;

    port = descrPtr->srcPort;
    regAddr = PORT_CTRL_REG(devObjPtr,port);

    if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        smemRegFldGet(devObjPtr, regAddr, 4, 2, &fldValue);
        LOG_FIELD_VALUE("InitialPri",fldValue);
        useTagPrio = fldValue & 1 ? 1 : 0;
        useIpPrio  = fldValue & 2 ? 1 : 0;
        LOG_FIELD_VALUE("Use Tag priority",useTagPrio);
        LOG_FIELD_VALUE("Use Ip Priority",useIpPrio);
    }
    else
    {
        smemRegFldGet(devObjPtr, regAddr, 4, 1, &fldValue);
        LOG_FIELD_VALUE("Use Tag priority",fldValue);
        useTagPrio = fldValue;
        smemRegFldGet(devObjPtr, regAddr, 5, 1, &fldValue);
        LOG_FIELD_VALUE("Use Ip Priority",fldValue);
        useIpPrio = fldValue;
    }
    smemRegFldGet(devObjPtr, regAddr, 6, 1, &fldValue);
    LOG_FIELD_VALUE("Use Tag if both (IEEE_QPRI/IP_QPRI)",fldValue);
    TagIfBoth = fldValue;

    if(useIpPrio && descrPtr->ipPriority.useIpvxPriority &&
       useTagPrio && descrPtr->srcVlanTagged)
    {
        if(TagIfBoth)
        {
            __LOG(("tag priority and ip priority : use TAG priority \n"));
            useIpPrio = 0;
        }
        else
        {
            __LOG(("tag priority and ip priority : use IP priority \n"));
            useTagPrio = 0;
        }
    }

    if(useIpPrio && descrPtr->ipPriority.useIpvxPriority)
    {
        /* IP_QPRI */
        descrPtr->qPri = descrPtr->ipPriority.ipPriority;
        __LOG(("use IP priority [%d] \n" ,
            descrPtr->qPri));
    }
    else
    if(useTagPrio && (descrPtr->srcVlanTagged || descrPtr->useDt))
    {
        /* IEEE_QPRI */
        descrPtr->qPri = descrPtr->ipPriority.ieeePiority;
        __LOG(("use TAG priority [%d] \n" ,
            descrPtr->qPri));
    }
    else
    {
        useDefaultUserPrio = GT_TRUE;
    }

    if (useDefaultUserPrio == GT_TRUE)
    {
        /* getting here means that packet isn't Tagged
           and/or Use Tag Priority bit is disabled */

        regAddr = PORT_DFLT_VLAN_PRI_REG(devObjPtr,port);
        /* DefPri */
        smemRegFldGet(devObjPtr, regAddr, 13, 3, &fldValue);
        LOG_FIELD_VALUE("DefPri",fldValue);


        if  (descrPtr->cascade == 0)
        {

            descrPtr->fPri = (GT_U8)fldValue;
        }
        /* Remap the default to global priority */
        __LOG(("Remap the default to global priority"));
        fldFirstBit = (fldValue * 2);
        smemRegFldGet(devObjPtr, GLB_IEEE_PRI_REG, fldFirstBit, 2,
                      &fldValue);
        LOG_FIELD_VALUE("ieeePiority",fldValue);

        descrPtr->ipPriority.ieeePiority = (GT_U8)fldValue;

        /* queue default */
        descrPtr->qPri = descrPtr->ipPriority.ieeePiority;

        /* get Use_IP_Priority bit */
        regAddr = PORT_CTRL_REG(devObjPtr,port);
        smemRegFldGet(devObjPtr, regAddr, 5, 1, &fldValue);
        LOG_FIELD_VALUE("Use IP Priority",fldValue);

        if (fldValue)
        {
            /* Use IP Priority */
            __LOG(("Use IP Priority"));
            if (descrPtr->ipPriority.useIpvxPriority != 0)
            {
                /* packet is IPv4 or IPv6 */

                /* IP_QPRI */
                descrPtr->qPri = descrPtr->ipPriority.ipPriority;

                /*descrPtr->fPri |= (descrPtr->qPri & 0x3) << 1;*/
                descrPtr->fPri = (descrPtr->fPri & 0x1) | (descrPtr->qPri << 1);

                useDefaultUserPrio = GT_FALSE;
            }
        }
    }

    /* Get Port Control Register2 Address */
    regAddr = PORT_CTRL_2_REG(devObjPtr,port);

    if (descrPtr->vtuHit)
    {
        /* VTU Hit */
        smemRegFldGet(devObjPtr, regAddr, 14, 1, &fldValue);
        LOG_FIELD_VALUE("VTU Priority Override",fldValue);

        if (fldValue)
        {
            /* VTU Priority Override */
            if (descrPtr->priorityInfo.vtu_pri.useVtuPriority)
            {
                /* Use VTU's PRI */
                __LOG(("Use VTU's PRI"));
                descrPtr->fPri = descrPtr->priorityInfo.vtu_pri.vtuPriority & 0x7;
                descrPtr->qPri = (descrPtr->fPri >> 1) & 0x3;
            }
        }
    }

    if (descrPtr->saHit)
    {
        /* SA Hit */
        smemRegFldGet(devObjPtr, regAddr, 13, 1, &fldValue);
        LOG_FIELD_VALUE("SA Priority Override",fldValue);

        if (fldValue)
        {
            /* SA Priority Override */
            if (descrPtr->priorityInfo.sa_pri.useSaPriority)
            {
                /* Use SA's PRI */
                __LOG(("Use SA's PRI"));
                descrPtr->fPri = descrPtr->priorityInfo.sa_pri.saPriority & 0x7;
                descrPtr->qPri = (descrPtr->fPri >> 1) & 0x3;
            }
        }
    }

    if (descrPtr->daHit)
    {
        /* DA Hit */
        smemRegFldGet(devObjPtr, regAddr, 12, 1, &fldValue);
        LOG_FIELD_VALUE("DA Priority Override",fldValue);

        if (fldValue)
        {
            /* DA Priority Override */
            if (descrPtr->priorityInfo.da_pri.useDaPriority)
            {
                /* Use DA's PRI */
                __LOG(("Use DA's PRI"));
                descrPtr->fPri = descrPtr->priorityInfo.da_pri.daPriority & 0x7;
                descrPtr->qPri = (descrPtr->fPri >> 1) & 0x3;
            }
        }
    }
}

/**
* @internal snetSohoL2Decision function
* @endinternal
*
* @brief   Check frame for VLAN violation, port authentication and size errors
*         and apply decision on the frame
*         INPUTS:
*         devObjPtr  - pointer to device object.
*         descrPtr  - pointer to the frame's descriptor.
*         Check frame for VLAN violation, port authentication and size errors
*         and apply decision on the frame
*/
static GT_VOID snetSohoL2Decision
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoL2Decision);

    GT_U32 regAddr, fldValue;           /* registers address and value */
    GT_U32 packetSize;                  /* actual packet's data size */
    GT_U32 portState,vtuPortState;
    SNET_SOHO_VTU_STC vtuEntry;
    GT_U8   portMap=0;
    GT_U32  port;                       /* source Port */
    GT_U32 readLearnCnt;                /* limit cnt /learn option */
    GT_U32 learnCnt;                    /* counter */
    GT_U32  entryState_uni;               /* ATU  static policy unicast  enrty*/
    GT_U32  entryState_multi;             /* ATU  static policy multicast  enrty*/
    GT_U32 globLearnCnt; /* learn counter value */
    GT_BIT  dpvMember;                      /*port member */
    GT_U32  MaxFrameSize;


    port = descrPtr->srcPort;

    if (devObjPtr->deviceType == SKERNEL_RUBY)
    {
        snetSohoL2RubyDecision(devObjPtr, descrPtr);
        return;
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
       /* DA policy */
        entryState_uni = SMEM_U32_GET_FIELD(descrPtr->atuEntry[1], 17, 3);
        LOG_FIELD_VALUE("entryState_uni",entryState_uni);
        entryState_multi = SMEM_U32_GET_FIELD(descrPtr->atuEntry[1], 16, 3);
        LOG_FIELD_VALUE("entryState_multi",entryState_multi);

        if ((entryState_uni == 4) || (entryState_multi == 4))
        {

            /* Get Port Rate Override Address */
            __LOG(("Get Port Rate Override Address"));
            regAddr = PORT_PRIORITY_OVERRIDE(devObjPtr,port);

            if (descrPtr->daHit == 1)
            {
                smemRegFldGet(devObjPtr, regAddr, 14, 2, &fldValue);
                LOG_FIELD_VALUE("DA pktCmd",fldValue);

                if (fldValue == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
                }
                else if (fldValue == 2)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else if (fldValue == 3)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                    __LOG_DROP("DA pktCmd = drop");
                }
                else
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;

            }

            /* SA policy*/

            else if (descrPtr->saHit == 1)
            {
                smemRegFldGet(devObjPtr, regAddr, 12, 2, &fldValue);
                LOG_FIELD_VALUE("SA pktCmd",fldValue);

                if (fldValue == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
                }
                else if (fldValue == 2)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else if (fldValue == 3)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                    __LOG_DROP("SA pktCmd = drop");
                }
                else
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;

            }


            /* VTU policy*/

            else if ((smemSohoVtuEntryGet(devObjPtr, descrPtr->vid, &vtuEntry) == GT_OK) &&
                 (vtuEntry.vidPolicy))
            {
                /* Get Port Policy control Address */
                regAddr = PORT_POLICY_CONTROL(devObjPtr,port);
                smemRegFldGet(devObjPtr, regAddr, 10, 2, &fldValue);
                LOG_FIELD_VALUE("VTU pktCmd",fldValue);

                if (fldValue == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
                }
                else if (fldValue == 2)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else if (fldValue == 3)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                    __LOG_DROP("VTU pktCmd = drop");
                }
                else
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;

            }
            /* Etype policy*/
            else if (descrPtr->eType)
            {
                /* Get Port Policy control Address */
                regAddr = PORT_POLICY_CONTROL(devObjPtr,port);
                smemRegFldGet(devObjPtr, regAddr, 8, 2, &fldValue);
                LOG_FIELD_VALUE("eType pktCmd",fldValue);

                if (fldValue == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
                }
                else if (fldValue == 2)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else if (fldValue == 3)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                    __LOG_DROP("eType pktCmd = drop");
                }
                else
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;

            }
            /* PPPoE policy*/
            else if (descrPtr->pppOE)
            {
                /* Get Port Policy control Address */
                regAddr = PORT_POLICY_CONTROL(devObjPtr,port);
                smemRegFldGet(devObjPtr, regAddr, 6, 2, &fldValue);
                LOG_FIELD_VALUE("pppOE pktCmd",fldValue);

                if (fldValue == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
                }
                else if (fldValue == 2)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else if (fldValue == 3)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                    __LOG_DROP("pppOE pktCmd = drop");
                }
                else
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;

            }

                /* vBas policy*/
           else if (descrPtr->vBas)
           {
                /* Get Port Policy control Address */
                regAddr = PORT_POLICY_CONTROL(devObjPtr,port);
                smemRegFldGet(devObjPtr, regAddr, 4, 2, &fldValue);
                LOG_FIELD_VALUE("vBas pktCmd",fldValue);

                if (fldValue == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
                }
                else if (fldValue == 2)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else if (fldValue == 3)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                    __LOG_DROP("vBas pktCmd = drop");
                }
                else
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;

           }
        }
        switch(descrPtr->pktCmd)
        {
            case SKERNEL_PKT_TRAP_CPU_E:
                 smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fldValue);
                 LOG_FIELD_VALUE("CPUport",fldValue);
                 /* CPUport */
                 descrPtr->destPortVector = 1 << fldValue;
                 break;

            case SKERNEL_PKT_MIRROR_CPU_E:
                 smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fldValue);
                 LOG_FIELD_VALUE("CPUport",fldValue);
                 /* CPUport */
                 descrPtr->destPortVector |= 1 << fldValue;
                 break;

            case SKERNEL_PKT_DROP_E:
                 descrPtr->destPortVector = 0;
                 break;

            default:
                 break;
        }

    }
    if (descrPtr->Mgmt == 0)
    {
        regAddr = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {

            smemRegFldGet(devObjPtr, regAddr, 14, 2, &fldValue);
            LOG_FIELD_VALUE("Drop on lock",fldValue);
            /* Drop on lock */
            if (descrPtr->saHit == 0)
            {
                if ((fldValue == 1) && (descrPtr->saNoDpv))
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                     __LOG_DROP("Drop on lock");
                    return;
                }
            }


        }
        else
        {

            smemRegFldGet(devObjPtr, regAddr, 14, 1, &fldValue);
            LOG_FIELD_VALUE("Drop on lock",fldValue);
            /* Drop on lock */
            if (fldValue)
            {
                if (descrPtr->saHit == 0)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                     __LOG_DROP("Drop on lock");
                    return;
                }
            }
        }
        regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 10, 2, &fldValue);
        LOG_FIELD_VALUE("802.1Q mode check/secured",fldValue);
        /* 802.1Q mode check/secured */
        if (fldValue == 2 || fldValue == 3)
        {
            if (descrPtr->vtuMiss)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                __LOG_DROP("vtuMiss");
                return;
            }
        }
        /* 802.1Q mode secured  */
        if (fldValue == 3)
        {
            __LOG(("802.1Q mode secured"));
            if (descrPtr->spOk == 0)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                __LOG_DROP("vtuMiss and spOk == 0");
                return;
            }
        }
    }
    if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        if (descrPtr->srcVlanTagged == 0)
        {
            if (descrPtr->priOnlyTag)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                __LOG_DROP("priOnlyTag : only tagged allowed ");
                return;
            }
        }
    }
    /* Error frame check and port state */
    packetSize = descrPtr->frameBuf->actualDataSize;
    /*  The simulation accepted small sized packets
        to be backward compatible with RTG
    if (packetSize < 64)
    {
        descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
        return;
    }
    */

    if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
    {
        smemRegFldGet(devObjPtr, PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort), 12, 2, &fldValue);
        LOG_FIELD_VALUE("(MRU)JumboMode",fldValue);
        MaxFrameSize = fldValue == 0 ? 1522 :
                       fldValue == 1 ? 2048 :
                       fldValue == 2 ? 10240 : 0/*reserved value */;
    }
    else
    {
        smemRegFldGet(devObjPtr, GLB_CTRL_REG, 10, 1, &fldValue);
        LOG_FIELD_VALUE("(MRU)Max frame size",fldValue);
        MaxFrameSize = fldValue ?
            (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType) ? 1632 : 1532):
             1522;
    }

    if(descrPtr->srcVlanTagged == 0)
    {
        MaxFrameSize -= 4;
    }

    if(packetSize > MaxFrameSize)
    {
        descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
        descrPtr->oversizeDrop = GT_TRUE;
        __LOG(("DROP occurred: packet size [%d] was dropped on MRU size[%d] \n",
            packetSize,MaxFrameSize));
        return;
    }

    if (descrPtr->rxSnif)
    {
        descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
        return;
    }

    /* Analyse of Port state */
    regAddr = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
    smemRegFldGet(devObjPtr, regAddr, 0, 2, &portState);
    LOG_FIELD_VALUE("portState",portState);
    if (portState == 0)
    {
        descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
        __LOG_DROP("port disabled ");
        return;
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        if (smemSohoVtuEntryGet(devObjPtr, descrPtr->vid, &vtuEntry) == GT_OK)
        {
            portMap = vtuEntry.portsMap[descrPtr->srcPort];
            vtuPortState = (portMap >> 2) & 0x3;
            if (vtuPortState != SNET_SOHO_DISABLED_STATE)
            {
                portState = vtuPortState;
            }
        }
    }

    if(descrPtr->cascade && descrPtr->marvellTagCmd != TAG_CMD_FORWARD_E)
    {
        /* not checking STP of ingress port and not changing packet command */
    }
    else
    /* check the if packet can be learned according to the port STP state */
    if (portState == SKERNEL_STP_LEARN_E ||
        portState == SKERNEL_STP_FORWARD_E)
    {
        regAddr = PORT_BASED_VLAN_MAP_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 11, 1, &fldValue);
        LOG_FIELD_VALUE("Learn disable",fldValue);
        /* Learn disable */
        if (fldValue == 0)
        {
            /* store ATU */
            if (descrPtr->Mgmt == 1)
            {
                descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
            }
            else if (descrPtr->rxSnif == 0)
            {
                descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;
            }
            else
            {
                descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
            }

            if (descrPtr->rxSnif == 0 && descrPtr->atuEntryAddr)
            {
                smemRegSet(devObjPtr, descrPtr->atuEntryAddr,descrPtr->atuEntry[0]);
                smemRegSet(devObjPtr, (descrPtr->atuEntryAddr + 4),descrPtr->atuEntry[1]);
            }
        }
    }

    /* check the STP state for non management packets */
    if (descrPtr->Mgmt == 0)
    {
        if (portState != SKERNEL_STP_FORWARD_E)
        {
            descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
            __LOG_DROP("stp not FORWARD ");
        }
    }

    /* hardware address learn limit for non trunk packets */
    if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        if (!(descrPtr->srcTrunk))
        {
            globLearnCnt = 0;

            for (port = 0 ; port < 11 ; port++)
            {
                    dpvMember =  SMEM_U32_GET_FIELD(descrPtr->destPortVector, port, 1);
                    __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                        port ,
                        "dpvMember" ,
                        dpvMember));
                    regAddr = PORT_ATU_CONTROL(devObjPtr,port);
                    smemRegFldGet(devObjPtr, regAddr, 15, 1, &readLearnCnt);
                    __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                        port ,
                        "readLearnCnt" ,
                        readLearnCnt));
                    if  ((readLearnCnt == 0) && (dpvMember == 1))
                    {
                        /* PUT VALUE IN GLOBAL */
                        smemRegFldGet(devObjPtr, regAddr, 0, 8, &learnCnt);
                        __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                            port ,
                            "learnCnt" ,
                            learnCnt));
                        learnCnt++;
                        globLearnCnt = learnCnt;
                        /* increment port learn limit counter */
                        smemRegFldSet(devObjPtr, regAddr, 0, 8, learnCnt);
                    }

            }

            if(globLearnCnt)
            {
                /* interrupt */
                smemRegFldSet(devObjPtr, regAddr, 14, 1, 1);
            }
        }
    }
}

/**
* @internal snetSohoL2RubyDecision function
* @endinternal
*
* @brief   Check frame for VLAN violation, port authentication and size errors
*         and apply decision on the frame
*         INPUTS:
*         devObjPtr  - pointer to device object.
*         descrPtr  - pointer to the frame's descriptor.
*/
static GT_VOID snetSohoL2RubyDecision
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoL2RubyDecision);

    GT_U32 regAddr, fldValue;           /* registers address and value */
    GT_U32 packetSize;                  /* actual packet's data size */
    GT_U32 maxPacketSize;               /* max packet's data size */

    if (descrPtr->Mgmt == 0)
    {
        regAddr = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 14, 1, &fldValue);
        LOG_FIELD_VALUE("Drop on lock",fldValue);
        /* Drop on lock */
        if (fldValue)
        {
            if (descrPtr->saHit == 0)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                descrPtr->xdrop = 1;
                descrPtr->filtered = 1;
                descrPtr->destPortVector = 0;
                __LOG_DROP("Drop on lock ");
                return;
            }
        }
        regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 10, 2, &fldValue);
        LOG_FIELD_VALUE("802.1Q mode check/secured",fldValue);
        /* 802.1Q mode check/secured */
        if (fldValue == 2 || fldValue == 3)
        {
            if (descrPtr->vtuMiss)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                descrPtr->filtered = 1;
                descrPtr->destPortVector = 0;
                __LOG_DROP("802.1Q mode check/secured");
                return;
            }
        }

        /* 802.1Q mode secured  */
        if (fldValue == 3)
        {
            if (descrPtr->spOk == 0)
            {
                descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
                descrPtr->filtered = 1;
                descrPtr->destPortVector = 0;
                __LOG_DROP("802.1Q mode secured , spOk == 0");
                return;
            }
        }

        /* Discard tagged frames
           According to the Don Pannel the implementation of Opal
           is such that the untagged/tagged packet checking is done
            even if the port is in Secure mode (tuvia,8/8/05) */
        regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
        if (descrPtr->srcVlanTagged)
        {
            smemRegFldGet(devObjPtr, regAddr, 9, 1, &fldValue);
            LOG_FIELD_VALUE("Discard tagged",fldValue);
            /* Discard tagged */
            if (fldValue)
            {
                descrPtr->filtered = 1;
                descrPtr->destPortVector = 0;
            }
        }
        else
        {
            smemRegFldGet(devObjPtr, regAddr, 8, 1, &fldValue);
            LOG_FIELD_VALUE("Discard tagged",fldValue);
            /* Discard untagged */
            if (fldValue)
            {
                descrPtr->filtered = 1;
                descrPtr->destPortVector = 0;
            }
        }
    }

    /* Error frame check and port state */
    packetSize = descrPtr->frameBuf->actualDataSize;
#if 0 /* not check packteSize for RTG compatibility */
    if (packetSize < 64)
    {
        descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
        return;
    }
#endif
    smemRegFldGet(devObjPtr, GLB_CTRL_REG, 10, 1, &fldValue);
    LOG_FIELD_VALUE("Max frame size",fldValue);
    /* Max frame size */
    if (fldValue == 1)
    {
        maxPacketSize =
            SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType) ? 1632 : 1536;
        if (packetSize > maxPacketSize)
        {
            descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
            return;
        }
    }
    else
    if (descrPtr->srcVlanTagged == 0)
    {
        if (packetSize > 1522)
        {
            descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
            return;
        }
    }
    else
    if (packetSize > 1522)
    {
        descrPtr->pktCmd = SKERNEL_PKT_DROP_E;
        return;
    }

    regAddr = PORT_CTRL_REG(devObjPtr,descrPtr->srcPort);
    smemRegFldGet(devObjPtr, regAddr, 0, 2, &fldValue);
    LOG_FIELD_VALUE("Port state",fldValue);
    /* Port state */
    if (fldValue == SKERNEL_STP_LEARN_E ||
        fldValue == SKERNEL_STP_FORWARD_E)
    {
        regAddr = PORT_BASED_VLAN_MAP_REG(devObjPtr,descrPtr->srcPort);
        smemRegFldGet(devObjPtr, regAddr, 11, 1, &fldValue);
        LOG_FIELD_VALUE("Learn disable",fldValue);
        /* Learn disable */
        if (fldValue == 0)
        {
            if (descrPtr->rxSnif == 0)
            {
                if(descrPtr->atuEntryAddr)
                {
                    smemRegSet(devObjPtr, descrPtr->atuEntryAddr,
                               descrPtr->atuEntry[0]);

                    smemRegSet(devObjPtr, (descrPtr->atuEntryAddr + 4),
                               descrPtr->atuEntry[1]);
                }

                if (descrPtr->Mgmt == 1)
                {
                    descrPtr->pktCmd = SKERNEL_PKT_TRAP_CPU_E;
                }
                else
                {
                    descrPtr->pktCmd = SKERNEL_PKT_FORWARD_E;
                }
            }
            else
            {
                descrPtr->pktCmd = SKERNEL_PKT_MIRROR_CPU_E;
            }
        }
    }
}

/**
* @internal snetSohoStatCountUpdate function
* @endinternal
*
* @brief   Update Stats Counter Ingress Group
*/
static GT_VOID snetSohoStatCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoStatCountUpdate);

    GT_U32 regAddr;                     /* registers address */
    SMEM_REGISTER  counter;             /* counter's value */
    GT_U32 port;                        /* source port id */
    GT_U32 fldValue;

    port = descrPtr->srcPort;

    if (descrPtr->pktCmd == SKERNEL_PKT_DROP_E)
    {
        if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
        {
            if (descrPtr->oversizeDrop)
            {
                /* Bad frame received -
                    currently only Oversize counting is supported at simulation */
                __LOG(("Oversize frame received"));
                regAddr = CNT_IN_OVERSIZE_REG(port);
                smemRegGet(devObjPtr, regAddr, &counter);
                smemRegSet(devObjPtr, regAddr, ++counter);

                regAddr = PORT_CTRL_2_REG(devObjPtr,port);
                smemRegFldGet(devObjPtr, regAddr, 12, 2, &fldValue);
                regAddr = CNT_IN_BAD_OCTETS_LO_REG(port);
                smemRegGet(devObjPtr, regAddr, &counter);
                counter += fldValue == 0 ? 1522 :
                            fldValue == 1 ? 2048 :
                            fldValue == 2 ? 10240 : 0/*reserved value */;
                smemRegSet(devObjPtr, regAddr, counter);
            }
        }
        __LOG(("Dropped packet, so no update of Good MIB counters"));
        return;
    }

    /* Good frames received */
    __LOG(("Good frames received"));
    if (SGT_MAC_ADDR_IS_BCST(descrPtr->dstMacPtr))
    {
        regAddr = CNT_IN_BCST_REG(port);

        smemRegGet(devObjPtr, regAddr, &counter);
        smemRegSet(devObjPtr, regAddr, ++counter);
    }
    else
    if (SGT_MAC_ADDR_IS_MCST(descrPtr->dstMacPtr))
    {
        regAddr = CNT_IN_MCST_REG(port);
        smemRegGet(devObjPtr, regAddr, &counter);
        smemRegSet(devObjPtr, regAddr, ++counter);
    }
    else
    {
        regAddr = CNT_IN_UCAST_FRAMES_REG(port);
        smemRegGet(devObjPtr, regAddr, &counter);
        smemRegSet(devObjPtr, regAddr, ++counter);
    }

    regAddr = GLB_CTRL_2_REG;
    smemRegFldGet(devObjPtr, regAddr, 6, 1, &fldValue);
    if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType) &&
        (fldValue == 0))
    {
        __LOG(("RX histogram is globally disabled \n"));
    }
    else
    {
        switch (SNET_GET_NUM_OCTETS_IN_FRAME(descrPtr->byteCount))
        {
            case SNET_FRAMES_1024_TO_MAX_OCTETS:
                regAddr = CNT_1024_OCTETS_REG(port);
                smemRegGet(devObjPtr, regAddr, &fldValue);
                fldValue++;
                smemRegSet(devObjPtr, regAddr, fldValue);
            break;
            case SNET_FRAMES_512_TO_1023_OCTETS:
                regAddr = CNT_512_TO_1023_OCTETS_REG(port);
                smemRegGet(devObjPtr, regAddr, &fldValue);
                fldValue++;
                smemRegSet(devObjPtr, regAddr, fldValue);
            break;
            case SNET_FRAMES_256_TO_511_OCTETS:
                regAddr = CNT_256_TO_511_OCTETS_REG(port);
                smemRegGet(devObjPtr, regAddr, &fldValue);
                fldValue++;
                smemRegSet(devObjPtr, regAddr, fldValue);
            break;
            case SNET_FRAMES_128_TO_255_OCTETS:
                regAddr = CNT_128_TO_255_OCTETS_REG(port);
                smemRegGet(devObjPtr, regAddr, &fldValue);
                fldValue++;
                smemRegSet(devObjPtr, regAddr, fldValue);
            break;
            case SNET_FRAMES_65_TO_127_OCTETS:
                regAddr = CNT_65_TO_127_OCTETS_REG(port);
                smemRegGet(devObjPtr, regAddr, &fldValue);
                fldValue++;
                smemRegSet(devObjPtr, regAddr, fldValue);
            break;
            case SNET_FRAMES_64_OCTETS:
                regAddr = CNT_64_OCTETS_REG(port);
                smemRegGet(devObjPtr, regAddr, &fldValue);
                fldValue++;
                smemRegSet(devObjPtr, regAddr, fldValue);
            break;
        }
    }

    regAddr = CNT_IN_GOOD_OCTETS_LO_REG(port);
    smemRegGet(devObjPtr, regAddr, &counter);
    counter += descrPtr->byteCount;
    smemRegSet(devObjPtr, regAddr, counter);


}

/**
* @internal snetSohoQcMonitorIngress function
* @endinternal
*
* @brief   Monitoring support in the QC Ingress
*/
static GT_VOID snetSohoQcMonitorIngress
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr,
    OUT SNET_SOHO_MIRROR_STC * mirrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoQcMonitorIngress);

    GT_U32 regAddr;                     /* registers address */
    GT_U32 fldValue;                    /* registers field value */
    GT_U32 arpValue=0;                  /* registers arp value */
    GT_U8 inMonitorDest=0;              /* Ingress monitor dest */
    GT_U8 outMonitorDest=0;             /* Egress monitor dest */
    GT_U32 destRegAddr;                 /* destination register address */
    GT_U8 destPortInd;                  /* counter for destination port loop */

    if ((devObjPtr->deviceType != SKERNEL_RUBY) &&
        !SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        return;
    }

    memset(mirrPtr,0,sizeof(SNET_SOHO_MIRROR_STC));
    /* QC Monitor handling - Egress  */
    regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
    smemRegFldGet(devObjPtr, regAddr, 4, 1, &fldValue);
    LOG_FIELD_VALUE("Ingress Monitor Source",fldValue);
    /* Ingress Monitor Source */
    if (fldValue)
    {
        smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 12, 4, &fldValue);
        LOG_FIELD_VALUE("Ingress monitor destination",fldValue);
        /* Ingress monitor destination */
        inMonitorDest = (GT_U8)fldValue;
        if (descrPtr->destPortVector & inMonitorDest)
        {
            mirrPtr->monFwd = 1;
        }
        else
        {
            descrPtr->destPortVector |= 1 << inMonitorDest;
        }
        mirrPtr->inMon = 1;
    }

    /* Egress Monitor Source */
    __LOG(("Egress Monitor Source"));
    for (destPortInd = 0; destPortInd < devObjPtr->portsNumber; destPortInd++)
    {
       if ((1 << destPortInd) & descrPtr->destPortVector)
       {
            destRegAddr =  PORT_CTRL_2_REG(devObjPtr,destPortInd);
            smemRegFldGet(devObjPtr, destRegAddr, 5, 1, &fldValue);
            __LOG(("port[%d] : <%s> = [0x%x]\n" ,
                destPortInd ,
                "Egress Monitor Source" ,
                fldValue));
            if (fldValue != 0)
            {
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 8, 4, &fldValue);
                LOG_FIELD_VALUE("Egress monitor destination",fldValue);
                /* Egress monitor destination */
                outMonitorDest = (GT_U8)fldValue;
                if (descrPtr->destPortVector & outMonitorDest)
                {
                    mirrPtr->monFwd = 1;
                }
                else
                {
                    descrPtr->destPortVector |= 1 << outMonitorDest;
                }
                mirrPtr->outMon = 1;
            }
        }
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        if (descrPtr->arp)
        {
            if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            {/* ARP MIRROR */
                regAddr = PORT_CTRL_2_REG(devObjPtr,descrPtr->srcPort);
                smemRegFldGet(devObjPtr, regAddr, 6, 1, &fldValue);
                LOG_FIELD_VALUE("ARP MIRROR",fldValue);
            }

            if  (arpValue == 0)
            {
                smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 4, 4, &fldValue);
                LOG_FIELD_VALUE("ARP monitor destination",fldValue);
                /* ARP monitor destination */
                mirrPtr->arpMonitorDest = (GT_U8)fldValue;
                if (mirrPtr->arpMonitorDest != 0xf)
                {
                    if ((descrPtr->destPortVector >> mirrPtr->arpMonitorDest) & 0x1)
                    {
                        mirrPtr->monFwd = 1;
                    }
                    else
                    {
                        descrPtr->destPortVector |= (1 << mirrPtr->arpMonitorDest);
                    }
                    mirrPtr->outMon = 1;
                }
                else
                {
                    descrPtr->arp = 0;
                }
            }
        }
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
        {

            if ((descrPtr->uDPpolicy == 1) || (descrPtr->oPt82policy == 1) ||
                (descrPtr->vBaspolicy == 1) || (descrPtr->pPppolicy == 1) ||
                (descrPtr->eTypepolicy == 1) || (descrPtr->vTupolicy == 1) ||
                (descrPtr->sApolicy == 1) || (descrPtr->oPt82policy == 1))
                {
                    descrPtr->polMirror = 1;
                    smemRegFldGet(devObjPtr, GLB_MON_CTRL_REG, 0, 4, &fldValue);
                    LOG_FIELD_VALUE("Mirror Dest",fldValue);

                    if ( fldValue != 0xf)
                    {
                        if ((descrPtr->destPortVector >> mirrPtr->arpMonitorDest) & 0x1)
                        {
                            mirrPtr->monFwd = 1;
                        }
                        else
                        {
                            descrPtr->destPortVector |= (1 << mirrPtr->arpMonitorDest);
                        }
                    }
                    else
                        descrPtr->polMirror = 0;
                }

            else
                descrPtr->polMirror = 0;
        }
    }
}

/**
* @internal snetSohoQcMonitorEgress function
* @endinternal
*
* @brief   Monitoring support in the QC Egress
*/
static GT_VOID snetSohoQcMonitorEgress
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC * descrPtr,
    IN SNET_SOHO_MIRROR_STC * mirrPtr
)
{
    DECLARE_FUNC_NAME(snetSohoQcMonitorEgress);

    /* QC Monitor handling - Egress  */
    __LOG(("QC Monitor handling - Egress"));

    __LOG_PARAM(mirrPtr->monFwd);
    __LOG_PARAM(mirrPtr->inMon);
    __LOG_PARAM(mirrPtr->outMon);

    if (mirrPtr->monFwd || (mirrPtr->inMon == mirrPtr->outMon))
    {
        descrPtr->txSnif = 0;
        descrPtr->rxSnif = 0;
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            descrPtr->polMirror = 0;

        __LOG_PARAM(descrPtr->txSnif);
        __LOG_PARAM(descrPtr->rxSnif);
        __LOG_PARAM(descrPtr->polMirror);
        __LOG_PARAM(descrPtr->Mgmt);

        snetSohoEgressPacketProc(devObjPtr,descrPtr);
    }

    if (mirrPtr->inMon == 1)
    {
        descrPtr->rxSnif = 1;
        descrPtr->txSnif = 0;
        descrPtr->Mgmt = 1;
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            descrPtr->polMirror = 0;

        __LOG_PARAM(descrPtr->txSnif);
        __LOG_PARAM(descrPtr->rxSnif);
        __LOG_PARAM(descrPtr->polMirror);
        __LOG_PARAM(descrPtr->Mgmt);

        snetSohoEgressPacketProc(devObjPtr,descrPtr);
    }

    if (mirrPtr->outMon == 1)
    {
        descrPtr->rxSnif = 0;
        descrPtr->txSnif = 1;

        __LOG_PARAM(descrPtr->txSnif);
        __LOG_PARAM(descrPtr->rxSnif);
        __LOG_PARAM(descrPtr->polMirror);
        __LOG_PARAM(descrPtr->Mgmt);

        snetSohoEgressPacketProc(devObjPtr,descrPtr);
        if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
            descrPtr->polMirror = 0;

        __LOG_PARAM(descrPtr->polMirror);
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO2(devObjPtr->deviceType))
    {
        __LOG_PARAM(descrPtr->destPortVector);
        __LOG_PARAM(mirrPtr->arpMonitorDest);

        if (descrPtr->destPortVector & mirrPtr->arpMonitorDest)
        {
            __LOG_PARAM(descrPtr->arp);
            if (descrPtr->arp)
            {
                if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
                        descrPtr->polMirror = 0;

                descrPtr->rxSnif = 0;
                descrPtr->txSnif = 0;
                descrPtr->Mgmt = 1;

                __LOG_PARAM(descrPtr->txSnif);
                __LOG_PARAM(descrPtr->rxSnif);
                __LOG_PARAM(descrPtr->polMirror);
                __LOG_PARAM(descrPtr->Mgmt);

                snetSohoEgressPacketProc(devObjPtr,descrPtr);
            }
        }
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO_PLUS(devObjPtr->deviceType))
    {
        __LOG_PARAM(descrPtr->polMirror);
        if (descrPtr->polMirror)
        {
            descrPtr->rxSnif = 0;
            descrPtr->txSnif = 0;
            descrPtr->Mgmt = 0;

            __LOG_PARAM(descrPtr->txSnif);
            __LOG_PARAM(descrPtr->rxSnif);
            __LOG_PARAM(descrPtr->polMirror);
            __LOG_PARAM(descrPtr->Mgmt);

            snetSohoEgressPacketProc(devObjPtr,descrPtr);
        }
    }
}


