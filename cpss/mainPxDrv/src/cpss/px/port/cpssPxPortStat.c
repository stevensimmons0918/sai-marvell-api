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
* @file cpssPxPortStat.c
*
* @brief CPSS implementation for port MAC MIB counters.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>

/* for device that all counters are 64 bits : BC3 */
static GT_U32 internal_gtMacCounterOffset_all64Bits[CPSS_PX_PORT_MAC_COUNTER____LAST____E] =
{
     0x00   /*CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E                */
    ,0x08   /*CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E                 */
    ,0x10   /*CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E                     */
    ,0x18   /*CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E        */
/*0x20 in other place */
    ,0x28   /*CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E           */
    ,0x30   /*CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E           */
    ,0x38   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E                    */
    ,0x40   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E             */
    ,0x48   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E            */
    ,0x50   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E            */
    ,0x58   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E           */
    ,0x60   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E           */
    ,0x20   /*CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E          */
    ,0x68   /*CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E                    */
    ,0x70   /*CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E                  */
/*0x78 -- reserved*/
    ,0x80   /*CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E               */
    ,0x88   /*CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E               */
/*0x90 -- reserved*/
    ,0x98   /*CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E                             */
    ,0xA0   /*CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E                         */
    ,0xA8   /*CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E               */
    ,0xB0   /*CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E                           */
    ,0xB8   /*CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E                           */
    ,0xC0   /*CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E                            */
    ,0xC8   /*CPSS_PX_PORT_MAC_COUNTER_JABBER_E                              */
    ,0xD0   /*CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E             */
    ,0xD8   /*CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E                             */
    ,0xE0   /*CPSS_PX_PORT_MAC_COUNTER_COLLISION_E                           */
    ,0xE8   /*CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E                      */
};

static GT_STATUS prvCpssPxPortMacCountersCGMibsConvert
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U32  *xlgMibEntry/*[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];*/
);

/**
* @internal prvCpssPxPortMibEntrySpecificGet function
* @endinternal
*
* @brief   Gets specific MAC MIB counter from 'HW entry of the port counters'.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] entryArr[/PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS/] -  (pointer to) the memory that hold the port's counters.
* @param[in] cntrName                 - specific counter name
*
* @param[out] counterPtr               - (pointer to) the counter value
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssPxPortMibEntrySpecificGet
(
    IN  GT_U32         entryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName,
    OUT GT_U64          *counterPtr
)
{
    GT_U32 *counterU32Ptr;
    GT_U32 byteOffset;

    byteOffset = internal_gtMacCounterOffset_all64Bits[cntrName];
    counterU32Ptr = &entryArr[byteOffset/4];

    counterPtr->l[0] = counterU32Ptr[0];
    counterPtr->l[1] = counterU32Ptr[1];

    return GT_OK;
}

/**
* @internal prvCpssPxPortMibEntrySpecificSet function
* @endinternal
*
* @brief   Sets specific MAC MIB counter into 'HW entry of the port counters'.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] entryArr[/PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS/] -  (pointer to) the memory that hold the port's counters.
* @param[in] cntrName                 - specific counter name
* @param[in] counterPtr               - (pointer to) the new counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad cntrName
*/
static GT_STATUS prvCpssPxPortMibEntrySpecificSet
(
    IN  GT_U32         entryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName,
    IN  GT_U64          *counterPtr
)
{
    GT_U32 *counterU32Ptr;
    GT_U32 byteOffset;

    CPSS_PARAM_CHECK_MAX_MAC(cntrName,CPSS_PX_PORT_MAC_COUNTER____LAST____E);

    byteOffset = internal_gtMacCounterOffset_all64Bits[cntrName];
    counterU32Ptr = &entryArr[byteOffset/4];

    counterU32Ptr[0] = counterPtr->l[0] ;
    counterU32Ptr[1] = counterPtr->l[1] ;

    return GT_OK;
}

/**
* @internal prvCpssPxPortMibEntrySpecificClear function
* @endinternal
*
* @brief   Clear (set to ZERO) specific MAC MIB counter from 'HW entry of the port counters'.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] entryArr[/PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS/] -  (pointer to) the memory that hold the port's counters.
* @param[in] cntrName                 - specific counter name
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssPxPortMibEntrySpecificClear
(
    IN  GT_U32         entryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName
)
{
    GT_U32 *counterU32Ptr;
    GT_U32 byteOffset;

    byteOffset = internal_gtMacCounterOffset_all64Bits[cntrName];
    counterU32Ptr = &entryArr[byteOffset/4];

    counterU32Ptr[0] = 0;
    counterU32Ptr[1] = 0;

    return GT_OK;
}

/**
* @internal prvCpssPxPortSumMibEntry function
* @endinternal
*
* @brief   Sums the counters (destEntry+addEntryPtr), puts the result to destEntry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] destEntryArr[/PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS/]
* @param[in] addEntryArr[/PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS/]
* @param[in,out] destEntryArr[/PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS/]
*                                       none
*
* @note none
*
*/
GT_VOID prvCpssPxPortSumMibEntry
(
    INOUT GT_U32 destEntryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/],
    IN    GT_U32 addEntryArr[/*PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*/]
)
{
    GT_U32 ii;
    GT_U64 temp1, temp2;

    for (ii = 0 ; ii < PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS ; ii += 2)
    {
        temp1.l[0] = destEntryArr[ii];
        temp1.l[1] = destEntryArr[ii+1];
        temp2.l[0] = addEntryArr[ii];
        temp2.l[1] = addEntryArr[ii+1];
        temp1 = prvCpssMathAdd64(temp1,temp2);
        destEntryArr[ii]   = temp1.l[0];
        destEntryArr[ii+1] = temp1.l[1];
    }
}

