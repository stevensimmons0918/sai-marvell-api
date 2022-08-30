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
* @file cpssDrvPpIntCtrl.c
*
* @brief Includes driver interrupts control functions.
*
* @version   10
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#ifdef CHX_FAMILY
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

/**
* @internal internalDriverDxExMxEventsMask function
* @endinternal
*
* @brief   mask/unmask a given event. A masked event doesn't reach a CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] portGroupId              - the port group Id.
* @param[in] intIndex                 - The interrupt cause to enable/disable.
* @param[in] maskEn                   - GT_TRUE mask, GT_FALSE unmask.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS internalDriverDxExMxEventsMask
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  intIndex,
    IN  GT_BOOL maskEn
)
{
    GT_U32 maskIdx;             /* Index in the mask shadows array,     */
                                /* representing the register to which   */
                                /* this cause belongs.                  */
    GT_U32 *intMaskShadow;      /* Holds the mask shadow array pointer. */
    GT_U32 causeBitIdx;         /* Index in the cause register, of the  */
                                /* bit representing the given cause.    */
    GT_U32 maskRegAddr;         /* Address of the interrupt mask        */
                                /* register.                            */
    GT_U32 dataPath;            /* Data Path ID for MACSec interrupts   */
    GT_STATUS retVal;
    PRV_CPSS_DRV_EV_REQ_NODE_STC * intReqNodeList;

    intMaskShadow   = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow;
    maskIdx         = intIndex / 32;
    causeBitIdx     = intIndex % 32;

    intReqNodeList = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;

    if(intReqNodeList == NULL)
    {
        /* the interrupt engine was not initialized. */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(intReqNodeList[intIndex].intMaskReg == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
    {
        /* the entry should be fully ignored !!! */
        return GT_OK;
    }

    if (intReqNodeList[intIndex].uniEvCause != CPSS_UNI_RSRVD_EVENT_E)
    {
        if (prvCpssDrvEvReqQUserHndlGet(intReqNodeList[intIndex].uniEvCause) == NULL)
        {
            /* event not binded yet by application */
            if (GT_TRUE == maskEn)
            {
                /* by default the event is disabled, nothing to do here... */
                return GT_OK;
            }
            else
            {
                /* only binded events can be enabled */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    maskRegAddr = intReqNodeList[intIndex].intMaskReg;

    if(maskEn == GT_FALSE)
        intMaskShadow[maskIdx] |= (1 << causeBitIdx);
    else
        intMaskShadow[maskIdx] &= (~(1 << causeBitIdx));

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase[maskIdx] ==
         PRV_CPSS_DRV_INT_INIT_PHASE_MACSEC_E)
    {
        /* Access to MACSec registers must be avoided for disabled MACSec units.
           DP is coded in bits 7:1 of Extended Data. */
        dataPath = (intReqNodeList[intIndex].uniEvExt >> 1) & 0x7F;
        if (0 == (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->enabledMacSecDpBitmap & (1 << dataPath)))
        {
            /* MACSec is disabled, skip register access. */
            return GT_OK;
        }
    }

    if ((intReqNodeList[intIndex].uniEvCause >= CPSS_SRVCPU_IPC_E) &&
        (intReqNodeList[intIndex].uniEvCause <= CPSS_SRVCPU_MAX_E))
    {
        retVal = prvCpssGenericSrvCpuRegisterWrite(devNum,portGroupId,maskRegAddr,intMaskShadow[maskIdx]);
    }
    else
    if (PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum, maskRegAddr))
    {
        retVal = prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,portGroupId,
                                        PRV_CPSS_REG_MASK_MAC(maskRegAddr),
                                        intMaskShadow[maskIdx]);
    }
    else if (PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(maskRegAddr))
    {
        retVal = prvCpssDrvHwPpMgWriteReg(devNum,
                                          PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(maskRegAddr),
                                          PRV_CPSS_MGS_REG_MASK_MAC(maskRegAddr),
                                          intMaskShadow[maskIdx]);
    }
    else
    {
        retVal = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,maskRegAddr,intMaskShadow[maskIdx]);
    }

    return retVal;
}
/**
* @internal driverDxExMxEventsMask function
* @endinternal
*
* @brief   mask/unmask a given event. A masked event doesn't reach a CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] intIndex                 - The interrupt cause to enable/disable.
* @param[in] maskEn                   - GT_TRUE mask, GT_FALSE unmask.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS driverDxExMxEventsMask
(
    IN  GT_U8   devNum,
    IN  GT_U32  intIndex,
    IN  GT_BOOL maskEn
)
{
    GT_STATUS rc;
    GT_U32  portGroupId;/*port group Id*/

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        rc = internalDriverDxExMxEventsMask(devNum,portGroupId,intIndex,maskEn);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}

