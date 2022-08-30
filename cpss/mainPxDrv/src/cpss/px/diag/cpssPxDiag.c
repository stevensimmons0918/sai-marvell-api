/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*********************************************************************************/
/**
********************************************************************************
* @file cpssPxDiag.c
*
* @brief Implementation of CPSS PX API.
*
* PRBS sequence for tri-speed ports:
* 1. Enable PRBS checker on receiver port (cpssPxDiagPrbsPortCheckEnableSet)
* 2. Check that checker initialization is done (cpssPxDiagPrbsPortCheckReadyGet)
* 3. Set CPSS_PX_DIAG_TRANSMIT_MODE_PRBS_E transmit mode on transmiting port
* (cpssPxDiagPrbsPortTransmitModeSet)
* 4. Enable PRBS generator on transmiting port (cpssPxDiagPrbsPortGenerateEnableSet)
* 5. Check results on receiving port (cpssPxDiagPrbsPortStatusGet)
*
* PRBS sequence for XG ports:
* 1. Set CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E or CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E
* transmit mode on both ports(cpssPxDiagPrbsPortTransmitModeSet)
* 2. Enable PRBS checker on receiver port (cpssPxDiagPrbsPortCheckEnableSet)
* 3. Enable PRBS generator on transmiting port (cpssPxDiagPrbsPortGenerateEnableSet)
* 4. Check results on receiving port (cpssPxDiagPrbsPortStatusGet)
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/diag/private/prvCpssPxDiagLog.h>
#include <cpss/px/diag/cpssPxDiag.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC  (PRV_CPSS_PX_NUM_LANES_CNS-1)
/*******************************************************************************
* cgPortAddrValid
*
* Description:
*      Check CG MAC register validity.
*
* INPUTS:
*       devNum          - device number
*       regAddr         - Register address
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE    -  registers either not exist or exist and can be accessed
*       GT_FALSE   -  registers are exist but cannot be accessed
*
* Comment:
*       None.
*
 *******************************************************************************/
static GT_BOOL cgPortAddrValid
(
    IN GT_U8  devNum,
    IN GT_U32 regAddr
)
{
    GT_U32 portMacNum;
    GT_U32 port_first_cg_regAddr;
    GT_U32 port_last_cg_regAddr;
    CPSS_PORT_INTERFACE_MODE_ENT portInterfaceMode;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;

    unitId = prvCpssPxHwRegAddrToUnitIdConvert(devNum, regAddr);
    if(unitId != PRV_CPSS_DXCH_UNIT_GOP_E)
    {
        /* the address not belong to GOP memory space */
        return GT_TRUE;
    }

    /* get the register addr of CG MAC of port 0 */
    portMacNum = 0;
    port_first_cg_regAddr   = 0x1034C400;
    port_last_cg_regAddr    = 0x1034CBFF;

    if((regAddr < port_first_cg_regAddr) || (regAddr > port_last_cg_regAddr))
    {
        /* the address not belongs to CG MAC */
        return GT_TRUE;
    }


    /* the address is in CG MAC */
    portMacNum = (regAddr / 0x1000) & 0x3f;

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    portInterfaceMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum);
    if(portMacType       != PRV_CPSS_PORT_CG_E ||
       (portInterfaceMode != CPSS_PORT_INTERFACE_MODE_KR4_E &&
        portInterfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR4_E))
    {
        /* the CG MAC is NOT in proper speed or mode */
        /* access to it's registers may HANG the PEX */
        return GT_FALSE;
    }

    /* the CG MAC is in proper speed and mode */
    return GT_TRUE;
}

/*******************************************************************************
* prvCpssPxcheckIfRegisterExist
*
* Description:
*      Check register existance.
*
* INPUTS:
*       devNum          - device number
*       portGroupId     - the port group Id , to support multi-port-group
*                           devices that need to access specific port group
*       regAddr         - Register address
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE    -  registers either not exist or exist and can be accessed
*       GT_FALSE   -  registers are exist but cannot be accessed
*
* Comment:
*       None.
*
 *******************************************************************************/
GT_BOOL prvCpssPxcheckIfRegisterExist
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 regAddr
)
{
    portGroupId = portGroupId;

    if(GT_FALSE == cgPortAddrValid(devNum, regAddr))
    {
        /* filter out CG mac registers that not supported */
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal internal_cpssPxDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssPxDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssPxDiagRegsNumGet
(
    IN GT_SW_DEV_NUM devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_U32 *regsListPtr;    /* points to all elements in
                               PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  */
    GT_U32 regsListSize;    /* number of elements in
                               PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC */
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);

    regsListPtr = (GT_U32*)PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC) / sizeof(GT_U32);

    rc = prvCpssDiagRegsNumGet(CAST_SW_DEVNUM(devNum), 0,
                               regsListPtr, regsListSize,
                               0, &prvCpssPxcheckIfRegisterExist, regsNumPtr);
    return rc;
}

/**
* @internal cpssPxDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssPxDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssPxDiagRegsNumGet
(
    IN GT_SW_DEV_NUM devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagRegsNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr));

    rc = internal_cpssPxDiagRegsNumGet(devNum, regsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssPxDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssPxDiagResetAndInitControllerRegsNumGet
(
    IN GT_SW_DEV_NUM devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_U32 *regsListPtr;    /* points to all elements in */
                            /* PRV_CPSS_PX_RESET_AND_INIT_CTRL_REGS_ADDR_STC */
    GT_U32 regsListSize;    /* number of  elements in */
                            /* PRV_CPSS_PX_RESET_AND_INIT_CTRL_REGS_ADDR_STC */
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *regsNumPtr = 0;
        return GT_OK;
    }

    regsListPtr =
        (GT_U32*)PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC) /
                                                                sizeof(GT_U32);

    rc = prvCpssDiagRegsNumGet(CAST_SW_DEVNUM(devNum), 0,
                               regsListPtr, regsListSize, 0, NULL,regsNumPtr);
    return rc;
}

/**
* @internal cpssPxDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssPxDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssPxDiagResetAndInitControllerRegsNumGet
(
    IN GT_SW_DEV_NUM devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagResetAndInitControllerRegsNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr));

    rc = internal_cpssPxDiagResetAndInitControllerRegsNumGet(devNum, regsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
static GT_STATUS internal_cpssPxDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    return prvCpssDiagRegWrite(
        baseAddr, ifChannel, regType, offset, data,
        doByteSwap);
}

/**
* @internal cpssPxDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssPxDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagRegWrite);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, baseAddr, ifChannel, regType, offset, data, doByteSwap));

    rc = internal_cpssPxDiagRegWrite(baseAddr, ifChannel, regType, offset, data, doByteSwap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, baseAddr, ifChannel, regType, offset, data, doByteSwap));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
static GT_STATUS internal_cpssPxDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    return prvCpssDiagRegRead(
        baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap);
}

/**
* @internal cpssPxDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssPxDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagRegRead);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap));

    rc = internal_cpssPxDiagRegRead(baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] regsNumPtr               - (pointer to) number of registers to dump.
*                                      This number must not be bigger
*                                      than the number of registers that can be dumped
*                                      (starting at offset).
* @param[in] offset                   - the first register address to dump.
* @param[in,out] regsNumPtr               - (pointer to) number of registers that were dumped.
*
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers.
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssPxDiagRegsNumGet in
*       order to get the number of registers of the PP.
*
*/
static GT_STATUS internal_cpssPxDiagRegsDump
(
    IN GT_SW_DEV_NUM devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_U32 *regsListPtr;    /* points to all elements in
                               PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC */
    GT_U32 regsListSize;    /* number of elements in
                               PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC */
    GT_STATUS rc;           /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);

    regsListPtr = (GT_U32*)PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC) / sizeof(GT_U32);

    rc = prvCpssDiagRegsDataGet(CAST_SW_DEVNUM(devNum), 0,
                                regsListPtr, regsListSize,offset, prvCpssPxcheckIfRegisterExist,
                                regsNumPtr,regAddrPtr, regDataPtr);

    return rc;
}

