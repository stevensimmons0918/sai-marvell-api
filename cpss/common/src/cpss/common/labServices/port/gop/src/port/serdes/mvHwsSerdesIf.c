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
* mvHwsSerdesIf.c
*
* DESCRIPTION:
*       Serdes specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 58 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#ifndef CO_CPU_RUN
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyHDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2Db.h>
#endif
#ifdef HWS_DEBUG
extern GT_FILEP fdHws;
#endif
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>

#endif

/************************* Globals *******************************************************/

#ifdef MV_HWS_REDUCED_BUILD
GT_BOOL serdesExternalFirmware = GT_FALSE;
static MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr[HWS_MAX_DEVICE_NUM][SERDES_LAST] = {{NULL}};
#endif

#ifdef MV_HWS_REDUCED_BUILD
#define PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(_var) _var

#define PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(_var)  _var

#else
#define PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.serdesDir.mvHwsSerdesIfSrc._var)
/*For debug :
cpssNonSharedGlobalVarsPtr->commonMod.serdesDir.mvHwsSerdesIfSrc
*/
#define PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.serdesDir.mvHwsSerdesIfSrc._var)

#endif

#if 0 /* Temp - open this when all Comphy function are ready */
#define MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(_devNum, _serdesType, _funcPtrName) \
        if((NULL == PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[_devNum][_serdesType]) || (NULL == PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[_devNum][_serdesType]->_funcPtrName)) return GT_NOT_IMPLEMENTED;
#else
#define MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(_devNum, _serdesType, _funcPtrName)\
        if((NULL == PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[_devNum][_serdesType]) || (NULL == PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[_devNum][_serdesType]->_funcPtrName)) return (_serdesType > D2D_PHY_GW16) ?  GT_OK : GT_NOT_IMPLEMENTED;
#endif

/**
* @internal mvHwsSerdesPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] powerUp                  - true for power UP
*                                      baudRate  -
*                                      refClock  - ref clock value
*                                      refClockSource - ref cloack source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
* @param[in] serdesConfigPtr            Serdes Configuration
*                                       structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_STATUS rc = GT_OK;

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesConfigPtr->serdesType, serdesPowerCntrlFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Power Control: serdes %s.\n",
        PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesConfigPtr->serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif

#ifndef MV_HWS_REDUCED_BUILD
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {
        SERDES_LOCK(devNum, serdesConfigPtr->serdesType);
        rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesConfigPtr->serdesType]->serdesPowerCntrlFunc(devNum,portGroup,serdesNum,powerUp,serdesConfigPtr);
        SERDES_UNLOCK(devNum, serdesConfigPtr->serdesType);
    }
    return rc;
}

/**
* @internal mvHwsSerdesApPowerCtrl function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] powerUp                  - true for power UP
*                                      baudRate  -
*                                      refClock  - ref clock value
*                                      refClockSource - ref cloack source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
* @param[in] serdesConfigPtr            Serdes Configuration
*                                       structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesApPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesConfigPtr->serdesType, serdesApPowerCntrlFunc);
    SERDES_LOCK(devNum, serdesConfigPtr->serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesConfigPtr->serdesType]->serdesApPowerCntrlFunc(devNum,portGroup,serdesNum,powerUp,serdesConfigPtr);
    SERDES_UNLOCK(devNum, serdesConfigPtr->serdesType);

    return rc;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsSerdesManualRxConfig function
* @endinternal
*
* @brief   Per SERDES configure the RX parameters
* @param[in] devNum            - system device number
* @param[in] portGroup         - port group (core) number
* @param[in] serdesNum         - physical lane number
* @param[in] serdesType        - serdes type
* @param[in] rxConfigPtr       - pointer to unoin of params
*
* @retval 0                    - on success
* @retval 1                    - on error
*/
GT_STATUS mvHwsSerdesManualRxConfig
(
    IN GT_U8                             devNum,
    IN GT_U32                            portGroup,
    IN GT_U32                            serdesNum,
    IN MV_HWS_SERDES_TYPE                serdesType,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *rxConfigPtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesManualRxCfgFunc);

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
    {
        SERDES_LOCK(devNum, serdesType);
        rc = mvHwsSerdesManualRxConfigIpc(devNum,portGroup,serdesNum,serdesType,rxConfigPtr);
        SERDES_UNLOCK(devNum, serdesType);
        return rc;
    }
#endif

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Manual Rx Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc =  PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesManualRxCfgFunc(devNum,portGroup,serdesNum,rxConfigPtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                     - system device number
* @param[in] portGroup                  - port group (core) number
* @param[in] serdesNum                  - physical serdes number
* @param[in] serdesType                 - serdes type
* @param[in] manTxTuneStcPtr            - pointer to Tx params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TYPE              serdesType,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesManualTxCfgFunc);

    if (manTxTuneStcPtr == NULL)
    {
        return GT_BAD_PTR;
    }
#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
    {
        SERDES_LOCK(devNum, serdesType);
        rc =  mvHwsSerdesManualTxConfigIpc(devNum, portGroup, serdesNum, serdesType, manTxTuneStcPtr);
        SERDES_UNLOCK(devNum, serdesType);
        return rc;
    }
