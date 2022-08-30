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
* @file cpssDxChPortCtrl.c
*
* @brief CPSS implementation for Port configuration and control facility.
*
* The following APIs can run now "preliminary stage" and "post stage" callbacks
* if the bind with port MAC object pointer will be run:
*
* - cpssDxChPortDuplexAutoNegEnableSet;
* - cpssDxChPortDuplexAutoNegEnableGet;
* - cpssDxChPortFlowCntrlAutoNegEnableSet;
* - cpssDxChPortFlowCntrlAutoNegEnableGet;
* - cpssDxChPortSpeedAutoNegEnableSet;
* - cpssDxChPortSpeedAutoNegEnableGet;
* - cpssDxChPortFlowControlEnableSet;
* - cpssDxChPortFlowControlEnableGet;
* - cpssDxChPortPeriodicFcEnableSet;
* - cpssDxChPortPeriodicFcEnableGet;
* - cpssDxChPortBackPressureEnableSet;
* - cpssDxChPortBackPressureEnableGet;
* - cpssDxChPortLinkStatusGet;
* - cpssDxChPortDuplexModeSet;
* - cpssDxChPortDuplexModeGet;
* - cpssDxChPortEnableSet;
* - cpssDxChPortEnableGet;
* - cpssDxChPortExcessiveCollisionDropEnableSet;
* - cpssDxChPortExcessiveCollisionDropEnableGet;
* - cpssDxChPortPaddingEnableSet;
* - cpssDxChPortPaddingEnableGet;
* - cpssDxChPortPreambleLengthSet;
* - cpssDxChPortPreambleLengthGet;
* - cpssDxChPortCrcCheckEnableSet;
* - cpssDxChPortCrcCheckEnableGet;
* - cpssDxChPortMruSet;
* - cpssDxChPortMruGet;
*
* @version   225
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCombo.h>

#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/generic/labservices/port/gop/silicon/bobk/mvHwsBobKCpll.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>

#include <cpss/common/port/private/prvCpssPortManagerTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define DXCH_HWINIT_GLOVAR(_x) PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit._x)

#define PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.txqDbgDir.txqDbgSrc._var,_value)

#define PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.txqDbgDir.txqDbgSrc._var)

#define PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.portDir.portCtrlSrc._var,_value)

#define PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portCtrlSrc._var)

#define PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.portDir.portCtrlSrc._var,_value)

#define PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portCtrlSrc._var)

extern GT_STATUS prvSpeedPortSpeedChangeAllowOsPrintGet(GT_VOID);

#define OS_PORT_ENABLE_PRINTF(_x)      do\
                                       {\
                                           if(prvSpeedPortSpeedChangeAllowOsPrintGet())\
                                           {\
                                             cpssOsPrintf _x;\
                                           }\
                                       }while(0);



#define PRV_CPSS_DXCH_SIP6_TXQ_DRAIN_ITERATION_NUM_MAC 100
#define PRV_CPSS_DXCH_SIP6_TXQ_DRAIN_ITERATION_NUM_ON_EMULATOR_MAC 300
#define PRV_CPSS_DXCH_SIP6_MAC_DRAIN_ITERATION_NUM_MAC 150
#define PRV_CPSS_DXCH_SIP6_MAC_IDLE_CONSECUTIVE_ITERATION_NUM_MAC 5
#define PRV_CPSS_PORT_DOWN_HW_WAIT_TO(_devNum) do\
                                   {\
                                        if(PRV_CPSS_SIP_6_CHECK_MAC(_devNum) == GT_TRUE)\
                                        {\
                                          OS_PORT_ENABLE_PRINTF(("sleep 1ms\n"));\
                                          cpssOsTimerWkAfter(1);\
                                        }\
                                        else\
                                        {\
                                          cpssOsTimerWkAfter(10);\
                                        }\
                                   }\
                                   while(0);\



#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myCpssOsMalloc
(
    IN GT_U32 size
)
{
    return cpssOsMalloc_MemoryLeakageDbg(size,__FILE__ " for HWS",__LINE__);
}
static GT_VOID myCpssOsFree
(
    IN GT_VOID* const memblock
)
{
    cpssOsFree_MemoryLeakageDbg(memblock,__FILE__ " for HWS",__LINE__);
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

#ifdef ASIC_SIMULATION
extern GT_STATUS snetGmPortInfoGet
(
    IN  GT_U8    deviceNumber,
    IN  GT_U8    portNumber,
    OUT GT_BOOL  *isLinkUpPtr,
    OUT GT_BOOL  *isLinkChangedPtr
);
extern GT_STATUS   getSimDevIdFromSwDevNum
(
    IN GT_U8    swDevNum,
    IN  GT_U32  portGroupId,
    OUT GT_U32  *simDeviceIdPtr
);

#endif/*ASIC_SIMULATION*/

extern GT_STATUS  prvCpssDxChTxqSip6_10PreemptionEnableSet
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             physicalPortNum,
    IN GT_BOOL                          enable
);

/* flag to allow print of the 'TXQ num of buffers' when we do operations to wait
   for it to drain */
GT_STATUS debug_sip6_cpssDxChPortTxBufNumberGet_set(IN GT_U32 allowPrint)
{
    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(debug_sip6_cpssDxChPortTxBufNumberGet, allowPrint);
    return GT_OK;
}

#define PRINT_TXQ_DESC(devNum, portNum) \
    if(PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_cpssDxChPortTxBufNumberGet)) /* for debug*/                \
    {                                   \
        GT_U32 _portTxqBufNum;          \
        cpssDxChPortTxBufNumberGet(devNum, portNum, &_portTxqBufNum);    \
        /* print value if non zero or was not zero last time */          \
        if(PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(old_value) != 0 || _portTxqBufNum != 0)                        \
        {                                                                \
            cpssOsPrintf("_portTxqBufNum[%d] %s %d portNum[%d] \n",      \
                 _portTxqBufNum , __FILE__ , __LINE__ , portNum);        \
        }                                                                \
        PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(old_value, _portTxqBufNum);                                      \
    }

extern PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN
        portIfModeFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1]
                                                                        [CPSS_PORT_INTERFACE_MODE_NA_E];
extern PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN
        portIfModeGetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1];

#define portSpeedSetFuncPtrArray PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSpeedSrc.portSpeedSetFuncPtrArray)

#define portSpeedGetFuncPtrArray PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSpeedSrc.portSpeedGetFuncPtrArray)

#define portSerdesPowerStatusSetFuncPtrArray PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSerdesCfgSrc.portSerdesPowerStatusSetFuncPtrArray)


/* In Falcon physical ports that are  higher then 257 can not generate flow control */
#define PRV_CPSS_SIP_6_FC_CFG_ENABLE_CHECK_MAC(_devNum, _portNum,_fcMode) \
   do\
   {\
    if ((PRV_CPSS_PP_MAC(_devNum)->appDevFamily & (CPSS_FALCON_E))&&\
        (_fcMode!=CPSS_DXCH_PORT_FC_MODE_DISABLE_E)) \
    {\
        if(_portNum>257)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Port %d does not support flow control frame generation\n",_portNum);\
        }\
    }\
   }\
   while(0);

/**
* @internal prvCpssDxChPortPmFuncBind function
* @endinternal
*
* @brief   bind pm functions.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cpssPmFuncPtr         - function pointers to bind.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
*/
GT_STATUS prvCpssDxChPortPmFuncBind
(
    IN  GT_U8                   devNum,
    IN  PRV_CPSS_PORT_PM_FUNC_PTRS *cpssPmFuncPtr
)
{
    if ( cpssPmFuncPtr != NULL )
    {
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->pmPortFuncPtrsStc).cpssPmIsFwFunc = cpssPmFuncPtr->cpssPmIsFwFunc;
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChPortMacConfigurationClear function
* @endinternal
*
* @brief   Clear array of registers data
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] regDataArray             - "clean" array of register's data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChPortMacConfigurationClear
(
    INOUT PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   *regDataArray
)
{
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    CPSS_NULL_PTR_CHECK_MAC(regDataArray);

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType <
                                            PRV_CPSS_PORT_NOT_APPLICABLE_E;
                                                                portMacType++)
    {
        regDataArray[portMacType].regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacConfiguration function
* @endinternal
* @brief   Write value to register field and duplicate it to other members of SW
*         combo if needed
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] regDataArray             - array of register's address/offset/field lenght/value to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortMacConfiguration
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  const PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   *regDataArray
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroupId; /* port group id */
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* MAC unit of port */
    GT_PHYSICAL_PORT_NUM    portMacNum;  /* MAC number of given physical port */
    GT_U32                  comboRegAddr;/* address of not active combo port */
    GT_U32                  i;          /* iterator */
    CPSS_DXCH_PORT_MAC_PARAMS_STC  mac; /* params of MAC member in combo */

    CPSS_NULL_PTR_CHECK_MAC(regDataArray);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
        {
            if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;
        }

        if (regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if(PRV_CPSS_PORT_CG_E == portMacType)
                mvHwsCgMac28nmAccessLock(devNum, portMacNum);

            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                    regDataArray[portMacType].regAddr,
                                                    regDataArray[portMacType].fieldOffset,
                                                    regDataArray[portMacType].fieldLength,
                                                    regDataArray[portMacType].fieldData);
            if(PRV_CPSS_PORT_CG_E == portMacType)
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        && (portNum != CPSS_CPU_PORT_NUM_CNS))
    {
        GT_BOOL                 enable;
        CPSS_DXCH_PORT_COMBO_PARAMS_STC comboParams;

        rc = cpssDxChPortComboModeEnableGet(devNum, portNum, &enable, &comboParams);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(enable)
        {
            if (cpssDxChPortComboPortActiveMacGet(devNum, portMacNum, &mac) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            for(i = 0; i < CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS; i++)
            {
                if(CPSS_DXCH_PORT_COMBO_NA_MAC_CNS == comboParams.macArray[i].macNum)
                    break;

                if((mac.macNum == comboParams.macArray[i].macNum) &&
                    (mac.macPortGroupNum == comboParams.macArray[i].macPortGroupNum))
                {
                    continue;
                }

                for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
                    {
                        if((portNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                            continue;
                    }

                    if(regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                    {
                        comboRegAddr = (regDataArray[portMacType].regAddr & 0xFFFF0FFF) + comboParams.macArray[i].macNum * 0x1000;

                        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                                comboRegAddr,
                                                                regDataArray[portMacType].fieldOffset,
                                                                regDataArray[portMacType].fieldLength,
                                                                regDataArray[portMacType].fieldData) != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                        }
                    }

                }/* for(portMacType */

            }/* for(i = 0; */

        }/* if combo port */

    }/* if lion2 */

    return GT_OK;
}

#define PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_LEN_CNS   8
#define PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_GE_CNS   7
#define PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS   0
#define PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_CG_CNS   0

#define PRV_CPSS_DXCH_SD_MUX_PORT_TO_LANE_NUMBER_MAC(curPortNumber) (4*curPortNumber)
#define PRV_CPSS_DXCH_MUX_DB_ARRAY_SLOT_MAC(num) (2*num)

/**
* @internal internal_cpssDxChPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macSaLsb                 - The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*/
static GT_STATUS internal_cpssDxChPortMacSaLsbSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U8    macSaLsb
)
{
    GT_STATUS       rc;              /* return code */
    GT_U32          regAddr;         /* register address */
    GT_U32          value;           /* value to write into the register */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
    GT_U32          portMacNum;      /* MAC number */
    GT_BOOL         doPpMacConfig = GT_TRUE;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if ((portMacObjPtr != NULL) &&
        (portMacObjPtr->macDrvMacSaLsbSetFunc))
    {

        rc = portMacObjPtr->macDrvMacSaLsbSetFunc(devNum,portNum,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  macSaLsb);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSip6MacSaLsbSet(devNum, portNum, macSaLsb);
    }

    value = macSaLsb;

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
       (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            regAddr = fcaStcPtr->MACSA0To15;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
        }

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, value);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_GE_CNS;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS;
    }

    /* In CG unit devices, there is a dedicated 32bit register for the LSBs (and another 32 bit
       register for the MSB bites) */
    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* Setting the dedicated 32 LSBs register */
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_CG_CNS;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.macAddr0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 8;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.macAddr0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = 8;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.macAddr0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 8;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.macAddr0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 8;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 0;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);


    /**************/
    /* BR section */
    /**************/
    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_macAddr0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 8;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);

    return rc;
}

/**
* @internal cpssDxChPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macSaLsb                 - The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*/
GT_STATUS cpssDxChPortMacSaLsbSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U8    macSaLsb
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacSaLsbSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macSaLsb));

    rc = internal_cpssDxChPortMacSaLsbSet(devNum, portNum, macSaLsb);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macSaLsb));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macSaLsbPtr              - (pointer to) The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*/
static GT_STATUS internal_cpssDxChPortMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8    *macSaLsbPtr
)
{
    GT_U32          regAddr;         /* register address */
    GT_U32          value;
    GT_U32          fieldOffset;     /* start to write register at this bit */
    GT_STATUS       rc = GT_OK;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
    GT_U32          portMacNum;      /* MAC number */
    GT_BOOL         doPpMacConfig = GT_TRUE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(macSaLsbPtr);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if ((portMacObjPtr != NULL) &&
       (portMacObjPtr->macDrvMacSaLsbGetFunc))
    {

       rc = portMacObjPtr->macDrvMacSaLsbGetFunc(devNum,portNum,
                                 CPSS_MACDRV_STAGE_PRE_E,
                                 &doPpMacConfig,
                                 macSaLsbPtr);
       if(rc!=GT_OK)
       {
           return rc;
       }

       if(doPpMacConfig == GT_FALSE)
       {
           return GT_OK;
       }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSip6MacSaLsbGet(devNum, portNum, macSaLsbPtr);
    }

   /* cpssDxChPortMacSaLsbSet updates all available MAC's - here enough to read */
    /* one of them */
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (portMacType == PRV_CPSS_PORT_CG_E )
        {
            PRV_CPSS_DXCH_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum, portMacNum, &regAddr);
            fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_CG_CNS;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        fieldOffset = PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_OFFSET_GE_CNS;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
            regAddr, fieldOffset, PRV_CPSS_DXCH_PORT_MAC_SA_LSB_FLD_LEN_CNS, &value);
    if(rc != GT_OK)
        return rc;

    *macSaLsbPtr = (GT_U8)value;

    return rc;
}

/**
* @internal cpssDxChPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macSaLsbPtr              - (pointer to) The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*/
GT_STATUS cpssDxChPortMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8    *macSaLsbPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacSaLsbGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macSaLsbPtr));

    rc = internal_cpssDxChPortMacSaLsbGet(devNum, portNum, macSaLsbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macSaLsbPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortDisableWait function
* @endinternal
*
* @brief   Wait for queues empty on disable port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortDisableWait
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacNum; /* MAC number */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    GT_U16      portTxqDescNum = (GT_U16)-1; /* number of not treated TXQ descriptors */
    GT_U16      prevPortTxqDescNum; /* previous number of not treated TXQ descriptors */
    GT_U32      portTxqBufNum = 0; /* Used for SIP6 devices */
    GT_U32      timeout;        /* resources free timeout counter */
    GT_U32      portTxdmaNum;   /* TXDMA allocated for port */
    GT_U32      scdmaTxFifoCounters;    /* value of both Header(bits 10-19)
                                and Payload(bits 0-9) scdma TxFifo Counters */
    GT_U8       tcQueue;
    GT_BOOL     queueEn;
    GT_U32      portGroupId; /* group number of port (in multi-port-group device)*/
    GT_U32      fifoThresholdCounters = 0;
    GT_U32      txdmaPortCreditsCounters = 0;
    GT_BOOL     extenderCascadePort;
    GT_U32      isValidPortInterfaceMode;/* 0 - non-valid speed/interface , 1 - valid speed/interface */
    GT_U32      timeOutIterations =PRV_CPSS_DXCH_SIP6_TXQ_DRAIN_ITERATION_NUM_MAC;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    isValidPortInterfaceMode = (PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum) != CPSS_PORT_INTERFACE_MODE_NA_E) ?
                    1 : /* there is no   speed/interface mode */
                    0;  /* there is real speed/interface mode */

    /* ensure last packets passed pipe */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (cpssDeviceRunCheck_onEmulator())
    {
        timeOutIterations = PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_polling_iterations);
    }

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                   portMacNum);
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
    {
        extenderCascadePort = portMapShadowPtr->portMap.txqNum == GT_NA ? GT_TRUE : GT_FALSE;
    }
    else
    {
        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &extenderCascadePort,NULL);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(extenderCascadePort == GT_FALSE) /* extender cascade port has no TXQ (GT_NA in TXQ at mapping) therefore no sense in test */
    {
        for(tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            rc = cpssDxChPortTxQueueTxEnableGet(devNum, portNum, tcQueue,
                                                &queueEn);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(!queueEn)
            { /* if at least one of queues disabled, exit without wait,
                 we suppose that user debugging system and descriptors left in
                 pipe no matter */
                return GT_OK;
            }
        }
     }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)/*In SIP6 cascade ports also drained */||
            (GT_FALSE == portMapShadowPtr->portMap.trafficManagerEn &&
            (GT_FALSE == portMapShadowPtr->portMap.isExtendedCascadePort))) /* if port passed through TM or has no TxQ , dont't check */
        {
            for(timeout = timeOutIterations; timeout > 0; timeout--)
            {
                do
                {
                    prevPortTxqDescNum = portTxqDescNum;
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
                    {
                        rc = cpssDxChPortTxDescNumberGet(devNum, portNum, &portTxqDescNum);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                    else
                    { /*SIP 6 does not support descriptors,only buffers*/
                        portTxqBufNum = 0;
                        rc = prvCpssFalconTxqUtilsPortTxBufNumberGet(devNum, portNum, &portTxqBufNum);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                        portTxqDescNum = (GT_U16)portTxqBufNum;

                        if(timeout < 50 && cpssDeviceRunCheck_onEmulator())
                        {
                            if(timeout<=3)
                            {
                                cpssOsPrintf("portTxqBufNum[%d] \n",portTxqBufNum);
                            }
                            cpssOsTimerWkAfter(100);
                        }

                    }
                    if(0 == portTxqDescNum)
                    {
                        break;
                    }
                    else
                    {
                        if (cpssDeviceRunCheck_onEmulator())
                        {
                            cpssOsTimerWkAfter(PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_txq_polling_interval));
                        }
                        else
                        {
                            PRV_CPSS_PORT_DOWN_HW_WAIT_TO(devNum);
                        }
                    }
                } while (portTxqDescNum < prevPortTxqDescNum);
                if(0 == portTxqDescNum)
                {
                    break;
                }
            }

            if(0 == timeout)
            {
                if(cpssDeviceRunCheck_onEmulator())
                {   /* print */
                    cpssOsPrintf(
                    "ERROR prvCpssDxChPortDisableWait GT_TIMEOUT : [%s] The TXQ failed to drain on portNum[%d] , portTxqDescNum[%d] \n",
                    (isValidPortInterfaceMode ? "valid speed" : "non valid speed"),
                    portNum,portTxqDescNum);
                }

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
                    "[%s] The TXQ failed to drain on portNum[%d] , portTxqDescNum[%d]",
                    (isValidPortInterfaceMode ? "valid speed" : "non valid speed"),
                    portNum,portTxqDescNum);
            }
        }

        if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) ||
           PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) ||
           PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            if(isValidPortInterfaceMode
                  && (portMapShadowPtr->portMap.trafficManagerEn == GT_FALSE)
              )
            {
                rc = prvCpssDxChCaelumPortRegFieldGet(devNum, portNum,
                                                      PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                                      &fifoThresholdCounters);
                if(rc != GT_OK)
                {
                    return rc;
                }

                for(timeout = 100; timeout > 0; timeout--)
                {
                    rc = prvCpssDxChCaelumPortRegFieldGet(devNum, portNum,
                                                          PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E,
                                                          &txdmaPortCreditsCounters);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
#ifdef ASIC_SIMULATION /* just for simulation to ensure we don't trapped into endless loop */
                    txdmaPortCreditsCounters = fifoThresholdCounters;
#endif
                    if(fifoThresholdCounters == txdmaPortCreditsCounters)
                    {
                        break;
                    }
                    else
                    {
                       PRV_CPSS_PORT_DOWN_HW_WAIT_TO(devNum);
                    }
                }

                if(0 == timeout)
                {
                    cpssOsPrintf("prvCpssDxChPortEnableSet[port=%d]: TIMEOUT error -> FIFOsThresholdsConfigsSCDMAReg1(%d) != txdmaPortCreditCounter(%d)\r\n", portNum, fifoThresholdCounters, txdmaPortCreditsCounters);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
                        "The TXQ/TXDMA credits are not synch on portNum[%d]",
                        portNum);
                }
            }
        }


    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) /*TxFifo and TxDMA status polling is  not supported for SIP6*/
    {   /* if port passed through TM, dont't check scdmaTxFifoCounters */
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if((GT_FALSE == portMapShadowPtr->portMap.trafficManagerEn) &&
           (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E))
        {
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum,
                                                               PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E,
                                                               &portTxdmaNum);
            if(GT_OK != rc)
            {
                return rc;
            }
            regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMADebug.
                informativeDebug.SCDMAStatusReg1[portTxdmaNum];
            for(timeout = 100; timeout > 0; timeout--)
            {
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0,
                                                     20, &scdmaTxFifoCounters);
                if(rc != GT_OK)
                {
                    return rc;
                }
#ifdef GM_USED
                /* the registers don't exist in GM ... and return '0xbadad' */
                scdmaTxFifoCounters = 0;
#endif /*GM_USED*/
                if(0 == scdmaTxFifoCounters)
                    break;
                else
                   PRV_CPSS_PORT_DOWN_HW_WAIT_TO(devNum);
            }

            if(0 == timeout)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
                    "[%s] The TxDma fifo counters are not ZERO on portNum[%d]",
                    (isValidPortInterfaceMode ? "valid speed" : "non valid speed"),
                    portNum);
            }
        }
    }
    else
    {

        /*********************************************/
        /* 2.g wait 1us for the TXQ-MAC path to drain */
        /*********************************************/
#ifndef ASIC_SIMULATION
        if(cpssDeviceRunCheck_onEmulator())
        {
            /* allow the packet after the TXQ to do : txdma,txfifo,D2D ... and then Raven */
            cpssOsTimerWkAfter(2*PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_after_txq_drain));/*1000*/
        }
        else
#endif  /*ASIC_SIMULATION*/
        {
            cpssOsTimerWkAfter(1);
        }
    }
    return GT_OK;
}

GT_STATUS mvHwsMpfSauOpenDrainSet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 openDrain
);

GT_STATUS mvHwsMifOpenDrainSet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 openDrain
);

/**
* @internal sip6_prvCpssDxChTxqWaitForDrain function
* @endinternal
*
* @brief   Sip6 : Wait for TXQ to empty.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_prvCpssDxChTxqWaitForDrain
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;

    /* 2.f  and 2g Wait for queues to empty */
    rc = prvCpssDxChPortDisableWait(devNum, portNum);

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)
    {
        /* no WA needed in the Raven only */
        return rc;
    }

    if(rc == GT_OK)
    {
        /*Handle MAC here.Since it may enter power saving mode.CPSS-11474*/

        /*Enable PFC responce at MAC*/
        rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ", portNum);
        }
        /*Enable PFC generation  at MAC. Handle xoff_gen bus towards MAC*/
        rc = prvCpssDxChPortSip6XoffOverrideEnableSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ", portNum);
        }
     }
    else
    {
        (void)prvCpssTxqSip6DebugFailureCountSet(devNum,portNum,PRV_CPSS_TXQ_FAILURE_QUEUE_DRAIN_E,PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_INCREASE_E);
    }

    if(rc == GT_OK && /* the TXQ is empty */
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->doingPortDisableFromPortDelete == GT_TRUE&&
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->waAlreadyDoneArr[portNum] == GT_FALSE)
    {
        /* start the WA for the port that is going down.*/
        rc = prvCpssDxChFalconPortDeleteWa_start(devNum,portNum);

        PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->waAlreadyDoneArr[portNum] = GT_TRUE;
    }

    return rc;
}

/* debug_halt_on_TXQ_stuck: debug flag for the TXQ when is stuck ! ...
   do not change any configuration , we want to generate waver on the Emulator
*/
GT_STATUS debug_halt_on_TXQ_stuck_set_timeout(GT_U32    timeout/*in seconds*/)
{
    PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(debug_halt_on_TXQ_stuck, timeout);
    return GT_OK;
}

/**
* @internal sip6_prvCpssDxChMacFifoTryWaitForDrain function
* @endinternal
*
* @brief   The function try to wait until MAC fifo is empty.
*                In debug mode warnign is printed.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_prvCpssDxChMacFifoTryWaitForDrain
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    GT_U32    i,emptyStatusCount;
    GT_BOOL   txMacFifoIdle = 0;
    GT_BOOL   rxMacFifoIdle = 0;


     for(i=0,emptyStatusCount=0;i<PRV_CPSS_DXCH_SIP6_MAC_DRAIN_ITERATION_NUM_MAC;i++)
     {
         rc = prvCpssDxChPortSip6GopFifoStatusGet(devNum,portNum,&txMacFifoIdle,&rxMacFifoIdle);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6GopFifoStatusGet failed");
         }

         if((GT_TRUE == txMacFifoIdle) && (GT_TRUE == rxMacFifoIdle))
         {   emptyStatusCount++;
             if(emptyStatusCount==PRV_CPSS_DXCH_SIP6_MAC_IDLE_CONSECUTIVE_ITERATION_NUM_MAC)
             {
                 break;
             }
         }
         else
         {
             emptyStatusCount =0;
 #ifndef ASIC_SIMULATION
         if(cpssDeviceRunCheck_onEmulator())
         {
             cpssOsTimerWkAfter(100);
         }

 #endif /*ASIC_SIMULATION*/
         }
     }

     if(i == PRV_CPSS_DXCH_SIP6_MAC_DRAIN_ITERATION_NUM_MAC)
     {
          (void)prvCpssTxqSip6DebugFailureCountSet(devNum,portNum,PRV_CPSS_TXQ_FAILURE_MAC_FIFO_DRAIN_E,PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_INCREASE_E);

          if(cpssDeviceRunCheck_onEmulator()||PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(debug_halt_on_TXQ_stuck))
          {
              if(GT_FALSE==txMacFifoIdle)
              {
              /* print */
              cpssOsPrintf("Warning :MAC TX fifo on portNum[%d] is not empty.\n",
               portNum);
              }

              if(GT_FALSE==rxMacFifoIdle)
              {
              /* print */
               cpssOsPrintf("Warning :MAC RX fifo on portNum[%d] is not empty.\n",
                portNum);
              }

              rc = GT_TIMEOUT;
          }
     }

    return rc;
}


/**
* @internal sip6_prvCpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
* @param[in] portMacNum               - the MAC port number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_prvCpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable,
    IN  GT_U32    portMacNum
)
{
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArrayBr[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_STATUS                       rc;                 /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode;
    CPSS_PORT_SPEED_ENT             cpssSpeed;
    MV_HWS_PORT_STANDARD            hwsIfMode;
    GT_U32                          pfcRespBitMap;
    CPSS_DXCH_PORT_FC_MODE_ENT      pfcGenerationMode;
    GT_U32                          value;
    GT_U32                          macBrRegAddr;            /* register address */


    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }
    if(prvCpssDxChPortMacConfigurationClear(regDataArrayBr) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }

    /* note : some of those registers will hold 'PRV_CPSS_SW_PTR_ENTRY_UNUSED' ... and will be ignored */
    regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr  = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC .commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;

    if(enable)
    {

        /* Unreset PTP before port state is enabled */
        rc = prvCpssDxChPortPtpReset(devNum, portNum, GT_FALSE, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType != CPSS_CHANNEL_PEX_FALCON_Z_E)
        {
            rc = prvCpssFalconTxqUtilsPortEnableSet(devNum, portNum,GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPortEnableSet fail");
            }
        }


        value = BIT_0; /*enableTx(bit 0)*/
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.mtiMacInfo[portMacNum].rxState ==
           PRV_CPSS_DXCH_PORT_MTI_MAC_RX_STATE_FORCE_DISABLED_E)
        {
            /* the application FORCE to keep the Rx as disabled (although the TX
               is about to be enabled) */

            /* this is needed for the 'force link UP' that we achieve by doing :
             1. MAC Loopback -- to force the MAC to operational mode (like , link UP)
             2. disable RX   -- so packets that egress will not be ingress due to loopback
                and also will not be counted in the RX MIB counters !

               if the current function will 'enable Rx' at this point , some packets
               may ingress the port while it is in LB mode as described above.

               so we must not enable the Rx.
             */
        }
        else
        {
            /* the Rx should be enabled as forced by the application or synched
               with the Tx state */
            value |= BIT_1; /*enableRx(bit 1)*/
        }

        regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldData =
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData =
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData =
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;

        regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldOffset =
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset =
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset =
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 0;

        regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldLength =
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength =
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength =
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 2;

        /* config the needed MAC(s) */
        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if(rc != GT_OK)
        {
            return rc;
        }

        /****************/
        /*  BR section  */
        /****************/
        macBrRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_commandConfig;
        if(macBrRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].regAddr = macBrRegAddr;
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldLength = 2;
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;

            rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArrayBr);
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->port.mtiMacInfo[portMacNum].txState = PRV_CPSS_DXCH_PORT_MTI_MAC_TX_STATE_FORCE_ENABLED_E;

        /* restoring Egf force link status so enqueueing could be made
           according to the configuration prior to port disable */
        /* NOTE: unlike sip5.20 : we call next function because it protect us
           from setting 'EGF link UP' before the time is ready ! */
        rc = prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed(devNum,portNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else /* disable the port */
    {
        /* this is 'part 1' of the sequence : EGF go down */
        PRINT_TXQ_DESC(devNum, portNum);
        /* Force EGF to go down ... so the TXQ will not get any more packets */
        /*
            Force link down without modifying the 'DB'
        */
        rc = prvCpssDxChEgfPortLinkFilterForceLinkDown(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "egf filter (force link down) on portNum[%d]",
                portNum);
        }


        PRINT_TXQ_DESC(devNum, portNum);

#ifndef ASIC_SIMULATION
        if(cpssDeviceRunCheck_onEmulator())
        {
            cpssOsTimerWkAfter(100);
        }
        else
#endif /*ASIC_SIMULATION*/
        {
           PRV_CPSS_PORT_DOWN_HW_WAIT_TO(devNum);
        }

        PRINT_TXQ_DESC(devNum, portNum);

        /* this is 'part 2.1' of the sequence : disable Rx so pause will not effect the rate of TX direction */
        regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldData =
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData =
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData =
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = 0;/* disable Rx */

        regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldOffset =
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset =
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset =
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 1;/* Rx(bit 1) */

        regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldLength =
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength =
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength =
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;

        /* config the needed MAC(s) */
        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if(rc != GT_OK)
        {
            return rc;
        }

        macBrRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_commandConfig;
        if(macBrRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].regAddr = macBrRegAddr;
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldData = 0;
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;
            regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 1;

            rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArrayBr);
        }

        PRINT_TXQ_DESC(devNum, portNum);
        cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
        cpssSpeed  = PRV_CPSS_DXCH_PORT_SPEED_MAC (devNum, portMacNum);
        hwsIfMode  = LAST_PORT_MODE;

        if(((CPSS_PORT_INTERFACE_MODE_NA_E != cpssIfMode) && (CPSS_PORT_SPEED_NA_E != cpssSpeed)) &&
            ((CPSS_PORT_INTERFACE_MODE_NA_HCD_E != cpssIfMode) && (CPSS_PORT_SPEED_NA_HCD_E != cpssSpeed)))
        {
            /* get the <hwsIfMode> value */
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode, cpssSpeed, &hwsIfMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssIfMode %d, cpssSpeed %d",cpssIfMode, cpssSpeed);
            }

            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
            {
#if 0
                rc = mvHwsMifOpenDrainSet(devNum,portMacNum,hwsIfMode,GT_TRUE/*open drain*/);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsMifOpenDrainSet : failed to 'open drain' on MAC[%d] (portNum[%d])",
                        portMacNum,portNum);
                }
#endif

                /*
                    Do nothing: performing MIF open drain under traffic may cause to certain
                    packet to be transmitted with CRC error - that is not desirable behaviour.
                    It may be disabled with insignificantly increase of packet transmit time .
                */
            }
            else
            {
                /* this is 'part 2.2' of the sequence : 'open drain' (drop Tx packet from getting to the MAC) in the MPF before the MAC */
                rc = mvHwsMpfSauOpenDrainSet(devNum,portMacNum,hwsIfMode,GT_TRUE/*open drain*/);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsMpfSauOpenDrainSet : failed to 'open drain' on MAC[%d] (portNum[%d])",
                        portMacNum,portNum);
                }
            }
        }

        PRINT_TXQ_DESC(devNum, portNum);

        if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType != CPSS_CHANNEL_PEX_FALCON_Z_E)
        {
            /* Wait for all the descriptors to get to PDS */
#ifndef ASIC_SIMULATION
            if(cpssDeviceRunCheck_onEmulator())
            {
                cpssOsTimerWkAfter(100);
            }
            else
#endif /*ASIC_SIMULATION*/
            {
              PRV_CPSS_PORT_DOWN_HW_WAIT_TO(devNum);
            }

            PRINT_TXQ_DESC(devNum, portNum);
            /* 2.d and 2.e Disable PFC */
            rc = prvCpssFalconTxqUtilsPfcDisable(devNum,portNum,&pfcRespBitMap,&pfcGenerationMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPfcDisable failed");
            }
        }

        PRINT_TXQ_DESC(devNum, portNum);

        /* this is 'part 3' of the sequence : wait for TXQ to be empty */
        rc = sip6_prvCpssDxChTxqWaitForDrain(devNum,portNum);
        /* rc to be returned at end of function */

        PRINT_TXQ_DESC(devNum, portNum);

        if(rc != GT_OK && PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(debug_halt_on_TXQ_stuck))
        {
            GT_U32  numIterations = PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(debug_halt_on_TXQ_stuck);
            cpssOsPrintf("TXQ stuck , wait for [%d] seconds \n",numIterations);
            /* the TXQ is stuck ! ... do not change any configuration , we want to generate waver on the Emulator */
            while(numIterations--)
            {
                cpssOsTimerWkAfter(1000);
                cpssOsPrintf(".");
            }
            cpssOsPrintf("\n continue with the configurations \n");
        }

        /* let MAC and PCS to finish transaction */
        if (cpssDeviceRunCheck_onEmulator())
        {
            cpssOsTimerWkAfter(PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_after_txq_drain));
        }

        /*
         Try to wait until MAC fifo is empty.There maybe a  case that MAC fifo stay full so do not check return code.
         In case of link down and MAC fifo is not empty there is  "open drain" mechanism that does not empty mac fifo.
         There is a bypass of MAC fifo in link down case.
         Since we do not know if port disabled due to speed change or due to link  down we will not fail  port disabling.
         Still it is better solution then polling since it will take less time in success oriented scenario.
        */
        (void)sip6_prvCpssDxChMacFifoTryWaitForDrain(devNum,portNum);

        if(rc == GT_OK /* TXQ drained */)
        {
            /* 'Tx disable'MUST be done only after TXQ is empty (otherwise TXQ will never get empty under FWS) */
            /* this is 'part 4.1' of the sequence : disable Tx , because the TXQ is already empty */
            regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldData =
            regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData =
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData =
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = 0;/* disable Tx */

            regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldOffset =
            regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset =
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset =
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 0;/* Tx(bit 0) */

            regDataArray[PRV_CPSS_PORT_MTI_100_E ].fieldLength =
            regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength =
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength =
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;

            /* config the needed MAC(s) */
            (void) prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);

            /****************/
            /*  BR section  */
            /****************/
            if(macBrRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].regAddr = macBrRegAddr;
                regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldData = 0;
                regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;
                regDataArrayBr[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;

                rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArrayBr);
            }

            PRV_CPSS_DXCH_PP_MAC(devNum)->port.mtiMacInfo[portMacNum].txState = PRV_CPSS_DXCH_PORT_MTI_MAC_TX_STATE_FORCE_DISABLED_E;
        }

        /* let MAC and PCS to finish transaction */
        if (cpssDeviceRunCheck_onEmulator())
        {
            cpssOsTimerWkAfter(PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_after_mac_disabled));
        }

        if(hwsIfMode  != LAST_PORT_MODE)
        {

            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
            {
#if 0
                (void) mvHwsMifOpenDrainSet(devNum,portMacNum,hwsIfMode,GT_FALSE/*disable the open drain*/);
#endif
            }
            else
            {
                /* this is 'part 4.2' of the sequence : disable the 'open drain' */
                (void) mvHwsMpfSauOpenDrainSet(devNum,portMacNum,hwsIfMode,GT_FALSE/*disable the open drain*/);
            }
        }

        if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType != CPSS_CHANNEL_PEX_FALCON_Z_E)
        {
            (void) prvCpssFalconTxqUtilsPortEnableSet(devNum, portNum,GT_FALSE);

            /*2.h and 2.i Re -Enable PFC */
            (void) prvCpssFalconTxqUtilsPfcEnable(devNum,portNum,&pfcRespBitMap,&pfcGenerationMode);
        }
        /* Reset PTP unit after port state is disabled */
        (void)prvCpssDxChPortPtpReset(devNum, portNum, GT_TRUE, GT_TRUE);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regAddr1;   /* register address */
    GT_U32      value;      /* value to write */
    GT_U32      value1;     /* value to write */
    GT_STATUS   rc;         /* return code */
    GT_STATUS   rc1 = GT_OK;    /* alternative return code */
    GT_U32      portMacNum; /* MAC number */
    GT_U8       tcQueue;
    GT_BOOL     isPortTxShaperEnable = GT_FALSE;
    GT_BOOL     isPortTxQShaperEnable[CPSS_TC_RANGE_CNS] = {GT_FALSE};
    GT_BOOL     isAnyPortShaperEnable = GT_FALSE;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portEgfLinkStatusState;
    CPSS_PORT_MAC_TYPE_ENT                     portMacType;
    GT_BOOL targetEnable = enable;
    GT_BOOL     tcQueueEnable;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(CAST_SW_DEVNUM(devNum),portMacNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* do sip6 clean and explicit logic */
        return sip6_prvCpssDxChPortEnableSet(devNum,portNum,enable,portMacNum);
    }
   /* Don't apply WA on CPU port in SDMA mode */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_XCAT_GE_PORT_DISABLE_WA_E) &&
       ((portNum != CPSS_CPU_PORT_NUM_CNS) || ((portNum == CPSS_CPU_PORT_NUM_CNS) &&
        (PRV_CPSS_PP_MAC(devNum)->cpuPortMode != CPSS_NET_CPU_PORT_MODE_SDMA_E)) ||
        ((portNum == CPSS_CPU_PORT_NUM_CNS) && PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))))
    {
        for(tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            if(GT_FALSE == targetEnable)
            {
                tcQueueEnable = GT_FALSE;
            }
            else
            {
                /* restore queueing of packets on all TC queues */
                tcQueueEnable = CPSS_PORTS_BMP_IS_PORT_SET_MAC((&(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                        info_PRV_CPSS_DXCH_XCAT_GE_PORT_DISABLE_WA_E.portTxQueuesBmpPtr[tcQueue])),portNum);
                tcQueueEnable = BIT2BOOL_MAC(tcQueueEnable);
            }

            if((rc = prvCpssDxChPortTxQueueingEnableSet(devNum,portNum,tcQueue,tcQueueEnable)) != GT_OK)
                return rc;
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(enable)
        {/* Unreset PTP before port state is enabled */
            rc = prvCpssDxChPortPtpReset(devNum, portNum, GT_FALSE, GT_FALSE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* in sip_5_20 when the mac is disabled, the TxQ enqueing does not automatically stop, and descriptors
       can still be inserted to TxQ. In this case we need to stop enqueing. Note: the functionality of
       Egf API is similar to cpssDxChPortTxQueueingEnableSet API, both stop the enqueing, we use the Egf
       as it operate on a port while the latter API needs to be perform on all of the ports TxQ's */
    if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) )
    {
        if (enable == GT_FALSE)
        {
            /* 2. set CG Flush configuration */
            if (portMacType == CPSS_PORT_MAC_TYPE_CG_E)
            {
                regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMacNum).CGPORTMACCommandConfig;
                if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                {
                     rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 22, 1, 1);
                     if (rc != GT_OK)
                     {
                         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "CG flush failed=%d ", rc);
                     }
                }
            }

            /* overriding Egf status */
            portEgfLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                PRV_CPSS_DXCH_EGF_CONVERT_MAC(portEgfLinkStatusState));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "egf filter (force link down) failed=%d ", rc);
            }
        }
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    value = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 20;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /************************************************************************************/
    /* Additional configuration should be done for CG MAC in order to disable/enable it */
    /************************************************************************************/

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }

    if((!enable) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        /* Check which port shapers are enabled */
        isAnyPortShaperEnable = GT_FALSE;
        rc1 = cpssDxChPortTxShaperEnableGet(devNum,portNum, &isPortTxShaperEnable);
        isAnyPortShaperEnable = isAnyPortShaperEnable ||
                                ((rc1 == GT_OK) && isPortTxShaperEnable);
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            rc1 = cpssDxChPortTxQShaperEnableGet(devNum,portNum,tcQueue,
                                                &isPortTxQShaperEnable[tcQueue]);
            isAnyPortShaperEnable = isAnyPortShaperEnable ||
                                ((rc1 == GT_OK) && isPortTxQShaperEnable[tcQueue]);
        }

        /* Disable port and per-TC shapers */
        if (isAnyPortShaperEnable)
        {
            rc1 = cpssDxChTxPortShapersDisable(devNum,portNum);
            /* Ignore rc1 */
        }
    }

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        PRV_CPSS_DXCH_REG1_CG_CONVERTERS_RESETS_REG_MAC(devNum, portMacNum,&regAddr1);

        mvHwsCgMac28nmAccessLock(devNum, portMacNum);

        /* save HW CG unit reset value */
        rc = prvCpssDrvHwPpGetRegField(devNum,regAddr1, 26, 1, &value1);
        if(rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            return rc;
        }
        if(value1 == 0)
        {
            rc = prvCpssDrvHwPpSetRegField(devNum,regAddr1, 26, 1, 1);
            if(rc != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                return rc;
            }
        }

        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value | (value << 1);
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 0;
        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if(rc != GT_OK)
        {
            mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
            return rc;
        }
        /* restore previous HW state */
        if(value1 == 0)
        {
            rc = prvCpssDrvHwPpSetRegField(devNum,regAddr1, 26, 1, value1);
            if(rc != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                return rc;
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    }

    if((!enable) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        /* 2.f  and 2g Wait for queues to empty */
        rc = prvCpssDxChPortDisableWait(devNum, portNum);
        /* rc to be returned at end of function */

        /* Re-enable port and per-TC shapers - independent of rc/rc1 */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            if (isPortTxQShaperEnable[tcQueue])
            {
                rc1 = cpssDxChPortTxQShaperEnableSet(devNum,portNum,tcQueue,GT_TRUE);
                /* Ignore rc1 */
            }
        }
        if (isPortTxShaperEnable)
        {
            rc1 = cpssDxChPortTxShaperEnableSet(devNum,portNum,GT_TRUE);
            /* Ignore rc1 */
        }
    }

    /* restoring Egf force link status so enqueueing could be made
       according to the configuration prior to port disable */
    if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) )
    {
        if (enable == GT_TRUE)
        {
            if (PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "portEgfForceStatusBitmapPtr is NULL");
            }
            /* fetching Egf status from db */
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum))
            {
                portEgfLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
            }
            else
            {
                portEgfLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;
            }

            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                PRV_CPSS_DXCH_EGF_CONVERT_MAC(portEgfLinkStatusState));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "egf filter (force %s) failed=%d ",
                        (portEgfLinkStatusState == CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E) ?
                                              "link_up" : "link_down", rc);
            }
        }

        if((portMacType == CPSS_PORT_MAC_TYPE_CG_E) && (enable == GT_FALSE))
        {
            regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMacNum).CGPORTMACCommandConfig;
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                 rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 22, 1, 0);
                 if (rc != GT_OK)
                 {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "CG unflush failed %d",rc);
                 }
            }
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(!enable)
        {/* Reset PTP unit after port state is disabled */
            rc1 = prvCpssDxChPortPtpReset(devNum, portNum, GT_TRUE, GT_TRUE);
        }
    }

    if (rc != GT_OK)
    {
        return rc;
    }
    else
    {
        return rc1;
    }

}