/**
* @internal cpssPxDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] regsNumPtr               - (pointer to) number of registers to dump.
*                                      This number must not be bigger
*                                      than the number of registers that can be dumped
*                                      (starting at offset).
* @param[in] offset                   - the first register address to dump.
* @param[in,out] regsNumPtr               - (pointer to) number of registers that were dumped.
*
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers.
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssPxDiagRegsNumGet in
*       order to get the number of registers of the PP.
*
*/
GT_STATUS cpssPxDiagRegsDump
(
    IN GT_SW_DEV_NUM devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagRegsDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));

    rc = internal_cpssPxDiagRegsDump(devNum, regsNumPtr, offset, regAddrPtr, regDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] regsNumPtr               - (pointer to) number of registers to dump.
*                                      This number must not be bigger
*                                      than the number of registers that can be dumped
*                                      (starting at offset).
* @param[in] offset                   - the first register address to dump.
* @param[in,out] regsNumPtr               - (pointer to) number of registers that were dumped.
*
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers.
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssPxDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*
*/
static GT_STATUS internal_cpssPxDiagResetAndInitControllerRegsDump
(
    IN GT_SW_DEV_NUM devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_U32 *regsListPtr;    /* points to all elements in
                               PRV_CPSS_PX_RESET_AND_INIT_CTRL_REGS_ADDR_STC */
    GT_U32 regsListSize;    /* number of elements in
                               PRV_CPSS_PX_RESET_AND_INIT_CTRL_REGS_ADDR_STC */
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    regsListPtr =
        (GT_U32*)PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC) /
                                                                sizeof(GT_U32);

    rc = prvCpssDiagResetAndInitControllerRegsDataGet(CAST_SW_DEVNUM(devNum), regsListPtr,
                                                      regsListSize, offset,
                                                      NULL, regsNumPtr,
                                                      regAddrPtr, regDataPtr);
    return rc;
}

/**
* @internal cpssPxDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] regsNumPtr               - (pointer to) number of registers to dump.
*                                      This number must not be bigger
*                                      than the number of registers that can be dumped
*                                      (starting at offset).
* @param[in] offset                   - the first register address to dump.
* @param[in,out] regsNumPtr               - (pointer to) number of registers that were dumped.
*
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers.
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssPxDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*
*/
GT_STATUS cpssPxDiagResetAndInitControllerRegsDump
(
    IN GT_SW_DEV_NUM devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagResetAndInitControllerRegsDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));

    rc = internal_cpssPxDiagResetAndInitControllerRegsDump(devNum,
                                                           regsNumPtr, offset,
                                                           regAddrPtr, regDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits. Use 0xFFFFFFFF to test all bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - (pointer to) to test result. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - (pointer to) value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static GT_STATUS internal_cpssPxDiagRegTest
(
    IN  GT_SW_DEV_NUM                 devNum,
    IN  GT_U32                        regAddr,
    IN  GT_U32                        regMask,
    IN  CPSS_DIAG_TEST_PROFILE_ENT    profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(readValPtr);
    CPSS_NULL_PTR_CHECK_MAC(writeValPtr);

    return prvCpssDiagRegTest(
        CAST_SW_DEVNUM(devNum), regAddr, regMask, profile,
        testStatusPtr, readValPtr, writeValPtr);
}

/**
* @internal cpssPxDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits. Use 0xFFFFFFFF to test all bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - (pointer to) to test result. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - (pointer to) value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
GT_STATUS cpssPxDiagRegTest
(
    IN  GT_SW_DEV_NUM                 devNum,
    IN  GT_U32                        regAddr,
    IN  GT_U32                        regMask,
    IN  CPSS_DIAG_TEST_PROFILE_ENT    profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagRegTest);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, regAddr, regMask, profile, testStatusPtr, readValPtr, writeValPtr));

    rc = internal_cpssPxDiagRegTest(
        devNum, regAddr, regMask, profile, testStatusPtr, readValPtr, writeValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, regAddr, regMask, profile, testStatusPtr, readValPtr, writeValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] testStatusPtr            -is GT_FALSE. Irrelevant if testStatusPtr
* @param[out] badRegPtr                - (pointer to) address of the register which caused the failure if
* @param[out] testStatusPtr            is GT_FALSE. Irrelevant if
*                                      is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused the
*                                      failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE
* @param[out] writeValPtr              - (pointer to) value written to the register which caused the
*                                      failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssPxDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssPxDiagAllRegTest
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *testStatusPtr,
    OUT GT_U32          *badRegPtr,
    OUT GT_U32          *readValPtr,
    OUT GT_U32          *writeValPtr
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regsArray[20];        /* diagnostic registers : all 32 bits of the register must be 'RW' (read+write) */
    GT_U32 regIdx;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(badRegPtr);
    CPSS_NULL_PTR_CHECK_MAC(readValPtr);
    CPSS_NULL_PTR_CHECK_MAC(writeValPtr);

    /* below registers forom different units     */
    /* that all bits 31:0 of them are read write */
    /* index calculated to make convinient to    */
    /* add condition or comment out registers    */
    regIdx = 0;

    /* PRV_CPSS_DXCH_UNIT_MG_E */
    regsArray[regIdx++] = 0x00000070; /* user defined register0 */

    /* PRV_CPSS_DXCH_UNIT_RXDMA_E */
    regsArray[regIdx++] = regsAddrPtr->rxDMA.globalRxDMAConfigs.
        buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[0];

    /* PRV_CPSS_DXCH_UNIT_TXDMA_E */
    regsArray[regIdx++] = regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.burstLimiterSCDMA[0];

    /* PRV_CPSS_DXCH_UNIT_TX_FIFO_E */
    regsArray[regIdx++] = regsAddrPtr->txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0];

    /* PRV_CPSS_DXCH_UNIT_CNC_0_E */
    regsArray[regIdx++] = regsAddrPtr->CNC.globalRegs.CNCClearByReadValueRegWord1;

    /* PRV_CPSS_DXCH_UNIT_LED_0_E */
    regsArray[regIdx++] = regsAddrPtr->GOP.LED.classForcedData[0];

    /* PRV_CPSS_DXCH_UNIT_TAI_E - ommited: all registers have 16-bits only */

    /* PRV_CPSS_DXCH_UNIT_SMI_0_E - ommited: no 32 bit r/w registers */

    /* PRV_CPSS_PX_UNIT_PCP_E */
    regsArray[regIdx++] = regsAddrPtr->PCP.HASH.PCPHashCRC32Seed;

    /* PRV_CPSS_PX_UNIT_PHA_E */
    regsArray[regIdx++] = regsAddrPtr->PHA.pha_regs.egrTimestampConfig;

    /* PRV_CPSS_DXCH_UNIT_GOP_E - ommited: LED included, per port typically 16 bit */

    /* PRV_CPSS_DXCH_UNIT_MIB_E - ommited */

    /* PRV_CPSS_DXCH_UNIT_SERDES_E - ommited */

    /* PRV_CPSS_DXCH_UNIT_MPPM_E */
    regsArray[regIdx++] = regsAddrPtr->MPPM.pizzaArbiter.pizzaArbiterConfigReg[0];

    /* PRV_CPSS_DXCH_UNIT_BM_E */
    regsArray[regIdx++] = regsAddrPtr->BM.BMGlobalConfigs.BMBufferLimitConfig1;

    /* PRV_CPSS_DXCH_UNIT_BMA_E - ommited */

    /* PRV_CPSS_DXCH_UNIT_TXQ_PFC_E - CPFC unit of PX - ommited (no 32-bits r/w regs )*/

    /* PRV_CPSS_PX_UNIT_MCFC_E - ommited */

    /* PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E */
    regsArray[regIdx++] = regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mirroredPktsToAnalyzerPortDescsLimit;

    /* PRV_CPSS_DXCH_UNIT_TXQ_LL_E - ommited */

    /* PRV_CPSS_DXCH_UNIT_TXQ_QCN_E */
    regsArray[regIdx++] = regsAddrPtr->TXQ.qcn.CNSampleTbl[0];

    /* PRV_CPSS_DXCH_UNIT_TXQ_DQ_E */
    regsArray[regIdx++] = regsAddrPtr->TXQ.dq[0].global.globalDQConfig.BCForCutThrough;

    /* PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E */
    regsArray[regIdx++] = regsAddrPtr->TXQ.dq[1].global.globalDQConfig.BCForCutThrough;

    /* PRV_CPSS_DXCH_UNIT_TXQ_BMX_E - ommited */

    return prvCpssDiagAllRegTest(
        CAST_SW_DEVNUM(devNum), regsArray, NULL/*regMasksPtr*/, regIdx, testStatusPtr,
        badRegPtr, readValPtr, writeValPtr);
}