#endif

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Manual Tx Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc =  PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesManualTxCfgFunc(devNum, portGroup, serdesNum, manTxTuneStcPtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
*
* @param[out] MV_HWS_MAN_TUNE_TX_CONFIG_DATA - pointer to Tx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfigGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TYPE              serdesType,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesManualTxCfgGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Manual Tx Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc =  PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesManualTxCfgGetFunc(devNum, portGroup, serdesNum, manTxTuneStcPtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesLoopback function
* @endinternal
*
* @brief   Perform an internal loopback (SERDES loopback) for debug/BIST use.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
* @param[in] lbMode                   - loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLoopback
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_SERDES_LB  lbMode
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesLoopbackCfgFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Loopback Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesLoopbackCfgFunc(devNum,portGroup,serdesNum,lbMode);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}


/**
* @internal mvHwsSerdesLoopbackGet function
* @endinternal
*
* @brief   Get status of internal loopback (SERDES loopback).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
*
* @param[out] lbModePtr                - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLoopbackGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_SERDES_LB    *lbModePtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesLoopbackGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Loopback Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesLoopbackGetFunc(devNum,portGroup,serdesNum,lbModePtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] serdesType               - type of SerDes
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTestGen
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTestGenFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Test Generator: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTestGenFunc(devNum,portGroup,serdesNum,txPattern,mode);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
* @param[in] txPattern                - pattern to transmit
*
* @param[out] modePtr               - Results
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on wrong serdes type
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsSerdesTestGenGet
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    OUT MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTestGenGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Get Test Generator: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTestGenGetFunc(devNum, portGroup, serdesNum,
                                            txPatternPtr, modePtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Accumulate result (T/F)
*
* @param[out] status                - Test status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTestGenStatusFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Test Generator Status: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTestGenStatusFunc(devNum, portGroup, serdesNum, txPattern, counterAccumulateMode, status);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif

/**
* @internal mvHwsSerdesEOMGet function
* @endinternal
*
* @brief   Returns the horizontal/vertical Rx eye margin.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
* @param[in] timeout                  - wait time in msec
*
* @param[out] results                - Results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesEOMGet
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TYPE        serdesType,
    IN GT_U32                    timeout,
    OUT MV_HWS_SERDES_EOM_RES     *results
)
{
    GT_STATUS rc = GT_OK;

    if ((NULL == results) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesEomGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes EOM: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesEomGetFunc(devNum, portGroup, serdesNum, timeout, results);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesPpmConfig function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] ppmVal                   - ppm value to config
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesPpmConfig
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_PPM_VALUE  ppmVal
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPpmCfgFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PPM Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPpmCfgFunc(devNum,portGroup,serdesNum,ppmVal);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}


/**
* @internal mvHwsSerdesPpmConfigGet function
* @endinternal
*
* @brief   Per SERDES read PPM value.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] ppmVal                   - PPM current value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPpmConfigGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_PPM_VALUE    *ppmVal
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPpmGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPpmGetFunc(devNum,portGroup,serdesNum,ppmVal);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */
#endif /* CO_CPU_RUN */

/**
* @internal mvHwsSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesAutoTuneCfg
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL rxTraining,
    IN GT_BOOL txTraining
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAutoTuneCfgFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAutoTuneCfgFunc(devNum,portGroup,serdesNum,rxTraining,txTraining);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRxAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL rxTraining
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesRxAutoTuneStartFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run RX Auto Tune Start: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesRxAutoTuneStartFunc(devNum,portGroup,serdesNum,rxTraining);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL txTraining
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTxAutoTuneStartFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run TX Auto Tune Start: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTxAutoTuneStartFunc(devNum,portGroup,serdesNum,txTraining);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif
/**
* @internal mvHwsSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStart
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL rxTraining,
    IN GT_BOOL txTraining
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAutoTuneStartFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Start: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAutoTuneStartFunc(devNum,portGroup,serdesNum,rxTraining,txTraining);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}


/**
* @internal mvHwsSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[out] results                  - the adapted tuning results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneResult
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              portGroup,
    IN  GT_U32                              serdesNum,
    IN  MV_HWS_SERDES_TYPE                  serdesType,
    OUT MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *results
)
{
    GT_STATUS rc = GT_OK;

    if ((results == NULL) || (serdesType >= SERDES_LAST) )
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAutoTuneResultFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Result: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAutoTuneResultFunc(devNum,portGroup,serdesNum,results);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] rxStatus                 - Rx Training status.
* @param[out] txStatus                 - Tx Training status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStatus
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAutoTuneStatusFunc);


#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Status: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAutoTuneStatusFunc(devNum,portGroup,serdesNum,rxStatus,txStatus);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesReset function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesReset
(
    IN GT_U8      devNum,
    IN GT_U32     portGroup,
    IN GT_U32     serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_BOOL    analogReset,
    IN GT_BOOL    digitalReset,
    IN GT_BOOL    syncEReset
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_SERDES_IDX_MAC(serdesNum));
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesResetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Reset: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesResetFunc(devNum,portGroup,serdesNum,analogReset,digitalReset,syncEReset);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesResetExt function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesNum                - physical serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
* @param[in] coreReset                - Core Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesResetExt
(
    IN GT_U8      devNum,
    IN GT_U32     portGroup,
    IN GT_U32     serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_BOOL    analogReset,
    IN GT_BOOL    digitalReset,
    IN GT_BOOL    syncEReset,
    IN GT_BOOL    coreReset
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesResetExtFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Reset Extended: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesResetExtFunc(devNum,portGroup,serdesNum,analogReset,digitalReset,syncEReset, coreReset);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeConfig
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN MV_HWS_DFE_MODE   dfeMode,
    IN GT_32             *dfeCfg
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDfeCfgFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDfeCfgFunc(devNum,portGroup,serdesNum,dfeMode,(GT_REG_DATA *)dfeCfg);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}


/**
* @internal mvHwsSerdesDfeStatus function
* @endinternal
*
* @brief   Per SERDES get the DFE status and parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] dfeLock                 - Locked/Not locked
* @param[out] dfeCfg                  - array of dfe
*                                       configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesDfeStatus
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL           *dfeLock,
    OUT GT_32             *dfeCfg
)
{
    GT_STATUS rc = GT_OK;

    if ((dfeCfg == NULL) || (dfeLock == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDfeStatusFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Status: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDfeStatusFunc(devNum,portGroup,serdesNum,dfeLock,dfeCfg);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesDfeStatusExt function
* @endinternal
*
* @brief   Per SERDES get the of DFE status and parameters (extended API).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] dfeLock                  - Locked/Not locked
* @param[out] dfeCfg                   - array of dfe configuration parameters
* @param[out] f0Cfg                    - array of f0 configuration parameters
* @param[out] savCfg                   - array of sav configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesDfeStatusExt
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL           *dfeLock,
    OUT GT_32             *dfeCfg,
    OUT GT_32             *f0Cfg,
    OUT GT_32             *savCfg
)
{
    GT_STATUS rc = GT_OK;

    if ((dfeCfg == NULL) || (dfeLock == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDfeStatusExtFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Status: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDfeStatusExtFunc(devNum,portGroup,serdesNum,dfeLock,
                                                                (GT_REG_DATA *)dfeCfg, (GT_REG_DATA *)f0Cfg, (GT_REG_DATA *)savCfg);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesPolarityConfig function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[in] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPolarityConfig
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    IN GT_BOOL           invertTx,
    IN GT_BOOL           invertRx
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPolarityCfgFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Polarity Config: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPolarityCfgFunc(devNum,portGroup, serdesNum,
                                                    invertTx, invertRx);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsSerdesPolarityConfigGet function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[out] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPolarityConfigGet
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL           *invertTx,
    OUT GT_BOOL           *invertRx
)
{
    GT_STATUS rc = GT_OK;

    if ((invertTx == NULL) || (invertRx == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPolarityCfgGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Polarity Get: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPolarityCfgGetFunc(devNum, portGroup, serdesNum,
                                            invertTx, invertRx);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif

/**
* @internal mvHwsSerdesDfeOpti function
* @endinternal
*
* @brief   Per SERDES find best FFE_R is the min value for scanned DFE range.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] ffeR                     - best value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesDfeOpti
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_U32  *ffeR
)
{
    GT_STATUS rc = GT_OK;

    if ((ffeR == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDfeOptiFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Optimal: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDfeOptiFunc(devNum, portGroup, serdesNum, (GT_UREG_DATA *)ffeR);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesFfeConfig function
* @endinternal
*
* @brief   Per SERDES configure FFE.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] ffeRes                   - FFE R value
* @param[in] ffeCap                   - FFE C value
* @param[in] alig                     - sampler value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesFfeConfig
(
  IN GT_U8   devNum,
  IN GT_U32  portGroup,
  IN GT_U32  serdesNum,
  IN MV_HWS_SERDES_TYPE serdesType,
  IN GT_U32  ffeRes,
  IN GT_U32  ffeCap,
  IN GT_U32  alig
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesFfeCfgFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Optimal: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesFfeCfgFunc(devNum, portGroup, serdesNum, ffeRes, ffeCap, alig);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesCalibrationStatus function
* @endinternal
*
* @brief   Per SERDES return the calibration results.
*         Can be run after create port
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] results                  - the calibration results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesCalibrationStatus
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN MV_HWS_SERDES_TYPE          serdesType,
    OUT MV_HWS_CALIBRATION_RESULTS  *results
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesCalibrationStatusFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Calibration Status: serdes %s.\n",
        PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesCalibrationStatusFunc(devNum, portGroup, serdesNum, results);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesTxEnable function
* @endinternal
*
* @brief   Enable/Disable Tx.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] enable                   - whether to  or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxEnable
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL             enable
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTxEnableFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Tx Disable: serdes %s.\n",
        PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTxEnableFunc(devNum, portGroup, serdesNum, enable);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

#ifndef CO_CPU_RUN
#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesTxEnableGet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] enablePtr                - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, serdesNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS mvHwsSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE serdesType,
    OUT GT_BOOL *enable
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTxEnableGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTxEnableGetFunc(devNum, portGroup, serdesNum, enable);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif

/**
* @internal hwsSerdesSeqGet function
* @endinternal
*
* @brief   Get SERDES sequence one line.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] seqType                  - sequence type
* @param[in] lineNum                  - line number
*
* @param[out] seqLinePtr               - sequence line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesSeqGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN MV_HWS_COM_PHY_H_SUB_SEQ seqType,
    IN GT_U32 lineNum,
    OUT MV_OP_PARAMS *seqLine
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_SERDES_TYPE serdesType;
    devNum = devNum;
    portGroup = portGroup;

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, lineNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesSeqGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesSeqGetFunc(seqType, seqLine, lineNum);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#ifndef RAVEN_DEV_SUPPORT

/**
* @internal hwsSerdesSeqSet function
* @endinternal
*
* @brief   Set SERDES sequence one line.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] firstLine                - the first line
* @param[in] seqType                  - sequence type
* @param[in] seqLinePtr               - sequence line
* @param[in] numOfOp                  - number of op
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesSeqSet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_BOOL firstLine,
    IN MV_HWS_COM_PHY_H_SUB_SEQ seqType,
    IN MV_OP_PARAMS *seqLine,
    IN GT_U32 numOfOp
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_SERDES_TYPE serdesType;
    devNum = devNum;
    portGroup = portGroup;

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 0));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesSeqSetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesSeqSetFunc(firstLine, seqType, seqLine, numOfOp);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif
#endif

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxIfSelect
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_U32              serdesTxIfNum
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_SERDES_IDX_MAC(serdesNum));
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTxIfSelectFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTxIfSelectFunc(devNum, portGroup, serdesNum, serdesTxIfNum);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - SERDES types
*
* @param[out] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxIfSelectGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT GT_U32              *serdesTxIfNum
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTxIfSelectGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTxIfSelectGetFunc(devNum, portGroup, serdesNum, serdesTxIfNum);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif
/**
* @internal mvHwsSerdesSqlchCfg function
* @endinternal
*
* @brief   Configure squelch threshold value.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] sqlchVal                 - squelch threshold value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesSqlchCfg
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_U32              sqlchVal
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesSqlchCfgFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesSqlchCfgFunc(devNum, portGroup, serdesNum, sqlchVal);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check Rx or Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] rxStatus               - rxStatus
* @param[out] txStatus               - txStatus
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStatusShort
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAutoTuneStatusShortFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAutoTuneStatusShortFunc(devNum, portGroup, serdesNum, rxStatus, txStatus);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training & Rx Training process
*         Can be run after create port and training start.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxAutoTuneStop
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAutoTuneStopFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAutoTuneStopFunc(devNum, portGroup, serdesNum);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}
#endif
/**
* @internal mvHwsSerdesFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current SERDES.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] params                   - SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesFixAlign90Start
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_ALIGN90_PARAMS   *params
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesFixAlign90Start);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesFixAlign90Start(devNum, portGroup, serdesNum, params);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesFixAlign90Status function
* @endinternal
*
* @brief   Return fix Align90 process current status on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] trainingStatus          - Training Status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Status
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT MV_HWS_AUTO_TUNE_STATUS *trainingStatus
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesFixAlign90Status);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesFixAlign90Status(devNum, portGroup, serdesNum, trainingStatus);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] params                   - SERDES parameters that must be restored (return by mvHwsComHRev2SerdesFixAlign90Start)
* @param[in] fixAlignPass             - true, if fix Align90 process passed; false otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Stop
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_ALIGN90_PARAMS   *params,
    IN GT_BOOL                 fixAlignPass
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesFixAlign90Stop);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesFixAlign90Stop(devNum, portGroup, serdesNum, params, fixAlignPass);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesRev2DfeCheck function
* @endinternal
*
* @brief   Check DFE values range and start sampler calibration, if needed.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRev2DfeCheck
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDfeCheckFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDfeCheckFunc(devNum, portGroup, serdesNum);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] rate                    - rate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSpeedGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_SERDES_SPEED *rate
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesSpeedGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesSpeedGetFunc(devNum, portGroup, serdesNum, rate);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Return SERDES RX parameters configured manually
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] portTuningModePtr        - Tuning mode params
* @param[out] sqlchPtr                 - Squelch threshold
* @param[out] ffeResPtr                - FFE R value
* @param[out] ffeCapPtr                - FFE C value
* @param[out] dfeEnPtr                 - DFE Enable/Disable
* @param[out] aligPtr                  - align 90 value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualRxConfigGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParams
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesManualRxCfgGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesManualRxCfgGetFunc(devNum, portGroup, serdesNum, configParams);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsSerdesArrayHaltDfeTune function
* @endinternal
*
* @brief   Safely halt DFE tune.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSer                 - number of SERDESes to
* @param[in] serdesConfigPtr          - serdes config structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesArrayHaltDfeTune
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      *serdesArr,
    IN GT_U32                      numOfSer,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_STATUS rc = GT_OK;

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesConfigPtr->serdesType, serdesArrayHaltDfeTuneFunc);

#ifndef MV_HWS_REDUCED_BUILD
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {
        SERDES_LOCK(devNum, serdesConfigPtr->serdesType);
        rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesConfigPtr->serdesType]->serdesArrayHaltDfeTuneFunc(devNum,portGroup,(GT_UOPT *)serdesArr, numOfSer);
        SERDES_UNLOCK(devNum, serdesConfigPtr->serdesType);
    }
    return rc;
}
#endif
/**
* @internal mvHwsSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] numOfSer                 - number of SERDESes to configure
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
* @param[in] serdesConfigPtr          - serdesConfigPtr
*                                      baudRate  - Serdes speed
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesArrayPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      numOfSer,
    IN GT_U32                      *serdesArr,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_STATUS rc = GT_OK;

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesConfigPtr->serdesType, serdesArrayPowerCntrlFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Power Control: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesConfigPtr->serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
#ifndef MV_HWS_REDUCED_BUILD
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {
        SERDES_LOCK(devNum, serdesConfigPtr->serdesType);
        rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesConfigPtr->serdesType]->serdesArrayPowerCntrlFunc(devNum,portGroup,numOfSer,(GT_UOPT *)serdesArr,powerUp,serdesConfigPtr);
        SERDES_UNLOCK(devNum, serdesConfigPtr->serdesType);
    }
    return rc;
}

/**
* @internal mvHwsSerdesAcTerminationCfg function
* @endinternal
*
* @brief   Configures AC termination on current serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] acTermEn                 - enable or disable AC termination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesAcTerminationCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN GT_BOOL                 acTermEn
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesAcTermCfgFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesAcTermCfgFunc(devNum,portGroup,serdesNum,acTermEn);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */
#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDigitalReset
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN MV_HWS_RESET        digitalReset
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDigitalReset);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDigitalReset(devNum,portGroup,serdesNum,digitalReset);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesSqDetectCfg function
* @endinternal
*
* @brief   Serdes sq_detect mask configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] sqDetect                 - true to enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSqDetectCfg
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    IN GT_BOOL             sqDetect
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesSqDetectCfgFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesSqDetectCfgFunc(devNum,portGroup,serdesNum,sqDetect);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif
/**
* @internal mvHwsSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on PCS (true/false).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] signalDet                - if true, signal was detected
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSignalDetectGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT GT_BOOL                 *signalDet
)
{
    GT_STATUS rc;
    *signalDet = GT_FALSE; /* Init the value to avoid "non initialized" problem */
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesSignalDetectGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesSignalDetectGetFunc(devNum,portGroup,serdesNum,signalDet);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - notlocked).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] cdrLock                  - if true, CDR was locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesCdrLockStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    OUT GT_BOOL                 *cdrLock
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesCdrLockStatusGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesCdrLockStatusGetFunc(devNum,portGroup,serdesNum,cdrLock);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesEncodingTypeGet function
* @endinternal
*
* @brief   Retrieves the Tx and Rx line encoding values.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] txEncodingPtr           - NRZ/PAM4
* @param[out] rxEncodingPtr           - NRZ/PAM4
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesEncodingTypeGet
(
    IN GT_U8                            devNum,
    IN GT_U32                           serdesNum,
    IN MV_HWS_SERDES_TYPE               serdesType,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesEncodingTypeGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesEncodingTypeGetFunc(devNum,serdesNum, txEncodingPtr, rxEncodingPtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesRxDatapathConfigGet function
* @endinternal
*
* @brief   Retrieves the Rx data path configuration values
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] rxDatapathConfigPtr     - (pointer to)struct:
*                                        polarityInvert (true/false)
*                                        grayEnable     (true/false)
*                                        precodeEnable  (true/false)
*                                        swizzleEnable  (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRxDatapathConfigGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 serdesNum,
    IN  MV_HWS_SERDES_TYPE                     serdesType,
    OUT MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC   *rxDatapathConfigPtr
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesRxDatapathConfigGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesRxDatapathConfigGetFunc(devNum,serdesNum, rxDatapathConfigPtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesRxPllLockGet function
* @endinternal
*
* @brief   Returns whether or not the RX PLL is frequency locked
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] lockPtr                 - (pointer to) lock state
*                                        GT_TRUE =  locked
*                                        GT_FALSE = not locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRxPllLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  serdesNum,
    IN  MV_HWS_SERDES_TYPE      serdesType,
    OUT GT_BOOL                 *lockPtr
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesRxPllLockGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesRxPllLockGetFunc(devNum,serdesNum, lockPtr);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesScanSampler function
* @endinternal
*
* @brief   Scan SERDES sampler.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
* @param[in] waitTime                 - waitTime
* @param[in] samplerSelect            - samplerSelect
* @param[in] errorThreshold           - errorThreshold
*
* @param[out] samplerResults          - Results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesScanSampler
(
    IN GT_U8                  devNum,
    IN GT_U8                  portGroup,
    IN GT_U32                 serdesNum,
    IN MV_HWS_SERDES_TYPE     serdesType,
    IN GT_U32                 waitTime,
    IN GT_U32                 samplerSelect,
    IN GT_U32                 errorThreshold,
    OUT MV_HWS_SAMPLER_RESULTS *samplerResults
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesScanSamplerFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesScanSamplerFunc(devNum, portGroup, serdesNum, waitTime, samplerSelect, errorThreshold, samplerResults);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT))) */