/**
* @internal internal_original_cpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS internal_original_cpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;
    GT_U32 portMacNum; /* MAC number */

    GT_BOOL targetEnable = enable;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /*check that port does not serve as additional preeemption  channel*/
    rc = prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet(devNum,portNum);
    if(rc!=GT_OK)
    {
        return rc;
    }

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortEnableSetFunc(devNum,portNum,
                                  targetEnable, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* switch MAC port enable configuration */
    if (doPpMacConfig == GT_TRUE)
    {

        rc = prvCpssDxChPortEnableSet(devNum,portNum,targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                        info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.enabled == GT_TRUE)
        {
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
               info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.portEnableBmpPtr,portMacNum,targetEnable);
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortEnableSetFunc(devNum,portNum,
                                  targetEnable,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal internal_cpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; AC5; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS               rc = GT_OK,rcTimeMeassure;
    GT_U32                  portMacNum;
    CPSS_PORTS_BMP_STC      *xlgUniDirPortsBmpPtr;
    GT_U32                  xlgUniDirPort;
    GT_BOOL                 loopedBack;
    GT_BOOL                 hwUnidirectional;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);


    rcTimeMeassure = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    if(rcTimeMeassure != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rcTimeMeassure, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)== GT_TRUE)
    {
        if(prvCpssDxChPortRemotePortCheck(devNum,portNum)== GT_TRUE)
        {
            if(enable == GT_FALSE)
            {
               rc = prvCpssFalconTxqUtilsRemotePortFlush(devNum,portNum);
            }
            return rc;
        }
    }

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    xlgUniDirPortsBmpPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_PORTS_XLG_UNIDIRECTIONAL_WA_E.xlgUniDirPortsBmp);
    xlgUniDirPort = CPSS_PORTS_BMP_IS_PORT_SET_MAC(xlgUniDirPortsBmpPtr, portNum);

    if ((xlgUniDirPort) && (enable != GT_FALSE))
    {
        /* all time when port is enabled it must be XLG unidirectional  */
        /* if it not looped back, otherwise bidirectional               */
        /* XLG unidirectional enabled/disabled before enabling the port */
        rc = cpssDxChPortInternalLoopbackEnableGet(
            devNum, portNum, &loopedBack);
        if (rc != GT_OK)
        {
            return rc;
        }

        hwUnidirectional = ((loopedBack == GT_FALSE) ? GT_TRUE : GT_FALSE);
        rc = prvCpssDxChPortXlgUnidirectionalEnableSet(
            devNum, portNum, hwUnidirectional);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = internal_original_cpssDxChPortEnableSet(devNum, portNum, enable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((xlgUniDirPort) && (enable == GT_FALSE))
    {
        /* XLG unidirectional disabled after disabling the port */
        /* regardless loopback state                            */
        rc = prvCpssDxChPortXlgUnidirectionalEnableSet(
            devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rcTimeMeassure = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rcTimeMeassure != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rcTimeMeassure, LOG_ERROR_NO_MSG);
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }

    OS_PORT_ENABLE_PRINTF(("Port enable = %d (cpssDxChPortEnableSet) finished after: %d sec., %d nanosec.\n",enable,seconds, nanoSec));

    return GT_OK;
}

/**
* @internal cpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL   *statePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.enabled == GT_TRUE)
    {
        *statePtr = CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.portEnableBmpPtr,portMacNum);
        return GT_OK;
    }

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    /* store value of port state */
    if(macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, 0, 1, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, 0, 1, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, 0, 1, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, 0, 1, &value);
    }
    else if (macType == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, portMacNum, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, 20, 1, &value);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, macType, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, 0, 1, &value);
    }
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortEnableGetFunc(devNum,portNum,
                                  statePtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get port enable status from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortEnableGet(devNum,portNum,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortEnableGetFunc(devNum,portNum,
                                  statePtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* prvCpssDxChPortDuplexModeSet
*
* DESCRIPTION:
*       Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       dMode    - duplex mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or dMode
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported duplex mode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#define PRV_CPSS_DXCH_PORT_MAC_DUPLEX_FLD_LEN_CNS   1
#define PRV_CPSS_DXCH_PORT_MAC_DUPLEX_FLD_OFFSET_GE_CNS   12
static GT_STATUS prvCpssDxChPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32 duplexModeRegAddr; /* config. reg. address */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* mac type to use */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    if (dMode != CPSS_PORT_FULL_DUPLEX_E &&
        dMode != CPSS_PORT_HALF_DUPLEX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* in devices of Lion family half-duplex not supported in any port mode/speed */
    if (dMode == CPSS_PORT_HALF_DUPLEX_E)
    {
        if ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
             (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum)) /* BC2 A0 */           ||
             (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
           )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if((GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported)
            || (portNum != CPSS_CPU_PORT_NUM_CNS))
        {/* check if auto-neg. supported for current mode/port*/
            if (dMode == CPSS_PORT_HALF_DUPLEX_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&duplexModeRegAddr);
    if(duplexModeRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);


    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = duplexModeRegAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = (dMode == CPSS_PORT_FULL_DUPLEX_E) ? 1 : 0;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = PRV_CPSS_DXCH_PORT_MAC_DUPLEX_FLD_OFFSET_GE_CNS;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = PRV_CPSS_DXCH_PORT_MAC_DUPLEX_FLD_LEN_CNS;

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {/* restore port enable state */
        rc = prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] dMode                    - duplex mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
{
    CPSS_PORT_DUPLEX_ENT  targetdMode = dMode;   /* mode value - set by callback */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (dMode != CPSS_PORT_FULL_DUPLEX_E &&
        dMode != CPSS_PORT_HALF_DUPLEX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        CPSS_PORT_DUPLEX_ENT   currentState;
        rc = cpssDxChPortDuplexModeGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(dMode == currentState)
        {
            return GT_OK;
        }
    }

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);
    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacDuplexSetFunc(devNum,portNum,
                                  targetdMode, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetdMode);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* switch MAC configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortDuplexModeSet(devNum,portNum,targetdMode);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run */
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacDuplexSetFunc(devNum,portNum,
                                  targetdMode,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetdMode);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] dMode                    - duplex mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDuplexModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, dMode));

    rc = internal_cpssDxChPortDuplexModeSet(devNum, portNum, dMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, dMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
{
    GT_U32 value;
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 duplexModeRegAddr;
    GT_STATUS rc;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(dModePtr);

     /* can not be changed for 1G and above */
    if((portNum != CPSS_CPU_PORT_NUM_CNS) || ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)))
    {
        if((rc = cpssDxChPortSpeedGet(devNum, portNum, &speed)) != GT_OK)
            return rc;

        if((speed > CPSS_PORT_SPEED_1000_E) || PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            *dModePtr = CPSS_PORT_FULL_DUPLEX_E;
            return GT_OK;
        }
    }

    PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portMacNum,&duplexModeRegAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == duplexModeRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    offset = 3;

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                  duplexModeRegAddr,
                                  offset,
                                  1,
                                  &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *dModePtr = (value == 1) ? CPSS_PORT_FULL_DUPLEX_E : CPSS_PORT_HALF_DUPLEX_E;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(dModePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacDuplexGetFunc(devNum,portNum,
                                  dModePtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,dModePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get value from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortDuplexModeGet(devNum,portNum,dModePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacDuplexGetFunc(devNum,portNum,
                                  dModePtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,dModePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDuplexModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, dModePtr));

    rc = internal_cpssDxChPortDuplexModeGet(devNum, portNum, dModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, dModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the duplex auto negotiation
*
*/
static GT_STATUS prvCpssDxChPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32 dupledAutoNegRegAddr; /* address of reg. for duplex auto-neg. config. */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* in devices of Lion family half-duplex not supported in any port mode/speed,
        so disable duplex auto-neg. to prevent unpredictable behaviour */
    if (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)) &&
        (state == GT_TRUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
#if 0
    if((portMacType >= PRV_CPSS_PORT_XG_E) ||
        ((portNum == CPSS_CPU_PORT_NUM_CNS) &&
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)))
    {/* check if auto-neg. supported for current mode/port*/
        if (state == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
#endif
    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&dupledAutoNegRegAddr);
    if(dupledAutoNegRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = dupledAutoNegRegAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(state);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 13;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* restore port enable state */
        rc = prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the duplex auto negotiation
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;
    GT_BOOL targetPortState = state;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_BOOL   currentState;

        rc = cpssDxChPortDuplexAutoNegEnableGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback  */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacDuplexANSetFunc(devNum,portNum,
                                  targetPortState,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetPortState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* if required run prvCpssDxChPortDuplexAutoNegEnableSet -
       switch MAC configuration code */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortDuplexAutoNegEnableSet(devNum,portNum,targetPortState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

       /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacDuplexANSetFunc(devNum,portNum,
                                  targetPortState,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetPortState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the duplex auto negotiation
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDuplexAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the duplex auto negotiation
*
*/
static GT_STATUS prvCpssDxChPortDuplexAutoNegEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *statePtr
)
{
    GT_U32 value;       /* value of duplex auto-neg. status */
    GT_U32 portGroupId; /* number of port group for mlti-port group dev's */
    GT_U32 dupledAutoNegRegAddr; /* address of duplex auto-neg. config. reg. */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* can not be changed for 10 Gb interface or CPU port */
    if((portNum == CPSS_CPU_PORT_NUM_CNS)&&(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE))
    {
        *statePtr = GT_FALSE;
        return GT_OK;
    }

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&dupledAutoNegRegAddr);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                             portMacNum);
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, dupledAutoNegRegAddr,
                                            13, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. CPU port not supports the duplex auto negotiation
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortDuplexAutoNegEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *statePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port callback pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;                       /* do switch mac code indicator */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacDuplexANGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
           /*
     *  the function returns the switch MAC value if callback not bound or callback set
     *  doPpMacConfig = GT_TRUE; in this case prvCpssDxChPortDuplexAutoNegEnableGet
     *  will run
     */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortDuplexAutoNegEnableGet(devNum,portNum,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* post callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacDuplexANGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. CPU port not supports the duplex auto negotiation
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexAutoNegEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDuplexAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortDuplexAutoNegEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
static GT_STATUS prvCpssDxChPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 stateValue;      /* value to write into the register */
    GT_U32 pauseAdvValue;   /* value to write into the register */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32 fcAutoNegRegAddr;/* config. reg. address */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32          portMacNum;      /* MAC number */
    GT_U32 fcaRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;         /* register address */
    GT_U32 fcaValue = 0;    /* value to write into the FCA register */

    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    if((portMacType >= PRV_CPSS_PORT_XG_E) ||
        ((portNum == CPSS_CPU_PORT_NUM_CNS) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)))
    {/* check if auto-neg. supported for current mode/port*/
        if ((state == GT_TRUE) || (pauseAdvertise == GT_TRUE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&fcAutoNegRegAddr);
    if(fcAutoNegRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    stateValue = BOOL2BIT_MAC(state);
    pauseAdvValue = BOOL2BIT_MAC(pauseAdvertise);
    if(fcAutoNegRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = fcAutoNegRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = stateValue;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 11;              /* AnFCEn Enables Auto-Negotiation for Flow */
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fcAutoNegRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = fcAutoNegRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = pauseAdvValue;           /* PauseAdv This is the Flow Control advertise  */
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 9;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {
        /* restore port enable state */
        rc = prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            fcaRegAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            fcaRegAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }
        /* <FCA <enable/disable> bit configuration */
        if((GT_TRUE == state) && (GT_TRUE == pauseAdvertise))
        {
            fcaValue = 0;
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                    fcaRegAddr, 1, 1, fcaValue) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the flow control auto negotiation
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
)
{
    GT_BOOL   targetState = state;
    GT_BOOL   targetPauseAdvertise = pauseAdvertise;

    CPSS_MACDRV_OBJ_STC * portMacObjPtr;      /* port PHYMAC object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;          /* do switch mac code indicator */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacFlowCntlANSetFunc(devNum,portNum,
                                     targetState,targetPauseAdvertise,
                                     CPSS_MACDRV_STAGE_PRE_E,
                                     &doPpMacConfig,&targetState,
                                     &targetPauseAdvertise);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

         /* run prvCpssDxChPortDuplexAutoNegEnableGet - switch MAC configuration code */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortFlowCntrlAutoNegEnableSet(devNum,portNum,targetState,
                                     targetPauseAdvertise);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacFlowCntlANSetFunc(devNum,portNum,
                                  state,pauseAdvertise,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetState,
                                  &targetPauseAdvertise);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the flow control auto negotiation
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowCntrlAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state, pauseAdvertise));

    rc = internal_cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, portNum, state, pauseAdvertise);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state, pauseAdvertise));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
static GT_STATUS prvCpssDxChPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
)
{
    GT_U32 value;   /* current status */
    GT_U32 fcAutoNegRegAddr;    /* config. reg. address */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum);

    if((PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E) ||
        (portNum == CPSS_CPU_PORT_NUM_CNS))
    {
        *statePtr = GT_FALSE;
        *pauseAdvertisePtr = GT_FALSE;
        return GT_OK;
    }

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&fcAutoNegRegAddr);
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, fcAutoNegRegAddr,
                                            11, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, fcAutoNegRegAddr,
                                            9, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *pauseAdvertisePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
static GT_STATUS internal_cpssDxChPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;       /* port PHYMAC object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;           /* do switch mac code indicator */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    CPSS_NULL_PTR_CHECK_MAC(pauseAdvertisePtr);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacFlowCntlANGetFunc(devNum,portNum,
                                  statePtr,pauseAdvertisePtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,statePtr,
                                  pauseAdvertisePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /*   if switch MAC value should be returned prvCpssDxChPortFlowCntrlAutoNegEnableGet
     *   will run
     */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortFlowCntrlAutoNegEnableGet(devNum,portNum,statePtr,
                                                   pauseAdvertisePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post sage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacFlowCntlANGetFunc(devNum,portNum,
                                                    statePtr,pauseAdvertisePtr,
                                                    CPSS_MACDRV_STAGE_POST_E,
                                                    &doPpMacConfig,statePtr,
                                                    pauseAdvertisePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowCntrlAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr, pauseAdvertisePtr));

    rc = internal_cpssDxChPortFlowCntrlAutoNegEnableGet(devNum, portNum, statePtr, pauseAdvertisePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr, pauseAdvertisePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal prvCpssDxChPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the speed auto negotiation
*
*/
static GT_STATUS prvCpssDxChPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32 speedAutoNegRegAddr; /* address of speed auto-neg. config. reg. */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port mac unit to use */
    GT_U32          portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
#if 0
    if((portMacType >= PRV_CPSS_PORT_XG_E) ||
        ((portNum == CPSS_CPU_PORT_NUM_CNS) &&
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)))
    {/* check if auto-neg. supported for current mode/port*/
        if (state == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
#endif

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&speedAutoNegRegAddr);
    if(speedAutoNegRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(speedAutoNegRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = speedAutoNegRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(state);
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 7;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* restore port enable state */
        rc = prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2.CPU port not supports the speed auto negotiation
*
*/
static GT_STATUS internal_cpssDxChPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state
)
{
    GT_BOOL targetState = state;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;            /* do switch mac code indicator */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_BOOL   currentState;

        rc = cpssDxChPortSpeedAutoNegEnableGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacSpeedANSetFunc(devNum,portNum,
                                  targetState,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set switch MAC configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortSpeedAutoNegEnableSet(devNum,portNum,state);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacSpeedANSetFunc(devNum,portNum,
                                  targetState,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2.CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssDxChPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSpeedAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the speed auto negotiation
*
*/
static GT_STATUS prvCpssDxChPortSpeedAutoNegEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL  *statePtr
)
{
    GT_U32 value;               /* current speed auto-neg. status */
    GT_U32 speedAutoNegRegAddr; /* speed auto-neg. reg. address */
    GT_U32 portGroupId;         /* port group num. for multi-port group dev's */
    GT_U32 portMacNum;          /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if((portNum == CPSS_CPU_PORT_NUM_CNS)&&(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE))
    {
        *statePtr = GT_FALSE;
        return GT_OK;
    }

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&speedAutoNegRegAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == speedAutoNegRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum);
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, speedAutoNegRegAddr,
                                            7, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. CPU port not supports the speed auto negotiation
*
*/
static GT_STATUS internal_cpssDxChPortSpeedAutoNegEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL  *statePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port callback pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;                       /* do switch mac code indicator */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* MACPHY callback run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacSpeedANGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get the value from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortSpeedAutoNegEnableGet(devNum,portNum,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacSpeedANGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssDxChPortSpeedAutoNegEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL  *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSpeedAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortSpeedAutoNegEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal prvCpssDxChPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 regAddr;         /* register address */
    GT_U32 macRegAddr;      /* register address */
    GT_U32 fcaRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 fcaValue = 0;    /* value to write into the FCA register */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 length;          /* number of bits to be written to the register */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {
        /* disable port if current interface is FE or GE */
        if (prvCpssDxChPortStateDisableAndGet(devNum,portNum, &portStateStc) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON /* definition for FC configuration not ready yet */
        return GT_OK;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            fcaRegAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            fcaRegAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }

        /* FCA unit handles TX FCA. '1' is Bypass and means disable TX FC. '0' means enable TX FCA. */
        switch (state)
        {
            case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
                fcaValue = 1;
                break;
            case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
                fcaValue = 0;
                break;
            case CPSS_PORT_FLOW_CONTROL_RX_ONLY_E:
                fcaValue = 1;
                break;
            case CPSS_PORT_FLOW_CONTROL_TX_ONLY_E:
                fcaValue = 0;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* <FCA TX disable> bit configuration */
        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                fcaRegAddr, 1, 1, fcaValue) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
        offset = 3;
        length = 2;

        switch (state)
        {
            case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
                value = 0;
                break;
            case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
                value = 3;
                break;
            case CPSS_PORT_FLOW_CONTROL_RX_ONLY_E:
                value = 1;
                break;
            case CPSS_PORT_FLOW_CONTROL_TX_ONLY_E:
                value = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
        {
            value &= 0x1; /* For BC2-A0 only RX is configured via MAC - TX is controlled by FCA unit */
                          /* TX_FN_EN is disabled as part of CPSS_DXCH_IMPLEMENT_WA_TRI_SPEED_PORT_AN_FC_E */
        }
    }
    else
    {
        PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);

        offset = 8;
        length = 1;

        switch (state)
        {
            case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
                value = 0;
                break;
            case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
                value = 1;
                break;
            default:
                if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum)<PRV_CPSS_PORT_XG_E)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                goto xg_configuration;
        }

        /* in addition to AN-register configuration, MAC4 register should be configured - bit_3 txq_FC_update_en */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&macRegAddr);
            if(macRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regDataArray[PRV_CPSS_PORT_GE_E].regAddr = macRegAddr;
                regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
                regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 3;
                regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
            }

            rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = offset;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = length;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

xg_configuration:
    switch (state)
    {
        case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
            value = 0;
            break;
        case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
            value = 3;
            break;
        case CPSS_PORT_FLOW_CONTROL_RX_ONLY_E:
            value = 1;
            break;
        case CPSS_PORT_FLOW_CONTROL_TX_ONLY_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = 7;

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        value &= 0x1; /* For BC2 and XCAT3 only for XLG ports RX is configured via MAC - TX is controlled by FCA unit */
    }

    length = 2;

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = length;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = length;
    }

    regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 15;/*<Enable 802.3x FC Tx halting>*/
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {
        /* restore port enable state */
        if (prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
{
    CPSS_PORT_FLOW_CONTROL_ENT  targetState = state;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port PHYMAC object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacFlowCntlSetFunc(devNum,portNum,
                                  state,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* switch MAC configuration*/
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortFlowControlEnableSet(devNum,portNum,targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

       /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacFlowCntlSetFunc(devNum,portNum,
                                  targetState,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Before calling cpssDxChPortFlowControlEnableSet,
*       cpssDxChPortPeriodicFlowControlCounterSet should be called
*       to set the interval between the transmission of two consecutive
*       Flow Control packets according to port speed
*
*/
GT_STATUS cpssDxChPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowControlEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortFlowControlEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
)
{
    GT_U32  value;
    GT_U32  fcaValue = 1;
    GT_U32  regAddr;
    GT_U32  fcaRegAddr;
    GT_U32  fieldOffset;
    GT_U32  fieldLength;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */
    GT_U32  portGroup;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    portGroup = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON /* definition for FC configuration not ready yet */
        *statePtr = CPSS_PORT_FLOW_CONTROL_DISABLE_E;
        return GT_OK;
    }

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            fcaRegAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            fcaRegAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroup, fcaRegAddr, 1,
                                                        1, &fcaValue) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    fieldLength = 2;

    if (portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if(portMacType == PRV_CPSS_PORT_CG_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
            fieldOffset = 15;/*<Forward 802.3x FC Enable>*/
            fieldLength = 1;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            fieldOffset = 7;
        }
    }
    else
    {
        if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            fieldOffset = 3;
            fieldLength = 1;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
            fieldOffset = 4;
        }
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroup, regAddr,
                                    fieldOffset, fieldLength, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        /* combine Rx and Tx values */
        value |= ((~fcaValue & 0x1) << 1);
    }

    switch (value)
    {
        case 0:
            *statePtr = CPSS_PORT_FLOW_CONTROL_DISABLE_E;
            break;
        case 1:
            *statePtr = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E;
            break;
        case 2:
            *statePtr = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E;
            break;
        case 3:
            *statePtr = CPSS_PORT_FLOW_CONTROL_RX_TX_E;
            break;
        default:
            /* no chance getting here */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port PHYMAC object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacFlowCntlGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get vavue from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortFlowControlEnableGet(devNum,portNum,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacFlowCntlGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,statePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowControlEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortFlowControlEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: In Bobcat2, Caelum, Bobcat3, Aldrin, AC3X CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
static GT_STATUS prvCpssDxChPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
)
{
    GT_STATUS   rc; /* return code */
    GT_U32 regAddr; /* register address               */
    GT_U32 regValue=0, valueOn=0,valueOff=0;           /* value to write into the register    */
    GT_U32     portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON /* definition for FC configuration not ready yet */
        return GT_OK;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    switch (enable) {
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E:
        break;
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E:
        valueOn=1;
        valueOff=1;
        break;
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E:
        valueOn=1;
        if((PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E:
        valueOff=1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            regAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }

        regValue = (valueOff << 11) | (valueOn << 7);

        return prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr, (BIT_11 | BIT_7), regValue);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* configure XON */
    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = valueOn;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = valueOn;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset =
            ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
              (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))? 10 : 11;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = valueOn;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset =
            ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
              (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))? 10 : 11;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear reg's data for new configuration */
    rc = prvCpssDxChPortMacConfigurationClear(regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* configure XOFF only on XG/XLG */
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = valueOff;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset =
            ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
              (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))? 10 : 11;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = valueOff;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset =
            ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
              (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))? 10 : 11;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal internal_cpssDxChPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on unsupported request
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: In Bobcat2, Caelum, Bobcat3, Aldrin, AC3X CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
static GT_STATUS internal_cpssDxChPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
)
{
    GT_BOOL targetEnable = enable;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;            /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if ((portMacObjPtr != NULL) &&
        (portMacObjPtr->macDrvMacPeriodFlowCntlSetFunc != NULL))
    {
        rc = portMacObjPtr->macDrvMacPeriodFlowCntlSetFunc(devNum,portNum,
                                  targetEnable,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set switch MAC configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortPeriodicFcEnableSet(devNum,portNum,targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if ((portMacObjPtr != NULL) &&
        (portMacObjPtr->macDrvMacPeriodFlowCntlSetFunc != NULL))
    {
        rc = portMacObjPtr->macDrvMacPeriodFlowCntlSetFunc(devNum,portNum,
                                    targetEnable,CPSS_MACDRV_STAGE_POST_E,
                                    &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on unsupported request
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: In Bobcat2, Caelum, Bobcat3, Aldrin, AC3X CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
GT_STATUS cpssDxChPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPeriodicFcEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
static GT_STATUS prvCpssDxChPortPeriodicFcEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
)
{
    GT_U32 regAddr, regAddrXoff; /* register address       */
    GT_U32 value, valueXoff;     /* value read from register    */
    GT_U32 offset;          /* bit number inside register  */
    GT_STATUS rc;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32     portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON /* definition for FC configuration not ready yet */
        *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E;
        return GT_OK;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            regAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }

        rc = prvCpssDrvHwPpPortGroupReadRegBitMask(devNum, portGroupId, regAddr, (BIT_11 | BIT_7), &value);
        if(GT_OK != rc)
        {
            return rc;
        }

        switch(value)
        {
            case (0):
                *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E;
                break;
            case (BIT_7):
                *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E;
                break;
            case (BIT_11):
                *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E;
                break;
            case (BIT_7 | BIT_11):
                *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        return GT_OK;
    }

   /* Gig has only Xon (Xoff always enabled)*/
    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        offset = 1;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset, 1, &value);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        *enablePtr=(value==1)?CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E:
            CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E;
    }
    /* XG has both xoff and xon*/
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddrXoff);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddrXoff)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            offset = 10;
        }
        else
        {
            offset = 11;
        }

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset, 1, &value);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddrXoff, offset, 1, &valueXoff);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        *enablePtr=(value==0)? (valueXoff==0?CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E:
                                CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E  ):
                               (valueXoff==0?CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E:
                                CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E  );
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
static GT_STATUS internal_cpssDxChPortPeriodicFcEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;    /* port object pointer */
    GT_BOOL doPpMacConfig = GT_TRUE;        /* do switch mac code indicator */
    GT_U32  portMacNum;                     /* MAC number */

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run for remote ports only         */
    /* No need to check external mac for PHY, checking switch side only  */
    if (portMacObjPtr != NULL && prvCpssDxChPortRemotePortCheck(devNum, portNum)) {
        rc = portMacObjPtr->macDrvMacPeriodFlowCntlGetFunc(devNum, portNum,
                                  (GT_BOOL*)enablePtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig, (GT_BOOL*)enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* get value from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortPeriodicFcEnableGet(devNum,portNum,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* "post stage" callback run*/
    if (portMacObjPtr != NULL && prvCpssDxChPortRemotePortCheck(devNum, portNum))  {
        rc = portMacObjPtr->macDrvMacPeriodFlowCntlGetFunc(devNum, portNum,
                                  (GT_BOOL*)enablePtr, CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig, (GT_BOOL*)enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
GT_STATUS cpssDxChPortPeriodicFcEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPeriodicFcEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note Although for XGMII (10 Gbps) ports feature is not supported we let
*       application to configure it, in case GE mac unit exists for this port
*
*/
static GT_STATUS prvCpssDxChPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_U32 backPressureRegAddr; /* register address */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* set BackPressure Enable bit */
    /* Serial Parameters Register is one for 6 ports set */
    PRV_CPSS_DXCH_PORT_SERIAL_PARAM_CTRL_REG_MAC(devNum, portMacNum,
                                                    &backPressureRegAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == backPressureRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = backPressureRegAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(state);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 4;

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal internal_cpssDxChPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
static GT_STATUS internal_cpssDxChPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_BOOL targetState = state;

    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacBackPrSetFunc(devNum,portNum,
                                  targetState,CPSS_MACDRV_STAGE_PRE_E,
                                       &doPpMacConfig,&targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set switch MAC configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortBackPressureEnableSet(devNum,portNum,targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacBackPrSetFunc(devNum,portNum,
                                  targetState,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetState);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssDxChPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortBackPressureEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortBackPressureEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
static GT_STATUS prvCpssDxChPortBackPressureEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_U32 value;           /* value to write into the register */
    GT_U32 regAddr;         /* register address            */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* get BackPressure Enable bit */
    /* Serial Parameters Register is one for 6 ports set */
    PRV_CPSS_DXCH_PORT_SERIAL_PARAM_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacNum),
            regAddr, 4, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
static GT_STATUS internal_cpssDxChPortBackPressureEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *enablePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacBackPrGetFunc(devNum,portNum,
                                  enablePtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get value from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortBackPressureEnableGet(devNum,portNum,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacBackPrGetFunc(devNum,portNum,
                                  enablePtr,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssDxChPortBackPressureEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortBackPressureEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortBackPressureEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *isLinkUpPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 portGroupId;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                                devNum, portMacNum);
    if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ||
        (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
          (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))) &&
                                            (portNum != CPSS_CPU_PORT_NUM_CNS)))
    {
        GT_STATUS   rc; /* return code */
        GT_U32      phyPortNum; /* port number in local core */
        MV_HWS_PORT_STANDARD    portMode;
        CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;
        CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode;
        CPSS_PORT_SPEED_ENT             cpssSpeed;

#ifdef GM_USED
        {
            GT_U32  sim_deviceId;
            /* convert cpss devNum to simulation deviceId */
            rc = getSimDevIdFromSwDevNum(devNum,0,&sim_deviceId);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* get info from the simulation about the GM */
            snetGmPortInfoGet((GT_U8)sim_deviceId,(GT_U8)portMacNum,isLinkUpPtr,NULL);

            /* to avoid :  warning C4702: unreachable code. after the :
                return GT_OK;
            */
            if(rc != GT_ABORTED)
            {
                /* in GM we always get in here */
                return GT_OK;
            }
        }
#endif /*GM_USED*/

        cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
        cpssSpeed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);

        if((CPSS_PORT_INTERFACE_MODE_NA_E == cpssIfMode) ||
           (CPSS_PORT_SPEED_NA_E == cpssSpeed))
        {
            *isLinkUpPtr = GT_FALSE;
            return GT_OK;
        }

        portMode = NON_SUP_MODE;
        comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portMacNum];
        if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,
                                                                        portMacNum);
        }
        else
        {
            phyPortNum = portMacNum;
            portGroupId = 0;
        }

        if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
        { /* if it's combo port */
            CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
            rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
            if(rc != GT_OK)
                return rc;
            if (mac.macNum != phyPortNum)
            {
                switch(cpssIfMode)
                {
                    case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                    case CPSS_PORT_INTERFACE_MODE_KR_E:
                        portMode = _100GBase_KR10;
                        break;
                    case CPSS_PORT_INTERFACE_MODE_SGMII_E:
                        portMode = QSGMII;
                        break;
                    case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                        portMode = _100Base_FX;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }
        }

        if (NON_SUP_MODE == portMode)
        {
            if((CPSS_PORT_INTERFACE_MODE_NA_HCD_E == cpssIfMode) ||
               (CPSS_PORT_SPEED_NA_HCD_E == cpssSpeed))
            {
                *isLinkUpPtr = GT_FALSE;
                return GT_OK;
            }

            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                                        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum),
                                        &portMode);
            if(rc != GT_OK)
                return rc;
        }
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLinkStatusGet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], *isLinkUpPtr)",devNum, portGroupId, phyPortNum, portMode);
        rc = mvHwsPortLinkStatusGet(devNum, portGroupId, phyPortNum, portMode,
                                    isLinkUpPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLinkStatusGet : Hws Failed");
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (*isLinkUpPtr) == GT_FALSE)
        {
            /* the MTI mac although in PCS loopback mode ,
               still not generate interrupt (about link change) and
               state that port is 'down' */
            rc = cpssDxChPortInternalLoopbackEnableGet(devNum, portNum,isLinkUpPtr);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackStatusGet : Hws Failed on port[%d]",portNum);
            }
        }
    }
    else
    {
        PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 1,
                                               &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        *isLinkUpPtr = BIT2BOOL_MAC(value);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *isLinkUpPtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isLinkUpPtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortlinkGetFunc(devNum,portNum,
                                  isLinkUpPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,isLinkUpPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get value from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortLinkStatusGet(devNum,portNum,isLinkUpPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortlinkGetFunc(devNum,portNum,
                                  isLinkUpPtr,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,isLinkUpPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *isLinkUpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLinkStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isLinkUpPtr));

    rc = internal_cpssDxChPortLinkStatusGet(devNum, portNum, isLinkUpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isLinkUpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortForceLinkPassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32  regAddr;    /* register address */
    GT_U32  portMacNum; /* MAC number */
    GT_U32  value;      /* data to write to register */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
#if 0
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* port MAC type */
    GT_U32                  portGroupId; /* port group id */
    GT_U32                  macCtrlReg4Addr;
    GT_U32                  idleCheck;
#endif
    GT_STATUS               rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
#ifndef GM_USED
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
#endif
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortForceLinkPassEnableSetFunc(devNum,portNum,
                                  state, CPSS_MACDRV_STAGE_PRE_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* switch MAC port enable configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
        {
            GT_STATUS   rc;
            GT_BOOL     currentState;

            rc = cpssDxChPortForceLinkPassEnableGet(devNum,portNum,&currentState);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(state == currentState)
            {
                return GT_OK;
            }
        }

        value = BOOL2BIT_MAC(state);

        if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
        }

        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 3;
        }

        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 3;
        }

        PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 27;
        }

        /* Disable the idle_check_for_link bit #14 in XLG-MAC Control_Register4 */
        /* Enable this bit is mandatory for linkDown indication between 10G port to
           2.5G/20G/40G ports, but in Force Link Pass operation it should be disabled */
        /* This Errata implementation cuases the link to not link up under traffic
           so currently the decision is to discard this Errata */
#if 0
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
        if((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) && (portMacType == PRV_CPSS_PORT_XLG_E))
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &macCtrlReg4Addr);
            if (macCtrlReg4Addr !=  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                idleCheck = (state == GT_TRUE) ? 0 : 1;
                if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, macCtrlReg4Addr, 14, 1, idleCheck) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
#endif
        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortForceLinkPassEnableSetFunc(devNum,portNum,
                                  state,CPSS_MACDRV_STAGE_POST_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkPassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForceLinkPassEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortForceLinkPassEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - (ptr to) current force link pass state:
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortForceLinkPassEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 portGroupId;     /*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* mac type of port */
    GT_U32 portMacNum; /* MAC number */
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
#ifndef GM_USED
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
#endif
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortForceLinkPassEnableGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_PRE_E,&doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* get port enable status from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {

        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
        if (portMacType >= PRV_CPSS_PORT_XG_E)
        {
            if (portMacType == PRV_CPSS_PORT_CG_E)
            {
                PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
                offset =27;
            }
            else
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
                offset = 3;
            }
        }
        else
        {
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
            offset = 1;
        }
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* get force link pass bit */
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
        if(prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, offset, 1, &value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        *statePtr = BIT2BOOL_MAC(value);
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortForceLinkPassEnableGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_POST_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - (ptr to) current force link pass state:
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkPassEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForceLinkPassEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortForceLinkPassEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortForceLinkDownEnableSetMac function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*  Note: tx_rem_fault takes precedence over tx_loc_fault. for CG
*  mac, make sure to disable tx_rem_fault, otherwise this
*  function won't work.
*
* @note   APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnableSetMac
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    GT_U32  forceLinkDownRegAddr; /* register address */
    GT_U32  value;                /* value to write into the register */
    GT_U32  portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_STATUS   rc;
        GT_BOOL     currentState;

        rc = cpssDxChPortForceLinkDownEnableGet(devNum,portNum,&currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    /*Avoid overriding ForceLinkDown bit when fastlink is configured by application*/
    /*Applies to BC3 & Aldrin2 devices only*/
    if ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        /*Check if fastlink is configured & avoid overriding ForceLinkDown bit*/
        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].isFastLink)
        {
            return GT_OK;
        }
    }

    value = BOOL2BIT_MAC(state);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E,
                                                        &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 2;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E,
                                                        &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 2;
    }

    /* WA for CG port type, in CG there is no force link down.
    Instead, using Local Fault bit in Control 0 register.
    Note: tx_rem_fault takes precedence over tx_loc_fault, make sure to disable tx_rem_fault to use tx_loc_fault bit.
     */
    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum, &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 0;
    }

    forceLinkDownRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.portControl;
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
    }

    forceLinkDownRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.segPortControl;
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = 0;
    }

    forceLinkDownRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_EXT.portControl;
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 1;
        }
        else
        {
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 4;
        }
    }
    forceLinkDownRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_EXT.portControl;
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 0;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);


        /*
            in order to create local fault and send remote fault to peer, in Aldrin2 need to configure "force_local_fault"
            Port MAC Control Register2 bit<12>.
        */
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E,
                                                             &forceLinkDownRegAddr);
        if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = forceLinkDownRegAddr;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 12;
        }

        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortFaultSendSet function