/**
* @internal cpssPxDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] testStatusPtr            -is GT_FALSE. Irrelevant if testStatusPtr
* @param[out] badRegPtr                - (pointer to) address of the register which caused the failure if
* @param[out] testStatusPtr            is GT_FALSE. Irrelevant if
*                                      is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused the
*                                      failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE
* @param[out] writeValPtr              - (pointer to) value written to the register which caused the
*                                      failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssPxDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssPxDiagAllRegTest
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *testStatusPtr,
    OUT GT_U32          *badRegPtr,
    OUT GT_U32          *readValPtr,
    OUT GT_U32          *writeValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagAllRegTest);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, testStatusPtr, badRegPtr, readValPtr, writeValPtr));

    rc = internal_cpssPxDiagAllRegTest(
        devNum, testStatusPtr, badRegPtr, readValPtr, writeValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, testStatusPtr, badRegPtr, readValPtr, writeValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sensorType               - Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    switch(sensorType)
    {
        case CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E:
            value = 0;
            break;
        case CPSS_PX_DIAG_TEMPERATURE_SENSOR_MAX_E:
            value = 6;
            break;
        case CPSS_PX_DIAG_TEMPERATURE_SENSOR_AVERAGE_E:
            value = 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set Temperature Sensor */
    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                  DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 28, 3, value);

    return rc;
}

/**
* @internal cpssPxDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sensorType               - Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDeviceTemperatureSensorsSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, sensorType));

    rc = internal_cpssPxDiagDeviceTemperatureSensorsSelectSet(
        devNum, sensorType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, sensorType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] sensorTypePtr            - Pointer to Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */
    GT_STATUS   rc;         /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(sensorTypePtr);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *sensorTypePtr = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;
        return GT_OK;
    }

    /* Get Temperature Sensor */
    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                  DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 28, 3, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
            *sensorTypePtr = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;
            break;
        case 6:
            *sensorTypePtr = CPSS_PX_DIAG_TEMPERATURE_SENSOR_MAX_E;
            break;
        case 7:
            *sensorTypePtr = CPSS_PX_DIAG_TEMPERATURE_SENSOR_AVERAGE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] sensorTypePtr            - Pointer to Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDeviceTemperatureSensorsSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, sensorTypePtr));

    rc = internal_cpssPxDiagDeviceTemperatureSensorsSelectGet(
        devNum, sensorTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, sensorTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief  Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] thresholdValue           - Threshold value in Celsius degrees.
*                                      APPLICABLE RANGES: -277..198
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxDiagDeviceTemperatureThresholdSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_32          thresholdValue
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    /* TSEN28 Formula from IP doc: y = 2.1445*x + 584.75,
       where x=T, y=Temperature Sensor Readout */
    value = (5847500 + (thresholdValue * 21445)) / 10000;

    if(value >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 16, 10, value);

    return rc;
}

/**
* @internal cpssPxDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] thresholdValue           - Threshold value in Celsius degrees.
*                                      APPLICABLE RANGES: -277..198
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureThresholdSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_32          thresholdValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDeviceTemperatureThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, thresholdValue));

    rc = internal_cpssPxDiagDeviceTemperatureThresholdSet(
        devNum, thresholdValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, thresholdValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] thresholdValuePtr        - pointer to Threshold value in Celsius degrees.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxDiagDeviceTemperatureThresholdGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_32          *thresholdValuePtr
)
{
    GT_STATUS   rc;         /* return status */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(thresholdValuePtr);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *thresholdValuePtr = 110;
        return GT_OK;
    }

    /* Get Threshold for Interrupt. */
    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 16, 10, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Calculate temperature by formulas below: */
    /* TSEN28 Formula from IP doc: y = 2.1445*x + 584.75,
       where x=T, y=Temperature Sensor Readout */
    /* T(in Celsius) = (hwValue - 584.75)/2.1445 */
    *thresholdValuePtr = (GT_32)((value * 10000) - 5847500) / 21445;

    return rc;
}