#ifndef VIPS_BUILD
#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesDigitalRfResetToggleSet function
* @endinternal
*
* @brief   Run digital RF Reset/Unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] waitTime                 - wait time between Reset/Unreset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on error
*/
GT_STATUS mvHwsSerdesDigitalRfResetToggleSet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U8                   waitTime
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8   i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CHECK_STATUS_EXT(GT_NOT_SUPPORTED, LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        SERDES_LOCK(devNum, (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))));
        rc = mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                             (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))), RESET);
        SERDES_UNLOCK(devNum, (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))));
        if ( GT_OK != rc )
        {
            return rc;
        }
    }

    hwsOsTimerWkFuncPtr(waitTime);

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        SERDES_LOCK(devNum, (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))));
        rc = mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                             (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))), UNRESET);
        SERDES_UNLOCK(devNum, (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))));
        if ( GT_OK != rc )
        {
            return rc;
        }
    }

    return GT_OK;
}
#endif
#endif
#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsSerdesPartialPowerDown function
* @endinternal
*
* @brief   Enable/Disable power down of Tx and Rx on Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] serdesType               - serdes type
* @param[in] powerDownRx              - Enable/Disable the power down on Serdes Rx
* @param[in] powerDownTx              - Enable/Disable the power down on Serdes Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPartialPowerDown
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    IN MV_HWS_SERDES_TYPE     serdesType,
    IN GT_BOOL     powerDownRx,
    IN GT_BOOL     powerDownTx
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPartialPowerDownFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPartialPowerDownFunc(devNum, portGroup, serdesNum, powerDownRx, powerDownTx);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesPartialPowerStatusGet function
* @endinternal
*
* @brief   Get the status of power Tx and Rx on Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] serdesType               - serdes type
* @param[out] powerRxStatus            - GT_TRUE - Serdes power
*                                      Rx is down GT_FALSE -
*                                      Serdes power Rx is up
* @param[out] powerTxStatus            - GT_TRUE - Serdes power
*                                      Tx is down GT_FALSE -
*                                      Serdes power Tx is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPartialPowerStatusGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    IN MV_HWS_SERDES_TYPE     serdesType,
    OUT GT_BOOL     *powerRxStatus,
    OUT GT_BOOL     *powerTxStatus
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPartialPowerStatusGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPartialPowerStatusGetFunc(devNum, portGroup, serdesNum, powerRxStatus, powerTxStatus);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesFirmwareDownload function
* @endinternal
*
* @brief   Download Firmware
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesType        - serdes type
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - The size of serdesArr
* @param[in] firmwareSelector   - FW to download
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/
GT_STATUS mvHwsSerdesFirmwareDownload
(
    IN  GT_U8                  devNum,
    IN  GT_UOPT                portGroup,
    IN  MV_HWS_SERDES_TYPE     serdesType,
    IN  GT_U32                 serdesArr[],
    IN  GT_U8                  numOfSerdeses,
    IN  GT_U8                  firmwareSelector
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesFirmwareDownloadFunc);

#ifdef GM_USED /* the GM not supports the SERDES */
    return GT_OK;
#endif

#ifndef MV_HWS_REDUCED_BUILD
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {   SERDES_LOCK(devNum, serdesType);
        rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesFirmwareDownloadFunc(devNum, portGroup, serdesArr, numOfSerdeses, firmwareSelector);
        SERDES_UNLOCK(devNum, serdesType);
    }

    return rc;
}

/**
* @internal mvHwsSerdesPMDLogGet function
* @endinternal
*
* @brief   Display's Tx Training Log
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesNum                - serdes number
*
* @param[out] validEntriesPtr         - (pointer to) number of
*                                        valid entries
* @param[out] pmdLogStr               - (pointer to) struct
*                                       aligned to string
* @param[in] pmdLogStrSize            - max size of pmdLogStr
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPMDLogGet
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 serdesNum,
    OUT GT_U32                 *validEntriesPtr,
    OUT GT_CHAR                *pmdLogStr,
    IN  GT_U32                 pmdLogStrSize
)
{
    GT_STATUS           rc;
    MV_HWS_SERDES_TYPE  serdesType;

    serdesType = HWS_DEV_SERDES_TYPE(devNum, serdesNum);

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPMDLogGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPMDLogGetFunc(devNum,serdesNum, validEntriesPtr, pmdLogStr, pmdLogStrSize);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*
*/
GT_STATUS mvHwsSerdesErrorInject
(
    IN GT_U8                        devNum,
    IN GT_UOPT                      portGroup,
    IN GT_UOPT                      serdesNum,
    IN GT_UOPT                      numOfBits,
    IN MV_HWS_SERDES_DIRECTION      serdesDirection
)
{
    GT_STATUS           rc;
    MV_HWS_SERDES_TYPE  serdesType;

    serdesType = HWS_DEV_SERDES_TYPE(devNum, serdesNum);

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesErrorInjectFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesErrorInjectFunc(devNum, portGroup, serdesNum, numOfBits, serdesDirection);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}
#endif