/**
* @internal driverDxExMxEventsMaskGet function
* @endinternal
*
* @brief   Gets enable/disable status of a given event reaching the CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] intIndex                 - The interrupt cause to get enable/disable status.
*
* @param[out] enablePtr                - (pointer to)GT_TRUE enable, GT_FALSE disable.
*                                       None.
*/
static GT_VOID driverDxExMxEventsMaskGet
(
    IN   GT_U8   devNum,
    IN   GT_U32  intIndex,
    OUT  GT_BOOL *enablePtr
)
{
    GT_U32 maskIdx;             /* Index in the mask shadows array,     */
                                /* representing the register to which   */
                                /* this cause belongs.                  */
    GT_U32 *intMaskShadow;      /* Holds the mask shadow array pointer. */
    GT_U32 causeBitIdx;         /* Index in the cause register, of the  */
                                /* bit representing the given cause.    */
    GT_U32  portGroupId;        /*port group Id*/


    portGroupId = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.firstActivePortGroup;

    /* defaults */
    *enablePtr = GT_FALSE;

    intMaskShadow   = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow;
    /* Interrupts mechanism isn't initialized -> return GT_FALSE*/
    if (intMaskShadow == NULL)
    {
        return;
    }
    maskIdx         = intIndex >> 5;
    causeBitIdx     = intIndex & 0x1F;

    if ((intMaskShadow[maskIdx] & (1 << causeBitIdx)) != 0)
    {
        /* found unmasked */
        *enablePtr = GT_TRUE;
    }

}

#ifdef CHX_FAMILY