/**
* @internal cpssPxDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] thresholdValuePtr        - pointer to Threshold value in Celsius degrees.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureThresholdGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_32          *thresholdValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDeviceTemperatureThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, thresholdValuePtr));

    rc = internal_cpssPxDiagDeviceTemperatureThresholdGet(
        devNum, thresholdValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, thresholdValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxDiagDeviceTemperatureGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_32            *temperaturePtr
)
{
    GT_STATUS  rc;            /* return code       */
    GT_U32     regAddr;       /* register address  */
    GT_U32     hwValue;       /* HW Value          */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(temperaturePtr);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *temperaturePtr = 0;
        return GT_OK;
    }
    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnits.DFXServerRegs.temperatureSensorStatus;

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 0, 11, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Calculate temperature by formula below: */
    /* TSEN28 Formula from IP doc: y = 2.1445*x + 584.75,
       where x=T, y=Temperature Sensor Readout */
    /* Data ftom Cider: */
    /* T(in Celsius) = (hwValue - 584.45)/2.1445 */
    /* hwValue & 0x3FF - from bit 0 to bit 9 */
    *temperaturePtr = (GT_32)(((hwValue & 0x3FF) * 10000) - 5844500) / 21445;

    /*Temperature Sensor Readout is Valid*/
#ifndef ASIC_SIMULATION
    /* hwValue & 0x400 - bit 10 */
    if((hwValue & 0x400) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, LOG_ERROR_NO_MSG);
    }
#endif

    return GT_OK;
}

/**
* @internal cpssPxDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxDiagDeviceTemperatureGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_32            *temperaturePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDeviceTemperatureGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, temperaturePtr));

    rc = internal_cpssPxDiagDeviceTemperatureGet(
        devNum, temperaturePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, temperaturePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sensorNum                - sensor number (APPLICABLE RANGES: 0..3)
*
* @param[out] voltagePtr               - (pointer to) voltage in milivolts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
static GT_STATUS internal_cpssPxDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
)
{
    GT_STATUS  rc;
    GT_U32     regAddrTempSensor;       /* register addresses */
    GT_U32     regAddrTempCtrlSensorLSB;
    GT_U32     regAddrTempCtrlSensorMSB;
    GT_U32     hwValue;       /* HW Value */
    GT_U32     divider_enabled;
    GT_U32     divider_config;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(voltagePtr);
    if (sensorNum >= 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddrTempCtrlSensorLSB = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB;
    regAddrTempCtrlSensorMSB = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
    regAddrTempSensor = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnits.DFXServerRegs.temperatureSensorStatus;

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum), regAddrTempCtrlSensorLSB, 31, 1, &divider_enabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum), regAddrTempCtrlSensorMSB, 3, 1, &divider_config);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* choose sensor to read voltage */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddrTempCtrlSensorLSB, 3, 2, sensorNum); /* sensor ID  from 0 to 3 */
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set VSEN mode */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddrTempCtrlSensorLSB, 6, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* WAIT is necessary here to give time for correct value to settle in tempRegister,
       may depend on 'Tsen Readout Avg' */
    cpssOsTimerWkAfter(100);

    /* readout raw value */
    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum),  regAddrTempSensor, 0, 10, &hwValue);
    if (rc != GT_OK)
    {
        /* restore default value - Temperature sensor (TSEN) mode */
        prvCpssDrvHwPpResetAndInitControllerSetRegField(
            CAST_SW_DEVNUM(devNum), regAddrTempCtrlSensorLSB, 6, 1, 1);
        return rc;
    }

    if (divider_enabled)
    {
        if (divider_config)
        {
            /* DIV_EN = 1  DIV_CFG = 1 */
            /* hwValue = -981.63 * V + 1595.8 */
            *voltagePtr = ((159580 - (hwValue * 100)) * 1000) / 98163;
        }
        else
        {
            /* DIV_EN = 1  DIV_CFG = 0 */
            /* hwValue = -662.37 * V + 1605.7 */
            *voltagePtr = ((160570 - (hwValue * 100)) * 1000) / 66237;
        }
    }
    else
    {
        /* DIV_EN = 0 */
        /* hwValue = -1322.6 * V + 1598.5 */
        *voltagePtr = ((15985 - (hwValue * 10)) * 1000) / 13226;
    }

    /* reset  back to TSEN mode */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddrTempCtrlSensorLSB, 6, 1, 1);
    return rc;
}

/**
* @internal cpssPxDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sensorNum                - sensor number (APPLICABLE RANGES: 0..3)
*
* @param[out] voltagePtr               - (pointer to) voltage in milivolts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
GT_STATUS cpssPxDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDeviceVoltageGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((
        funcId, devNum, sensorNum, voltagePtr));

    rc = internal_cpssPxDiagDeviceVoltageGet(
        devNum, sensorNum, voltagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((
        funcId, rc, devNum, sensorNum, voltagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxDiagPrbsSerdesIndexGet function
* @endinternal
*
* @brief   Get SERDES index for lane of a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] portGroupIdPtr           - (pointer to) the port group Id
*                                      - support multi-port-groups device
* @param[out] serdesIndexPtr           - (pointer to) SERDES index
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxDiagPrbsSerdesIndexGet
(
    IN   GT_SW_DEV_NUM        devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    IN   GT_U32               laneNum,
    OUT  GT_U32               *portGroupIdPtr,
    OUT  GT_U32               *serdesIndexPtr
)
{
    GT_STATUS rc = GT_OK;    /* function return value */
    GT_U32 firstSerdes;      /* index of first SERDES for port */
    GT_U32 numOfSerdesLanes; /* number of SERDES lanes for port */
    GT_U32 portMacNum;      /* MAC number */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssPxPortNumberOfSerdesLanesGet(devNum, portNum,
                                               &firstSerdes, &numOfSerdesLanes);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PX_IS_FLEX_LINK_MAC(devNum,portMacNum) == GT_TRUE)
    {
        if (laneNum >= numOfSerdesLanes)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* stack/flex ports have up to 4 SERDESes per port */
        *serdesIndexPtr = firstSerdes + laneNum;
    }
    else
    {
        /* each network port SERDES is used for 4 ports */
        *serdesIndexPtr = firstSerdes;
    }

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*       CPU port doesn't support the transmit mode.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortTransmitModeSet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_PX_DIAG_TRANSMIT_MODE_ENT   mode
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr, macCtrl2RegAddr; /* register address of macCtrl2 */
    PRV_CPSS_PX_PORT_STATE_STC   portStateStc;  /* current port state */
    GT_U32      value;      /* register field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* current MAC unit used by port */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        switch(mode)
        {
            case CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E:
                value = 0;
                break;
            case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS_E:
                value = 1;
                break;
            case CPSS_PX_DIAG_TRANSMIT_MODE_ZEROS_E:
                value = 2;
                break;
            case CPSS_PX_DIAG_TRANSMIT_MODE_ONES_E:
                value = 3;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* disable port if we need */
        rc = prvCpssPxPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&macCtrl2RegAddr);
        /* set SelectDataToTransmit */
        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),portGroupId,macCtrl2RegAddr,12,2,value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        /* restore port enable state */
        rc = prvCpssPxPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else if(portMacType >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        switch(mode)
        {
            case CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E:
                value = 0;
                break;
            case CPSS_PX_DIAG_TRANSMIT_MODE_CYCLIC_E:
                value = 4;
                break;
            case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E:
                value = 5;
                break;
            case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E:
                value = 6;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneConfig0[laneNum];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 13, 3, value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssPxDiagPrbsPortTransmitModeSet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_PX_DIAG_TRANSMIT_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortTransmitModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, mode));

    rc = internal_cpssPxDiagPrbsPortTransmitModeSet(devNum, portNum, laneNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] modePtr                  - (pointer to) transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsPortTransmitModeGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_PX_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);
    laneNum = laneNum;
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);


    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 12, 2, &value)!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        switch (value)
        {
            case 0:
                *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E;
                break;
            case 1:
                *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_PRBS_E;
                break;
            case 2:
                *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_ZEROS_E;
                break;
            case 3:
                *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_ONES_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else if(portMacType >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneConfig0[laneNum];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 13, 3, &value)!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        if ((value & 0x4) == 0)
        {
            *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E;
        }
        else
        {
            switch (value & 0x3)
            {
                case 0:
                    *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_CYCLIC_E;
                    break;
                case 1:
                    *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E;
                    break;
                case 2:
                    *modePtr = CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] modePtr                  - (pointer to) transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsPortTransmitModeGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_PX_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortTransmitModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, modePtr));

    rc = internal_cpssPxDiagPrbsPortTransmitModeGet(devNum, portNum, laneNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all Px devices
*       When operating, enable the PRBS checker before the generator.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortGenerateEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr, macCtrl2RegAddr;
    PRV_CPSS_PX_PORT_STATE_STC      portStateStc;  /* current port state */
    GT_U32      value;      /* register field value */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    laneNum = laneNum;
    value = (enable == GT_TRUE) ? 1 : 0;

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* disable port if we need */
        rc = prvCpssPxPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;

        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&macCtrl2RegAddr);
        /* set PRBS Generate enable */
        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, macCtrl2RegAddr,11,1,value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        /* restore port enable state */
        rc = prvCpssPxPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }
    else if(portMacType >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneConfig0[laneNum];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),portGroupId, regAddr, 12, 1, value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all Px devices
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssPxDiagPrbsPortGenerateEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortGenerateEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssPxDiagPrbsPortGenerateEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all Px devices
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortGenerateEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    laneNum = laneNum;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 11;
    }
    else if(portMacType >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneConfig0[laneNum];
        offset = 12;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),portGroupId, regAddr, offset, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