* @endinternal
*
* @brief  Configure the port to start or stop sending fault
*         signals to partner. When port is configured to send,
*         link on both sides will be down.
*         Note: tx_rem_fault takes precedence over
*         tx_loc_fault. for CG mac, know that using tx_rem_fault
*         will cancel tx_loc_fault effect.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum;
*         Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - start sending fault signals to peer (or stop)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFaultSendSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         send
)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32      portMacNum;      /* MAC number */
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32      localPort;
    CPSS_DXCH_PORT_MAC_PARAMS_STC  mac; /* params of MAC member in combo */
    GT_BOOL                 enable;
    CPSS_DXCH_PORT_COMBO_PARAMS_STC comboParams;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    /* This API currently supported only in SIP_5 and xCat3 devices */
    if (!(PRV_CPSS_SIP_5_CHECK_MAC(devNum)) &&
        !(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
        !(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* Port sanity check and getting port macNum */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get port parameters */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

    /* Sanity check */
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /*Avoid overriding fault signalling when fastlink is configured by application*/
    /*Applies to BC3 & Aldrin2 devices only*/
    if ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        /*Check if fastlink is configured & avoid sending fault signal*/
        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].isFastLink)
        {
            return GT_OK;
        }
    }

    /* Getting HWS supported port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) {
        rc = cpssDxChPortComboModeEnableGet(devNum, portNum, &enable, &comboParams);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(enable)
        {
            if (cpssDxChPortComboPortActiveMacGet(devNum, portMacNum, &mac) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            localPort = mac.macNum;
        }

        if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = 0x088C043C + (localPort * 0x1000);
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = send ? 3 : 0;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 3;
        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);

    } else {

        /* Configure hws to start ot stop sending fault signals */
        rc = hwsPortFaultSendSet(devNum, portGroupId, localPort, portMode, send);
    }

    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
* @param[in] modifyRemoteFaultSend    - whether or not
*                                      to change fault signals
*                                      status which cause
*                                      partner link to go down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL     state,
    IN  GT_BOOL     modifyRemoteFaultSend
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_STATUS   rc;
    GT_U32 portMacNum;  /* MAC number */
    GT_U32 portGroupId;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_BOOL apEnabled = GT_FALSE;
    CPSS_DXCH_PORT_AP_PARAMS_STC apParams;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;/* current interface of port */
    CPSS_PORT_SPEED_ENT     speed;/* current speed of port */
    OUT CPSS_PORT_MAC_TYPE_ENT    macType;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    speed =  PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortForceLinkDownEnableSet with state=%d, modifyRemoteFaultSend=%d",
                                        state, modifyRemoteFaultSend);

    if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        || (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily && portNum != CPSS_CPU_PORT_NUM_CNS)
        || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        /* check if AP is enabled */
        rc = cpssDxChPortApEnableGet(devNum, portGroupId, &apEnabled);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (apEnabled)
        {
            /* check if AP is enabled on specific port */
            rc = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnabled, &apParams);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (apEnabled && (ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This api does not support force link down on an AP port when port is in 1000BASE_X mode");
            }
        }

    }


    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortForceLinkDownSetFunc(devNum,portNum,
                                  state, CPSS_MACDRV_STAGE_PRE_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* switch MAC port enable configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, portNum, state);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if ((PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                 PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
            || (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
               info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.
                            portForceLinkDownBmpPtr,portNum,state);
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortForceLinkDownSetFunc(devNum,portNum,
                                  state,CPSS_MACDRV_STAGE_POST_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }


    if ((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_BOBCAT2_FORCE_DOWN_LINK_PARTNER_WA_E)) &&
        (portNum != CPSS_CPU_PORT_NUM_CNS))
    {
        rc = cpssDxChPortMacTypeGet(devNum,portNum,&macType);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (macType == CPSS_PORT_MAC_TYPE_XLG_E ||
            (macType == CPSS_PORT_MAC_TYPE_XG_E &&
            (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)))
        {
            if (modifyRemoteFaultSend == GT_TRUE && (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E || speed != CPSS_PORT_SPEED_NA_E))
            {
                /* start or stop sending fault signals to partner,
                When port is configured to send signals, link on partner's side will be down */
                rc = prvCpssDxChPortFaultSendSet(devNum, portNum, ifMode, speed, state);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    GT_BOOL modifyRemoteFaultSend;

    /* the default behavior of force link down should be to also force the remote peer
       link to go down */
    modifyRemoteFaultSend = GT_TRUE;

    CPSS_LOG_INFORMATION_MAC("calling prvCpssDxChPortForceLinkDownEnableSet with state=%d, modifyRemoteFaultSend=%d",
                                        state, modifyRemoteFaultSend);
    rc = prvCpssDxChPortForceLinkDownEnableSet(devNum, portNum, state, modifyRemoteFaultSend);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"force link down failed");
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForceLinkDownEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortForceLinkDownEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - (ponter to) current force link down status:
*                                      GT_TRUE - force link down, GT_FALSE - otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortForceLinkDownEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32        forceLinkDownRegAddr; /* register address */
    GT_U32        value;                /* value to write into the register */
    GT_U32        forceLinkDownOffset;  /* bit number inside register       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */
    GT_BOOL doPpMacConfig = GT_TRUE;
    GT_STATUS rc;
    GT_BOOL apEnabled = GT_FALSE;
    CPSS_DXCH_PORT_AP_PARAMS_STC apParams;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
#ifndef GM_USED
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
#endif
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    /* check if port is AP port */
    rc = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnabled, &apParams);
    if ((rc != GT_OK) && (rc != GT_BAD_PARAM) && (rc != GT_NOT_APPLICABLE_DEVICE))
    {
        return rc;
    }

    if (apEnabled) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This api does not support force link down on an AP port");
    }

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* Get the port type */
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortForceLinkDownGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_PRE_E,&doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* get port enable status from switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == tempSystemRecovery_Info.systemRecoveryProcess)
        {/* read from SW DB only if not during some recovery */
            if ((PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                             PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
                || (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))

            {
                *statePtr = CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                   info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.
                                     portForceLinkDownBmpPtr,portNum);

                return GT_OK;
            }
        }

        /* In CG unit we do not have force link down ability */
        if ( portMacType == PRV_CPSS_PORT_CG_E)
        {
            *statePtr = GT_FALSE;
        }
        else
        {
            /* for 10 Gb interface and up */
            if(portMacType >= PRV_CPSS_PORT_XG_E)
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&forceLinkDownRegAddr);
                forceLinkDownOffset = 2;
            }
            else
            {
                PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&forceLinkDownRegAddr);
                forceLinkDownOffset = 0;
            }
            if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == forceLinkDownRegAddr)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

            if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, forceLinkDownRegAddr,
                                                   forceLinkDownOffset, 1, &value) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }

            *statePtr = BIT2BOOL_MAC(value);
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortForceLinkDownGetFunc(devNum,portNum,
                                  statePtr,CPSS_MACDRV_STAGE_POST_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - (ponter to) current force link down status:
*                                      GT_TRUE - force link down, GT_FALSE - otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkDownEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForceLinkDownEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortForceLinkDownEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* prvCpssDxChPortMruSet
*
* DESCRIPTION:
*       Sets the Maximal Receive Packet size for specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number (or CPU port)
*       mruSize    - max receive packet size in bytes. (0..10304) - jumbo frame + 64  (including 4 bytes CRC)
*                    value must be even
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or
*                                   odd value of mruSize
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - mruSize > 10304
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
#define PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CNS 13
#define PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS 16
#define PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_GE_CNS 2
#define PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_XG_CNS 0
#define PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_CG_CNS 0
static GT_STATUS prvCpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 mruSize
)
{
    GT_STATUS   rc;
    GT_U32  valueGranularity1B;
    GT_U32  valueGranularity2B;
    GT_U32  regAddr;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* The resolution of this field is 2 bytes and
     * the default value is 1522 bytes => 0x2f9
     */
    if(mruSize & 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(mruSize > CPSS_DXCH_PORT_MAX_MRU_CNS)
    {
        /* 10KB + 64 max frame size supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* In units prior to CG, the MRU size is in granularity of 2B, so the requested number
     should be devided by two before written to register. In CG unit, the granularity is 1
     so the size should be as requested. */
    valueGranularity2B = mruSize >> 1;
    valueGranularity1B = mruSize;

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E,
                                                                        &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_GE_CNS;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CNS;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = valueGranularity2B;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E,
                                                                        &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_XG_CNS;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CNS;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = valueGranularity2B;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E,
                                                                        &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_XG_CNS;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CNS;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = valueGranularity2B;
    }

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum, portMacNum, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_CG_CNS;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = valueGranularity1B;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.frmLength;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = valueGranularity1B;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.frmLength;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = valueGranularity1B;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.frmLength;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = valueGranularity1B;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.frmLength;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = valueGranularity1B;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 0;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);

    /****************/
    /*  BR section  */
    /****************/
    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_frmLength;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = valueGranularity1B;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);


    return rc;
}

/**
* @internal internal_cpssDxChPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] mruSize                  - max receive packet size in bytes.
*                                      (APPLICABLE RANGES: 0..10304).
*                                      max : jumbo frame(10K) + 64 (including 4 bytes CRC).
*                                      Value must be even.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 mruSize
)
{
    /***********************************************
    * to solve jumbo packet problem:
    *  - set MRU value on PHY MACs as maximum - 0x3FFF
    *  - switch MAC MRU value set as require in API
    ************************************************/
    GT_U32 targetMruSize = 0x3FFF;

    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* true MRU value should be configured on remote ports */
            targetMruSize = mruSize;
        }

        rc = portMacObjPtr->macDrvMacMRUSetFunc(devNum,portNum,
                                  targetMruSize,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  &targetMruSize);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set MRU size value for switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortMruSet(devNum,portNum,mruSize);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacMRUSetFunc(devNum,portNum,
                                  targetMruSize,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,
                                  &targetMruSize);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] mruSize                  - max receive packet size in bytes.
*                                      (APPLICABLE RANGES: 0..10304).
*                                      max : jumbo frame(10K) + 64 (including 4 bytes CRC).
*                                      Value must be even.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 mruSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mruSize));

    rc = internal_cpssDxChPortMruSet(devNum, portNum, mruSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mruSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. (0..10304)
*                                      jumbo frame + 64  (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortMruGet
(
    IN   GT_U8  devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    OUT  GT_U32 *mruSizePtr
)
{
    GT_U32 value;           /* value to read from the register  */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 regAddr;         /* register address                 */
    GT_U32 fieldLength;     /* field length                     */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */
    GT_STATUS rc = GT_OK;
    GT_U32  use_valueGranularity1B = 0;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* cpssDxChPortMruSet updates all available MAC's - here enough to read */
    /* one of them */
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (portMacType == PRV_CPSS_PORT_CG_E)
        {
            PRV_CPSS_DXCH_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum, portMacNum, &regAddr);
            offset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_CG_CNS;
            fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS;
        }
        else if (portMacType == PRV_CPSS_PORT_MTI_100_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.frmLength;
            offset = 0;
            fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS;
            use_valueGranularity1B = 1;
        }
        else if (portMacType == PRV_CPSS_PORT_MTI_400_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.frmLength;
            offset = 0;
            fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS;
            use_valueGranularity1B = 1;
        }
        else if (portMacType == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.frmLength;
            offset = 0;
            fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS;
            use_valueGranularity1B = 1;
        }
        else if (portMacType == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.frmLength;
            offset = 0;
            fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CG_CNS;
            use_valueGranularity1B = 1;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            offset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_XG_CNS;
            fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CNS;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = PRV_CPSS_DXCH_PORT_MRU_FIELD_OFFSET_GE_CNS;
        fieldLength = PRV_CPSS_DXCH_PORT_MRU_FIELD_LENGTH_CNS;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (portMacType == PRV_CPSS_PORT_CG_E)
    {
        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacNum),
            PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portMacNum).CGMAConvertersResets,
            26, 1, &value);
        if (0 == value)
        {/* if CG MAC in reset, return HW default, because access to MAC at this
            time causes hang */
            *mruSizePtr = 0x600;
        }
        else
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
                regAddr, offset, fieldLength, &value);
            /*  In CG unit, the granularity of MRU size is 1 so no further handling
            is needed on this value. */
            *mruSizePtr = value;
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    }
    else
    {
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacNum),
            regAddr, offset, fieldLength, &value);
        if(use_valueGranularity1B)
        {
            *mruSizePtr = value;
        }
        else
        {
            /* In units prior to CG, the MRU size is in granularity of 2B, so the value in the
            register was devided by two before written to register and need to be multiplied
            by two upon import */
            *mruSizePtr = value<<1;
        }
    }

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;

}

/**
* @internal internal_cpssDxChPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruGet
(
    IN   GT_U8  devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    OUT  GT_U32 *mruSizePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  targetMruSize;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(mruSizePtr);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacMRUGetFunc(devNum,portNum,
                                  mruSizePtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  &targetMruSize);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set MRU size value for switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortMruGet(devNum,portNum,mruSizePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacMRUGetFunc(devNum,portNum,
                                  mruSizePtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,
                                  &targetMruSize);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;

}

/**
* @internal cpssDxChPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruGet
(
    IN   GT_U8  devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    OUT  GT_U32 *mruSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mruSizePtr));

    rc = internal_cpssDxChPortMruGet(devNum, portNum, mruSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mruSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMruProfileSet function
* @endinternal
*
* @brief   Set an MRU profile for port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*                                      In eArch devices portNum is default ePort.
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruProfileSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           profileId
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    if (profileId > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write to pre-tti-lookup-ingress-eport table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MRU_INDEX_E, /* field name */
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           profileId);
}

/**
* @internal cpssDxChPortMruProfileSet function
* @endinternal
*
* @brief   Set an MRU profile for port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*                                      In eArch devices portNum is default ePort.
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruProfileSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           profileId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileId));

    rc = internal_cpssDxChPortMruProfileSet(devNum, portNum, profileId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMruProfileGet function
* @endinternal
*
* @brief   Get an MRU profile for port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*                                      In eArch devices portNum is default ePort.
*
* @param[out] profileIdPtr             - (pointer to) the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruProfileGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *profileIdPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(profileIdPtr);

    /* write to pre-tti-lookup-ingress-eport table */
    return prvCpssDxChReadTableEntryField(devNum,
                                          CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                          portNum,
                                          PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                          SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MRU_INDEX_E, /* field name */
                                          PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                          profileIdPtr);
}

/**
* @internal cpssDxChPortMruProfileGet function
* @endinternal
*
* @brief   Get an MRU profile for port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*                                      In eArch devices portNum is default ePort.
*
* @param[out] profileIdPtr             - (pointer to) the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruProfileGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *profileIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIdPtr));

    rc = internal_cpssDxChPortMruProfileGet(devNum, portNum, profileIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortProfileMruSizeSet function
* @endinternal
*
* @brief   Set an MRU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..7)
* @param[in] mruSize                  - max receive packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, profile or MRU size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortProfileMruSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mruSize
)
{
    GT_U32 regAddr;
    GT_U32 offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (profile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        offset = ((profile % 2) ? 14 : 0);
    }

    if (mruSize > 16383)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_FALCON_MRU_PER_DEFAULT_EPORT_WA_E)) &&
       (!PRV_CPSS_PP_MAC(devNum)->isGmDevice))
    {
        if (mruSize < 4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* reduce MRU to 4 bytes. GM does not have such problem. */
        mruSize -= 4;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MRUException.MRUSize[profile / 2];

    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 14, mruSize);
}

/**
* @internal cpssDxChPortProfileMruSizeSet function
* @endinternal
*
* @brief   Set an MRU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..7)
* @param[in] mruSize                  - max receive packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, profile or MRU size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortProfileMruSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mruSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortProfileMruSizeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, mruSize));

    rc = internal_cpssDxChPortProfileMruSizeSet(devNum, profile, mruSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, mruSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortProfileMruSizeGet function
* @endinternal
*
* @brief   Get an MRU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..7)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortProfileMruSizeGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mruSizePtr
)
{
    GT_U32 regAddr;
    GT_U32 offset;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mruSizePtr);

    if (profile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        offset = ((profile % 2) ? 14 : 0);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MRUException.MRUSize[profile / 2];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 14, mruSizePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_FALCON_MRU_PER_DEFAULT_EPORT_WA_E)) &&
       (!PRV_CPSS_PP_MAC(devNum)->isGmDevice))
    {
        /* increase MRU to 4 bytes. GM does not have such problem. */
        *mruSizePtr += 4;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortProfileMruSizeGet function
* @endinternal
*
* @brief   Get an MRU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..7)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortProfileMruSizeGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mruSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortProfileMruSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, mruSizePtr));

    rc = internal_cpssDxChPortProfileMruSizeGet(devNum, profile, mruSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, mruSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMruExceptionCommandSet function
* @endinternal
*
* @brief   Set the command assigned to frames that exceed the default ePort MRU
*         size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruExceptionCommandSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_U32  regAddr;
    GT_U32  value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MRUException.MRUCommandAndCode;
    return prvCpssHwPpSetRegField(devNum, regAddr, 8, 3, value);
}

/**
* @internal cpssDxChPortMruExceptionCommandSet function
* @endinternal
*
* @brief   Set the command assigned to frames that exceed the default ePort MRU
*         size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCommandSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruExceptionCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, command));

    rc = internal_cpssDxChPortMruExceptionCommandSet(devNum, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMruExceptionCommandGet function
* @endinternal
*
* @brief   Get the command assigned to frames that exceed the default ePort MRU
*         size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruExceptionCommandGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MRUException.MRUCommandAndCode;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 3, &value);
    if (rc == GT_OK)
    {
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr,value);
    }
    return rc;
}

/**
* @internal cpssDxChPortMruExceptionCommandGet function
* @endinternal
*
* @brief   Get the command assigned to frames that exceed the default ePort MRU
*         size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCommandGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruExceptionCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, commandPtr));

    rc = internal_cpssDxChPortMruExceptionCommandGet(devNum, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMruExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/drop code assigned to a frame which fails the MRU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or cpu/drop code
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruExceptionCpuCodeSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
)
{
    GT_U32                                  regAddr;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;
    GT_STATUS                               rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MRUException.MRUCommandAndCode;

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, (GT_U32)dsaCpuCode);
}

/**
* @internal cpssDxChPortMruExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/drop code assigned to a frame which fails the MRU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or cpu/drop code
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCpuCodeSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruExceptionCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCode));

    rc = internal_cpssDxChPortMruExceptionCpuCodeSet(devNum, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMruExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/drop code assigned to a frame which fails the MRU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr               - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMruExceptionCpuCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
)
{
    GT_U32      regAddr;
    GT_U32      dsaCpuCode;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).MRUException.MRUCommandAndCode;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)dsaCpuCode,
                                        cpuCodePtr);
}

/**
* @internal cpssDxChPortMruExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/drop code assigned to a frame which fails the MRU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr               - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCpuCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMruExceptionCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodePtr));

    rc = internal_cpssDxChPortMruExceptionCpuCodeGet(devNum, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    value = (enable == GT_TRUE) ? 1 : 0;

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
    }

    offset = 9;

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* In CG unit the value configures the disable operation(reversed operation, hence (1 - value) ) */
        value = 1 - value;
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 16;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.crcMode;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* In MTI MAC100 unit the value configures the disable operation(reversed operation, hence (1 - value) ) */
            value = (enable == GT_TRUE) ? 0 : 1;
            regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 16;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.crcMode;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* In MTI MAC100 unit the value configures the disable operation(reversed operation, hence (1 - value) ) */
            value = (enable == GT_TRUE) ? 0 : 1;
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 1;
            regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 16;
        }

        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.crcMode;
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                /* In MTI MAC100 unit the value configures the disable operation(reversed operation, hence (1 - value) ) */
                value = (enable == GT_TRUE) ? 0 : 1;
                regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
                regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
                regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;
                regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 16;
            }
        }


    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal internal_cpssDxChPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
    GT_BOOL     targetEnable = enable;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacCRCCheckSetFunc(devNum,portNum,
                                  targetEnable,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  &targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set Port Crc Check Enable value for switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortCrcCheckEnableSet(devNum,portNum,targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacCRCCheckSetFunc(devNum,portNum,
                                  targetEnable,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,
                                  &targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCrcCheckEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortCrcCheckEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortCrcCheckEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_STATUS rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            if ( portMacType == PRV_CPSS_PORT_MTI_100_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.crcMode;

                if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

                offset  = 16;
                rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, offset, 1, &value);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }

                *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
            }
            else if ( portMacType == PRV_CPSS_PORT_MTI_USX_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.crcMode;

                if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

                offset  = 16;
                rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, offset, 1, &value);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }

                *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
            }

            else if((portMacType == PRV_CPSS_PORT_MTI_CPU_E) && (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.crcMode;

                if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

                offset  = 16;
                rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, offset, 1, &value);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }

                *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
            }

            else
            {
                /* Not supported for other MTI mac */
                *enablePtr = GT_FALSE;
            }
            return GT_OK;
        }

        if(portMacType == PRV_CPSS_PORT_CG_E)
        {
            PRV_CPSS_DXCH_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum, portMacNum, &regAddr);
            offset = 16;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            offset = 9;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 0;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* get state */
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, offset, 1, &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if (portMacType == PRV_CPSS_PORT_CG_E)
    {
        *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
    }
    else
    {
        *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortCrcCheckEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacCRCCheckGetFunc(devNum,portNum,
                                  enablePtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get Port CRC Check Enable value from switch MAC  */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortCrcCheckEnableGet(devNum,portNum,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacCRCCheckGetFunc(devNum,portNum,
                                          enablePtr,
                                          CPSS_MACDRV_STAGE_POST_E,
                                          &doPpMacConfig,
                                          enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortCrcCheckEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCrcCheckEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortCrcCheckEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortXGmiiModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
)
{
    GT_U32  value;          /* value to write into the register */
    GT_U32  offset;         /* field offset */
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(mode)
    {
        case CPSS_PORT_XGMII_LAN_E:
            value = 0;
            break;
        case CPSS_PORT_XGMII_WAN_E:
            value = 1;
            break;
        case CPSS_PORT_XGMII_FIXED_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
       (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        offset = 5;
    }
    else
    {
        offset = 6;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* Setting the XGMII Transmit Inter-Packet Gap (IPG) mode */

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXGmiiModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXGmiiModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChPortXGmiiModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - Pointer to XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortXGmiiModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
)
{
    GT_U32 value;           /* value read from the register */
    GT_U32 offset; /* field offset */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32          regAddr;         /* register address */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
       (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        offset = 5;
    }
    else
    {
        offset = 6;
    }

    /* CG unit supports only LAN PHY mode */
    if (portMacType == PRV_CPSS_PORT_CG_E)
    {
        *modePtr = CPSS_PORT_XGMII_LAN_E;
        return GT_OK;
    }

    /* get xgmii mode */
    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, offset, 2, &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    switch(value)
    {
        case 0:
            *modePtr = CPSS_PORT_XGMII_LAN_E;
            break;
        case 1:
            *modePtr = CPSS_PORT_XGMII_WAN_E;
            break;
        case 2:
            *modePtr = CPSS_PORT_XGMII_FIXED_E;
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - Pointer to XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXGmiiModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXGmiiModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChPortXGmiiModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ipgBase                  - IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or ipgBase
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
static GT_STATUS internal_cpssDxChPortIpgBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
)
{
    GT_U32 value;   /* value to write into the register */
    GT_U32 offset;  /* bit number inside register       */
    GT_U32 regAddr;    /* register address */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    offset = 9;

    switch(ipgBase)
    {
        case CPSS_PORT_XG_FIXED_IPG_12_BYTES_E: value = 0;
                                                break;
        case CPSS_PORT_XG_FIXED_IPG_8_BYTES_E:  value = 1;
                                                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ipgBase                  - IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or ipgBase
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssDxChPortIpgBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortIpgBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipgBase));

    rc = internal_cpssDxChPortIpgBaseSet(devNum, portNum, ipgBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipgBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] ipgBasePtr               - pointer to IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
static GT_STATUS internal_cpssDxChPortIpgBaseGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
)
{
    GT_U32 value;   /* value read from the register */
    GT_U32 offset;  /* bit number inside register   */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32          regAddr;         /* register address */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ipgBasePtr);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E || portMacType == PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    offset = 9;
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* set xgmii mode */
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                regAddr, offset, 1, &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *ipgBasePtr = (value == 0) ? CPSS_PORT_XG_FIXED_IPG_12_BYTES_E :
                                 CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;

    return GT_OK;
}

/**
* @internal cpssDxChPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] ipgBasePtr               - pointer to IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssDxChPortIpgBaseGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortIpgBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipgBasePtr));

    rc = internal_cpssDxChPortIpgBaseGet(devNum, portNum, ipgBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipgBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] ipg                      - IPG in bytes, acceptable range:
*                                      (APPLICABLE RANGES: 0..511) (APPLICABLE DEVICES DxCh3 and above)
*                                      Default HW value is 12 bytes.
*                                       for CG MAC (BC3 and
*                                       above) --> 1-8 means IPG
*                                       depend on packet size,
*                                       move between 1byte to
*                                       8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS internal_cpssDxChPortIpgSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  ipg
)
{
    GT_STATUS rc;
    GT_U32 regAddr; /* register address */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChFalconPortMacIPGLengthSet(devNum,portNum,ipg);
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        rc = prvCpssDxChBobcat2PortMacIPGLengthSet(devNum,portNum,ipg);

        return rc;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* set */
    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {
        if(ipg >= BIT_9)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = ipg;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 9;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 6;
    }
    else
    {
        if(ipg >= BIT_4)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        if(portNum != CPSS_CPU_PORT_NUM_CNS)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serialParameters;
        }
        else /* CPU port */
        {
             regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                                    cpuPortRegs.serialParameters;
        }

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = ipg;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 4;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] ipg                      - IPG in bytes, acceptable range:
*                                      (APPLICABLE RANGES: 0..511) (APPLICABLE DEVICES DxCh3 and above)
*                                      Default HW value is 12 bytes.
*                                       for CG MAC (BC3 and
*                                       above) --> 1-8 means IPG
*                                       depend on packet size,
*                                       move between 1byte to
*                                       8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS cpssDxChPortIpgSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  ipg
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortIpgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipg));

    rc = internal_cpssDxChPortIpgSet(devNum, portNum, ipg);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipg));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] ipgPtr                   - (pointer to) IPG value in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChPortIpgGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_U32  *ipgPtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ipgPtr);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *ipgPtr = 0;

        return prvCpssDxChFalconPortMacIPGLengthGet(devNum,portNum,portMacType,/*OUT*/ipgPtr);
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        GT_STATUS rc;

        *ipgPtr = 0;

        rc = prvCpssDxChBobcat2PortMacIPGLengthGet(devNum,portNum,portMacType,/*OUT*/ipgPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        return GT_OK;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* get */
    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        return prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 6, 9, ipgPtr);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_SERIAL_PARAM_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        return prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0, 4, ipgPtr);
    }
}

/**
* @internal cpssDxChPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] ipgPtr                   - (pointer to) IPG value in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortIpgGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_U32  *ipgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortIpgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipgPtr));

    rc = internal_cpssDxChPortIpgGet(devNum, portNum, ipgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] number                   -   of words
*                                      (APPLICABLE RANGES: 0..127) (APPLICABLE DEVICES DxCh3 and above)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortExtraIpgSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8       number
)
{
    GT_U32 value;           /* value to write into the register */
    GT_U32  regAddr;    /* register address */
    GT_U32 offset;      /* bit number inside register       */
    GT_U32 sizeOfValue; /* number of bits to write to the register       */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = number;

    /* set */
    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {
        if(value >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        sizeOfValue=7;
        offset=6;
    }
    else
    {
        if(value >= BIT_2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        sizeOfValue=2;
        offset=4;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {
       PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    }
    else
    {
       PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portNum,PRV_CPSS_PORT_XG_E,&regAddr);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = sizeOfValue;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {
       PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    }
    else
    {
       PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    }
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = sizeOfValue;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] number                   -   of words
*                                      (APPLICABLE RANGES: 0..127) (APPLICABLE DEVICES DxCh3 and above)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortExtraIpgSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8       number
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortExtraIpgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, number));

    rc = internal_cpssDxChPortExtraIpgSet(devNum, portNum, number);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, number));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                -  pointer to number of words
*                                      (APPLICABLE RANGES: 0..127) (APPLICABLE DEVICES DxCh3 and above)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortExtraIpgGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8       *numberPtr
)
{
    GT_U32 value;           /* value to write into the register */
    GT_U32  regAddr;
    GT_STATUS rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E || portMacType == PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
    {

        PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,6, 7, &value);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portNum,portMacType,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,4, 2, &value);
    }
    if (rc != GT_OK)
        return rc;

    *numberPtr = (GT_U8)value;

    return GT_OK;
}

/**
* @internal cpssDxChPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                -  pointer to number of words
*                                      (APPLICABLE RANGES: 0..127) (APPLICABLE DEVICES DxCh3 and above)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortExtraIpgGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8       *numberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortExtraIpgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numberPtr));

    rc = internal_cpssDxChPortExtraIpgGet(devNum, portNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLocalFaultPtr          - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isLocalFaultPtr
)
{
    GT_U32  value = 0;
    GT_U32  regAddr;
    GT_U32  portMacNum;      /* MAC number */
    GT_U32  offset = 0;
    GT_U32  length = 1;
    GT_BOOL canReadCg;
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD portMode = NON_SUP_MODE;
    GT_BOOL accessEnable;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isLocalFaultPtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portMacNum,&regAddr);

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* check if cg is in reset and with no clk */
        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        canReadCg = mvHwsCgMac28nmAccessGet(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum), portMacNum);
        if (canReadCg == GT_TRUE)
        {
            if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                        regAddr, offset, length, &value) != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
        *isLocalFaultPtr = BIT2BOOL_MAC(value);

        return GT_OK;

    }
    else
    {
        mvHwsCm3SemOper(devNum,portMacNum, GT_TRUE);
        if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) &&
           (CPSS_PORT_INTERFACE_MODE_NA_E != PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum)) &&
           (CPSS_PORT_SPEED_NA_E != PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum)))
        {
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                                            PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                                            &portMode);
           /*protect mac clock enable*/
            if(( rc == GT_OK) &&(portMode != NON_SUP_MODE))
            {
                value = 0xFFFFFFFF;
                rc = mvHwsPortAccessCheck(devNum, portMacNum, portMode, &accessEnable);
                if((rc != GT_OK) ||(accessEnable == GT_FALSE))
                {
                    *isLocalFaultPtr = GT_FALSE;
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    return GT_OK;

                }
            }
        }


        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_100_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                        regAddr, offset, length, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_400_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                        regAddr, offset, length, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                        regAddr, offset, length, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else if((PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_CPU_E) && (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.status;
            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                        regAddr, offset, length, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }

        }
        else
        {
            PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
            offset = 2;
        }
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
    {
        mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacNum),
                regAddr, offset, length, &value) != GT_OK)
    {
        mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);

    *isLocalFaultPtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLocalFaultPtr          - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isLocalFaultPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXgmiiLocalFaultGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isLocalFaultPtr));

    rc = internal_cpssDxChPortXgmiiLocalFaultGet(devNum, portNum, isLocalFaultPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isLocalFaultPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRemoteFaultConfigGet function
* @endinternal
*
* @brief   Reads bit then indicate if the rmf is configured.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRemoteFaultConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
{
    GT_U32 value = 0;
    GT_U32  regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isRemoteFaultPtr);

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.portControl;

        if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
        {
            /* MTI ports, we need to read twice, as the first call is to clear the bit */
            if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                     regAddr, 1, 1, &value) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.segPortControl;

        if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
        {
            /* MTI ports, we need to read twice, as the first call is to clear the bit */
            if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                     regAddr, 1, 1, &value) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
             regAddr, 1, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *isRemoteFaultPtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortRemoteFaultConfigGet function
* @endinternal
*
* @brief   Reads bit that indicate if the remote fault was
*         configured .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRemoteFaultConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRemoteFaultConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isRemoteFaultPtr));

    rc = internal_cpssDxChPortRemoteFaultConfigGet(devNum, portNum, isRemoteFaultPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isRemoteFaultPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
{
    GT_U32 value = 0;
    GT_U32  regAddr;
    GT_U32  portMacNum;      /* MAC number */
    GT_BOOL canReadCg;
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD portMode = NON_SUP_MODE;
    GT_BOOL accessEnable;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isRemoteFaultPtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

            /* check if cg is in reset and with no clk */
        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        canReadCg = mvHwsCgMac28nmAccessGet(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum), portMacNum);
        if (canReadCg == GT_TRUE)
        {

            if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                    regAddr, 1, 1, &value) != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
        *isRemoteFaultPtr = BIT2BOOL_MAC(value);

        return GT_OK;
    }
    else
    {
        mvHwsCm3SemOper(devNum,portMacNum, GT_TRUE);
        if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) &&
           (CPSS_PORT_INTERFACE_MODE_NA_E != PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum)) &&
           (CPSS_PORT_SPEED_NA_E != PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum)))
        {
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                                            PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                                            &portMode);
           /*protect mac clock enable*/
            if(( rc == GT_OK) &&(portMode != NON_SUP_MODE))
            {
                value = 0xFFFFFFFF;
                rc = mvHwsPortAccessCheck(devNum, portMacNum, portMode, &accessEnable);
                if((rc != GT_OK) ||(accessEnable == GT_FALSE))
                {
                    *isRemoteFaultPtr = GT_FALSE;
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    return GT_OK;

                }
            }
        }

        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_100_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                         regAddr, 1, 1, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_400_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                         regAddr, 1, 1, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                         regAddr, 1, 1, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else if((PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_MTI_CPU_E) && (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.status;

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
            {
                /* MTI ports, we need to read twice, as the first call is to clear the bit */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                         regAddr, 1, 1, &value) != GT_OK)
                {
                    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
        else
        {
            PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        }
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
    {
        mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
             regAddr, 1, 1, &value) != GT_OK)
    {
        mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);

    *isRemoteFaultPtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXgmiiRemoteFaultGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isRemoteFaultPtr));

    rc = internal_cpssDxChPortXgmiiRemoteFaultGet(devNum, portNum, isRemoteFaultPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isRemoteFaultPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChXlgMacIntMaskGet function
* @endinternal
*
* @brief   Get the address of the XLG mac interrupt mask register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMacNum               - port mac number
*
* @param[out] regAddrPtr               - address of the XLG mac interrupt mask register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChXlgMacIntMaskGet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_U32         portMacNum,
    OUT GT_U32         *regAddrPtr
)
{
    GT_U32 regAddr;

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    PRV_CPSS_DXCH_PORT_MAC_INT_MASK_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);

    *regAddrPtr = regAddr;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortRemoteFaultSet function