/**
* @internal prvCpssMacSecIntMaskRegCheck function
* @endinternal
*
* @brief   Checks read/write access availability for MACSec EIP units registers.
*          Access to these registers are closed when EIP units are not initialized.
*
* @param[in] devNum                  - The PP's device number to mask / unmask the interrupt for.
* @param[in] regAddr                 - address of interrupt register.
*
* @param[out] skipPtr                - (pointer to)GT_TRUE skip configuration,
*                                       GT_FALSE don't skip configuration.
*/
static GT_STATUS prvCpssMacSecIntMaskRegCheck
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               regAddr,
    OUT GT_BOOL                              *skipPtr
)
{
    GT_U32                  dataPath; /* data path ID where register is */
    PRV_CPSS_DXCH_UNIT_ENT  unitId;   /* Unit ID */

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum, regAddr);
    switch (unitId)
    {
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E:
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E:
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E:
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E:
            dataPath = 0;
            break;
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E:
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E:
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E:
        case PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E:
            dataPath = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (0 == (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->enabledMacSecDpBitmap & (1 << dataPath)))
    {
        /* MACSec is disabled, skip register access. */
        *skipPtr = GT_TRUE;
        return GT_OK;
    }

    *skipPtr = GT_FALSE;
    return GT_OK;
}
#endif
/**
* @internal driverDxExMxInitIntMasks function
* @endinternal
*
* @brief   Set the interrupt mask for a given device.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] portGroupId              - The port group Id
* @param[in] initPhase                - Specifies the PP's init phase.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS driverDxExMxInitIntMasks
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               portGroupId,
    IN  PRV_CPSS_DRV_INT_INIT_PHASE_ENT      initPhase
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC     *intReqNodeList; /* pointer to interrupt nodes DB */
    GT_U32          *intMaskShadow;   /* pointer to interrupt masks shadow DB */
    GT_U32          numOfIntBits;     /* Number of interrupt bits.    */
    GT_U32          i;                /* iterator */
    GT_STATUS       rc, ret = GT_OK;  /* return codes */
    GT_U32          intMaskRegAddr;   /* address of interrupt mask register */

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    intMaskShadow   = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow;
    numOfIntBits    = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numOfIntBits;

    intReqNodeList = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;

    for(i = 0; i < (numOfIntBits >> 5); i++)
    {
        intMaskRegAddr = intReqNodeList[i << 5].intMaskReg;
        if(intMaskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            continue;
        }

        if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase[i] == initPhase)
        {

#ifdef CHX_FAMILY
            if (initPhase == PRV_CPSS_DRV_INT_INIT_PHASE_MACSEC_E)
            {
                GT_BOOL  skip; /* skip configuration of register */

                /* check access to MACSec registers */
                rc = prvCpssMacSecIntMaskRegCheck(devNum, intMaskRegAddr, &skip);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (skip)
                {
                    continue;
                }
            }
#endif

            if ((intReqNodeList[i << 5].uniEvCause >= CPSS_SRVCPU_IPC_E) &&
                (intReqNodeList[i << 5].uniEvCause <= CPSS_SRVCPU_MAX_E))
            {
#ifndef ASIC_SIMULATION
                prvCpssGenericSrvCpuRegisterWrite(devNum,portGroupId,
                                intMaskRegAddr,
                                intMaskShadow[i]);
#endif
                /* ignore return status */
                rc = GT_OK;
            }
            else
            if (PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum, intMaskRegAddr))
            {
                rc = prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,portGroupId,
                                PRV_CPSS_REG_MASK_MAC(intMaskRegAddr),
                                intMaskShadow[i]);
            }
            else if (PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(intMaskRegAddr))
            {
                rc = prvCpssDrvHwPpMgWriteReg(devNum,
                                          PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(intMaskRegAddr),
                                          PRV_CPSS_MGS_REG_MASK_MAC(intMaskRegAddr),
                                          intMaskShadow[i]);
            }
            else
            {
                rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,
                                  intMaskRegAddr,
                                  intMaskShadow[i]);
            }

            if (rc != GT_OK)
            {
                ret = GT_FAIL;
            }
        }
    }
    return ret;
}


/**
* @internal prvCpssDrvDxExMxGppIsrConnect function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt.
*
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] gppId                    - The Gpp Id to be connected.
* @param[in] isrFuncPtr               - A pointer to the function to be called on Gpp interrupt
*                                      reception.
* @param[in] cookie                   - A  to be passed to the isrFuncPtr when its called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 1. To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
GT_STATUS prvCpssDrvDxExMxGppIsrConnect
(
    IN  GT_U8           devNum,
    IN  CPSS_EVENT_GPP_ID_ENT  gppId,
    IN  CPSS_EVENT_ISR_FUNC      isrFuncPtr,
    IN  void            *cookie
)
{
    if((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL) ||
       (gppId >= CPSS_EVENT_GPP_MAX_NUM_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr == NULL)
    {
        /* first time . so need to allocate the ARRAY */
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr =
            cpssOsMalloc(sizeof(PRV_CPSS_DRV_GPP_INT_INFO_STC*) * CPSS_EVENT_GPP_MAX_NUM_E);

        cpssOsMemSet(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr , 0 ,
            sizeof(PRV_CPSS_DRV_GPP_INT_INFO_STC*) * CPSS_EVENT_GPP_MAX_NUM_E);
    }

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId] == NULL)
    {
        /* first bind . so need to allocate the element */
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId] =
            cpssOsMalloc(sizeof(PRV_CPSS_DRV_GPP_INT_INFO_STC));

        cpssOsMemSet(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId],0,
            sizeof(PRV_CPSS_DRV_GPP_INT_INFO_STC));
    }

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId]->gppIsrPtr  = isrFuncPtr;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId]->cookie     = cookie;

    return GT_OK;
}