GT_STATUS cpssPxDiagPrbsPortGenerateEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortGenerateEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssPxDiagPrbsPortGenerateEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssPxDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortCheckEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 10;
    }
    else if(portMacType >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneConfig0[laneNum];
        offset = 11;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, offset, 1, value);
}

/**
* @internal cpssPxDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssPxDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssPxDiagPrbsPortCheckEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortCheckEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssPxDiagPrbsPortCheckEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of PRBS (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortCheckEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);

        offset = 10;
    }
    else if(portMacType >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneConfig0[laneNum];
        offset = 11;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, offset, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of PRBS (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
GT_STATUS cpssPxDiagPrbsPortCheckEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortCheckEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssPxDiagPrbsPortCheckEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isReadyPtr               - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is ready.
*                                      PRBS checker has completed the initialization phase.
*                                      GT_FALSE - PRBS checker is not ready.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortCheckReadyGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL                *isReadyPtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isReadyPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        GOP.perPortRegs[portMacNum].prbsCheckStatus;

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 1, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *isReadyPtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isReadyPtr               - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is ready.
*                                      PRBS checker has completed the initialization phase.
*                                      GT_FALSE - PRBS checker is not ready.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
GT_STATUS cpssPxDiagPrbsPortCheckReadyGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL                *isReadyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortCheckReadyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isReadyPtr));

    rc = internal_cpssPxDiagPrbsPortCheckReadyGet(devNum, portNum, isReadyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isReadyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] checkerLockedPtr         - (pointer to) checker locked state.
*                                      GT_TRUE - checker is locked on the sequence stream.
*                                      GT_FALSE - checker isn't locked on the sequence
*                                      stream.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
*       Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsPortStatusGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *checkerLockedPtr,
    OUT  GT_U32                 *errorCntrPtr
)
{
    GT_U32      statusRegAddr;  /* register address for locked status*/
    GT_U32      counterRegAddr; /* register address for error counter*/
    GT_U32      value;          /* register field value */
    GT_U32      portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_U32      portMacNum;     /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(checkerLockedPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorCntrPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        statusRegAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].prbsCheckStatus;

        counterRegAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].prbsErrorCounter;

    }
    else if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        statusRegAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].laneStatus[laneNum];

        counterRegAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            GOP.perPortRegs[portMacNum].prbsErrorCounterLane[laneNum];
    }

    if((PRV_CPSS_SW_PTR_ENTRY_UNUSED == statusRegAddr) ||
            (PRV_CPSS_SW_PTR_ENTRY_UNUSED == counterRegAddr))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* get lock status */
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,statusRegAddr, 0, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *checkerLockedPtr = (value == 1) ? GT_TRUE : GT_FALSE;

    /* get error counter */
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,counterRegAddr, 0, 16, errorCntrPtr)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] checkerLockedPtr         - (pointer to) checker locked state.
*                                      GT_TRUE - checker is locked on the sequence stream.
*                                      GT_FALSE - checker isn't locked on the sequence
*                                      stream.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
*       Applicable only for Tri-speed and XAUI/HGS ports of all Px devices.
*
*/
GT_STATUS cpssPxDiagPrbsPortStatusGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *checkerLockedPtr,
    OUT  GT_U32                 *errorCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsPortStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, checkerLockedPtr, errorCntrPtr));

    rc = internal_cpssPxDiagPrbsPortStatusGet(devNum, portNum, laneNum, checkerLockedPtr, errorCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, checkerLockedPtr, errorCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssPxDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsCyclicDataSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_U32                 cyclicDataArr[4]
)
{
    GT_U32      regAddr;  /* register address */
    GT_U32      i;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cyclicDataArr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        GOP.perPortRegs[portMacNum].cyclicData[laneNum];
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i =0; i < 4;i++)
    {
        if (prvCpssDrvHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, cyclicDataArr[i]) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        regAddr += 4;
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssPxDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssPxDiagPrbsCyclicDataSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_U32                 cyclicDataArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsCyclicDataSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, cyclicDataArr));

    rc = internal_cpssPxDiagPrbsCyclicDataSet(devNum, portNum, laneNum, cyclicDataArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, cyclicDataArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssPxDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsCyclicDataGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_U32                 cyclicDataArr[4]
)
{
    GT_U32      regAddr;  /* register address */
    GT_U32      i;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cyclicDataArr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if((PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E) ||
       (PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_CG_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (laneNum > PRV_PX_DIAG_MAX_XPCS_LANES_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        GOP.perPortRegs[portMacNum].cyclicData[laneNum];
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i =0; i < 4;i++)
    {
        if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, &cyclicDataArr[i]) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        regAddr += 4;
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssPxDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssPxDiagPrbsCyclicDataGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_U32                 cyclicDataArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsCyclicDataGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, cyclicDataArr));

    rc = internal_cpssPxDiagPrbsCyclicDataGet(devNum, portNum, laneNum, cyclicDataArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, cyclicDataArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
* @param[in] enable                   - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Transmit mode should be set before enabling test mode.
*       See test cpssPxDiagPrbsSerdesTransmitModeSet.
*
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesTestEnableSet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    IN   GT_BOOL                enable
)
{
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
    MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    /* check input parameters and get SERDES index */
    rc = prvCpssPxDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

        rc = mvHwsSerdesTestGenGet(CAST_SW_DEVNUM(devNum), portGroupId, serdesIndex,
                                   HWS_DEV_SERDES_TYPE(devNum, serdesIndex), &txPattern,
                                   &hwsMode);
        if (rc != GT_OK)
        {
            return rc;
        }

    switch(PRV_CPSS_PX_PP_MAC(devNum)->port.prbsMode[portNum])
    {
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E:
            txPattern = PRBS7;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS9_E:
            txPattern = PRBS9;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS15_E:
            txPattern = PRBS15;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E:
            txPattern = PRBS23;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS31_E:
            txPattern = PRBS31;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_1T_E:
            txPattern = _1T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_2T_E:
            txPattern = _2T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_5T_E:
            txPattern = _5T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_10T_E:
            txPattern = _10T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_DFETraining:
            txPattern = DFETraining;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

        hwsMode = (GT_TRUE == enable) ? SERDES_TEST : SERDES_NORMAL;
        rc = mvHwsSerdesTestGen(CAST_SW_DEVNUM(devNum), portGroupId, serdesIndex, txPattern,
                                HWS_DEV_SERDES_TYPE(devNum, serdesIndex), hwsMode);
        if (rc != GT_OK)
        {
            return rc;
        }

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
* @param[in] enable                   - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Transmit mode should be set before enabling test mode.
*       See test cpssPxDiagPrbsSerdesTransmitModeSet.
*
*/
GT_STATUS cpssPxDiagPrbsSerdesTestEnableSet
(
    IN   GT_SW_DEV_NUM         devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32                laneNum,
    IN   GT_BOOL               enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesTestEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssPxDiagPrbsSerdesTestEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] enablePtr                - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesTestEnableGet
(
    IN   GT_SW_DEV_NUM         devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32                laneNum,
    OUT  GT_BOOL               *enablePtr
)
{
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
    MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* check input parameters and get SERDES index */
    rc = prvCpssPxDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = mvHwsSerdesTestGenGet(CAST_SW_DEVNUM(devNum), portGroupId, serdesIndex,
                               HWS_DEV_SERDES_TYPE(devNum, serdesIndex), &txPattern,
                               &hwsMode);
    if (rc != GT_OK)
    {
        return rc;
    }

        *enablePtr = (SERDES_TEST == hwsMode);

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] enablePtr                - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesTestEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesTestEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssPxDiagPrbsSerdesTestEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesTransmitModeSet
(
    IN   GT_SW_DEV_NUM                    devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_U32                           laneNum,
    IN   CPSS_PX_DIAG_TRANSMIT_MODE_ENT   mode
)
{
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
    MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    /* check input parameters and get SERDES index */
    rc = prvCpssPxDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = mvHwsSerdesTestGenGet(CAST_SW_DEVNUM(devNum), portGroupId, serdesIndex,
                               HWS_DEV_SERDES_TYPE(devNum, serdesIndex), &txPattern,
                               &hwsMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(mode)
    {
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E:
            txPattern = PRBS7;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS9_E:
            txPattern = PRBS9;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS15_E:
            txPattern = PRBS15;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E:
            txPattern = PRBS23;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_PRBS31_E:
            txPattern = PRBS31;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_1T_E:
            txPattern = _1T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_2T_E:
            txPattern = _2T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_5T_E:
            txPattern = _5T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_10T_E:
            txPattern = _10T;
            break;
        case CPSS_PX_DIAG_TRANSMIT_MODE_DFETraining:
            txPattern = DFETraining;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PP_MAC(devNum)->port.prbsMode[portNum] = mode;

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesTransmitModeSet
(
    IN   GT_SW_DEV_NUM                    devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_U32                           laneNum,
    IN   CPSS_PX_DIAG_TRANSMIT_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesTransmitModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, mode));

    rc = internal_cpssPxDiagPrbsSerdesTransmitModeSet(devNum, portNum, laneNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] modePtr                  - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesTransmitModeGet
(
    IN   GT_SW_DEV_NUM                     devNum,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_U32                            laneNum,
    OUT  CPSS_PX_DIAG_TRANSMIT_MODE_ENT    *modePtr
)
{
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* check input parameters and get SERDES index */
    rc = prvCpssPxDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = PRV_CPSS_PX_PP_MAC(devNum)->port.prbsMode[portNum];
    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] modePtr                  - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesTransmitModeGet
(
    IN   GT_SW_DEV_NUM                     devNum,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_U32                            laneNum,
    OUT  CPSS_PX_DIAG_TRANSMIT_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesTransmitModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, modePtr));

    rc = internal_cpssPxDiagPrbsSerdesTransmitModeGet(devNum, portNum, laneNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet function
* @endinternal
*
* @brief   Get Prbs Counter Clear on read enable or disable status per port and lane
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_U32    portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* check input parameters */
    if(laneNum >= PRV_CPSS_MAX_PORT_LANES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "laneNum[%d] >= max[%d]",
            laneNum,PRV_CPSS_MAX_PORT_LANES_CNS);
    }

    *enablePtr = PRV_CPSS_PX_PP_MAC(devNum)->port.prbsSerdesCountersClearOnReadEnable[portMacNum][laneNum];
    return GT_OK;
}

/**
* @internal internal_cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet function
* @endinternal
*
* @brief   Enable or disable Prbs Counter Clear on read status per port and lane
*         Can be run after port creation.
*         After port reconfiguration (cpssPxPortModeSpeedSet) Should be run once again.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
)
{
    GT_U32    portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    /* check input parameters */
    if(laneNum >= PRV_CPSS_MAX_PORT_LANES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "laneNum[%d] >= max[%d]",
            laneNum,PRV_CPSS_MAX_PORT_LANES_CNS);
    }

    PRV_CPSS_PX_PP_MAC(devNum)->port.prbsSerdesCountersClearOnReadEnable[portMacNum][laneNum] = enable;
    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet function
* @endinternal
*
* @brief   Enable or disable Prbs Counter Clear on read status per port and lane
*         Can be run after port creation.
*         After port reconfiguration (cpssPxPortModeSpeedSet) Should be run once again.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet function
* @endinternal
*
* @brief   Get Prbs Counter Clear on read enable or disable status per port and lane
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] lockedPtr                - (pointer to) Pattern detector state.
*                                      GT_TRUE - Pattern detector had locked onto the pattern.
*                                      GT_FALSE - Pattern detector is not locked onto
*                                      the pattern.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
* @param[out] patternCntrPtr           - (pointer to) Pattern counter. Number of 40-bit patterns
*                                      received since acquiring pattern lock.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPrbsSerdesStatusGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL               *lockedPtr,
    OUT  GT_U32                *errorCntrPtr,
    OUT  GT_U64                *patternCntrPtr
)
{
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    MV_HWS_SERDES_TEST_GEN_STATUS status;
    GT_BOOL clearOnReadEnable;
    GT_BOOL counterAccunulateMode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(lockedPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorCntrPtr);
    CPSS_NULL_PTR_CHECK_MAC(patternCntrPtr);

    /* check input parameters and get SERDES index */
    rc = prvCpssPxDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet(devNum, portNum, laneNum, &clearOnReadEnable);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(clearOnReadEnable == GT_TRUE)
    {
        counterAccunulateMode = GT_FALSE;
    }
    else
    {
        counterAccunulateMode = GT_TRUE;
    }

    /* clean buffer to avoid random trash in LOG */
    cpssOsMemSet(&status, 0, sizeof(status));

    rc = mvHwsSerdesTestGenStatus(CAST_SW_DEVNUM(devNum), portGroupId, serdesIndex,
                                  HWS_DEV_SERDES_TYPE(devNum, serdesIndex),
                                  0 /* txPattern not used */, counterAccunulateMode /* clear on read mode - GT_FALSE */, &status);
    if (rc != GT_OK)
    {
        return rc;
    }

    *lockedPtr = BIT2BOOL_MAC(status.lockStatus);
    *errorCntrPtr = status.errorsCntr;
    *patternCntrPtr = status.txFramesCntr;

    return GT_OK;
}