/**
* @internal prvCpssPxPortMacCounterGet function
* @endinternal
*
* @brief   Gets specific MAC MIB counter of a specific port.
*         OR
*         Gets 'captured' specific MAC MIB counter of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
* @param[in] getFromCapture           -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortMacCounterGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName,
    IN  GT_BOOL                     getFromCapture,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;      /* return code */
    GT_U32 *entryPtr;
    GT_U32 xlgMibEntry[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32 portMacNum; /* MAC number */
    GT_U32  portGroupId; /* Group id associated with the portNum */
    GT_BOOL isCgMac;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    CPSS_PARAM_CHECK_MAX_MAC(cntrName,CPSS_PX_PORT_MAC_COUNTER____LAST____E);

    /* Get port group according to port mac num */
    portGroupId = 0;

    cntrValuePtr->l[0] = 0;
    cntrValuePtr->l[1] = 0;

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    xlgMibShadowPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];
    if(getFromCapture == GT_TRUE)
    {
        entryPtr = xlgMibShadowPtr->captureMibShadow;
    }
    else
    {
        isCgMac = (PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_CG_E);
        if (isCgMac)
        {
            /* CG MAC MIB counters need to be converted to the regular structure */
            rc = prvCpssPxPortMacCountersCGMibsConvert(devNum,portNum,GT_TRUE,xlgMibEntry);
        }
        else
        {
            /* read the whole entry */
            rc = prvCpssDrvHwPpPortGroupReadRam(CAST_SW_DEVNUM(devNum),portGroupId,
                  PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].macCounters,
                  PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS,xlgMibEntry);
        }

        if (rc != GT_OK)
        {
            return rc;
        }
        /* add to shadow */
        prvCpssPxPortSumMibEntry(xlgMibShadowPtr->mibShadow,xlgMibEntry);
        entryPtr = xlgMibShadowPtr->mibShadow;
    }

    /* get the counter from the entryPtr */
    rc = prvCpssPxPortMibEntrySpecificGet(entryPtr,cntrName,cntrValuePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(getFromCapture == GT_FALSE && xlgMibShadowPtr->clearOnReadEnable == GT_TRUE)
    {
        /* Clearing the specific entry in the shadow */
        rc = prvCpssPxPortMibEntrySpecificClear(entryPtr,cntrName);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortMacCounterGet function
* @endinternal
*
* @brief   Gets specific MAC MIB counter of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCounterGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    return prvCpssPxPortMacCounterGet(devNum, portNum, cntrName,
        GT_FALSE, cntrValuePtr);
}

/**
* @internal cpssPxPortMacCounterGet function
* @endinternal
*
* @brief   Gets specific MAC MIB counter of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCounterGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cntrName, cntrValuePtr));

    rc = internal_cpssPxPortMacCounterGet(devNum, portNum, cntrName, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cntrName, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxPortMacCountersCGPortReadMem function
* @endinternal
*
* @brief   Gets all CG mac mib counters from HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] newCapture               - whether or not to capture a new counters snapshot
*
* @param[out] cgMacMibsCaptureArr      - (pointer to) array of 64bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note cgMacMibsCaptureArr should be of size CPSS_CG_LAST_E.
*
*/
static GT_STATUS prvCpssPxPortMacCountersCGPortReadMem
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U64  *cgMacMibsCaptureArr /* should be sized CPSS_CG_LAST_E */
)
{
    GT_U32 *cgMacMibsRegsAddrDBArrPtr;    /* The CG MIB counters addresses */
    GT_U32  cgMacMibsRegsAddrDBLength;   /* The CG MIB counters length */
    GT_U32  i;   /* Iterator */
    GT_U32  regAddr;    /* CG MIB counter register address */
    GT_U32  portMacNum;  /* Mac number associated with the portNum */
    GT_U32  portGroupId; /* Group id associated with the portNum */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* Mac type associated with the portNum */
    CPSS_PORT_INTERFACE_MODE_ENT portInterfaceMode; /* Interface mode associated with the portNum */
    GT_STATUS rc = GT_OK;
    CPSS_PORT_SPEED_ENT speed;      /* port speed */
    GT_BOOL canReadCg;
    /* Arguments check */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cgMacMibsCaptureArr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get port group according to port mac num */
    portGroupId = 0;

    /* CG port check */
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if (portMacType != PRV_CPSS_PORT_CG_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "portNum[%d] must be in CG MAC mode portMacType :current mode[%d]",
            portNum,portMacType);
    }

    /* Interface mode check. CG interface modes are KR4 and SRLR4 */
    portInterfaceMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portMacNum);
    if ((portInterfaceMode != CPSS_PORT_INTERFACE_MODE_KR4_E && portInterfaceMode != CPSS_PORT_INTERFACE_MODE_CR4_E &&
         portInterfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR4_E) || (portInterfaceMode == CPSS_PORT_INTERFACE_MODE_CR4_E && speed == CPSS_PORT_SPEED_40000_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "portNum[%d] must be in CG MAC mode portInterfaceMode :current mode[%d]",
            portNum,portInterfaceMode);
    }
    mvHwsCgMac28nmAccessLock(CAST_SW_DEVNUM(devNum), portMacNum);
    canReadCg = mvHwsCgMac28nmAccessGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum);
    if (canReadCg == GT_FALSE) {
        mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
        for (i=0 ; i<CPSS_CG_LAST_E ; i++)
        {
            cgMacMibsCaptureArr[i].l[0] = 0;
            cgMacMibsCaptureArr[i].l[1] = 0;
        }
        return GT_OK;
    }

    /* Capturing a new snapshot of the counters */
    if (newCapture)
    {
        /* CG unit requires a capture command to be performed prior to capturing counters for a port */
        PRV_CPSS_PX_REG1_CG_PORT_MAC_STATN_CONFIG_MAC(devNum,portMacNum,&regAddr);
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Failed to Get valid address for the CG port[%d]",portNum);
        }
        rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),portGroupId,regAddr,0/*offset*/,32/*length*/,0x32/*captureRX, captureTX, clearOnRead*/);
        if (rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
            return rc;
        }
    }

    /* Getting length of MIB regs array size */
    cgMacMibsRegsAddrDBLength = CPSS_CG_LAST_E;
    /* get reference to the CG MIB register addresses array */
    cgMacMibsRegsAddrDBArrPtr = PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(devNum,portMacNum);
    if (cgMacMibsRegsAddrDBArrPtr == NULL) {
        mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
    }

    /* Null pointer check */
    CPSS_NULL_PTR_CHECK_MAC(cgMacMibsRegsAddrDBArrPtr);

    /* Reading content of all CG MIBS registers */
    for (i=0 ; i<cgMacMibsRegsAddrDBLength ; i++)
    {
        /* Getting mib counter address */
        regAddr = cgMacMibsRegsAddrDBArrPtr[i];

        /* Reading first 32bits */
        rc = prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,
                    regAddr, &(cgMacMibsCaptureArr[i].l[0]));
        if (rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read first CG mib counter entry from address 0x%08x", regAddr);
        }
        /* Reading second 32bits */
        regAddr += 4;
        rc = prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,
                    regAddr, &(cgMacMibsCaptureArr[i].l[1]));
        if (rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read second CG mib counter entry from address 0x%08x", regAddr);
        }
    }
    mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);

    return GT_OK;
}