* @endinternal
*
* @brief  Configure the port to start or stop sending fault
*         signals to partner. When port is configured to send,
*         link on both sides will be down.
*         Note: tx_rem_fault takes precedence over
*         tx_loc_fault. for CG mac, know that using tx_rem_fault
*         will cancel tx_loc_fault effect.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2;  AC5; Aldrin.
* @note   NOT APPLICABLE DEVICES:  Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - start sending fault signals to peer (or stop)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortRemoteFaultSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         send
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (!(PRV_CPSS_SIP_5_CHECK_MAC(devNum)) &&
        !(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChPortFaultSendSet(devNum, portNum, ifMode, speed, send);
}

/**
* @internal cpssDxChPortRemoteFaultSet function
* @endinternal
*
* @brief  Configure the port to start or stop sending fault
*         signals to partner. When port is configured to send,
*         link on both sides will be down.
*         Note: tx_rem_fault takes precedence over
*         tx_loc_fault. for CG mac, know that using tx_rem_fault
*         will cancel tx_loc_fault effect.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum;
*         Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - start sending fault signals to peer (or stop)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRemoteFaultSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         send
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRemoteFaultSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssDxChPortRemoteFaultSet(devNum, portNum, ifMode, speed, send);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacStatusPtr         - info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacStatusGet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;
    GT_U32 cgValue1,cgValue2,cgValue3;
    GT_U32 portMacNum;      /* MAC number */
    GT_U32 xoff_status = 1; /* The status is XOFF when xoff_status = 0 */
    GT_STATUS rc;           /* return code */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
    GT_BOOL doPpMacConfig = GT_TRUE;
    GT_BOOL canReadCg; /* check if cg is in reset and with no clk */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacStatusPtr);

    value = 0;
    cgValue1 = 0;
    cgValue2 = 0;
    cgValue3 = 0;

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if ((portMacObjPtr != NULL) &&
        (portMacObjPtr->macDrvMacPortMacStatusGetFunc))
    {

        rc = portMacObjPtr->macDrvMacPortMacStatusGetFunc(devNum, portNum,
                                                          portMacStatusPtr,
                                                          CPSS_MACDRV_STAGE_PRE_E,
                                                          &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) != PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupReadRegister(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                regAddr, &value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* For CG MAC the status bits are in different registers */
    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_DXCH_REG1_CG_CONVERTERS_STATUS_REG_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupReadRegister(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                regAddr, &cgValue1) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* check if cg is in reset and with no clk */
        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        canReadCg = mvHwsCgMac28nmAccessGet(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum), portMacNum);
        if (canReadCg == GT_FALSE)
        {
            cgValue2 = 0;
        }
        else
        {
            if (prvCpssDrvHwPpPortGroupReadRegister(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                    regAddr, &cgValue2) != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);


        PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupReadRegister(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                regAddr, &cgValue3) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* Read XOFF status from FCA DB register0 */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(prvCpssHwPpGetRegField(devNum, PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).DBReg0, 0, 1, &xoff_status) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    /* CG unit */
    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        portMacStatusPtr->isPortRxPause  = GT_FALSE;

        if(xoff_status == 0)
        {
            portMacStatusPtr->isPortTxPause = GT_TRUE;
        }
        else
        {
            portMacStatusPtr->isPortTxPause = GT_FALSE;
        }
        portMacStatusPtr->isPortBackPres =    GT_FALSE;
        portMacStatusPtr->isPortBufFull  =    (cgValue1 & 0x1) ? GT_TRUE : GT_FALSE;/* Bit#0 */
        portMacStatusPtr->isPortSyncFail =    (cgValue3 & (0x1 << 29)) ? GT_FALSE : GT_TRUE; /* bit#29 -> SyncFail */
        portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
        portMacStatusPtr->isPortAnDone =      GT_FALSE;
        portMacStatusPtr->isPortFatalError =  (cgValue2 & 0x3) ? GT_TRUE : GT_FALSE; /* Bit#0 Or Bit#1 */
    }
    else{
        /*  10 Gb interface */
        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
        {
            portMacStatusPtr->isPortRxPause  =    (value & 0x40) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortTxPause  =    (value & 0x80) ? GT_TRUE : GT_FALSE;

            /* Return XOFF status from FCA DB register0 */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                if(xoff_status == 0)
                {
                    portMacStatusPtr->isPortTxPause = GT_TRUE;
                }
                else
                {
                    portMacStatusPtr->isPortTxPause = GT_FALSE;
                }
            }

            portMacStatusPtr->isPortBackPres =    GT_FALSE;
            portMacStatusPtr->isPortBufFull  =    (value & 0x100) ? GT_TRUE : GT_FALSE;/* Bit#8 */
            portMacStatusPtr->isPortSyncFail =    (value & 0x1) ? GT_FALSE : GT_TRUE; /* value 0 -> SyncFail */
            portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
            portMacStatusPtr->isPortAnDone =      GT_FALSE;
            portMacStatusPtr->isPortFatalError =  (value & 0x6) ? GT_TRUE : GT_FALSE; /* Bit#1 Or Bit#2 */
        }
        else
        {
            portMacStatusPtr->isPortRxPause = (value & 0x40) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortTxPause = (value & 0x80) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortBackPres = (value & 0x100) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortBufFull = (value & 0x200) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortSyncFail = (value & 0x400) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortAnDone = (value & 0x800) ? GT_TRUE : GT_FALSE;

            portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
            portMacStatusPtr->isPortFatalError = GT_FALSE;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacStatusPtr         - info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacStatusGet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacStatusPtr));

    rc = internal_cpssDxChPortMacStatusGet(devNum, portNum, portMacStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortLion2GeLinkStatusWaEnableSet function
* @endinternal
*
* @brief   Enable/disable WA for FE-4933007 (In Lion2 port in tri-speed mode link
*         status doesn't change in some cases when cable is disconnected/connected.)
*         Must disable WA if any type of loopback defined on GE port to see link up.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  WA
*                                      If GT_FALSE, disable WA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortLion2GeLinkStatusWaEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E,
                                                                    &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(enable);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 5;

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal prvCpssDxChPortLion2InternalLoopbackEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortLion2InternalLoopbackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      portGroupId;    /* core number of port */
    GT_U32      localPort;      /* port number in local core */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface in HWS format */
    MV_HWS_PORT_STANDARD    portModeCombo;   /* port interface in HWS format for
                                                    non-active combo MAC member*/
    MV_HWS_PORT_LB_TYPE     lbType;     /* loopback type in HWS format */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* current interface of port */
    CPSS_PORT_SPEED_ENT             speed;  /* current speed of port */
    GT_U32      portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {/* loopback on port which interface not defined yet forbidden */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);
    if (PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                     PRV_CPSS_DXCH_LION2_GIGE_MAC_LINK_STATUS_WA_E))
    {/* disable WA to let link up, when loopback enabled */
        rc = prvCpssDxChPortLion2GeLinkStatusWaEnableSet(devNum,portNum,!enable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                               PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
    {
        if(enable)
        {/* restore application configuration for force link down if loopback enabled */
            rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum,portNum,
           CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                              info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.
                                portForceLinkDownBmpPtr,portNum));
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    lbType = (enable) ? TX_2_RX_LB : DISABLE_LB;

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* disable port always because loopback procedure resets and unreset
       port's MAC. Traffic should be avoided during reset/unreset of MAC to
       avoid corrupted packets */
    if (prvCpssDxChPortStateDisableAndGet(devNum, portNum, &portStateStc) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                               PRV_CPSS_DXCH_LION2_PORT_MAC_MIB_COUNTERS_CORRUPTION_WA_E))
    {
        /* read MAC counters and store values in shadow */
        rc = prvCpssDxChPortMacCountersOnPortGet(devNum,portNum, GT_FALSE, NULL,
                                                 PRV_DXCH_PORT_MAC_CNTR_READ_MODE_UPDATE_SHADOW_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if((portMode != HGL) &&
       (PRV_CPSS_PORT_MTI_100_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum)) &&
       (PRV_CPSS_PORT_MTI_400_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum)) &&
       (PRV_CPSS_PORT_MTI_USX_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum)) &&
       (PRV_CPSS_PORT_MTI_CPU_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
#ifdef ASIC_SIMULATION
       && (PRV_CPSS_PORT_CG_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
#endif
       ) /* HGL and MTI doesn't support MAC loopback */
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, localPort, portMode, HWS_MAC, lbType);
        rc = mvHwsPortLoopbackSet(devNum, portGroupId, localPort, portMode,
                                  HWS_MAC, lbType);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
        }
        /* set Rate limit */
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChPortResourcesRateLimitSet(devNum, portNum, speed, (enable? GT_FALSE:GT_TRUE));
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortResourcesConfigRateLimit : Failed");
            }
        }
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, localPort, portMode, HWS_PCS, lbType);
    rc = mvHwsPortLoopbackSet(devNum, portGroupId, localPort, portMode,
                              HWS_PCS, lbType);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
    }

    if ((_10GBase_KR == portMode) || (SGMII == portMode) || (_1000Base_X == portMode))
    {
        CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;

        comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum];
        if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
        { /* if it's combo port */
            if (_10GBase_KR == portMode)
            {
                portModeCombo = _100GBase_KR10;
            }
            else if ((SGMII == portMode) || (_1000Base_X == portMode))
            {
                portModeCombo = (SGMII == portMode) ? QSGMII : _100Base_FX;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, localPort, portModeCombo, HWS_MAC, lbType);
            rc = mvHwsPortLoopbackSet(devNum, portGroupId, localPort, portModeCombo,
                                      HWS_MAC, lbType);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, localPort, portModeCombo, HWS_PCS, lbType);
            rc = mvHwsPortLoopbackSet(devNum, portGroupId, localPort, portModeCombo,
                                      HWS_PCS, lbType);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
            }
        }
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                               PRV_CPSS_DXCH_LION2_PORT_MAC_MIB_COUNTERS_CORRUPTION_WA_E))
    {
        /* reset MAC counters and do not update shadow */
        rc = prvCpssDxChPortMacCountersOnPortGet(devNum,portNum, GT_FALSE, NULL,
                                                 PRV_DXCH_PORT_MAC_CNTR_READ_MODE_RESET_HW_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable port if we need */
    if (prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                               PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
    {
        if((CPSS_PORT_SPEED_10000_E == speed) &&
            ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) ||
            (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode) ||
            (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) ||
            (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode)))
        {
            if(!enable)
            {/* run WA again to configure force link down state appropriate for
                current state of port */
                return prvCpssDxChPortLion2LinkFixWa(devNum, portNum);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPreLion2InternalLoopbackEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortPreLion2InternalLoopbackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32 regAddr;               /* register address                    */
    GT_U32 value;                 /* value to write into the register    */
    PRV_CPSS_DXCH_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32 fieldOffset;           /* bit field offset */
    GT_U32 fieldLength;           /* number of bits to be written to register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* MAC unit used by port */
    GT_STATUS rc;       /* return code */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {
        /* disable port if we need */
        if (prvCpssDxChPortStateDisableAndGet(devNum,portNum, &portStateStc) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortPcsLoopbackEnableSet(devNum,portNum,enable)) != GT_OK)
        return rc;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if((portMacNum == CPSS_CPU_PORT_NUM_CNS) && (portMacType >= PRV_CPSS_PORT_XG_E))
                continue;

        if(portMacType < PRV_CPSS_PORT_XG_E)
        /* Set GMII loopback mode */
        {
            fieldOffset = 5;
            fieldLength = 1;
            value = (enable == GT_TRUE) ? 1 : 0;
        }
        else
            continue;

        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,portMacType,&regAddr);

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr,
                                                   fieldOffset, fieldLength,
                                                   value) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {
        /* enable port if we need */
        if (prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; AC5; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon, AC5P, AC5X, Harrier, Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For port 25 in DX269 (XG/HX port) it's the application responsibility to
*       reconfigure the loopback mode after switching XG/HX mode.
*
*/
static GT_STATUS internal_cpssDxChPortInternalLoopbackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32 portMacNum;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_STATUS rc;
    GT_BOOL apPortEnabled = GT_FALSE; /* indicate ap enabled on port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* Check if AP Enabled on port */
    /* No need to check if AP enabled on system - it is being done inside the per port  function */
    /* check AP enabled on port */
    rc = cpssDxChPortApPortEnableGet(devNum, portNum, &apPortEnabled);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "cpssDxChPortSerdesLoopbackModeSet - try to set AP port but can not get port status");
    }

    if (apPortEnabled)
    {
        /* No support for SerDes Loopback on AP enabled port */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }


    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortLoopbackSetFunc(devNum,portNum,
                                  enable, CPSS_MACDRV_STAGE_PRE_E, &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    /* switch MAC port enable configuration */
    if (doPpMacConfig == GT_TRUE)
    {

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                        info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.enabled == GT_TRUE)
        {/* could be this WA needed not only for XCAT */
            CPSS_PORTS_BMP_PORT_ENABLE_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                           info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.
                                           pcsLoopbackBmpPtr,portNum,enable);
        }

        if((CPSS_PP_FAMILY_DXCH_LION2_E <= PRV_CPSS_PP_MAC(devNum)->devFamily) ||
           (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            if(portNum != CPSS_CPU_PORT_NUM_CNS ||
            GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported)
            {
                return prvCpssDxChPortLion2InternalLoopbackEnableSet(devNum, portNum,
                                                                     enable);
            }
        }

        /* for CPU port and older devices */
        rc = prvCpssDxChPortPreLion2InternalLoopbackEnableSet(devNum, portNum,enable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortLoopbackSetFunc(devNum,portNum,enable,
                                                         CPSS_MACDRV_STAGE_POST_E,&doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For port 25 in DX269 (XG/HX port) it's the application responsibility to
*       reconfigure the loopback mode after switching XG/HX mode.
*
*/
GT_STATUS cpssDxChPortInternalLoopbackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInternalLoopbackEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortInternalLoopbackEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get Internal Loopback
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not relevant for the CPU port.
*
*/
static GT_STATUS internal_cpssDxChPortInternalLoopbackEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_STATUS status;       /* execution status */
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* MAC unit used by port */
    GT_U32      portGroupId;    /* core number of port */
    GT_U32          portMacNum;      /* MAC number */
    GT_U32 regAddr;         /* register address */
    GT_U32 portState;       /* current port state (enable/disable) */
    GT_U32 fieldOffset;     /* bit field offset */
    GT_BOOL doPpMacConfig = GT_TRUE;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPortLoopbackGetFunc(devNum,portNum,
                                  enablePtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* switch MAC port enable configuration */
    if (doPpMacConfig == GT_TRUE)
    {

        *enablePtr = GT_FALSE;

        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                                 portMacNum);

        if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ||
           (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            GT_U32      localPort;      /* port number in local core */
            MV_HWS_PORT_STANDARD    portMode;   /* port interface in HWS format */
            MV_HWS_PORT_LB_TYPE     lbType;     /* loopback type in HWS format */
            MV_HWS_UNIT             lpPlace;    /* which unit to read for lb state */
            CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* current interface of port */
            CPSS_PORT_SPEED_ENT             speed;  /* current speed of port */

            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

            lpPlace = HWS_MAC;
            ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
            speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
            {/* loopback on port which interface not defined yet - forbidden */
                status = GT_NOT_INITIALIZED;
                return status;
            }

            status = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
            if(status != GT_OK)
            {
                return status;
            }
            /* on CG mac the loopback is configured over the PCS*/
            if((PRV_CPSS_PORT_CG_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum)) || (PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
            {
                lpPlace = HWS_PCS;
            }
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackStatusGet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], *lbType)", devNum, portGroupId, localPort, portMode, lpPlace);
            status = mvHwsPortLoopbackStatusGet(devNum, portGroupId, localPort, portMode,
                                                lpPlace, &lbType);
            if (status != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status,"mvHwsPortLoopbackStatusGet : Hws Failed");
            }

            *enablePtr = (TX_2_RX_LB == lbType);
        }
        else/* not Lion2/3 devices */
        {
            portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
            PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

            if(portMacType >= PRV_CPSS_PORT_XG_E)
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.enabled == GT_TRUE)
                {/* not needed for FE and GE mac's, because there GMII loopback state returned */
                    *enablePtr = CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                    info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.pcsLoopbackBmpPtr,portMacNum);
                    *enablePtr = BIT2BOOL_MAC(*enablePtr);
                    return GT_OK;
                }

                fieldOffset = 13;
            }
            else
            {
                fieldOffset = 5;
            }

            /* store value of port state */
            status = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                        fieldOffset, 1, &portState);
            if (status != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            *enablePtr = BIT2BOOL_MAC(portState);
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPortLoopbackGetFunc(devNum,portNum,
                                  enablePtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get Internal Loopback
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not relevant for the CPU port.
*
*/
GT_STATUS cpssDxChPortInternalLoopbackEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInternalLoopbackEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortInternalLoopbackEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssDxChPortInbandAutoNegEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL  enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 regAddr;         /* register address */
    GT_U32 portState;       /* current port state (enabled/disabled) */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32          portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* related to GE interfaces only */
    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* store value of port state */
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0, 1, &portState) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    /* Disable port if it's enabled */
    if (portState == 1)
    {
        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 0, 1, 0) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(GT_TRUE == enable)
    {
        CPSS_PORT_INTERFACE_MODE_ENT ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum);
        /* the only modes that the function supports */
        if(ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E ||
           ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E ||
           ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
        {
            /* no need to check result, application may call this function prior
               to GE ifMode configuration, then this function will be engaged
               during ifModeSet */
            (GT_VOID)prvCpssDxChPortInbandAutonegMode(devNum, portNum,
                                      ifMode);
        }
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* Set inband auto-negotiation */
    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(enable);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 2;

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Restore original port state */
    if (portState == 1)
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);

        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 0, 1, 1) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInbandAutoNegEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInbandAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssDxChPortInbandAutoNegEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_STATUS rc;
    GT_U32      portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* store value of port state */
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, 2, 1, &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInbandAutoNegEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInbandAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortInbandAutoNegEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
*
* @param[out] portAttributSetArrayPtr  - Pointer to attributes values array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortAttributesOnPortGet
(
    IN    GT_U8                     devNum,
    IN    GT_PHYSICAL_PORT_NUM      portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
)
{
    GT_BOOL              portLinkUp;      /* port attributes */
    CPSS_PORT_SPEED_ENT  portSpeed;       /* port attributes */
    CPSS_PORT_DUPLEX_ENT portDuplexity;   /* port attributes */
    GT_STATUS            rc;              /* return code     */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
    GT_U32               portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portAttributSetArrayPtr);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if ((portMacObjPtr != NULL) &&
        (portMacObjPtr->macDrvMacPortAttributesGetFunc))
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacPortAttributesGetFunc(devNum,portNum,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  portAttributSetArrayPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* get port attributes */

    rc = cpssDxChPortLinkStatusGet(devNum, portNum, &portLinkUp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortSpeedGet(devNum, portNum, &portSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortDuplexModeGet(devNum ,portNum ,&portDuplexity);
    if (rc != GT_OK)
    {
        return rc;
    }

    portAttributSetArrayPtr->portLinkUp     = portLinkUp;
    portAttributSetArrayPtr->portSpeed      = portSpeed;
    portAttributSetArrayPtr->portDuplexity  = portDuplexity;

    return GT_OK;
}

/**
* @internal cpssDxChPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
*
* @param[out] portAttributSetArrayPtr  - Pointer to attributes values array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortAttributesOnPortGet
(
    IN    GT_U8                     devNum,
    IN    GT_PHYSICAL_PORT_NUM      portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortAttributesOnPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portAttributSetArrayPtr));

    rc = internal_cpssDxChPortAttributesOnPortGet(devNum, portNum, portAttributSetArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portAttributSetArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_U32  regAddr;/* register address to set */
    GT_U32  offset;/* start offset in register to set */
    GT_U32  fieldLen;/* number of bits to set */
    GT_U32  value;/* value to set to bits */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChFalconPortMacPreambleLengthSet(devNum,portNum,length);
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        return prvCpssDxChBobcat2PortMacPreambleLengthSet(devNum,portNum,direction,length);
    }

    if((length != 4) && (length != 8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (direction > CPSS_PORT_DIRECTION_BOTH_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* XG and up accept all directions , GE and down only accepts TX*/
    if ((direction != CPSS_PORT_DIRECTION_TX_E) &&
            (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) < PRV_CPSS_PORT_XG_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    fieldLen = 1;/* 1 bit to set*/
    if (direction != CPSS_PORT_DIRECTION_RX_E)
    {/* if current used MAC unit is XG/XLG then for DIRECTION_BOTH configure in
        GE DIRECTION_TX, but if user wants just RX skip GE configuration */
        value = (length == 8) ? 0 : 1;
        offset = 15;

        /* MAC Control 1 */
        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = fieldLen;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = offset;
        }
    }

    value = (length == 8) ? 1 : 0;
    switch(direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            offset = 0;/* rx bit */
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            offset = 1;/* tx bit */
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            fieldLen = 2;/* 2 bits to set*/
            offset = 0;/* start from rx bit */
            value |= (value << 1);/* set 2 bits instead of 1 ,  1 => 11 , 0 =>00 */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* MAC Control 0 */
    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    offset += ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum))) ? 11 : 12;

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = fieldLen;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = fieldLen;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal internal_cpssDxChPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    CPSS_PORT_DIRECTION_ENT  targetDirection = direction;
     GT_U32                   targetLength = length;

    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL)
    {
        rc = portMacObjPtr->macDrvMacPreambleLengthSetFunc(devNum,portNum,
                                  targetDirection,targetLength,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetDirection,
                                  &targetLength);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
      /* set switch MAC Port Preamble Length */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortPreambleLengthSet(devNum,portNum,
                                              targetDirection,targetLength);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)
    {
        rc = portMacObjPtr->macDrvMacPreambleLengthSetFunc(devNum,portNum,
                                  targetDirection,targetLength,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetDirection,
                                  &targetLength);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPreambleLengthSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, length));

    rc = internal_cpssDxChPortPreambleLengthSet(devNum, portNum, direction, length);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, length));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortPreambleLengthGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DIRECTION_ENT     direction,
    OUT GT_U32                      *lengthPtr
)
{
    GT_U32  regAddr;/* register address to set */
    GT_U32  offset;/* start offset in register to set */
    GT_U32  value;/* value to set to bits */
    GT_BOOL XGport = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */

    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChFalconPortMacPreambleLengthGet(devNum,portNum,portMacType,lengthPtr);
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        GT_STATUS rc;

        *lengthPtr = 0;
        rc = prvCpssDxChBobcat2PortMacPreambleLengthGet(devNum, portNum, portMacType, direction, lengthPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        return GT_OK;
    }

    if(((portNum != CPSS_CPU_PORT_NUM_CNS) || ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE))) &&
        (portMacType >= PRV_CPSS_PORT_XG_E))
    {
        XGport = GT_TRUE;
        /* MAC Control 0 */
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);

        switch(direction)
        {
        case CPSS_PORT_DIRECTION_RX_E:
            offset = ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
                      (PRV_CPSS_SIP_5_CHECK_MAC(devNum))) ? 11 : 12;/* rx bit */
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            offset = ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
                      (PRV_CPSS_SIP_5_CHECK_MAC(devNum))) ? 12 : 13;/* tx bit */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch(direction)
        {
            case CPSS_PORT_DIRECTION_TX_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* MAC Control 1 */
        PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 15;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacNum),
            regAddr, offset, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if (value == 0)
    {
        *lengthPtr = (XGport == GT_TRUE) ? 4 : 8;
    }
    else /* value == 1*/
    {
        *lengthPtr = (XGport == GT_TRUE) ? 8 : 4;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortPreambleLengthGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
      OUT GT_U32                   *lengthPtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    CPSS_PORT_DIRECTION_ENT  targetDirection = direction;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPreambleLengthGetFunc(devNum,portNum,
                                  direction,lengthPtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetDirection,
                                  lengthPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get switch MAC Port Preamble Length value  */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortPreambleLengthGet(devNum,portNum,targetDirection,lengthPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPreambleLengthGetFunc(devNum,portNum,
                                  targetDirection,lengthPtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetDirection,
                                  lengthPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPreambleLengthGet(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
      OUT GT_U32                   *lengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPreambleLengthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, lengthPtr));

    rc = internal_cpssDxChPortPreambleLengthGet(devNum, portNum, direction, lengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, lengthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortMacSaAddr function
* @endinternal
*
* @brief   Get port source middle MAC address
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] lmsInstance              - LMS instance number
*
* @param[out] regAddrMiddlePtr         - pointer to middle SA MAC address register
* @param[out] regAddrHighPtr           - pointer to high SA MAC address register
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported register address
*/
static GT_STATUS prvCpssDxChPortMacSaAddr
(
    IN  GT_U8           devNum,
    IN  GT_U32          lmsInstance,
    OUT GT_U32          *regAddrMiddlePtr,
    OUT GT_U32          *regAddrHighPtr
)
{
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        *regAddrMiddlePtr = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, lmsInstance).
            LMS0.LMS0Group0.portsMACSourceAddr.sourceAddrMiddle;
        *regAddrHighPtr = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, lmsInstance).
            LMS0.LMS0Group0.portsMACSourceAddr.sourceAddrHigh;
    }
    else
    {
        *regAddrMiddlePtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.srcAddrMiddle;
        *regAddrHighPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.srcAddrHigh;
    }

    if((PRV_CPSS_SW_PTR_ENTRY_UNUSED == *regAddrMiddlePtr) ||
       (PRV_CPSS_SW_PTR_ENTRY_UNUSED == *regAddrHighPtr))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the base part(40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
static GT_STATUS internal_cpssDxChPortMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
{
    GT_U32  macMiddle;          /* Holds bits 8 - 15 of mac     */
    GT_U32  macHigh;            /* Holds bits 16 - 47 of mac    */
    GT_STATUS rc;               /* Return code */
    GT_U32  regAddrMiddle;      /* pointer to middle SA MAC register address */
    GT_U32  regAddrHigh;        /* pointer to high SA MAC register address */
    GT_U32  regAddr;            /* register address */
    GT_U32  lmsInstance;        /* LMS instance */
    GT_U32  lmsInstanceNum;     /* LMS instance number */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  maxMacNum;          /* max MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_BOOL doPpMacConfig;
    GT_U32 maxPortNum, portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        maxPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
        for(portNum = 0; portNum < maxPortNum; portNum++)
        {
            rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
            if(rc != GT_OK)
            {
                return rc;
            }
            if((portMapShadow.valid) && (portMapShadow.portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E))
            {
                rc = prvCpssDxChPortSip6MacSaSet(devNum, portNum, macPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        return GT_OK;
    }
    else if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        maxMacNum = (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE) ? 28 : 72;

        for(portMacNum = 0; portMacNum < maxMacNum; portMacNum++)
        {
            /* support not continous MACs of bobk */
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

            /* Get PHY MAC object pnt */
            portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

            /* check if MACPHY callback should run */
            if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacSaBaseSetFunc)
            {

                rc = portMacObjPtr->macDrvMacSaBaseSetFunc(
                                        devNum,
                                        portMacNum,
                                        CPSS_MACDRV_STAGE_PRE_E,
                                        &doPpMacConfig,
                                        macPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }

                if(doPpMacConfig == GT_FALSE)
                {
                    continue;
                }
            }


            /* MAC SA - bits [15:8] */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
            }
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 8, 8, macPtr->arEther[4]);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* MAC SA - bits [31:16] */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA16To31;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA16To31;
            }
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, ((macPtr->arEther[3]) | (macPtr->arEther[2] << 8)));
            if(GT_OK != rc)
            {
                return rc;
            }

            /* MAC SA - bits [48:32] */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA32To47;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA32To47;
            }
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, ((macPtr->arEther[1]) | (macPtr->arEther[0] << 8)));
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        /* no LMS to configure */
        return GT_OK;
    }

    macMiddle   =  macPtr->arEther[4];
    macHigh     = (macPtr->arEther[3]        |
                  (macPtr->arEther[2] << 8)  |
                  (macPtr->arEther[1] << 16) |
                  (macPtr->arEther[0] << 24));

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        lmsInstanceNum = 3;
    }
    else
    {
        lmsInstanceNum = 1;
    }

    for(lmsInstance = 0; lmsInstance < lmsInstanceNum; lmsInstance++)
    {
        rc = prvCpssDxChPortMacSaAddr(devNum, lmsInstance,
                                      &regAddrMiddle, &regAddrHigh);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddrMiddle, 0, 8, macMiddle);
        if( rc != GT_OK )
        {
            return rc;
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddrHigh, macHigh);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the base part(40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssDxChPortMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacSaBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macPtr));

    rc = internal_cpssDxChPortMacSaBaseSet(devNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal portMacSaBaseFromFirstPortGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         take from first valid port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS portMacSaBaseFromFirstPortGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;               /* Return code */
    GT_U32  regAddr;            /* register address */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  maxMacNum;          /* max MAC number */
    GT_U32  value;              /* value from register */


    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        maxMacNum = (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE) ? 28 : 72;

        for(portMacNum = 0; portMacNum < maxMacNum; portMacNum++)
        {
            /* support not continues MACs of bobk */
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

            /* MAC SA - bits [15:8] */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
            }
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 8, &value);
            if(GT_OK != rc)
            {
                return rc;
            }

            macPtr->arEther[4] = (GT_U8)value;

            /* MAC SA - bits [31:16] */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA16To31;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA16To31;
            }
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &value);
            if(GT_OK != rc)
            {
                return rc;
            }
            macPtr->arEther[2] = (GT_U8)(value>>8);
            macPtr->arEther[3] = (GT_U8)value;

            /* MAC SA - bits [48:32] */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA32To47;
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA32To47;
            }
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &value);
            if(GT_OK != rc)
            {
                return rc;
            }

            macPtr->arEther[0] = (GT_U8)(value>>8);
            macPtr->arEther[1] = (GT_U8)value;

            /* stop after first valid port */
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);/* error ... no valid ports */
}


/**
* @internal internal_cpssDxChPortMacSaBaseGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
static GT_STATUS internal_cpssDxChPortMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
    GT_STATUS status = GT_OK;
    GT_U32  macMiddle;          /* Holds bits 8 - 15 of mac     */
    GT_U32  macHigh;            /* Holds bits 16 - 47 of mac    */
    GT_U32  regAddrMiddle;      /* middle SA MAC register address */
    GT_U32  regAddrHigh;        /* high SA MAC register address */
    GT_STATUS rc;               /* return code */
    GT_U32 maxPortNum, portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        maxPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
        for(portNum = 0; portNum < maxPortNum; portNum++)
        {
            rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
            if(rc != GT_OK)
            {
                return rc;
            }
            if((portMapShadow.valid) && (portMapShadow.portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E))
            {
                rc = prvCpssDxChPortSip6MacSaGet(devNum, portNum, macPtr);
                if(rc == GT_OK)
                {
                    return rc; /* Stop after first valid */
                }
            }
        }
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
    {
        /* no LMS , get info from first port */
        return portMacSaBaseFromFirstPortGet(devNum,macPtr);
    }

    rc = prvCpssDxChPortMacSaAddr(devNum, 0, &regAddrMiddle, &regAddrHigh);
    if(rc != GT_OK)
    {
        return rc;
    }

    status = prvCpssDrvHwPpGetRegField(devNum, regAddrMiddle, 0, 8, &macMiddle);
    if(status != GT_OK)
        return status;


    status = prvCpssDrvHwPpReadRegister(devNum, regAddrHigh, &macHigh);
    if(status != GT_OK)
        return status;

    macPtr->arEther[3] = (GT_U8)(macHigh & 0xFF);
    macPtr->arEther[2] = (GT_U8)((macHigh & 0xFF00) >> 8);
    macPtr->arEther[1] = (GT_U8)((macHigh & 0xFF0000) >> 16);
    macPtr->arEther[0] = (GT_U8)((macHigh & 0xFF000000) >> 24);
    macPtr->arEther[4] = (GT_U8)macMiddle;

    return GT_OK;
}

/**
* @internal cpssDxChPortMacSaBaseGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssDxChPortMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacSaBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macPtr));

    rc = internal_cpssDxChPortMacSaBaseGet(devNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortStateDisableSet function
* @endinternal
*
* @brief   Disable a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatePtr             - (pointer to) the state (en/dis) of port before calling this function
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPortStateDisableSet
(
    IN GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32  *portStatePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_BOOL     portState; /* current port state(enabled/disabled) */

    rc = prvCpssDxChPortEnableGet(devNum, portNum, &portState);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    *portStatePtr = (GT_U32)portState;

    /* disable port if we need */
    if (1 == *portStatePtr)
    {
        rc = cpssDxChPortEnableSet(devNum, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortStateEnableSet function
* @endinternal
*
* @brief   Enable a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portState                - the state (en/dis) of port before calling the port disable
*                                      function
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPortStateEnableSet
(
    IN GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 portState
)
{
    /* enable port if we need */
    if (portState == 1)
    {
        return cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortStateDisableAndGet function
* @endinternal
*
* @brief   Disable port and get current port parameters that should be
*         restored after port configuration.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStateStcPtr          - (pointer to) the port state struct for saving parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortStateDisableAndGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT PRV_CPSS_DXCH_PORT_STATE_STC    *portStateStcPtr
)
{
    GT_STATUS   rc;
    GT_U32      portEnableStatus;
    GT_U32      egfHwStatus;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT  egfPortLinkStatusState;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT  egfPortLinkStatusStateDown;
    CPSS_NULL_PTR_CHECK_MAC(portStateStcPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /*
            Force link down without modifying the 'DB'
        */
        rc = prvCpssDxChEgfPortLinkFilterForceLinkDown(devNum,portNum);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* NOTE: value portStateStcPtr->egfPortLinkStatusState is ignored in SIP6 !!! */
    }
    else
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChHwEgfEftFieldGet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                &egfHwStatus);

            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            egfPortLinkStatusState = PRV_CPSS_DXCH_HW_2_EGF_CONVERT_MAC(egfHwStatus);

            egfPortLinkStatusStateDown = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                PRV_CPSS_DXCH_EGF_CONVERT_MAC(egfPortLinkStatusStateDown));

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            egfPortLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E;
        }
        portStateStcPtr->egfPortLinkStatusState = egfPortLinkStatusState;
    }


    rc = prvCpssDxChPortStateDisableSet(devNum, portNum, &portEnableStatus);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    portStateStcPtr->portEnableState = portEnableStatus;
    return GT_OK;
}


/**
* @internal prvCpssDxChPortStateRestore function
* @endinternal
*
* @brief   Restore port parameters that was saved before port configuration.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portStateStcPtr          - (pointer to) the port state struct for restoring parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortStateRestore
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CPSS_DXCH_PORT_STATE_STC    *portStateStcPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT  egfPortLinkStatusState;

    CPSS_NULL_PTR_CHECK_MAC(portStateStcPtr);

    rc = prvCpssDxChPortStateEnableSet(devNum, portNum, portStateStcPtr->portEnableState);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* restoring Egf force link status so enqueueing could be made
           according to the configuration prior to port disable */
        /* NOTE: unlike sip5.20 : we call next function because it protect us
           from setting 'EGF link UP' before the time is ready ! */
        rc = prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed(devNum,portNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* NOTE: value portStateStcPtr->egfPortLinkStatusState is ignored in SIP6 !!! */
    }
    else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        egfPortLinkStatusState = portStateStcPtr->egfPortLinkStatusState;
        rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
            PRV_CPSS_DXCH_EGF_CONVERT_MAC(egfPortLinkStatusState));
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    value = (enable == GT_TRUE) ? 0 : 1;
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 5;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    offset = ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
              (PRV_CPSS_SIP_5_CHECK_MAC(devNum))) ? 13 : 14;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal internal_cpssDxChPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_BOOL    targetEnable = enable;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;            /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPaddingEnableSetFunc(devNum,portNum,
                                  targetEnable, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set switch MAC Padding Enable configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortPaddingEnableSet(devNum,portNum,targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPaddingEnableSetFunc(devNum,portNum,
                                  targetEnable,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPaddingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPaddingEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssDxChPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_STATUS   rc;      /* return status */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 5;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            offset = 13;
        }
        else
        {
            offset = 14;
        }
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
            regAddr, offset, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_STATUS   rc = GT_OK;                      /* return status */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacPaddingEnableGetFunc(devNum,portNum,
                                  enablePtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,enablePtr);
          if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* get Port Padding Enable value from switch MAC*/
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortPaddingEnableGet(devNum,portNum,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacPaddingEnableGetFunc(devNum,portNum,
                                  enablePtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPaddingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPaddingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*
*/
static GT_STATUS prvCpssDxChPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = (enable == GT_TRUE) ? 0 : 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 4;
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal internal_cpssDxChPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_BOOL    targetEnable = enable;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacExcessiveCollisionDropSetFunc(devNum,portNum,
                                  targetEnable, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* set switch MAC Collision Drop configuration */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortExcessiveCollisionDropEnableSet(devNum,portNum,targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacExcessiveCollisionDropSetFunc(devNum,portNum,
                                  targetEnable,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,&targetEnable);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortExcessiveCollisionDropEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortExcessiveCollisionDropEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Not relevant in full duplex mode
*
*/
static GT_STATUS prvCpssDxChPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_STATUS   rc;      /* return status */
    GT_U32      portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
            regAddr, 4, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;

}

/**
* @internal internal_cpssDxChPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Not relevant in full duplex mode
*
*/
static GT_STATUS internal_cpssDxChPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;         /* port object pointer */
    GT_STATUS rc = GT_OK;
    GT_BOOL doPpMacConfig = GT_TRUE;             /* do switch mac code indicator */
    GT_U32      portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacExcessiveCollisionDropGetFunc(devNum,portNum,
                                  enablePtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* Get Port Excessive Collision Drop value from switch MAC*/
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortExcessiveCollisionDropEnableGet(devNum,portNum,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* "post stage" callback run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacExcessiveCollisionDropGetFunc(devNum,portNum,
                                  enablePtr,
                                  CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,enablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Not relevant in full duplex mode
*
*/
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortExcessiveCollisionDropEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortExcessiveCollisionDropEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssDxChPortInBandAutoNegBypassEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(enable);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 3;

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInBandAutoNegBypassEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to Auto-Negotiation by pass status.
*                                      - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssDxChPortInBandAutoNegBypassEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_STATUS   rc;      /* return status */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
            regAddr, 3, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;

}

/**
* @internal cpssDxChPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to Auto-Negotiation by pass status.
*                                      - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInBandAutoNegBypassEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortInBandAutoNegBypassEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesGroupGet function
* @endinternal
*
* @brief   Get SERDES port group that may be used in per SERDES group APIs.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portSerdesGroupPtr       - Pointer to port group number
*                                      DxCh3 Giga/2.5 G, xCat GE devices:
*                                      Ports       |    SERDES Group
*                                      0..3        |      0
*                                      4..7        |      1
*                                      8..11       |      2
*                                      12..15      |      3
*                                      16..19      |      4
*                                      20..23      |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      DxCh3 XG devices:
*                                      0           |      0
*                                      4           |      1
*                                      10          |      2
*                                      12          |      3
*                                      16          |      4
*                                      22          |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      xCat FE devices
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      Lion devices: Port == SERDES Group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortSerdesGroupGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32   *portSerdesGroupPtr
)
{
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E
                                          | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(portSerdesGroupPtr);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* xCat3 */
    if(portNum < 24)
    {
        *portSerdesGroupPtr = portNum / 4;
    }
    else
    {
        *portSerdesGroupPtr = portNum - 18;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesGroupGet function
* @endinternal
*
* @brief   Get SERDES port group that may be used in per SERDES group APIs.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portSerdesGroupPtr       - Pointer to port group number
*                                      DxCh3 Giga/2.5 G, xCat GE devices:
*                                      Ports       |    SERDES Group
*                                      0..3        |      0
*                                      4..7        |      1
*                                      8..11       |      2
*                                      12..15      |      3
*                                      16..19      |      4
*                                      20..23      |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      DxCh3 XG devices:
*                                      0           |      0
*                                      4           |      1
*                                      10          |      2
*                                      12          |      3
*                                      16          |      4
*                                      22          |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      xCat FE devices
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      Lion devices: Port == SERDES Group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesGroupGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32   *portSerdesGroupPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portSerdesGroupPtr));

    rc = internal_cpssDxChPortSerdesGroupGet(devNum, portNum, portSerdesGroupPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portSerdesGroupPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortFlowControlReceiveFramesModeSet function
* @endinternal
*
* @brief  Set mode of received PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] fcMode                   - receive FC frames mode - see comments in enum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortFlowControlReceiveFramesModeSet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId; /* port group id */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        && (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        /* configuration not applicable, but also not needed */
        return GT_OK;
    }
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    switch (fcMode)
    {
        case CPSS_DXCH_PORT_FC_MODE_802_3X_E:
            value = 0;
            break;
        case CPSS_DXCH_PORT_FC_MODE_PFC_E:
            value = 1;
            break;
        default: return GT_OK; /* configuration not needed */
    }

    /* support for CG port */
    regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum,portMacNum).CGPORTMACCommandConfig;
    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* configuration not applicable, but also not needed */
        return GT_OK;
    }

    return prvCpssDrvHwPpPortGroupSetRegField(
        devNum, portGroupId, regAddr, 19 /*fieldOffset*/, 1 /*fieldLength*/, value);
}

/**
* @internal prvCpssDxChPortFlowControlReceiveFramesModeGet function
* @endinternal
*
* @brief  Get mode of received PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[out] fcModePtr               - (pointer to)receive FC frames mode - see comments in enum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortFlowControlReceiveFramesModeGet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT *fcModePtr
)
{
    GT_STATUS rc;      /* return code */
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId; /* port group id */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        && (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        /* configuration not applicable, but also not needed */
        *fcModePtr = CPSS_DXCH_PORT_FC_MODE_DISABLE_E;
        return GT_OK;
    }
    CPSS_NULL_PTR_CHECK_MAC(fcModePtr);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* default value, returned if there is no 100G mac */
    *fcModePtr = CPSS_DXCH_PORT_FC_MODE_DISABLE_E;

    regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum,portMacNum).CGPORTMACCommandConfig;
    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        return GT_OK;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, portGroupId, regAddr, 19 /*fieldOffset*/, 1 /*fieldLength*/, &value);
    if (rc != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);;
    }

    *fcModePtr = (value == 0)
        ? CPSS_DXCH_PORT_FC_MODE_802_3X_E
        : CPSS_DXCH_PORT_FC_MODE_PFC_E;
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Lion2 and above:
*       This function also configures insertion of DSA tag for PFC frames.
*
*/
static GT_STATUS internal_cpssDxChPortFlowControlModeSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 fieldOffset;/* the start bit number in the register */
    GT_U32 value, oppcodeValue = 0;      /* register value */
    GT_STATUS rc;      /* function return value */
    CPSS_CSCD_PORT_TYPE_ENT portCscdType;
    GT_BOOL enablePfcCascade;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32  dipBtsEn;   /* for Lion2 value of DIP_BTS_xxx_en fields dependent on
                            FC mode */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacFcModeSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacFcModeSetFunc(devNum,portNum,
                                  fcMode,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }


    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
       PRV_CPSS_SIP_6_FC_CFG_ENABLE_CHECK_MAC(devNum,portNum,fcMode);
       rc = prvCpssDxChPortSip6FlowControlModeSet(devNum,portNum,fcMode);
       return rc;
    }

    switch (fcMode)
    {
        case CPSS_DXCH_PORT_FC_MODE_802_3X_E:

            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_FC_802_3X_NOT_SUPPORTED_TM_MAPPED_PORTS_WA_E))
            {
                rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* if port mapped to TM - FC not supported */
                if(portMapShadowPtr->portMap.trafficManagerEn)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }

            value = 0;
            oppcodeValue = 0x1;
            break;

        case CPSS_DXCH_PORT_FC_MODE_PFC_E:

            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_PFC_NOT_SUPPORTED_PORTS_48_71_WA_E))
            {
                /* For ports 48..71 - PFC not supported */
                if((portMacNum >= 48) && (portMacNum <= 71))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }

            value = 1;
            oppcodeValue = 0x101;
            break;

        case CPSS_DXCH_PORT_FC_MODE_LL_FC_E:

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            value = 2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* use FCA unit for TX FC mode configuration (only if supported)
       Note: RX FC mode is configured to the same value via MAC registers */
    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            regAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }
        fieldOffset = 2;

        /* set FC_MODE */
        rc =  prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 2, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            regAddr = fcaStcPtr->L2CtrlReg1;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).L2CtrlReg1;
        }

        fieldOffset = 0;

        /* set OPPCODE */
        rc =  prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 16, oppcodeValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 14;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 14;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

        /* The rule is when the field "fc_mode" is configured to
             802_3x_MODE" in the following register:
            Port MAC Control Register2, then the fields
                 DIP_BTS_690_fix_en" and "DIP_BTS_693_en" for GE ports
                                must to be configured to disabled (0)
            for XLG ports the fields "DIP_BTS_550_en" and "DIP_BTS_677_en"
            in the corresponding XLG register must be configured to disabled (0).
            Fortunately in both GE and XLG registers appropriate bits are 8:9,
            so just register address must be specific.
        */
        PRV_CPSS_DXCH_PORT_MAC_VAR_REG_MAC(devNum,portNum,PRV_CPSS_PORT_GE_E,ppfcControl,(&regAddr));
        dipBtsEn = (CPSS_DXCH_PORT_FC_MODE_802_3X_E == fcMode) ? 0 : 0x3;
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = dipBtsEn;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 8;

        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                    perPortRegs[portNum].macRegsPerType[PRV_CPSS_PORT_XLG_E].ppfcControl;
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = dipBtsEn;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 8;

        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* for cascade port and pfc mode, enable insertion of DSA tag to PFC packets */
    if (fcMode == CPSS_DXCH_PORT_FC_MODE_PFC_E)
    {
        /* get cascade port configuration */
        rc = cpssDxChCscdPortTypeGet(devNum,portNum,CPSS_PORT_DIRECTION_TX_E,&portCscdType);
        if (rc != GT_OK)
        {
            return rc;
        }
        enablePfcCascade = (portCscdType == CPSS_CSCD_PORT_NETWORK_E) ?
                            GT_FALSE : GT_TRUE;
    }
    else
    {
        enablePfcCascade = GT_FALSE;
    }

   rc = prvCpssDxChPortPfcCascadeEnableSet(devNum,portNum,enablePfcCascade);
   if (rc != GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);;
   }

   return prvCpssDxChPortFlowControlReceiveFramesModeSet(
       devNum, portNum, fcMode);
}