/**
* @internal cpssPxDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, (APPLICABLE RANGES: 0..3)
*
* @param[out] lockedPtr                - (pointer to) Pattern detector state.
*                                      GT_TRUE - Pattern detector had locked onto the pattern.
*                                      GT_FALSE - Pattern detector is not locked onto
*                                      the pattern.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
* @param[out] patternCntrPtr           - (pointer to) Pattern counter. Number of 40-bit patterns
*                                      received since acquiring pattern lock.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPrbsSerdesStatusGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    IN   GT_U32                 laneNum,
    OUT  GT_BOOL               *lockedPtr,
    OUT  GT_U32                *errorCntrPtr,
    OUT  GT_U64                *patternCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPrbsSerdesStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, lockedPtr, errorCntrPtr, patternCntrPtr));

    rc = internal_cpssPxDiagPrbsSerdesStatusGet(devNum, portNum, laneNum, lockedPtr, errorCntrPtr, patternCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, lockedPtr, errorCntrPtr, patternCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* list of CPU related RAM's DFX client information */
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC pipeCpuRamListArr[] = {{0,0}};

/**
* @internal prvCpssPxDiagBistCheckSkipOptionalClient function
* @endinternal
*
* @brief   Check skip of optional DFX client for BIST
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pipe                     - DFX pipe number
* @param[in] client                   - DFX client number
* @param[in] skipCpuMemory            - skip internal CPU related memories from output
*
* @retval GT_TRUE                  - skip BIST
* @retval GT_FALSE                 - does not skip BIST
*/
GT_BOOL prvCpssPxDiagBistCheckSkipOptionalClient
(
    IN  GT_U32              pipe,
    IN  GT_U32              client,
    IN  GT_BOOL             skipCpuMemory
)
{
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * clientsListPtr;     /* pointer to DFX clients list */
    GT_U32 listSize;    /* size of DFX clients list */
    GT_U32 ii;          /* iterator */

    if (skipCpuMemory)
    {
        /* get list of CPU clients including MSYS */
        clientsListPtr = pipeCpuRamListArr;
        listSize = sizeof(pipeCpuRamListArr) / sizeof(pipeCpuRamListArr[0]);
        if (clientsListPtr)
        {
            for (ii = 0; ii < listSize; ii++)
            {
                if ((pipe == clientsListPtr[ii].dfxPipeIndex) && (client == clientsListPtr[ii].dfxClientIndex))
                {
                    return GT_TRUE;
                }
            }
        }
    }

    return GT_FALSE;
}