/**
* @internal prvCpssPxPortMacCountersCGMibsConvert function
* @endinternal
*
* @brief   Gets CG ethernet MAC counters and convert to legacy mibs structure.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number associated with channel
* @param[in] newCapture               - whether or not to capture a new snapshot of the counters
*
* @param[out] xlgMibEntry              - (pointer to) array of 32bit MIB counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssPxPortMacCountersCGMibsConvert
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL newCapture,
    OUT GT_U32  *xlgMibEntry/*[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];*/
)
{
    GT_U64  cgMacMibsCaptureArr[CPSS_CG_LAST_E];   /* Array to load to all CG counters from HW */
    GT_U64  covertedCgMacMibCounterEntry ;   /* Single counter entry */
    GT_U64  temp64;
    GT_U32  i; /* iterator */
    GT_U32  portMacNum; /* Mac number associated with the portNum */
    GT_STATUS rc;
    CPSS_PX_PORT_MAC_COUNTER_ENT cntrName;    /* Counter names iterator */

    /* Check portNum and get macType */
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    cpssOsMemSet(xlgMibEntry,0,PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS*4);

    /* Reading CG mibs from HW */
    rc = prvCpssPxPortMacCountersCGPortReadMem(devNum, portNum, newCapture, &cgMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Converting CG MIBS to regular MIBS xlgMibEntry struct. Not all cg mib counters
       are needed for the legacy mib convention. */

    /* For all MIB counter */
    for(cntrName = 0; cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E; cntrName++)
    {
        covertedCgMacMibCounterEntry.l[0] = 0;
        covertedCgMacMibCounterEntry.l[1] = 0;

        /* Converting a counter */
        switch (cntrName)
        {
            case CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E]);
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E]);
                }
                covertedCgMacMibCounterEntry  = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E], temp64);
                break;
            case CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E:
                covertedCgMacMibCounterEntry = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OCTETS_E],
                                                                cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E]);
                break;
            case CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_ERRORS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_BC_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_MC_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E:
                /* ONLY RECEIVED!!! */
                covertedCgMacMibCounterEntry  = prvCpssMathAdd64(cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E],
                            cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E]);
                break;
            case CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E:
                temp64.l[0]=0;
                temp64.l[1]=0;
                for (i=0; i<64; i++)
                {
                    /*  all  Flow Control packets (802.3 and PFC)are counted by CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES counter
                       and per class counters shouldn't be taken into consideration in goodOctet calculation*/
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E]);
                    temp64 = prvCpssMathAdd64(temp64,cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E]);
                }
                covertedCgMacMibCounterEntry  = prvCpssMathSub64(cgMacMibsCaptureArr[CPSS_CG_IF_OUT_OCTETS_E], temp64);
                break;
            case CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_MC_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_BC_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_DROP_EVENTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_FRAGMENTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_JABBER_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_JABBERS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_ERRORS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_IN_UC_PKTS_E];
                break;
            case CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E:
                covertedCgMacMibCounterEntry  = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_UC_PKTS_E];
                break;
            default:
                break;
        }

        /* Assigning the converted CG MIB counter to the regular struct array */
        rc = prvCpssPxPortMibEntrySpecificSet(xlgMibEntry,cntrName,&covertedCgMacMibCounterEntry);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @enum PRV_PX_PORT_MAC_CNTR_READ_MODE_ENT
 *
 * @brief Enumeration of the Get MAC Counters function mode.
*/
typedef enum{

    /** usual mode for API call */
    PRV_PX_PORT_MAC_CNTR_READ_MODE_USUAL_E,

    /** @brief read MAC counters from HW
     *  and update shadow DB
     */
    PRV_PX_PORT_MAC_CNTR_READ_MODE_UPDATE_SHADOW_E,

    /** @brief reset counters in HW by read
     *  them. Do not update shadow DB.
     */
    PRV_PX_PORT_MAC_CNTR_READ_MODE_RESET_HW_E

} PRV_PX_PORT_MAC_CNTR_READ_MODE_ENT;