/**
* @internal prvCpssDrvDxExMxGppIsrCall function
* @endinternal
*
* @brief   Call the isr connected to a specified gpp.
*
* @param[in] devNum                   - The Pp device number.
* @param[in] gppId                    - The GPP Id to call the Isr for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvDxExMxGppIsrCall
(
    IN  GT_U8       devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId
)
{
    CPSS_EVENT_ISR_FUNC    gppIsrFunc;

    if((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL) ||
       (gppId >= CPSS_EVENT_GPP_MAX_NUM_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    gppIsrFunc = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId]->gppIsrPtr;
    if(gppIsrFunc)
    {
        gppIsrFunc(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.gppIsrFuncsArr[gppId]->cookie);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortGroupGopIsrRegRead function
* @endinternal
*
* @brief   Read ports GOP0-3 MIBs Interrupt Cause register value
*         for the specific port group
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - port group Id.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortGroupGopIsrRegRead
(
    IN GT_U8 devNum,
    IN GT_U32  portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
)
{
    GT_STATUS rc;

    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId,regAddr, dataPtr);
    if(rc == GT_OK)
    {
        /* The GOP interrupt cause bits starts from bit 24.
           The correspondent GOP interrupt mask bits and interrupt enum
           starts as usual from bit 0.
           To synch value of cause reg with mask one and enum move value
           of register from bit 24 to bit 0.*/
        *dataPtr >>= 24;
    }

    return rc;
}

/**
* @internal prvCpssDxChGopIsrRegRead function
* @endinternal
*
* @brief   Read ports GOP0-3 MIBs Interrupt Cause register value
*
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChGopIsrRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
)
{
    return prvCpssDxChPortGroupGopIsrRegRead(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,dataPtr);
}

/**
* @internal prvCpssDrvDxExMxRunitGppIsrCall function
* @endinternal
*
* @brief   Call the isr connected to a specified gpp.
*
* @param[in] devNum                   - The Pp device number.
* @param[in] gppId                    - The GPP Id to call the Isr for.
*                                      it is the start index of 8 bits
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvDxExMxRunitGppIsrCall
(
    IN  GT_U8       devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId
)
{
    GT_STATUS rc;
    GT_U32  relativeMpp = gppId;/* mpp index 0..32 */
    GT_U32  regIndex    = relativeMpp >> 5;/* register index 0..1*/
    GT_U32  startBit    = relativeMpp & 0x1f;/* start bit in register */
    GT_U32  tmpCauseBits;   /* Temp to hold the needed cause bits  */
    GT_U32  intMask;        /* Interrupt mask register value     */
    GT_U32  intCause;       /* Interrupt cause register value     */
    GT_U32  portGroupId = 0;
    GT_U32  ii;/* iterator*/
    CPSS_EVENT_GPP_ID_ENT   specific_gppId;

    /* read the cause */
    rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum,
        portGroupId,
        0x00018110 + 0x40*regIndex,
        &intCause);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* read the mask */
    rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum,
        portGroupId,
        0x0001811C + 0x40*regIndex,
        &intMask);
    if(rc != GT_OK)
    {
        return rc;
    }

    specific_gppId = gppId + CPSS_EVENT_MPP_INTERNAL_0_E;
    /* check which of the 8 bits need to call it's specific callback */
    tmpCauseBits = (intCause & intMask) >> startBit;
    for (ii = 0; ii < 8; ii++, tmpCauseBits >>= 1)
    {
        if (tmpCauseBits & 1)
        {
            /* generate explicit MPP index call back */
            rc = prvCpssDrvDxExMxGppIsrCall(devNum,specific_gppId + ii );
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvDxExMxEventsInitObject function
* @endinternal
*
* @brief   This function creates and initializes DxExMx device driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxEventsInitObject
(
     void
)
{
    /* driver object initialization */
    prvCpssDrvGenExMxDxObjPtr->drvHwPpEventsMask     = driverDxExMxEventsMask;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpEventsMaskGet  = driverDxExMxEventsMaskGet;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpInitInMasks    = driverDxExMxInitIntMasks;

    return GT_OK;
}