/**
* @internal cpssDxChPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Lion2 and above:
*       This function also configures insertion of DSA tag for PFC frames.
*
*/
GT_STATUS cpssDxChPortFlowControlModeSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowControlModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, fcMode));

    rc = internal_cpssDxChPortFlowControlModeSet(devNum, portNum, fcMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, fcMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
*
* @param[out] fcModePtr                - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortFlowControlModeGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT   *fcModePtr
)
{

    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 fieldOffset;/* the start bit number in the register */
    GT_U32 value;      /* register value */
    GT_STATUS rc;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(fcModePtr);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacFcModeGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacFcModeGetFunc(devNum,portNum,
                                  fcModePtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc = prvCpssDxChPortSip6FlowControlModeGet(devNum,portNum,fcModePtr);
       return rc;
    }


    /* use FCA unit (instead of MAC) for FC mode configuration (only if supported) */
    if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            fcaStcPtr = (portMacNum == CPSS_CPU_PORT_NUM_CNS) ?
                 &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
                 &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum);

            regAddr = fcaStcPtr->FCACtrl;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
        }
        fieldOffset = 2;

        /* get FC_MODE */
        rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 2, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if(portMacType >= PRV_CPSS_PORT_XG_E)
        {
            fieldOffset = 14;
        }
        else
        {
            fieldOffset = 1;
        }

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, fieldOffset, 2, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    *fcModePtr = (CPSS_DXCH_PORT_FC_MODE_ENT)value;

    return GT_OK;
}

/**
* @internal cpssDxChPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
*
* @param[out] fcModePtr                - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFlowControlModeGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT   *fcModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFlowControlModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, fcModePtr));

    rc = internal_cpssDxChPortFlowControlModeGet(devNum, portNum, fcModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, fcModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
)
{
    return prvCpssDxChPortMacResetStateSet(devNum, portNum, state);
}

/**
* @internal cpssDxChPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacResetStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortMacResetStateSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC and XPCS Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS                           rc;
    GT_U32                              regAddr;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_DXCH_PP_CONFIG_STC         *pDev;
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    GT_U32                              portGroup;      /* port group number for multi-port-group devs */
    MV_HWS_PORT_STANDARD                portMode;       /* port interface in HWS format */
    PRV_CPSS_PORT_TYPE_ENT              portMacType;    /* MAC unit used by port */
    GT_U32                              portMacNum;     /* MAC number */
    CPSS_PORT_INTERFACE_MODE_ENT        ifMode;         /* interface mode to configure */
    CPSS_PORT_SPEED_ENT                 speed;          /* current speed of port */
    GT_U32                              localPort=0;    /* port number in local core */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    pDev        = PRV_CPSS_DXCH_PP_MAC(devNum);
    ifMode      = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    speed       = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);

    if((pDev->genInfo.devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E) || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        if ((CPSS_PORT_INTERFACE_MODE_NA_E != ifMode) && (CPSS_PORT_SPEED_NA_E != speed) && (portNum != CPSS_CPU_PORT_NUM_CNS))
        {
            portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

            prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);

            if (hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            if ((curPortParams.portMacType != LAST_MAC) && (curPortParams.portMacType != MAC_NA))
            {
                if (CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
                {
                    /* for QSGMII mode mvHwsMacReset shoud get the first portNum in the GOP */
                    localPort = localPort - localPort % 4;
                }

                rc = mvHwsMacReset(devNum, portGroup, localPort, portMode, curPortParams.portMacType, ((state == GT_TRUE) ? RESET : UNRESET));
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsMacReset: error for portNum=%d\n", localPort);
                }
            }
        }
    }
    else
    {
        if (portMacType >= PRV_CPSS_PORT_XG_E)
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            regDataArray[portMacType].regAddr = regAddr;
            regDataArray[portMacType].fieldData = BOOL2BIT_MAC(!state);
            regDataArray[portMacType].fieldLength = 1;
            regDataArray[portMacType].fieldOffset = 1;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            regDataArray[portMacType].regAddr = regAddr;
            regDataArray[portMacType].fieldData = BOOL2BIT_MAC(state);
            regDataArray[portMacType].fieldLength = 1;
            regDataArray[portMacType].fieldOffset = 6;
        }
        if (PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* set MAC RESET/UNRESET register configuration */
        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* let system stabilise */
        HW_WAIT_MILLISECONDS_MAC(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 10);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacResetStateGet function
* @endinternal
*
* @brief   Get MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] statePtr                 - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - statePtr == NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacResetStateGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *statePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* MAC unit used by port */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    if(PRV_CPSS_PORT_NOT_EXISTS_E == portMacType)
    {
        *statePtr = GT_TRUE;
        return GT_OK;
    }

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, portMacType,
                                            &regAddr);
        offset = 1;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType,
                                             &regAddr);
        offset = 6;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* get MACResetn bit */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                                devNum, portMacNum);
    if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                offset, 1, &value)) != GT_OK)
    {
        return rc;
    }

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        *statePtr = (1 == value) ? GT_FALSE : GT_TRUE;
    }
    else
    {
        *statePtr = BIT2BOOL_MAC(value);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                                      GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port's packet transmission if it is an XOFF packet, or to
*       resume the port's packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is 00-01
*       - Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address
*
*/
static GT_STATUS internal_cpssDxChPortForward802_3xEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_U32 offset;     /* bit number inside register       */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType >= PRV_CPSS_PORT_XG_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((GT_TRUE == enable) &&
       (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_XCAT_FC_FORWARD_NOT_FUNCTIONAL_WA_E)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    value = BOOL2BIT_MAC(enable);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
        offset = 0;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_SERIAL_PARAM_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        offset = 3;
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = offset;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
        PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
        offset = 5;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
        offset = 4;
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
        PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 16;/*<Forward 802.3x FC Enable>*/
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 7;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = 7;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = 1;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 7;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 7;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 1;
    }

    if(prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /****************/
    /*  BR section  */
    /****************/
    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_commandConfig;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 7;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 1;
    }
    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                                      GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port's packet transmission if it is an XOFF packet, or to
*       resume the port's packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is 00-01
*       - Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address
*
*/
GT_STATUS cpssDxChPortForward802_3xEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForward802_3xEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortForward802_3xEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of 802.3x frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port's packet transmission if it is an XOFF packet, or to
*       resume the port's packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is 00-01
*       - Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address
*
*/
static GT_STATUS internal_cpssDxChPortForward802_3xEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 value;      /* register value */
    GT_STATUS rc;      /* return code */
    GT_U32 offset;     /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* type of mac unit of port */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);


    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
            PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) ||
            PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            if(portMacType == PRV_CPSS_PORT_CG_E)
            {
                regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
                offset  = 16;/*<Forward 802.3x FC Enable>*/
            }
            else if(portMacType ==PRV_CPSS_PORT_MTI_100_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
                offset = 7;
            }
            else if(portMacType ==PRV_CPSS_PORT_MTI_400_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
                offset = 7;
            }
            else if(portMacType ==PRV_CPSS_PORT_MTI_CPU_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
                offset = 7;
            }
            else if(portMacType ==PRV_CPSS_PORT_MTI_USX_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
                offset = 7;
            }
            else
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
                offset = 5;
            }
        }
        else
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            offset = 4;
        }
    }
    else
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            offset = 0;
        }
        else
        {
            PRV_CPSS_DXCH_PORT_SERIAL_PARAM_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
            offset = 3;
        }
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset, 1, &value)) != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of 802.3x frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port's packet transmission if it is an XOFF packet, or to
*       resume the port's packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is 00-01
*       - Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address
*
*/
GT_STATUS cpssDxChPortForward802_3xEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForward802_3xEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortForward802_3xEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                                      GT_FALSE: do not forward unknown MAC control frames to
*                                      the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is not 00-01 and not 01-01
*       OR
*       Packet's MAC DA is not 01-80-C2-00-00-01 and not the port's configured
*       MAC Address
*
*/
static GT_STATUS internal_cpssDxChPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);

    if (CPSS_PP_FAMILY_DXCH_XCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].serialParameters1;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

        return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 2, 1, value);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].serialParameters1;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 2;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 7;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 7;
    }
    regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 18;/*<Forward UnKnown FC Enable>*/
    }

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssDxChPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                                      GT_FALSE: do not forward unknown MAC control frames to
*                                      the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is not 00-01 and not 01-01
*       OR
*       Packet's MAC DA is not 01-80-C2-00-00-01 and not the port's configured
*       MAC Address
*
*/
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForwardUnknownMacControlFramesEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortForwardUnknownMacControlFramesEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of unknown MAC control frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is not 00-01 and not 01-01
*       OR
*       Packet's MAC DA is not 01-80-C2-00-00-01 and not the port's configured
*       MAC Address
*
*/
static GT_STATUS internal_cpssDxChPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 value;      /* register value */
    GT_STATUS rc;      /* return code */
    GT_U32 offset;     /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* type of mac unit of port */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].serialParameters1;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 2, 1, &value)) != GT_OK)
            return rc;
    }
    else
    {
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

        if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) &&
           (portMacType < PRV_CPSS_PORT_XG_E))
        {
            offset = 2;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].serialParameters1;
        }
        else
        {
            if(portMacType == PRV_CPSS_PORT_CG_E)
            {
                regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
                offset  = 18;/*<Forward UnKnown FC Enable>*/
            }
            else
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
                if(portMacType >= PRV_CPSS_PORT_XG_E)
                {
                    offset = 7;
                }
                else
                {
                    offset = 2;
                }
            }
        }

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset, 1, &value)) != GT_OK)
            return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssDxChPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of unknown MAC control frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is not 00-01 and not 01-01
*       OR
*       Packet's MAC DA is not 01-80-C2-00-00-01 and not the port's configured
*       MAC Address
*
*/
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortForwardUnknownMacControlFramesEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortForwardUnknownMacControlFramesEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortInbandAutonegMode function
* @endinternal
*
* @brief   Set inband autoneg mode accordingly to required ifMode
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when <InBandAnEn> is set to 1.
*       Not relevant for the CPU port.
*       This field may only be changed when the port link is down.
*       In existing devices inband auto-neg. disabled by default.
*
*/
GT_STATUS prvCpssDxChPortInbandAutonegMode
(
    IN GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
{
    GT_U32      regAddr;/* register address */
    GT_U32      value;  /* register value */
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
            value = 1;
            break;

        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            value = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;

    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal prvCpssDxChPortTraceDelay function
* @endinternal
*
* @brief   Make delay and trace it
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] millisec                 -  the delay in millisec
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
*   NOTE: called from prvCpssCommonPortTraceDelay(...)
*/
GT_STATUS prvCpssDxChPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
)
{
    if(millisec > (PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(tickTimeMsec)/2))
    {/* if required delay more than half of tick time do OS delay */
        HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,millisec);
    }
    else
    {/* if delay is short do busy wait */
        if (prvCpssDrvTraceHwDelay[devNum] == GT_TRUE)
        {
            cpssTraceHwAccessDelay(devNum,portGroupId,millisec);
        }
        {
            GT_STATUS   rc; /* return code */
            GT_U32  startTimeStamp, /* value of time stamp register on delay start */
                    currentTimeStamp;/* current value of time stamp register */
            GT_U32  diff; /* emount of core clocks passed */
            GT_U32  passed; /* delay time passed in millisec */
            GT_U32  value;  /* dummy - user defined register value */
            GT_U32  coreClockTime; /* in millisec */
            GT_U32  regAddr;    /* address of register to read */
            GT_U32  secondsStart, nanoSecondsStart, secondsCurrent, nanoSecondsCurrent;

            coreClockTime = PRV_CPSS_PP_MAC(devNum)->coreClock * 1000;
            /* register must be not in MG unit */
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)/* Falcon Z2 */
                {
                    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
                    do
                    {
                        cpssOsTimeRT(&secondsCurrent,&nanoSecondsCurrent);
                        if(nanoSecondsCurrent >= nanoSecondsStart)
                            passed = nanoSecondsCurrent-nanoSecondsStart;
                        else
                            passed = (1000000000 - nanoSecondsStart) + nanoSecondsCurrent;
                        passed = passed/1000000; /*from nano to mili*/
                    }while(passed < millisec);
                    return GT_OK;
                }
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalControl;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.
                                                            bridgeCommandConfig0;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];
            }
            if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            /* read register to cause time stamp register update */
            if (prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                    regAddr, &value) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }

            rc = cpssDxChVntPortGroupLastReadTimeStampGet(devNum, (1<<portGroupId),
                                                          &startTimeStamp);
            if(rc != GT_OK)
            {
                return rc;
            }

            do
            {
                if (prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                        regAddr, &value) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }

                rc = cpssDxChVntPortGroupLastReadTimeStampGet(devNum, (1<<portGroupId),
                                                              &currentTimeStamp);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* calculate difference between current and start */
                if (currentTimeStamp > startTimeStamp)
                {
                    diff = currentTimeStamp - startTimeStamp;
                }
                else
                {/* take care of wraparound */
                   diff = (0xFFFFFFFFU - startTimeStamp) + currentTimeStamp;
                }

                passed = diff/coreClockTime;

                #ifdef ASIC_SIMULATION
                    /* single iteration is enough ... as we 'checked' that above
                       logic access existing registers */
                    passed = millisec;
                #endif /*ASIC_SIMULATION*/
            }while(passed < millisec);
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortDevInfo
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    *devId = PRV_CPSS_PP_MAC(devNum)->devType;
    *revNum = PRV_CPSS_PP_MAC(devNum)->revision;
    return GT_OK;
}

PRV_CPSS_DXCH_PORT_CPLL_CONFIG_STC prvCpssDxChPortCpllConfigArr[PRV_CPSS_MAX_PP_DEVICES_CNS][PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM];

/**
* @internal prxCpssDxChPortCpllDbInit function
* @endinternal
*
* @brief   CPLL database initialization
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS prxCpssDxChPortCpllDbInit
(
    IN GT_U8 devNum
)
{
    GT_U32  serdesNum;  /* serdes loop iterator */
    GT_U32  numOfSerdes = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    for(serdesNum = 0; serdesNum < numOfSerdes ; serdesNum++)
    {
        /* CPLL in BobK A1 and above receives input clock 25Mhz */
        if(((PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) && (PRV_CPSS_PP_MAC(devNum)->revision > 0)) ||
           PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inFreq = MV_HWS_25MHzIN;
        }
        else
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inFreq = MV_HWS_156MHz_IN;
        }

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].outFreq = MV_HWS_78MHz_OUT;
        }
        else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].outFreq = MV_HWS_156MHz_OUT;
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inFreq = MV_HWS_156MHz_IN;
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].outFreq = MV_HWS_312MHz_OUT;
        }
        else
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].outFreq = MV_HWS_156MHz_OUT;
        }
        prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inputSrcClk = MV_HWS_CPLL;


        prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_FALSE;
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* all SERDESes in the loop of 'numOfSerdes' are valid */
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_TRUE;
        }
        else
        {
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                    {
                        switch(PRV_CPSS_PP_MAC(devNum)->devType)
                        {
                            case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
                                if((serdesNum <= 11)                    ||
                                   (serdesNum == 20)                    ||
                                   (serdesNum >= 24 && serdesNum <= 35))
                                {
                                    prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_TRUE;
                                }
                                break;

                            case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                                if((serdesNum == 20)                    ||
                                   (serdesNum >= 24 && serdesNum <= 35))
                                {
                                    prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_TRUE;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                case CPSS_PP_FAMILY_DXCH_AC3X_E:
                    if(serdesNum <= 32)
                    {
                        prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_TRUE;
                    }
                    break;

                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                    if(serdesNum < 74)
                    {
                        prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_TRUE;
                    }
                    break;
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    if(serdesNum < 73)
                    {
                        prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_TRUE;
                    }
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Illegal device family : %d\n", PRV_CPSS_PP_MAC(devNum)->devFamily);
            }
        }

        if(cpssDeviceRunCheck_onEmulator())
        {
            /* The emulator not supports CPLL registers */
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_FALSE;
        }
    }

    for(/*continue*/; serdesNum < PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM; serdesNum++)
    {
        prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid = GT_FALSE;
    }


    return GT_OK;
}
/**
* @internal prvCpssDxChConfigSerdesRefClock function
* @endinternal
*
* @brief   Configure the Serdes reference clock
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS prvCpssDxChConfigSerdesRefClock
(
    IN GT_U8 devNum
)
{
    GT_STATUS                   rc;
    MV_HWS_REF_CLOCK_SUP_VAL    refClock;
    GT_U32                      regAddr;
    GT_U32                      fieldData;

    rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum,&refClock);
    if(GT_OK != rc)
    {
        return rc;
    }
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            regAddr   = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl19;
            fieldData = (MHz_25 == refClock)? 0 : 1 ;
            rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,7,1,fieldData);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            regAddr   = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl31;
            fieldData = (MHz_25 == refClock)? 0 : 1 ;
            rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,5,1,fieldData);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

GT_VOID debugCpllInitFlagEnableSet()
{
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(aldrinDebugCpllInitFlag,GT_TRUE);
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(bc3DebugCpllInitFlag,GT_TRUE);
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(bobkDebugCpllInitFlag,GT_TRUE);

    return;
}

GT_VOID debugCpllInitFlagDisableSet()
{
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(aldrinDebugCpllInitFlag,GT_FALSE);
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(bc3DebugCpllInitFlag,GT_FALSE);
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(bobkDebugCpllInitFlag,GT_FALSE);

    return;
}

GT_STATUS debugEmulatorTimeOutSet
(
    GT_U32 timeOutType,
    GT_U32 timeOutValue
)
{
   switch(timeOutType)
   {
      case 1:
        PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(debug_sip6_TO_txq_polling_interval, timeOutValue);
        break;
      case 2:
        PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(debug_sip6_TO_polling_iterations, timeOutValue);
        break;
      case 3:
        PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(debug_sip6_TO_after_txq_drain, timeOutValue);
        break;
      case 4:
        PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(debug_sip6_TO_after_mac_disabled, timeOutValue);
        break;
      default:
        /*do nothing.Only print*/
        cpssOsPrintf("Display only :\n");
        break;
   }
   cpssOsPrintf("\n1.TO_txq_polling_interval  %d\n", PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_txq_polling_interval));
   cpssOsPrintf("2.TO_polling_iterations  %d\n\n", PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_polling_iterations));
   cpssOsPrintf("3.TO_after_txq_drain  %d\n", PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_after_txq_drain));
   cpssOsPrintf("4.TO_after_mac_disabled  %d\n", PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(debug_sip6_TO_after_mac_disabled));


   return GT_OK;
}


/**
* @internal prvCpssDxChPortCpllConfig function
* @endinternal
*
* @brief   CPLL initialization
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDxChPortCpllConfig
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc;                         /* return code */
    GT_U32      serdesNum;                  /* serdes iterator */
    GT_U32      regAddr;                    /* register address */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /*system Recovery Info*/

    rc = prxCpssDxChPortCpllDbInit(devNum);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) ||
        ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
         (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E))
       )
    {
        return GT_OK;
    }
    /************************************************************************
     * Internal CPLL initialization:
     *      if CPLL_x (x may be 0,1,2) was not initialized by internal CPU,
     *      it should be done before HWS initialization, because Serdes FW
     *      upload needs clock from CPLL if external reference clock does
     *      not exists.
     ************************************************************************/

    /* Aldrin Z0 CPLL1 initialization */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX8332_Z0_CNS)
        {
            /* configure CPLL1 to bypass */
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 9, 1, 0);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            /* configure MUX to choose external ref clock from "Cetus side" */
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 11, 1, 1);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            return GT_OK;
        }

        if(PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(bobkDebugCpllInitFlag) != GT_TRUE)
        {
            return GT_OK;
        }

#ifndef ASIC_SIMULATION
        /* In order to support 5G_BaseR port mode it is needed to initialize CPLL2 unit.
           No need to configure SERDES to work by default with CPLL, so only CPLL_INIT_API is called.
           Also no need to initialize CPLL_0 for SERDESes 0-11 -> there is no XLG MAC there, only
           CPLL_2 that works with SERDESes 24-35 is relevant */
        if(PRV_CPSS_PP_MAC(devNum)->revision > 0)
        {
            rc = cpssDxChPortCpllCfgInit(devNum, 2, CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_25M_E, CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_78M_E);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
#endif

        return GT_OK;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if(PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(aldrinDebugCpllInitFlag) != GT_TRUE)
        {
            return GT_OK;
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* do nothing */
    }
    else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(bc3DebugCpllInitFlag) != GT_TRUE)
        {
            /* configure all serdeses to use external ref clock */
            for (serdesNum = 0; serdesNum < PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM; serdesNum++)
            {
                if(prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid == GT_TRUE)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg1;
                    rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,8,1,1);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
            }
            return GT_OK;
        }
        else
        {
            for (serdesNum = 0; serdesNum < PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM; serdesNum++)
            {
                if(prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid == GT_TRUE)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg1;
                   /* Infact this register is SERDES External COnfiguration0 register. The name serdesExternalReg1 is from old production  */
                    rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,11,2,2);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
            }

            /* When BC3/Armstrong uses CPLL (which it's the default SW configuration) it is needed to  configure the following:
                a. PECL_EN=0 /DFX/Units/DFX Server Registers/Device General Control 1 [0] = 0x0 (PECL Disable)
                b. In addition, the software must configure it back to normal, once using on board ref clocks.
            */
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl1;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 1, 0);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

        }
    }

    for (serdesNum = 0; serdesNum < PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM; serdesNum++)
    {
        if(prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid == GT_TRUE)
        {
            rc = mvHwsSerdesClockGlobalControl(devNum, 0, serdesNum,
                                               prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inputSrcClk,
                                               prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inFreq,
                                               prvCpssDxChPortCpllConfigArr[devNum][serdesNum].outFreq);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortCpllCfgInit function
* @endinternal
*
* @brief   CPLL unit initialization routine.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] cpllNum                  - CPLL unit index
*                                      (APPLICABLE RANGES: Caelum            0..2;
*                                      Aldrin, AC3X, Bobcat3; Aldrin2   0)
* @param[in] inputFreq                - CPLL input frequency
* @param[in] outputFreq               - CPLL output frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
static GT_STATUS internal_cpssDxChPortCpllCfgInit
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       cpllNum,
    IN CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT      inputFreq,
    IN CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT     outputFreq
)
{
    GT_STATUS                       rc;         /* return code */
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq;     /* input frequency in HWS format */
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq;    /* output frequency in HWS format */
    GT_U32                          serdesNum;  /* serdes loop iterator */
    GT_BOOL                         isFound = GT_FALSE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);

    switch(inputFreq)
    {
        case CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_25M_E:
            inFreq = MV_HWS_25MHzIN;
            break;
        case CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_156M_E:
            inFreq = MV_HWS_156MHz_IN;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(outputFreq)
    {
        case CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_78M_E:
            outFreq = MV_HWS_78MHz_OUT;
            break;
        case CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_156M_E:
            outFreq = MV_HWS_156MHz_OUT;
            break;
        case CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_200M_E:
            outFreq = MV_HWS_200MHz_OUT;
            break;
        case CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_312M_E:
            outFreq = MV_HWS_312MHz_OUT;
            break;
        case CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_161M_E:
            outFreq = MV_HWS_161MHz_OUT;
            break;
        case CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_164M_E:
            outFreq = MV_HWS_164MHz_OUT;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if(cpllNum != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = mvHwsGeneralSrcClockModeConfigSet(devNum, 0, 0, (1<<8), 0, CPLL0, inFreq, outFreq);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) && (PRV_CPSS_PP_MAC(devNum)->revision > 0))
    {
        if(cpllNum == 0)
        {
            rc = mvHwsBobKSrcClockModeConfigSet(devNum, 0,  0, 0, 0, 0xFFFF, CPLL0, inFreq, outFreq);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else if(cpllNum == 2)
        {
            rc = mvHwsBobKSrcClockModeConfigSet(devNum, 0, 24, 0, 0, 0xFFFF, CPLL2, inFreq, outFreq);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(cpllNum == 0)
        {
            rc = mvHwsGeneralSrcClockModeConfigSet(devNum, 0, 0, 0, 0, CPLL0, inFreq, outFreq);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else if(cpllNum == 1)
        {
            rc = mvHwsGeneralSrcClockModeConfigSet(devNum, 0, 36, 0, 0, CPLL1, inFreq, outFreq);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }


    for(serdesNum = 0; serdesNum < PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM; serdesNum++)
    {
        if(prvCpssDxChPortCpllConfigArr[devNum][serdesNum].valid != GT_TRUE)
        {
            continue;
        }

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            isFound = GT_TRUE;
        }
        else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* currently Falcon as BC3 */
            CPSS_TBD_BOOKMARK_FALCON
            if(cpllNum == 0)
            {
                isFound = ((serdesNum < 36) || (serdesNum == 72)) ? GT_TRUE : GT_FALSE;
            }
            else if(cpllNum == 1)
            {
                isFound = ((serdesNum < 72) || (serdesNum == 73)) ? GT_TRUE : GT_FALSE;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            if(cpllNum == 0)
            {
                isFound = (serdesNum < 24) ? GT_TRUE : GT_FALSE;
            }
            else if(cpllNum == 1)
            {
                isFound = (serdesNum >= 24 && serdesNum < 73) ? GT_TRUE : GT_FALSE;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

        }
        else /* BobK */
        {
            if(cpllNum == 0)
            {
                isFound = (serdesNum < 12) ? GT_TRUE : GT_FALSE;
            }
            else if(cpllNum == 2)
            {
                isFound = ((serdesNum >= 24 && (serdesNum <= 35)) || (serdesNum == 20)) ? GT_TRUE : GT_FALSE;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        if(isFound == GT_TRUE)
        {
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].inFreq = inFreq;
            prvCpssDxChPortCpllConfigArr[devNum][serdesNum].outFreq = outFreq;
        }
    }


    return GT_OK;
}

/**
* @internal cpssDxChPortCpllCfgInit function
* @endinternal
*
* @brief   CPLL unit initialization routine.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] cpllNum                  - CPLL unit index
*                                      (APPLICABLE RANGES: Caelum            0..2;
*                                      Aldrin, AC3X, Bobcat3; Aldrin2   0)
* @param[in] inputFreq                - CPLL input frequency
* @param[in] outputFreq               - CPLL output frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
GT_STATUS cpssDxChPortCpllCfgInit
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       cpllNum,
    IN CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT      inputFreq,
    IN CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT     outputFreq
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCpllCfgInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpllNum, inputFreq, outputFreq));

    rc = internal_cpssDxChPortCpllCfgInit(devNum, cpllNum, inputFreq, outputFreq);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpllNum, inputFreq, outputFreq));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
GT_STATUS prvCpssDxChPortDevDriverGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              portGroupId,
    IN  GT_U32              smiIntr,
   /* IN  const char        *smiMasterPath,*/
    IN  GT_U32              slaveSmiPhyId,
    /*OUT CPSS_HW_INFO_STC    *hwInfoPtr*/
    OUT CPSS_HW_DRIVER_STC  **ssmiPtr
)
{
    GT_STATUS                   rc;
    CPSS_HW_DRIVER_STC          *smi = NULL; /*falcon smi master */
    PRV_CPSS_GEN_PP_CONFIG_STC  *ppConfig;
    GT_U32                      data;
    CPSS_SYSTEM_RECOVERY_INFO_STC oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_BOOL                     haSkipInit = GT_FALSE;
    /* save current recovery state */
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    ppConfig = PRV_CPSS_PP_MAC(devNum);
    if (ppConfig->smiMasters[portGroupId][smiIntr] == NULL){
        cpssOsPrintf("SMI device %d, Not used\n", smiIntr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    smi = ppConfig->smiMasters[portGroupId][smiIntr];

      /* smi = cpssHwDriverLookup(smiMasterPath);
    if(smi == NULL)
    {
        cpssOsPrintf("ERROR: SMI driver not found\n\n");
        return GT_NOT_FOUND;
    }*/
    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
    smi->read(smi, slaveSmiPhyId, 2/*SMI_DEVICE_IDENTIFIER1_REG_ADDR_CNS*/, &data, 1);
#ifdef ASIC_SIMULATION
    data = 0x0141;
#endif /*ASIC_SIMULATION*/
    if (data != 0x0141) /* not marvell slave smi device */
    {
        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
        cpssOsPrintf("Not MARVELL slave SMI device found, SMI slave addr = 0x%x, id=0x%x\n", slaveSmiPhyId, data);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    /*else
    {
        cpssOsPrintf("MARVELL slave SMI device found, SMI slave addr = 0x%x\n", slaveSmiPhyId);
    }*/
 /* old api -   rc = cpssHwDriverSlaveSmiConfigure(smiMasterPath, slaveSmiPhyId, hwInfoPtr);
    if ((rc != GT_OK) || (hwInfoPtr->driver == NULL))
    {
        cpssOsPrintf("cpssHwDriverSlaveSmiConfigure returned with NULL pointer for ravenIdx = %d\n", slaveSmiPhyId);
        return GT_BAD_PTR;
    }*/
    if (oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        haSkipInit = GT_TRUE;
    }
    *ssmiPtr = cpssHwDriverSip5SlaveSMICreateDrv(smi, slaveSmiPhyId,haSkipInit);
    if (*ssmiPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
     /*cpssOsSprintf(drvName, "SSMI@%d", slaveSmiPhyId);
    cpssHwDriverRegister(ssmi, drvName);*/

    return GT_OK;
}
/**
* @internal prvCpssDxChPortProprietaryHighSpeedSet function
* @endinternal
*
* @brief   Configure the CPU ports resources so Proprietary High
*         speed ports will have enough bencdwitdh.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman;
*         xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X;
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
GT_STATUS prvCpssDxChPortProprietaryHighSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    GT_BOOL                  enable
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32 i, numOfCpuPorts;
    CPSS_PORT_SPEED_ENT             portSpeed;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(enable)
    {
        portSpeed = CPSS_PORT_SPEED_1000_E;
        ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
    }
    else
    {
        portSpeed = CPSS_PORT_SPEED_10000_E;
        ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
    }

    if ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        rc = prvCpssDxChPortMappingCPUPortGet(CAST_SW_DEVNUM(devNum),/*OUT*/cpuPortNumArr,&numOfCpuPorts);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to get CPU ports list");

        }
        for (i = 0; i < numOfCpuPorts; i++)
        {
            rc = cpssDxChPortPizzaArbiterIfConfigSet(CAST_SW_DEVNUM(devNum), cpuPortNumArr[i], portSpeed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to set cpssDxChPortPizzaArbiterIfConfigSet");
            }
            rc = cpssDxChPortResourcesConfigSet(CAST_SW_DEVNUM(devNum), cpuPortNumArr[i], ifMode , portSpeed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to set cpssDxChPortPizzaArbiterIfConfigSet");
            }
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Device not supports this mode");
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMgrInit function
* @endinternal
*
* @brief   Init Port manager
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
* @param[in] devNum                   - physical device number
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMgrInit
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  PRV_CPSS_PORT_PM_FUNC_PTRS *cpssPmFuncPtr

)
{
    GT_STATUS rc = GT_OK;
    GT_U32 port, portMacNum;
    GT_BOOL isCpu = GT_FALSE;
    GT_BOOL macFound;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(port = 0 ; port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum) ; port++)
    {
        if ((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)||(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
        {
            macFound = GT_FALSE;
            if(!prvCpssDxChPortRemotePortCheck(devNum,port))
            {
                if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
                {
                    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, port,
                                                                       PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
                                                                       &portMacNum);
                    if(GT_OK == rc)
                    {
                        macFound = GT_TRUE;
                    }
                }
                else
                {
                    portMacNum = port;
                    if(PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portMacNum))
                    {
                        macFound = GT_TRUE;
                    }
                }

                if(macFound == GT_TRUE)
                {
                    rc = cpssDxChPortEnableSet(CAST_SW_DEVNUM(devNum), port, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortEnableSet failed for port=%d, rc = %d", port, rc);
                    }
                }
            }
        }

        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Valid for Bobcat3 and Aldrin2*/
        {
            rc = cpssDxChPortPhysicalPortMapIsCpuGet(CAST_SW_DEVNUM(devNum), port, &isCpu);
            if(rc != GT_OK)
            {
                return rc;
            }
            portLinkStatusState = (isCpu?CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E:CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);

            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /*in case of HS only cpu ports needs to be update */
                if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E || isCpu == GT_TRUE)
                {
                    rc = cpssDxChBrgEgrFltPortLinkEnableSet(CAST_SW_DEVNUM(devNum), port, portLinkStatusState);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "egf filter failed=%d, port=%d", rc,port);
                    }
                }
            }
            else
            {
                rc = prvCpssDxChHwEgfEftFieldSet(CAST_SW_DEVNUM(devNum),port,
                                                 PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                                                 PRV_CPSS_DXCH_EGF_CONVERT_MAC(portLinkStatusState));
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChPortPmFuncBind(devNum, cpssPmFuncPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPmFuncBind failed=%d", rc);
        }
    }

    return GT_OK;
}

GT_VOID prvCpssDxChPortMngEngineCallbacksInit
(
    IN GT_U8    devNum
)
{

    PRV_CPSS_PORT_FUNC_PTRS_STC * ppCommonPortFuncPtrs = &(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc));

    ppCommonPortFuncPtrs->ppCheckAndGetMacFunc = prvCpssDxChPortCheckAndGetMacNumberWrapper;
    ppCommonPortFuncPtrs->ppConvertMacToPortFunc = prvCpssDxChPortConvertMacToPortWrapper;
    ppCommonPortFuncPtrs->ppLinkStatusGetFunc = prvCpssDxChPortLinkStatusGetWrapper;
    ppCommonPortFuncPtrs->ppRemoteFaultSetFunc = prvCpssDxChPortFaultSendSetWrapper;
    ppCommonPortFuncPtrs->ppForceLinkDownSetFunc = prvCpssDxChPortForceLinkDownEnableSetWrapper;
    ppCommonPortFuncPtrs->ppPortMacEnableSetFunc = prvCpssDxChPortEnableSetWrapper;
    ppCommonPortFuncPtrs->ppPortMacEnableGetFunc = prvCpssDxChPortEnableGetWrapper;
    ppCommonPortFuncPtrs->ppCgConvertersGetFunc = prvCpssDxChCgConvertersRegAddrGetWrapper;
    ppCommonPortFuncPtrs->ppCgConvertersStatus2GetFunc = prvCpssDxChCgConvertersStatus2RegAddrGetWrapper;
    ppCommonPortFuncPtrs->ppSpeedGetFromExtFunc = prvCpssDxChPortSpeedGetWrapper;
    ppCommonPortFuncPtrs->ppPortManagerDbGetFunc = prvCpssDxChPortManagerDbGetWrapper;
    ppCommonPortFuncPtrs->ppMpcs40GCommonStatusRegGetFunc = prvCpssDxChMPCS40GRegAddrGetWrapper;
    ppCommonPortFuncPtrs->ppMacDmaModeRegAddrGetFunc = prvCpssDxChMacDmaModeRegAddrGetWrapper;
    ppCommonPortFuncPtrs->ppLinkStatusChangedNotifyFunc = prvCpssDxChLinkStatusChangedWAs;
    ppCommonPortFuncPtrs->ppXlgMacMaskAddrGetFunc = prvCpssDxChXlgMacIntMaskGet;
    ppCommonPortFuncPtrs->ppMacConfigClearFunc = prvCpssDxChPortMacConfigurationClearWrapper;
    ppCommonPortFuncPtrs->ppMacConfigurationFunc = prvCpssDxChPortMacConfigurationWrapper;
    /* common functions bind - (currently for Port Manager use) - HwInit and PortMapping related*/
    ppCommonPortFuncPtrs->ppNumOfSerdesGetFunc = prvCpssDxChPortSerdesGetWrapper;
    ppCommonPortFuncPtrs->ppMacConvertFunc = prvCpssDxChPortEventPortMapConvert;
    ppCommonPortFuncPtrs->ppPortRemoteFaultConfigGetFunc = NULL;
    ppCommonPortFuncPtrs->ppPortLkbSetFunc = NULL;
    ppCommonPortFuncPtrs->ppPortForceLinkPassSetFunc = cpssDxChPortForceLinkPassEnableSet;
    ppCommonPortFuncPtrs->ppPortForceLinkPassGetFunc = cpssDxChPortForceLinkPassEnableGet;
    /* AP port APIs */
    ppCommonPortFuncPtrs->ppPortApConfigSetFunc = prvCpssDxChPortApPortConfigSetConvert;
    ppCommonPortFuncPtrs->ppPortApStatusGetFunc = prvCpssDxChPortApPortStatusGetConvert;
    ppCommonPortFuncPtrs->ppPortApSerdesTxParamsOffsetSetFunc = cpssDxChPortApSerdesTxParametersOffsetSet;

    ppCommonPortFuncPtrs->ppPortMacLoopbackModeSetFunc = prvCpssDxChPortInternalLoopbackEnableSetWrapper;
    ppCommonPortFuncPtrs->ppPortPcsLoopbackModeSetFunc = prvCpssDxChPortPcsLoopbackModeSetWrapper;
    ppCommonPortFuncPtrs->ppPortSerdesLoopbackModeSetFunc = prvCpssDxChPortSerdesLoopbackModeSetWrapper;
    ppCommonPortFuncPtrs->ppPortSerdesPolaritySetFunc = prvCpssDxChPortSerdesLanePolaritySetWrapper;
    ppCommonPortFuncPtrs->ppPortProprietaryHighSpeedPortsSetFunc = prvCpssDxChPortProprietaryHighSpeedSet;
    ppCommonPortFuncPtrs->ppPortSerdesTuneResultGetFunc = prvCpssDxChPortSerdesAutoTuneResultsGetWrapper;
    ppCommonPortFuncPtrs->ppAutoNeg1GSgmiiFunc = prvCpssDxChAutoNeg1GSgmiiWrapper;
    ppCommonPortFuncPtrs->ppUsxReplicationSetFunc = prvCpssDxChUsxReplicationSetWrapper;
    ppCommonPortFuncPtrs->ppPortMgrInitFunc = prvCpssDxChPortMgrInit;
    ppCommonPortFuncPtrs->ppPortFastLinkDownSetFunc = cpssDxChPortFastLinkDownEnableSet;
    ppCommonPortFuncPtrs->ppPortExtraOperationsSetFunc = prvCpssDxChPortExtraOperationsSet;
    ppCommonPortFuncPtrs->ppPortLowPowerEnableFunc = prvCpssDxChPortSerdesLowPowerModeEnableWrapper;
    ppCommonPortFuncPtrs->ppRemotePortCheckFunc = prvCpssDxChPortRemotePortCheck;
    ppCommonPortFuncPtrs->ppPortPreemptionParamsSetFunc = prvCpssDxChPortPreemptionParamsSetWrapper;
    ppCommonPortFuncPtrs->ppPortPtpDelayParamsSetFunc = prvCpssDxChPortPtpDelayParamsSetWrapper;
    ppCommonPortFuncPtrs->ppPortEnableWaWithLinkStatusSet = cpssDxChPortEnableWaWithLinkStatusSet;

}


/**
* @internal prvCpssDxChPortMngEngineInit function
* @endinternal
*
* @brief   Initialize port manager related databases and
*          function pointers
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - port object allocation failed
* @retval GT_FAIL                  - wrong devFamily
*/
GT_STATUS prvCpssDxChPortMngEngineInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;


    prvCpssDxChPortMngEngineCallbacksInit(devNum);

    rc = prvCpssDxChPortManagerDbGetWrapper(devNum,&tmpPortManagerDbPtr);
    if (rc != GT_OK) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
      /*global parameters*/
    tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeInterval = PORT_MANAGER_SIGNAL_DETECT_INTERVAL_CNS;
    tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeMinWindowSize = PORT_MANAGER_SIGNAL_DETECT_WINDOW_SIZE_CNS;
    tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdCheckTimeExpired = PORT_MANAGER_SIGNAL_DETECT_TIME_EXPIRED_CNS;
    tmpPortManagerDbPtr->globalParamsCfg.anResolutionBit_40G = CPSS_PORT_MANAGER_AN_RESOLUTION_BIT_32_E;

    return GT_OK;
}

extern GT_U32 falcon_force_stop_init_after_d2d_init_get(void);

/**
* @internal prvCpssDxChPortIfCfgInit function
* @endinternal
*
* @brief   Initialize port interface mode configuration method in device object
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - port object allocation failed
* @retval GT_FAIL                  - wrong devFamily
*/
GT_STATUS prvCpssDxChPortIfCfgInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;      /* device family */
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portTypeOptions;/* options available for port */
    GT_U32  ticks;  /* system clock rate - ticks per second */


    if((PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_START_DXCH_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_END_DXCH_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily - CPSS_PP_FAMILY_START_DXCH_E - 1;

    /* port ifMode function obj init*/
    rc = prvCpssDxChPortIfFunctionsObjInit(devNum,  &(PORT_OBJ_FUNC(devNum)), PRV_CPSS_PP_MAC(devNum)->devFamily);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortIfFunctionsObjInit failed=%d", rc);
    }

    /* port Serdes function obj init*/
    rc = prvCpssDxChPortSerdesFunctionsObjInit(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortSerdesFunctionsObjInit failed=%d", rc);
    }

#if 0
    for(ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E; ifMode < CPSS_PORT_INTERFACE_MODE_NA_E; ifMode++)
    {
        PRV_CPSS_DXCH_DEV_OBJ_MAC(devNum)->portPtr->setPortInterfaceMode[ifMode] = portIfModeFuncPtrArray[devFamily][ifMode];
    }

    PRV_CPSS_DXCH_DEV_OBJ_MAC(devNum)->portPtr->getPortInterfaceMode = portIfModeGetFuncPtrArray[devFamily];
#endif
    for(portTypeOptions = PRV_CPSS_XG_PORT_XG_ONLY_E; portTypeOptions < PRV_CPSS_XG_PORT_OPTIONS_MAX_E; portTypeOptions++)
    {
        PORT_OBJ_FUNC(devNum).setPortSpeed[portTypeOptions] =  portSpeedSetFuncPtrArray[devFamily][portTypeOptions];
    }

    (PORT_OBJ_FUNC(devNum)).getPortSpeed = portSpeedGetFuncPtrArray[devFamily];

    (PORT_OBJ_FUNC(devNum)).setSerdesPowerStatus = portSerdesPowerStatusSetFuncPtrArray[devFamily];

    /* common functions bind - (currently for Port Manager use) - porCtrl related*/
    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = prvCpssDxChPortMngEngineInit(devNum);
        if (rc != GT_OK) {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

    }
    /* calculate length of tick in mSec for exact delay function */
    cpssOsGetSysClockRate(&ticks);
    PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(tickTimeMsec, 1000/ticks);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        GT_STATUS       rc;          /* return code */
        HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */
        CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboPortsInfoArray; /* ptr to combo
                                ports array for shorten code and quicker access */
        GT_U32  i, j;   /* iterators */

        cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));

        funcPtrsStc.osExactDelayPtr = prvCpssCommonPortTraceDelay;/*common code for DX,PX*/
        funcPtrsStc.osTimerWkPtr    = cpssOsTimerWkAfter;
        funcPtrsStc.osMemSetPtr     = cpssOsMemSet;
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
        funcPtrsStc.osFreePtr       = myCpssOsFree;
        funcPtrsStc.osMallocPtr     = myCpssOsMalloc;