/**
* @internal hwsSerdesGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @param[in] devNum                   - system device number
*
* @param[out] hwsFuncsPtr             - serdes function
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesGetFuncPtr
(
    IN GT_U8                       devNum,
    OUT MV_HWS_SERDES_FUNC_PTRS     ***hwsFuncsPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_NOT_SUPPORTED;
    }

    *hwsFuncsPtr = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum];

    return GT_OK;
}

/**
* @internal hwsSerdesIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported SerDes
*          types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesIfClose
(
    IN GT_U8                   devNum
)
{
    MV_HWS_SERDES_TYPE serdesType;
    for (serdesType = (MV_HWS_SERDES_TYPE)0 ; serdesType < SERDES_LAST ; serdesType++)
    {
        if(PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType])
        {
            hwsOsFreeFuncPtr(PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]);
            PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType] = NULL;
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsSerdesSetExternalFirmware function
* @endinternal
*
* @brief   Set SerDes firmware to load from external file
*
* @param[in] externalFirmware         - external firmware
*
*/
GT_STATUS mvHwsSerdesSetExternalFirmware
(
    IN GT_BOOL                 externalFirmware
)
{
    PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_SET(serdesExternalFirmware, externalFirmware);
    return GT_OK;
}

/**
* @internal mvHwsSerdesLowPowerModeEnable function
* @endinternal
*
* @brief   Enable low-power mode after training.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] enableLowPower           - 1 - enable, 0 - disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLowPowerModeEnable
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN GT_BOOL                 enableLowPower
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesLowPowerModeEnableFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesLowPowerModeEnableFunc(devNum,portGroup,serdesNum,enableLowPower);
    SERDES_UNLOCK(devNum, serdesType);
    return rc;
}

/**
* @internal mvHwsPortTxRxDefaultsDbEntryGet function
* @endinternal
*
* @brief   Get the default Tx and CTLE(Rx) parameters of the
*          defaults DB entry for the proper port mode
*
* @param[in] devNum                   - system device number
* @param[in] serdesSpeed              - SerDes speed/baud rate
*                                       applicable for Avago
*                                       based devices
* @param[out] txParamsPtr             - pointer of tx params to
*                                       store the db entry
*                                       params in
* @param[out] ctleParamsPtr           - pointer of rx (ctle)
*                                       params to store the db
*                                       entry params in
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && !defined(ASIC_SIMULATION)
extern MV_HWS_AVAGO_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[];
extern MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRx16nmTuneParamsArray[];
extern MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA hwPortTxDefaults[];
GT_STATUS mvHwsPortTxRxDefaultsDbEntryGet
(
    IN GT_U8                   devNum,
    IN MV_HWS_SERDES_SPEED     serdesSpeed,
    OUT MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA    *txParamsPtr,
    OUT MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA        *ctleParamsPtr
)
{
    MV_HWS_SERDES_SPEED i = SPEED_NA;

    if ((HWS_DEV_SERDES_TYPE(devNum, 0) != AVAGO) && (HWS_DEV_SERDES_TYPE(devNum, 0) != AVAGO_16NM))
    {
        CHECK_STATUS_EXT(GT_NOT_SUPPORTED, "mvHwsPortTxRxDefaultsDbEntryGet is not supported for this Serdes type");
    }

    if ((NULL == txParamsPtr) && (NULL == ctleParamsPtr))
    {
        return GT_BAD_PTR;
    }

    for (; i < LAST_MV_HWS_SERDES_SPEED; i++)
    {
        if (AVAGO_16NM == (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 0)))
        {
            if (hwsAvagoSerdesTxRx16nmTuneParamsArray[i].serdesSpeed == serdesSpeed) break;
        }
        else
        {
            if (hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed == serdesSpeed) break;
        }
    }

    if (LAST_MV_HWS_SERDES_SPEED == i)
    {
        return GT_NOT_SUPPORTED;
    }

    /* Tx paramteres */
    if (txParamsPtr != NULL)
    {
        /* Tx parameters */
        txParamsPtr->atten = hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.atten;
        txParamsPtr->post  = hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.post;
        txParamsPtr->pre   = hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.pre;
        txParamsPtr->pre2  = hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.pre2;
        txParamsPtr->pre3  = hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.pre3;
    }

    /* Rx paramteres - no static Rx parameters exist for non-Avago serdes */
    if (AVAGO == (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 0)))
    {
        if (ctleParamsPtr != NULL)
        {
            /* Rx (CTLE) parameters */
            ctleParamsPtr->bandWidth     =    hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.bandWidth;
            ctleParamsPtr->dcGain        =    hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.dcGain;
            ctleParamsPtr->highFrequency =    hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.highFrequency;
            ctleParamsPtr->lowFrequency  =    hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.lowFrequency;
            ctleParamsPtr->squelch       =    hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.squelch;
            ctleParamsPtr->gainshape1    =    0;
            ctleParamsPtr->gainshape2    =    0;
        }
    }
    if (AVAGO_16NM == (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 0)))
    {
        if (ctleParamsPtr != NULL)
        {
            /* Rx (CTLE) parameters */
            ctleParamsPtr->bandWidth     =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.bandWidth;
            ctleParamsPtr->dcGain        =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.dcGain;
            ctleParamsPtr->highFrequency =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.highFrequency;
            ctleParamsPtr->lowFrequency  =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.lowFrequency;
            ctleParamsPtr->squelch       =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.squelch;
            ctleParamsPtr->gainshape1    =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.gainShape1;
            ctleParamsPtr->gainshape2    =    hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.gainShape2;
        }
    }

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */


GT_STATUS mvHwsSerdesGetInfoBySerdesNum
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      serdesNum,
    OUT GT_U32                      *serdesIndexPtr,
    OUT MV_HWS_PER_SERDES_INFO_PTR  *serdesInfoPtr
)
{
    if(HWS_DEV_SERDES_TYPE(devNum, serdesNum) <= D2D_PHY_GW16)
    {
        return GT_NOT_SUPPORTED;
    }

    if(NULL == serdesInfoPtr)
    {
        return GT_BAD_PTR;
    }

    if(NULL == HWS_DEV_SERDES_INFO(devNum).serdesesDb)
    {
        return GT_BAD_STATE;
    }

    /* Currently assumes serdesNum == serdesIndex
       but for more complicated serdesInfo DBs, need to implement search algorithm */
    if (serdesNum >= HWS_DEV_SERDES_INFO(devNum).numOfSerdeses)
    {
        return GT_NOT_FOUND;
    }
    *serdesInfoPtr = &HWS_DEV_SERDES_INFO(devNum).serdesesDb[serdesNum];
    if(NULL != serdesIndexPtr)
    {
        *serdesIndexPtr = (*serdesInfoPtr - HWS_DEV_SERDES_INFO(devNum).serdesesDb);
    }

    return GT_OK;

}