/**
* @internal prvCpssPxDiagDataIntegrityDbPointerSet function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[out] dbArrayPtrPtr         - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr       - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
*/
GT_VOID prvCpssPxDiagDataIntegrityDbPointerSet
(
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    **dbArrayPtrPtr,
    OUT GT_U32                                          *dbArrayEntryNumPtr
)
{
    /* assign DB pointer and size */
    *dbArrayPtrPtr = pipeDataIntegrityDbArray;
    *dbArrayEntryNumPtr = pipeDataIntegrityDbArrayEntryNum;
}

/**
* @internal prvCpssPxDiagBistClientsListBuild function
* @endinternal
*
* @brief   Function creates list of applicable DFX clients for BIST operation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] skipCpuMemory            - skip internal CPU related memories from output
*
* @param[out] dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS] - array of DFX clients bitmaps, index is pipe
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail of algorithm
*/
static GT_STATUS prvCpssPxDiagBistClientsListBuild
(
    IN  GT_BOOL         skipCpuMemory,
    OUT GT_U32          dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS]
)
{
    GT_U32 pipe;    /* DFX pipe id */
    GT_U32 client;  /* DFX client id */
    GT_U32 ii;      /* loop iterator */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                         dbArrayEntryNum; /* size of data integrity DB */

    /* assign DB pointer and size */
    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    cpssOsMemSet(dfxClientsBmpArr, 0, PRV_CPSS_DFX_MAX_PIPES_CNS * sizeof(GT_U32));

    for( ii = 0; ii < dbArrayEntryNum ; ii++ )
    {
        pipe = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[ii].key);
        client = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[ii].key);

        if (pipe >= PRV_CPSS_DFX_MAX_PIPES_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (client >= PRV_CPSS_DFX_MAX_CLIENTS_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (dfxClientsBmpArr[pipe] & (1 << client))
        {
            /* client is already in list */
            continue;
        }

        if (prvCpssPxDiagBistCheckSkipOptionalClient(pipe, client,skipCpuMemory))
            continue;

        dfxClientsBmpArr[pipe] |= (1 << client);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxDiagBistMcDistributeSet function
* @endinternal
*
* @brief   Configure all relevant clients to be in the multicast distribution list.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
* @param[in] skipCpuMemory            - skip internal CPU related memories from MC distribution
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssPxDiagBistMcDistributeSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_BOOL             skipCpuMemory
)
{
    GT_STATUS rc;       /* return status          */
    /* array of DFX clients bitmaps, index is pipe */
    GT_U32   dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS];
    GT_U32   regAddr;  /* register's address     */
    GT_U32   pipe;     /* DFX pipe               */
    GT_U32   client;   /* DFX client             */

    /* configure pipe#0 and client#1 as reference one for MC access in prvCpssDfxClientSetRegField.
       Client#0 is not used by BIST. Don't use it as reference. */
    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetPipe = 0;
    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetClient = 1;

    rc = prvCpssPxDiagBistClientsListBuild(skipCpuMemory, dfxClientsBmpArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXClientUnits.clientControl;

    for (pipe = 0; pipe < PRV_CPSS_DFX_MAX_PIPES_CNS; pipe++)
    {
        if (dfxClientsBmpArr[pipe] == 0)
        {
            /* there are no clients on the pipe */
            continue;
        }

        for (client = 0; client < PRV_CPSS_DFX_MAX_CLIENTS_CNS; client++)
        {
            if (dfxClientsBmpArr[pipe] & (1 << client))
            {
                /* configure client */
                rc = prvCpssDfxClientSetRegField(CAST_SW_DEVNUM(devNum),
                                                 pipe, client, regAddr, 6, 1, 1);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
static GT_STATUS internal_cpssPxDiagBistTriggerAllSet
(
    IN  GT_SW_DEV_NUM         devNum
)
{
    GT_STATUS rc;             /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    /* Enable all relevant clients to be in the multicast group */
    rc = prvCpssPxDiagBistMcDistributeSet(devNum, GT_TRUE);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Start and Enable BIST in all clients those are
       members of multicast group */
    rc = prvCpssDfxDiagBistStartSet(CAST_SW_DEVNUM(devNum), GT_TRUE);
    if (GT_OK != rc)
    {
        return rc;
    }

    return rc;
}

/**
* @internal cpssPxDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS cpssPxDiagBistTriggerAllSet
(
    IN  GT_SW_DEV_NUM         devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagBistTriggerAllSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPxDiagBistTriggerAllSet(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxDiagBistStatusCheck function
* @endinternal
*
* @brief  Check BIST status of all relevant clients.
*         Configuration done according to memory map DB.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - PP device number
* @param[in] dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS] - array of DFX clients bitmaps, index is pipe
*
* @param[out] resultsStatusPtr     - (pointer to) BIST results status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssPxDiagBistStatusCheck
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          dfxClientsBmpArr[],
    OUT CPSS_PX_DIAG_BIST_STATUS_ENT   *resultsStatusPtr
)
{
    GT_STATUS                           rc;                         /* return code          */
    PRV_CPSS_DFX_DIAG_BIST_STATUS_ENT   prvResultsStatus;           /* generic result result */


    /* call generic BIST status check */
    rc = prvCpssDfxDiagBistStatusCheck(devNum, dfxClientsBmpArr, &prvResultsStatus);
    if (GT_OK != rc)
    {
        return rc;
    }

    switch (prvResultsStatus)
    {
        case PRV_CPSS_DFX_DIAG_BIST_STATUS_NOT_READY_E:
            *resultsStatusPtr = CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E;
            break;
        case PRV_CPSS_DFX_DIAG_BIST_STATUS_PASS_E:
            *resultsStatusPtr = CPSS_PX_DIAG_BIST_STATUS_PASS_E;
            break;
        case PRV_CPSS_DFX_DIAG_BIST_STATUS_FAIL_E:
            *resultsStatusPtr = CPSS_PX_DIAG_BIST_STATUS_FAIL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxDiagBistResultsGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
* @param[in,out] resultsNumPtr        - in: max num of results that can be
*                                           reported due to size limit of resultsArr[].
*                                       out: the number of failures detected.
*                                            Relevant only if BIST failed.
* @param[out] resultsStatusPtr         - (pointer to) the status of the BIST.
* @param[out] resultsArr[]             - (pointer to) the BIST failures.
*                                      Relevant only if BIST failed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on SW error, code that never be reached.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagBistResultsGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_DIAG_BIST_STATUS_ENT                *resultsStatusPtr,
    OUT CPSS_PX_DIAG_BIST_RESULT_STC                resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 regData; /* register data */
    GT_U32 pipe;    /* DFX pipe id */
    GT_U32 activePipe; /* active pipe for read access */
    GT_U32 client;  /* DFX client id */
    GT_U32 ram;     /* DFX RAM id */
    GT_U32 ii;      /* loop iterator */
    GT_U32 resultsCounter;  /* counter for number of reported failures */
    GT_STATUS rc;   /* return status */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32 dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(resultsStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(resultsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(resultsArr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *resultsStatusPtr = CPSS_PX_DIAG_BIST_STATUS_PASS_E;
        return GT_OK;
    }

    rc = prvCpssPxDiagBistClientsListBuild(GT_TRUE, dfxClientsBmpArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxDiagBistStatusCheck(devNum, dfxClientsBmpArr, resultsStatusPtr);
    if( GT_OK != rc )
    {
        return rc;
    }

    if (*resultsStatusPtr == CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E)
    {
        return GT_OK;
    }

    if( CPSS_PX_DIAG_BIST_STATUS_PASS_E == *resultsStatusPtr )
    {
        /* Stop and Disable MC Bist */
        return prvCpssDfxDiagBistStartSet(CAST_SW_DEVNUM(devNum), GT_FALSE);
    }

    resultsCounter = 0;

    /* Activate pipe 0 */
    rc = prvCpssDfxMemoryPipeIdSet(CAST_SW_DEVNUM(devNum), 0);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* assign DB pointer and size */
    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);
    if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    activePipe = 0;

    for( ii = 0 ; ii < dbArrayEntryNum ; ii++ )
    {
        pipe = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[ii].key);
        if( pipe != activePipe )
        {
            /* Activate pipe as "activePipe" */
            rc = prvCpssDfxMemoryPipeIdSet(CAST_SW_DEVNUM(devNum), pipe);
            if( GT_OK != rc )
            {
                return rc;
            }

            activePipe = pipe;
        }

        client = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[ii].key);

        if (prvCpssPxDiagBistCheckSkipOptionalClient(pipe, client, GT_TRUE))
        {
            continue;
        }

        ram = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[ii].key);

        rc = prvCpssDfxClientRegAddressGet(client,
               PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                      DFXClientUnits.clientRamBISTInfo[ram/32],
                                           &regAddr);
        if( GT_OK != rc )
        {
            return rc;
        }

        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                                    CAST_SW_DEVNUM(devNum), regAddr,
                                                    ram%32, 1, &regData);
        if( GT_OK != rc )
        {
            return rc;
        }

        if( 1 == regData )
        {
            if( resultsCounter < *resultsNumPtr )
            {
                resultsArr[resultsCounter].location.dfxPipeId = pipe;
                resultsArr[resultsCounter].location.dfxClientId = client;
                resultsArr[resultsCounter].location.dfxMemoryId = ram;
                resultsArr[resultsCounter].memType = dbArrayPtr[ii].memType;
            }
            resultsCounter++;
        }
    }

    *resultsNumPtr = resultsCounter;

    /* Stop MC Bist */
    rc = prvCpssDfxDiagBistStartSet(CAST_SW_DEVNUM(devNum), GT_FALSE);

    return rc;
}

/**
* @internal cpssPxDiagBistResultsGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number
* @param[in,out] resultsNumPtr        - in: max num of results that can be
*                                           reported due to size limit of resultsArr[].
*                                       out: the number of failures detected.
*                                            Relevant only if BIST failed.
* @param[out] resultsStatusPtr         - (pointer to) the status of the BIST.
* @param[out] resultsArr[]             - (pointer to) the BIST failures.
*                                      Relevant only if BIST failed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on SW error, code that never be reached.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagBistResultsGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_DIAG_BIST_STATUS_ENT                *resultsStatusPtr,
    OUT CPSS_PX_DIAG_BIST_RESULT_STC                resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagBistResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, resultsStatusPtr, resultsArr, resultsNumPtr));

    rc = internal_cpssPxDiagBistResultsGet(devNum, resultsStatusPtr, resultsArr, resultsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, resultsStatusPtr, resultsArr, resultsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