#else  /*! OS_MALLOC_MEMORY_LEAKAGE_DBG */
        funcPtrsStc.osFreePtr       = cpssOsFree;
        funcPtrsStc.osMallocPtr     = cpssOsMalloc;
#endif /*! OS_MALLOC_MEMORY_LEAKAGE_DBG */
        funcPtrsStc.osMemCopyPtr    = (MV_OS_MEM_COPY_FUNC)cpssOsMemCpy;
        funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;
        funcPtrsStc.serverRegSetAccess = cpssDrvHwPpResetAndInitControllerWriteReg;
        funcPtrsStc.serverRegGetAccess = cpssDrvHwPpResetAndInitControllerReadReg;
        funcPtrsStc.serverRegFieldSetAccess = cpssDrvHwPpResetAndInitControllerSetRegField;
        funcPtrsStc.serverRegFieldGetAccess = cpssDrvHwPpResetAndInitControllerGetRegField;
        funcPtrsStc.timerGet = cpssOsTimeRT;
        funcPtrsStc.osStrCatPtr = cpssOsStrCat;
        funcPtrsStc.registerSetAccess = (MV_REG_ACCESS_SET)genRegisterSet;
        funcPtrsStc.registerGetAccess = (MV_REG_ACCESS_GET)genRegisterGet;
        funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;

        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            funcPtrsStc.serdesRegGetAccess =
                (MV_SERDES_REG_ACCESS_GET)prvCpssGenPortGroupSerdesReadRegBitMask;
            funcPtrsStc.serdesRegSetAccess =
                (MV_SERDES_REG_ACCESS_SET)prvCpssGenPortGroupSerdesWriteRegBitMask;
            (GT_VOID)multiLaneConfigSupport(GT_FALSE);

            hwsFalconTypeSet(devNum, HWS_FALCON_DEV_NONE_E);

            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                CPSS_LOG_INFORMATION_MAC("Calling: hwsFalconAsBobcat3Set()");

                /* state that the HWS for BC3 should support Falcon */
                hwsFalconAsBobcat3Set(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);

                switch (PRV_CPSS_PP_MAC(devNum)->devType)
                {
                    case CPSS_98EX5610_CNS:     /* 6.4T package*/
                        hwsFalconTypeSet(devNum, HWS_ARMSTRONG2_DEV_98EX5610_E);
                        /* Ravens 0,2,5,6,7 connected to balls, ravens 1,3,4 present */
                        /* all ravens should be initialized and power reducted       */
                        /* for 5611 without ravens 1,3,4 bitmap should be 0xE5       */
                        hwsFalconSetRavenDevBmp(0xE5);
                        break;

                    case CPSS_98CX8512_CNS:     /* 6.4T package*/
                        hwsFalconTypeSet(devNum, HWS_ARMSTRONG2_DEV_98CX8512_E);
                        /* Ravens 0,2,5,6,7 connected to balls, ravens 1,3,4 present */
                        /* all ravens should be initialized and power reducted       */
                        /* for 5611 without ravens 1,3,4 bitmap should be 0xE5       */
                        hwsFalconSetRavenDevBmp(0xE5);
                        break;

                    case CPSS_98CX8530_CNS:     /* 6.4T package*/
                        /* Ravens 0,2,5,6,7 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8530_E);
                        hwsFalconSetRavenDevBmp(0xE5);
                        break;

                    case CPSS_98CX8522_CNS:     /* 6.4T package*/
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8522_E);
                        break;

                    case CPSS_98CX8542_CNS:     /*Clone of 12.8T*/
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8542_E);
                        break;

                    case CPSS_98CX8525_CNS:     /* 4T package */
                        /* Ravens 0,2,5,7 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8525_E);
                        hwsFalconSetRavenDevBmp(0xA5);
                        break;

                    case CPSS_98CX8520_CNS:     /* 6.4T package*/
                        /* Ravens 0,2,5,7 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8520_E);
                        hwsFalconSetRavenDevBmp(0xA5);
                        break;

                    case CPSS_98CX8535_CNS:     /* 4T package */
                    case CPSS_98CX8535_H_CNS:
                        /* Ravens 0,2,4,5,7 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8535_E);
                        hwsFalconSetRavenDevBmp(0xB5);
                        break;

                    case CPSS_98CX8514_CNS:     /* 4T package */
                        /* Ravens 0,2,4,5,7 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8514_E); /*2T*/
                        hwsFalconSetRavenDevBmp(0xB5);
                        break;

                    case CPSS_98EX5614_CNS:     /* 4T package */
                        /* Ravens 0,2,4,5,7 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98EX5614_E); /*2T*/
                        hwsFalconSetRavenDevBmp(0xB5);
                        break;

                    case CPSS_98CX8550_CNS:     /* 8T package*/
                        /* Ravens 0,1,2,5,7,8,10,13,14,15 */
                        hwsFalconTypeSet(devNum, HWS_FALCON_DEV_98CX8550_E);
                        hwsFalconSetRavenDevBmp(0xE5A7);
                        break;
                }
            }

            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
                /* falcon GM simulation use BC3 GOP/SERDES logic */
                 || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E && PRV_CPSS_PP_MAC(devNum)->isGmDevice)
                )
            {
                CPSS_LOG_INFORMATION_MAC("Calling: hwsBobcat3IfPreInit(devNum[%d], *funcPtr)", devNum);
                (GT_VOID)multiLaneConfigSupport(GT_TRUE);
                rc = hwsBobcat3IfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssDxChPortCpllConfig(devNum);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = hwsBobcat3IfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) /* for GM Falcon is like BC3 , see case above */
            {
                HWS_EXT_FUNC_STC_PTR extFuncPtrsStc;
                extFuncPtrsStc.getDeviceDriver = prvCpssDxChPortDevDriverGet;
                extFuncPtrsStc.ppHwTraceEnable = (MV_OS_HW_TRACE_ENABLE_FUNC)cpssDrvPpHwTraceEnable;
                funcPtrsStc.extFunctionStcPtr = &extFuncPtrsStc;
                /* create smi drivers */
                /*rc = cpssDxChPhyPortSmiInit(devNum);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }*/

                CPSS_LOG_INFORMATION_MAC("Calling: hwsFalconIfPreInit(devNum[%d], *funcPtr)", devNum);
                (GT_VOID)multiLaneConfigSupport(GT_TRUE);
                rc = hwsFalconIfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prxCpssDxChPortCpllDbInit(devNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)/* Only Ravens hold D2D */
                {
                    /* init raven D2D in order to use pex */
                    rc = hwsFalconStep0(devNum, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                    if(rc != GT_OK)
                    {
                        /* state that any error from this stage should be treated as 'must reset' */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR_NEED_RESET, LOG_ERROR_NO_MSG);

                    }
                    else
                    if(falcon_force_stop_init_after_d2d_init_get())
                    {
                        /* indication that the debug mode focus only on operations
                           of the D2D init , and not care about later operation */
                        return GT_OK;
                    }
                }

                rc = hwsFalconIfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                HWS_EXT_FUNC_STC_PTR extFuncPtrsStc;

                extFuncPtrsStc.ppHwTraceEnable = (MV_OS_HW_TRACE_ENABLE_FUNC)cpssDrvPpHwTraceEnable;
                funcPtrsStc.extFunctionStcPtr = &extFuncPtrsStc;
                funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;

                CPSS_LOG_INFORMATION_MAC("Calling: hwsHawkIfPreInit(devNum[%d], *funcPtr)", devNum);
                (GT_VOID)multiLaneConfigSupport(GT_TRUE);
                rc = hwsHawkIfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prxCpssDxChPortCpllDbInit(devNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                rc = prvCpssDxChConfigSerdesRefClock(devNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                rc = hwsHawkIfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
                    PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                HWS_EXT_FUNC_STC_PTR        extFuncPtrsStc;

                extFuncPtrsStc.ppHwTraceEnable = (MV_OS_HW_TRACE_ENABLE_FUNC)cpssDrvPpHwTraceEnable;
                funcPtrsStc.extFunctionStcPtr = &extFuncPtrsStc;
                funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
                if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
                {
                    hwsIronmanAsPhoenix();
                }

                CPSS_LOG_INFORMATION_MAC("Calling: hwsHawkIfPreInit(devNum[%d], *funcPtr)", devNum);
                (GT_VOID)multiLaneConfigSupport(GT_TRUE);
                rc = hwsPhoenixIfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssDxChConfigSerdesRefClock(devNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* hwsPhoenixIfInit calls mvHwsMifInit                           */
                /* mvHwsMifInit for Ironman bypasses MIF initialization          */
                /* MIF devices initialized by prvCpssDxChPortDpIronmanDeviceInit */
                rc = hwsPhoenixIfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
            {
                HWS_EXT_FUNC_STC_PTR extFuncPtrsStc;

                extFuncPtrsStc.ppHwTraceEnable = (MV_OS_HW_TRACE_ENABLE_FUNC)cpssDrvPpHwTraceEnable;
                funcPtrsStc.extFunctionStcPtr = &extFuncPtrsStc;
                funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;

                CPSS_LOG_INFORMATION_MAC("Calling: hwsHarrierIfPreInit(devNum[%d], *funcPtr)", devNum);
                (GT_VOID)multiLaneConfigSupport(GT_TRUE);
                rc = hwsHarrierIfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prxCpssDxChPortCpllDbInit(devNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                rc = hwsHarrierIfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                    (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
            {
                CPSS_LOG_INFORMATION_MAC("Calling: hwsAldrinIfPreInit(devNum[%d], *funcPtr)", devNum);

                rc = hwsAldrinIfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssDxChPortCpllConfig(devNum);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }


                rc = hwsAldrinIfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else
            if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                /* HWS function pointer initialization should be done before CPLL init */
                CPSS_LOG_INFORMATION_MAC("Calling: hwsBobKIfPreInit(*funcPtr)");
                rc = hwsBobKIfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssDxChPortCpllConfig(devNum);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                CPSS_LOG_INFORMATION_MAC("Calling: hwsBobKIfInit(devNum[%d], *funcPtr)", devNum);
                rc = hwsBobKIfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                CPSS_LOG_INFORMATION_MAC("Calling: hwsAldrin2IfPreInit(devNum[%d], *funcPtr)", devNum);
                (GT_VOID)multiLaneConfigSupport(GT_TRUE);
                rc = hwsAldrin2IfPreInit(devNum, &funcPtrsStc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssDxChPortCpllConfig(devNum);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = hwsAldrin2IfInit(devNum, &funcPtrsStc);
                if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                CPSS_LOG_INFORMATION_MAC("Calling: hwsBobcat2IfInit(devNum[%d], *funcPtr)", devNum);
                rc = hwsBobcat2IfInit(devNum, &funcPtrsStc);
            }
        }
        else /* AC3 / AC5 */
        if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            rc = GT_FAIL;
            funcPtrsStc.serdesRegGetAccess =
                (MV_SERDES_REG_ACCESS_GET)prvCpssGenPortGroupSerdesReadRegBitMask;
            funcPtrsStc.serdesRegSetAccess =
                (MV_SERDES_REG_ACCESS_SET)prvCpssGenPortGroupSerdesWriteRegBitMask;
            (GT_VOID)multiLaneConfigSupport(GT_FALSE);
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAlleycat3IfInit(devNum[%d], *funcPtr)", devNum);
                rc = mvHwsAlleycat3IfInit(devNum, &funcPtrsStc);
            }
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAlleycat3IfInit(devNum[%d], *funcPtr)", devNum);
                rc = mvHwsAlleycat5IfInit(devNum, &funcPtrsStc);
            }
        }
        else /* lion2 / hooper */
        {
            switch(PRV_CPSS_PP_MAC(devNum)->devType)
            {
                case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
                    CPSS_LOG_INFORMATION_MAC("Calling: hwsHooperIfInit(devNum[%d], *funcPtr)", devNum);
                    rc = hwsHooperIfInit(devNum, &funcPtrsStc);
                    break;
                default:
                    CPSS_LOG_INFORMATION_MAC("Calling: hwsLion2IfInit(devNum[%d], *funcPtr)", devNum);
                    rc = hwsLion2IfInit(devNum, &funcPtrsStc);
                    break;
            }
        }

        if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"initialization of Hws Failed");
        }

        /* initialize only cpssDriver for HW access, skip the rest */
        if (DXCH_HWINIT_GLOVAR(dxChHwInitSrc.dxChInitRegDefaults) == GT_TRUE)
            return GT_OK;

        if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            GT_U32  portGroup;
            GT_U32  localPort;
            CPSS_PORT_INTERFACE_MODE_ENT    ifModeHw;

            for(i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
            {
                PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, i);

                if(CPSS_CPU_PORT_NUM_CNS == i)
                {
                    continue;
                }

                rc = prvCpssDxChPortLion2InterfaceModeHwGet(devNum, i, &ifModeHw);
                if((GT_OK == rc) && (ifModeHw != CPSS_PORT_INTERFACE_MODE_NA_E))
                {/* if found one configured port we are not after HW reset -
                    skip following resets */
                    break;
                }

                portGroup = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, i);
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, i);
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, localPort, MMPCS, RESET);
                rc = mvHwsPcsReset(devNum, portGroup, localPort, _10GBase_KR, MMPCS, RESET);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPcsReset : Hws Failed");
                }

                if(9 == localPort)
                {
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 6/* xpcsNum of port 9 in RXAUI*/, XPCS, RESET);
                    rc = mvHwsPcsReset(devNum, portGroup,
                                        9/* mac#12 of port 9 in RXAUI should reset PCS #6*/,
                                        RXAUI, XPCS, RESET);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPcsReset : Hws Failed");
                    }
                }

                if(11 == localPort)
                {
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 7/* xpcsNum of port 11 in RXAUI*/, XPCS, RESET);
                    rc = mvHwsPcsReset(devNum, portGroup,
                                        11/* mac#14 of port 11 in RXAUI should reset PCS #7*/,
                                        RXAUI, XPCS, RESET);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPcsReset : Hws Failed");
                    }
                }
            }
        }

        /* init combo ports array */
        comboPortsInfoArray = PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray;
        for(i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
        {
            comboPortsInfoArray[i].preferredMacIdx = CPSS_DXCH_PORT_COMBO_NA_MAC_CNS;
            for(j = 0; j < CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS; j++)
            {
                comboPortsInfoArray[i].macArray[j].macNum = CPSS_DXCH_PORT_COMBO_NA_MAC_CNS;
            }
        }

        /* init PRBS modes ports array */
        for(i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsMode[i] = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;
        }

        /* init PRBS Serdes Counters Clear On Read Enable */
        for(i = 0; i < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; i++)
        {
            for(j = 0; j < PRV_CPSS_MAX_PORT_LANES_CNS; j++)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsSerdesCountersClearOnReadEnable[i][j] = GT_TRUE;
            }
        }
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortPeriodicFlowControlIntervalSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval.
*
* @note   APPLICABLE DEVICES:     Lion2; Falcon; AC5P; AC5X; Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2;  Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                 - physical port number(APPLICABLE DEVICES:Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] portType                 - port type(APPLICABLE DEVICES:Lion2)
* @param[in] value                    - For Lion and Lion2 - The interval in microseconds between two successive
*                                                       Flow Control frames that are sent periodically by the port.
*                                                       (APPLICABLE RANGES: 0..171798691)
*                                                       For Falcon - The interval is in 512 bit-time(APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portType
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS internal_cpssDxChPortPeriodicFlowControlIntervalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    IN  GT_U32                              value
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      i;
    GT_STATUS   rc;
    CPSS_DXCH_PORT_FC_MODE_ENT  fcMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                           CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_TRUE)
    {
        /*flow control should be disabled while configuring threshold*/
        rc = prvCpssDxChPortSip6FlowControlModeGet(devNum,portNum,&fcMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortSip6FlowControlModeGet Failed");
        }

        if(fcMode!=CPSS_DXCH_PORT_FC_MODE_DISABLE_E)
        {
            rc = prvCpssDxChPortSip6FlowControlModeSet(devNum,portNum,CPSS_DXCH_PORT_FC_MODE_DISABLE_E);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortSip6FlowControlModeSet Failed");
            }
        }

        /*configure all tc*/
        for(i=0;i<CPSS_TC_RANGE_CNS;i++)
        {
            rc = prvCpssDxChPortSip6PfcQuantaThreshSet(devNum,portNum,i,value);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortPeriodicFlowControlIntervalSet Failed");
            }
        }

        /*Re enable flow control  if required*/
        if(fcMode!=CPSS_DXCH_PORT_FC_MODE_DISABLE_E)
        {
            rc = prvCpssDxChPortSip6FlowControlModeSet(devNum,portNum,fcMode);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortSip6FlowControlModeSet Failed");
            }
        }

        return GT_OK;
    }

    switch (portType)
    {
        case CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                miscellaneousRegs.lms0PeriodicFlowControlInterval0;
                break;
        case CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                miscellaneousRegs.lms1PeriodicFlowControlInterval1;
                break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(value > 0xFFFFFFFF / 25)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regValue = value * 25; /* the register value is in 40 ns. */

    return prvCpssDrvHwPpWriteRegister(devNum, regAddr, regValue);
}

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval.
*
* @note   APPLICABLE DEVICES:     Lion2; Falcon; AC5P; AC5X; Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2;  Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                 - physical port number(APPLICABLE DEVICES:Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] portType                 - port type(APPLICABLE DEVICES:Lion2)
* @param[in] value                    - For Lion and Lion2 - The interval in microseconds between two successive
*                                                       Flow Control frames that are sent periodically by the port.
*                                                       (APPLICABLE RANGES: 0..171798691)
*                                                       For Falcon - The interval is in 512 bit-time(APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portType
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    IN  GT_U32                              value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFlowControlIntervalSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portType, value));

    rc = internal_cpssDxChPortPeriodicFlowControlIntervalSet(devNum, portNum, portType, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portType, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPeriodicFlowControlIntervalGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval.
*
* @note   APPLICABLE DEVICES:     Lion2; Falcon; AC5P; AC5X; Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2;  Aldrin; AC3X.
*
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                 - physical port number(APPLICABLE DEVICES:Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] portType                 - port type(APPLICABLE DEVICES:Lion2)
*
* @param[out] valuePtr                    (Pointer to)For Lion and Lion2 - The interval in microseconds between two successive
*                                                       Flow Control frames that are sent periodically by the port.
*                                                       (APPLICABLE RANGES: 0..171798691)
*                                                       For Falcon - The interval is in 512 bit-time(APPLICABLE RANGES: 0..0xFFFF)

*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPeriodicFlowControlIntervalGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    OUT GT_U32                              *valuePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                           CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_TRUE)
    {   /*configure tc 0 ,all TC should be equal*/
        rc = prvCpssDxChPortSip6PfcQuantaThreshGet(devNum,portNum,0,valuePtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortPeriodicFlowControlIntervalSet Failed");
        }
        return GT_OK;
    }

    switch (portType)
    {
        case CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                miscellaneousRegs.lms0PeriodicFlowControlInterval0;
                break;
        case CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                miscellaneousRegs.lms1PeriodicFlowControlInterval1;
                break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the register value is in 40 ns. */
    *valuePtr = regValue / 25;

    return rc;

}

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                 - physical port number(APPLICABLE DEVICES:Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] portType                 - port type(APPLICABLE DEVICES:Lion2)
*
* @param[out] valuePtr                    (Pointer to)For Lion and Lion2 - The interval in microseconds between two successive
*                                                       Flow Control frames that are sent periodically by the port.
*                                                       (APPLICABLE RANGES: 0..171798691)
*                                                       For Falcon - The interval is in 512 bit-time(APPLICABLE RANGES: 0..0xFFFF)

*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    OUT GT_U32                              *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFlowControlIntervalGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portType, valuePtr));

    rc = internal_cpssDxChPortPeriodicFlowControlIntervalGet(devNum, portNum,portType, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,portNum, portType, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portGroupId;/* core number */
    GT_U32      localPort;  /* port number in local core */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

    if((portType != CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E)
        && (portType != CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regValue = (CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E == portType) ? 0 : 1;
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->miscellaneousRegs.
                                            lms1PeriodicFlowControlIntervalSel;
    return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                localPort /* fieldOffset */,
                                                1, regValue);
}

/**
* @internal internal_cpssDxChPortPeriodicFlowControlIntervalSelectionSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPeriodicFlowControlIntervalSelectionSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType
)
{
    return prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet(devNum,
                                                                    portNum,
                                                                    portType);
}

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalSelectionSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSelectionSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFlowControlIntervalSelectionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portType));

    rc = internal_cpssDxChPortPeriodicFlowControlIntervalSelectionSet(devNum, portNum, portType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPeriodicFlowControlIntervalSelectionGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portTypePtr              - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPeriodicFlowControlIntervalSelectionGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT *portTypePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portGroupId;/* core number */
    GT_U32      localPort;  /* port number in local core */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portTypePtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->miscellaneousRegs.
                                            lms1PeriodicFlowControlIntervalSel;
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                localPort /* fieldOffset */,
                                                1, &regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    *portTypePtr = (0 == regValue) ? CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E :
                                        CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E;
    return GT_OK;
}

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalSelectionGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portTypePtr              - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSelectionGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT *portTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFlowControlIntervalSelectionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portTypePtr));

    rc = internal_cpssDxChPortPeriodicFlowControlIntervalSelectionGet(devNum, portNum, portTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortMacTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortMacTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr
)
{
    GT_STATUS               rc;
    GT_U32                  portMacNum;      /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT  prvPortMacType;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacTypePtr);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacTypeGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacTypeGetFunc(devNum,portNum,
                                  portMacTypePtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    prvPortMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);


    switch(prvPortMacType)
    {
        case PRV_CPSS_PORT_NOT_EXISTS_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_NOT_EXISTS_E;
            break;

        case PRV_CPSS_PORT_FE_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_FE_E;
            break;

        case PRV_CPSS_PORT_GE_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_GE_E;
            break;

        case PRV_CPSS_PORT_XG_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_XG_E;
            break;

        case PRV_CPSS_PORT_XLG_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_XLG_E;
            break;

        case PRV_CPSS_PORT_HGL_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_HGL_E;
            break;

        case PRV_CPSS_PORT_CG_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_CG_E;
            break;

        case PRV_CPSS_PORT_ILKN_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_ILKN_E;
            break;

        case PRV_CPSS_PORT_MTI_100_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_MTI_100_E;
            break;

        case PRV_CPSS_PORT_MTI_400_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_MTI_400_E;
            break;

        case PRV_CPSS_PORT_MTI_CPU_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_MTI_CPU_E;
            break;

        case PRV_CPSS_PORT_MTI_USX_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_MTI_USX_E;
            break;

        case PRV_CPSS_PORT_NOT_APPLICABLE_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_NOT_APPLICABLE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortMacTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacTypePtr));

    rc = internal_cpssDxChPortMacTypeGet(devNum, portNum, portMacTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:   Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] value                    - The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPeriodicFlowControlCounterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC number */
    GT_U32      periodicEnable; /* surrent status of periodic FC per port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    /* store <periodic_enable> field */
    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 11, 1, &periodicEnable);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* disable <periodic_enable> field */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 1, 0);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(value > (0xFFFFFFFF / PRV_CPSS_PP_MAC(devNum)->coreClock))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regValue = value * PRV_CPSS_PP_MAC(devNum)->coreClock;

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrLSB;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrLSB;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 16, U32_GET_FIELD_MAC(regValue, 0, 16));
    if(GT_OK != rc)
    {
        return rc;
    }


    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrMSB;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrMSB;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 16, U32_GET_FIELD_MAC(regValue, 16, 16));
    if(GT_OK != rc)
    {
        return rc;
    }

    /* restore <periodic_enable> field */
    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 1, periodicEnable);
    return rc;
}

/**
* @internal cpssDxChPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:   Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] value                    - The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The interval in micro seconds between transmission of two consecutive
*       Flow Control packets recommended interval is calculated by the following formula:
*       period (micro seconds) = 33553920 / speed(M)
*       Exception: for 10M, 100M and 10000M Flow Control packets recommended interval is 33500
*       Following are recommended intervals in micro seconds for common port speeds:
*       33500  for speed 10M
*       33500  for speed 100M
*       33500  for speed 1G
*       13421  for speed 2.5G
*       6710  for speed 5G
*       3355  for speed 10G
*       2843  for speed 11.8G
*       2796  for speed 12G
*       2467  for speed 13.6G
*       2236  for speed 15G
*       2097  for speed 16G
*       1677  for speed 20G
*       838   for speed 40G
*       710   for speed 47.2G
*       671   for speed 50G
*       447   for speed 75G
*       335   for speed 100G
*       239   for speed 140G
*
*/
GT_STATUS cpssDxChPortPeriodicFlowControlCounterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFlowControlCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, value));

    rc = internal_cpssDxChPortPeriodicFlowControlCounterSet(devNum, portNum, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:   Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] valuePtr                 - (pointer to) The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPeriodicFlowControlCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrLSB;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrLSB;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 16, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    *valuePtr = regValue;

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrMSB;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrMSB;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 16, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    *valuePtr |= (regValue << 16);

    *valuePtr = *valuePtr / PRV_CPSS_PP_MAC(devNum)->coreClock;

    return GT_OK;
}

/**
* @internal cpssDxChPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:   Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] valuePtr                 - (pointer to) The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPeriodicFlowControlCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, valuePtr));

    rc = internal_cpssDxChPortPeriodicFlowControlCounterGet(devNum, portNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvHwsUnitsIdToCpssUnitsId function
* @endinternal
*
* @brief   Conver from Hws Units Id Enum to Cpss Units Id Enum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2.
*
* @param[in] hwUnitId                 - Hws Units Id
*
* @param[out] unitId                   - Cpss Units Id
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad hwUnitId
*
* @note Debug function.
*
*/
static GT_STATUS prvHwsUnitsIdToCpssUnitsId
(
    IN  MV_HWS_UNITS_ID             hwUnitId,
    OUT CPSS_DXCH_PORT_UNITS_ID_ENT *unitId
)
{
    switch (hwUnitId)
    {
        case GEMAC_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_GEMAC_UNIT_E;
            break;
        case XLGMAC_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_XLGMAC_UNIT_E;
            break;
        case HGLMAC_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_HGLMAC_UNIT_E;
            break;
        case XPCS_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_XPCS_UNIT_E;
            break;
        case MMPCS_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_MMPCS_UNIT_E;
            break;
        case CG_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_CG_UNIT_E;
            break;
        case INTLKN_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_INTLKN_UNIT_E;
            break;
        case INTLKN_RF_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_INTLKN_RF_UNIT_E;
            break;
        case SERDES_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_SERDES_UNIT_E;
            break;
        case SERDES_PHY_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_SERDES_PHY_UNIT_E;
            break;
        case ETI_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_ETI_UNIT_E;
            break;
        case ETI_ILKN_RF_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_ETI_ILKN_RF_UNIT_E;
            break;
        case D_UNIT:
            (*unitId) = CPSS_DXCH_PORT_UNITS_ID_D_UNIT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvCpssUnitsIdToHwsUnitsId function
* @endinternal
*
* @brief   Conver from Cpss Units Id Enum to Hws Units Id Enum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2.
*
* @param[in] unitId                   - Cpss Units Id
*
* @param[out] hwUnitId                 - Hws Units Id
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad unitId
*
* @note Debug function.
*
*/
static GT_STATUS prvCpssUnitsIdToHwsUnitsId
(
    IN   CPSS_DXCH_PORT_UNITS_ID_ENT unitId,
    OUT  MV_HWS_UNITS_ID             *hwUnitId
)
{
    switch (unitId)
    {
        case CPSS_DXCH_PORT_UNITS_ID_GEMAC_UNIT_E:
            (*hwUnitId) = GEMAC_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_XLGMAC_UNIT_E:
            (*hwUnitId) = XLGMAC_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_HGLMAC_UNIT_E:
            (*hwUnitId) = HGLMAC_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_XPCS_UNIT_E:
            (*hwUnitId) = XPCS_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_MMPCS_UNIT_E:
            (*hwUnitId) = MMPCS_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_CG_UNIT_E:
            (*hwUnitId) = CG_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_INTLKN_UNIT_E:
            (*hwUnitId) = INTLKN_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_INTLKN_RF_UNIT_E:
            (*hwUnitId) = INTLKN_RF_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_SERDES_UNIT_E:
            (*hwUnitId) = SERDES_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_SERDES_PHY_UNIT_E:
            (*hwUnitId) = SERDES_PHY_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_ETI_UNIT_E:
            (*hwUnitId) = ETI_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_ETI_ILKN_RF_UNIT_E:
            (*hwUnitId) = ETI_ILKN_RF_UNIT;
            break;
        case CPSS_DXCH_PORT_UNITS_ID_D_UNIT_E:
            (*hwUnitId) = D_UNIT;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChPortUnitInfoGetByAddr function
* @endinternal
*
* @brief   Get unit ID by unit address in device
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
static GT_STATUS internal_cpssDxChPortUnitInfoGetByAddr
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      baseAddr,
    OUT CPSS_DXCH_PORT_UNITS_ID_ENT *unitId
)
{
    MV_HWS_UNITS_ID     hwUnitId;
    GT_STATUS           rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(unitId);

    CPSS_LOG_INFORMATION_MAC("Calling: mvUnitInfoGetByAddr(devNum[%d], baseAddr[%d], *hwUnitId)", devNum, baseAddr);
    rc = mvUnitInfoGetByAddr(devNum, baseAddr, &hwUnitId);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvUnitInfoGetByAddr : Hws Failed");
    }

    rc = prvHwsUnitsIdToCpssUnitsId(hwUnitId, unitId);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvHwsUnitsIdToCpssUnitsId : Hws Failed");
    }
    return rc;

}

/**
* @internal cpssDxChPortUnitInfoGetByAddr function
* @endinternal
*
* @brief   Get unit ID by unit address in device
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitIdPtr                - unit ID (MAC, PCS, SERDES)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortUnitInfoGetByAddr
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       baseAddr,
    OUT CPSS_DXCH_PORT_UNITS_ID_ENT  *unitIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortUnitInfoGetByAddr);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseAddr, unitIdPtr));

    rc = internal_cpssDxChPortUnitInfoGetByAddr(devNum, baseAddr, unitIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseAddr, unitIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2.
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
static GT_STATUS internal_cpssDxChPortUnitInfoGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_PORT_UNITS_ID_ENT unitId,
    OUT GT_U32                      *baseAddr,
    OUT GT_U32                      *unitIndex
)
{
    MV_HWS_UNITS_ID     hwUnitId;
    GT_STATUS           rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(baseAddr);
    CPSS_NULL_PTR_CHECK_MAC(unitIndex);

    rc = prvCpssUnitsIdToHwsUnitsId(unitId, &hwUnitId);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssUnitsIdToHwsUnitsId : Hws Failed");
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvUnitInfoGet(devNum[%d], unitId[%d], *baseAddr, *unitIndex)", devNum, hwUnitId);
    rc = mvUnitInfoGet(devNum, hwUnitId, baseAddr, unitIndex);

    return rc;
}

/**
* @internal cpssDxChPortUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2.
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddrPtr              - unit base address in device
* @param[out] unitIndexPtr             - unit index in device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortUnitInfoGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_PORT_UNITS_ID_ENT unitId,
    OUT GT_U32                      *baseAddrPtr,
    OUT GT_U32                      *unitIndexPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortUnitInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitId, baseAddrPtr, unitIndexPtr));

    rc = internal_cpssDxChPortUnitInfoGet(devNum, unitId, baseAddrPtr, unitIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitId, baseAddrPtr, unitIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
* @param[in] overrideEnable           - override the CTLE default value
* @param[in] ctleBiasValue            - value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
static GT_STATUS internal_cpssDxChPortCtleBiasOverrideEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  GT_BOOL                            overrideEnable,
    IN  CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT  ctleBiasValue
)
{
   GT_U32  portMacNum;      /* MAC number */
   GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);

        /* Applicable devices are Bobcat3, Aldrin, AC3X and Bobk revision 1 and above */
    if ( PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
         PRV_CPSS_PP_MAC(devNum)->revision == 0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "API is applicable only for Aldrin, AC3X, BC3 and Bobk rev 1 and above. Given revision: %d, family: %d, sub-family: %d",
                PRV_CPSS_PP_MAC(devNum)->revision, PRV_CPSS_PP_MAC(devNum)->devFamily, PRV_CPSS_PP_MAC(devNum)->devSubFamily);
    }

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = mvHwsPortCtleBiasOverrideEnableSet(devNum, portMacNum, (GT_U32)overrideEnable,(GT_U32)ctleBiasValue);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsPortCtleBiasOverrideEnableSet : Hws Failed");
    }


    return GT_OK;

}