/**
* @internal mvHwsSerdesOperation function
* @endinternal
*
* @brief   set serdes operation
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] portMode                 - port standard metric
* @param[in] operation                - port operation mode
* @param[in] dataPtr                     - data
*
* @param[out] resultPtr                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesOperation
(
   IN GT_U8                       devNum,
   IN GT_U32                      portGroup,
   IN GT_U32                      serdesNum,
   IN MV_HWS_PORT_STANDARD        portMode,
   IN MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
   IN GT_U32                     *dataPtr,
   OUT GT_U32                    *resultPtr
)
{
    GT_STATUS rc = GT_OK;

    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, (HWS_DEV_SERDES_TYPE(devNum, serdesNum)), serdesOperFunc);

    if (((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)) || (dataPtr == NULL) || (resultPtr == NULL))
    {
        return GT_BAD_PARAM;
    }
    *resultPtr = 0;

    if (PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][HWS_DEV_SERDES_TYPE(devNum, serdesNum)]->serdesOperFunc == NULL)
    {
        return GT_OK;
    }
    SERDES_LOCK(devNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    rc = (PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][HWS_DEV_SERDES_TYPE(devNum, serdesNum)]->serdesOperFunc(devNum, portGroup, serdesNum,
                                                                           portMode, operation, dataPtr, resultPtr));
    SERDES_UNLOCK(devNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    return rc;

}

#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsSerdesDbTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_TX_CONFIG_DATA - pointer to Tx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDbTxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_TYPE                serdesType,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParamsPtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDbTxCfgGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Db Tx Config Get: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDbTxCfgGetFunc(devNum, portGroup, serdesNum, baudRate, configParamsPtr);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}


/**
* @internal mvHwsSerdesDbRxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure RX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_RX_CONFIG_DATA - pointer to Rx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDbRxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_TYPE                serdesType,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParamsPtr
)
{
    GT_STATUS rc = GT_OK;

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDbRxCfgGetFunc);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Db Rx Config Get: serdes %s.\n",
      PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDbRxCfgGetFunc(devNum, portGroup, serdesNum, baudRate, configParamsPtr);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesDumpInfo function
* @endinternal
*
* @brief   Dump SerDes info
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - SerDes number to power up/down
* @param[in]  dumpType  - dump type
* @param[in]  printBool - print to terminal
* @param[out] outputPtr - generic pointer to output data
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsSerdesDumpInfo
(
    IN  GT_U8                           devNum,
    IN  GT_UOPT                         portGroup,
    IN  GT_UOPT                         serdesNum,
    IN  MV_HWS_SERDES_DUMP_TYPE_ENT     dumpType,
    IN  GT_BOOL                         printBool,
    OUT GT_U32                          *outputPtr
)
{
    GT_STATUS rc = GT_OK;

    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum), serdesDumpInfoFunc);
    SERDES_LOCK(devNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][HWS_DEV_SERDES_TYPE(devNum, serdesNum)]->serdesDumpInfoFunc(devNum, portGroup, serdesNum,
                                                                           dumpType, printBool, outputPtr);
    SERDES_UNLOCK(devNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    return rc;

}

/**
* @internal mvHwsSerdesPresetOverrideSet function
* @endinternal
*
* @brief   Debug hooks APIs : SetTxLocalPreset, SetRemotePreset, SetCTLEPreset
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] serdesPresetOverride     - override preset default paramters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPresetOverrideSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    IN  MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverride
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_SERDES_TYPE serdesType;

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPresetOverrideSetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPresetOverrideSetFunc(devNum,serdesNum,serdesPresetOverride);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesPresetOverrideGet function
* @endinternal
*
* @brief   Debug hooks APIs : GetTxLocalPreset, GetRemotePreset, GetCTLEPreset
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* 
* @param[out] serdesPresetOverridePtr - (pointer to)override preset default paramters
* 
* @param[out] val                     - value to get
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPresetOverrideGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    OUT MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverridePtr
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_SERDES_TYPE serdesType;

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesPresetOverrideGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesPresetOverrideGetFunc(devNum,serdesNum,serdesPresetOverridePtr);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

#endif
/**
* @internal mvHwsSerdesSdwAddrCalc function
* @endinternal
*
* @brief   Calculate register address in SDW (SerDes Wrapper)
*
* @param[in]  devNum       - device number
* @param[in]  serdesNum    - serdes number
* @param[in]  regAddr      - reg addr inside SDW
 *@param[in]  baseAddr     - base address of the SDW
 *@param[in]  regType      - EXTERNAL_REG / INTERNAL_REG
* @param[out] addressPtr   - pointer to the output address
*
* @retval 0                 - on success
* @retval 1                 - on error
*/
GT_VOID mvHwsSerdesSdwAddrCalc
(
    IN  GT_U8           devNum,
    IN  GT_UOPT         serdesNum,
    IN  GT_UREG_DATA    regAddr,
    IN  GT_UREG_DATA    baseAddr,
    IN  GT_U32          regType,
    OUT GT_U32          *addressPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;
    GT_U32                     serdesIndex;

    if(GT_OK != mvHwsSerdesGetInfoBySerdesNum(devNum,serdesNum,&serdesIndex,&serdesInfo))
    {
        *addressPtr = 0x0;
        return;
    }

    if (regType == INTERNAL_REG)
    {
        *addressPtr = SDW_INTERNAL_REG_OFFSET;
    }
    else if (regType == EXTERNAL_REG)
    {
        if (MV_SDW_IS_PER_LANE_REG_MAC(regAddr))
        {
            *addressPtr = SDW_EXTERNAL_REG_OFFSET +  (serdesInfo->internalLane * 0x4);
        }
        else
        {
            *addressPtr = SDW_EXTERNAL_REG_OFFSET;
        }
    }
    else
    {
        *addressPtr = 0x0;
        return;
    }

    *addressPtr  =  baseAddr + *addressPtr + regAddr ;

    return;
}