/**
* @internal prvCpssPxPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] getFromCapture           -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
* @param[in] readMode                 - read counters procedure mode
*
* @param[out] countersPtr              - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortMacCountersOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         getFromCapture,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *countersPtr,
    IN  PRV_PX_PORT_MAC_CNTR_READ_MODE_ENT readMode
)
{
    GT_STATUS                  rc;
    CPSS_PX_PORT_MAC_COUNTER_ENT cntrName;  /* read the counter name  */
    GT_U32  xlgMibEntry[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC *xlgMibShadowPtr;
    GT_U32  portMacNum;
    GT_BOOL isCgMac;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (readMode == PRV_PX_PORT_MAC_CNTR_READ_MODE_USUAL_E)
    {
        CPSS_NULL_PTR_CHECK_MAC(countersPtr);
        cpssOsMemSet(countersPtr, 0, sizeof(*countersPtr));
    }

    portGroupId = 0;
    xlgMibShadowPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];

    isCgMac = (PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_CG_E);

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */

    /* If we want to read values from the capture snapshot taken in a certain time */
    if(getFromCapture == GT_TRUE)
    {
        cpssOsMemCpy(xlgMibEntry,xlgMibShadowPtr->captureMibShadow,sizeof(xlgMibEntry));
    }
    /* If we want to get the current values */
    else
    {
        if (isCgMac)
        {
            /* CG MAC MIB counters need to be converted to the regular structure */
            rc = prvCpssPxPortMacCountersCGMibsConvert(devNum,portNum,GT_TRUE,xlgMibEntry);
        }
        else
        {
            /* read the whole entry */
            rc = prvCpssDrvHwPpPortGroupReadRam(CAST_SW_DEVNUM(devNum),portGroupId,
                  PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].macCounters,
                  PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS,xlgMibEntry);
        }
        if (rc != GT_OK)
        {
            return rc;
        }

        /* If the read mode is to reset the hardware counters */
        if (readMode == PRV_PX_PORT_MAC_CNTR_READ_MODE_RESET_HW_E)
        {
            return GT_OK;
        }

        /* We got the current hw entry values. We now need to update the shadow with
           those values. Capture remains as is */
        prvCpssPxPortSumMibEntry(xlgMibEntry,xlgMibShadowPtr->mibShadow);
    } /* else -> if(getFromCapture == GT_TRUE) */

    /* By here, we read the whole mib counters entry of a port, (either from the HW or
       from the software shadow), We will now decompose the entry (xlgMibEntry)
       for proper 64bit entries and update output array to whats needed*/
    if (readMode == PRV_PX_PORT_MAC_CNTR_READ_MODE_USUAL_E)
    {
        /* get all mac counters */
        for(cntrName = 0; cntrName < CPSS_PX_PORT_MAC_COUNTER____LAST____E; cntrName++)
        {
            /* get the counter from the xlgMibEntry */
            rc = prvCpssPxPortMibEntrySpecificGet(xlgMibEntry,cntrName,&countersPtr->mibCounter[cntrName]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* XLG/MSM MIB counters don't support single counter read, clear on read configuration */
    /* and capture. We emulate this features using shadow MIB entries. */
    if ((xlgMibShadowPtr->clearOnReadEnable == GT_TRUE) &&
        (readMode != PRV_PX_PORT_MAC_CNTR_READ_MODE_UPDATE_SHADOW_E))
    {
        cpssOsMemSet(xlgMibShadowPtr->mibShadow, 0, sizeof(xlgMibShadowPtr->mibShadow));
    }
    else
    {
        cpssOsMemCpy(xlgMibShadowPtr->mibShadow,xlgMibEntry,sizeof(xlgMibShadowPtr->mibShadow));
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets ALL MAC MIB counters of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] countersPtr              - (pointer to) counters values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   internal_cpssPxPortMacCountersOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *countersPtr
)
{
    return prvCpssPxPortMacCountersOnPortGet(devNum,portNum, GT_FALSE,
           countersPtr,
           PRV_PX_PORT_MAC_CNTR_READ_MODE_USUAL_E);
}

/**
* @internal cpssPxPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets ALL MAC MIB counters of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] countersPtr              - (pointer to) counters values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *countersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersOnPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, countersPtr));

    rc = internal_cpssPxPortMacCountersOnPortGet(devNum, portNum, countersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, countersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters 'Clear on read' on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCountersClearOnReadSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum]->clearOnReadEnable = enable;
    return GT_OK;
}

/**
* @internal cpssPxPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters 'Clear on read' on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersClearOnReadSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersClearOnReadSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortMacCountersClearOnReadSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get 'Clear on read' status of MAC Counters on the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to) 'Clear on read' status for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCountersClearOnReadGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    *enablePtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum]->clearOnReadEnable;
    return GT_OK;
}

/**
* @internal cpssPxPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get 'Clear on read' status of MAC Counters on the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to) 'Clear on read' status for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersClearOnReadGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersClearOnReadGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortMacCountersClearOnReadGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  NONE.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] counterMode              - oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacOversizedPacketsCounterModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
{
    GT_STATUS rc;
    GT_U32 regAddr;    /* register address to set */
    GT_U32 portMacNum; /* MAC number */
    GT_U32 value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    switch(counterMode)
    {
        case CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E:
            value = 0x0;
            break;
        case CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E:
            value = 0x1;
            break;
        case CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E:
            value = 0x3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(counterMode);
    }

    /* set the GIG MAC */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].mibCountersCtrl;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,6,2,value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set the XG MAC */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].xgMibCountersCtrl;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,11,2,value);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal cpssPxPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  NONE.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] counterMode              - oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacOversizedPacketsCounterModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacOversizedPacketsCounterModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterMode));

    rc = internal_cpssPxPortMacOversizedPacketsCounterModeSet(devNum, portNum, counterMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterModePtr           - (pointer to) oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacOversizedPacketsCounterModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;    /* register address to set */
    GT_U32 portMacNum; /* MAC number */
    GT_U32 value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(counterModePtr);

    /* read the XG port (the GIG mac should be with the same value) */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].xgMibCountersCtrl;
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),0,regAddr,11,2,&value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
        case 0x0:
            *counterModePtr = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E;
            break;
        case 0x1:
            *counterModePtr = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E;
            break;
        case 0x3:
            *counterModePtr = CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Value read from HW [%d] is not recognized",value);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterModePtr           - (pointer to) oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacOversizedPacketsCounterModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacOversizedPacketsCounterModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterModePtr));

    rc = internal_cpssPxPortMacOversizedPacketsCounterModeGet(devNum, portNum, counterModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersEnableSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  update of MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCountersEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32                  portMacNum;      /* MAC number             */
    GT_U32                  macControlReg0;  /* MAC Control register 0 */
    PRV_CPSS_PORT_REG_CONFIG_STC regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }

    /* To enable MIB counters:                           */
    /* Tri-Speed port - set "MIBCntEn"  - bit 15 - to 1. */
    /* XLG            - set "MIBCntDis" - bit 14 - to 0, */

    /* Tri-Speed interfaces */
    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E, &macControlReg0);
    if (macControlReg0 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = macControlReg0;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = (enable) ? 1 : 0;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 15;
    }

    /* XLG */
    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &macControlReg0);
    if (macControlReg0 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = macControlReg0;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = (enable) ? 0 : 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 14;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortMacCountersEnableSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  update of MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortMacCountersEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - state of the MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS internal_cpssPxPortMacCountersEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *enablePtr
)
{
    GT_U32                  offset;
    GT_U32                  portMacNum;
    GT_U32                  value;
    GT_U32                  macControlReg0;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);

    /* For CG port type MAC counters always enabled */
    if (portType == PRV_CPSS_PORT_CG_E)
    {
        *enablePtr = GT_TRUE;
        return GT_OK;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, portType, &macControlReg0);
    if (macControlReg0 == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* If MIB counters enabled:                   */
    /* Tri-Speed port - "MIBCntEn"  - bit 15 - 1. */
    /* XLG            - "MIBCntDis" - bit 14 - 0, */
    if (portType <= PRV_CPSS_PORT_GE_E)
    {
        offset = 15;
    }
    else
    {
        offset = 14;
    }

    if (prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), macControlReg0, offset, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if (portType <= PRV_CPSS_PORT_GE_E)
    {
        *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - state of the MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssPxPortMacCountersEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersEnableGet);

    CPSS_API_LOCK_MAC(0, 0);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortMacCountersEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(0, 0);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCounterCaptureGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT    cntrName,
    OUT GT_U64                          *cntrValuePtr
)
{
    GT_U32      portMacNum;

    if ((GT_U32)cntrName >= (GT_U32)CPSS_PX_PORT_MAC_COUNTER____LAST____E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    return prvCpssPxPortMacCounterGet(devNum, portNum, cntrName, GT_TRUE, cntrValuePtr);
}

/**
* @internal cpssPxPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCounterCaptureGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORT_MAC_COUNTER_ENT    cntrName,
    OUT GT_U64                          *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCounterCaptureGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cntrName, cntrValuePtr));

    rc = internal_cpssPxPortMacCounterCaptureGet(devNum, portNum, cntrName, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cntrName, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCountersCaptureOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *portMacCounterSetArrayPtr
)
{
    return prvCpssPxPortMacCountersOnPortGet(devNum, portNum, GT_TRUE,
            portMacCounterSetArrayPtr, PRV_PX_PORT_MAC_CNTR_READ_MODE_USUAL_E);
}

/**
* @internal cpssPxPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersCaptureOnPortGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORT_MAC_COUNTERS_STC   *portMacCounterSetArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersCaptureOnPortGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacCounterSetArrayPtr));

    rc = internal_cpssPxPortMacCountersCaptureOnPortGet(devNum, portNum,
            portMacCounterSetArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacCounterSetArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, whose counters are to be captured.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
static GT_STATUS internal_cpssPxPortMacCountersCaptureTriggerSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;           /* return status */
    GT_U32      portMacNum;   /* MAC number */
    GT_U32      portGroupId;  /* port group Id - support multi-port-groups device */
    GT_U32      xlgMibEntry[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC  *xlgMibShadowPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* XLG MIB counters does not support single counter read, clear on read          */
    /* configuration and capture. We emulate this features using shadow MIB entries. */

    portGroupId = 0;
    xlgMibShadowPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[portMacNum];

    if (PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        /* CG MAC MIB counters need to be converted to the regular structure */
        rc = prvCpssPxPortMacCountersCGMibsConvert(devNum, portNum, GT_TRUE, xlgMibEntry);
    }
    else
    {
        /* read the whole entry */
        rc = prvCpssDrvHwPpPortGroupReadRam(CAST_SW_DEVNUM(devNum), portGroupId,
                PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].macCounters,
                PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS, xlgMibEntry);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    prvCpssPxPortSumMibEntry(xlgMibShadowPtr->mibShadow, xlgMibEntry);

    cpssOsMemCpy(xlgMibShadowPtr->captureMibShadow, xlgMibShadowPtr->mibShadow,
            sizeof(xlgMibShadowPtr->captureMibShadow));

    return GT_OK;
}

/**
* @internal cpssPxPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, whose counters are to be captured.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
GT_STATUS cpssPxPortMacCountersCaptureTriggerSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersCaptureTriggerSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssPxPortMacCountersCaptureTriggerSet(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] captureIsDonePtr         - pointer to status of Capture counter Trigger
*                                      GT_TRUE  - capture is done.
*                                      GT_FALSE - capture action is in proccess.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
static GT_STATUS internal_cpssPxPortMacCountersCaptureTriggerGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *captureIsDonePtr
)
{

    GT_U32 portMacNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(captureIsDonePtr);

    *captureIsDonePtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssPxPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] captureIsDonePtr         - pointer to status of Capture counter Trigger
*                                      GT_TRUE  - capture is done.
*                                      GT_FALSE - capture action is in proccess.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
GT_STATUS cpssPxPortMacCountersCaptureTriggerGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *captureIsDonePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersCaptureTriggerGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, captureIsDonePtr));

    rc = internal_cpssPxPortMacCountersCaptureTriggerGet(devNum, portNum, captureIsDonePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, captureIsDonePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cgMibStcPtr              - (pointer to) CG mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
static GT_STATUS internal_cpssPxPortMacCountersOnCgPortGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_MAC_CG_COUNTER_SET_STC    *cgMibStcPtr
)
{
    GT_STATUS rc;
    GT_U64    cgMacMibsCaptureArr[CPSS_CG_LAST_E];  /* Array to load to all CG counters from HW */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cgMibStcPtr);

    /* Read all cg mib counters from HW */
    rc = prvCpssPxPortMacCountersCGPortReadMem(devNum, portNum, GT_TRUE,
            &cgMacMibsCaptureArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to read CG mibs");
    }

    /* Assign each value from the CG array to the proper field in the CG struct */
    cgMibStcPtr->ifOutOctets = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_OCTETS_E];
    cgMibStcPtr->ifOutMulticastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_MC_PKTS_E];
    cgMibStcPtr->ifOutErrors = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_ERRORS_E];
    cgMibStcPtr->ifOutUcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_UC_PKTS_E];
    cgMibStcPtr->ifOutBroadcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_OUT_BC_PKTS_E];
    cgMibStcPtr->ifInOctets = cgMacMibsCaptureArr[CPSS_CG_IF_IN_OCTETS_E];
    cgMibStcPtr->ifInErrors = cgMacMibsCaptureArr[CPSS_CG_IF_IN_ERRORS_E];
    cgMibStcPtr->ifInBroadcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_IN_BC_PKTS_E];
    cgMibStcPtr->ifInMulticastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_IN_MC_PKTS_E];
    cgMibStcPtr->ifInUcastPkts = cgMacMibsCaptureArr[CPSS_CG_IF_IN_UC_PKTS_E];
    cgMibStcPtr->aFramesTransmittedOK = cgMacMibsCaptureArr[CPSS_CG_A_FRAMES_TRANSMITTED_OK_E];
    cgMibStcPtr->aFramesReceivedOK = cgMacMibsCaptureArr[CPSS_CG_A_FRAMES_RECIEVED_OK_E];
    cgMibStcPtr->aFrameCheckSequenceErrors = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E];
    cgMibStcPtr->aMACControlFramesTransmitted = cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E];
    cgMibStcPtr->aMACControlFramesReceived = cgMacMibsCaptureArr[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E];
    cgMibStcPtr->aPAUSEMACCtrlFramesTransmitted = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E];
    cgMibStcPtr->aPAUSEMACCtrlFramesReceived = cgMacMibsCaptureArr[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_0 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_1 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_2 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_3 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_4 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_5 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_6 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E];
    cgMibStcPtr->aCBFCPAUSEFramesTransmitted_7 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_0 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_1 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_2 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_3 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_4 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_5 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_6 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E];
    cgMibStcPtr->aCBFCPAUSEFramesReceived_7 = cgMacMibsCaptureArr[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E];
    cgMibStcPtr->etherStatsOctets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OCTETS_E];
    cgMibStcPtr->etherStatsUndersizePkts = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E];
    cgMibStcPtr->etherStatsOversizePkts = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E];
    cgMibStcPtr->etherStatsJabbers = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_JABBERS_E];
    cgMibStcPtr->etherStatsDropEvents = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_DROP_EVENTS_E];
    cgMibStcPtr->etherStatsFragments = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_FRAGMENTS_E];
    cgMibStcPtr->etherStatsPkts = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PKTS_E];
    cgMibStcPtr->etherStatsPkts64Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E];
    cgMibStcPtr->etherStatsPkts65to127Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E];
    cgMibStcPtr->etherStatsPkts128to255Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E];
    cgMibStcPtr->etherStatsPkts256to511Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E];
    cgMibStcPtr->etherStatsPkts512to1023Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E];
    cgMibStcPtr->etherStatsPkts1024to1518Octets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E];
    cgMibStcPtr->etherStatsPkts1519toMaxOctets = cgMacMibsCaptureArr[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E];
    cgMibStcPtr->aAlignmentErrors = cgMacMibsCaptureArr[CPSS_CG_A_ALIGNMENT_ERRORS_E];
    cgMibStcPtr->aFrameTooLongErrors = cgMacMibsCaptureArr[CPSS_CG_A_FRAME_TOO_LONG_ERRORS_E];
    cgMibStcPtr->aInRangeLengthErrors = cgMacMibsCaptureArr[CPSS_CG_A_IN_RANGE_LENGTH_ERRORS_E];
    cgMibStcPtr->VLANTransmittedOK = cgMacMibsCaptureArr[CPSS_CG_VLAN_TRANSMITED_OK_E ];
    cgMibStcPtr->VLANRecievedOK = cgMacMibsCaptureArr[CPSS_CG_VLAN_RECIEVED_OK_E];

    return GT_OK;
}