/**
* @internal cpssDxChPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
* @param[in] overrideEnable           - override the CTLE default value
* @param[in] ctleBiasValue            - value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortCtleBiasOverrideEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             overrideEnable,
    IN  CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT   ctleBiasValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCtleBiasOverrideEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnable, ctleBiasValue));

    rc = internal_cpssDxChPortCtleBiasOverrideEnableSet(devNum, portNum, overrideEnable, ctleBiasValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnable, clteBiasValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssDxChPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
*
* @param[out] overrideEnablePtr        - pointer to override mode
* @param[out] ctleBiasValuePtr         - pointer to value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
static GT_STATUS internal_cpssDxChPortCtleBiasOverrideEnableGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_BOOL                               *overrideEnablePtr,
    IN  CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT     *ctleBiasValuePtr
)
{
   GT_U32  portMacNum;      /* MAC number */
   GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);

        /* Applicable devices are Bobcat3, Aldrin, AC3X and Bobk revision 1 and above */
    if ( PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
         PRV_CPSS_PP_MAC(devNum)->revision == 0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "API is applicable only for Aldrin, AC3X, BC3 and Bobk rev 1 and above. Given revision: %d, family: %d, sub-family: %d",
                PRV_CPSS_PP_MAC(devNum)->revision, PRV_CPSS_PP_MAC(devNum)->devFamily, PRV_CPSS_PP_MAC(devNum)->devSubFamily);
    }

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = mvHwsPortCtleBiasOverrideEnableGet(devNum, portMacNum, (GT_U32*)overrideEnablePtr,(GT_U32*)ctleBiasValuePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsPortCtleBiasOverrideEnableGet : Hws Failed");
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
*
* @param[out] overrideEnablePtr        - pointer to override mode
* @param[out] ctleBiasValuePtr         - pointer to value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortCtleBiasOverrideEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             *overrideEnablePtr,
    IN  CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT   *ctleBiasValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCtleBiasOverrideEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnablePtr, ctleBiasValuePtr));

    rc = internal_cpssDxChPortCtleBiasOverrideEnableGet(devNum, portNum, overrideEnablePtr, ctleBiasValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnablePtr, clteBiasValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssDxChPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden. The default value for the
*         override mode in Caelum rev 1 and above is true, on other devices is false.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number
* @param[in] vosOverride              - GT_TRUE means to override the VOS parameters for the device, GT_FALSE otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
static GT_STATUS internal_cpssDxChPortVosOverrideControlModeSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     vosOverride
)
{

    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);

    /* Applicable devices are Bobcat3, Aldrin, AC3X and Bobk revision 1 and above */
    if ( PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
         PRV_CPSS_PP_MAC(devNum)->revision == 0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "API is applicable only for Aldrin, AC3X, BC3 and Bobk rev 1 and above. Given revision: %d, family: %d, sub-family: %d",
                PRV_CPSS_PP_MAC(devNum)->revision, PRV_CPSS_PP_MAC(devNum)->devFamily, PRV_CPSS_PP_MAC(devNum)->devSubFamily);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortVosOverrideControlModeSet(devNum[%d], vosOverride[%d])", devNum, vosOverride);
    rc = mvHwsPortVosOverrideControlModeSet(devNum, 0, vosOverride);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwsSerdesVosOverrideControlModeSet : Hws Failed");
    }

    return rc;
}
/**
* @internal cpssDxChPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden. The default value for the
*         override mode in Caelum rev 1 and above is true, on other devices is false.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number
* @param[in] vosOverride              - GT_TRUE means to override the VOS parameters for the device, GT_FALSE otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortVosOverrideControlModeSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     vosOverride
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortVosOverrideControlModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vosOverride));

    rc = internal_cpssDxChPortVosOverrideControlModeSet(devNum, vosOverride);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vosOverride));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Get the override mode of the VOS parameters for all ports.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if vosOverride is NULL pointer
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
static GT_STATUS internal_cpssDxChPortVosOverrideControlModeGet
(
    IN   GT_U8       devNum,
    OUT  GT_BOOL     *vosOverridePtr
)
{
    GT_STATUS        rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(vosOverridePtr);

    /* Applicable devices are Bobcat3, Aldrin, AC3X and Bobk revision 1 and above */
    if ( PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E &&
         PRV_CPSS_PP_MAC(devNum)->revision == 0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "API is applicable only for Aldrin, AC3X, BC3 and Bobk rev 1 and above. Given revision: %d, family: %d, sub-family: %d",
                PRV_CPSS_PP_MAC(devNum)->revision, PRV_CPSS_PP_MAC(devNum)->devFamily, PRV_CPSS_PP_MAC(devNum)->devSubFamily);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortVosOverrideControlModeGet(devNum[%d])", devNum);
    rc = mvHwsPortVosOverrideControlModeGet(devNum, vosOverridePtr);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwsSerdesVosOverrideControlModeGet : Hws Failed");
    }

    return rc;
}

/**
* @internal cpssDxChPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Get the override mode of the VOS parameters for all ports.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  Lion2; Puma; Puma3; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - Device Number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if vosOverride is NULL pointer
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortVosOverrideControlModeGet
(
    IN   GT_U8       devNum,
    OUT  GT_BOOL     *vosOverridePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortVosOverrideControlModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vosOverridePtr));

    rc = internal_cpssDxChPortVosOverrideControlModeGet(devNum, vosOverridePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vosOverridePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXlgReduceAverageIPGSet function
* @endinternal
*
* @brief   Configure Reduce Average IPG in XLG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] value                    -  to set to the XLG MAC DIC_PPM_ IPG_Reduce Register (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM ' on bad parameters
*
* @note Function should be used to solve the problem:
*       The port BW is few PPMs lower than FWS, and tail drops occur.
*       (FE-7680593)
*
*/
GT_STATUS internal_cpssDxChPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          value
)
{
    GT_STATUS rc = GT_OK; /* return code */
    GT_U32    portMacNum; /* port Mac number */
    GT_U32    regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    if(value > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xlgDicPpmIpgReduce;
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, value);

    return rc;
}

/**
* @internal cpssDxChPortXlgReduceAverageIPGSet function
* @endinternal
*
* @brief   Configure Reduce Average IPG in XLG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] value                    -  to set to the XLG MAC DIC_PPM_ IPG_Reduce Register (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Function should be used to solve the problem:
*       The port BW is few PPMs lower than FWS, and tail drops occur.
*       (FE-7680593)
*
*/
GT_STATUS cpssDxChPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          value
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXlgReduceAverageIPGSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, value));

    rc = internal_cpssDxChPortXlgReduceAverageIPGSet(devNum, portNum, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXlgReduceAverageIPGGet function
* @endinternal
*
* @brief   Get Reduce Average IPG value in XLG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] valuePtr                 -  pointer to value - content of the XLG MAC DIC_PPM_IPG_Reduce register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS internal_cpssDxChPortXlgReduceAverageIPGGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *valuePtr
)
{
    GT_STATUS rc = GT_OK; /* return code */
    GT_U32    portMacNum; /* port Mac number */
    GT_U32    regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xlgDicPpmIpgReduce;
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, valuePtr);

    return rc;
}

/**
* @internal cpssDxChPortXlgReduceAverageIPGGet function
* @endinternal
*
* @brief   Get Reduce Average IPG value in XLG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] valuePtr                 -  pointer to value - content of the XLG MAC DIC_PPM_IPG_Reduce register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortXlgReduceAverageIPGGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *valuePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXlgReduceAverageIPGGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, valuePtr));

    rc = internal_cpssDxChPortXlgReduceAverageIPGGet(devNum, portNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortCgMacRxEnableSet function
* @endinternal
*
* @brief   Debug function to enable/disable of RX in CG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*                                      enable     - enable or disable RX on CG MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
GT_STATUS prvCpssDxChPortCgMacRxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  portMacNum; /* MAC number */
    GT_U32  value;      /* data to write to register */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(state);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* there is no CG MAC for port */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 1;

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortCgMacRxEnableGet function
* @endinternal
*
* @brief   Debug function to get RX enable/disable state in CG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[out] statePtr                - (pointer to) enable or disable of RX state on CG MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortCgMacRxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  portMacNum; /* MAC number */
    GT_U32  value;      /* data to write to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);


    PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* there is no CG MAC for port */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, 1, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMtiMacRxEnableSet function
* @endinternal
*
* @brief   Debug function to state the enable/disable of RX in MTI MAC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - enable or disable RX on MTI MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
GT_STATUS prvCpssDxChPortMtiMacRxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  PRV_CPSS_DXCH_PORT_MTI_MAC_RX_STATE_ENT   state
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  portMacNum; /* MAC number */
    GT_U32  value;      /* data to write to register */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_BOOL isPortEnabled;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(state)
    {
        case PRV_CPSS_DXCH_PORT_MTI_MAC_RX_STATE_FORCE_DISABLED_E:
            value = 0;
            break;
        case PRV_CPSS_DXCH_PORT_MTI_MAC_RX_STATE_FORCE_ENABLED_E:
            value = 1;
            break;
        case PRV_CPSS_DXCH_PORT_MTI_MAC_RX_STATE_AS_TX_STATE_E:
            /* synch the Rx according to the Tx */
            rc = cpssDxChPortEnableGet(devNum,portNum,&isPortEnabled);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            value = isPortEnabled;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(state);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* note : some of those registers will hold 'PRV_CPSS_SW_PTR_ENTRY_UNUSED' ... and will be ignored */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 1;
    regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;

    /* note : some of those registers will hold 'PRV_CPSS_SW_PTR_ENTRY_UNUSED' ... and will be ignored */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = 1;
    regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = 1;

    /* note : some of those registers will hold 'PRV_CPSS_SW_PTR_ENTRY_UNUSED' ... and will be ignored */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 1;
    regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 1;

    /* note : some of those registers will hold 'PRV_CPSS_SW_PTR_ENTRY_UNUSED' ... and will be ignored */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
    regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 1;
    regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 1;

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* save the state into the DB (per MAC) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.mtiMacInfo[portMacNum].rxState = state;

    /****************/
    /*  BR section  */
    /****************/
    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_commandConfig;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 1;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);


    return GT_OK;
}

/**
* @internal prvCpssDxChPortMtiMacRxEnableGet function
* @endinternal
*
* @brief   Debug function to get RX enable/disable state of MTI MAC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[out] dbStatePtr              - (pointer to) DB (database) state about the RX . (ignored if NULL)
* @param[out] hwStatePtr              - (pointer to) HW (hardware) value of enable or disable. (ignored if NULL)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMtiMacRxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT PRV_CPSS_DXCH_PORT_MTI_MAC_RX_STATE_ENT   *dbStatePtr,
    OUT GT_BOOL                 *hwStatePtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  portMacNum; /* MAC number */
    GT_U32  value;      /* data to write to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(hwStatePtr)
    {
        if(PRV_CPSS_PORT_MTI_100_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else if(PRV_CPSS_PORT_MTI_400_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else if(PRV_CPSS_PORT_MTI_CPU_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else if(PRV_CPSS_PORT_MTI_USX_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                regAddr, 1, 1, &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

        *hwStatePtr = BIT2BOOL_MAC(value);
    }

    if(dbStatePtr)
    {
        *dbStatePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.mtiMacInfo[portMacNum].rxState;
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortCascadePfcParametersSet function
* @endinternal
*
* @brief   Set PFC parameters per remoting cascade port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] timer                    - PFC  (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] tcBitmap                 - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note PFC is sent in uplink direction from remote port.
*
*/
static GT_STATUS internal_cpssDxChPortCascadePfcParametersSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      xOffThreshold,
    IN  GT_U32      xOnThreshold,
    IN  GT_U32      timer,
    IN  GT_U32      tcBitmap
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM                portNum;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
            CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_CAELUM_E | CPSS_ALDRIN_E);

    /* Walk over all remote cascading ports to configure according to new mode. */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    for(portNum = 0 ; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[portNum];
        if(remotePhyMacInfoPtr == NULL)
        {
            continue;
        }
        switch(remotePhyMacInfoPtr->connectedPhyMacInfo.phyMacType)
        {
            case CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E:
                rc = prvCpssDxChCfgPort88e1690CascadePfcParametersSet(devNum,
                                                                portNum,
                                                                xOffThreshold,
                                                                xOnThreshold,
                                                                timer,
                                                                tcBitmap);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;
            default:
                break;
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChPortCascadePfcParametersSet function
* @endinternal
*
* @brief   Set PFC parameters per remoting cascade port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] timer                    - PFC  (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] tcBitmap                 - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note PFC is sent in uplink direction from remote port.
*
*/
GT_STATUS cpssDxChPortCascadePfcParametersSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      xOffThreshold,
    IN  GT_U32      xOnThreshold,
    IN  GT_U32      timer,
    IN  GT_U32      tcBitmap
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCascadePfcParametersSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xOffThreshold, xOnThreshold, timer, tcBitmap));

    rc = internal_cpssDxChPortCascadePfcParametersSet(devNum, xOffThreshold, xOnThreshold, timer, tcBitmap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xOffThreshold, xOnThreshold, timer, tcBitmap));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCascadePfcParametersGet function
* @endinternal
*
* @brief   Get PFC parameters per remoting cascade port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
*
* @param[out] xOffThresholdPtr         - (Pointer to) xOff threshold in buffers
* @param[out] xOnThresholdPtr          - (Pointer to) xOn threshold in buffers
* @param[out] timerPtr                 - (Pointer to) PFC timer
* @param[out] tcBitmapPtr              - (Pointer to) list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internal_cpssDxChPortCascadePfcParametersGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *xOffThresholdPtr,
    OUT GT_U32      *xOnThresholdPtr,
    OUT GT_U32      *timerPtr,
    OUT GT_U32      *tcBitmapPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM                portNum;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
            CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_CAELUM_E | CPSS_ALDRIN_E);

    /* Walk over all remote cascading ports to configure according to new mode. */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    for(portNum = 0 ; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[portNum];
        if(remotePhyMacInfoPtr == NULL)
        {
            continue;
        }
        switch(remotePhyMacInfoPtr->connectedPhyMacInfo.phyMacType)
        {
            case CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E:
                rc = prvCpssDxChCfgPort88e1690CascadePfcParametersGet(devNum,
                                                                portNum,
                                                                xOffThresholdPtr,
                                                                xOnThresholdPtr,
                                                                timerPtr,
                                                                tcBitmapPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                return GT_OK;
            default:
                break;
        }

    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortCascadePfcParametersGet function
* @endinternal
*
* @brief   Get PFC parameters per remoting cascade port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
*
* @param[out] xOffThresholdPtr         - (Pointer to) xOff threshold in buffers
* @param[out] xOnThresholdPtr          - (Pointer to) xOn threshold in buffers
* @param[out] timerPtr                 - (Pointer to) PFC timer
* @param[out] tcBitmapPtr              - (Pointer to) list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChPortCascadePfcParametersGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *xOffThresholdPtr,
    OUT GT_U32      *xOnThresholdPtr,
    OUT GT_U32      *timerPtr,
    OUT GT_U32      *tcBitmapPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCascadePfcParametersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xOffThresholdPtr, xOnThresholdPtr, timerPtr, tcBitmapPtr));

    rc = internal_cpssDxChPortCascadePfcParametersGet(devNum, xOffThresholdPtr, xOnThresholdPtr, timerPtr, tcBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xOffThresholdPtr, xOnThresholdPtr, timerPtr, tcBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRemoteFcParametersSet function
* @endinternal
*
* @brief   Set FC parameters per remote port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] portNum                  - physical port number
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Threshold are detected by remoting PHY/MAC,
*       FC is sent in downlink (NW) direction.
*
*/
static GT_STATUS internal_cpssDxChPortRemoteFcParametersSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  xOffThreshold,
    IN  GT_U32                  xOnThreshold
)
{
    GT_STATUS rc = GT_OK;
    GT_U32                  portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
            CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_CAELUM_E | CPSS_ALDRIN_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacFcParamsSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacFcParamsSetFunc(devNum,portNum,
                                  xOffThreshold,
                                  xOnThreshold,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* Nothing to configure */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortRemoteFcParametersSet function
* @endinternal
*
* @brief   Set FC parameters per remote port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] portNum                  - physical port number
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Threshold are detected by remoting PHY/MAC,
*       FC is sent in downlink (NW) direction.
*
*/
GT_STATUS cpssDxChPortRemoteFcParametersSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  xOffThreshold,
    IN  GT_U32                  xOnThreshold
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRemoteFcParametersSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, xOffThreshold, xOnThreshold));

    rc = internal_cpssDxChPortRemoteFcParametersSet(devNum, portNum, xOffThreshold, xOnThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, xOffThreshold, xOnThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRemoteFcParametersGet function
* @endinternal
*
* @brief   Get FC parameters per remote port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] portNum                  - physical port number
*
* @param[out] xOffThresholdPtr         - (Pointer to) xOff threshold in buffers
* @param[out] xOnThresholdPtr          - (Pointer to) xOn threshold in buffers
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internal_cpssDxChPortRemoteFcParametersGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *xOffThresholdPtr,
    OUT GT_U32                  *xOnThresholdPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32                  portMacNum;      /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
            CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_CAELUM_E | CPSS_ALDRIN_E);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacFcParamsGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacFcParamsGetFunc(devNum,portNum,
                                  xOffThresholdPtr,
                                  xOnThresholdPtr,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* Nothing to configure */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortRemoteFcParametersGet function
* @endinternal
*
* @brief   Get FC parameters per remote port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] portNum                  - physical port number
*
* @param[out] xOffThresholdPtr         - (Pointer to) xOff threshold in buffers
* @param[out] xOnThresholdPtr          - (Pointer to) xOn threshold in buffers
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChPortRemoteFcParametersGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *xOffThresholdPtr,
    OUT GT_U32                  *xOnThresholdPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRemoteFcParametersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, xOffThresholdPtr, xOnThresholdPtr));

    rc = internal_cpssDxChPortRemoteFcParametersGet(devNum, portNum, xOffThresholdPtr, xOnThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, xOffThresholdPtr, xOnThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortCheckAndGetMacNumberWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMacNumPtr            - (pointer to) mac number for the given port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortCheckAndGetMacNumberWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *portMacNumPtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(portMacNumPtr);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(CAST_SW_DEVNUM(devNum), portNum, *portMacNumPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortConvertMacToPortWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMacNumPtr            - (pointer to) mac number for the given port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortConvertMacToPortWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_U32               portMacNum,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortConvertMacToPortWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, portMacNum, portNumPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortLinkStatusGetWrapper function
* @endinternal
*
* @brief   Wrapper function for cpssDxChPortLinkStatusGet
*         in order to use GT_SW_DEV_NUM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isLinkUpPtr             -(pointer to) Link up/down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortLinkStatusGetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isLinkUpPtr
)
{

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortLinkStatusGetWrapper function wrapper");

    if(prvCpssDxChPortRemotePortCheck(CAST_SW_DEVNUM(devNum),portNum))
    {
        GT_STATUS rc = GT_OK;
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

        rc = cpssDxChBrgEgrFltPortLinkEnableGet(CAST_SW_DEVNUM(devNum), portNum, &portLinkStatusState);
        if(rc != GT_OK)
        {
            return rc;
        }
        *isLinkUpPtr = (portLinkStatusState == CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E) ? GT_TRUE:GT_FALSE;
        return rc;
    }
    else
    {
        return cpssDxChPortLinkStatusGet(CAST_SW_DEVNUM(devNum), portNum, isLinkUpPtr);
    }
}

/**
* @internal prvCpssDxChPortFaultSendSetWrapper function
* @endinternal
*
* @brief   Configure the port to start or stop sending fault signals to partner.
*         When port is configured to send, link on both sides will be down.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum;
*         Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - or stop sending
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChPortFaultSendSetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN  CPSS_PORT_SPEED_ENT           speed,
    IN  GT_BOOL                       send
)
{

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortFaultSendSetWrapper function wrapper");
    return prvCpssDxChPortFaultSendSet(CAST_SW_DEVNUM(devNum), portNum, ifMode, speed, send);
}

/**
* @internal prvCpssDxChPortForceLinkDownEnableSetWrapper function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnableSetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              state
)
{

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortForceLinkDownEnableSetWrapper function wrapper");
    return prvCpssDxChPortForceLinkDownEnableSet(CAST_SW_DEVNUM(devNum), portNum, state, GT_FALSE/*don't modify fault signals status*/);
}

/**
* @internal prvCpssDxChPortEnableSetWrapper function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableSetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    GT_BOOL currEnable = GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortEnableSetWrapper function wrapper portNum %d enable %d",portNum, enable);

    if(prvCpssDxChPortRemotePortCheck(CAST_SW_DEVNUM(devNum),portNum))
    {
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

        portLinkStatusState =  enable ? CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                                            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

        rc = cpssDxChBrgEgrFltPortLinkEnableSet(CAST_SW_DEVNUM(devNum), portNum, portLinkStatusState);
    }
    else
    {
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            rc = prvCpssDxChPortEnableGet(CAST_SW_DEVNUM(devNum), portNum, &currEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (enable != currEnable)
            {
                rc =  cpssDxChPortEnableSet(CAST_SW_DEVNUM(devNum), portNum, enable);
            }
        }
        else
        {
            rc =  cpssDxChPortEnableSet(CAST_SW_DEVNUM(devNum), portNum, enable);
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChPortEnableGetWrapper function
* @endinternal
*
* @brief   Get status a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[out] statusPtr               - port enable status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableGetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statusPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortEnableGetWrapper function wrapper portNum %d ",portNum);

    rc =  prvCpssDxChPortEnableGet(CAST_SW_DEVNUM(devNum), portNum, statusPtr);
    if (rc != GT_OK)
    {
       return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSpeedGetWrapper function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvCpssDxChPortSpeedGetWrapper
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortSpeedGetWrapper function wrapper");
    return cpssDxChPortSpeedGet(CAST_SW_DEVNUM(devNum), portNum, speedPtr);
}

/**
* @internal prvCpssDxChPortSerdesGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortSerdesGetWrapper
(
    IN  GT_SW_DEV_NUM         devNum
)
{
    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortSerdesGetWrapper function wrapper");
    return prvCpssDxChHwInitNumOfSerdesGet(CAST_SW_DEVNUM(devNum));
}

/**
* @internal prvCpssDxChCgConvertersRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCgConvertersRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside CgConvertersRegAddrGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(CAST_SW_DEVNUM(devNum), portMacNum, regAddrPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChCgConvertersStatus2RegAddrGetWrapper
*           function
* @endinternal
*
* @brief   Wrapper function for CPSS macro
*         PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC in
*         order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCgConvertersStatus2RegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside CgConvertersStatus2RegAddrGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC(CAST_SW_DEVNUM(devNum), portMacNum, regAddrPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortManagerDbGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting PRV_CPSS_PORT_MNG_DB_STC structure pointer from
*         CPSS dxch pp structure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] portManagerDbPtr         - (pointer to) pointer to port manager DB.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortManagerDbGetWrapper
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT PRV_CPSS_PORT_MNG_DB_STC **portManagerDbPtr
)
{

    CPSS_LOG_INFORMATION_MAC("inside PortManagerDbGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(portManagerDbPtr);

    if ( !(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) ||
           PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                                      "port manager DB not applicable for device=%d",devNum);
    }

    *portManagerDbPtr = (PRV_CPSS_PORT_MNG_DB_STC* )(&(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portManagerDb));

    return GT_OK;
}

/**
* @internal prvCpssDxChMPCS40GRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting the MPCS 40G common status register address.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMPCS40GRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside MPCS40GRegAddrGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].mpcs40GCommonStatus;

    return GT_OK;
}

/**
* @internal prvCpssDxChMacDmaModeRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting XLG DMA mode register address
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMacDmaModeRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portMacNum,
    OUT GT_U32                  *regAddrPtr
)
{
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    CPSS_LOG_INFORMATION_MAC("inside MacDmaModeRegAddrGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(CAST_SW_DEVNUM(devNum),portMacNum);
    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, portMacType, regAddrPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChLinkStatusChangedSIP5WAs function
* @endinternal
*
* @brief   Wrapper function for handling WAs related to link
*          status change event (for sip 5).
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2;
*         Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] linkUp                   - link up status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is currently implementing nothing and is defined for future use.
*
*/
GT_U32 prvCpssDxChLinkStatusChangedSIP5WAs
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_STATUS               linkUp
)
{
    GT_STATUS                       rc;
    CPSS_PORT_MAC_TYPE_ENT          portMacType;
    GT_U32                          regAddr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    GT_U32                          timeout; /* resources free timeout counter */
    GT_U16                          portTxqDescNum = 0; /* number of not treated TXQ descriptors */
    GT_BOOL                         portShaperEnable; /* current state of port shaper */
    GT_BOOL                         portTcShaperEnable[CPSS_TC_RANGE_CNS]; /* current state of port TC shapers */
    GT_U8                           tc; /* traffic class */
    GT_BOOL                         portEnabled;
    GT_BOOL                         isPortInUnidirectionalMode;/*is port of a 'link down' in mode of 'Unidirectional' */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    CPSS_DXCH_IMPLEMENT_WA_ENT      waArr[1];
    GT_U32                          additionalInfoBmpArr[1];


    /* work arround appearing wrong MIB counters after port link down */
    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_WRONG_MIB_COUNTERS_LINK_DOWN_E;
    additionalInfoBmpArr[0] = portNum;
    rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum),1,&waArr[0],
                                    &additionalInfoBmpArr[0]);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                "cpssDxChHwPpImplementWaInit: wrong MIB counters after port link down error, devNum=%d, port=%d, rc=%d\n",
                devNum, portNum, rc);
    }

    /* WA for fixing CRC errors when Auto-Neg is disabled on 10M/100M port speed */
    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_100BASEX_AN_DISABLE_E;
    additionalInfoBmpArr[0] = portNum;
    rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum), 1, &waArr[0], &additionalInfoBmpArr[0]);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                "cpssDxChHwPpImplementWaInit: fixing CRC errors when Auto-Neg is disabled on 10M/100M port speed, devNum=%d, port=%d, rc=%d\n",
                devNum, portNum, rc);
    }

    /*checking if device below sip 5_20, no need to proceed for devices below sip 5_20*/
    if (!(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        return GT_OK;
    }

    rc = cpssDxChPortPhysicalPortDetailedMapGet(CAST_SW_DEVNUM(devNum), portNum, &portMap);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortPhysicalPortDetailedMapGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, portNum, rc);
    }

    rc = cpssDxChPortMacTypeGet(CAST_SW_DEVNUM(devNum), portNum, &portMacType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortMacTypeGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, portNum, rc);
    }

    isPortInUnidirectionalMode = GT_FALSE;
    /* CG MAC does not support unidirectional mode */
    if(linkUp == GT_FALSE)
    {
        if(portMacType != CPSS_PORT_MAC_TYPE_CG_E)
        {
            rc = cpssDxChVntOamPortUnidirectionalEnableGet(CAST_SW_DEVNUM(devNum), portNum, &isPortInUnidirectionalMode);
            if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                isPortInUnidirectionalMode = GT_FALSE;
                rc = GT_OK;
            }

            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChVntOamPortUnidirectionalEnableGet: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, portNum, rc);

                /* 100G (CG) : the port not supports the feature */
            }
        }
        if(isPortInUnidirectionalMode == GT_TRUE)
        {
            /* !!! do not modify the filter !!! keep it as 'link up' */
            return GT_OK;
        }
    }

    /* 2. set CG Flush configuration */
    if((portMacType == CPSS_PORT_MAC_TYPE_CG_E) && (linkUp == GT_FALSE))
    {
        regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMap.portMap.macNum).CGPORTMACCommandConfig;

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
             rc = prvCpssDrvHwPpSetRegField(
                 CAST_SW_DEVNUM(devNum), regAddr, 22, 1, 1);
             if (rc != GT_OK)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssHwPpSetRegField: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, portNum, rc);
             }
        }
    }

    portLinkStatusState = linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    if(linkUp == GT_TRUE)
    {
        rc = cpssDxChPortEnableGet(CAST_SW_DEVNUM(devNum),portNum,&portEnabled);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortEnableGet: error, devNum=%d, port=%d, rc=%d\n",
                   devNum, portNum, rc);
        }

        /* if the caller set the MAC to be disabled ...
           we need to assume that the EGF filter should treat as 'link down' */
        if(rc == GT_OK && portEnabled == GT_FALSE)
        {
            portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
        }

        /* In case of link-up need to check if the application didn't request explicitly to do force link down */
        if (PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "portEgfForceStatusBitmapPtr is NULL");
        }
        /* fetching Egf status from db */
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum))
        {
            portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
        }
        else
        {
            portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;
        }
    }

    /* 3. need to set the EGF link status filter according to new state of the
          port while keeping the application decision intact  */
    rc = prvCpssDxChHwEgfEftFieldSet(
        CAST_SW_DEVNUM(devNum),portNum,
        PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
        PRV_CPSS_DXCH_EGF_CONVERT_MAC(portLinkStatusState));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChBrgEgrFltPortLinkEnableSet: error, devNum=%d, port=%d, link state[%d] rc=%d\n",
                devNum, portNum, portLinkStatusState, rc);
    }

    if(linkUp == GT_FALSE)
    {
        /****************************************/
        /* 4. Disable any shapers on given port */
        /****************************************/
        rc = cpssDxChPortTxShaperEnableGet(CAST_SW_DEVNUM(devNum), portNum, &portShaperEnable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxShaperEnableGet: error, devNum=%d, port=%d\n", devNum, portNum);
        }
        for(tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = cpssDxChPortTxQShaperEnableGet(
                CAST_SW_DEVNUM(devNum), portNum, tc, &portTcShaperEnable[tc]);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxQShaperEnableGet: error, devNum=%d, port=%d tc =%d\n", devNum, portNum, tc);
            }
        }
        rc = cpssDxChTxPortShapersDisable(CAST_SW_DEVNUM(devNum), portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChTxPortShapersDisable: error, devNum=%d, port=%d\n", devNum, portNum);
        }

        /*******************************************************/
        /* 5. Polling the TXQ port counter until it reach zero */
        /*******************************************************/
        for(timeout = 500; timeout > 0; timeout--)
        {
            rc = cpssDxChPortTxDescNumberGet(
                CAST_SW_DEVNUM(devNum), portNum, &portTxqDescNum);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(0 == portTxqDescNum)
                break;
            else
                cpssOsTimerWkAfter(10);
        }

        if(0 == timeout)
        {
            cpssOsPrintf("cpssDxChPortTxDescNumberGet: TXQ descriptor counter read timeout error for port=%d,portTxqDescNum=0x%x \n", portNum, portTxqDescNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }

        /*********************************************/
        /* 6. wait 1us for the TXQ-MAC path to drain */
        /*********************************************/
        cpssOsTimerWkAfter(1);

        /*************************************/
        /* 7. Re-enable all disabled shapers */
        /*************************************/
        rc = cpssDxChPortTxShaperEnableSet(
            CAST_SW_DEVNUM(devNum), portNum, portShaperEnable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxShaperEnableSet: error, devNum=%d, port=%d\n", devNum, portNum);
        }

        for(tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = cpssDxChPortTxQShaperEnableSet(
                CAST_SW_DEVNUM(devNum), portNum, tc, portTcShaperEnable[tc]);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxQShaperEnableSet: error, devNum=%d, port=%d tc =%d\n", devNum, portNum, tc);
            }
        }

        /***********************************/
        /* 8. unset CG Flush configuration */
        /***********************************/
        if(portMacType == CPSS_PORT_MAC_TYPE_CG_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMap.portMap.macNum).CGPORTMACCommandConfig;

            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                 rc = prvCpssDrvHwPpSetRegField(
                     CAST_SW_DEVNUM(devNum), regAddr, 22, 1, 0);
                 if (rc != GT_OK)
                 {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssHwPpSetRegField: error, devNum=%d, port=%d, rc=%d\n",
                            devNum, portNum, rc);
                 }
            }
        }
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChLinkStatusChangedSIP6WAs function
* @endinternal
*
* @brief   Wrapper function for handling WAs related to link
*          status change event (for sip 6).
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2;
*         Bobcat2,Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] linkUp                   - link up status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is currently implementing nothing and is defined for future use.
*
*/
GT_U32 prvCpssDxChLinkStatusChangedSIP6WAs
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_STATUS               linkUp
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT  portLinkStatusState;
    GT_BOOL                                     portEnabled;

    portLinkStatusState = linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    rc = cpssDxChPortEnableGet(CAST_SW_DEVNUM(devNum),portNum,&portEnabled);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortEnableGet: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, portNum, rc);
    }

    /* 3. need to set the EGF link status filter according to new state of the
       port.
        Note: (unlike sip 5.20)
            cpssDxChBrgEgrFltPortLinkEnableSet implementation checks port link enable status and allows
            FORCE_LINK_UP_E only if port TX enabled and there is TXQ resources allocated.
         */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(CAST_SW_DEVNUM(devNum), portNum, portLinkStatusState);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChBrgEgrFltPortLinkEnableSet: error, devNum=%d, port=%d, link state[%d] rc=%d\n",
                devNum, portNum, portLinkStatusState, rc);
    }

    if((linkUp == GT_FALSE))
    {
        /*Disable port,this will cause "open drain"*/
        rc = cpssDxChPortEnableSet(CAST_SW_DEVNUM(devNum), portNum,GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortEnableSet: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, portNum, rc);

        }

        /*Restore previus port state*/
        if(portEnabled == GT_TRUE)
        {
            rc = cpssDxChPortEnableSet(CAST_SW_DEVNUM(devNum), portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortEnableSet: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, portNum, rc);
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLinkStatusChangedWAs function
* @endinternal
*
* @brief   Wrapper function for handling WAs related to link status change event.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portType                 - AP or Regular
* @param[in] LinkUp                   - Link up status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is currently implementing nothing and is defined for future use.
*
*/
GT_U32 prvCpssDxChLinkStatusChangedWAs
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    IN  GT_BOOL                         linkUp
)
{
    GT_STATUS rc;

    CPSS_LOG_INFORMATION_MAC("inside LinkStatusChangedWAs function wrapper");

    if ( !(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) ||
           PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                                      "LinkStatusChangedWAs not applicable for device=%d",devNum);
    }

    CPSS_LOG_INFORMATION_MAC("(port %d) mac_level changed", portNum);
    if ((portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E) && PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        /*If AP and XCAT3, doorbell interrupts is not supported, and enable/disable port is not done on SrvCpu level*/
        rc = cpssDxChPortEnableSet(devNum, portNum, linkUp);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortEnableSet: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, portNum);
        }
    }

    /*sip6*/
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChLinkStatusChangedSIP6WAs(devNum, portNum, linkUp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLinkStatusChangedSIP6WAs: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, portNum, rc);
        }
    }
    else /* sip5 */
    {
        rc = prvCpssDxChLinkStatusChangedSIP5WAs(devNum, portNum, linkUp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLinkStatusChangedSIP5_20WAs: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, portNum, rc);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacConfigurationWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssDxChPortMacConfiguration
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regDataArray             - (pointer to) registers data array to use.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacConfigurationWrapper
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  const PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
)
{
    GT_U32 portMacNum;
    GT_STATUS rc;

    CPSS_LOG_INFORMATION_MAC("inside PortMacConfigurationWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(regDataArray);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    rc = prvCpssDxChPortMacConfiguration(CAST_SW_DEVNUM(devNum), portNum, ( const PRV_CPSS_DXCH_PORT_REG_CONFIG_STC *)regDataArray);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortMacConfigurationWrapper failed=%d", rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortMacConfigurationClearWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssDxChPortMacConfigurationClear
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacConfigurationClearWrapper
(
    INOUT PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
)
{
    GT_STATUS rc;

    CPSS_LOG_INFORMATION_MAC("inside PortMacConfigurationClearWrapper function wrapper");

    CPSS_NULL_PTR_CHECK_MAC(regDataArray);

    rc = prvCpssDxChPortMacConfigurationClear((PRV_CPSS_DXCH_PORT_REG_CONFIG_STC *)regDataArray);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortMacConfigurationClearWrapper failed=%d", rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortInternalLoopbackEnableSetWrapper function
* @endinternal
*
* @brief   Wrapper function for setting the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For port 25 in DX269 (XG/HX port) it's the application responsibility to
*       reconfigure the loopback mode after switching XG/HX mode.
*
*/
GT_STATUS prvCpssDxChPortInternalLoopbackEnableSetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;

    rc = cpssDxChPortInternalLoopbackEnableSet(CAST_SW_DEVNUM(devNum), portNum, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssDxChPortInternalLoopbackEnableSet from wrapper failed=%d");
    }

    return GT_OK;
}

/*************************************************************************
* @internal numOfLanesForSerdesMuxCheck
* @endinternal
*
* @brief  Check if a MAC is allowed for 'SERDES muxing' (is it 'first in group')
*           the function also return the number of MACs need to be checked for 'no speed','no interface'
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                 - system device number
* @param[in] PortNum                - Physical port number
* @param[in] portMacNum             - MAC port number
*
* @param[out] numOfLanesPtr         - (pointer to) the number of MACs to check
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
************************************************************************/
static GT_STATUS numOfLanesForSerdesMuxCheck(
    IN GT_U8              devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32             portMacNum,
    OUT GT_U32            *numOfLanesPtr
)
{
    GT_U32   numOfLanes = 0;

    *numOfLanesPtr = 0;
    portNum = portNum;/* used only for error LOG */

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        numOfLanes = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numLanesPerPort;
        /* lane muxing is supported only in first mac of every gop */
        if ((portMacNum % numOfLanes) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"numOfLanes=%d, portNum=%d, portMacNum=%d",numOfLanes, portNum, portMacNum);
        }
        if(portMacNum >= 72)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"portNum=%d, portMacNum=%d >= [%d]",
                portNum, portMacNum ,
                72);
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        numOfLanes = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numLanesPerPort;
        /* lane muxing is supported only in first mac of every gop */
        if ((portMacNum % numOfLanes) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"numOfLanes=%d, portNum=%d, portMacNum=%d",numOfLanes, portNum, portMacNum);
        }

        if(portMacNum >= (64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"portNum=%d, portMacNum=%d >= [%d]",
                portNum, portMacNum ,
                (64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles));
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        numOfLanes = 26;/* all 26 MACs must not hold speed and interface when
            checked in internal_cpssDxChPortLaneMacToSerdesMuxSet*/
        /* lane muxing is supported only in first mac of every gop */
        if ((portMacNum % numOfLanes) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"numOfLanes=%d, portNum=%d, portMacNum=%d",numOfLanes, portNum, portMacNum);
        }
        if(portMacNum >= (4 * numOfLanes))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"portNum=%d, portMacNum=%d >= [%d]",
                portNum, portMacNum ,
                (4 * numOfLanes));
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        numOfLanes = 4;
        /* all other ports are using 'single serdes' , or are not 'first in group' */
        if(portMacNum != 50)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"portNum=%d, portMacNum=%d != [%d]",
                portNum, portMacNum ,
                50);
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        numOfLanes = 16;/* all 16 MACs must not hold speed and interface when
            checked in internal_cpssDxChPortLaneMacToSerdesMuxSet*/
        /* lane muxing is supported only in first mac of every gop */
        if ((portMacNum % numOfLanes) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"numOfLanes=%d, portNum=%d, portMacNum=%d",numOfLanes, portNum, portMacNum);
        }
        if(portMacNum > (3 * numOfLanes))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"portNum=%d, portMacNum=%d >= [%d]",
                portNum, portMacNum ,
                (3 * numOfLanes));
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* all the ports are using 'single serdes' (no KR2/KR4/KR8) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "No SERDES muxing in Ironman");
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    *numOfLanesPtr = numOfLanes;

    return GT_OK;
}