/**
* @internal mvHwsSerdesTemperatureGet function
* @endinternal
*
* @brief   Read SD temperature
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] serdesTemperature       - (pointer to) temperature
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTemperatureGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  serdesNum,
    OUT GT_32                   *serdesTemperature
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_SERDES_TYPE serdesType;


    if ( NULL == serdesTemperature )
    {
      return GT_BAD_PTR;
    }

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesTemperatureFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesTemperatureFunc(devNum,portGroup,serdesNum,serdesTemperature);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesDroGet function
* @endinternal
*
* @brief   Get DRO (Device Ring Oscillator).
*           Indicates performance of device
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
*
* @param[out] dro                     - (pointer to)dro
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDroGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U16                  *dro
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_SERDES_TYPE serdesType;


    if ( NULL == dro )
    {
      return GT_BAD_PTR;
    }

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, serdesNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    MV_HWS_IS_SERDES_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, serdesType, serdesDroGetFunc);
    SERDES_LOCK(devNum, serdesType);
    rc = PRV_NON_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR(hwsSerdesFuncsPtr)[devNum][serdesType]->serdesDroGetFunc(devNum,serdesNum,dro);
    SERDES_UNLOCK(devNum, serdesType);

    return rc;
}

/**
* @internal mvHwsSerdesAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         SDs cannot be accessed by more than one
*         client concurrently. Concurrent access might result in
*         invalid data read from the firmware. There are two
*         scenarios that require protection:
*         1. Multi-Process Application. This case is protected
*         by SW Semaphore. SW Semaphore should be defined for
*         each supported OS: FreeRTOS, Linux, and any customer
*         OS. This protection is relevant for Service CPU and
*         Host:
*         - Service CPU includes multi-process application, therefore protection is required
*         - Host customer application might / might not include multi-process, but from CPSS
*         point of view protection is required
*         2. Multi-Processor Environment
*         This case is protected by HW Semaphore.
*         HW Semaphore is defined based on MSYS / CM3 resources.
*         In case customer does not use MSYS / CM3 resources,
*         the customer will need to implement its own HW
*         Semaphore. This protection is relevant ONLY in case
*         Service CPU Firmware is loaded to CM3
*
* @param[in] devNum                   - system device number
*
*          */
void mvHwsSerdesAccessLock
(
    IN GT_U8 devNum
)
{
#ifdef PX_FAMILY
    devNum = devNum;
#endif
#ifndef FALCON_DEV_SUPPORT
    /*
    ** SW Semaphore Protection Section
    ** ===============================
    */
#ifdef CHX_FAMILY

    /* Host SW Protection */
    if (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesAccessMutex))
    {
        hwsOsMutexLock(PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesAccessMutex));
    }
#elif defined(MV_HWS_FREE_RTOS)
    /* Service CPU SW Protection */
    mvPortCtrlSerdesLock();
#endif /* defined(CHX_FAMILY) */

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#ifdef CHX_FAMILY
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) )
        mvSemaLock(devNum, devNum, MV_SEMA_SERDES);
#elif defined(MV_HWS_FREE_RTOS)
        mvSemaLock(devNum, MV_SEMA_SERDES);
#endif
#endif  /* FALCON_DEV_SUPPORT */
}

/**
* @internal mvHwsSerdesAccessLock function
* @endinternal
*
* @brief   See description in mvHwsSerdesAccessLock API
* @param[in] devNum                   - system device number
*/
void mvHwsSerdesAccessUnlock
(
    IN GT_U8 devNum
)
{
#ifdef PX_FAMILY
    devNum = devNum;
#endif
#ifndef FALCON_DEV_SUPPORT
    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
#ifdef CHX_FAMILY
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) )
        mvSemaUnlock(devNum, devNum, MV_SEMA_SERDES);
#elif defined(MV_HWS_FREE_RTOS)
        mvSemaUnlock(devNum, MV_SEMA_SERDES);
#endif

    /*
    ** SW Semaphore Protection Section
    ** ===============================
    */
#ifdef CHX_FAMILY
    /* Host SW Protection */
    if (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesAccessMutex))
    {
        hwsOsMutexUnlock(PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesAccessMutex));
    }
#elif defined(MV_HWS_FREE_RTOS)
    /* Service CPU SW Protection */
    mvPortCtrlSerdesUnlock();
#endif /* defined(CHX_FAMILY) */
#endif /*FALCON_DEV_SUPPORT*/
}