/**
* @internal cpssPxPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cgMibStcPtr              - (pointer to) CG mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssPxPortMacCountersOnCgPortGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_MAC_CG_COUNTER_SET_STC    *cgMibStcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersOnCgPortGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cgMibStcPtr));

    rc = internal_cpssPxPortMacCountersOnCgPortGet(devNum, portNum, cgMibStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cgMibStcPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] dropCntrStcPtr           - (pointer to) drop counter structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortStatTxDebugCountersGet
(
    IN  GT_SW_DEV_NUM                            devNum,
    OUT CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
)
{
    GT_STATUS  rc;        /* return code */
    GT_U32     regAddr;   /* register address */
    GT_U32     regValue;  /* register value */
    GT_U32     i;         /* loop iterator */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(dropCntrStcPtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.dropCntrs.egrMirroredDroppedCntr;

    rc = prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue, NULL);
    if (GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->egrMirrorDropCntr = regValue;

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.dropCntrs.STCDroppedCntr;

    rc = prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue, NULL);
    if (GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->egrStcDropCntr = regValue;

    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNDropCntr;

    rc = prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue, NULL);
    if (GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->egrQcnDropCntr = regValue;

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[0];

    rc = prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue, NULL);
    if (GT_OK != rc)
    {
        return rc;
    }
    dropCntrStcPtr->dropPfcEventsCntr = regValue;


    dropCntrStcPtr->clearPacketsDroppedCounter = 0;
    for (i = 0; i < 4; i++)
    {
        regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).global.dropCntrs.clearPktsDroppedCntrPipe[i];

        rc = prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue, NULL);
        if (GT_OK != rc)
        {
            return rc;
        }
        dropCntrStcPtr->clearPacketsDroppedCounter += regValue;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] dropCntrStcPtr           - (pointer to) drop counter structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortStatTxDebugCountersGet
(
    IN  GT_SW_DEV_NUM                            devNum,
    OUT CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortStatTxDebugCountersGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCntrStcPtr));

    rc = internal_cpssPxPortStatTxDebugCountersGet(devNum, dropCntrStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCntrStcPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
* @param[in] setModeBmp               - counter mode bitmap. Supported counter mode:
*                                      CPSS_EGRESS_CNT_TC_E,
*                                      CPSS_EGRESS_CNT_DP_E,
*                                      CPSS_EGRESS_CNT_PORT_E.
*                                      Example:
*                                      CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue, if corresponding bit in setModeBmp is 1.
*                                      (APPLICABLE RANGE: 0..7).
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels:
*       CPSS_DP_GREEN_E,
*       CPSS_DP_RED_E.
*
*/
static GT_STATUS internal_cpssPxPortEgressCntrModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
{
    GT_STATUS  rc;          /* return code      */
    GT_U32     regAddr;     /* register address */
    GT_U32     regAddr1;    /* register address */
    GT_U32     value;       /* register value   */
    GT_U32     value1;      /* register value   */
    GT_U32     pxDpLevel;   /* DP level value   */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* only 2 sets supported for now */
    if(cntrSetNum > 1)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(setModeBmp &~ (
            CPSS_EGRESS_CNT_PORT_E |
            CPSS_EGRESS_CNT_TC_E   |
            CPSS_EGRESS_CNT_DP_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.txQMIBCntrsSetConfig[cntrSetNum];
    regAddr1 = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.txQMIBCntrsPortSetConfig[cntrSetNum];

    value = setModeBmp & (CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E);
    value1 = setModeBmp & CPSS_EGRESS_CNT_PORT_E;

    if (setModeBmp & CPSS_EGRESS_CNT_TC_E)
    {
        PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);

        /* write the port monitored by set */
        value |= (tc & 0x07) << 26;
    }

    if (setModeBmp & CPSS_EGRESS_CNT_DP_E)
    {
        PRV_CPSS_PX_COS_DP_TO_HW_CONVERT_MAC(dpLevel, pxDpLevel);

        /* write the port monitored by set */
        value |= pxDpLevel << 29;
    }

    if (setModeBmp & CPSS_EGRESS_CNT_PORT_E)
    {
        PRV_CPSS_PORT_MAC_CHECK_MAC(devNum, portNum);

        value1 |= portNum << 4;
    }

    /* tc, dp */
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* port */
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr1, value1);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
* @param[in] setModeBmp               - counter mode bitmap. Supported counter mode:
*                                      CPSS_EGRESS_CNT_TC_E,
*                                      CPSS_EGRESS_CNT_DP_E,
*                                      CPSS_EGRESS_CNT_PORT_E.
*                                      Example:
*                                      CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue, if corresponding bit in setModeBmp is 1.
*                                      (APPLICABLE RANGE: 0..7).
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels:
*       CPSS_DP_GREEN_E,
*       CPSS_DP_RED_E.
*
*/
GT_STATUS cpssPxPortEgressCntrModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEgressCntrModeSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrSetNum, setModeBmp, portNum, tc, dpLevel));

    rc = internal_cpssPxPortEgressCntrModeSet(devNum, cntrSetNum, setModeBmp, portNum, tc, dpLevel);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrSetNum, setModeBmp, portNum, tc, dpLevel));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of egress counters set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. Supported counter mode:
*                                      CPSS_EGRESS_CNT_TC_E,
*                                      CPSS_EGRESS_CNT_DP_E,
*                                      CPSS_EGRESS_CNT_PORT_E.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortEgressCntrModeGet
(
    IN   GT_SW_DEV_NUM                   devNum,
    IN   GT_U32                          cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PHYSICAL_PORT_NUM            *portNumPtr,
    OUT  GT_U32                          *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
{
    GT_STATUS  rc;          /* return code      */
    GT_U32     regAddr;     /* register address */
    GT_U32     regAddr1;    /* register address */
    GT_U32     value;       /* register value   */
    GT_U32     value1;      /* register value   */
    GT_U32     pxDpLevel;   /* DP HW value      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(setModeBmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcPtr);
    CPSS_NULL_PTR_CHECK_MAC(dpLevelPtr);

    /* only 2 sets supported for now */
    if(cntrSetNum > 1)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).
            peripheralAccess.egrMIBCntrs.txQMIBCntrsSetConfig[cntrSetNum];
    regAddr1 = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).
            peripheralAccess.egrMIBCntrs.txQMIBCntrsPortSetConfig[cntrSetNum];

    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr1, &value1);
    if (rc != GT_OK)
    {
        return rc;
    }

    *setModeBmpPtr = value1 & CPSS_EGRESS_CNT_PORT_E;
    *setModeBmpPtr |= value & (CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E);

    *portNumPtr = U32_GET_FIELD_MAC(value1, 4, 17);
    *tcPtr = U32_GET_FIELD_MAC(value, 26, 3);

    pxDpLevel = (value >> 29) & 0x03;

    PRV_CPSS_PX_COS_HW_TO_DP_CONVERT_MAC(pxDpLevel, (*dpLevelPtr));

    return GT_OK;
}