/*************************************************************************
* @internal internal_cpssDxChPortLaneMacToSerdesMuxSet
* @endinternal
 *
* @brief   Set the Port lane Mac to Serdes Mux
 *
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on pointer problem
 *
* @param[in] devNum                 - system device number
* @param[in] PortNum                - Physical port number
* @param[in] macToSerdesMuxStcPtr   - the setup of the muxing
************************************************************************/
GT_STATUS internal_cpssDxChPortLaneMacToSerdesMuxSet
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_MAC_TO_SERDES_STC      *macToSerdesMuxStcPtr
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portMacNum;
    GT_U32                          numOfLanes;
    GT_U32                          i;
    MV_HWS_PORT_SERDES_TO_MAC_MUX   tmpMacToSerdesMuxStc;

    CPSS_NULL_PTR_CHECK_MAC(macToSerdesMuxStcPtr);

    /* check device number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* converting to portMacNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = numOfLanesForSerdesMuxCheck(devNum, portNum ,portMacNum , &numOfLanes);
    /* lane muxing is supported only in first mac of every gop */
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "numOfLanesForSerdesMuxCheck : Failed for MAC_PORT = %d", portMacNum);
    }

    /* checking if one of the ports from the gop is in not deleted */
    for (i = 0; i < numOfLanes; i++)
    {
        if((portMacNum + i) < PRV_CPSS_PP_MAC(devNum)->numOfPorts)
        {
            if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum + i].portIfMode != CPSS_PORT_INTERFACE_MODE_NA_E )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "cpssDxChPortLaneMacToSerdesMuxSet : cannot set serdes muxing when there is linkup on one of the ports in the group of ports, ifmode[MAC_%d]=%d", (portMacNum + i), PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum + i].portIfMode);
            }
        }
    }

    for(i = 0 ; i < MV_HWS_MAX_LANES_NUM_PER_PORT ; i++)
    {
        tmpMacToSerdesMuxStc.serdesLanes[i] = macToSerdesMuxStcPtr->serdesLanes[i];
    }

    /* config hw */
    rc = mvHwsPortLaneMacToSerdesMuxSet(devNum, 0, portMacNum, &tmpMacToSerdesMuxStc);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsLaneMacToSerdesMuxSet : Hws Failed for MAC_PORT = %d", portMacNum);
    }

    return rc;
}

/*************************************************************************
* @internal cpssDxChPortLaneMacToSerdesMuxSet
* @endinternal
 *
* @brief   Set the Port lane Mac to Serdes Mux
 *
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on pointer problem
 *
* @param[in] devNum                 - system device number
* @param[in] PortNum                - Physical port number
* @param[in] macToSerdesMuxStcPtr   - the setup of the muxing
************************************************************************/
GT_STATUS cpssDxChPortLaneMacToSerdesMuxSet
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_MAC_TO_SERDES_STC      *macToSerdesMuxStcPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLaneMacToSerdesMuxSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macToSerdesMuxStcPtr));

    rc = internal_cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum, macToSerdesMuxStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesToMacMuxStr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*************************************************************************
* @internal internal_cpssDxChPortLaneMacToSerdesMuxGet
* @endinternal
 *
* @brief   Get the Port lane Mac to Serdes Mux
 *
* @note   APPLICABLE DEVICES: Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on pointer problem
 *
* @param[in] devNum                 - system device number
* @param[in] portNum                - physical port number
* @param[out] macToSerdesMuxStcPtr  - pointer to struct that will contaion the mux setup from the database
************************************************************************/
GT_STATUS internal_cpssDxChPortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_TO_SERDES_STC     *macToSerdesMuxStcPtr
)
{
    GT_U32                          portMacNum;
    GT_U32                          numOfLanes;
    GT_U8                           i;
    GT_STATUS                       rc;
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;

    CPSS_NULL_PTR_CHECK_MAC(macToSerdesMuxStcPtr);

    /* check device number */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* converting to portMacNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = numOfLanesForSerdesMuxCheck(devNum, portNum ,portMacNum , &numOfLanes);
    /* lane muxing is supported only in first mac of every gop */
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = mvHwsPortLaneMacToSerdesMuxGet(devNum, 0, portMacNum, &hwsMacToSerdesMuxStc);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsLaneMacToSerdesMuxGet : Hws Failed");
    }

    for(i = 0 ; i < MV_HWS_MAX_LANES_NUM_PER_PORT ; i++)
    {
        macToSerdesMuxStcPtr->serdesLanes[i] = hwsMacToSerdesMuxStc.serdesLanes[i];
    }

    return GT_OK;
}

/*************************************************************************
* @internal cpssDxChPortLaneMacToSerdesMuxGet
* @endinternal
 *
* @brief   Get the Port lane Mac to Serdes Mux
 *
* @note   APPLICABLE DEVICES: Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on pointer problem
 *
* @param[in] devNum                 - system device number
* @param[in] portNum                - physical port number
* @param[out] macToSerdesMuxStcPtr  - pointer to struct that will contaion the mux setup from the database
************************************************************************/
GT_STATUS cpssDxChPortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_TO_SERDES_STC    *macToSerdesMuxStcPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLaneMacToSerdesMuxGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macToSerdesMuxStcPtr));

    rc = internal_cpssDxChPortLaneMacToSerdesMuxGet(devNum, portNum, macToSerdesMuxStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesToMacMuxStr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortDebugLogEntrySet function
* @endinternal
*
* @brief   Set CPSS logging mode for all libs and all log-types and enter (or exit)
*         port lib cpss logger. For debug purposes.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   - whether to  or disable logging
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note This API is for debug purposes only. It is public CPSS API in order for
*       the automatic tools to keep generate log data for this API. It is usefull
*       for Hws APIs in which when calling them directly, the Hws debug information
*       will be treated as CPSS log.
*
*/
GT_STATUS cpssDxChPortDebugLogEntrySet
(
    IN  GT_BOOL   enable
)
{
#if (defined(CPSS_LOG_ENABLE) && defined(CPSS_LOG_IN_MODULE_ENABLE))
/*whether generic log enabled and whether log enabled on module, at top of file*/
    GT_STATUS rc;
#endif

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDebugLogEntrySet);

    if (enable)
    {
        /* enable cpss logging */
#if (defined(CPSS_LOG_ENABLE) && defined(CPSS_LOG_IN_MODULE_ENABLE))
/*whether generic log enabled and whether log enabled on module, at top of file*/
        cpssLogEnableSet(GT_TRUE);
        rc = cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ALL_E, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
#endif
        /* emulate CPSS API enter */
        CPSS_LOG_API_ENTER_MAC((funcId, enable));
    }
    else
    {
        /* emulate CPSS API exit */
        CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
        /* disable cpss logging */
#if (defined(CPSS_LOG_ENABLE) && defined(CPSS_LOG_IN_MODULE_ENABLE))
/*whether generic log enabled and whether log enabled on module, at top of file*/
        cpssLogEnableSet(GT_FALSE);
        rc = cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ALL_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
#endif
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortEnableWaWithLinkStatusSet function
* @endinternal
*
* @brief   Enable/Disable unidirectional port according to found link status.
*          For bidirectional ports done nothing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEnableWaWithLinkStatusSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  port
)
{
   GT_STATUS               rc;
   GT_U32                  portMacNum;
   GT_BOOL                 linkUp;
   GT_U32                  xlgUnidirectional;
   CPSS_PORTS_BMP_STC      *xlgUniDirPortsBmpPtr;

   PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
   PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, port, portMacNum);

   xlgUniDirPortsBmpPtr =
       &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
       info_PRV_CPSS_DXCH_PORTS_XLG_UNIDIRECTIONAL_WA_E.xlgUniDirPortsBmp);

   xlgUnidirectional = CPSS_PORTS_BMP_IS_PORT_SET_MAC(xlgUniDirPortsBmpPtr, port);
   if (xlgUnidirectional == 0)
   {
       /* port is not subject of WA */
       return GT_OK;
   }

   rc = cpssDxChPortLinkStatusGet(devNum, port, &linkUp);
   if (rc != GT_OK)
   {
       return rc;
   }

   rc = cpssDxChPortEnableSet(devNum, port, linkUp);

   return rc;
}

/**
* @internal cpssDxChPortEnableWaWithLinkStatusSet function
* @endinternal
*
* @brief   Enable/Disable unidirectional port according to found link status.
*          For bidirectional ports done nothing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEnableWaWithLinkStatusSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  port
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEnableWaWithLinkStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port));

    rc = internal_cpssDxChPortEnableWaWithLinkStatusSet(devNum, port);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

void prvCpssDxChPortXlgUnidirectionalEnableSetTraceEnableSet(GT_U32 enable)
{
    PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_SET(prvCpssDxChPortXlgUnidirectionalEnableSetTraceEnable, enable);
}

/**
* @internal prvCpssDxChPortXlgUnidirectionalEnableSet function
* @endinternal
*
* @brief   Enable/Disable the XLG port for unidirectional transmit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
* @param[in] enable                   - GT_TRUE:   Enable the port for unidirectional transmit.
*                                       GT_FALSE:  Disable the port for unidirectional transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortXlgUnidirectionalEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    port,
    IN GT_BOOL  enable
)
{
    GT_STATUS               rc;             /* return status                */
    GT_U32                  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32                  regAddr;        /* register address             */
    GT_U32                  regData;        /* register fdata               */
    GT_U32                  offset;         /* offset in register           */
    GT_U32                  data;           /* register field data          */
    GT_U32                  mask;           /* register field mask          */
    PRV_CPSS_PORT_TYPE_ENT  portMacType;    /* port MAC type                */
    GT_U32                  portMac;        /* port MAC number              */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,port,portMac);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMac);

    /* Enable/Disable the port for unidirectional transmit */

    portMacType = PRV_CPSS_PORT_XLG_E;
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum, portMac, portMacType, &regAddr);
    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* port has no XGL MAC*/
        return GT_OK;
    }

    rc =  prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    offset = 8;
    data = (((enable != GT_FALSE) ? 1 : 0) << offset);
    mask = (1 << offset);
    if ((regData & mask) == data)
    {
        /* unidirectional status already as required */
        return GT_OK;
    }

    if (PRV_NON_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(prvCpssDxChPortXlgUnidirectionalEnableSetTraceEnable))
    {
        cpssOsPrintf(
            "prvCpssDxChPortXlgUnidirectionalEnableSet dev %d port %d enable %d\n",
            devNum, port, enable);
    }

    regData = ((regData & (~ mask)) | data);
    rc =  prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, regData);
    return rc;
}

/**
* @internal prvCpssDxChPortXlgBufferStuckWaEnableSet function
* @endinternal
*
:$
* @brief   Enable/Disable the XLG port Link status WA using unidirectional transmit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
* @param[in] enable                   - GT_TRUE:   Enable the port for unidirectional transmit.
*                                       GT_FALSE:  Disable the port for unidirectional transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortXlgBufferStuckWaEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    port,
    IN GT_BOOL  enable
)
{
    GT_STATUS               rc;
    CPSS_PORTS_BMP_STC      *xlgUniDirPortsBmpPtr;
    GT_BOOL                 trafficWasEnable;
    GT_BOOL                 loopBackEnable;
    GT_BOOL                 portWasXlgUnidir;
    GT_BOOL                 portHwXlgUnidir;

    xlgUniDirPortsBmpPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_PORTS_XLG_UNIDIRECTIONAL_WA_E.xlgUniDirPortsBmp);

    portWasXlgUnidir = CPSS_PORTS_BMP_IS_PORT_SET_MAC(xlgUniDirPortsBmpPtr, port);

    if ((enable != GT_FALSE) && portWasXlgUnidir)
    {
        /* adding port found in DB */
        return GT_OK;
    }
    if ((enable == GT_FALSE) && (portWasXlgUnidir == 0))
    {
        /* removing port not found in DB */
        return GT_OK;
    }

    if (enable != GT_FALSE)
    {
        /* new port being added to xlg-unidirectional ports DB */
        trafficWasEnable = GT_FALSE;
        rc = prvCpssDxChPortEnableGet(devNum, port, &trafficWasEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
        loopBackEnable = GT_FALSE;
        rc = cpssDxChPortInternalLoopbackEnableGet(
            devNum, port, &loopBackEnable);
        if (rc != GT_OK)
        {
            if ((trafficWasEnable == GT_FALSE) && (rc == GT_NOT_INITIALIZED))
            {
                /* the legal state for powered down port */
                loopBackEnable = GT_FALSE;
            }
            else
            {
                return rc;
            }
        }
        portHwXlgUnidir =
            ((trafficWasEnable != GT_FALSE) && (loopBackEnable == GT_FALSE))
            ? GT_TRUE : GT_FALSE;
        rc = prvCpssDxChPortXlgUnidirectionalEnableSet(devNum, port, portHwXlgUnidir);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_PORTS_BMP_PORT_SET_MAC(xlgUniDirPortsBmpPtr, port);
    }
    else
    {
        /* old port being removed from  xlg-unidirectional ports DB */
        rc = prvCpssDxChPortXlgUnidirectionalEnableSet(devNum, port, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
        CPSS_PORTS_BMP_PORT_CLEAR_MAC(xlgUniDirPortsBmpPtr, port);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortMacPcsStatusGet function
* @endinternal
*
* @brief   port mac pcs status collector.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatusPtr          - port status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPortMacPcsStatusGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_STATUS_STC *portStatusPtr
)
{
    GT_U32                     portMacNum;
    GT_STATUS                  rc;
    GT_U32                     portGroup;
    MV_HWS_PORT_STATUS_STC     result;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portStatusPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = mvHwsPortMacPcsStatus(devNum,portGroup,portMacNum,&result);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsPortMacPcsStatus failed=%d");
    }

    cpssOsMemCpy(portStatusPtr,&result,  sizeof(CPSS_DXCH_PORT_STATUS_STC));

    return GT_OK;
}

/**
* @internal cpssDxChPortMacPcsStatusGet function
* @endinternal
*
* @brief   port mac pcs status collector.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatusPtr          - port status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacPcsStatusGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_STATUS_STC *portStatusPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortMacPcsStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portStatusPtr));

    rc = internal_cpssDxChPortMacPcsStatusGet(devNum, portNum, portStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChRsFecStatusGet function
* @endinternal
*
* @brief   RSFEC status collector.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] rsfecStatusPtr          - RSFEC counters/status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChRsFecStatusGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_RSFEC_STATUS_STC *rsfecStatusPtr
)
{
    GT_U32                          portMacNum;
    GT_STATUS                       rc;
    GT_U32                      portGroup;
    MV_HWS_RSFEC_STATUS_STC     result;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rsfecStatusPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = mvHwsRsfecStatusGet(devNum,portGroup,portMacNum,&result);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsRsfecStatusGet failed=%d");
    }
    rsfecStatusPtr->ampsLock = result.ampsLock;
    rsfecStatusPtr->fecAlignStatus = result.fecAlignStatus;
    rsfecStatusPtr->highSer = result.highSer;

    cpssOsMemCpy(rsfecStatusPtr->symbolError,result.symbolError,  sizeof(rsfecStatusPtr->symbolError));

    return GT_OK;
}

/**
* @internal cpssDxChRsFecStatusGet function
* @endinternal
*
* @brief   RSFEC status collector.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] rsfecStatusPtr          - RSFEC counters/status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChRsFecStatusGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_RSFEC_STATUS_STC *rsfecStatusPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChRsFecStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rsfecStatusPtr));

    rc = internal_cpssDxChRsFecStatusGet(devNum, portNum, rsfecStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rsfecStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChRsFecCounterGet function
* @endinternal
*
* @brief   Return RS-FEC counters.
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS internal_cpssDxChRsFecCounterGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_RSFEC_COUNTERS_STC *rsfecCountersPtr
)
{
    GT_U32                      portMacNum;
    GT_STATUS                   rc;
    GT_U32                      portGroup;
    MV_HWS_RSFEC_COUNTERS_STC   result = {{{0}},{{0}},{{0}},{0},{{0}}};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rsfecCountersPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = mvHwsRsFecCorrectedError(devNum, portGroup, portMacNum, &result);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsRsFecCorrectedError failed=%d");
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && HWS_DEV_SILICON_TYPE(devNum) != AC5X)
    {
        /** need to add AC5X support   */
        rsfecCountersPtr->correctFecCodeword.l[0] = result.correctFecCodeword.l[0];
        rsfecCountersPtr->correctFecCodeword.l[1] = result.correctFecCodeword.l[1];

        rsfecCountersPtr->correctedFecCodeword.l[0] = result.correctedFecCodeword.l[0];
        rsfecCountersPtr->correctedFecCodeword.l[1] = result.correctedFecCodeword.l[1];

        rsfecCountersPtr->uncorrectedFecCodeword.l[0] = result.uncorrectedFecCodeword.l[0];
        rsfecCountersPtr->uncorrectedFecCodeword.l[1] = result.uncorrectedFecCodeword.l[1];

        rsfecCountersPtr->total_cw_received.l[0] = result.total_cw_received.l[0];
        rsfecCountersPtr->total_cw_received.l[1] = result.total_cw_received.l[1];
    }
    else
    {
        rsfecCountersPtr->correctedFecCodeword.l[0] = result.correctedFecCodeword.l[0];
        rsfecCountersPtr->uncorrectedFecCodeword.l[0] = result.uncorrectedFecCodeword.l[0];

        cpssOsMemCpy(rsfecCountersPtr->symbolError,result.symbolError,  sizeof(rsfecCountersPtr->symbolError));
    }

    return GT_OK;
}

/**
* @internal cpssDxChRsFecCounterGet function
* @endinternal
*
* @brief   Return RS-FEC counters.
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssDxChRsFecCounterGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_RSFEC_COUNTERS_STC *rsfecCountersPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChRsFecCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rsfecCountersPtr));

    rc = internal_cpssDxChRsFecCounterGet(devNum, portNum, rsfecCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rsfecCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFcFecCounterGet function
* @endinternal
*
* @brief   Return FC-FEC counters.
*
* @note   APPLICABLE DEVICES:       AC3X; Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS internal_cpssDxChFcFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_FCFEC_COUNTERS_STC    *fcfecCountersPtr
)
{
    GT_U32                      portMacNum;
    GT_STATUS                   rc;
    GT_U32                      portGroup;
    MV_HWS_FCFEC_COUNTERS_STC   result={0,0,0,0,0,0,{0},{0}};


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(fcfecCountersPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_FALCON_E | CPSS_AC5X_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = mvHwsFcFecCounterGet(devNum, portGroup, portMacNum, &result);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsFcFecCorrectedError failed=%d");
        }
    }
    else
    {
        rc = mvHwsMMPcs28nmFcFecCorrectedError(devNum, portGroup, portMacNum, &result);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsMMPcs28nmFcFecCountersGet failed=%d");
        }
    }

    fcfecCountersPtr->numReceivedBlocks = result.numReceivedBlocks;
    fcfecCountersPtr->numReceivedBlocksNoError = result.numReceivedBlocksNoError;
    fcfecCountersPtr->numReceivedBlocksCorrectedError = result.numReceivedBlocksCorrectedError;
    fcfecCountersPtr->numReceivedBlocksUncorrectedError = result.numReceivedBlocksUncorrectedError;
    fcfecCountersPtr->numReceivedCorrectedErrorBits = result.numReceivedCorrectedErrorBits;
    fcfecCountersPtr->numReceivedUncorrectedErrorBits = result.numReceivedUncorrectedErrorBits;

    cpssOsMemCpy(fcfecCountersPtr->blocksCorrectedError,result.blocksCorrectedError,  sizeof(fcfecCountersPtr->blocksCorrectedError));
    cpssOsMemCpy(fcfecCountersPtr->blocksUncorrectedError,result.blocksUncorrectedError,  sizeof(fcfecCountersPtr->blocksUncorrectedError));
    return GT_OK;
}

/**
* @internal cpssDxChFcFecCounterGet function
* @endinternal
*
* @brief   Return FC-FEC counters.
*
* @note   APPLICABLE DEVICES:       AC3X; Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssDxChFcFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_FCFEC_COUNTERS_STC    *fcfecCountersPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFcFecCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, fcfecCountersPtr));

    rc = internal_cpssDxChFcFecCounterGet(devNum, portNum, fcfecCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, fcfecCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortSerdesActiveLanesListGet function
* @endinternal
*
* @brief   Return number of active serdeses and array of active
*          serdeses numbers that port uses
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] activeLanePtr          - number of active lanes
*                                        and list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS internal_cpssDxChPortSerdesActiveLanesListGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_ACTIVE_LANES_STC *activeLanePtr
)
{
    GT_U32                          portMacNum;
    MV_HWS_PORT_INIT_PARAMS portParamPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(activeLanePtr);


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, NON_SUP_MODE, &portParamPtr))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    activeLanePtr->numActiveLanes = portParamPtr.numOfActLanes;

    cpssOsMemCpy(activeLanePtr->activeLaneList,portParamPtr.activeLanesList,  sizeof(activeLanePtr->activeLaneList));

    return GT_OK;
}


/**
* @internal cpssDxChPortSerdesActiveLanesListGet function
* @endinternal
*
* @brief   Return number of active serdeses and array of active
*          serdeses numbers that port uses
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] activeLanePtr          - number of active lanes
*                                        and list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssDxChPortSerdesActiveLanesListGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_ACTIVE_LANES_STC *activeLanePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesActiveLanesListGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, activeLanePtr));

    rc = internal_cpssDxChPortSerdesActiveLanesListGet(devNum, portNum, activeLanePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, activeLanePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortFastLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Fast link "DOWN" on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - GT_TRUE for Fast link DOWN, GT_FALSE for Fast Link UP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortFastLinkDownEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS                       rc;
    GT_U32                          portGroup;
    GT_U32                          localPort;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* NOT applicable device family. Applies only to BC3 & Aldrin2 */
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E |
                    CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* converting portNum to portMacNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_BOOL     currentState;

        rc = cpssDxChPortFastLinkDownEnableGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(enable == currentState)
        {
            return GT_OK;
        }
    }

    /* get interface type, speed, port group and local port number form port mac*/
    ifMode      = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    speed       = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
    portGroup   = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum);
    localPort   = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    /* Sanity check */
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {
     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /*Translate port interface mode and speed to port mode*/
    prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode, speed, &portMode);

    /*Fast link down (enable = 1) - meaning, MAC Rx receives LF signal.
      Fast link up (enable = 0) - clears LF, normal operation.
      Note: As the MAC RX receives these local faults and get into faulted state,
      MAC inturn will transmit REMOTE FAULT to the peer.*/
    rc = mvHwsPortSendLocalFaultSet(devNum, portGroup, localPort, portMode, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsPortSendLocalFaultSet : Failed");
    }

    /*stores/holds fastLink set/unset stauts for specific port*/
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].isFastLink = enable;

    return rc;
}

/**
* @internal cpssDxChPortFastLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Fast link "Down" on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - GT_TRUE for Fast link DOWN, GT_FALSE for Fast Link UP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFastLinkDownEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFastLinkDownEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortFastLinkDownEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortFastLinkDownEnableGet function
* @endinternal
*
* @brief   Get Enable/disable Fast link "Down" on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                - (pointer to) current Fast link up status:
*                                       GT_TRUE - Fast link DOWN, GT_FALSE - Fast link UP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortFastLinkDownEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                *statePtr
)
{
    GT_STATUS                       rc;
    GT_U32                          portGroup;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    /* NOT applicable device family. Applies only to BC3 & Aldrin2 */
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E |
                    CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* converting portNum to portMacNum */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* get interface type, speed, port group and local port number form port mac*/
    ifMode      = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    portGroup   = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum);
    speed       = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);

    /* Sanity check */
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {
     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /*Translate port interface mode and speed to port mode*/
    prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode, speed, &portMode);

    /* Get current port params*/
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*Read current fast link status */
    rc = mvHwsPortFastLinkDownGet(devNum, portGroup, portMacNum, curPortParams.portPcsType, statePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsPortFastLinkDownGet : Failed");
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortFastLinkDownEnableGet function
* @endinternal
*
* @brief   Get Enable/disable Fast link "Down" on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                - (pointer to) current Fast link up status:
*                                       GT_TRUE - Fast link DOWN, GT_FALSE - Fast link UP.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFastLinkDownEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFastLinkDownEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssDxChPortFastLinkDownEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortExtraOperationsSet function
* @endinternal
*
* @brief  Configures port extra operations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - speed
* @param[in] operations               - port extra operations
* @param[out] result                  - operations result

* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortExtraOperationsSet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                portGroup,
    IN  CPSS_PORT_INTERFACE_MODE_ENT          ifMode,
    IN  CPSS_PORT_SPEED_ENT                   speed,
    IN  GT_U32                                operationsBitmap,
    OUT GT_U32                               *result

)
{
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD portMode;
    GT_U32    portMacNum, data[2] = {0};

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(operationsBitmap & MV_HWS_PORT_SERDES_OPERATION_PRECODING_E)
    {
        rc = mvHwsPortOperation(devNum, portGroup, portMacNum, portMode,
                     MV_HWS_PORT_SERDES_OPERATION_PRECODING_E, data, result);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if(operationsBitmap & MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E)
    {
        rc = mvHwsPortOperation(devNum, portGroup, portMacNum, portMode,
                     MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E, data, result);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if(operationsBitmap & MV_HWS_PORT_SERDES_OPERATION_CALC_LEVEL3_E)
    {
        rc = mvHwsPortOperation(devNum, portGroup, portMacNum, portMode,
                     MV_HWS_PORT_SERDES_OPERATION_CALC_LEVEL3_E, data, result);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if(operationsBitmap & MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E)
    {
        rc = mvHwsPortOperation(devNum, portGroup, portMacNum, portMode,
                     MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E, data, result);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if(operationsBitmap & MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_E)
    {
        if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E )
        {
            if ( _10GBase_KR == portMode ||  _22GBase_SR == portMode || _20GBase_KR2 == portMode )
            {
                rc = mvHwsPortOperation(devNum, portGroup, portMacNum, portMode,
                             MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_E, data, result);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
        else
        {
            rc = GT_OK;
        }
    }

    return rc;
}


/**
* @internal prvCpssDxChPortSerdesLowPowerModeEnableWrapper
*           function
* @endinternal
*
* @brief   activate low power mode after port configured
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] laneNum               - lane number
* @param[in] enableLowPower        -  1 - enable, 0 - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortSerdesLowPowerModeEnableWrapper
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  laneNum,
    IN GT_BOOL                 enableLowPower
)
{
    GT_STATUS rc;

    rc = prvCpssDxChPortSerdesLowPowerModeEnable(devNum,portNum,laneNum,enableLowPower);
    return rc;
}

/**
* @internal prvCpssDxChPortPreemptionParamsSetWrapper function
* @endinternal
*
* @brief   Set enable/disable preemption at PDX glue for specific physical port.
*          Determine preemption counting mode
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] preemptionParamsPtr          - preemption attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChPortPreemptionParamsSetWrapper
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_PM_MAC_PREEMPTION_PARAMS_STC     *preemptionParamsPtr
)
{
    GT_STATUS                                rc = GT_OK;
    GT_BOOL                                  txqPreemption;
    GT_U32                                   portMacNum;
    PRV_CPSS_PORT_TYPE_ENT                   macType;
    GT_U32                                   regAddr;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC        regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_U32                                   value = 0;
    GT_U32                                   fieldData;

    CPSS_NULL_PTR_CHECK_MAC(preemptionParamsPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    switch (preemptionParamsPtr->type )
    {
    case CPSS_PM_MAC_PREEMPTION_DISABLED_E:
        txqPreemption = GT_FALSE;
        break;
    case CPSS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E:
    case CPSS_PM_MAC_PREEMPTION_ENABLED_SEPARATED_E:
        txqPreemption = GT_TRUE;
      break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    }

   if (TXQ_IS_PREEMPTIVE_DEVICE(devNum)&& (macType == PRV_CPSS_PORT_MTI_100_E))
   {
      rc = prvCpssDxChTxqSip6_10PreemptionEnableSet(CAST_SW_DEVNUM(devNum), portNum, txqPreemption);
      if ( GT_OK != rc )
      {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
      }

      regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brCntrl;
       if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

      /*read the current value*/
      rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                regAddr, 0,32, &value);
      if (rc != GT_OK)
      {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsRsFecCorrectedError failed=%d");
      }

       if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);


       if(preemptionParamsPtr->type==CPSS_PM_MAC_PREEMPTION_ENABLED_SEPARATED_E)
       {
           U32_SET_FIELD_MASKED_MAC(value,24,1,0);
       }
       else/*DEFAULT : for disabled or aggregated*/
       {
           U32_SET_FIELD_MASKED_MAC(value,24,1,1);
       }



       switch(preemptionParamsPtr->minFragSize)
       {
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E:
            fieldData = 0;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_128_BYTE_E:
            fieldData = 1;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_192_BYTE_E:
           fieldData = 2;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_256_BYTE_E:
           fieldData = 3;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_320_BYTE_E:
           fieldData = 4;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_384_BYTE_E:
           fieldData = 5;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_448_BYTE_E:
            fieldData = 6;
          break;
         case CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_512_BYTE_E:
            fieldData = 7;
          break;
         default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
           break;
       }

       U32_SET_FIELD_MASKED_MAC(value,4,3,fieldData);

       if(CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E==preemptionParamsPtr->preemptionMethod)
       {
            U32_SET_FIELD_MASKED_MAC(value,18,1,0);
       }
       else
       {
            U32_SET_FIELD_MASKED_MAC(value,18,1,1);
       }

       regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
       regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 32;
       regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
       regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;

       rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
       if(rc!=GT_OK)
       {
          return rc;
       }


   }
   else
   {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
   }

    return rc;
}

/**
* @internal prvCpssDxChPortPtpDelayParamsSetWrapper function
* @endinternal
*
* @brief   Set PTP delay parametrs for specific physical port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortPtpDelayParamsSetWrapper
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC  egrDelayVal;
    GT_32 ingressDelayCorr;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
    CPSS_DXCH_PORT_FEC_MODE_ENT fecMode;
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT ptpSpeed;
    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT ptpInterfaceMode;
    CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT ptpFecMode;
    GT_BOOL foundParams = GT_TRUE;
    GT_U32 offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
        return GT_OK;

    rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch (speed)
    {
    case CPSS_PORT_SPEED_100_E:
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100M_E;
        }
        else
        {
            foundParams = GT_FALSE;
        }
        break;
    case CPSS_PORT_SPEED_1000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_1G_E;
        break;
    case CPSS_PORT_SPEED_10000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_10G_E;
        break;
    case CPSS_PORT_SPEED_25000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_25G_E;
        break;
    case CPSS_PORT_SPEED_40000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_40G_E;
        break;
    case CPSS_PORT_SPEED_50000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_50G_E;
        break;
    case CPSS_PORT_SPEED_100G_E:
    case CPSS_PORT_SPEED_102G_E:
    case CPSS_PORT_SPEED_106G_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100G_E;
        break;
    case CPSS_PORT_SPEED_200G_E:
    case CPSS_PORT_SPEED_212G_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_200G_E;
        break;
    case CPSS_PORT_SPEED_400G_E:
    case CPSS_PORT_SPEED_424G_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_400G_E;
        break;
    default:
        /* set default '0' */
        foundParams = GT_FALSE;
    }

    if (foundParams)
    {

        rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&interfaceMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        switch (interfaceMode)
        {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            ptpInterfaceMode = CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_DEFAULT_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
            ptpInterfaceMode = CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_2LANE_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            ptpInterfaceMode = CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_4LANE_E;
            break;
        default:
            /* set default '0' */
            foundParams = GT_FALSE;
        }

        if (foundParams)
        {
            rc = cpssDxChPortFecModeGet(devNum,portNum,&fecMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            switch (fecMode)
            {
            case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_FC_FEC_MODE_ENABLED_E;
                break;
            case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:
            case CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_DISABLED_E;
                break;
            case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_528_514_MODE_ENABLED_E;
                break;
            case CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            default:
                /* set default '0' */
                foundParams = GT_FALSE;
            }
        }
    }

    if (foundParams)
    {
        offset = portNum*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 ptpSpeed*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 ptpInterfaceMode*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + ptpFecMode;

        egrDelayVal.egressPipeDelay = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelay;
        egrDelayVal.egressPipeDelayEncrypted = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayEncrypted;
        egrDelayVal.egressPipeDelayFsuOffset = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayFsuOffset;
        ingressDelayCorr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ingressPortDelay;

    }
    else
    {
        /* set default '0' */
        egrDelayVal.egressPipeDelay = 0;
        egrDelayVal.egressPipeDelayEncrypted = 0;
        egrDelayVal.egressPipeDelayFsuOffset = 0;
        ingressDelayCorr = 0;
    }

    /* configure Egress Pipe delays */
    rc = cpssDxChPtpTsDelayEgressPipeDelaySet(devNum,portNum,&egrDelayVal);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* configure Ingress Port delay */
    rc = cpssDxChPtpTsDelayIngressPortDelaySet(devNum,portNum,ingressDelayCorr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortAnpInfoGet function
* @endinternal
*
* @brief   Get anp info.
*
* @note   APPLICABLE DEVICES:  AC5P, AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P;
*         AC5X; Harrier; Ironman..
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[out] anpInfoPtr               - anp structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortAnpInfoGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_ANP_INFO_STC    *anpInfoPtr
)
{
    GT_U32    portMacNum;
    GT_STATUS rc;
    MV_HWS_ANP_INFO_STC  hwsAnpInfo;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(anpInfoPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    cpssOsMemSet(anpInfoPtr, 0, sizeof(CPSS_ANP_INFO_STC));

    rc = mvHwsAnpPortInfoGet(devNum, portMacNum, &hwsAnpInfo);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    anpInfoPtr->anRestartCounter      = hwsAnpInfo.portConters.anRestartCounter;
    anpInfoPtr->dspLockFailCounter    = hwsAnpInfo.portConters.dspLockFailCounter;
    anpInfoPtr->linkFailCounter       = hwsAnpInfo.portConters.linkFailCounter;
    anpInfoPtr->txTrainDuration       = hwsAnpInfo.portConters.txTrainDuration;
    anpInfoPtr->txTrainFailCounter    = hwsAnpInfo.portConters.txTrainFailCounter;
    anpInfoPtr->txTrainTimeoutCounter = hwsAnpInfo.portConters.txTrainTimeoutCounter;
    anpInfoPtr->txTrainOkCounter      = hwsAnpInfo.portConters.txTrainOkCounter;
    anpInfoPtr->rxInitOk              = hwsAnpInfo.portConters.rxInitOk;
    anpInfoPtr->rxInitTimeOut         = hwsAnpInfo.portConters.rxInitTimeOut;
    anpInfoPtr->rxTrainFailed         = hwsAnpInfo.portConters.rxTrainFailed;
    anpInfoPtr->rxTrainOk             = hwsAnpInfo.portConters.rxTrainOk;
    anpInfoPtr->rxTrainTimeOut        = hwsAnpInfo.portConters.rxTrainTimeOut;
    anpInfoPtr->chSmState             = hwsAnpInfo.portSm.chSmState;
    anpInfoPtr->txSmState             = hwsAnpInfo.portSm.txSmState;
    anpInfoPtr->rxSmState             = hwsAnpInfo.portSm.rxSmState;

    return GT_OK;
}

/**
* @internal cpssDxChPortAnpInfoGet function
* @endinternal
*
* @brief   Get anp info.
*
* @note   APPLICABLE DEVICES:  AC5P, AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P;
*         AC5X; Harrier; Ironman..
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[out] anpInfoPtr               - anp structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortAnpInfoGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_ANP_INFO_STC    *anpInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortAnpInfoGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, anpInfoPtr));

    rc = internal_cpssDxChPortAnpInfoGet(devNum, portNum, anpInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, anpInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