/**
* @internal cpssPxPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of egress counters set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. Supported counter mode:
*                                      CPSS_EGRESS_CNT_TC_E,
*                                      CPSS_EGRESS_CNT_DP_E,
*                                      CPSS_EGRESS_CNT_PORT_E.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEgressCntrModeGet
(
    IN   GT_SW_DEV_NUM                   devNum,
    IN   GT_U32                          cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PHYSICAL_PORT_NUM            *portNumPtr,
    OUT  GT_U32                          *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEgressCntrModeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrSetNum, setModeBmpPtr, portNumPtr, tcPtr, dpLevelPtr));

    rc = internal_cpssPxPortEgressCntrModeGet(devNum, cntrSetNum, setModeBmpPtr, portNumPtr, tcPtr, dpLevelPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrSetNum, setModeBmpPtr, portNumPtr, tcPtr, dpLevelPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortEgressCntrsGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          cntrSetNum,
    OUT CPSS_PX_PORT_EGRESS_CNTR_STC    *egrCntrPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(egrCntrPtr);

    if (cntrSetNum > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read Outgoing Packet Count Register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setOutgoingPktCntr[cntrSetNum];
    if (prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &egrCntrPtr->outFrames, NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* read Transmit Queue Filtered Packet Count Register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[cntrSetNum];
    if (prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &egrCntrPtr->txqFilterDisc, NULL) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number.
*                                      (APPLICABLE RANGE: 0..1).
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEgressCntrsGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          cntrSetNum,
    OUT CPSS_PX_PORT_EGRESS_CNTR_STC    *egrCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEgressCntrsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrSetNum, egrCntrPtr));

    rc = internal_cpssPxPortEgressCntrsGet(devNum, cntrSetNum, egrCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrSetNum, egrCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortMacCountersHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         received/transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] direction                - CPSS_PORT_DIRECTION_RX_E - set enable updating for
*                                      received packets;
*                                      CPSS_PORT_DIRECTION_TX_E - set enable updating for
*                                      transmitted packets.
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE  - counters are updated;
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device or physical port number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS prvCpssPxPortMacCountersHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_BOOL                 enable
)
{
    GT_U32      regAddr;        /* register address to set */
    GT_U32      value;          /* value to set to bits    */
    GT_U32      offset;         /* bit's offset            */
    GT_STATUS   rc;             /* return code             */
    GT_U32      portGroupId;    /* the port group Id - support multi-port-groups device */
    GT_U32      portMacNum;     /* port MAC number         */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Supported just CPSS_PORT_DIRECTION_RX_E and CPSS_PORT_DIRECTION_TX_E */
    if ((direction != CPSS_PORT_DIRECTION_RX_E) &&
        (direction != CPSS_PORT_DIRECTION_TX_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = (enable) ? 1 : 0;
    offset = (direction == CPSS_PORT_DIRECTION_RX_E) ? 2 : 3;
    portGroupId = 0;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].xgMibCountersCtrl;

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,
                                                regAddr, offset, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].mibCountersCtrl;

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        return prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,
                                                  regAddr, offset, 1, value);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortMacCountersRxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE  - counters are updated.
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device or physical port number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssPxPortMacCountersRxHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    return prvCpssPxPortMacCountersHistogramEnableSet(devNum, portNum,
            CPSS_PORT_DIRECTION_RX_E, enable);
}

/**
* @internal cpssPxPortMacCountersRxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE  - counters are updated.
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device or physical port number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssPxPortMacCountersRxHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersRxHistogramEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortMacCountersRxHistogramEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersTxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_TRUE  - counters are updated.
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacCountersTxHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    return prvCpssPxPortMacCountersHistogramEnableSet(devNum, portNum,
            CPSS_PORT_DIRECTION_TX_E, enable);
}

/**
* @internal cpssPxPortMacCountersTxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_TRUE  - counters are updated.
*                                      GT_FALSE - counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacCountersTxHistogramEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersTxHistogramEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortMacCountersTxHistogramEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortMacCountersHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for received/transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] direction                - CPSS_PORT_DIRECTION_RX_E - get state of received packets
*                                      CPSS_PORT_DIRECTION_TX_E - get state of transmitted packets
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, physical port or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS prvCpssPxPortMacCountersHistogramEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      offset;        /* bit's offset */
    GT_U32      portMacNum;
    GT_U32      portGroupId;
    GT_U32      value    = 0;  /* value read from register */
    GT_U32      regAddr  = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if ((direction != CPSS_PORT_DIRECTION_RX_E) &&
        (direction != CPSS_PORT_DIRECTION_TX_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = (direction == CPSS_PORT_DIRECTION_RX_E) ? 2 : 3;
    portType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    portGroupId = 0;

    if (portType <= PRV_CPSS_PORT_GE_E)
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
                GOP.perPortRegs[portMacNum].mibCountersCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
                GOP.perPortRegs[portMacNum].xgMibCountersCtrl;
    }

    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,
                                            regAddr, offset, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal internal_cpssPxPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS internal_cpssPxPortMacCountersRxHistogramEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    return prvCpssPxPortMacCountersHistogramEnableGet(devNum, portNum,
            CPSS_PORT_DIRECTION_RX_E, enablePtr);
}

/**
* @internal cpssPxPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssPxPortMacCountersRxHistogramEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersRxHistogramEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortMacCountersRxHistogramEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for transmitted packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
static GT_STATUS internal_cpssPxPortMacCountersTxHistogramEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    return prvCpssPxPortMacCountersHistogramEnableGet(devNum, portNum,
            CPSS_PORT_DIRECTION_TX_E, enablePtr);
}

/**
* @internal cpssPxPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for transmitted packets
*                                      GT_TRUE  - counters are enabled.
*                                      GT_FALSE - counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssPxPortMacCountersTxHistogramEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacCountersTxHistogramEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortMacCountersTxHistogramEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


