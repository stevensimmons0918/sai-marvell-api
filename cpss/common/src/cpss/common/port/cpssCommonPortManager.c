/********************************************************************************
*       (c), Copyright 2001, Marvell International Ltd.         *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE    *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.   *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,    *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssCommonPortManager.c
*
* @brief CPSS implementation for Port manager module.
*
*
* @version  1
********************************************************************************
*/
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/port/cpssPortSerdes.h>
#include <cpss/common/private/prvCpssCyclicLogger.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/port/private/prvCpssPortPcsCfg.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>

#ifdef CMD_LUA_CLI
#include <cpss/common/port/private/prvCpssPortManagerLuaTypes.h>
#endif /* CMD_LUA_CLI */
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables include and macros*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/*#include <cpss/common/private/globalShared/prvCpssCommonModGlobalSharedDb.h>*/
#define PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.portDir.commonPortManagerSrc._var)

#define CPSS_PORT_MANAGER_LOG_INFORMATION_MAC                       CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC
#define CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC                  CPSS_CYCLIC_LOGGER_LOG_ERROR_AND_RETURN_MAC
#define CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC                  CPSS_CYCLIC_LOGGER_LOG_PORT_INFORMATION_MAC

#undef PORT_MGR_DEBUG
#ifdef PORT_MGR_DEBUG
#define CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC(...)      CPSS_CYCLIC_LOGGER_DBG_LOG_INFORMATION_MAC(..)
#else
#define CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC(...)
#endif
#define CPSS_PORT_MANAGER_LOG_EVENT_CONVERT_INFORMATION_MAC(...)
#define CPSS_PORT_MANAGER_LOG_MASK_CONVERT_INFORMATION_MAC(...)

/* check NULL pointer */
#define CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(ptr)  if(ptr == NULL) { \
    CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("parameter is null");     \
    CPSS_NULL_PTR_CHECK_MAC(ptr);                    \
}

/* check that the device exists
  return GT_BAD_PARAM on error
*/
#define CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum)               \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))    {          \
        CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("device number not exist");  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, prvCpssLogErrorMsgDeviceNotExist, devNum);\
    }

#define PRV_CPSS_PORT_MANAGER_BIT_MAC(bit) (1<<bit)

/* checking if mac loopback is configure and enable*/
#define PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(_loopbackCfg) ((_loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) \
               && (_loopbackCfg.loopbackMode.macLoopbackEnable))

/* checking if serdes loopback analog TX2RX is configure */
#define PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(_loopbackCfg) ((_loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) \
               && (_loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E))

/* checking if serdes loopback digital RX2TX is configure */
#define PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(_loopbackCfg) ((_loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) \
               && (_loopbackCfg.loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E))

/* Macro for two lanes speeds/port modes - to ease the use. */
#define PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(_mode) ((_mode == CPSS_PORT_INTERFACE_MODE_KR2_E) || (_mode == CPSS_PORT_INTERFACE_MODE_CR2_E) \
     || (_mode == CPSS_PORT_INTERFACE_MODE_CR2_C_E) || (_mode == CPSS_PORT_INTERFACE_MODE_KR2_C_E) \
     || (_mode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E) || (_mode == CPSS_PORT_INTERFACE_MODE_RXAUI_E))


#define PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(_portMode) (_1000Base_X == _portMode || SGMII == _portMode || SGMII2_5 == _portMode || QSGMII == _portMode || \
                                                                 _5_625GBaseR == _portMode || _100Base_FX == _portMode || _2500Base_X == _portMode ||              \
                                                                  (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && portMode == _5GBaseR) )

/* Macro for slow port modes */
#define PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(_mode) ((_mode == SGMII) || (_mode == QSGMII) || (_mode == _5G_QUSGMII)|| \
                                        (_mode == _10G_OUSGMII) || (_mode == _1000Base_X) || (_mode == SGMII2_5) || (_mode == _2500Base_X))

/* From port manager perspective, around 100g are those speeds */
#define PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(_speed) (CPSS_PORT_SPEED_100G_E == _speed || CPSS_PORT_SPEED_102G_E == _speed \
               || CPSS_PORT_SPEED_107G_E == _speed)

/* Macro for port modes that use XPCS - to ease the use. */
#define PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(_mode) ((_mode == CPSS_PORT_INTERFACE_MODE_XGMII_E) || (_mode == CPSS_PORT_INTERFACE_MODE_RXAUI_E) \
     || (_mode == CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E))

/* Marco to check gige ports (ports 0-23) in AC3/5 */
#define PRV_CPSS_PORT_MANAGER_CHECKING_GIGE_PORTS_ON_AC3_BASED_DEVICE(_devNum,_portNum) (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(_devNum) && (_portNum < 24))

/* Marco to check gige ports (ports 0-47) in BOBK Caelum*/
#define PRV_CPSS_PORT_MANAGER_CHECKING_GIGE_PORTS_ON_BOBK_CAELUM_BASED_DEVICE(_devNum,_portNum) (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E && (_portNum < 48))

/* Macro for getting the port manager database */
#define PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc)                                       \
rc = ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortManagerDbGetFunc(devNum, &tmpPortManagerDbPtr))); \
if (rc != GT_OK)                                                                                                \
{                                                                                                               \
    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get portManagerDbPtr");                           \
}

/* Macro that show devies using polling method */
#define PRV_PORT_MANAGER_POLLING_DEVICES(devNum) ((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) ||  \
        (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
/* memory allocation check macros */
#define PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, erFlag)      \
do                                                     \
{                                                      \
    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr == NULL)                   \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                      /*" perPhyPortDbPtr is NULL", portNum, __FUNCTION__);      */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                         \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
    if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum] == NULL)            \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                  /*" perPhyPortDbPtr[portNum] is NULL", portNum, __FUNCTION__);      */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                         \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum, erFlag)     \
do                                                   \
{                                                    \
    if (tmpPortManagerDbPtr->portMngSmDbPerMac == NULL)                         \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("(portMac %d)%s: "                    */\
                    /*" portMngSmDbPerMac is NULL", portMacNum, __FUNCTION__);    */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                       \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
    if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum] == NULL)                   \
    {                                                  \
        /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                /*" portMngSmDbPerMac[portMacNum] is NULL", portMacNum, __FUNCTION__);  */\
        if (erFlag)                                           \
        {                                                \
            return /*stub comment*/ GT_NOT_INITIALIZED;                         \
        }                                                \
        return GT_OK;                                          \
    }                                                  \
} while (0)

#define PRV_PORT_MANAGER_CHECK_XLG_CG_MAC_PORT_MAC(curPortParams)\
((curPortParams.numOfActLanes > 1) && ((curPortParams.portMacType == XLGMAC) \
                                        || (curPortParams.portMacType ==  CGMAC)) \
                                        && !PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, serdesNum, erFlag)      \
do                                                   \
{                                                    \
  if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr == NULL)                 \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                      /*" perSerdesDbPtr is NULL", serdesNum, __FUNCTION__);    */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
  if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[serdesNum] == NULL)            \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
                 /*" perSerdesDbPtr[serdesNum] is NULL", serdesNum, __FUNCTION__);   */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, erFlag)   \
do                                                 \
{                                                  \
  if (tmpPortManagerDbPtr->portMngSmDb == NULL)                          \
  {                                                \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                   */\
                      /*" portMngSmDb is NULL", portNum, __FUNCTION__);    */\
    if (erFlag)                                         \
    {                                              \
      return /*stub comment*/ GT_NOT_INITIALIZED;                       \
    }                                              \
    return GT_OK;                                        \
  }                                                \
  if (tmpPortManagerDbPtr->portMngSmDb[portNum] == NULL)                     \
  {                                                \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                   */\
                /*" portMngSmDb[portNum] is NULL", portNum, __FUNCTION__);      */\
    if (erFlag)                                         \
    {                                              \
      return /*stub comment*/ GT_NOT_INITIALIZED;                       \
    }                                              \
    return GT_OK;                                        \
  }                                                \
} while (0)

#define PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, erFlag)       \
do {                                                  \
  if (tmpPortManagerDbPtr->portsApAttributesDb == NULL)                        \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
        /*" portsApAttributesDb is NULL. erFlag=%d", portNum, __func__, erFlag);         */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
  if (tmpPortManagerDbPtr->portsApAttributesDb[portNum] == NULL)                   \
  {                                                  \
    /*CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC("[Port %2d] %s: "                     */\
          /*" portsApAttributesDb[portNum] is NULL. erFlag=%d", portNum, __func__, erFlag);  */\
    if (erFlag)                                           \
    {                                                \
      return /*stub comment*/ GT_NOT_INITIALIZED;                         \
    }                                                \
    return GT_OK;                                          \
  }                                                  \
} while (0)

#define CPSS_PM_SET_MASKING_BIT(tmpPortManagerDbPtr, portNum, eventNum) \
    (tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap |= eventNum) \

#define CPSS_PM_SET_UNMASKING_BIT(tmpPortManagerDbPtr, portNum, eventNum) \
    (tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap &= ~(eventNum)) \

#define  CPSS_PM_START_TIME(pmStats)\
        cpssOsTimeRT(&(pmStats->timeStampSec),&(pmStats->timeStampNSec))

#define PRV_PORT_MANAGER_DEFAULT_PARALLEL_DETECT_EN         GT_TRUE
#define PRV_PORT_MANAGER_DEFAULT_PARALLEL_DETECT_RESTAR_NUM 15
#define PRV_PORT_MANAGER_DEFAULT_FC_ASM_DIR                 CPSS_PORT_AP_FLOW_CONTROL_SYMMETRIC_E
#define PRV_PORT_MANAGER_DEFAULT_FC_PAUSE                   GT_TRUE
#define PRV_PORT_MANAGER_DEFAULT_LANE_NUM                   0
#define PRV_PORT_MANAGER_DEFAULT_NONECE_DISABLE             GT_TRUE
#define PRV_PORT_MANAGER_DEFAULT_SKIP_RES                   GT_FALSE
/* default were changed in sip 6_10 */
#define PRV_PORT_MANAGER_DEFAULT_6_10_NONECE_DISABLE     GT_FALSE
#define PRV_PORT_MANAGER_DEFAULT_6_10_FC_PAUSE           GT_FALSE


#define PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(val, high, def)\
        ((val > high) && (val != def))

#define PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(val, low, high, def)\
        (((val < low) || (val > high)) && (val != def))

#define PRV_PORT_MANAGER_CHECK_AND_SET(hwsParam, cpssParam, defVal, cast)\
        hwsParam = (cast)((cpssParam == NA_16BIT) ?  defVal : cpssParam)

#define AP_SM_WAIT_FOR_PORT_DISABLE          42      /*ap SM status AN_PORT_ENABLE_WAIT*/

#define PRV_PORT_MANAGER_TIME_DIFF(startSec,startNsec, endSec,endNsec)\
        ((int)(endSec - startSec)*1000 + (int)(endNsec-startNsec)/1000000);

/************************************************
        External References
 ************************************************/
/* array defining serdes speed used in given interface mode for given port data speed */
extern CPSS_PORT_SERDES_SPEED_ENT serdesFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E];
extern MV_HWS_AVAGO_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[];
extern MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesTxTune16nmPreset1ParamsProfiles[4][HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES];

extern GT_STATUS prvFindCauseRegAddrByEvent
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData/*portNum*/,
    GT_U32         *regAddr
);

extern GT_STATUS prvFindIntCauseBit
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData/*portNum*/,
    GT_U32         *bit
);

extern GT_STATUS prvFindInterruptIdx
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData,
    GT_U32         *interruptIdx
);
static GT_STATUS prvCpssPortManagerEventBitMaskingReading
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_U32                     portMacNum,
    IN  MV_HWS_PORT_INIT_PARAMS    *curPortParamsPtr,
    IN  PRV_CPSS_PORT_MNG_DB_STC   *tmpPortManagerDbPtr,
    IN  CPSS_UNI_EV_CAUSE_ENT      uniEv,
    OUT GT_U32                     *data
);

extern GT_STATUS prvCpssPmRegPortParamsGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT  CPSS_PM_PORT_PARAMS_STC  *portParams
);

extern GT_STATUS prvCpssPmApPortParamsSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PM_PORT_PARAMS_STC  *portParams
);

#if (defined CHX_FAMILY)
extern const CPSS_PORT_AP_PARAMS_STC prvCpssPortApDefaultParams;
#endif

/************************************************
        Code
************************************************/
GT_STATUS prvCpssPortManagerStatTimeGet
(
    IN  PRV_CPSS_PORT_MNG_PORT_SM_DB_STC *timeStampPtr,
    OUT GT_U32 *timerPtr
)
{
    GT_U32 sec,nsec;
    GT_32  secDiff, nsecDiff;
    CHECK_STATUS(cpssOsTimeRT(&(sec),&(nsec)));
    secDiff = sec - timeStampPtr->timeStampSec;
    nsecDiff = nsec - timeStampPtr->timeStampNSec;
    *timerPtr = secDiff*1000 + (nsecDiff/1000000);
   return GT_OK;
}

/**
* @internal prvCpssLockLowLevelApiSet function
* @endinternal
*
* @brief  change the lockLowLevelApi manually
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                 - device number
* @param[in] lockStatus             - set lock or unlock
*
* @retval GT_OK                    - on success
*
* @note this is only for debugging, this can't take part in
*       correct port configuration
*
*/
GT_STATUS prvCpssLockLowLevelApiSet
(
    IN GT_U8   devNum,
    IN GT_BOOL lockStatus
)
{
   PRV_CPSS_PP_MAC(devNum)->lockLowLevelApi  = lockStatus;
   return GT_OK;
}

/**
* @internal prvCpssPortManagerMaskMacLevelEvents function
* @endinternal
*
* @brief  API for masking mac interrupts. port manager mac
*         interrupts is CPSS_PM_PORT_LINK_STATUS_CHANGED_E
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum             - device number
* @param[in] portNum            - physical port number
* @param[in] operation          - mask operation (mask or unmask)
*
* @retval GT_OK                - on success
* @retval GT_FAIL              - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM         - on bad physical port number
*/
GT_STATUS prvCpssPortManagerMaskMacLevelEvents
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, 1);

    if (operation == CPSS_EVENT_UNMASK_E)
    {
        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_DISABLE_MODE_E) ||
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_MAC_LEVEL_EVENTS_DISABLE_MODE_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "unMasking MAC LEVEL EVENTS is not allowed, blocked by application");
            return GT_OK;
        }
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"%s mac level events on port",
                                          (operation == CPSS_EVENT_MASK_E) ? "Masking" : "Unmasking");

    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E, portNum, operation);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d LINK_STATUS_CHANGE returned %d ", devNum, portNum, operation, rc);
    }

    /* updating masking bitmap */
    if (operation == CPSS_EVENT_MASK_E)
    {
        CPSS_PM_SET_MASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_PORT_LINK_STATUS_CHANGED_E);
    }
    else
    {
        CPSS_PM_SET_UNMASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_PORT_LINK_STATUS_CHANGED_E);
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"masking bitmap: %08x"
                                               ,tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap);

    return GT_OK;
}

/**
* @internal prvCpssPortManagerMaskLowLevelEvents function
* @endinternal
*
* @brief  API for masking low interrupts. port manager low
*         interrupts are: CPSS_PM_PCS_ALIGN_LOCK_LOST_MASKING_E,
*         CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E and (only below sip 6)
*         CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E(only below sip 6)
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum             - device number
* @param[in] portNum            - physical port number
* @param[in] operation          - mask operation (mask or unmask)
*
* @retval GT_OK                - on success
* @retval GT_FAIL              - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM         - on bad physical port number
*/
GT_STATUS prvCpssPortManagerMaskLowLevelEvents
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS                 rc;
    PRV_CPSS_PORT_MNG_DB_STC  *tmpPortManagerDbPtr;
    CPSS_EVENT_MASK_SET_ENT tmpOperation;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"%s SIGNAL_DETECT,ALIGN_LOCK,GB_LOCK ",
                                      (operation == CPSS_EVENT_MASK_E) ? "Masking" : "Unmasking" );

    if (operation == CPSS_EVENT_UNMASK_E)
    {
        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_DISABLE_MODE_E) ||
        (tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode == CPSS_PORT_MANAGER_UNMASK_LOW_LEVEL_EVENTS_DISABLE_MODE_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "unMasking LOW LEVEL EVENTS is not allowed, blocked by application");
            return GT_OK;
        }
    }

    if(PRV_CPSS_PORT_MANAGER_CHECKING_GIGE_PORTS_ON_AC3_BASED_DEVICE(devNum,portNum))
    {
        /*AC3 based devices on Gige ports are not supporting the low level interrupts, but supporting CPSS_PP_PORT_SYNC_STATUS_CHANGED_E */
        rc = cpssEventDeviceMaskWithEvExtDataGet(devNum,
                                                 CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
                                                 portNum, &tmpOperation);
        if (rc == GT_NOT_FOUND)
        {
            /*do nothing - no such event for this port */
        }
        else if(rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataGet %d SYNC_STATUS_CHANGED returned %d ", devNum, portNum, operation, rc);
        }
        else
        {
            rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                     CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
                                                     portNum, operation);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d SYNC_STATUS_CHANGED returned %d ", devNum, portNum, operation, rc);
            }
        }
        return GT_OK;
    }

    /* check if CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E exists for 'portNum' */
    rc = cpssEventDeviceMaskWithEvExtDataGet(devNum,
                                             CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                             portNum, &tmpOperation);
    if (rc == GT_NOT_FOUND)
    {
        /*do nothing - no such event for this port */
    }
    else if(rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataGet %d ALIGN_LOCK_LOST returned %d ", devNum, portNum, operation, rc);
    }
    else
    {
        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                                 portNum, operation);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d ALIGN_LOCK_LOST returned %d ", devNum,portNum, operation, rc);
        }
    }

    if (operation == CPSS_EVENT_MASK_E)
    {
        CPSS_PM_SET_MASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_PCS_ALIGN_LOCK_LOST_MASKING_E);
    }
    else
    {
        CPSS_PM_SET_UNMASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_PCS_ALIGN_LOCK_LOST_MASKING_E);
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* check if CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E exists for 'portNum' */
        rc = cpssEventDeviceMaskWithEvExtDataGet(devNum,
                                                 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
                                                 portNum, &tmpOperation);
        if (rc == GT_NOT_FOUND)
        {
            /*do nothing - no such event for this port */
        }
        else if(rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataGet %d GB_LOCK_SYNC_CHANGE returned %d ", devNum, portNum, operation, rc);
        }
        else
        {
            rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                     CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
                                                     portNum, operation);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d GB_LOCK_SYNC_CHANGE returned %d ", devNum, portNum, operation, rc);
            }
        }


        /* check if CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E exists for 'portNum' */
        rc = cpssEventDeviceMaskWithEvExtDataGet(devNum,
                                                 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                 portNum, &tmpOperation);
        if (rc == GT_NOT_FOUND)
        {
            /*do nothing - no such event for this port */
        }
        else if(rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataGet %d SIGNAL_DETECT_CHANGE returned %d ", devNum, portNum, operation, rc);
        }
        else
        {
            /* mask signal detect */
            rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                 portNum, operation);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d SIGNAL_DETECT_CHANGE returned %d ", devNum, portNum, operation, rc);
            }
        }

        /* updating masking bitmap */
        if (operation == CPSS_EVENT_MASK_E)
        {
            CPSS_PM_SET_MASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_PCS_GB_LOCK_SYNC_CHANGE_MASKING_E);
            CPSS_PM_SET_MASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_MMPCS_SIGNAL_DETECT_CHANGE_MASKING_E);
        }
        else
        {
            CPSS_PM_SET_UNMASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_PCS_GB_LOCK_SYNC_CHANGE_MASKING_E);
            CPSS_PM_SET_UNMASKING_BIT(tmpPortManagerDbPtr, portNum, CPSS_PM_MMPCS_SIGNAL_DETECT_CHANGE_MASKING_E);
        }
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"masking bitmap: %08x"
                                            ,tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap);

    return GT_OK;
}

/**
* @internal prvCpssPortManagerMaskEvents function
* @endinternal
*
* @brief  API that calling mac and low masking functions
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum             - device number
* @param[in] portNum            - physical port number
* @param[in] operation          - mask operation (mask or unmask)
*
* @retval GT_OK                - on success
* @retval GT_FAIL              - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM         - on bad physical port number
*/
GT_STATUS prvCpssPortManagerMaskEvents
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_EVENT_MASK_SET_ENT  operation
)
{
    GT_STATUS rc;
    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, operation);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
    }
    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, operation);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
    }
    return GT_OK;
}

/**
* @internal prvCpssPortManagerInterruptAdditionals function
* @endinternal
*
* @brief  API to associate all port MACs of a multi-lane interface.
*     For example, 40Gig port interface combined of total 4
*     ports (and will return 4 active lanes and arrPtr with 4
*     mac ports number), 10G port combined of 1 port. This API
*     helps port manager to manage interrupts of a group of
*     ports as if they are raised within the first port in the
*     group which is the port that the interface mode is
*     configured upon.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in]  devNum             - device number
* @param[in]  macPortNum         - mac port number
* @param[out] arrPtr             - lane + mac port data
* @param[out] numOfActLanes      - number of active lanes
*
* @retval GT_OK                 - on success
* @retval GT_FAIL               - on error
* @retval GT_NOT_SUPPORTED      - on not supported device
* @retval GT_BAD_PARAM          - on bad physical port number
*/
GT_STATUS prvCpssPortManagerInterruptAdditionals
(
    IN  GT_U8   devNum,
    IN  GT_U32  macPortNum,
    OUT GT_U32  *arrPtr,
    OUT GT_U8   *numOfActLanes
)
{
    GT_STATUS                     rc;
    GT_PHYSICAL_PORT_NUM          portNum;
    GT_U32                        lane;
    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    MV_HWS_PORT_STANDARD          portMode;
    PRV_CPSS_PORT_MNG_DB_STC      *tmpPortManagerDbPtr;
    GT_U32                        portGroup;
    GT_U32                        macPortNumTmp = 0;

    *numOfActLanes = 0;

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_INFORMATION_MAC("[MacPort %d] device not applicable for additionals convertion mac. Returning.",macPortNum);
        return GT_OK;
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* convert mac to phy */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E/*PRV_CPSS_EV_CONVERT_TYPE_MAC_TO_PHY_E*/,
                                                                           macPortNum, &portNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling ppMacConvertFunc on port returned %d ", macPortNum, rc);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 0);


    portGroup = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;
    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;

    /* some devices enter this  function with a converted mac port number, in order to get curPortParams, using a temp unconverted mac port from the port manager dataase.
       other devies will get non-converted mac port, this will not effect them */
     macPortNumTmp = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, macPortNumTmp, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null", devNum, portNum );
    }

    CPSS_PORT_MANAGER_LOG_MASK_CONVERT_INFORMATION_MAC("[MacPort %d] NumOfActLanes(total interrupts) %d, portMode %d"
                                                            ,macPortNum,curPortParams.numOfActLanes, portMode);
    /* Filling OUT data */
    *numOfActLanes = curPortParams.numOfActLanes;
    for (lane = 0; lane < *numOfActLanes; lane++)
    {
        arrPtr[lane] = macPortNum + lane;
    }

    return GT_OK;
}

/**
* @internal prvCpssPortMacEventConvert function
* @endinternal
*
* @brief  API to convert MAC extended data for port related unified events.
*     This API is needed in order for a multi-lane interface mode
*     to receive interrupts from all of it's lanes and in order for
*     them to be received as if they are raised from the port related
*     to the multi-lane interface.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2 Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]    devNum            - device number
* @param[inout] macPortNum        - (pointer to) the converted extended data of the interrupt
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortMacEventConvert
(
    IN    GT_U8 devNum,
    INOUT GT_U32 *macPortNum
)
{
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    GT_STATUS                    rc;
    GT_PHYSICAL_PORT_NUM         portNumTemp;
    GT_PHYSICAL_PORT_NUM         portNum;
    GT_U32                       portMacNumTemp;
    GT_U32                       i;
    MV_HWS_PORT_INIT_PARAMS      curPortParams;
    MV_HWS_PORT_STANDARD         portMode;
    GT_U32                       portGroup;
    GT_U32                       hwsPhyPortNum;
    GT_U32                       firstPortNumInGroup = 0; /* the first port in a group of 4 ports*/

    CPSS_NULL_PTR_CHECK_MAC(macPortNum);

    portMacNumTemp = 0;
    portNumTemp = 0;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E/*PRV_CPSS_EV_CONVERT_TYPE_MAC_TO_PHY_E*/,
                                                                           *macPortNum, &portNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc(devNum, portNum, &speed);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Maybe this physical port is just a lane on another physical port */
    if (speed == CPSS_PORT_SPEED_NA_E)
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc(devNum, portNum, &ifMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E/*PRV_CPSS_EV_CONVERT_TYPE_PHY_TO_MAC_E*/,
                                                                                portNum, &portMacNumTemp);
        if (rc != GT_OK)
        {
            portMacNumTemp = *macPortNum;
        }

        if (portMacNumTemp % 4 == 0)
        {
            return GT_OK; /*first lane in the group don't need to go back in the lanes to enable interrupts cause he is the first */
        }

        firstPortNumInGroup = portMacNumTemp - (portMacNumTemp % 4);

        /* Searching for a physical port which is configured to multi lane speed. If found, and
          this physical port has a mac which is in offset of up to 3 from our port mac,
          than our physical port is must be a lane of it.*/
        for (i = 1; i < 4; i++)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConvertFunc(devNum, PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E/*PRV_CPSS_EV_CONVERT_TYPE_MAC_TO_PHY_E*/,
                                                                                   (portMacNumTemp - i), &portNumTemp);

            if (rc == GT_OK)
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc(devNum, portNumTemp, &speed);
                if (rc != GT_OK)
                {
                    return rc;
                }

                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc(devNum, portNumTemp, &ifMode);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (speed != CPSS_PORT_SPEED_NA_E && ifMode != CPSS_PORT_INTERFACE_MODE_NA_E && ifMode != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E)
                {
                    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &hwsPhyPortNum);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), hwsPhyPortNum);

                    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, (portMacNumTemp - i), portMode, &curPortParams))
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }

                    if (curPortParams.numOfActLanes > 1 &&
                        ( curPortParams.numOfActLanes >= (*macPortNum - (portMacNumTemp - i)) ))
                    {
                        /* if not enter here, might because *macPortNum,"probably a part of deleted interface and was triggered by such operation */
                        *macPortNum = (portMacNumTemp - i);
                        break;
                    }
                }
            }

            if(firstPortNumInGroup == portMacNumTemp - i)
            {
                break; /* the first port in group is done, keeping the 'for' from here will override other group of ports, in the case we the for will stop here */
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerPushEvent function
* @endinternal
*
* @brief  Push artificial event into event queues of CPSS.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group
* @param[in] portNum         - physical port number
* @param[in] extraData       - extra data relevant for the unified event
* @param[in] uniEv           - unified event to push
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortManagerPushEvent
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_U32                extraData,
    IN CPSS_UNI_EV_CAUSE_ENT uniEv
)
{
    GT_U32 interruptIdx;
    GT_STATUS rc;
    PRV_CPSS_DRV_EV_REQ_NODE_STC *evNodesPool;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        /* xCat3/AC5 not supporting push event, but still applicable device */
        return GT_OK;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (CPSS_SYSTEM_RECOVERY_PROCESS_HA_E == tempSystemRecovery_Info.systemRecoveryProcess)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "skip event push uniEv %d due to process HA ", uniEv);
        return GT_OK;
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "pushing event uniEv %d", uniEv);

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortManagerPushEvent not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    PRV_CPSS_INT_SCAN_LOCK();

    evNodesPool = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.
                  portGroupInfo[CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS].intNodesPool;

    /* get interrupt index */
    rc = prvFindInterruptIdx(devNum, portGroup, uniEv/*CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E*/,
                             extraData/*portMacNum*/, &interruptIdx);

    if (rc!=GT_OK)
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvFindInterruptIdx failed=%d", devNum, portNum, rc);
    }

    /* insert to event queue */
    rc = prvCpssDrvEvReqQInsert(evNodesPool, interruptIdx/*evIndex*/, GT_FALSE/*masked boolean*/);
    if (rc != GT_OK)
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssDrvEvReqQInsert failed=%d", devNum, portNum, rc);
    }

    PRV_CPSS_INT_SCAN_UNLOCK();

    return GT_OK;
}

/**
* @internal prvCpssPortXlgMacMaskConfig function
* @endinternal
*
* @brief  Configure XLG mac to enable PCS interrrupts to be raised from the port
*     associated with the given MAC index (which is an additional port in a
*     multi-lane interface and not the first port of the interface), and also,
*     mask every other interrupt on the mac.
*     The operations being done are:
*     (1) configuring mac dma mode field in the mac to allow interrupt to flow.
*     (2) masking all mac interrupts in order to get only PCS interrupts from
*     the port.
*     This function acts as a work-around in order to get all PCS interrupts
*     for every serdes in a given multi-lane port.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] restore         - whether or not to XLG mac state to previous state
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
GT_STATUS prvCpssPortXlgMacMaskConfig
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              restore
)
{
    GT_U32   regAddr;
    GT_U32   regValue;
    GT_U32   i;
    GT_U32   portGroupId;
    GT_STATUS  rc;
    GT_U32   portMacNum;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;
    PRV_CPSS_PORT_REG_CONFIG_STC  regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
#if 0
    CPSS_EVENT_MASK_SET_ENT     masked;
#endif
    MV_HWS_PORT_STANDARD      portMode;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    CPSS_PORT_SPEED_ENT       speed;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32             totalMacs;

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortXlgMacMaskConfig not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* getting appropriate mac num for the given physical port num */
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

    /* getting port group id */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);

    /* getting port speed and interface mode */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc(devNum, portNum, &ifMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppIfModeGetFromExtFunc failed=%d ", devNum, portNum, rc);
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc(devNum, portNum, &speed);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] Calling ppSpeedGetFromExtFunc failed=%d ", devNum, portNum, rc);
    }

    /* checking port legality */
    if ( ifMode == CPSS_PORT_INTERFACE_MODE_NA_E || speed == CPSS_PORT_SPEED_NA_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port illegal speed or interface mode"
                                                   "maybe not needed now", devNum, portNum);
    }

    /* getting hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d",devNum, portNum, rc );
    }

    /* getting hws params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroupId , portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null ", devNum, portNum );
    }

    totalMacs = curPortParams.numOfActLanes;

    /* clear mac configuration array */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigClearFunc(&regDataArray[0]);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling ppMacConfigClearFunc on port returned %d ", rc);
    }

#if 0
    /* get the current mask configuration on the port. If application masked the port,
      no need to unmask the neighbous ports assosiated with the interface */
    rc = cpssEventDeviceMaskWithEvExtDataGet(devNum, CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                             portNum, &masked);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling cpssEventDeviceMaskWithEvExtDataGet failed=%d ", portNum, rc);
    }
/* code currently not needed */
    /* if port is masked, not performing work-around. If port is unmasked
      (and already been created while port was unmasked and WA was not
      performed) then performing work-around.

      general cases:
                  |   UNMASKED            | MASKED
       ----------------------|-----------------------------------------------------
      1. restore == GT_FALSE | (called upon creation of general  | not performing WA when
                  |unmasking operation and port is   | port is masked.
                  | already created)          |
                  | => performing WA          |
                  |                  |
      -----------------------|-----------------------------------------------------------------------------
      3. restore == GT_TRUE  | performing restore WA       | (A) if WA performed in port life-cycle
                  |                  | than restoring WA (B) else => not
                  |                  | performing restore WA
      -----------------------------------------------------------------------------------------------------
    */
    if ( (masked == CPSS_EVENT_MASK_E && restore == GT_FALSE) ||
         (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag==0 &&
          restore == GT_TRUE && masked == CPSS_EVENT_MASK_E)
       )
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortXlgMacMaskConfig: masking condition not met, returning. ");
        return GT_OK;
    }
#endif
    if (restore == GT_TRUE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"handle XLG - restoring values  totalMacs %d", totalMacs);
        for (i = 1; i < totalMacs; i++)
        {
            /* verify allocation */
            PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum+i, 1);

            /* getting xlg mac mask register address */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppXlgMacMaskAddrGetFunc(devNum, portMacNum+i, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] handle xlg macs failed calling funcXlgMacIntMaskGet ", devNum, portNum);
            }

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] regAddr unused", devNum, portNum);
            }

            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %d/%2d] handle XLG - restoring first value: 0x%08x, mac %d",devNum, portNum,
                                                      tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[0], (portMacNum+i));
            /* setting previous xlg mac interrupt mask register values (restoring) */
            rc = prvCpssDrvHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[0]);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] handle xlg macs failed calling prvCpssDrvHwPpWriteRegister ", devNum, portNum);
            }


            /* setting previous mac dma mode value (restoring ) */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacDmaModeRegAddrGetFunc(devNum, (portMacNum+i)/*, PRV_CPSS_PORT_XLG_E*/, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppMacDmaModeRegAddrGetFunc failed %d ", devNum, portNum, rc);
            }

            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %d/%2d] handle XLG - restoring second value: 0x%08x, mac %d",devNum, portNum,
                                                      tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[1], (portMacNum+i));
            if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[1];
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 12;
            }

            /* performing mac configuration */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigurationFunc(devNum, portNum, &regDataArray[0]);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppMacConfigurationFunc failed- %d", devNum, portNum, rc);
            }

            /* marking WA flag */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag = 0;
        }
    }
    else
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"handle XLG - NOT restoring values totalMacs %d", totalMacs);
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum, 1);

        if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag==1)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"handle XLG - NOT restoring values - already loaded. returning ");
            return GT_OK;
        }

        for (i = 1; i < totalMacs; i++)
        {
            /* verify allocation */
            PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum+i, 1);

            /* getting xlg mac mask register address */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppXlgMacMaskAddrGetFunc(devNum, portMacNum + i, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] handle xlg macs failed calling funcXlgMacIntMaskGet ", devNum, portNum);
            }

            if (PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] regAddr unused", devNum, portNum);
            }

            /* getting xlg mac interrupt mask register value */
            rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] handle xlg macs failed calling prvCpssDrvHwPpReadRegister ", devNum, portNum);
            }

            /* saving xlg mac interrupt mask register value in order to save for later restore */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[0] = regValue;
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %d/%2d] handle XLG - NOT restoring values - first Value: 0x%08x,",
                                                      " mac %d, regAddr: 0x%08x",
                                                      devNum, portNum, regValue, (portMacNum+i), regAddr);

            /* overriding xlg mac interrupt mask register value - mask all. Only pcs we want
              (controlled by xlg summary register - another register)*/
            rc = prvCpssDrvHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, 0);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] handle xlg macs failed calling prvCpssDrvHwPpWriteRegister ", devNum, portNum);
            }

            /* getting mac dma mode register address in order to get interrupts */
            /*regAddr = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacDmaModeRegAddrGetFunc(devNum, (portMacNum+i), PRV_CPSS_PORT_XLG_E);*/
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacDmaModeRegAddrGetFunc(devNum, (portMacNum+i)/*, PRV_CPSS_PORT_XLG_E*/, &regAddr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppMacDmaModeRegAddrGetFunc #2 failed %d ", devNum, portNum, rc);
            }

            /* getting mac dma mode value in order to save for later restore */
            rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),portGroupId, regAddr,
                                                    12, 1, &regValue);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] handle xlg macs failed calling prvCpssDrvHwPpPortGroupGetRegField ", devNum, portNum);
            }

            /* saving mac dma mode value */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+i]->xlgMacDb[1] = regValue;
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %d/%2d] handle XLG - NOT restoring values - second Value: 0x%08x, mac %d",
                                                      devNum, portNum, regValue, (portMacNum+i));

            /* overriding mac dma mode value to get interrupts */
            if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = 0;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
                regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 12;
            }

            /* performing mac configuration */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigurationFunc(devNum, portNum, &regDataArray[0]);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppMacConfigurationFunc failed- %d", devNum, portNum, rc);
            }

            /* marking WA flag */
            tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum]->xlgMacDbFlag = 1;

        }
    }

    return GT_OK;

}

/**
* @internal prvCpssPortStageTrainPerformPhase function
* @endinternal
*
* @brief  API that will check the serdes auto tune and return is status
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
*
* @param[out] tuneStatusPtr  - return tune status
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
static GT_STATUS prvCpssPortStageTrainPerformPhase
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
)
{

    GT_STATUS rc;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT tmpTuneStatus;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    /************************************************
        Training Results
    ************************************************/
    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;

    rc = cpssPortSerdesCyclicAutoTuneStatusGet(CAST_SW_DEVNUM(devNum), portNum,
                                               (tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune), &tmpTuneStatus);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"cpssPortSerdesCyclicAutoTuneStatusGet failed %d ", rc );
        return rc;
    }

    /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Training check, status: %d ", portNum, tmpTuneStatus);*/
    /* we will perform next training mode only if we past previous one */
    if (tmpTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_PASS_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling tune2results on port. Training check and next. tune %d. Phase %d. PASSED ",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.phase);

        if (1/*tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune != CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_LAST_E*/)
        {
            /*PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC *ptrR = trainCookiePtrEnclosingDB.cookiePtr;*/
            /* performing next training */
            /*rc = cpssDxChPortSerdesCyclicAutoTune(devNum, portNum,
                               &tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB);*/

            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.overrideEtParams =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->overrideEtParams;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.max_LF =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->max_LF;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.min_LF =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->min_LF;

            rc = cpssPortSerdesCyclicAutoTune(CAST_SW_DEVNUM(devNum), portNum,
                                              &tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB);

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"launched tune: %d ", tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune);

            if (rc != GT_OK)
            {
                /* overall status failed */
                *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
                /*  CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Calling auto tune port %d returned error code %d ",
                               portNum, rc);*/
            }
            else
            {
                /* overall status not yet finished */
                *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E;
            }
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune
                == CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
            {
                *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
            }
        }
        else
        {
            /* overall status finished successfully */
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
        }

    }
    else if (tmpTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling tune2results. Training check and next. tune %d. Phase %d. FAILED",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cyclicTune,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.phase);
        /* overall status failed */
        *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
    }
    else
    {
        /* overall status not yet finished */
        *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E;
    }

    return GT_OK;
}

/**
* @internal  prvCpssPortStageTrainMngPhases function
* @endinternal
*
* @brief  API that will check the serdes auto tune and according
*         to the status it's will return if the train is done
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
*
* @param[out] tuneStatusPtr  - return tune status
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
static GT_STATUS prvCpssPortStageTrainMngPhases
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
)
{
    /* Variables declarations */
    GT_STATUS  rc;
    GT_U32 timerSecondsNow, timerNsecNew;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    rc = prvCpssPortStageTrainPerformPhase(devNum, portNum, tuneStatusPtr);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortStageTrainPerformPhase returned %d", rc);
        return rc;
    }
    /**************************************************************************************************
       If training passed, check port statuses good then enabling port. Else, not advancing
    **************************************************************************************************/
    /* Train pass */
    if (CPSS_PORT_SERDES_AUTO_TUNE_PASS_E == *tuneStatusPtr)
    {
        /* Checking overall port statuses */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"training passed ");

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_DONE_E;

        return GT_OK;
    }
    /***********************************************************************
        3. If training failed, not advancing
    ***********************************************************************/
    /* Train fail */
    else if (CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E == *tuneStatusPtr)
    {
        /* Thats it. returning to wait for signal detection */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"training failed on port");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_RESET_E;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;

        return GT_OK;
    }

    /***********************************************************************
        4. If training timed-out, not advancing
    ***********************************************************************/
    /* Not ready - continue to poll */
    else
    {
        /* Check Timeout */
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] >= 2)
        {
            /* Thats it. returning to wait for signal detection */
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"training timed-out");
            /** NEED TO ENSURE ANY TRAINING MODE WILL BE STOPPED NOW */
            /*appDemoPortInitSeqDisablePortStage(devNum, portNum);*/
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;

            /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(configureInputParams->portsBmpPtr, portNum);*/
            *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_RESET_E;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;

            return GT_OK;
        }
    }


    /* Initialize timers if this is the first iteration in the algorithm */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev==0 &&
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev==0)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev,
                          &tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] FATAL timer TrainRes #1 ", devNum, portNum);
        }
    }

    rc = cpssOsTimeRT(&timerSecondsNow,&timerNsecNew);
    if (rc != GT_OK)
    {
        /* Thats it. returning to wait for signal detection */
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"training timed-out TrainRes");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = 0;

        *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;
    }

    if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime =
        ( 1000000000 - (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev) + timerNsecNew );
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime =
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime;
    }
    /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime =
        ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev) );
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerSecondsPrev = timerSecondsNow;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimerNsecPrev = timerNsecNew;

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0]+=
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime;

    if(tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] > (4000000000UL))
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[0] = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultCounter.l[1]++;
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer +=
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeDiffFromLastTime;

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer >= 10000000)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries++;
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerEnableRemoteLink
(
    GT_SW_DEV_NUM devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT      speed,
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr,
    GT_BOOL enable
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_PORT_STANDARD  portMode;

    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;

    /* First make sure that not removing RF for LKB port, as it is done by LKB FW */
    if ( (tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled == GT_FALSE) || (enable == GT_FALSE) )
    {
        if(CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode || CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
        {
            /* XAUI/RXAUI use XPCS which does not implement remote fault */
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppRemoteFaultSetFunc was skipped since XPCS is used");
        }
        else if ((!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) &&
                 (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb))) &&
                 !PRV_CPSS_SIP_6_CHECK_MAC(devNum)) /* for 100G/MTI we set local fault */
        {
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault == GT_FALSE)
            {

                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemoteFaultSetFunc(devNum, portNum, ifMode, speed, (enable == GT_TRUE) ? GT_FALSE : GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppRemoteFaultSetFunc returned error code %d ", rc);
                    return rc;
                }
            }

        }
    }
    /* Force link down */
    if (!HWS_USX_MULTI_LANE_MODE_CHECK(portMode))
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppForceLinkDownSetFunc(devNum, portNum, (enable == GT_TRUE) ? GT_FALSE : GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"disabling force link down returned error code %d ", rc);
            return rc;
        }
    }
    return rc;
}

/**
* @internal prvCpssPortManagerEnableLinkLevel
* @endinternal
*
* @brief  perform enable/disable link with force down and mac
*         enable set to the port
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum              - device number
* @param[in] portNum             - physical port number
* @param[in] ifMode              - port interface mode
* @param[in] speed               - port speed
* @param[in] enable              - enable/disable link
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PTR        - on bad pointer
*/
GT_STATUS prvCpssPortManagerEnableLinkLevel
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed,
    IN GT_BOOL                         enable
)
{
    GT_STATUS rc,rc_tmp;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_BOOL anyLb;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;
    GT_BOOL  macEnableStatus;

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Performing port-enable stage %s ", (enable == GT_TRUE) ? "enable" : "disable");

    rc = ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortManagerDbGetFunc(CAST_SW_DEVNUM(devNum), &tmpPortManagerDbPtr)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Could not get portManagerDbPtr",devNum, portNum);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    rc = prvCpssPortManagerLoopbackModesDbGet(devNum,portNum,CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E,&loopbackCfg, &anyLb);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   devNum, portNum, rc);
    }

    /* currently, we will only need 'anyLoopback' case and not per loopback mode so just avoid warning for now*/

    /* in loopback mode we will not force link-down nor force peer-link-down and we will enable mac */
    if ((anyLb == GT_TRUE) &&
        !PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "not performing link-level enabled-disable as there is a loopback on port");
        return GT_OK;
    }
    /*********************************************************
      Enable port - removing force downs and enabling port
     ********************************************************/
    rc = prvCpssPortManagerEnableRemoteLink(devNum,portNum,ifMode,speed,tmpPortManagerDbPtr,enable);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerEnableRemoteLink returned error code %d ", rc);
        return rc;
    }

    if(PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg))
    {
        if (enable)
        {
            if (loopbackCfg.enableRegularTrafic == GT_FALSE)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Port stay disabled because SERDES_LOOPBACK_DIGITAL_RX2TX mode");
                return GT_OK;
            }
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        if(tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM == GT_TRUE && enable == GT_TRUE)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"skip enabling port enable=portOperDisableSM=%d ", enable);
            return GT_OK;
        }
    }

    /* Enable port */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E && enable == GT_FALSE)
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableGetFunc(devNum, portNum, &macEnableStatus);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
            return rc;
        }

        /*
             For SIP6.0 in order to perform port delete WA when port
             already in disabled state it's needed to enable/disable it to clean port queue.
             hint: this method also apper in prvCpssDxChPortFalconModeSpeedSet, any change here will need also change there.
        */
        if (macEnableStatus == GT_FALSE)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                return rc;
            }
        }
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, enable);
    if (rc != GT_OK)
    {
        if (enable == GT_FALSE)
        {
            rc_tmp = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
            if (rc_tmp != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc_tmp);
            }
            rc_tmp = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
            if (rc_tmp != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc_tmp);
            }
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
            return rc;
        }
    }

    return GT_OK;
}

static GT_STATUS prvCpssPortManagerModeToDownState
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_U32   portGroup;
    GT_STATUS rc;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    GT_U32                          result = 0;
    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "move to down");
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal = GT_TRUE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkFailCnt++;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkUpTime = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.alignLockTime = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.rxTrainingTime = 0;
    CPSS_PM_START_TIME(tmpPortManagerDbPtr->portMngSmDb[portNum]);

    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

    if(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc != NULL && PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                         portGroup, ifMode, speed,
                                                                                         MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E, &result);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform reset rxtx operation", devNum, portNum);
        }
    }

    if ((PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
    {
         CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"stop Rx training (mode to down)");
         rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc(devNum, portNum, CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_STOP_E);

         if (rc != GT_OK)
         {
             CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppTuneExtSetFunc failed=%d ",
                                                        devNum, portNum, rc);
         }
    }


#if 0
    if ( PRV_CPSS_SIP_6_CHECK_MAC(devNum) )
    {
        rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_FALSE);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                       " returned error code %d ",
                                                       devNum, portNum, rc);
        }
    }
#endif
    rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
    }
    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
    }
    return GT_OK;
}

/**
* @internal  prvCpssPortStatusesCheck function
* @endinternal
*
* @brief  API that will check the port status according the bitmap.
*         the statuses:
*          - alignLock
*          - gear box lock
*          - signal detect
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note  NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum            - device number
* @param[in] portNum           - physical port number
* @param[in] unitsBitmap       - bitmap to witch status to check
*
* @param[out] portStatusesPtr  - return tune status
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
static GT_STATUS prvCpssPortStatusesCheck
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC  *portStatusesPtr,
    IN  GT_U32                                   unitsBitmap
)
{
    GT_STATUS                               rc;
    GT_BOOL                                 anyLb;
    CPSS_PORT_INTERFACE_MODE_ENT            ifModeDb;
    PRV_CPSS_PORT_MNG_DB_STC                *tmpPortManagerDbPtr;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC   loopbackCfg;
    MV_HWS_PORT_INIT_PARAMS                 curPortParams;
    MV_HWS_PORT_STANDARD                    portMode;
    GT_U32                                  portGroup;

    GT_U32 portMacNum = 0;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    ifModeDb = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portGroup = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;

    rc = prvCpssPortManagerLoopbackModesDbGet(devNum,portNum,CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E,&loopbackCfg, &anyLb);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   devNum, portNum, rc);
    }

    portStatusesPtr->gbLock    = GT_FALSE;
    portStatusesPtr->sigDet    = GT_FALSE;
    portStatusesPtr->alignLock = GT_FALSE;
    portStatusesPtr->overallStatusesOk = GT_FALSE;

    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                   "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
    }

    /**********************************************************
      Serdes Signal Detect, PCS GB lock, PCS align lock
    **********************************************************/
    if(PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg) &&
       (loopbackCfg.enableRegularTrafic == GT_FALSE))
    {   /* no need to check pcs and mac*/
        portStatusesPtr->gbLock = GT_TRUE;
        portStatusesPtr->alignLock = GT_TRUE;
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "gbLock and alignLock are ignored because loopback mode");
    }
    else
    {
        if (unitsBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E))
        {
            /* CG MAC / MTI_USX_PCS_LOW_SPEED / MTI_CPU_SGPCS don't have GearBox lock */
            if (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb) ||
                 (curPortParams.portPcsType == MTI_USX_PCS_LOW_SPEED) ||
                 (curPortParams.portPcsType == MTI_CPU_SGPCS)))
            {
                if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    rc = prvCpssPortPcsGearBoxStatusGet(CAST_SW_DEVNUM(devNum), portNum, portMacNum, &(portStatusesPtr->gbLock));
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortGbStatusGetFunc:rc=%d\n",
                                                                   devNum, portNum, rc);
                    }
                }
                else
                {
                    /* in case of RS_FEC/ FEC_OFF with R2 we don't have GearBox lock */
                    if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_RS_FEC_MODE_ENABLED_E ||
                          tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_FEC_MODE_DISABLED_E) &&
                         PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(ifModeDb))
                    {
                        portStatusesPtr->gbLock = GT_TRUE;
                    }
                    else
                    {
                        /* rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortGbStatusGetFunc(devNum, portNum, &(portStatusesPtr->gbLock));*/
                        rc = prvCpssPortPcsGearBoxStatusGet(CAST_SW_DEVNUM(devNum), portNum, portMacNum, &(portStatusesPtr->gbLock));
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortGbStatusGetFunc:rc=%d\n",
                                                                       devNum, portNum, rc);
                        }
                    }
                }
            }
            else
            {
                portStatusesPtr->gbLock = GT_TRUE;
            }
        }

        if (unitsBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E))
        {
            /* in single lane no need to check for align lock */
            if (curPortParams.numOfActLanes > 1)
            {
                rc = prvCpssPortPcsAlignLockStatusGet(CAST_SW_DEVNUM(devNum), portNum, portMacNum, &(portStatusesPtr->alignLock));
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortPcsAlignLockStatusGet:rc=%d",
                                                               devNum, portNum,rc);
                }
            }
            else
            {
                 portStatusesPtr->alignLock = GT_TRUE;
            }
        }
    }

    if (unitsBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E))
    {
        /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"getting PRV_CPSS_PORT_MANAGER_SIG_DET_E status");*/
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSigDetGetFunc(devNum, portNum, &(portStatusesPtr->sigDet));
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppSigDetGetFunc(PRV_CPSS_PORT_MANAGER_SIG_DET_E): rc=%d\n",
                                                       devNum, portNum, rc);
        }
    }

    if (portStatusesPtr->gbLock    == GT_TRUE &&
        portStatusesPtr->sigDet    == GT_TRUE &&
        portStatusesPtr->alignLock == GT_TRUE)
    {
        portStatusesPtr->overallStatusesOk = GT_TRUE;
    }

    return GT_OK;
}

static GT_STATUS prvCpssPortManagerStatusesStabilityCheckPhase
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC  *portStatusesPtr,
    IN  GT_U32                                   unitsBitmapPtr,
    IN  GT_U32                                   totalPcs
)
{
    GT_STATUS rc;
    GT_U32 SD_Change_Interval;
    GT_U32 SD_Change_Min_Window_Size;
    GT_U32 SD_Check_Time_Expired;
    GT_U32 timerNsecNew;
    GT_U32 timerSecondsNow;
    GT_U32 i;
    GT_U8 flag;
    CPSS_UNI_EV_CAUSE_ENT uniEv;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_BOOL signalOkChange = GT_FALSE;
    MV_HWS_PORT_STANDARD      portMode;
    GT_U32                    portGroup;

    GT_U32 portMacNum = 0;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portGroup = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;

    flag = 0;
    cpssOsMemSet(portStatusesPtr, 0, sizeof(PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC));

    if (unitsBitmapPtr & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E))
    {
        SD_Change_Interval = tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeInterval;
        SD_Change_Min_Window_Size = tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeMinWindowSize;
        SD_Check_Time_Expired = tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdCheckTimeExpired;

        uniEv = 0;

        for (i = 0; i < totalPcs; i++)
        {
            if (PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
            {
                uniEv = CPSS_PP_PORT_SYNC_STATUS_CHANGED_E;
            }
            else
            {
                uniEv = CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E;
            }
        }


        /* Initialize timers if this is the first iteration in the algorithm */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev==0 &&
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev==0)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"signal stability numLanes %d - START", totalPcs);
            if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
            {
                signalOkChange = GT_FALSE;
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"signal stability stop (sip 6)");
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = CPSS_PORT_MANAGER_DONE_E;
                 /* FINISHED :)) */
                 portStatusesPtr->sigDet = GT_TRUE;
                 return GT_OK;
            }
            else
            {
                /* First time: clear cause bits.  */
                rc = mvHwsPortAvagoSerdesRxSignalOkChangeGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum,portMode,&signalOkChange);
                if(rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling mvHwsPortAvagoSerdesRxSignalOkChangeGet failed %d ", portMacNum, rc);
                }
            }

            tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart  = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer   = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
            rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev,
                              &tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev);
            if (rc!=GT_OK)
            {
                if (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
                {
                    rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum), uniEv, portNum, CPSS_EVENT_UNMASK_E);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet failed=%d ", devNum, portNum, rc);
                    }
                }
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Unmasking event %d as rollback failed #1", devNum, portNum, uniEv);
            }
        }

        /* Get current time, and update port diff-timer and total-time-from-start timer accordingly*/
        rc = cpssOsTimeRT(&timerSecondsNow, &timerNsecNew);
        if (rc!=GT_OK)
        {
            if (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
            {
                rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum), uniEv, portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet failed=%d ", devNum, portNum, rc);
                }
            }
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Unmasking event %d as rollback failed #2", devNum, portNum, uniEv);
        }
        /* In case we in different time in meanning of a second, the difference in nano seconds
          is the following equation */
        if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = ( (1000000000 - tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev) + timerNsecNew );
        }
        /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
        else
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime =
              ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev) /*/ 1000000*/ );
        }

        /* update the port seconds and nanosecs timers */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = timerSecondsNow;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = timerNsecNew;

        /* Intervals timer */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer +=
          tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime;

        /* updating total time spent */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart +=
          tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange +=
          tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime;

        /* Check if interval of recheck reached */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer >= SD_Change_Interval*1000000)
        {
            CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %d/%2d] interval passed. Time diff from last interval: %d ", devNum, portNum,
                               tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer);
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer =0;
            if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
            {
                signalOkChange = GT_FALSE;
            }
            else
            {
                rc = mvHwsPortAvagoSerdesRxSignalOkChangeGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum,portMode,&signalOkChange);
                if(rc != GT_OK)
                {
                    if (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
                    {
                        rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum), uniEv, portNum, CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling "
                                      "cpssEventDeviceMaskWithEvExtDataSet returned=%d ", devNum, portNum, rc);
                        }
                    }
                    /* Reset timers */
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = CPSS_PORT_MANAGER_RESET_E;
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[MacPort %d] Calling mvHwsPortAvagoSerdesRxSignalOkChangeGet failed %d ", portMacNum, rc);
                }
            }
            if (signalOkChange == GT_TRUE)
            {
                flag = 1;
            }

            if (flag == 1)
            {
                /* there was a change continue measure */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
            }

            /* Finished measure, signal is stable enought */
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange >= SD_Change_Min_Window_Size*1000000)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"signal stability - FINISHED time(%dms)",(tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange/1000000));

                /* Reset timers */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = CPSS_PORT_MANAGER_DONE_E;
                /* FINISHED :)) */
                portStatusesPtr->sigDet = GT_TRUE;
                return GT_OK;
            }

            /* If we are timed-out */
            if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart >= SD_Check_Time_Expired*1000000) && (!PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) || CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E == ifMode))
            {
                if (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
                {
                    rc = cpssEventDeviceMaskWithEvExtDataSet(CAST_SW_DEVNUM(devNum), uniEv, portNum, CPSS_EVENT_UNMASK_E);
                    if(rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling "
                                "cpssEventDeviceMaskWithEvExtDataSet returned=%d ", devNum, portNum, rc);
                    }
                }

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Overall time expiration: %dms ", (tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart/1000000));
                /* Reset timers */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer=0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = CPSS_PORT_MANAGER_RESET_E;
                /* FINISHED :(( HW ERROR */
                portStatusesPtr->sigDet = GT_FALSE;
                return GT_OK;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerSigStableRxTraining function
* @endinternal
*
* @brief  Runs Rx training in order to stabilize signal detection.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
*
* @retval GT_OK                - on success
* @retval GT_FAIL              - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM         - on bad physical port number
*/
GT_STATUS prvCpssPortManagerSigStableRxTraining
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS                            rc;
    PRV_CPSS_PORT_MNG_DB_STC*            tmpPortManagerDbPtr;
    CPSS_PORT_INTERFACE_MODE_ENT         ifModeDb;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT  rxTrainMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC        systemRecoveryInfo;

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"device not applicable for prvCpssPortManagerSigStableRxTraining. Returning.");
        return GT_OK;
    }

    rc = cpssSystemRecoveryStateGet(&systemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Skip training if in a middle of recovery*/
    if(CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == systemRecoveryInfo.systemRecoveryProcess)
    {
        /* getting port manager database*/
        PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

        /* TBD harrier */
        if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
           if(cpssDeviceRunCheck_onEmulator() )
           {
              tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
              return GT_OK;
           }
        }

        ifModeDb = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;

        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

        rxTrainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E))
        {
             rxTrainMode = tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideTrainMode;
        }
        if ( /*Avago SerDes*/(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))) &&
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility == CPSS_PORT_FEC_MODE_DISABLED_E) && (PRV_CPSS_PORT_MANAGER_TWO_LANES_MODE_CHECK(ifModeDb)) &&
            (rxTrainMode == CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E))

        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"- Launch iCal -");
            rc = prvCpssPortSerdesLunchSingleiCal(CAST_SW_DEVNUM(devNum), portNum);
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"- Start Training -");
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc(devNum, portNum, rxTrainMode);
        }

        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortStageTrainMngPhases failed=%d ",
                                                       devNum, portNum, rc);
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
    }

    return GT_OK;

}

/**
* @internal prvCpssPortManagerLoopbackModesHwSet function
* @endinternal
*
* @brief  Set loopback mode/s of the port to the HW.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum              - device number
* @param[in] portNum             - physical port number
* @param[in] ifMode              - port interface mode
* @param[in] speed               - port speed
* @param[in] loopbackCfgPtr      - (pointer to) loopback configuration structure
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PTR        - on bad pointer
*/
GT_STATUS prvCpssPortManagerLoopbackModesHwSet
(
    IN GT_SW_DEV_NUM                          devNum,
    IN GT_PHYSICAL_PORT_NUM                   portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT           ifMode,
    IN CPSS_PORT_SPEED_ENT                    speed,
    IN CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC  *loopbackCfgPtr
)
{
    GT_U32 laneBmp, laneIdx;
    GT_STATUS rc;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtr);

    switch (loopbackCfgPtr->loopbackType)
    {
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
        rc = ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacLoopbackModeSetFunc(devNum, portNum,
                                                                                             loopbackCfgPtr->loopbackMode.macLoopbackEnable)));
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Could not set MAC Loopback", devNum, portNum);
        }
        /* WA to raise link change interrup on MAC loopback */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
             CPSS_PORT_SERDES_LOOPBACK_MODE_ENT  serdesLoopbackMode;

             serdesLoopbackMode = (loopbackCfgPtr->loopbackMode.macLoopbackEnable == GT_TRUE) ?
                                      CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E : CPSS_PORT_SERDES_LOOPBACK_DISABLE_E;
             /* Not expected to receive low level events */
             rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
             if (rc != GT_OK)
             {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
             }

             /* applying serdes loopback on all applicable local lanes */
             rc = ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesLoopbackModeSetFunc(devNum, portNum,
                                                                                             0xff,
                                                                                             serdesLoopbackMode)));

             if (rc!=GT_OK)
             {
                 CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortSerdesLoopbackModeSetFunc failed=%d",
                                                            devNum, portNum, rc);
             }
        }
        break;
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
        laneBmp = 0;
        rc = prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed(devNum,0,portNum,ifMode,speed,&curPortParams);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                                                       "[Port %d/%2d] prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed failed=%d",
                                                       devNum, portNum, rc);
        }

        /* filling local lanes bitmap */
        for (laneIdx=0; laneIdx<curPortParams.numOfActLanes; laneIdx++)
        {
            {
                laneBmp |= (1<<laneIdx);
            }
        }

        /* applying serdes loopback on all applicable local lanes */
        rc = ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesLoopbackModeSetFunc(devNum, portNum,
                                                                                                laneBmp,
                                                                                                loopbackCfgPtr->loopbackMode.serdesLoopbackMode)));

        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortSerdesLoopbackModeSetFunc failed=%d",
                                                       devNum, portNum, rc);
        }
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                                   "[Port %d/%2d] setting HW loopback failed type=%d",
                                                   devNum, portNum, loopbackCfgPtr->loopbackType);
    }

    return GT_OK;
}


/**
* @internal prvCpssPortManagerLanePolarityHwSet function
* @endinternal
*
* @brief  set serdes lane rx and tx polarity to hw.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] globalSerdesIndex    - lane num
*                   rxInvert       - rx polarity data
*                   txInvert       - tx polarity data
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
#if 0
GT_STATUS prvCpssPortManagerLanePolarityHwSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U16 globalSerdesIndex
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);

    if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.txInvert == GT_FALSE
        &&
        tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.rxInvert == GT_FALSE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "[SerDes Lane %d] setting serdes polarity: both FALSE so just returning", globalSerdesIndex);
        return GT_OK;
    }

    rc = mvHwsSerdesPolarityConfig(devNum, 0, globalSerdesIndex,
                                   HWS_DEV_SERDES_TYPE(devNum),
                                   tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.txInvert,
                                   tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesPolarity.rxInvert);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[SerDes Lane %d] mvHwsSerdesPolarityConfig failed=%d", globalSerdesIndex, rc);
    }


    return GT_OK;
}
#endif
/**
* @internal prvCpssPortManagerLaneTuneDbGet function
* @endinternal
*
* @brief   Get serdes lane RxTx parameters from port manager data base.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum           - device number
* @param[in] portNum          - port number
* @param[in] advertisedNum    - advertise number
* @param[in] serdesIndex      - serdex index
* @param[in] portType         - port type
* @param[out] serdesParamsPtr - serdes parameters
* @param[out] dbValidBitMapPtr- ValidBitMap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssPortManagerLaneTuneDbGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32 advertisedNum, /* for ap only */
    IN  GT_U32 serdesIndex,
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    OUT CPSS_PORT_SERDES_TUNE_STC *serdesParamsPtr,
    OUT GT_U32                    *dbValidBitMapPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC *serdesDbStcPtr;

    GT_U32 laneSerdes,globalSerdesIndex;

    CPSS_NULL_PTR_CHECK_MAC(serdesParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbValidBitMapPtr);

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        globalSerdesIndex = serdesIndex; /* for non-ap we using gloabl serdes*/
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);
        serdesDbStcPtr = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex];
    }
    else
    {
        laneSerdes = serdesIndex; /* for ap we using lane serdes */
        if (laneSerdes >= PORT_MANAGER_MAX_LANES)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Lane index bigger than max lane");
        }
        serdesDbStcPtr = &tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdes)];
    }

    switch (serdesDbStcPtr->portSerdesParams.serdesCfgDb.type)
    {
    case CPSS_PORT_SERDES_AVAGO_E:
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_AVAGO_E;
                cpssOsMemCpy(&(serdesParamsPtr->txTune.avago),
                         &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.txTune.avago),
                         sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
            }
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_AVAGO_E;
                cpssOsMemCpy(&(serdesParamsPtr->rxTune.avago),
                         &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.rxTune.avago),
                         sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
            }
        break; /* Avago */
    case CPSS_PORT_SERDES_COMPHY_H_E:
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_H_E;
                cpssOsMemCpy(&(serdesParamsPtr->txTune.comphy),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.txTune.comphy),
                          sizeof(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC));
            }
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_H_E;
                cpssOsMemCpy(&(serdesParamsPtr->rxTune.comphy),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.rxTune.comphy),
                         sizeof(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC));
            }
        break; /* Comphy H */
    case CPSS_PORT_SERDES_COMPHY_C12G_E:
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C12G_E;
                cpssOsMemCpy(&(serdesParamsPtr->txTune.comphy_C12G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C12G),
                          sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
            }
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C12G_E;
                cpssOsMemCpy(&(serdesParamsPtr->rxTune.comphy_C12G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G),
                          sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));
            }
        break; /* Comphy C12G */
    case CPSS_PORT_SERDES_COMPHY_C28G_E:
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                cpssOsMemCpy(&(serdesParamsPtr->txTune.comphy_C28G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C28G),
                          sizeof(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC));
            }
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                cpssOsMemCpy(&(serdesParamsPtr->rxTune.comphy_C28G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G),
                          sizeof(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC));
            }
        break; /* Comphy C28G */
    case CPSS_PORT_SERDES_COMPHY_C112G_E:
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C112G_E;
                cpssOsMemCpy(&(serdesParamsPtr->txTune.comphy_C112G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C112G),
                          sizeof(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC));
            }
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C112G_E;
                cpssOsMemCpy(&(serdesParamsPtr->rxTune.comphy_C112G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G),
                          sizeof(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC));
            }
        break; /* Comphy C112G */
        case CPSS_PORT_SERDES_COMPHY_C56G_E:
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C56G_E;
                cpssOsMemCpy(&(serdesParamsPtr->txTune.comphy_C56G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C56G),
                          sizeof(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC));
            }
            if (serdesDbStcPtr->sdParamValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesParamsPtr->type = CPSS_PORT_SERDES_COMPHY_C56G_E;
                cpssOsMemCpy(&(serdesParamsPtr->rxTune.comphy_C56G),
                          &(serdesDbStcPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G),
                          sizeof(CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC));
            }
        break; /* Comphy C56G */
    default:
        break;
    }

    *dbValidBitMapPtr = serdesDbStcPtr->sdParamValidBitMap;

    return GT_OK;
}

/**
* @internal prvCpssPortManagerSerdesValuesCheck function
* @endinternal
*
* @brief   Checking the range of the serdes values.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] dbValidBitMap            - ValidBitMap
* @param[in] tuneValuesPtr            - (pointer to) serdes RxTx parameters
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - values out of range
*/
GT_STATUS prvCpssPortManagerSerdesValuesCheck
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN GT_U32                           dbValidBitMap,
    IN CPSS_PORT_SERDES_TUNE_STC        *tuneValuesPtr
)
{
    GT_STATUS rc = GT_OK;

    switch (tuneValuesPtr->type)
    {
    case CPSS_PORT_SERDES_AVAGO_E:
        if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
        {
            if ( PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->rxTune.avago.sqlch, 310, NA_16BIT) ||
                 PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->rxTune.avago.DC, 255, NA_16BIT) ||
                 PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->rxTune.avago.LF, 15,  NA_16BIT) ||
                 PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->rxTune.avago.HF, 15,  NA_16BIT) ||
                 PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->rxTune.avago.BW, 15,  NA_16BIT))
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port d/%2d] RX param out of range for device", devNum, portNum);
            }

            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {

                if ( PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->rxTune.avago.BFHF, -31, 31, NA_16BIT) ||
                     PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->rxTune.avago.BFLF, -31, 31, NA_16BIT))
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] RX param out of range for device", devNum, portNum);
                }
            }
        }
        if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
        {
            if (PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.avago.atten, 31, NA_16BIT) ||
                PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.avago.post, -31, 31, NA_16BIT)||
                PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.avago.pre, -31, 31, NA_16BIT))
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] TX param out of range for device", devNum, portNum);
            }

            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {

                if( PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.avago.pre2, -15, 15, NA_16BIT) ||
                    PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.avago.pre3, -1, 1, NA_16BIT))
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] TX param out of range for device", devNum, portNum);
                }
            }
        }
        break; /* Avago */
    case CPSS_PORT_SERDES_COMPHY_H_E:
        if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
        {
            if (tuneValuesPtr->rxTune.comphy.ffeC     >= BIT_4 ||
                tuneValuesPtr->rxTune.comphy.ffeR     >= BIT_3 ||
                tuneValuesPtr->rxTune.comphy.align90   >= BIT_7)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] non sip5_15 out of range RX", devNum, portNum);
            }
        }
        if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
        {
            /* for now just these parameters supported in Lion2 */
            if (tuneValuesPtr->txTune.comphy.emph0  >= BIT_4 ||
                tuneValuesPtr->txTune.comphy.emph1   >= BIT_4 ||
                tuneValuesPtr->txTune.comphy.txAmp    >= BIT_5 ||
                tuneValuesPtr->txTune.comphy.txAmpAdj   >= BIT_1)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] non sip5_15 out of range TX", devNum, portNum);
            }
        }
        break; /* Comphy H */
    case CPSS_PORT_SERDES_COMPHY_C12G_E:
        if (PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.comphy_C12G.post, 0xE, NA_16BIT)  ||
            PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.comphy_C12G.pre,  0xE, NA_16BIT)  ||
            PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.comphy_C12G.peak, 0xF, NA_16BIT))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] TX param out of range for device", devNum, portNum);
        }
        break; /* Comphy C12G */
    case CPSS_PORT_SERDES_COMPHY_C28G_E:
        if (PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.comphy_C28G.post, 0xF, NA_16BIT)  ||
            PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.comphy_C28G.pre,  0xF, NA_16BIT)  ||
            PRV_PORT_MANAGER_SERDES_PARAMS_CHECK(tuneValuesPtr->txTune.comphy_C28G.peak, 0xF, NA_16BIT))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] TX param out of range for device", devNum, portNum);
        }
        break; /* Comphy C28G */
    case CPSS_PORT_SERDES_COMPHY_C112G_E:
        if (PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C112G.pre2, -9,  9,   NA_16BIT) ||
            PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C112G.pre,  -22, 22,  NA_16BIT) ||
            PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C112G.post, -35, 35,  NA_16BIT) ||
            PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C112G.main,  0,  63,  NA_16BIT) ||
            (ABS(tuneValuesPtr->txTune.comphy_C112G.pre2 + tuneValuesPtr->txTune.comphy_C112G.pre + tuneValuesPtr->txTune.comphy_C112G.post + tuneValuesPtr->txTune.comphy_C112G.main) > 63))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] TX param out of range for device", devNum, portNum);
        }
        break; /* Comphy C112G */
    case CPSS_PORT_SERDES_COMPHY_C56G_E:
        if (PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C56G.pre2, -9,  9,  NA_16BIT) ||
            PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C56G.pre,  -22, 22, NA_16BIT) ||
            PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C56G.post, -22, 22, NA_16BIT) ||
            PRV_PORT_MANAGER_SERDES_PARAMS_SIGNED_CHECK(tuneValuesPtr->txTune.comphy_C56G.main, 35,  63, NA_16BIT) ||
            (ABS(tuneValuesPtr->txTune.comphy_C56G.pre2 + tuneValuesPtr->txTune.comphy_C56G.pre + tuneValuesPtr->txTune.comphy_C56G.post + tuneValuesPtr->txTune.comphy_C56G.main) > 63))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] TX param out of range for device", devNum, portNum);
        }
        break; /* Comphy C56G */
        default:
        break;
    }

    return rc;
}

/**
* @internal prvCpssPortManagerLaneTuneDbSet function
* @endinternal
*
* @brief   Set serdes lane RxTx parameters in port manager data base.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] advertisedNum            - advertise number
* @param[in] SerdesIndex              - global/lane serdex index
* @param[in] portType                 - AP or Non-AP port type
* @param[in] dbValidBitMap            - which parameters are
*       Valid
* @param[in] tuneValuesPtr            - (pointer to) serdes RxTx parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssPortManagerLaneTuneDbSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN GT_U32                           advertisedNum, /* for ap only */
    IN GT_U32                           serdesIndex,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT  portType,
    IN GT_U32                           dbValidBitMap,
    IN CPSS_PORT_SERDES_TUNE_STC        *tuneValuesPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC *serdesDbPtr;
    GT_U32 globalSerdesIndex=0,laneSerdes=0;
    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* checking if the serdes values are in range*/
    rc = prvCpssPortManagerSerdesValuesCheck(devNum,portNum,dbValidBitMap,tuneValuesPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        globalSerdesIndex = serdesIndex; /* for non-ap we using gloabl serdes*/
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);
        serdesDbPtr = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex];
    }
    else /* ap */
    {
        laneSerdes = serdesIndex; /* for ap we using lane serdes */
        if (laneSerdes > PORT_MANAGER_MAX_LANES)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] Lane index bigger than max lane", devNum, portNum);
        }

        serdesDbPtr = &tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdes)];
    }

    switch (tuneValuesPtr->type)
    {
    case CPSS_PORT_SERDES_AVAGO_E:
            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_AVAGO_E;
                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.rxTune.avago),
                             &(tuneValuesPtr->rxTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));

                serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
            }

            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                /* tx params */
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_AVAGO_E;
                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.txTune.avago),
                             &(tuneValuesPtr->txTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
                serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
            }
            break;
    case CPSS_PORT_SERDES_COMPHY_H_E:
            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                /* ctle params */
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_H_E;

                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.rxTune.comphy),
                             &(tuneValuesPtr->rxTune.comphy),
                             sizeof(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC));

                serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
            }

            /* 'sampler' field - not for cpss_4.2 - not argument in params set in HWS,
              only ffeC, ffeR, sqlch, dfeEn=TRUE, align90 */
            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_H_E;
                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.txTune.comphy),
                             &(tuneValuesPtr->txTune.comphy),
                             sizeof(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC));
                serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
            }
            break;
    case CPSS_PORT_SERDES_COMPHY_C12G_E:
            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C12G_E;

                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G),
                             &(tuneValuesPtr->rxTune.comphy_C12G),
                             sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));

                serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
            }

            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C12G_E;
                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C12G),
                             &(tuneValuesPtr->txTune.comphy_C12G),
                             sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
                serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
            }
            break;
    case CPSS_PORT_SERDES_COMPHY_C28G_E:
            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C28G_E;

                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G),
                             &(tuneValuesPtr->rxTune.comphy_C28G),
                             sizeof(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC));

                serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
            }

            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C28G_E;
                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C28G),
                             &(tuneValuesPtr->txTune.comphy_C28G),
                             sizeof(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC));
                serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
            }
            break;
    case CPSS_PORT_SERDES_COMPHY_C112G_E:
        if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
        {
            serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C112G_E;

            cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G),
                         &(tuneValuesPtr->rxTune.comphy_C112G),
                         sizeof(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC));

            serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
        }

        if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
        {
            serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C112G_E;
            cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C112G),
                         &(tuneValuesPtr->txTune.comphy_C112G),
                         sizeof(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC));
            serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
        }
            break;
    case CPSS_PORT_SERDES_COMPHY_C56G_E:
            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C56G_E;

                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G),
                             &(tuneValuesPtr->rxTune.comphy_C56G),
                             sizeof(CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC));

                serdesDbPtr->portSerdesParams.rxValid = GT_TRUE;
            }

            if (dbValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
            {
                serdesDbPtr->portSerdesParams.serdesCfgDb.type = CPSS_PORT_SERDES_COMPHY_C56G_E;
                cpssOsMemCpy(&(serdesDbPtr->portSerdesParams.serdesCfgDb.txTune.comphy_C56G),
                             &(tuneValuesPtr->txTune.comphy_C56G),
                             sizeof(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC));
                serdesDbPtr->portSerdesParams.txValid = GT_TRUE;
            }
                break;
    default:
            break;
    }

    serdesDbPtr->sdParamValidBitMap = dbValidBitMap;

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLaneTuneHwSet function
* @endinternal
*
* @brief  Set serdes lane RxTx parameters from port manager data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] globalSerdesIndex - serdes index
* @param[in] rxSet          - whether or not to set the rx parameters
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*
* @note this function is for non-ap port, for ap ports use:
*       prvCpssPortManagerApLaneTuneHwSet
*/
GT_STATUS prvCpssPortManagerLaneTuneHwSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroup,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   globalSerdesIndex,
    IN GT_BOOL                  rxSet
)
{
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;
    GT_STATUS                rc;
    GT_U32                   portMacNum;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);

    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC( GT_NOT_INITIALIZED, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null", devNum, portNum );
    }

    if (rxSet == GT_TRUE)
    {
        if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_AVAGO_E)
        {
            MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT rxConfig;
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.dcGain,           tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.DC,             NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.lowFrequency,     tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.LF,             NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.highFrequency,    tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.HF,             NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bandWidth,        tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.BW,             NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.gainShape1,       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.gainshape1,     NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.gainShape2,       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.gainshape2,     NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minLf,            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.minLf,          NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxLf,            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.maxLf,          NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minHf,            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.minHf,          NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxHf,            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.maxHf,          NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bfLf,             tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.BFLF,           NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bfHf,             tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.BFHF,           NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minPre1,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.minPre1,        NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxPre1,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.maxPre1,        NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minPre2,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.minPre2,        NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxPre2,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.maxPre2,        NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minPost,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.minPost,        NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxPost,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.maxPost,        NA_8BIT_SIGNED,    GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.squelch,          tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.sqlch,          NA_16BIT,          GT_U16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.shortChannelEn,   tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.shortChannelEn, NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.termination,      tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.termination,    NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.coldEnvelope,     tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.coldEnvelope,   NA_8BIT,           GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.hotEnvelope,      tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.hotEnvelope,    NA_8BIT,           GT_U8);
            }
            else
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.dcGain,        tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.DC,    NA_8BIT,            GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.lowFrequency,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.LF,    NA_8BIT,            GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.highFrequency, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.HF,    NA_8BIT,            GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bandWidth,     tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.BW,    NA_8BIT,            GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.squelch,       tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.sqlch, NA_16BIT,           GT_U16);

            }
#if !defined (ASIC_SIMULATION)
            rc = mvHwsAvagoSerdesManualRxDBSet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, curPortParams.serdesSpeed, globalSerdesIndex, &rxConfig.rxAvago);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesManualRxDBSet failed=%d", devNum, portNum, rc);
            }
#else
            GT_UNUSED_PARAM(rxConfig);
            GT_UNUSED_PARAM(rc);
#endif
        }
        else /*comphy serdes seires*/
        {
            MV_HWS_SERDES_RX_CONFIG_DATA_UNT rxConfig;
            if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_H_E)
            {
                rxConfig.rxComphyH.portTuningMode = (((CPSS_PORT_INTERFACE_MODE_KR_E ==
                                                  tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb) ||
                                                  _40GBase_CR4 == portMode) ?
                                                  StaticLongReach : StaticShortReach);
                rxConfig.rxComphyH.sqlch  = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy.sqlch;
                rxConfig.rxComphyH.ffeRes = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy.ffeR;
                rxConfig.rxComphyH.ffeCap = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy.ffeC;
                rxConfig.rxComphyH.dfeEn  = GT_TRUE;
                rxConfig.rxComphyH.alig   = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy.align90;
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.resSel  ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.resSel  , NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.resShift,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.resShift, NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.capSel  ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.capSel  , NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmufi ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmufi , NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmuff ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmuff , NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmupi ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmupi , NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmupf ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmupf , NA_8BIT,         GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.squelch ,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.squelch , NA_16BIT_SIGNED, GT_16);
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C28G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.dataRate, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.dataRate, NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.res1Sel , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.res1Sel , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.res2Sel , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.res2Sel , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.cap1Sel , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.cap1Sel , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.cap2Sel , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.cap2Sel , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.squelch , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.squelch , NA_16BIT_SIGNED, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.selmufi , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.selmufi , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.selmuff , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.selmuff , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.selmupi , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.selmupi , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.selmupf , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.selmupf , NA_32BIT,        GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.midpointLargeThresKLane, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.midpointLargeThresKLane, NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.midpointSmallThresKLane, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.midpointSmallThresKLane, NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.midpointLargeThresCLane, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.midpointLargeThresCLane, NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.midpointSmallThresCLane, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.midpointSmallThresCLane, NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.dfeResF0aHighThresInitLane, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.dfeResF0aHighThresInitLane, NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC28GP4.dfeResF0aHighThresEndLane , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C28G.dfeResF0aHighThresEndLane, NA_32BIT, GT_U32);
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C112G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.current1Sel   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.current1Sel,    NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rl1Sel        , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rl1Sel,         NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rl1Extra      , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rl1Extra,       NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.res1Sel       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.res1Sel,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.cap1Sel       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.cap1Sel,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.cl1Ctrl       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.cl1Ctrl,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.enMidFreq     , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.enMidFreq,      NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.cs1Mid        , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.cs1Mid,         NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rs1Mid        , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rs1Mid,         NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rfCtrl        , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rfCtrl,         NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rl1TiaSel     , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rl1TiaSel,      NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rl1TiaExtra   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rl1TiaExtra,    NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.hpfRSel1st    , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.hpfRSel1st,     NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.current1TiaSel, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.current1TiaSel, NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rl2Tune       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rl2Tune,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rl2Sel        , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rl2Sel,         NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.rs2Sel        , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.rs2Sel,         NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.current2Sel   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.current2Sel,    NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.cap2Sel       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.cap2Sel,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.hpfRsel2nd    , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.hpfRsel2nd,     NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.selmufi       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.selmufi,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.selmuff       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.selmuff,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.selmupi       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.selmupi,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.selmupf       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.selmupf,        NA_32BIT, GT_U32);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC112G.squelch       , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C112G.squelch,        NA_8BIT,  GT_U8);
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C56G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.cur1Sel  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.cur1Sel  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.rl1Sel   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.rl1Sel   , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.rl1Extra , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.rl1Extra , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.res1Sel  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.res1Sel  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.cap1Sel  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.cap1Sel  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.enMidfreq, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.enMidfreq, NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.cs1Mid   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.cs1Mid   , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.rs1Mid   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.rs1Mid   , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.cur2Sel  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.cur2Sel  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.rl2Sel   , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.rl2Sel   , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.rl2TuneG , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.rl2TuneG , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.res2Sel  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.res2Sel  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.cap2Sel  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.cap2Sel  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.selmufi  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.selmufi  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.selmuff  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.selmuff  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.selmupi  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.selmupi  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.selmupf  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.selmupf  , NA_16BIT, GT_16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC56G.squelch  , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.comphy_C56G.squelch  , NA_16BIT_SIGNED, GT_16);
            }

            rc = mvHwsSerdesManualRxConfig(CAST_SW_DEVNUM(devNum), portGroup, globalSerdesIndex, HWS_DEV_SERDES_TYPE(devNum, globalSerdesIndex), &rxConfig );
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsSerdesManualRxConfig failed=%d", devNum, portNum, rc);
            }
        }

    }
    else /*tx set*/
    {
        if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_AVAGO_E)
        {
            MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT txConfig;
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.atten, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.avago.atten, NA_8BIT,           GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.pre,   tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.avago.pre,   NA_8BIT_SIGNED,    GT_8);
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.post,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.avago.post,  NA_8BIT_SIGNED,    GT_8);
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.pre2,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.avago.pre2,  NA_8BIT_SIGNED,    GT_8);
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.pre3,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.avago.pre3,  NA_8BIT_SIGNED,    GT_8);

#if !defined (ASIC_SIMULATION)
            rc = mvHwsAvagoSerdesManualTxDBSet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, curPortParams.serdesSpeed, globalSerdesIndex, &txConfig.txAvago);

            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesManualTxDBSet failed=%d", devNum, portNum, rc);
            }
#else
            GT_UNUSED_PARAM(txConfig);
            GT_UNUSED_PARAM(rc);
#endif
        }
        else /*comphy serdes */
        {
            MV_HWS_SERDES_TX_CONFIG_DATA_UNT txConfig;
            if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_H_E)
            {
                txConfig.txComphyH.txAmp      = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.txAmp;
                txConfig.txComphyH.txAmpAdj   = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.txAmpAdj;
                txConfig.txComphyH.txEmph0    = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.emph0;
                txConfig.txComphyH.txEmph1    = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.emph1;
                txConfig.txComphyH.txAmpShft  = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.txAmpShft;
                txConfig.txComphyH.txEmphEn   = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.txEmphEn;
                txConfig.txComphyH.slewRate   = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.slewRate;
                txConfig.txComphyH.slewCtrlEn = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy.slewCtrlEn;
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC12GP41P2V.pre , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C12G.pre,  NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC12GP41P2V.peak, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C12G.peak, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC12GP41P2V.post, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C12G.post, NA_8BIT, GT_U8);
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C28G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC28GP4.pre , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C12G.pre,  NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC28GP4.peak, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C12G.peak, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC28GP4.post, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C12G.post, NA_8BIT, GT_U8);
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C112G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC112G.pre2, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C112G.pre2, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC112G.pre , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C112G.pre,  NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC112G.main, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C112G.main, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC112G.post, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C112G.post, NA_8BIT_SIGNED, GT_8);
            }
            else if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C56G_E)
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC56G.pre2, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C56G.pre2, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC56G.pre , tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C56G.pre,  NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC56G.main, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C56G.main, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC56G.post, tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C56G.post, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC56G.usr,  tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.txTune.comphy_C56G.usr,  NA_8BIT_SIGNED, GT_8);
            }

            rc = mvHwsSerdesManualTxConfig(CAST_SW_DEVNUM(devNum), portGroup, globalSerdesIndex, HWS_DEV_SERDES_TYPE(devNum, globalSerdesIndex), &txConfig );
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsSerdesManualTxConfig failed=%d", devNum, portNum, rc);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerSerdesOverride function
* @endinternal
*
* @brief  override serdes lane RxTx parameters from port manager
*         data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] curPortParams   - port params
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*
* @note this function is for non-ap port, for ap ports use:
*       prvCpssPortManagerApLaneTuneHwSet
*/
GT_STATUS prvCpssPortManagerSerdesOverride
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_INIT_PARAMS  curPortParams
)
{
    GT_STATUS                               rc;
    GT_U32                                  i;
    CPSS_PORT_SERDES_TUNE_STC               serdesParams;
    GT_U32                                  dbSdValidBitMap = 0;
    PRV_CPSS_PORT_MNG_DB_STC                *tmpPortManagerDbPtr;
    GT_U32                                  portGroup;
    GT_U32                                  portMacNum;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType;
    MV_HWS_PORT_STANDARD                    portMode;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    portGroup  = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;
    portType   = tmpPortManagerDbPtr->portMngSmDb[portNum]->portType;
    portMode   = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) && (tmpPortManagerDbPtr->portMngSmDb[portNum]->interconnectProfile))
        {
#if !defined (ASIC_SIMULATION)
            rc = mvHwsAvagoSerdesManualInterconnectDBSet(CAST_SW_DEVNUM(devNum),
                             portGroup, portMacNum, curPortParams.activeLanesList[i], (GT_U32)tmpPortManagerDbPtr->portMngSmDb[portNum]->interconnectProfile);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesManualInterconnectDBSet returned error code %d ",
                                                           devNum, portNum, rc);
            }
#else
            GT_UNUSED_PARAM(portMacNum);
#endif
        }
        rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum), portNum, 0,
                                             curPortParams.activeLanesList[i], portType, &serdesParams, &dbSdValidBitMap);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneDbGet failed=%d on Rx",devNum, portNum,rc);
        }
        if (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
        {
            rc = prvCpssPortManagerLaneTuneHwSet(devNum,portGroup,portNum, portMode,curPortParams.activeLanesList[i], GT_TRUE);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Rx",devNum, portNum,rc);
            }
        }
        if (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
        {
            rc = prvCpssPortManagerLaneTuneHwSet(devNum,portGroup,portNum,portMode,curPortParams.activeLanesList[i], GT_FALSE);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Tx",devNum, portNum,rc);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerApLaneTuneHwSet function
* @endinternal
*
* @brief  Set serdes AP lane attr parameters from port manager
*         data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group number
* @param[in] portNum         - physical port number
* @param[in] advertisedNum   - advertised interface number
* @param[in] laneSerdesIndex - lane serdes index
* @param[in] curPortParams   - current port parameters
* @param[in] rxSet           - whether or not to set the rx parameters
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*
* @note this function is for ap port
*/
GT_STATUS prvCpssPortManagerApLaneTuneHwSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroup,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   advertisedNum,
    IN GT_U32                   laneSerdesIndex,
    IN MV_HWS_PORT_INIT_PARAMS  curPortParams,
    IN GT_BOOL                  rxSet
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT rxConfig;
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT txConfig;
    GT_U32 portMacNum;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d",devNum, portNum, rc);
    }

    if (laneSerdesIndex > PORT_MANAGER_MAX_LANES)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] Lane index bigger than max lane",devNum, portNum);
    }
    if (advertisedNum > CPSS_PORT_AP_IF_ARRAY_SIZE_CNS)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] Advertised number bigger than max advertised number",devNum, portNum);
    }

    if (rxSet == GT_TRUE)
    {
        if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
            perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_AVAGO_E)
        {
            if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.dcGain, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.DC, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.lowFrequency, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.LF, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.highFrequency, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.HF, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bandWidth, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.BW, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.gainShape1, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.gainshape1, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.gainShape2, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.gainshape2, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minLf, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.minLf, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxLf, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.maxLf, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minHf, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.minHf, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxHf, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.maxHf, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bfLf, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.BFLF, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.bfHf, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.BFHF, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minPre1, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.minPre1, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxPre1, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.maxPre1, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minPre2, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * CPSS_PORT_AP_IF_ARRAY_SIZE_CNS) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.minPre2, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxPre2, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.maxPre2, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.minPost, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.minPost, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.maxPost, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.maxPost, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.squelch, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.sqlch, NA_16BIT, GT_U16);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.shortChannelEn, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.shortChannelEn, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.termination, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.termination, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.coldEnvelope, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.coldEnvelope, NA_8BIT, GT_U8);
                PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxAvago.hotEnvelope, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.avago.hotEnvelope, NA_8BIT, GT_U8);

                rc = mvHwsApSerdesRxManualConfigSet(CAST_SW_DEVNUM(devNum),portGroup,portMacNum,
                                                    curPortParams.serdesSpeed,curPortParams.activeLanesList[laneSerdesIndex],AVAGO_16NM,&rxConfig);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsApSerdesRxManualConfigSet failed=%d", devNum, portNum, rc);
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
            }
        }
        else if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
        {
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.resSel  ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.resSel  , NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.resShift,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.resShift, NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.capSel  ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.capSel  , NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmufi ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmufi , NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmuff ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmuff , NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmupi ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmupi , NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.selmupf ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.selmupf , NA_8BIT,         GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(rxConfig.rxComphyC12GP41P2V.squelch ,  tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.rxTune.comphy_C12G.squelch , NA_16BIT_SIGNED, GT_16);

            rc = mvHwsApSerdesRxManualConfigSet(CAST_SW_DEVNUM(devNum),portGroup,portMacNum,
                                                curPortParams.serdesSpeed,curPortParams.activeLanesList[laneSerdesIndex],COM_PHY_C12GP41P2V,&rxConfig);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsApSerdesRxManualConfigSet failed=%d", portNum, rc);
            }
        }
        else
        {
            /* TBD */
            CPSS_TBD_BOOKMARK_PHOENIX
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        }
    }
    else /*tx set*/
    {
        if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_AVAGO_E)
        {
            if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
            {
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.atten, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.avago.atten, NA_8BIT, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.post, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.avago.post, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.pre, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.avago.pre, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.pre2, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.avago.pre2, NA_8BIT_SIGNED, GT_8);
                PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txAvago.pre3, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
                    perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.avago.pre3, NA_8BIT_SIGNED, GT_8);

                rc = mvHwsApSerdesTxManualConfigSet(CAST_SW_DEVNUM(devNum),portGroup,portMacNum,
                                                    curPortParams.serdesSpeed,curPortParams.activeLanesList[laneSerdesIndex],AVAGO_16NM,&txConfig);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsApSerdesTxManualConfigSet failed=%d", devNum, portNum, rc);
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
            }
        }
        else if(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
        {
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC12GP41P2V.pre , tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.comphy_C12G.pre,  NA_8BIT, GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC12GP41P2V.peak, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.comphy_C12G.peak, NA_8BIT, GT_U8);
            PRV_PORT_MANAGER_CHECK_AND_SET(txConfig.txComphyC12GP41P2V.post, tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (laneSerdesIndex)].portSerdesParams.serdesCfgDb.txTune.comphy_C12G.post, NA_8BIT, GT_U8);

            rc = mvHwsApSerdesTxManualConfigSet(CAST_SW_DEVNUM(devNum),portGroup,portMacNum,
                                                curPortParams.serdesSpeed,curPortParams.activeLanesList[laneSerdesIndex],COM_PHY_C12GP41P2V,&txConfig);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] mvHwsApSerdesTxManualConfigSet failed=%d", portNum, rc);
            }
        }
        else
        {
            /* TBD */
            CPSS_TBD_BOOKMARK_PHOENIX
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssPortManagerApLaneTxPresetHwSet function
* @endinternal
*
* @brief  Set serdes AP TX Preset parameters from port manager
*         data base into serdes Hw.
*
* @note  APPLICABLE DEVICES:   Falcon.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2; xCat3; AC5;
*        Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum          - device number
* @param[in] portGroup       - port group number
* @param[in] portNum         - physical port number
* @param[in] portMode        - Port Mode
* @param[in] laneSerdesIndex - lane serdes index
* @param[in] curPortParams   - current port parameters
* @param[in] interconnectProfile - Profile
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*
* @note this function is for ap port
*/
GT_STATUS prvCpssPortManagerApLaneTxPresetHwSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroup,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   laneSerdesIndex,
    IN MV_HWS_PORT_INIT_PARAMS  curPortParams,
    IN GT_U32                   interconnectProfile
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    GT_U8 speedIdx = UNPERMITTED_SD_SPEED_INDEX;

    if ( interconnectProfile >= HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] unsupported Interconnect Profile - %d",devNum, portNum, interconnectProfile);
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d",devNum, portNum, rc);
    }

    if (laneSerdesIndex > PORT_MANAGER_MAX_LANES)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[Port %d/%2d] Lane index bigger than max lane",devNum, portNum);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {

        switch (curPortParams.serdesSpeed)
        {
            case _10_3125G:
            case _10_9375G:
            case _12_1875G:
                speedIdx = 0;
                break;
            case _20_625G:
            case _25_78125G:
            case _27_34375G:
                speedIdx = 1;
                break;
            case _26_5625G_PAM4:
            case _27_1875_PAM4:
                speedIdx = 2 ;
                break;
            case _28_125G_PAM4:
                speedIdx = 3 ;
                break;
            default:
                return GT_OK; /* Do nothing for unsupported speed */
        }
        if (( interconnectProfile == 2 ) && (speedIdx<2)) {
            /* we are using AUTO PROFILE - dont overwrite tx preset */
            return GT_OK;
        }
        rc = mvHwsApSerdesTxPresetSet(devNum, portGroup, portMacNum,
                                      laneSerdesIndex, portMode,
                                      hwsAvagoSerdesTxTune16nmPreset1ParamsProfiles[speedIdx][interconnectProfile].pre,
                                      hwsAvagoSerdesTxTune16nmPreset1ParamsProfiles[speedIdx][interconnectProfile].atten,
                                      hwsAvagoSerdesTxTune16nmPreset1ParamsProfiles[speedIdx][interconnectProfile].post);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsApSerdesTxPresetSet failed=%d", devNum, portNum, rc);
        }

    }

    return GT_OK;

}

GT_STATUS prvCpssPortManagerLaneCiRxTraining
(
    IN GT_SW_DEV_NUM         devNum,
    IN GT_U32                portGroup,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_U32                globalSerdesIndex,
    IN MV_HWS_SERDES_SPEED   serdesSpeed,
    IN GT_BOOL               rxOverride
)
{
#ifdef ASIC_SIMULATION
     devNum = devNum;
     portGroup = portGroup;
     portNum = portNum;
     globalSerdesIndex = globalSerdesIndex;
     serdesSpeed = serdesSpeed;
     rxOverride = rxOverride;
#else
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    GT_BOOL matched = GT_FALSE;
    GT_U32 i;
    GT_STATUS rc;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT rxConfig;
    cpssOsMemSet(&rxConfig, 0, sizeof(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA));

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    cpssOsMemSet(&rxConfig, 0, sizeof(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA));

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, globalSerdesIndex, 1);

    if (rxOverride)
    {
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            rxConfig.rxAvago.dcGain         = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.DC;
            rxConfig.rxAvago.lowFrequency   = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.LF;
            rxConfig.rxAvago.highFrequency  = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.HF;
            rxConfig.rxAvago.bandWidth      = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.BW;
            rxConfig.rxAvago.squelch        = tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[globalSerdesIndex]->portSerdesParams.serdesCfgDb.rxTune.avago.sqlch;
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesManualCtleConfig not supporting this SerDes type", devNum, portNum);
        }

        rc = mvHwsAvagoSerdesManualCtleConfig(CAST_SW_DEVNUM(devNum), portGroup, globalSerdesIndex, &rxConfig);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesManualCtleConfig failed=%d", devNum, portNum, rc);
        }
    }
    else
    {
        for (i = SPEED_NA; i < LAST_MV_HWS_SERDES_SPEED; i++)
        {
            if (hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed == serdesSpeed)
            {
                matched = GT_TRUE;
                break;
            }
        }

        if (matched == GT_FALSE)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] SerDes Speed Not Found=%d", devNum, portNum, rc);
        }

        rxConfig.rxAvago.dcGain = hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.dcGain;
        rxConfig.rxAvago.lowFrequency = hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.lowFrequency;
        rxConfig.rxAvago.highFrequency = hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.highFrequency;
        rxConfig.rxAvago.bandWidth = hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.bandWidth;
        rxConfig.rxAvago.squelch = hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams.squelch;

        rc = mvHwsAvagoSerdesManualCtleConfig(CAST_SW_DEVNUM(devNum),portGroup,globalSerdesIndex, &rxConfig);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesManualCtleConfig failed=%d", devNum, portNum, rc);
        }
    }


    rc = mvHwsAvagoSerdesRxAutoTuneStart(CAST_SW_DEVNUM(devNum),portGroup,globalSerdesIndex, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsAvagoSerdesRxAutoTuneStart failed=%d", devNum, portNum, rc);
    }
#endif
    return GT_OK;
}


GT_STATUS prvCpssPortManagerResetOpticalCalCounters
(
    GT_PHYSICAL_PORT_NUM  portNum,
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr
)
{
    GT_U32 i;
    for (i = 0; i < PORT_MANAGER_MAX_LANES; i++)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[i] = GT_FALSE;
    }
    tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask = GT_FALSE;

    return GT_OK;
}

static GT_STATUS prvCpssPortManagerOpticalCalibrationSet
(
    GT_SW_DEV_NUM        devNum,
    GT_U32               portGroup,
    GT_PHYSICAL_PORT_NUM portNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    GT_U32               localLaneIdx,
    GT_U32               globalSerdesIndex,
    GT_BOOL              *calibrationFinished
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_SERDES_TUNE_STC tuneValues;
    GT_BOOL needTraining = GT_FALSE;
    CPSS_PORT_SERDES_TUNE_STC serdesParams;
    GT_U32 dbSdValidBitMap = 0;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    rc = ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesTuneResultGetFunc(devNum,portNum,localLaneIdx,&tuneValues)));
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Could not get Tuning Result");
    }
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum," [serdes %2d] serdesSpeed %d EO %d HF %d LF %d",serdesSpeed, localLaneIdx, tuneValues.rxTune.avago.EO, tuneValues.rxTune.avago.HF, tuneValues.rxTune.avago.LF);
    *calibrationFinished = GT_FALSE;
#ifndef ASIC_SIMULATION
    if (((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MIN_LF_OFFSET_E) & 0x1) &&
        ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MAX_LF_OFFSET_E) & 0x1))
    {
        if ((tuneValues.rxTune.avago.LF < tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.minLfThreshold) ||
            (tuneValues.rxTune.avago.LF > tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.maxLfThreshold))
        {
            /* signal is not in confidence range */
            needTraining = GT_TRUE;
        }

    }
    if (((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MIN_HF_OFFSET_E) & 0x1) &&
        ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MAX_HF_OFFSET_E) & 0x1))
    {
        if ((tuneValues.rxTune.avago.HF < tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.minHfThreshold) ||
            (tuneValues.rxTune.avago.HF > tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.maxHfThreshold))
        {
            /* signal is not in confidence range */
            needTraining = GT_TRUE;
        }

    }
    if (((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MIN_EO_OFFSET_E) & 0x1) &&
        ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.confidenceBitMap >> CPSS_PORT_MANAGER_MAX_EO_OFFSET_E) & 0x1))
    {
        if ((tuneValues.rxTune.avago.EO < tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.minEoThreshold) ||
            (tuneValues.rxTune.avago.EO > tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.maxEoThreshold))
        {
            needTraining = GT_TRUE;
            /* signal is not in confidence range */
        }
    }
#endif
    if (needTraining)
    {
        GT_BOOL rx_valid = GT_FALSE;

        /* check if application overrides default RX data */
        rc = prvCpssPortManagerLaneTuneDbGet(devNum, portNum, 0,
                                             globalSerdesIndex, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &serdesParams, &dbSdValidBitMap);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneDbGet failed=%d on Rx",devNum, portNum,rc);
        }
        rx_valid = (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)?GT_TRUE:GT_FALSE;

        rc = prvCpssPortManagerLaneCiRxTraining(devNum,portGroup,portNum,globalSerdesIndex, serdesSpeed,rx_valid);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneCiRxTraining failed", devNum, portNum);
        }
    }
    else
    {
        *calibrationFinished = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerOpticalStabilityCheck function
* @endinternal
*
* @brief  Run optical stability Algorithm
*
* @note  APPLICABLE DEVICES:   Pipe; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum               - device number
* @param[in] portGroup            - port group
* @param[in] portNum              - physical port number
* @param[in] portMode             - port mode
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_NOT_SUPPORTED     - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
*/
static GT_STATUS prvCpssPortManagerOpticalStabilityCheck
(
    IN GT_SW_DEV_NUM         devNum,
    IN GT_U32                portGroup,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN MV_HWS_PORT_STANDARD  portMode
)
{
        GT_STATUS rc;
        GT_U32 portMacNum;
        GT_BOOL rxSgnlOk = GT_FALSE;
        GT_BOOL rxSgnlDetect = GT_FALSE;
        MV_HWS_PORT_INIT_PARAMS     curPortParams;
        PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
        GT_U32 localLaneIdx;
        GT_BOOL calibrationFinished = GT_FALSE;
        GT_U32 timerSec;
        GT_U32 timerNsec;
        GT_U32 timeOutSec;
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

        /* getting port manager database*/
        PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

        portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

        if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC( GT_NOT_INITIALIZED, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null", devNum, portNum );
        }

        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew == 0) &&
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev == 0))
        {
            rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev,
                         &tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew);
            if (rc != GT_OK)
            {
                /* reset timers */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
                prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
                prvCpssPortManagerModeToDownState(devNum,portNum);

                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Timer issue - moving to link down", devNum, portNum);
            }


        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimeoutSec > 0)
        {
            timeOutSec = tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimeoutSec;
        }
        else
        {
            timeOutSec = 10;
#ifdef ASIC_SIMULATION
            timeOutSec = 100;
#endif
        }

        rc = cpssOsTimeRT(&timerSec,&timerNsec);
        if (rc != GT_OK)
        {
            /* reset timers */
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
            prvCpssPortManagerModeToDownState(devNum,portNum);

            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Timer issue - moving to link down", devNum, portNum);
        }

        if (timerSec - tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev > timeOutSec)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
            prvCpssPortManagerModeToDownState(devNum,portNum);

            return GT_OK;
        }

         /* mask low level interrupts */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask == GT_FALSE)
        {
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
            }
            /* avoid spamming the log */
            tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask = GT_TRUE;
        }

        if (!(PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneStatusGetFunc(devNum, portNum,
                                                                                      &rxTuneStatus, &txTuneStatus);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform ppTuneStatusGetFunc operation", devNum, portNum);
            }
            if (rxTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
            {
                if (timerSec - tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev > (timeOutSec/2))
                {
                    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc(devNum, portNum, CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
                }
                return GT_OK;
            }
        }

        rc = mvHwsPortAvagoSerdesRxSignalOkStateGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &rxSgnlOk);
        if (rc != GT_OK)
        {
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDrvPpHwRegBitMaskRead failed=%d", devNum, portNum, rc);
        }

        if (rxSgnlOk == GT_FALSE)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSigDetGetFunc(devNum, portNum, &rxSgnlDetect);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppSigDetGetFunc(PRV_CPSS_PORT_MANAGER_SIG_DET_E): rc=%d\n",
                                                           devNum, portNum, rc);
            }
            if (rxSgnlDetect)
            {
                return GT_OK;
            }
            else
            {
                /* if there is no signal move to link down state, only low_level_event will recover the port */
                if ((!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
                {
                    rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", devNum, portNum, rc);
                    }
                }
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"moves to link_down state, no signal detect");
                /* reset optical counters */
                prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
                prvCpssPortManagerModeToDownState(devNum,portNum);

                return GT_OK;
            }
        }
        for (localLaneIdx = 0; localLaneIdx < curPortParams.numOfActLanes; localLaneIdx++)
        {
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[localLaneIdx] == GT_FALSE)
            {
                rc = prvCpssPortManagerOpticalCalibrationSet(devNum, portGroup, portNum,
                        curPortParams.serdesSpeed, localLaneIdx, curPortParams.activeLanesList[localLaneIdx], &calibrationFinished);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerOpticalCalibrationSet failed=%d", devNum, portNum, rc);
                }

                if (calibrationFinished)
                {
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[localLaneIdx] = GT_TRUE;
                }
            }
        }
        for (localLaneIdx = 0; localLaneIdx < curPortParams.numOfActLanes; localLaneIdx++)
        {
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalFinished[localLaneIdx] == GT_FALSE)
            {
                return GT_OK;
            }
        }
        /* in case of cable plug out mask operation will take place*/
        tmpPortManagerDbPtr->portMngSmDb[portNum]->disableMaskUnMask = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished = GT_TRUE;

        /* unmask low level interrupts (working on mac lvl only)*/
        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
        }

        /* if we passed optical calibration successfuly reset timers */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerSecPrev = 0;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.opticalCalTimerNsecNew = 0;
        }

        return GT_OK;
}

/**
* @internal prvCpssPortManagerLoopbackModeInit function
* @endinternal
*
* @brief  Init loopback configuration structure according to given loopback type.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
*
* @param[out] loopbackCfgPtrOut    - (pointer to) loopback configuration structure to initialize
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_BAD_PTR        - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLoopbackModeInit
(
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT loopbackType,
    OUT CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtrOut
)
{

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtrOut);

    loopbackCfgPtrOut->loopbackType = loopbackType;
    loopbackCfgPtrOut->enableRegularTrafic = GT_FALSE;
    switch (loopbackType)
    {
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
        loopbackCfgPtrOut->loopbackMode.macLoopbackEnable = GT_FALSE;
        break;
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
        loopbackCfgPtrOut->loopbackMode.serdesLoopbackMode = CPSS_PORT_SERDES_LOOPBACK_DISABLE_E;
        break;

    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E:
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                                   "[Port %2d] prvCpssPortManagerLoopbackModeInit:bad loopback type=%d",portNum,loopbackType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLoopbackModesDbGet function
* @endinternal
*
* @brief  Get loopback mode of the port.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum               - device number
* @param[in] portNum              - physical port number
* @param[in] portType             - AP or Non-AP port type
*
* @param[out] loopbackCfgPtrOut   - (pointer to) loopback configuration structure
* @param[out] anyLoopbackPtr      - (pointer to) whether or not any loopback is configured
*
* @retval GT_OK          - on success
* @retval GT_FAIL         - on error
* @retval GT_BAD_PTR        - on bad pointer
*/
GT_STATUS prvCpssPortManagerLoopbackModesDbGet
(
    IN GT_SW_DEV_NUM                           devNum,
    IN GT_PHYSICAL_PORT_NUM                    portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType,
    OUT CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC  *loopbackCfgPtrOut,
    OUT GT_BOOL                                *anyLoopbackPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *tmpLoopbackCfgPtr;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtrOut);
    CPSS_NULL_PTR_CHECK_MAC(anyLoopbackPtr);

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
        if(tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort == GT_TRUE)
        {
            *anyLoopbackPtr = GT_FALSE;
            return GT_OK;
        }
    }
    else
    {
        /* Loopback is currently not supported in AP - so need to return no Loopback with OK status */
        *anyLoopbackPtr = GT_FALSE;
        return GT_OK;
        /* verify allocation */
        /*PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);*/
    }

    /* first, fill defaults */
    *anyLoopbackPtr = GT_FALSE;
    /* next, fill real data */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        tmpLoopbackCfgPtr = &(tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->
                              loopbackDbCfg);
    }
    else
    {
/* TBD - add loopback support for AP port */
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                                   "[Port %d/%2d] loopback not suported for AP port", devNum, portNum);
        /*tmpLoopbackCfgPtr = &(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->generalPortCfg.
                   loopbackDbCfg);*/
    }

    cpssOsMemCpy(loopbackCfgPtrOut, tmpLoopbackCfgPtr, sizeof(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC));

    switch (tmpLoopbackCfgPtr->loopbackType)
    {
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
        if (tmpLoopbackCfgPtr->loopbackMode.macLoopbackEnable==GT_TRUE)
        {
            *anyLoopbackPtr = GT_TRUE;
        }
        break;
    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
        if (loopbackCfgPtrOut->loopbackMode.serdesLoopbackMode != CPSS_PORT_SERDES_LOOPBACK_DISABLE_E)
        {
            *anyLoopbackPtr = GT_TRUE;
        }
        break;

    case CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E:
        /* not filling data */
        break;

    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"[Port %d/%2d] loopback type undefined=%d"
                                                   ,devNum, portNum, tmpLoopbackCfgPtr->loopbackType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerLoopbackModesDbSet function
* @endinternal
*
* @brief  Set loopback mode of the port.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum          - device number
* @param[in] portNum         - physical port number
* @param[in] portType        - AP or Non-AP port type
* @param[in] loopbackCfgPtr  - (pointer to) loopback configuration structure
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_BAD_PTR         - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLoopbackModesDbSet
(
    IN GT_SW_DEV_NUM                         devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT       portType,
    IN CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *portCfgDbPtr;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtr);

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
        portCfgDbPtr = &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg;
    }
    else
    {
/* TBD - AP port loopback */
        /* verify allocation */
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %d/%2d] loopback not suported for AP port", devNum, portNum);
        /*
        PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
        portCfgDbPtr = &tmpPortManagerDbPtr->portsApAttributesDb[portNum]->
          generalPortCfg.loopbackDbCfg;*/
    }

/* TBD - currently, loopback not supported in AP (portMode needed in this stage in order
  to perform serdes loopback) */
    cpssOsMemSet(portCfgDbPtr, 0, sizeof(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC));
    portCfgDbPtr->loopbackType = loopbackCfgPtr->loopbackType;
    /* MAC loopback */
    if (loopbackCfgPtr->loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)
    {
        portCfgDbPtr->loopbackMode.macLoopbackEnable = loopbackCfgPtr->loopbackMode.macLoopbackEnable;
    }
    /* SERDES loopback */
    else if (loopbackCfgPtr->loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E)
    {
        portCfgDbPtr->loopbackMode.serdesLoopbackMode = loopbackCfgPtr->loopbackMode.serdesLoopbackMode;
    }
    else if (loopbackCfgPtr->loopbackType != CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                                   "[Port %d/%2d] loopback type undefined=%d", devNum, portNum, loopbackCfgPtr->loopbackType);
    }
    portCfgDbPtr->enableRegularTrafic = loopbackCfgPtr->enableRegularTrafic;

    return GT_OK;
}

/**
* @internal prvCpssPortManagerGetPortHwsParams function
* @endinternal
*
* @brief  Get Hws parameters for the given port according to it's parameters.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum                 - device number
* @param[in] portGroup              - port group number
* @param[in] portNum                - physical port number
*
* @param[out] curPortParamsPtr      - (pointer to pointer to) hws parameters
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
* @retval GT_NOT_SUPPORTED   - on not supported device
* @retval GT_BAD_PARAM       - on bad physical port number
* @retval GT_BAD_PTR         - on null pointer
*/
GT_STATUS prvCpssPortManagerGetPortHwsParams
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroup,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT MV_HWS_PORT_INIT_PARAMS *curPortParamsPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    MV_HWS_PORT_STANDARD portMode;
    PRV_CPSS_PORT_MNG_DB_STC*    tmpPortManagerDbPtr;

    CPSS_NULL_PTR_CHECK_MAC(curPortParamsPtr);

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "%s not supported for %d device",
                                                   __FUNCNAME__, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    /* getting appropriate mac num for the given physical port num */
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb,
                                               tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssLion2CpssIfModeToHwsTranslate returned rc %d", devNum, portNum, rc );
    }

    /* get hws parameters for a given Hws port mode */
    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, curPortParamsPtr))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null ", devNum, portNum );
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed function
* @endinternal
*
* @brief  Get Hws parameters for the given port according to it's ifMode and speed.
*
* @note  APPLICABLE DEVICES:   xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note  NOT APPLICABLE DEVICES: Lion2; Bobcat2.
*
* @param[in] devNum              - device number
* @param[in] portGroup           - port group number
* @param[in] portNum             - physical port number
* @param[in] ifMode              - port interface mode
* @param[in] speed               - port speed
*
* @param[out] curPortParamsPtr   - pointer to hws parameters
*
* @retval GT_OK                  - on success
* @retval GT_FAIL                - on error
* @retval GT_NOT_SUPPORTED       - on not supported device
* @retval GT_BAD_PARAM           - on bad physical port number
* @retval GT_BAD_PTR             - on null pointer
*/
GT_STATUS prvCpssPortManagerGetPortHwsParamsByIfModeAndSpeed
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       portGroup,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed,
    OUT MV_HWS_PORT_INIT_PARAMS      *curPortParamsPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    MV_HWS_PORT_STANDARD portMode;

    CPSS_NULL_PTR_CHECK_MAC(curPortParamsPtr);

    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "%s not supported for %d device",
                                                   __FUNCNAME__, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting appropriate mac num for the given physical port num */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d", devNum, portNum, rc);
    }

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode,
                                               speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d", devNum, portNum, rc );
    }

    /* get hws parameters for a given Hws port mode */
    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, curPortParamsPtr))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] hwsPortModeParamsGet returned null ", devNum, portNum );
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerCleanPortParameters function
* @endinternal
*
* @brief   Clean all port parameters in Port Manager DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - AP or Non-AP port type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortManagerCleanPortParameters
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_U32                          portGroup,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_MANAGER_PORT_TYPE_ENT portType
)
{
    GT_STATUS        rc;
    GT_U8          laneIndex,i;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    PRV_CPSS_PORT_MNG_DB_STC  *tmpPortManagerDbPtr;

    cpssOsMemSet(&curPortParams, 0, sizeof(MV_HWS_PORT_INIT_PARAMS));
    /* port manager device applicability check */
    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortManagerGetPortHwsParams not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* verify allocation */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    }
    else
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
    }
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, 1);

    /* getting hws parameters of the port */
    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        rc = prvCpssPortManagerGetPortHwsParams(devNum, portGroup, portNum, &curPortParams);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerGetPortHwsParams failed=%d", devNum, portNum, rc);
        }
    }

    /* full clean-up */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideTrainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideEdgeDetectSupported = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility = CPSS_PORT_FEC_MODE_LAST_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->overrideEtParams = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->max_LF = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->min_LF = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperationsBitmap = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort = GT_FALSE;

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
        tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb = CPSS_PORT_SPEED_NA_E;
        for (laneIndex=0; laneIndex<curPortParams.numOfActLanes; laneIndex++)
        {
            /* verify allocation */
            PRV_PORT_MANAGER_CHECK_ALLOCATION_SERDES_MAC(tmpPortManagerDbPtr, curPortParams.activeLanesList[laneIndex], 1);
            cpssOsMemSet((&tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[laneIndex]]->portSerdesParams),0,
                         sizeof(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[0]->portSerdesParams));
        }

        /* cleaning loopback db */
        rc = prvCpssPortManagerLoopbackModeInit(portNum,CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                                                &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModeInit failed=%d", devNum, portNum, rc);
        }
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        i=0;
        /* ap attributes */
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fcAsmDir = PRV_PORT_MANAGER_DEFAULT_FC_ASM_DIR;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.laneNum = PRV_PORT_MANAGER_DEFAULT_LANE_NUM;
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable   = PRV_PORT_MANAGER_DEFAULT_6_10_NONECE_DISABLE;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fcPause = PRV_PORT_MANAGER_DEFAULT_6_10_FC_PAUSE;
        }
        else
        {
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable   = PRV_PORT_MANAGER_DEFAULT_NONECE_DISABLE;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fcPause = PRV_PORT_MANAGER_DEFAULT_FC_PAUSE;
        }
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask = 0;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.portOperationsBitmap = 0;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortApNumOfModes = 0;
        /* in AP, in port manager DB those are the fields for the supported advertisement modes */
        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fecAbilityArr[i] = CPSS_PORT_FEC_MODE_DISABLED_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fecRequestedArr[i] = CPSS_PORT_FEC_MODE_DISABLED_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb = CPSS_PORT_SPEED_NA_E;
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb, 0,
                     sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC) * CPSS_PORT_AP_IF_ARRAY_SIZE_CNS * PORT_MANAGER_MAX_LANES);

        /* cleaning loopback db */
        /* TBD - add loopback support for AP port */
        /*rc = prvCpssPortManagerLoopbackModeInit(portNum,CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                      &tmpPortManagerDbPtr->portsApAttributesDb[portNum]->generalPortCfg.loopbackDbCfg);
        if (rc!=GT_OK)
        {
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                    "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d", portNum, rc);
        }*/

    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portType = CPSS_PORT_MANAGER_PORT_TYPE_LAST_E;

    return GT_OK;
}

GT_STATUS prvCpssPortManagerFillSerdesDb
(
     IN  GT_SW_DEV_NUM                 devNum,
     IN  CPSS_PORT_SERDES_TUNE_STC     *serdesCfgDbPtr
)
{
    if(serdesCfgDbPtr != NULL)
    {
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
        {
            serdesCfgDbPtr->type = CPSS_PORT_SERDES_AVAGO_E;
            serdesCfgDbPtr->txTune.avago.atten = NA_16BIT;
            serdesCfgDbPtr->txTune.avago.post  = NA_16BIT;
            serdesCfgDbPtr->txTune.avago.pre   = NA_16BIT;
            serdesCfgDbPtr->txTune.avago.pre2  = NA_16BIT;
            serdesCfgDbPtr->txTune.avago.pre3  = NA_16BIT;

            serdesCfgDbPtr->rxTune.avago.sqlch = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.DC    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.LF    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.HF    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.BW    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.EO    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.gainshape1 = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.gainshape2 = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.dfeGAIN    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.dfeGAIN2   = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.BFLF       = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.BFHF       = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minLf      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxLf      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minHf      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxHf      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minPre1    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxPre1    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minLf      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxLf      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minPre1    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxPre1    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxPre1    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minPre2    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxPre2    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.minPost    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.maxPost    = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.pre1       = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.pre2       = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.post1      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.coldEnvelope     = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.hotEnvelope      = NA_16BIT;
            serdesCfgDbPtr->rxTune.avago.shortChannelEn   = NA_16BIT;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

GT_STATUS prvCpssPortManagerInitBeforeFirstCreate
(
    IN GT_SW_DEV_NUM                             devNum
)
{
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;
    CPSS_EVENT_MASK_SET_ENT tmpOperation;
    GT_U32                  portMacNum, port;
    GT_STATUS               rc;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        for(port = 0 ; port < PRV_CPSS_MAX_PP_PORTS_NUM_CNS ; port++)
        {
            /* getting appropriate mac num for the given physical port num */
            if (GT_OK != PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, port, &portMacNum))
            {
                continue;
            }
            /* mask mac and low level interrupts */
            rc = cpssEventDeviceMaskWithEvExtDataSet(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E, port, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d LINK_STATUS_CHANGE returned %d ", devNum, port, CPSS_EVENT_MASK_E, rc);
            }
            rc = cpssEventDeviceMaskWithEvExtDataGet(devNum,
                                                     CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                                     port, &tmpOperation);
            if (rc == GT_NOT_FOUND)
            {
                /*do nothing - no such event for this port */
            }
            else if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataGet %d CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E returned %d ", devNum, port, CPSS_EVENT_MASK_E, rc);
            }
            else
            {
                rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                         CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                                         port, CPSS_EVENT_MASK_E);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet %d CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E returned %d ", devNum, port, CPSS_EVENT_MASK_E, rc);
                }
            }
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerVerifyAllocation
(
    IN GT_SW_DEV_NUM                             devNum,
    IN GT_PHYSICAL_PORT_NUM                      portNum,
    IN PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ENT allocType
)
{
    GT_STATUS rc;
    GT_U32 portMacNum, i;
    GT_U8 serdesIndex;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    /* getting appropriate mac num for the given physical port num */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d",devNum, portNum, rc);
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        /* allocate port */
        if (tmpPortManagerDbPtr->portMngSmDb==NULL)
        {
            prvCpssPortManagerInitBeforeFirstCreate(devNum);
            tmpPortManagerDbPtr->portMngSmDb =
                (PRV_CPSS_PORT_MNG_PORT_SM_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portMngSmDb==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

            cpssOsMemSet( tmpPortManagerDbPtr->portMngSmDb, 0, sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]==NULL)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum] =
            (PRV_CPSS_PORT_MNG_PORT_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC));
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portMngSmDb[portNum], 0, sizeof(PRV_CPSS_PORT_MNG_PORT_SM_DB_STC));

        /* default state for a port is RESET state */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_RESET_E;
        /* default failure for a port is FAILURE_NONE */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
        /* default operative-disabled state for a port is FALSE */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility = CPSS_PORT_FEC_MODE_LAST_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portType = CPSS_PORT_MANAGER_PORT_TYPE_LAST_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->overrideEtParams = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->min_LF = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->max_LF = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType = CPSS_PORT_MANAGER_DEFAULT_CALIBRATION_TYPE_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->unMaskEventsMode = CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_ENABLE_MODE_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->bwMode = CPSS_PORT_PA_BW_MODE_REGULAR_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->enterDebugState = CPSS_PORT_MANAGER_STATE_RESET_E;

        /* in 6_10 we change oneShotiCal default to true in order to support usx ports */
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal = GT_TRUE;
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_MAC_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        MV_HWS_PORT_INIT_PARAMS curPortParams;

        /* getting hws parameters of the port */
        rc = prvCpssPortManagerGetPortHwsParams(devNum, 0, portNum, &curPortParams);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerGetPortHwsParams failed=%d", devNum, portNum, rc);

        /* mac pointers allocation */
        if (tmpPortManagerDbPtr->portMngSmDbPerMac==NULL)
        {
            tmpPortManagerDbPtr->portMngSmDbPerMac =
                (PRV_CPSS_PORT_MNG_MAC_SM_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portMngSmDbPerMac==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

            cpssOsMemSet( tmpPortManagerDbPtr->portMngSmDbPerMac, 0, sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS );
        }

        /* mac allocation */
        for (serdesIndex=0; serdesIndex<curPortParams.numOfActLanes; serdesIndex++)
        {
            if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex]==NULL)
            {
                tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex] =
                    (PRV_CPSS_PORT_MNG_MAC_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC));
                if (tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex]==NULL)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);
            }
            cpssOsMemSet(tmpPortManagerDbPtr->portMngSmDbPerMac[portMacNum+serdesIndex], 0, sizeof(PRV_CPSS_PORT_MNG_MAC_SM_DB_STC));
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_SERDES_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        MV_HWS_PORT_INIT_PARAMS curPortParams;
        CPSS_PORT_SERDES_TUNE_STC *serdesCfgDbPtr= NULL;

        /* getting hws parameters of the port */
        rc = prvCpssPortManagerGetPortHwsParams(devNum, 0, portNum, &curPortParams);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerGetPortHwsParams failed=%d", devNum, portNum, rc);

        /* serdes pointer allocation */
        if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr =
                (PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS*8);
            if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr, 0,
                         sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS*8);
        }

        /* per serdes allocation */
        for (serdesIndex=0; serdesIndex<curPortParams.numOfActLanes; serdesIndex++)
        {
            if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]==NULL)
            {
                tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]] =
                    (PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC));

                if (tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]==NULL)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);
            }
            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]],
                        0, sizeof(PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC));

            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]->portSerdesParams.rxValid = GT_FALSE;
            tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]->portSerdesParams.txValid = GT_FALSE;

            serdesCfgDbPtr = &(tmpPortManagerDbPtr->portsAttributedDb.perSerdesDbPtr[curPortParams.activeLanesList[serdesIndex]]->portSerdesParams.serdesCfgDb);
            rc = prvCpssPortManagerFillSerdesDb(devNum, serdesCfgDbPtr);
            if(rc!=GT_OK)
                return rc;
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr =
                (PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC**)
            cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr, 0,
                        sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS );
        }

        if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum] =
                (PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC));

            if (tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum], 0, sizeof(PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC));

        /*tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->portType = CPSS_PORT_MANAGER_PORT_TYPE_LAST_E;*/
        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb = CPSS_PORT_SPEED_NA_E;
        }
    }

    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        CPSS_PORT_SERDES_TUNE_STC *serdesCfgDbPtr= NULL;
        GT_U32 speedLaneIdx, lanesArrIdx;

        if (tmpPortManagerDbPtr->portsApAttributesDb == NULL)
        {
            tmpPortManagerDbPtr->portsApAttributesDb =
                (PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC**)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portsApAttributesDb==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsApAttributesDb,0,
                        sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
        }

        if (tmpPortManagerDbPtr->portsApAttributesDb[portNum] == NULL)
        {
            tmpPortManagerDbPtr->portsApAttributesDb[portNum] =
                (PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC));
            if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]==NULL)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);
            }
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsApAttributesDb[portNum], 0, sizeof(PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC));

        for (i=0; i<CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb = CPSS_PORT_SPEED_NA_E;
        }
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortApNumOfModes = 0;
        for (speedLaneIdx=0 ; speedLaneIdx < CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E; speedLaneIdx++)
        {
            for (lanesArrIdx = 0; lanesArrIdx < PORT_MANAGER_MAX_LANES; lanesArrIdx++)
            {
                serdesCfgDbPtr = &(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perSerdesDb[speedLaneIdx + lanesArrIdx].portSerdesParams.serdesCfgDb);
                rc = prvCpssPortManagerFillSerdesDb(devNum, serdesCfgDbPtr);
                if(rc!=GT_OK)
                    return rc;
            }
        }
    }

#if 0
    /* initialize loopback configurations */
    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        /* allocation - in AP structures this is allocated statically as the whole AP enclosing structure
          is already allocated per port, and the loopback configuration is global per the port. */

        /* initialization */
        /* TBD - add loopback support for AP port */
        /*rc = prvCpssPortManagerLoopbackModeInit(portNum,
                    CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                    &tmpPortManagerDbPtr->portsApAttributesDb[portNum]->generalPortCfg.loopbackDbCfg);
        if (rc!=GT_OK)
        {
          CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                    "[Port %2d] prvCpssPortManagerLoopbackModeInit failed=%d", portNum, rc);
        }*/
    }
#endif
    /* initialize loopback configurations */
    if (allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E ||
        allocType == PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_ALL_E)
    {
        /* allocation */
        if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr == NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr =
                (PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC**)
            cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
            if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);

            cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr, 0,
                        sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC*)*PRV_CPSS_MAX_PP_PORTS_NUM_CNS );
        }

        if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]==NULL)
        {
            tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum] =
                (PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC*)cpssOsMalloc(sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC));

            if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]==NULL)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"[Port %d/%2d] allocation failed", devNum, portNum);
        }
        cpssOsMemSet(tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum], 0,
                     sizeof(PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC));

        /* initialization */
        rc = prvCpssPortManagerLoopbackModeInit(portNum, CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                                                &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModeInit failed=%d", devNum, portNum, rc);
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerInit function
* @endinternal
*
* @brief   Initialize and checking port parameters function
*          pointer binding
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPortManagerInit
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_STATUS rc = GT_OK;

    if (!(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "prvCpssPortManagerInit not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwUnresetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwResetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemoteFaultSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppForceLinkDownSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableGetFunc == NULL ||
        /*PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortDeleteWASetFunc == NULL || it is not a must, not supported in px*/
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneStatusGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSigDetGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCgConvertersGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCgConvertersStatus2GetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConvertFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSerdesNumGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppNumOfSerdesGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortManagerDbGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMpcs40GCommonStatusRegGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacDmaModeRegAddrGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusChangedNotifyFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppXlgMacMaskAddrGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLaneTuneSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLaneTuneGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigClearFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigurationFunc == NULL /*||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApConfigSetFunc == NULL*/ /*AP is not a must*/
        /* PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApStatusGetFunc == NULL */||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacLoopbackModeSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPcsLoopbackModeSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesLoopbackModeSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesPolaritySetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortProprietaryHighSpeedPortsSetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesTuneResultGetFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppAutoNeg1GSgmiiFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMgrInitFunc == NULL)/* ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLowPowerEnableFunc == NULL ||
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortFastLinkDownSetFunc == NULL)||*/ /*currently not supported in PX*/
        /*PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc == NULL*/  /*it is not a must, not supported in px*/
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "function pointer is null");
    }

    return rc;
}

/**
* @internal prvCpssPortManagerResetCounters function
* @endinternal
*
* @brief  reset the port manager counters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                - device number
*
* @param[out] tmpPortManagerDbPtr  - port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPortManagerResetCounters
(
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT PRV_CPSS_PORT_MNG_DB_STC*  tmpPortManagerDbPtr
)
{
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart    = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimeDiffFromLastTime  = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev      = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev         = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries                 = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portWaitForTrainResultTries           = 0;
    /* Reset timers */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerSecondsPrev           = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimerNsecPrev              = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeDiffFromLastTime       = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromStart         = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTimeIntervalTimer          = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTotalTimeFromChange        = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone            = CPSS_PORT_MANAGER_RESET_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone                = CPSS_PORT_MANAGER_RESET_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone            = CPSS_PORT_MANAGER_RESET_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portTrainResTimeIntervalTimer  = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault              = GT_FALSE;

    /* in case of disable need to restart the optical algo */
    prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);

    return GT_OK;
}

GT_STATUS prvCpssPortManagerApPortHCDFound
(
    IN GT_SW_DEV_NUM         devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum
)
{
    CPSS_PORT_AP_STATUS_STC apStatus;
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;


    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum,tmpPortManagerDbPtr,rc);

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperationsBitmap & MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E)
    {
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.fecStatus = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.ifModeDb = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.speedDb = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
        return GT_OK;
    }
    /* query resolution results */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApStatusGetFunc(CAST_SW_DEVNUM(devNum),portNum,&apStatus);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] CPSS_SRVCPU_PORT_802_3_AP_E - ppPortApStatusGetFunc:rc=%d,portNum=%d\n", devNum, portNum, rc, portNum);
    }

    /* resolution found - allocate pizza resources*/
    if (apStatus.hcdFound)
    {
        CPSS_PORT_SPEED_ENT speed;
        CPSS_PORT_SPEED_ENT specialSpeed;
        CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
        CPSS_PORTS_BMP_STC portsBmp;
        /*GT_BOOL locked = GT_FALSE;*/
        GT_U32 portMacNum;
#ifdef CHX_FAMILY
        MV_HWS_PORT_STANDARD  portMode;
#endif
        CPSS_PORT_FEC_MODE_ENT fecType;

        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

        /* Check current speed */
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc(devNum, portNum, &speed);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] CPSS_SRVCPU_PORT_802_3_AP_E "
                                                           "- ppSpeedGetFromExtFunc:rc=%d\n",
                                                           devNum, portNum, rc);
        }
        /* get port interface mode */
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc(devNum, portNum, &ifMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] CPSS_SRVCPU_PORT_802_3_AP_E - ppIfModeGetFromExtFunc:rc=%d\n", devNum, portNum, rc);
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "last speed %d apSpeed %d  last ifmode %d apIfmode %d", speed, apStatus.portMode.speed, ifMode, apStatus.portMode.ifMode);

        /* check if port was already deleted */
        if (speed == CPSS_PORT_SPEED_NA_E)
        {
            return GT_OK;
        }

        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d", devNum, portNum, rc);
        }

        /*if pizza already configured, make sure it is the same as reported (need to check speed only)*/
        if((((speed != CPSS_PORT_SPEED_NA_HCD_E) && (apStatus.portMode.speed != speed)) ||
            ((ifMode != CPSS_PORT_INTERFACE_MODE_NA_HCD_E) && (apStatus.portMode.ifMode != ifMode)))&&
           (!CPSS_PORT_SPEED_SPECIAL_MODE(speed)) && (!(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.portOperationsBitmap & MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E)))
        {

#ifdef CHX_FAMILY
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                mvHwsAnpPortSkipResetSet(devNum,portMacNum,GT_TRUE);
#endif
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc(devNum, portsBmp, GT_FALSE, ifMode
            + CPSS_PORT_INTERFACE_MODE_NA_E, speed + CPSS_PORT_SPEED_NA_E, tmpPortManagerDbPtr->portMngSmDb[portNum]);
#ifdef CHX_FAMILY
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                mvHwsAnpPortSkipResetSet(devNum,portMacNum,GT_FALSE);
#endif
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] CPSS_SRVCPU_PORT_802_3_AP_E - ppPortModeSpeedSetFunc:rc=%d\n", devNum, portNum, rc);
            }
            speed = CPSS_PORT_SPEED_NA_HCD_E;
        }

        /* allocate pizza resources only if not allocated before*/
        if(speed == CPSS_PORT_SPEED_NA_HCD_E)
        {
            specialSpeed = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[0].speedDb;
            if ( CPSS_PORT_SPEED_SPECIAL_MODE(specialSpeed))
            {
                apStatus.portMode.speed = specialSpeed;
                apStatus.portMode.ifMode = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[0].ifModeDb;
            }
            fecType = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility = apStatus.fecType;
             rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc
                 (devNum, portsBmp, GT_TRUE,
                 apStatus.portMode.ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, apStatus.portMode.speed + CPSS_PORT_SPEED_NA_E, tmpPortManagerDbPtr->portMngSmDb[portNum]);
            tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility = fecType;
            /* if ( locked )
             {
                 mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);
                 locked = GT_FALSE;
             }*/
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] CPSS_SRVCPU_PORT_802_3_AP_E - ppPortModeSpeedSetFunc:rc=%d ifMode:%d speed:%d fec %d\n",devNum, portNum, rc, apStatus.portMode.ifMode, apStatus.portMode.speed, apStatus.fecType);
            }

            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPtpDelayParamsSetFunc(devNum,portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortPtpDelayParamsSetFunc failed=%d\n",devNum, portNum,rc);
                }
            }
        }
#ifdef CHX_FAMILY
        else  if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
             /* translate cpss ifMode and speed to single Hws port mode */
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,apStatus.portMode.ifMode, apStatus.portMode.speed, &portMode);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d",devNum, portNum, rc );
            }

            /* Set pcs_cfg_done (ANP_RF)
               m_RAL_ANP.anp_units_RegFile.Port_Status[port_num].p_pcs_cfg_done.set(1'b1); */
           CHECK_STATUS(mvHwsAnpCfgPcsDone(devNum, portMacNum, portMode));

        }
#endif

#if 0
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP port enable ");
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PortEnable failed=%d for port=%d", rc, portNum);
            }
        }
#endif
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.fecStatus = apStatus.fecType;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.ifModeDb = apStatus.portMode.ifMode;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.speedDb = apStatus.portMode.speed;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.fecStatus = apStatus.fecType;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.ifModeDb = apStatus.portMode.ifMode;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.speedDb = apStatus.portMode.speed;
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerStopAdaptive
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr
)
{
    GT_STATUS               rc = GT_OK;
    MV_HWS_PORT_STANDARD    portMode;

    CPSS_NULL_PTR_CHECK_MAC(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneStatusGetFunc);

    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;

    if (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
    {
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

        if(cpssDeviceRunCheck_onEmulator())
        {
            rxTuneStatus = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
            txTuneStatus = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
        }
        else
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneStatusGetFunc(devNum, portNum,
                                                                                      &rxTuneStatus, &txTuneStatus);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppTuneStatusGetFunc failed %d ", rc );
                return rc;
            }
            if ((rxTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E) &&
                    (tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted == GT_FALSE))
            {
                /* if application tries to delete port under enhance tune or when connecting to peer with different speed,
                   this delay is required in additon to sbus reset */
                cpssOsTimerWkAfter(100);
            }
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stopping Adaptive portMode %d ", portMode);
            /* Stop Adaptive anyways */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc(devNum,
                        portNum,CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortStageTrainMngPhases returned error code %d ",
                                                           devNum, portNum, rc);
            }
            tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted = GT_FALSE;
        }
    }

    return rc;
}

static GT_STATUS prvCpssPortManagerTx2RxLoopbackPortEnable
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
        GT_STATUS rc;
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port enable with TX2RX MAC/SerDex loopback");
        /* mask low level interrupts */
        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
        }
        /* Enable port */
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Enable returned error code %d ", rc);
            return rc;
        }

        return GT_OK;
}

/**
* @internal prvCpssPortManagerLinkUpCheck function
* @endinternal
*
* @brief   checking linkup status and update port manager DB
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] Notify                - checking link status change
*
* @param[inout] tmpPortManagerDbPtr - port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported device
* @retval GT_BAD_PARAM             - on bad physical port number
*/
GT_STATUS prvCpssPortManagerLinkUpCheck
(
    IN    GT_SW_DEV_NUM            devNum,
    IN    GT_PHYSICAL_PORT_NUM     portNum,
    IN    GT_BOOL                  Notify,
    INOUT PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr
)
{
    GT_STATUS                             rc = GT_OK;
    GT_BOOL                               linkUp;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;
    GT_BOOL                               anyLoopback;
    MV_HWS_PORT_INIT_INPUT_PARAMS         portInitInParam;
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32 hwsPhyPortNum;

    cpssOsMemSet(&portInitInParam, 0, sizeof(MV_HWS_PORT_INIT_INPUT_PARAMS));

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusGetFunc(devNum, portNum, &linkUp);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppLinkStatusGetFunc returned %d ", devNum, portNum, rc);
    }
    if (( linkUp == GT_TRUE)&& (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_UP_E))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port already in Link Up");
        return GT_OK;
    }

    /* loopback configuration - get from DB */
    rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &loopbackCfg, &anyLoopback);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",devNum, portNum, rc);
    }

    if(tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort == GT_TRUE)
    {
        if (linkUp)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"     --- Link Up ---");
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"     --- Link Down ---");
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
        }
        return GT_OK;
    }

    if (linkUp && (tmpPortManagerDbPtr->portMngSmDb[portNum]->autoNegotiation.inbandEnable && !tmpPortManagerDbPtr->portMngSmDb[portNum]->autoNegotiation.byPassEnable) && PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        portInitInParam.autoNegEnabled = GT_TRUE;
        portInitInParam.portSpeed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;

        rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb, portInitInParam.portSpeed, &portMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &hwsPhyPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppUsxReplicationSetFunc != NULL && HWS_USX_PORT_MODE_FAMILY_CHECK(portMode))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppUsxReplicationSetFunc(devNum, 0, hwsPhyPortNum, portMode, &portInitInParam, GT_TRUE );
        }
    }

    if (Notify)
    {
        if (!(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusChangedNotifyFunc(devNum, portNum, tmpPortManagerDbPtr->portMngSmDb[portNum]->portType, linkUp);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppLinkStatusChangedNotifyFunc on port %d returned %d ", devNum, portNum, rc);
            }
        }
        else /* AC3 based */
        {
            if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM != GT_TRUE &&
                 PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortEnableWaWithLinkStatusSet != NULL)
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortEnableWaWithLinkStatusSet(devNum, portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDxChPortEnableWaWithLinkStatusSet: error, rc=%d\n",
                            devNum, portNum,rc);
                }
            }
        }
    }

    if (linkUp)
    {
        #ifdef RUN_ADAPTIVE_CTLE
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_LINK_UP_E)
        {
            rc = cpssDxChPortAdaptiveCtleBasedTemperatureSendMsg(devNum,portNum,portMode,linkUp);
        }
        #endif
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"     --- Link Up ---");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
        prvCpssPortManagerStatTimeGet(tmpPortManagerDbPtr->portMngSmDb[portNum], &tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkUpTime);
        tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkOkCnt++;
    }
    else
    {
        #ifdef RUN_ADAPTIVE_CTLE
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
        {
            rc = cpssDxChPortAdaptiveCtleBasedTemperatureSendMsg(devNum,portNum,portMode,linkUp);
        }
        #endif
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"     --- Link Down ---");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
    }

    return rc;
}

/**
* @internal prvCpssPortManagerDisable
* @endinternal
*
* @brief  function for handling disable event
*    Disable event putting the link to down. In disable only the
*    hardware components are powered off but all the PM
*    attributes are saved.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] tmpPortManagerDbPtr   - port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerDisable
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr
)
{
    GT_STATUS               rc;
    CPSS_PORT_SPEED_ENT     speed;
    MV_HWS_PORT_STANDARD    portMode;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;
    GT_BOOL                 anyLoopback;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled == GT_TRUE)
    {
        if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum, NON_SUP_MODE, 0, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortLkbSetFunc failed=%d", devNum, portNum, rc);
            }
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Port %d/%2d] Link Binding not supported", devNum, portNum);
        }
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = prvCpssPortManagerStopAdaptive(devNum, portNum, tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerStopAdaptive failed %d ", rc );
            return rc;
        }
    }

    prvCpssPortManagerResetCounters(portNum,tmpPortManagerDbPtr);

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssLion2CpssIfModeToHwsTranslate returned rc %d",devNum, portNum, rc );
    }

    if ( HWS_USX_MULTI_LANE_MODE_CHECK(portMode) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"[Port %d/%2d] port disable operation not supported for QSGMII ifMode", devNum, portNum);
    }

    rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_FALSE);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                   " returned error code %d ",
                                                   devNum, portNum, rc);
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"unmasking port");
    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
    if (PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal = GT_TRUE;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwResetFunc(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortHwResetFunc"
                                                       " returned error code %d ",
                                                       devNum, portNum, rc);
        }
    }
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;
    if ((PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(speed)) || (PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        /* loopback configuration - get from DB */
        rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &loopbackCfg, &anyLoopback);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",devNum, portNum, rc);
        }

        if (PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            /* set loopback to disable */
            rc = prvCpssPortManagerLoopbackModeInit(portNum,loopbackCfg.loopbackType,&loopbackCfg);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModeInit failed=%d",
                                                       devNum, portNum, rc);
            }
            /* set disable state to HW */
            rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                           devNum, portNum, rc);
            }
        }
    }

    return rc;
}

GT_STATUS prvCpssPortManagerApIsConfigured
(
    IN GT_U8                  devNum,
    IN GT_U32                 portNum,
    OUT GT_BOOL               *apConfiguredPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC        *tmpPortManagerDbPtr;
    GT_STATUS rc;
    *apConfiguredPtr = GT_FALSE;
    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);
    if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E) ||
        (tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw))
    {
        *apConfiguredPtr = GT_TRUE;

    }
    return GT_OK;
}

/** @internalprvCpssPortManagerApEventSet
* @endinternal
*
* @brief  Handling events from the port manager, and set them in  ap ports
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] stage                 - pm event
* @param[in] tmpPortManagerDbPtr   - om database ptr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerApEventSet
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_MANAGER_EVENT_ENT     stage,
    IN PRV_CPSS_PORT_MNG_DB_STC        *tmpPortManagerDbPtr
)
{
    GT_STATUS                 rc; /* return code */
    GT_U32                    laneIndex, i, advertisedNum = 0, portGroup;
    GT_BOOL                   linkUp;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    MV_HWS_PORT_STANDARD      portMode;
    GT_U32                    portMacNum;
    GT_U32                    interconnectProfile;
    /* serdes params*/
    GT_U32                    dbSdValidBitMap = 0;
    CPSS_PORT_SERDES_TUNE_STC serdesParams;
    GT_U8 maxLanes = 0;
    GT_BOOL profileUpdate = GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

#ifdef PX_FAMILY
    CPSS_PORT_MANAGER_LKB_CONFIG_STC    *pLinkBinding;
#endif

    /* AP port will be configured only in this next scope */
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

    if(CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E != stage)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP port eventSet: Event=%d, PM Status %d", stage,
                                                   tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM);
    }
    /* getting appropriate mac num for the given physical port num */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d",devNum, portNum, rc);
    }

    switch (stage)
    {
    CPSS_PORT_AP_PARAMS_STC apParams;

    case CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E:
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP - CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E , create port in FW");
        GT_ATTR_FALLTHROUGH;
    case CPSS_PORT_MANAGER_EVENT_CREATE_E:
    case CPSS_PORT_MANAGER_EVENT_ENABLE_E:
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) &&
           (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.enable))
        {
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartCnt = 0;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_NO_PARALLEL_DETECT_E;
        }
        interconnectProfile = 0;
        if ( (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.interconnectProfile) )
        {
            profileUpdate = GT_TRUE;
        }

        for (i = 0; i < CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
        {
            apParams.modesAdvertiseArr[i].ifMode = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb;
            apParams.modesAdvertiseArr[i].speed  = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb;
            apParams.fecAbilityArr[i]            = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fecAbilityArr[i];
            apParams.fecRequestedArr[i]          = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fecRequestedArr[i];

            /* old mode for 10G/40G fec */
            if ((apParams.modesAdvertiseArr[i].speed == CPSS_PORT_SPEED_10000_E) || (apParams.modesAdvertiseArr[i].speed == CPSS_PORT_SPEED_40000_E) ||
                (apParams.modesAdvertiseArr[i].speed == CPSS_PORT_SPEED_42000_E))
            {
                apParams.fecRequired   = (apParams.fecRequestedArr[i] == CPSS_PORT_FEC_MODE_DISABLED_E) ? GT_FALSE : GT_TRUE;
                apParams.fecSupported  = (apParams.fecAbilityArr[i] == CPSS_PORT_FEC_MODE_DISABLED_E) ? GT_FALSE : GT_TRUE;
            }

            if ((tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb != CPSS_PORT_INTERFACE_MODE_NA_E)&&
                (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb  != CPSS_PORT_SPEED_NA_E) &&
                (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb  != CPSS_PORT_SPEED_10_E) &&
                (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E))
            {
                /* Getting HWS portMode for the port*/
                rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb
                                                           , tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb, &portMode);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d",devNum, portNum, rc );
                }

                if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null", devNum, portNum );
                }

                if (maxLanes < curPortParams.numOfActLanes)
                {
                    maxLanes = curPortParams.numOfActLanes;
                }
            }
        }

        apParams.fcAsmDir      = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fcAsmDir;
        apParams.fcPause       = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.fcPause;
        apParams.laneNum       = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.laneNum;
        apParams.noneceDisable = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable;


        if (profileUpdate)
        {
            interconnectProfile = (GT_U32)tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.interconnectProfile;
            rc = mvHwsApSerdesInterconnectProfileConfigSet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, maxLanes, interconnectProfile);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d]  mvHwsPortApSerdesAttributesConfigSetIpc failed- %d", devNum, portNum, rc);
            }
        }

        /* going through all port mode lanes, for each lane checking if there are tx/rx
                params in db, then send the values, lane and port mode to AP*/
        for (advertisedNum = 0; advertisedNum < CPSS_PORT_AP_IF_ARRAY_SIZE_CNS; advertisedNum++)
        {
            /* checking for advertising speed and mode*/
            if ((tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[advertisedNum].ifModeDb != CPSS_PORT_INTERFACE_MODE_NA_E)&&
                (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[advertisedNum].speedDb  != CPSS_PORT_SPEED_NA_E) &&
                /*(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[advertisedNum].speedDb  != CPSS_PORT_SPEED_10_E) &&*/
                (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[advertisedNum].ifModeDb != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E))
            {
                /* Getting HWS portMode for the port*/
                rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[advertisedNum].ifModeDb
                                                           , tmpPortManagerDbPtr->portsApAttributesDb[portNum]->perPhyPortDb[advertisedNum].speedDb, &portMode);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d", devNum, portNum, rc );
                }

                if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null", devNum, portNum );
                }

                for (laneIndex = 0; laneIndex < curPortParams.numOfActLanes; laneIndex++)
                {
                    rc = prvCpssPortManagerLaneTuneDbGet(devNum, portNum, advertisedNum,laneIndex,
                                CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, &serdesParams, &dbSdValidBitMap);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneDbGet failed=%d", devNum, portNum, rc);
                    }

                    if (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
                    {
                        rc = prvCpssPortManagerApLaneTuneHwSet(devNum, portGroup, portNum, advertisedNum, laneIndex, curPortParams,GT_FALSE);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerApLaneTuneHwSet failed=%d on Tx", devNum, portNum,rc);
                        }
                    }

                    if (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
                    {
                        rc = prvCpssPortManagerApLaneTuneHwSet(devNum, portGroup, portNum, advertisedNum, laneIndex, curPortParams,GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerApLaneTuneHwSet failed=%d on Rx", devNum, portNum,rc);
                        }
                    }
                    if ( profileUpdate )
                    {
                        rc = prvCpssPortManagerApLaneTxPresetHwSet(devNum, portGroup, portNum, portMode, laneIndex, curPortParams, interconnectProfile);

                        if (rc!=GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d]  mvHwsPortApSerdesAttributesConfigSetIpc failed- %d",devNum, portNum, rc);
                        }

                    }
                }
            }
        }

#ifdef PX_FAMILY
        pLinkBinding = &(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.linkBinding);
        if (pLinkBinding->enabled) {
            rc = mvHwsPortLkbRegisterApPortSet(CAST_SW_DEVNUM(devNum), portGroup, (GT_U16)portNum, (GT_U16)pLinkBinding->pairPortNum);
            if (rc != GT_OK)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding register failed=%d", devNum, portNum, rc);
        }
#endif
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.fecStatus = CPSS_PORT_FEC_MODE_LAST_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.speedDb = CPSS_PORT_SPEED_NA_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.fecStatus = CPSS_PORT_FEC_MODE_LAST_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.speedDb = CPSS_PORT_SPEED_NA_E;

        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApConfigSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                    GT_TRUE, &apParams,
                                                                                    tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.portOperationsBitmap,
                                                                                    tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.skipRes);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] creating ap port failed=%d", devNum, portNum, rc);
        }
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_TRUE;
        if (stage == CPSS_PORT_MANAGER_EVENT_ENABLE_E)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
        }

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if  (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            rc = prvCpssPortManagerApPortHCDFound(devNum, portNum);
            if (rc!=GT_OK)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] apHdcFound failed=%d", devNum, portNum, rc);

            /*check if link is up */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusGetFunc(devNum, portNum, &linkUp);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppLinkStatusGetFunc returned %d ", devNum, portNum, rc);
            }
            if (linkUp == GT_TRUE)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
            }
        }
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* un-mask mac and low level interrupts */
            rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
            }
        }

        break;

    case CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E:
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP - CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E , delete port from FW");
        GT_ATTR_FALLTHROUGH;
     case CPSS_PORT_MANAGER_EVENT_DELETE_E:
        /* del LKB from FW */
        if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.linkBinding.enabled == GT_TRUE)
        {
            if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum, NON_SUP_MODE, 0, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding delete port failed=%d", devNum, portNum, rc);
                }
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %d/%2d] Link Binding not supported", devNum, portNum);
            }
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPtpDelayParamsSetFunc(devNum,portNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortPtpDelayParamsSetFunc failed=%d\n", devNum, portNum,rc);
            }
        }
#ifdef CHX_FAMILY
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) &&
           ((tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E) ||
            (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_SUCCESS_E)))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect status = %d resert back AN",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
            rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
            }
        }
#endif
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApConfigSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                    GT_FALSE, &apParams, 0, GT_FALSE);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] deleting ap port failed=%d", devNum, portNum, rc);

        if (stage == CPSS_PORT_MANAGER_EVENT_DELETE_E)
        {
            rc = prvCpssPortManagerCleanPortParameters(CAST_SW_DEVNUM(devNum), portGroup,
                                                   portNum, CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E);
            if (rc!=GT_OK)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerCleanPortParameters failed=%d", devNum, portNum, rc);

            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_RESET_E;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_FALSE;
            tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw = GT_FALSE;
        }
        else
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E;
        }

        break;

    case CPSS_PORT_MANAGER_EVENT_DISABLE_E:
        {
#ifdef CHX_FAMILY
            if(( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) &&
               ((tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E) ||
                (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_SUCCESS_E)))
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect status = %d resert back AN", tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                }
            }
#endif
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApConfigSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                        GT_FALSE, &apParams, 0, GT_FALSE);
            if (rc!=GT_OK)
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] deleting ap port failed=%d", devNum, portNum, rc);
        } /* Fall through on purpose since the below status need to be changed for both Events */
        GT_ATTR_FALLTHROUGH;

    case CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E:
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM       = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged       = GT_TRUE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;

        break;

    case CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E:
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            CPSS_PM_START_TIME(tmpPortManagerDbPtr->portMngSmDb[portNum]);
            tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkUpTime = 0;
        }
        rc = prvCpssPortManagerApPortHCDFound(devNum, portNum);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] apHdcFound failed=%d", devNum, portNum, rc);
        break;
    case CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E:
        /*WA implmentation for AN-154*/
        /*****************************/
        /*bug in the ANP combining with the 100G_IP behavior creates an issue that requires a workaround.
        the relevant port modes are only ports where all portions of PCS lock are in sd_rx_clk domain - 5GBASE-R ; 10GBASE-R ; 25GBASE-R (no RS-FEC).
        In the 100G_IP, the block_lock indication goes from sd_rx_clk domain to mac_clk domain through a dual clock memory.
        there is a special sync reset mechanism - once block_lock fails, either due to fail in the block_lock itself, or due to de-assertion of the signal_detect -
        sync_reset is asserted to both sd_rx_clk domain and mac_clk domain. this sync_reset on the mac_clk domain cause the block_lock to drop thus the link_status towards the ANP drops.
        this circuit is self-clearing, assuming both domains are operating, and it will clear the sync_reset.
        now the block_lock is again driven from the CDC memory, but we need at least 1 read so it value will change from the ZEROS it holds due to the sync_reset.
        if the reset from the ANP is coming in the following window -
        after sync_reset is done.
        at least 1 write done to memory.
        less then 64 entries written to memory.
        since there is a write - the write pointer increases, so the read side starts to read.
        then due to the reset, the write pointer becomes ZERO and stays that way - the read side will continue reading until the pointers are equal - that is until read pointer is '0' which means the last entry read is entry 63 - if this entry holds '1' for the block_lock - link_status stays up constantly while reset_sd_rx_clk is asserted.
        the same sync_reset mentioned above that resets the block_lock on the mac_clk domain - is driven from OR, so another way to trigger this reset is by asserting PCS SW reset (CONTROL1[15]) - this will clear the "stuck" link_status.*/
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
             /* translate cpss ifMode and speed to single Hws port mode */
            rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),
                                                       tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.ifModeDb,
                                                       tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.speedDb,
                                                       &portMode);
            if (rc != GT_OK)
            {
                return GT_OK;
            }
            if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &curPortParams))
            {
                return GT_OK;
            }

            if ((curPortParams.portMacType == MTI_MAC_100) && (curPortParams.numOfActLanes == 1))
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E. portNum=%d", portNum);
                mvHwsPcsReset(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode,curPortParams.portPcsType, UNRESET);
            }
        }
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.fecStatus = CPSS_PORT_FEC_MODE_LAST_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.speedDb = CPSS_PORT_SPEED_NA_E;

        break;
    case CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E:
#ifdef CHX_FAMILY
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            if(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.enable)
            {
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartCnt++;

                if((tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartCnt == tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartMaxNum) &&
                   (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_NO_PARALLEL_DETECT_E))
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E try parallel detect");
                    tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E;
                    rc = mvHwsAnpPortParallelDetectInit(devNum,portMacNum);
                    if(rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E parallel detect init fail");
                        rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN");
                        }
                        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_FAIL_E;
                    }
                    else
                    {
                        rc = mvHwsAnpPortParallelDetectWaitForPWRUPprog(devNum,portMacNum);
                        if(rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E parallel detect wait for PWRUP prog fail");
                            rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN");
                            }
                            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_FAIL_E;
                        }
                        if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E )
                        {
                            /*start with AN37 disabled when attempting the parallel detect*/
                            rc = mvHwsAnpPortParallelDetectAutoNegSet(devNum,portMacNum,GT_FALSE);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to set AN37",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                                rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                                if (rc != GT_OK)
                                {
                                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN");
                                }
                                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_FAIL_E;
                            }
                        }
                    }
                }
                else if(tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartCnt > tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartMaxNum)
                {

                    if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E parallel detect attempt fail");
                        tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_FAIL_E;
                        rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                        }
                    }
                    else if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_AN37_CHECK_E)
                    {
                        /*didn't reach link after 50msec - peer AN37 is disabled - switch back to AN37 disabled (was enabled during peer AN37 status)*/
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect peer AN37 is disabled",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                        rc = mvHwsAnpPortParallelDetectAutoNegSet(devNum,portMacNum,GT_FALSE);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to set AN37",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                            rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN");
                            }
                        }
                    }
                }
            }
        }
#endif
        break;
    case CPSS_PORT_MANAGER_EVENT_INIT_E:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %d/%2d] operation not supported for AP port. operation=%d", devNum, portNum, stage);
        break;

    case CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E:
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"operation not needed for AP port. operation=%d", stage);
        break;

    case CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E:
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusGetFunc(devNum, portNum, &linkUp);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppLinkStatusGetFunc failed=%d", devNum, portNum, rc);

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            linkUp ? (prvCpssPortManagerStatTimeGet(tmpPortManagerDbPtr->portMngSmDb[portNum], &tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkUpTime)) :
                     (tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkUpTime = 0);
        }
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (linkUp == GT_TRUE))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP port enable ");
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, linkUp);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PortEnable failed=%d for port=%d", rc, portNum);
            }
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP port mac_level changed, link %d ", linkUp);
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusChangedNotifyFunc(devNum, portNum, CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, linkUp);
        if (rc != GT_OK)
        {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
                            "[Port %d/%2d] Calling ppLinkStatusChangedNotifyFunc on port %d returned %d ", devNum, portNum, rc);
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"AP CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E link %d",linkUp);

        if (linkUp == GT_TRUE)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM =  CPSS_PORT_MANAGER_STATE_LINK_UP_E;
#ifdef CHX_FAMILY
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E )
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "parallel detect link up - check peer AN37");
                    tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_AN37_CHECK_E;
                    rc = mvHwsAnpPortParallelDetectAN37Seq(devNum,portMacNum,GT_TRUE);
                    if(rc != GT_OK)
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "parallel detect - start check peer AN37 - fail");
                }
                else if (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_AN37_CHECK_E)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "parallel detect link up");
                    rc = mvHwsAnpPortParallelDetectAN37Seq(devNum,portMacNum,GT_FALSE);
                    if(rc != GT_OK)
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "parallel detect - end check peer AN37 - fail");

                    tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_SUCCESS_E;
                }
            }
            else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) && (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E))
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "parallel detect link up");
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_PARALLEL_DETECT_SUCCESS_E;
            }
#endif
        }
        else
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM =  CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.fecStatus = CPSS_PORT_FEC_MODE_LAST_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.ifModeDb = CPSS_PORT_INTERFACE_MODE_NA_E;
            tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.speedDb = CPSS_PORT_SPEED_NA_E;
#ifdef CHX_FAMILY
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                   if((tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_SUCCESS_E) ||
                      (tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status == CPSS_PORT_MANAGER_PARALLEL_DETECT_IN_PROGRESS_E))
                   {
                       CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect link down");
                       rc = mvHwsAnpPortParallelDetectReset(devNum,portMacNum);
                       if (rc != GT_OK)
                       {
                           CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"parallel detect fail to resert back to AN",tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status);
                       }
                       tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartCnt = 0;
                       tmpPortManagerDbPtr->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.status = CPSS_PORT_MANAGER_NO_PARALLEL_DETECT_E;
                   }
            }
#endif
        }
        break;
    case CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E:
        {
            /*TODO only if AP SM is in AN_PORT_ENABLE_WAIT
            1) call port disable
            2) call ipc to indicate that the port is disabled */

            MV_HWS_AP_PORT_STATUS    apResult;

            /* query AP SM status */
            rc = mvHwsApPortCtrlStatusGet(devNum, portGroup, portMacNum, &apResult);

            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E - mvHwsApPortCtrlStatusGet:rc=%d,portNum=%d\n", devNum, portNum, rc, portNum);
            }

            if(apResult.smStatus == AP_SM_WAIT_FOR_PORT_DISABLE)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E disbale mac");
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
                if (rc!=GT_OK)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mac disable failed=%d", devNum, portNum, rc);

                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"     --- Change to link Down ---");
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.fecStatus = CPSS_PORT_FEC_MODE_LAST_E;
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.ifModeDb =CPSS_PORT_INTERFACE_MODE_NA_E;
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.speedDb = CPSS_PORT_SPEED_NA_E;
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.fecStatus = CPSS_PORT_FEC_MODE_LAST_E;
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.ifModeDb =CPSS_PORT_INTERFACE_MODE_NA_E;
                tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portLastResolution.speedDb = CPSS_PORT_SPEED_NA_E;

                rc = mvHwsApPortEnableWaitAck(devNum, portGroup, portMacNum);
                if (rc!=GT_OK)
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] send enable wait failed=%d", devNum, portNum, rc);
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E AP SM = %d, do nothing",apResult.smStatus);
            }
        }
        break;

    case CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E:
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E ");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged       = GT_TRUE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;

        break;
    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] ap port bad switch-case operation=%d", devNum, portNum, stage);
        break;
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerRxStabilityTimeCheck
(
    IN GT_SW_DEV_NUM             devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN MV_HWS_PORT_STANDARD      portMode,
    IN PRV_CPSS_PORT_MNG_DB_STC  *tmpPortManagerDbPtr
)
{
    GT_STATUS  rc; /* return code */
    GT_U32 timerSecondsNow;
    GT_U32 timerNsecNew;
    GT_U32 timeout = 4000000000UL;

    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (!HWS_PAM4_MODE_CHECK(portMode))) {
        timeout = 1200000000UL;
    }
    /* Timeout check 1Sec */
    if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart > (timeout) &&
         tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries > 20/*40*/ )
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Training timeout reached, moving to link down");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart   = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev     = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev        = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries                = 0;

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
        {
            /* reset Optical calibration counters */
            prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
        }

        /* tbd del if (tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled == GT_TRUE)
            * del LKB from FW *
            mvHwsPortLkbPortSet(devNum, tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup,
                                portNum, 0, GT_FALSE); */

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    /* Initialize timers if this is the first iteration in the algorithm */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev == 0 &&
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev    == 0)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart = 0;
        rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev,
                          &tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev);
        if (rc!=GT_OK)
        {
            prvCpssPortManagerModeToDownState(devNum,portNum);
             CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Timer issue - moving to link down", devNum, portNum);
        }
    }

    rc = cpssOsTimeRT(&timerSecondsNow,&timerNsecNew);
    if (rc != GT_OK)
    {
        prvCpssPortManagerModeToDownState(devNum,portNum);
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Timer issue - moving to link down", devNum, portNum);
    }

    if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerSecondsPrev)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime =
        ( 1000000000 - (tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev) + timerNsecNew );
    }
    /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime =
        ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev) );
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerSecondsPrev = timerSecondsNow;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimerNsecPrev = timerNsecNew;

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTotalTimeFromStart+=
        tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTimeDiffFromLastTime;

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> rxStabilityTrainTries++;

    return GT_OK;
}

/**
* @internalprvCpssPortManagerResetAlignCounters
* @endinternal
*
* @brief reset the timer in the port manager database (per port)
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] tmpPortManagerDbPtr   - port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssPortManagerResetAlignCounters
(
     IN GT_PHYSICAL_PORT_NUM      portNum,
     IN PRV_CPSS_PORT_MNG_DB_STC  *tmpPortManagerDbPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(tmpPortManagerDbPtr);

    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portDbAlignLockDone           = CPSS_PORT_MANAGER_RESET_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimeDiffFromLastTime = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimerSecondsPrev     = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimerNsecPrev        = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTotalTimeFromStart   = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTimeIntervalTimer    = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]-> portAlignTries                = 0;

    return GT_OK;
}

/**
* @internal prvCpssPortManagerAlignTimerCheck
* @endinternal
*
* @brief  function that checking align lock done by setting
*         timer and checking for interval of time for
*         overallStatusesOk (gbLock, sigDet and alignLock =
*         true). each time we have status ok, increasing counter
*         by one, at 3 counters we have align lock
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] tmpPortManagerDbPtr   - port manager DB
* @param[in] overallStatusesOk     - gbLock, sigDet and
*                                     alignLock status
*
* @param[out] isAlign              - return true on align lock
* @param[out] timeout              - return true when timeout
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerAlignTimerCheck
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr,
    IN GT_BOOL              overallStatusesOk,
    OUT GT_BOOL             *isAlign,
    OUT GT_BOOL             *timeout
)
{
    GT_STATUS  rc; /* return code */
    GT_U32 timerSecondsNow;
    GT_U32 timerNsecNew;

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTries++;

    CPSS_NULL_PTR_CHECK_MAC(isAlign);
    CPSS_NULL_PTR_CHECK_MAC(timeout);
    *isAlign = GT_FALSE;
    *timeout = GT_FALSE;
    /* advancing alignlock timers */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev == 0
        && tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev == 0)
    {
        /*portDbStabilityDone=CPSS_PORT_MANAGER_IN_PROGRESS_E;*/
        rc = cpssOsTimeRT(&tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev,
                          &tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Alignlock timer issue", devNum, portNum);
        }
    }

    rc = cpssOsTimeRT(&timerSecondsNow, &timerNsecNew);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Alignlock timer issue", devNum, portNum);
    }
    /* In case we in different time in meanning of a second, the difference in nano seconds
      is the following equation */
    if (timerSecondsNow > tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime =
        ( (1000000000 - tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev) + timerNsecNew );
    }
    /* If we at the same time in meaning of a second, the difference in nanosec is the simple difference */
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime =
        ( (timerNsecNew - tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev) /*/ 1000000*/ );
    }

    /* update the port seconds and nanosecs timers */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerSecondsPrev = timerSecondsNow;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimerNsecPrev    = timerNsecNew;

    /* updating total time spent */
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart+=
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeIntervalTimer +=
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeDiffFromLastTime;

    /* if interval time passed, checking the status. We want statuses to be steady
      for X tries but also over Y time*/
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeIntervalTimer >= 1000000/*0*/)
    {
        /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"asking ");*/
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTimeIntervalTimer = 0;
        if (overallStatusesOk == GT_TRUE)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ALIGN: alignLockDone %d, stabilityDone %d, trainDone %d. portAlignTotalTimeFromStart %dms cnt %d",
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone,
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone,
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone,
                                                  (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart/1000000),
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->logCounter);
            prvCpssPortManagerResetAlignCounters(portNum,tmpPortManagerDbPtr);
            *isAlign = GT_TRUE;
            return GT_OK;
        }
    }
    else
    {
        /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"not yet, returning");*/
        return GT_OK;
    }


    /* maximux timeout */
    if ( ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart)) >= 4000000000UL &&
              tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTries>10)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"TIMEOUT: alignLockDone %d, stabilityDone %d, trainDone %d. portAlignTotalTimeFromStart %dms",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone,
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone,
                                              (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAlignTotalTimeFromStart/1000000));
        prvCpssPortManagerResetAlignCounters(portNum,tmpPortManagerDbPtr);
        *timeout = GT_TRUE;
    }
    return GT_OK;
}

GT_STATUS prvCpssPortManagerGetAttributes
(
    IN    GT_SW_DEV_NUM                         devNum,
    IN    GT_PHYSICAL_PORT_NUM                  portNum,
    INOUT PRV_CPSS_PORT_MNG_DB_STC              *tmpPortManagerDbPtr,
    OUT   CPSS_PORT_MANAGER_PORT_TYPE_ENT       *portTypePtr,
    OUT   CPSS_PORT_SPEED_ENT                   *speedPtr,
    OUT   CPSS_PORT_INTERFACE_MODE_ENT          *ifModePtr,
    OUT   MV_HWS_PORT_STANDARD                  *portModePtr,
    OUT   GT_U32                                *portGroupPtr,
    OUT   CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *loopbackCfgPtr,
    OUT   GT_BOOL                               *anyLoopbackPtr,
    OUT   GT_BOOL                               *ignoreTrainingPtr
)
{
    GT_STATUS    rc;

    CPSS_NULL_PTR_CHECK_MAC(portTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);
    CPSS_NULL_PTR_CHECK_MAC(ifModePtr);
    CPSS_NULL_PTR_CHECK_MAC(portModePtr);
    CPSS_NULL_PTR_CHECK_MAC(portGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(loopbackCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(anyLoopbackPtr);
    CPSS_NULL_PTR_CHECK_MAC(ignoreTrainingPtr);

    *portTypePtr = tmpPortManagerDbPtr->portMngSmDb[portNum]->portType;

    if (*portTypePtr >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] **invalid portType %d", devNum, portNum, *portTypePtr);
    }

    if (*portTypePtr == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %d/%2d] ap not support %d", devNum, portNum, *portTypePtr);
    }

    *ifModePtr = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    *speedPtr = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    *portModePtr = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    *portGroupPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

    tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup = *portGroupPtr;

    rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, *portTypePtr, loopbackCfgPtr, anyLoopbackPtr);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   devNum, portNum, rc);
    }
    *ignoreTrainingPtr = GT_FALSE;

    if ((loopbackCfgPtr->loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) &&
        (loopbackCfgPtr->loopbackMode.serdesLoopbackMode == CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E))
    {
        /*ignore training */
        if ((PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(*portModePtr)) || (HWS_10G_MODE_CHECK(*portModePtr)) || (HWS_40G_R4_MODE_CHECK(*portModePtr)) || (*portModePtr == _40GBase_KR2))
        {
            *ignoreTrainingPtr = GT_TRUE;
        }
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"config SERDES_LOOPBACK TX2RX enableRegularTrafic %d ignore training %d portMode %d",loopbackCfgPtr->enableRegularTrafic, *ignoreTrainingPtr, *portModePtr);
    }
    else
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            *ignoreTrainingPtr = GT_TRUE;
        }
    }

    tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->ignoreTraining = *ignoreTrainingPtr;

    return GT_OK;
}

GT_STATUS prvCpssPortManagerInitializeSignalStabilityFilerPhase
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  MV_HWS_PORT_INIT_PARAMS                *curPortParamsPtr,
    IN  PRV_CPSS_PORT_MNG_DB_STC               *tmpPortManagerDbPtr
)
{
    GT_STATUS                               rc;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_SPEED_ENT                     speed;
    MV_HWS_PORT_STANDARD                    portMode;
    GT_BOOL                                 ignoreTraining;
    PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC portStatuses;
    PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC portStatusesStability;

    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    ignoreTraining = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->ignoreTraining;

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected == GT_FALSE)
    {
        rc = prvCpssPortManagerStatusesStabilityCheckPhase(devNum, portNum, &portStatusesStability,
                                                           PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E), curPortParamsPtr->numOfActLanes);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerStatusesStabilityCheckPhase returned %d", devNum, portNum, rc);
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == CPSS_PORT_MANAGER_IN_PROGRESS_E)
        {
            return GT_OK;
        }

        if (!(portStatusesStability.sigDet))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Signal stability failed, Unmasking port caused signal stability issue");

            prvCpssPortManagerModeToDownState(devNum,portNum);

            if ((!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))  && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
            {
                rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", devNum, portNum, rc);
                }
            }

            return GT_OK;
        }
    }

    /***************************************
      1. Getting port statuses
    ****************************************/
    /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"getting all port statuses ");*/
    /* signal - for all lanes */
    if ( PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)  || PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(ifMode))
    {
        rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E));
        /* in low speeds (by here only 1000baseX) and in port modes that use XPCs we don't care about gear box or align lock,
          only signal. So now that we passed signal stability, we can check the signal itself */
        portStatuses.alignLock = GT_TRUE;
        portStatuses.gbLock = GT_TRUE;
    }
    else
    {
        rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) |
                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E) |
                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E));
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"getting all port statuses: alignLock %d gbLock %d sigDet %d overallStatusesOk %d ",
                                          portStatuses.alignLock,
                                          portStatuses.gbLock,
                                          portStatuses.sigDet,
                                          portStatuses.overallStatusesOk);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortStatusesCheck returned %d", devNum, portNum, rc);
    }

    /***************************************
      2. GearBox-lock and Align-lock check
    ****************************************/
    /* If signal was up and training launched and finished (which on success
      can cause those statuses to be good), then, those low level MAC units can be checked */

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected == GT_TRUE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"section of gbLock and align lock.");

        /* Loss of any of those units will cause us to disable port */
        if (((curPortParamsPtr->numOfActLanes > 1) && (portStatuses.alignLock == GT_FALSE || portStatuses.gbLock == GT_FALSE)) ||
            ((!(curPortParamsPtr->numOfActLanes > 1)) && (portStatuses.gbLock == GT_FALSE)))
        {

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"section of gbLock and align lock. align %d gb %d",
                                                  portStatuses.alignLock, portStatuses.gbLock);

            /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum); */
            tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;

            /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Unmasking port cause signal not good so only finish and unmask ");

            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling cpssEventDeviceMaskWithEvExtDataSet returned %d", devNum, portNum, rc);
            }

            /* disable port high level unit */
            /* if we reach this point, it means we GB is not locked or no alignment so no need to force Remote Fault, it will
                           generated automaticly since the port will be in fault condition aka local fault */
            tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault = GT_TRUE;
            rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_FALSE);
            if (rc!=GT_OK)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault = GT_FALSE;
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                           " returned error code %d ",
                                                           devNum, portNum, rc);
            }

            rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
            }

            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType ==
                    CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
            {
                /* reset Optical calibration counters */
                prvCpssPortManagerResetOpticalCalCounters(portNum, tmpPortManagerDbPtr);
            }

            /* del LKB from FW, in case cable is disconnected */
            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled == GT_TRUE)
            {
                if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
                {
                    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum, NON_SUP_MODE, 0, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding delete port failed=%d", devNum, portNum, rc);
                    }
                }
                else
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %d/%2d] Link Binding not supported", devNum, portNum);
                }
            }
            prvCpssPortManagerModeToDownState(devNum,portNum);

            if(!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode) && (!ignoreTraining) && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
            {
                rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", devNum, portNum, rc);
                }
            }
            /* no furthur port handling needed */

            return GT_OK;
        }
        /* Only signal detect event will cause us to launch training. Gb lock or align lock
          will not cause performing training. */
    }

    /*******************************
       3. Signal state
    ********************************/
    /* If signal detected and signal was up before, skip to next port
      bacause there is nothing more to do since the previous time we got signal detect
      and perform stability check than training. We want to avoid the situation where
      we finish train, getting signal event, seing that is up, do another train,
      getting event again... infinite loop. */
    if (portStatuses.sigDet == GT_TRUE && tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected == GT_TRUE)
    {
        /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum);*/
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"unmasking port cause signal still up, no change ");
        /* here do not reset optical counters, as this point means the port finished calibration but the peer is not up yet */

        rc = prvCpssPortManagerLinkUpCheck(devNum, portNum, GT_TRUE, tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
            prvCpssPortManagerModeToDownState(devNum,portNum);
            return rc;
        }

        /* no furthur port handling needed */

        /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/
        /* state machine should not change */

        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        return GT_OK;
    }

    /*******************************
       4. Signal stability
    ********************************/
    /* If signal is detected and signal was down before, continue to signal stability check*/
    if (portStatuses.sigDet == GT_TRUE)
    {
        /*************************************
         By here, signal was detected as UP, and its the first UP from the last DOWN.
         So, from this point and until we finish handle the signal which is after
         stability check, training, and port enable, the signal interrupt can be masked.
         This mask is the CPSS way to ensure the port init sequence completeness.
         It will be unmask only at the end of the whole handling. (also in a fail point between
         here and the end, when we fail and breaking the handling, we unmask)
        *************************************/
    }

    if (portStatuses.sigDet == GT_FALSE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"signal or signal stability not good on port");
        /*CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsBmp, portNum);*/
        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"unmasking port cause signal not good so only finish and unmask ");

        /* del LKB from FW */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled == GT_TRUE)
        {
            if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum, NON_SUP_MODE, 0, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding delete port failed=%d", devNum, portNum, rc);
                }
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Link Binding not supported");
            }
        }

        /* no furthur port handling needed */
        /*CPSS_PORTS_BMP_PORT_SET_MAC(&tmpBitmapForDisable, portNum);*/
        if(!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)  && (!ignoreTraining) && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
        {
            rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", devNum, portNum, rc);
            }
        }
        prvCpssPortManagerModeToDownState(devNum,portNum);
        return GT_OK;
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_TRUE;

    /**********************************************************
      Pre-Training [1/3] - set default rx\tx parameters
    **********************************************************/
    /* applying DB defaults to serdes */
    /* INSTEAD OF HIGHTEST SQUELCH => SETTING THE USER DEFAULTS */

    if ((!ignoreTraining) && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
    {
        rc = cpssPortSerdesCyclicAutoTuneDefaultsSet(CAST_SW_DEVNUM(devNum), portMode,
                                                     &tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling cpssPortSerdesCyclicAutoTuneDefaultsSet returned %d ", rc);
            return rc;
        }

        /* override tune related per user request before performing the training sequence */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E))
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.trainSequence.tune =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideTrainMode;
        }
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E))
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.trainSequence.adaptiveEnabled =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported;
        }
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E))
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->trainCookiePtrEnclosingDB.cookie.trainSequence.edgeDetectEnabled =
            tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideEdgeDetectSupported;
        }

        /* Launching the auto tune. */
        /**********************************************************
          Pre-Training [3/3] - Launching the auto tune
        **********************************************************/
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling rx-tune on port ");

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling tune2 on port ");
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerInitializeTunePhase
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  PRV_CPSS_PORT_MNG_DB_STC               *tmpPortManagerDbPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
)
{
    GT_U32                                 i;
    GT_STATUS                              rc;
    CPSS_PORT_INTERFACE_MODE_ENT           ifMode;
    CPSS_PORT_SPEED_ENT                    speed;
    GT_BOOL                                sip6RunAdaptive;
    MV_HWS_PORT_STANDARD                   portMode;
    MV_HWS_PORT_INIT_PARAMS                curPortParams;      /* parameters of current port */
    GT_32                                  portGroup;

    ifMode    = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    speed     = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    portMode  = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    portGroup = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* if application has override this, then get it from override db */
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap &
             PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E))
        {
            sip6RunAdaptive = tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported;
        }
        else
        {
            sip6RunAdaptive = GT_TRUE;
        }

        if (sip6RunAdaptive)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc(devNum, portNum, CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppTuneExtSetFunc - rc %d ", devNum, portNum, rc);
            }
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_DONE_E;
        *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
    }
    else if ((!tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->ignoreTraining) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = prvCpssPortStageTrainMngPhases(devNum, portNum, tuneStatusPtr);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortStageTrainMngPhases - rc %d ", devNum, portNum, rc);
        }
    }
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_DONE_E;
        *tuneStatusPtr = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
    }

    if (CPSS_PORT_SERDES_AUTO_TUNE_PASS_E == *tuneStatusPtr)
    {
        if (CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode || CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
        {
            /* XAUI/RXAUI use XPCS which does not implement remote fault */
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppRemoteFaultSetFunc was skipped since XPCS is used");
        }
        else
        {
            if ((!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) &&
                                (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb))) &&
                (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))) /* for 100G we set local fault */
            {
                CPSS_PORT_MANAGER_LKB_CONFIG_STC *pLinkBinding = &(tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding);

                if (pLinkBinding->enabled == GT_TRUE)
                {
                    if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
                    {
                        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum,
                                                                                               tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode,
                                                                                               pLinkBinding->pairPortNum, GT_TRUE);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding add port failed=%d", devNum, portNum, rc);
                        }
                    }
                    else
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Link Binding not supported");
                    }
                }
                else
                {
                    /* i tune passes, remove 'fault send' so partners could start align */
                    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemoteFaultSetFunc(devNum, portNum, ifMode, speed, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppRemoteFaultSetFunc returned error code %d ",
                                                                   devNum, portNum, rc);
                    }
                }
            }
        }

        /* after training, force port to be in low-power mode (for falcon only)*/
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup,
                                                      tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum,
                                                      portMode, &curPortParams))
            {
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoDfeCfgSet null hws param ptr"));
            }


            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLowPowerEnableFunc(CAST_SW_DEVNUM(devNum), portNum, i, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDxChPortSerdesLowPowerModeEnable failed=%d", devNum, portNum, rc);
                }
            }
        }

        if (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
        {
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (!(PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) &&  PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* if application has override this, then get it from override db */
            if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideByteBitmap &
                 PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E) )
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted =
                tmpPortManagerDbPtr->portMngSmDb[portNum]->userOverrideAdaptiveRxTrainSupported;
            }
            else
            {
                /* by default Adaptive is running */
                tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted = GT_TRUE;
                if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted )
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Adaptive started speed %d ", speed);
                }
            }
        }
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerInitializeAlignLockTimerPhase
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  MV_HWS_PORT_INIT_PARAMS                *curPortParamsPtr,
    IN  PRV_CPSS_PORT_MNG_DB_STC               *tmpPortManagerDbPtr,
    OUT GT_BOOL                                *alignPtr,
    OUT GT_BOOL                                *timeoutPtr
)
{
    GT_STATUS                               rc;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    PRV_CPSS_PORT_MANAGER_PORT_STATUSES_STC portStatuses;

    CPSS_NULL_PTR_CHECK_MAC(alignPtr);
    CPSS_NULL_PTR_CHECK_MAC(timeoutPtr);

    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;

    if (curPortParamsPtr->numOfActLanes > 1)
    {
        if(PRV_CPSS_PORT_MANAGER_XPCS_MODE_CHECK(ifMode))
        {
            /* in modes that use xpcs there is no gearbox thus check only signal */
            rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E));
            if (portStatuses.sigDet == GT_TRUE)
            {
                portStatuses.overallStatusesOk = GT_TRUE;
            }
            else
            {
                if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->logCounter < 2 )
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "overallStatuses is NOT locked, portStatuses.sigDet %d ", portStatuses.sigDet);
                }
            }
        }
        else
        {
            rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                          /*PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) |*/
                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E) |
                                          PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ALIGN_LOCK_E));
            /*if (portStatuses.overallStatusesOk == GT_FALSE)*/
            if (portStatuses.gbLock == GT_TRUE && portStatuses.alignLock == GT_TRUE)
            {
                portStatuses.overallStatusesOk = GT_TRUE;
            }
            else
            {
                if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->logCounter < 2 )
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"overallStatuses is NOT locked, portStatuses.sigDet %d portStatuses.gbLock %d portStatuses.alignLock %d",
                                                               portStatuses.sigDet, portStatuses.gbLock, portStatuses.alignLock);
                }
            }
        }
    }
    else
    {
        /* in single lane no need to check for align lock */
        rc = prvCpssPortStatusesCheck(devNum, portNum, &portStatuses,
                                      /*PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_SIG_DET_E) |*/
                                      PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_GB_LOCK_E));
        if (portStatuses.gbLock == GT_TRUE /*&& portStatuses.sigDet == GT_TRUE*/)
        {
            portStatuses.overallStatusesOk = GT_TRUE;
        }
        else
        {
            if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->logCounter < 2 )
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"overallStatuses is NOT locked, portStatuses.sigDet %d portStatuses.gbLock %d", portStatuses.sigDet,portStatuses.gbLock);
            }
        }
    }

    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortStatusesCheck returned %d", devNum, portNum, rc);
    }

    rc = prvCpssPortManagerAlignTimerCheck(devNum, portNum, tmpPortManagerDbPtr, portStatuses.overallStatusesOk, alignPtr,timeoutPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerEventBitMaskingReading
* @endinternal
*
* @brief  function for readig bit mask per uni event (also to
*         checking good initialize).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC3X;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] portMacNum            - mac port number
* @param[in] curPortParamsPtr      - (pointer to) current port params
* @param[in] tmpPortManagerDbPtr   - (pointer to) port manager DB
* @param[in] uniEv                 - events from HW
*
* @param[out] data                 - bit mask from the register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPortManagerEventBitMaskingReading
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_U32                     portMacNum,
    IN  MV_HWS_PORT_INIT_PARAMS    *curPortParamsPtr,
    IN  PRV_CPSS_PORT_MNG_DB_STC   *tmpPortManagerDbPtr,
    IN  CPSS_UNI_EV_CAUSE_ENT      uniEv,
    OUT GT_U32                     *data
)
{
    GT_U32                  portGroup, regAddr, i, iterationNum;
    GT_STATUS               rc;

    portGroup  = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        iterationNum = 1;
    }
    else
    {
        iterationNum = curPortParamsPtr->numOfActLanes;
    }

    for (i = 0; i < iterationNum; i++)
    {
        /* finding the address */
        rc = prvFindCauseRegAddrByEvent(CAST_SW_DEVNUM(devNum), portGroup, uniEv/*uniEv*/, portMacNum+i /*evExtData*/, &regAddr);
        if (rc!=GT_OK)
        {
            rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)

            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvFindCauseRegAddrByEvent %d ", devNum, portMacNum+i, rc);
            }
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvFindCauseRegAddrByEvent %d ", devNum, portMacNum+i, rc);
        }

        CPSS_PORT_MANAGER_DBG_LOG_INFORMATION_MAC("[Port %d/%2d] clearing link_status_change interrupt"
                                                  " from register 0x%08x, for mac %d", devNum, portNum, regAddr, (portMacNum+i));

        rc = cpssDrvPpHwRegBitMaskRead(CAST_SW_DEVNUM(devNum), portGroup, regAddr, 0xFFFFFFFF, data);
        if (rc != GT_OK)
        {
            rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Could not perform maskUnmask operation", devNum, portNum);
            }
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDrvPpHwRegBitMaskRead failed=%d ", devNum, portNum, rc);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerInitialize
* @endinternal
*
* @brief  function for handling init event
*    After configure the port on the create, init event
*    processing the port and checking the functionality of the ports.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] curPortParamsPtr      - (pointer to) current port params
* @param[in] tmpPortManagerDbPtr   - (pointer to) port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerInitialize
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_INIT_PARAMS  *curPortParamsPtr,
    IN PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr
)
{
    /* Variables declarations */
    GT_STATUS                               rc;
    GT_U32                                  portGroup, result = 0, portMacNum;
    CPSS_PORT_SPEED_ENT                     speed;
    MV_HWS_PORT_STANDARD                    portMode;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    GT_BOOL                                 ignoreTraining;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC   loopbackCfg;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType;
    GT_BOOL                                 anyLoopback;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT   tuneStatus;
    GT_BOOL                                 align = GT_FALSE;
    GT_BOOL                                 timeout, skipIcal = GT_FALSE, bundleTrainDone = GT_FALSE;
    CPSS_PORT_MANAGER_LKB_CONFIG_STC        *pLinkBinding;
   /* MV_HWS_PORT_IN_GROUP_ENT stateInSerdesGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;*/
    GT_U32                                  step, portsInBundle = 1, startPort, tmpPort, i;

/**-------------------------------------Initialize-Set-Up---------------------------------------------------------*/
    if (PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(stageInit)[portNum] == GT_FALSE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: PORT_MANAGER_EVENT_INIT");
        PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(stageInit)[portNum] = GT_TRUE;
    }

    /* update those parameters  */
    rc = prvCpssPortManagerGetAttributes(devNum, portNum, tmpPortManagerDbPtr, &portType, &speed, &ifMode, &portMode, &portGroup, &loopbackCfg, &anyLoopback, &ignoreTraining);
    if (rc != GT_OK)
    {
        return rc;
    }

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;

    /* in case we return from link down, need to re-force Remote Fault */
    if ((!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) &&
        (!(PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb))) &&
        (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))) /* for 100G we set local fault */
    {
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->byPassRemoteFault = GT_FALSE;
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemoteFaultSetFunc(devNum, portNum, ifMode, speed, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppRemoteFaultSetFunc returned error code %d ", rc);
                return rc;
            }
        }
    }

    /*
        For USX port modes (non single) need to check
        all ports in SD_group. If all are in MAC_LINK_DOWN
        state - 'oneShotiCal' may be set to default.
    */
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    if ( mvHwsUsxModeCheck(devNum,portMacNum,portMode))
    {
        rc = mvHwsUsxExtSerdesGroupParamsGet(devNum,portMacNum,portMode, &startPort, &step, &portsInBundle);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "serdes group failed=%d",rc);
        }
        if ( portsInBundle>1 )
        {
            if ( PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppConvertMacToPortFunc != NULL)
            {
                for ( i = startPort; portsInBundle > 0; i += step, portsInBundle-- )
                {
                    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppConvertMacToPortFunc(devNum, i, &tmpPort);
                    if (tmpPortManagerDbPtr->portMngSmDb[tmpPort]->oneShotiCal == GT_FALSE)
                    {
                        skipIcal = GT_TRUE;
                    }
                    if (tmpPortManagerDbPtr->portMngSmDb[tmpPort]->rxStabilityTrainDone == CPSS_PORT_MANAGER_DONE_E)
                    {
                        bundleTrainDone = GT_TRUE;
                        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_DONE_E;
                    }
                    if ( skipIcal && bundleTrainDone )
                    {
                        break;
                    }
                }
            }
        }
    }
    /* for polling devices need only 1 shot iCal */
    if ((PRV_PORT_MANAGER_POLLING_DEVICES(devNum)) && (tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal) &&
        (!PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) || CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E == ifMode) && (!ignoreTraining))
    {
        /* (C) lanuching iCal */
        /*GT_BOOL signalDetect;

        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSigDetGetFunc(devNum, portNum, &signalDetect);
        if((rc != GT_OK) ||(!signalDetect))
        {
            return GT_OK;
        }*/

        tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal = GT_FALSE;
        if ( !skipIcal )
        {
            rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining returned error code %d ",
                                                           devNum, portNum, rc);
            }
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTotalTimeFromStart   = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimeDiffFromLastTime = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerSecondsPrev     = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTimerNsecPrev        = 0;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries                = 0;

    }
    else if ((PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)) && (PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        /* for CM4 SerDes type, need to run RX training and only validate the training is finished as well as CDR is stable,
           other than that skip all the rest stages */
        rc = prvCpssPortManagerTx2RxLoopbackPortEnable(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerTx2RxLoopbackPortEnable failed %d ", rc );
            return rc;
        }
    }

    if (!(PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)) &&
        !(PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) &&
        !(PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) )
    {
        /* for Optical calibration check Confidence is in range before starting the init process */
        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected == GT_FALSE) &&
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished == GT_FALSE)&&
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E))
        {
            rc = prvCpssPortManagerOpticalStabilityCheck(devNum,portGroup,portNum,portMode);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerOpticalStabilityCheck failed", devNum, portNum);
            }
            else
            {
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->calibrationMode.overallOpticalCalFinished == GT_FALSE)
                {
                    return GT_OK;
                }
            }
        }

    }
/**--------------------------------------------------------------------------------------------------------------*/

    if (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg) && !(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        rc = prvCpssPortManagerTx2RxLoopbackPortEnable(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerTx2RxLoopbackPortEnable failed %d ", rc );
            return rc;
        }
        goto skipStabilityCheck;
    }

    /* Special case for SD RX-2-TX Loopback for 1000Base_X and all slow modes except OUSGMII */
    if ((PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) && CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E != ifMode) &&
        (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg)))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "config loopback type %d  enableRegularTrafic %d", loopbackCfg.loopbackType, loopbackCfg.enableRegularTrafic);
        rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                       portNum, rc);
        }
    }

    /* if port is in stability checking phase (which is prior to train) */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == CPSS_PORT_MANAGER_IN_PROGRESS_E)
    {
        goto signalStabilityFilerPhase;
    }
    /* if port is in training phase */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone == CPSS_PORT_MANAGER_IN_PROGRESS_E)
    {
        goto tunePhase;
    }
    /* if port is in waiting for align lock phase */
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone == CPSS_PORT_MANAGER_IN_PROGRESS_E)
    {
        goto alignLockTimerPhase;
    }

    if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone == CPSS_PORT_MANAGER_IN_PROGRESS_E) && (!ignoreTraining) && (!bundleTrainDone))
    {
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
        CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

        /* TBD harrier */
        if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
            if(cpssDeviceRunCheck_onEmulator())
            {
                rxTuneStatus = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
                txTuneStatus = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;

                tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_DONE_E;
                return GT_OK;
            }
        }

        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneStatusGetFunc(devNum, portNum,
                                                                                  &rxTuneStatus, &txTuneStatus);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppTuneStatusGetFunc failed %d ", rc );
            return rc;
        }
        if (rxTuneStatus == CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E)
        {
            /* TBD - when signal is lost and also alignment lost or gb lock lost, then more than one event
              will be received upon the port that goes down (signal_detect_change and gb_lock change or
              algin_lock_lost). First event will insert us to sigDetTrain, good, but then the second event
              will make the seuqnce try to finish the sigDetTrain but there is no signal and it will reach
              timeout. This scenario is not a bug as eventually port will be in state link_down, but there
              is a bad side affect to this scenario as application will keep initialize (passing event init)
              for nothing, and CPU time is wasted. We need to think about a better sequence in order to not
              perform this side effect. */
            if (curPortParamsPtr->numOfActLanes == 1)
            {
                /* log this information only once, as this scope will be reached multiple times until
                  rx training will be finished */
                if (tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainTries == 0)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Possible signal lost during training - restarting");
                }
            }
            rc = prvCpssPortManagerRxStabilityTimeCheck(CAST_SW_DEVNUM(devNum), portNum, portMode, tmpPortManagerDbPtr);
            if ( rc == GT_TIMEOUT)
            {
                rc = GT_OK;
                /* when training fail on timeout - check cal delta and update dfe common if needed*/
                if ( HWS_PAM4_MODE_CHECK(portMode)) {
                    if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc != NULL) && (PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
                     {
                         rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                                            portGroup, ifMode, speed,
                                                                                                            MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E, &result);
                         if (rc != GT_OK)
                         {
                             CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortExtraOperationsSetFunc returned error code %d ", devNum, portNum, rc);
                         }
                     }
                }
                tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.trainingFailCnt++;
                prvCpssPortManagerModeToDownState(devNum,portNum);
            }
            return rc;
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"stop Rx training ");
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc(devNum, portNum, CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_STOP_E);

            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppTuneExtSetFunc failed=%d ",
                                                           devNum, portNum, rc);
            }
            if ( CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E == rxTuneStatus )
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Rx training failed");
                tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.trainingFailCnt++;
                prvCpssPortManagerModeToDownState(devNum,portNum);

                return GT_OK;
            }
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Training finish set rxStabilityTrainDone = CPSS_PORT_MANAGER_DONE_E ");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_DONE_E;
        prvCpssPortManagerStatTimeGet(tmpPortManagerDbPtr->portMngSmDb[portNum], &tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.rxTrainingTime);

        /* link is up */
        if(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc != NULL)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc(
                CAST_SW_DEVNUM(devNum), portNum, portGroup, ifMode, speed, MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_E, &result);
            if ( GT_OK != rc )
            {
                return rc;
            }
        }

        if ( HWS_PAM4_MODE_CHECK(portMode)) {
            /* when training finish - check cal delta and update dfe common if needed, if dfe chenged we need to retrain*/
            if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc != NULL) && (PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
            {
                GT_U32 needRetrain = 0;
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                                    portGroup, ifMode, speed,
                                                                                                    MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E, &needRetrain);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E returned error code %d ", devNum, portNum, rc);
                }
                if ( needRetrain) {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Training failed in cal delta ");
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_RESET_E;
                    tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal = GT_TRUE;
                    return GT_OK;
                }
                /* trainning is ok recalc level 3*/
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                                     portGroup, ifMode, speed,
                                                                                                     MV_HWS_PORT_SERDES_OPERATION_CALC_LEVEL3_E, &result);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling MV_HWS_PORT_SERDES_OPERATION_CALC_LEVEL3_E ifmode %d seed %d returned error code %d ", devNum, portNum, ifMode, speed, rc);
                }
            }
        }

        return GT_OK;
    }

    /* we will check stability only in case signal is down and this is the first up.
      (and also in case we already in the stability process - hence 'goto' lable)*/
/**-------------------------------------------*/
signalStabilityFilerPhase:
   /* 1000baseX and all slow modes except OUSGMII should go through signal stability check but thats it, should need to throug training and alignment check */
    if (PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) && CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E != ifMode)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"finished signal integrity for 1000baseX ");

        rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_TRUE);
        if (rc != GT_OK)
        {
            prvCpssPortManagerModeToDownState(devNum,portNum);
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel - rc %d ", devNum, portNum, rc);
        }

        rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Checking link up status");
        rc = prvCpssPortManagerLinkUpCheck(devNum,portNum,GT_TRUE,tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
            prvCpssPortManagerModeToDownState(devNum,portNum);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
            return rc;
        }

        return GT_OK;
    }

    rc = prvCpssPortManagerInitializeSignalStabilityFilerPhase(devNum, portNum, curPortParamsPtr, tmpPortManagerDbPtr);
    if (rc != GT_OK)
    {
         return rc;
    }
    else if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E) ||
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == CPSS_PORT_MANAGER_IN_PROGRESS_E))
    {
        /* start training again for pipe rx2tx loopback in case that alignlock is reset and the peer can't lock it */
        if((PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg)) &&
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E))
        {
            if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone  == CPSS_PORT_MANAGER_RESET_E) &&
                (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM          == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E))
            {
                rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", devNum, portNum, rc);
                }
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
            }
        }

        return GT_OK;
    }
    else if ((PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)) && (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) &&
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == CPSS_PORT_MANAGER_DONE_E))
    {
        /* for CM4 SerDes type, need to run RX training and only validate the training is finished as well as CDR is stable,
           other than that skip all the rest stages */
        rc = prvCpssPortManagerTx2RxLoopbackPortEnable(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerTx2RxLoopbackPortEnable failed %d ", rc );
            return rc;
        }

        goto skipStabilityCheck;
    }
    else if (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg) &&
             (loopbackCfg.enableRegularTrafic == GT_FALSE) &&
             (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E || PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {
        /*skipping tuning and align lock in rx2tx on AC5 when traffic is false because data
          do not enter the mac and pcs.
          no need align/GB locking and AC5 using comphy, so no need to do tuning (only avago).*/
        goto skipStabilityCheck;
    }

tunePhase:

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone = CPSS_PORT_MANAGER_DONE_E;
        tuneStatus = CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->adaptiveStarted = GT_FALSE;
        if (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
        {
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
    }
    else
    {
        rc = prvCpssPortManagerInitializeTunePhase(devNum, portNum, tmpPortManagerDbPtr, &tuneStatus);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (CPSS_PORT_SERDES_AUTO_TUNE_PASS_E == tuneStatus)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"start align lock check ");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->logCounter = 0;
alignLockTimerPhase:
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) &&
            (CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode || CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode))
        {
            /* XAUI/RXAUI use XPCS which does not implement remote fault */
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppRemoteFaultSetFunc was skipped since XPCS is used");
        }

        rc = prvCpssPortManagerInitializeAlignLockTimerPhase(devNum, portNum, curPortParamsPtr, tmpPortManagerDbPtr, &align, &timeout);
        if(rc != GT_OK)
        {
            return rc;
        }

        if ((align == GT_FALSE) && (timeout == GT_FALSE))
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->logCounter++;
            return GT_OK;
            /*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,
             "[Port %d/%2d] prvCpssPortManagerInitializeAlignLockTimerPhase not align. align: %d rc = %d", devNum, portNum, align, rc);*/
        }

        /* if 3 qonsequtive good-all-statuses existed, we are good */
        if (align == GT_TRUE)
        {
            prvCpssPortManagerStatTimeGet(tmpPortManagerDbPtr->portMngSmDb[portNum], &tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.alignLockTime);
skipStabilityCheck:
            prvCpssPortManagerResetCounters(portNum,tmpPortManagerDbPtr);

            /* ENABLE */
            /*CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"all statuses OK ");*/

            /* In case of PIPE (or newer) CG MAC, need to turn on RF before sending msg to LKB FW,
               to reach same state as in XLG MAC */
            pLinkBinding = &(tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding);

            if ( (pLinkBinding->enabled == GT_TRUE) &&
                 PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb) )
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemoteFaultSetFunc(devNum, portNum, ifMode, speed, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"ppRemoteFaultSetFunc returned error code %d ", rc);
                    return rc;
                }
                if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
                {
                    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum,
                                                                                           tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode,
                                                                                           pLinkBinding->pairPortNum, GT_TRUE);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding add port failed=%d", devNum, portNum, rc);
                    }
                }
                else
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Link Binding not supported");
                }
            }
            if (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg))
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "config loopback type %d  enableRegularTrafic %d", loopbackCfg.loopbackType, loopbackCfg.enableRegularTrafic);
                rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                               devNum, portNum, rc);
                }
            }
            rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_TRUE);
            if (rc != GT_OK)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel - rc %d ", devNum, portNum, rc);
            }

            rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
            }

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Checking link up status");
            rc = prvCpssPortManagerLinkUpCheck(devNum, portNum, GT_TRUE, tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                prvCpssPortManagerModeToDownState(devNum,portNum);

                /* only high speed ports will reach here so no need to check for high speed port before
                  Calling this next function */
                if((!ignoreTraining) && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
                {
                    rc = prvCpssPortManagerSigStableRxTraining(devNum, portNum);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining failed=%d \n ", devNum, portNum, rc);
                    }
                }
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerLinkUpCheck - rc %d ", devNum, portNum, rc);
            }
        }
        /* maximux timeout */
        else if (timeout == GT_TRUE)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.alignLockFailCnt++;
            prvCpssPortManagerModeToDownState(devNum,portNum);

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerInitializeAlignLockTimerPhase got timeout = true");
            /* disable port high level unit */
            rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel "
                                                           " returned error code %d ",
                                                           devNum, portNum, rc);
            }

            rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
            }
        }
    }
/**-------------------------------------------*/
    else if (CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E == tuneStatus)
    {
        prvCpssPortManagerResetCounters(portNum,tmpPortManagerDbPtr);

        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected   = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone  = CPSS_PORT_MANAGER_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM      = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"unmasking port, signal_det, cause prvCpssPortStageTrainMngPhases failed ");

        rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel - rc %d ", devNum, portNum, rc);
        }

        rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
        }
    }

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone      == CPSS_PORT_MANAGER_IN_PROGRESS_E ||
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone  == CPSS_PORT_MANAGER_IN_PROGRESS_E ||
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone  == CPSS_PORT_MANAGER_IN_PROGRESS_E)
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerSimpleInitialize
* @endinternal
*
* @brief  function for handling init event for simple port mode.
*    After configure the port on the create, init event
*    processing the port and checking the functionality of the
*    ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC3X;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] tmpPortManagerDbPtr   - port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerSimpleInitialize
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr
)
{
    GT_STATUS                               rc;
    GT_U32                                  portGroup;
    CPSS_PORT_SPEED_ENT                     speed;
    MV_HWS_PORT_STANDARD                    portMode;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC   loopbackCfg;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType;
    GT_BOOL                                 anyLoopback;
    GT_BOOL                                 ignoreTraining;

    /* update those parameters  */
    rc = prvCpssPortManagerGetAttributes(devNum, portNum, tmpPortManagerDbPtr, &portType, &speed, &ifMode, &portMode, &portGroup, &loopbackCfg, &anyLoopback, &ignoreTraining);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(stageInit)[portNum] == GT_FALSE)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: PORT_MANAGER_EVENT_INIT");
        PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(stageInit)[portNum] = GT_TRUE;
    }

#if 0
    /* Auto-Negotiation sequence for 1G QSGMII/SGMII */
    if (((ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)) &&
         (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum))))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Disable Auto-Negotiation 1G QSGMII/SGMII");
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppAutoNeg1GSgmiiFunc(devNum, portNum, &(tmpPortManagerDbPtr->portMngSmDb[portNum]->autoNegotiation));
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] Calling ppAutoNeg1GSgmiiFunc returned %d \n ", portNum, rc);
        }
    }
#endif

    /* enable port high level unit */
    rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_TRUE);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                   " returned error code %d ",
                                                   devNum, portNum, rc);
    }

    if (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg))
    {
        rc = prvCpssPortManagerTx2RxLoopbackPortEnable(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerTx2RxLoopbackPortEnable failed %d ", rc );
            return rc;
        }
    }

    if (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "config loopback type %d  enableRegularTrafic %d", loopbackCfg.loopbackType, loopbackCfg.enableRegularTrafic);
        rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                       devNum, portNum, rc);
        }
    }

    rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Checking link up status");
    rc = prvCpssPortManagerLinkUpCheck(devNum,portNum,GT_TRUE,tmpPortManagerDbPtr);
    if (rc != GT_OK)
    {
        prvCpssPortManagerModeToDownState(devNum,portNum);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssPortManagerEnable
* @endinternal
*
* @brief  function for handling enable event
*    Enable event resume the port after disable.
*    In enable the ports using the saved database and just powering the hardware components.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] curPortParamsPtr      - (pointer to) current port params
* @param[in] tmpPortManagerDbPtr   - (pointer to) port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerEnable
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_INIT_PARAMS  *curPortParamsPtr,
    IN PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr
)
{
    GT_STATUS                               rc;
    GT_U32                                  portGroup;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_SPEED_ENT                     speed;
    MV_HWS_PORT_STANDARD                    portMode;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC   loopbackCfg;
    GT_BOOL                                 anyLoopback;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType;
    GT_U32                                  i;
    GT_U32                                  data;
    GT_BOOL                                 rxSignalOk;
    CPSS_UNI_EV_CAUSE_ENT                   uniEv = CPSS_UNI_RSRVD_EVENT_E;
    GT_U32                                  portMacNum;

    ifMode     = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    speed      = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    portGroup  = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;
    portMode   = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    portType   = tmpPortManagerDbPtr->portMngSmDb[portNum]->portType;

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: PORT_MANAGER_EVENT_ENABLE");
    if (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
    {
        /* mask low level so false interrupt from serdes power-up won't */
        rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* loopback configuration - get from DB */
    rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &loopbackCfg, &anyLoopback);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",devNum, portNum, rc);
    }

    /* power-up port */
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwUnresetFunc(devNum, portNum);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppPortHwUnresetFunc returned %d ", devNum, portNum, rc );
    }

    if ((PRV_CPSS_PORT_MANAGER_SPEED_CG_CHECK_MAC(speed)) || (PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        if (PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                           devNum, portNum, rc);
            }
        }
    }

    /* if user configured rx\tx params and polarity*/
    for (i=0; i<curPortParamsPtr->numOfActLanes; i++)
    {
        CPSS_PORT_SERDES_TUNE_STC serdesParams;
        GT_U32                    dbSdValidBitMap = 0;

        rc = prvCpssPortManagerLaneTuneDbGet(devNum,portNum, 0, curPortParamsPtr->activeLanesList[i], portType,
                                             &serdesParams, &dbSdValidBitMap);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneDbGet failed=%d on Rx", devNum, portNum,rc);
        }
        if (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_RX_E)
        {
            rc = prvCpssPortManagerLaneTuneHwSet(devNum,portGroup,portNum, portMode,curPortParamsPtr->activeLanesList[i], GT_TRUE);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Rx",devNum, portNum,rc);
            }
        }
        if (dbSdValidBitMap & CPSS_PM_DB_LANE_OVERRIDE_TX_E)
        {
            rc = prvCpssPortManagerLaneTuneHwSet(devNum,portGroup,portNum,portMode,curPortParamsPtr->activeLanesList[i], GT_FALSE);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLaneTuneHwSet failed=%d on Tx", devNum, portNum,rc);
            }
        }
    }


    /* Auto-Negotiation sequence for slow modes except SGMII2_5 and _2500Base_X for SIP_6 devices */
    if (PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) || (SGMII2_5 != portMode && _2500Base_X != portMode)))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Configure Auto-Negotiation 1G QSGMII/SGMII");
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppAutoNeg1GSgmiiFunc(devNum, portNum, ifMode, &(tmpPortManagerDbPtr->portMngSmDb[portNum]->autoNegotiation));
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppAutoNeg1GSgmiiFunc returned %d \n ", devNum, portNum, rc);
        }
    }


    if (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
    {
        if(!PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
        {
            /* clear cause bit of interrupt link_status_change */
            uniEv = CPSS_PP_PORT_LINK_STATUS_CHANGED_E;
            rc = prvCpssPortManagerEventBitMaskingReading(devNum, portNum,portMacNum, curPortParamsPtr, tmpPortManagerDbPtr, uniEv, &data);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] calling prvCpssPortManagerEventBitMaskingReading %d ", devNum, portNum, rc);
            }

            /* Reading the cause register of the interrupt */
            uniEv = CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E;
            rc = prvCpssPortManagerEventBitMaskingReading(devNum, portNum,portMacNum, curPortParamsPtr, tmpPortManagerDbPtr, uniEv, &data);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] calling prvCpssPortManagerEventBitMaskingReading %d ", devNum, portNum, rc);
            }
        }
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"clearing signal_detect AND link_status_change interrupt for mac %d numOfActLanes %d", (portMacNum), curPortParamsPtr->numOfActLanes);

        if (!PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            /* unmask low level interrupts */
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
            }
        }
         /* loopback configuration */
        if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && (loopbackCfg.loopbackMode.serdesLoopbackMode != CPSS_PORT_SERDES_LOOPBACK_DISABLE_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "config loopback type %d  enableRegularTrafic %d", loopbackCfg.loopbackType, loopbackCfg.enableRegularTrafic);
            rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                           devNum, portNum, rc);
            }
        }

        if ((PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))  ||
            (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)))
        {
            if (loopbackCfg.enableRegularTrafic == GT_FALSE)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"In TX2RX mode serdes Tx is disable ");
                rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, GT_FALSE);
                if(rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsPortTxEnable failed=%d", devNum, portNum, rc);
                }
            }

            if(!PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
            }
        }
        else
        {
            /* if rx_signal_ok is up, pushing event that might have been missed */
            rc = mvHwsPortAvagoSerdesRxSignalOkStateGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &rxSignalOk);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDrvPpHwRegBitMaskRead failed=%d", devNum, portNum, rc);
            }
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"rx_signal_ok state=%d", rxSignalOk);
            if ((rxSignalOk == GT_TRUE) && (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
            {
                rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum/*extraData*/, uniEv);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerPushEvent"
                                                               " failed=%d", devNum, portNum, rc);
                }
            }

            /* launching iCal */
            if (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
                /* for polling devices the iCal is done only when EI's are detected, application polls this in link down state */
            {
                if (!(PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)))
                {
                    rc = prvCpssPortManagerSigStableRxTraining(devNum,portNum);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining returned error code %d ",
                                                               devNum, portNum, rc);
                    }
                }
            }
        }

        /* marking rx train ran */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
    }
    if(PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port enable with MAC loopback");
        /* mask low level interrupts (working on mac lvl only)*/
        rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
        }

        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
            }
        }
        rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
        }

        rc = prvCpssPortManagerLinkUpCheck(devNum, portNum, GT_TRUE, tmpPortManagerDbPtr);
        if(rc != GT_OK)
        {
            prvCpssPortManagerModeToDownState(devNum,portNum);
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerLinkUpCheck - rc %d ", devNum, portNum, rc);
        }
    }
    CPSS_PM_START_TIME(tmpPortManagerDbPtr->portMngSmDb[portNum]);

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: PORT_MANAGER_EVENT_ENABLE - END");

    return GT_OK;
}

/**
* @internal prvCpssPortManagerDelete
* @endinternal
*
* @brief  function for handling deleting event
*    The port will be Deleted, all hardware components will be
*    powered off and all the port attributes will be removed
*    from both PM and CPSS data bases. In order to re-create the
*    port, application must call to set parameters again before
*    create.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] curPortParamsPt       - (pointer to) current port params
* @param[in] tmpPortManagerDbPtr   - (pointer to) port manager DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerDelete
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN MV_HWS_PORT_INIT_PARAMS  *curPortParamsPtr,
    IN PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr,
    IN CPSS_PORT_MANAGER_EVENT_ENT  stage

)
{
    GT_STATUS                               rc;
    GT_U32                                  portGroup;
    GT_U32                                  portMacNum;
    CPSS_PORT_SPEED_ENT                     speed;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC   loopbackCfg;
    CPSS_PORT_MANAGER_PORT_TYPE_ENT         portType;
    GT_BOOL                                 anyLoopback, onlyPmDelete = GT_FALSE;
    CPSS_PORTS_BMP_STC                      portModeSpeedSetBitmap;
    MV_HWS_PORT_STANDARD                    portMode;
    MV_HWS_PORT_INIT_PARAMS                 curPortParams = *curPortParamsPtr;
    CPSS_PM_MAC_PREEMPTION_PARAMS_STC       defaultPreemptionParams =
            {
                CPSS_PM_MAC_PREEMPTION_DISABLED_E,
                CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E,
                CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E
            };

    tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone  = CPSS_PORT_MANAGER_RESET_E;
    if ( stage == CPSS_PORT_MANAGER_EVENT_DELETE_E )
    {

        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM  == CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E )
            onlyPmDelete = GT_TRUE;

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM       = CPSS_PORT_MANAGER_STATE_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged       = GT_FALSE;
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)|| PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM     = GT_FALSE;
        }
    }
    else
    {
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM       = CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E;
    }

    ifMode    = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    speed     = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    portGroup = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;
    portType  = tmpPortManagerDbPtr->portMngSmDb[portNum]->portType;
    portMode  = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;


    if ( onlyPmDelete == GT_FALSE )
    {
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->linkBinding.enabled == GT_TRUE )
        {
            if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc))
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc(devNum, portNum, NON_SUP_MODE, 0, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] link-binding delete port failed=%d", devNum, portNum, rc);
                }
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Link Binding not supported");
            }
        }
    }

    /*Get portMac number for a given physical portNum*/
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portModeSpeedSetBitmap);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portModeSpeedSetBitmap, portNum);

    if(tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort == GT_TRUE)
    {
        if ( stage == CPSS_PORT_MANAGER_EVENT_DELETE_E )
        {
            /* when port is deleted, all of his stored attributes are deleted from Port Manager DB */
            rc = prvCpssPortManagerCleanPortParameters(CAST_SW_DEVNUM(devNum), portGroup, portNum, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerCleanPortParameters failed=%d",
                                                       devNum, portNum, rc);
            }
        }
        if ( onlyPmDelete == GT_FALSE )
        {
            /*mvHwsCm3SemOper(devNum,portMacNum, GT_TRUE);*/
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc(devNum, portModeSpeedSetBitmap, GT_FALSE, ifMode, speed, tmpPortManagerDbPtr->portMngSmDb[portNum]);
            /*mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);*/
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortModeSpeedSetFunc"
                                                           " returned error code %d ",
                                                           devNum, portNum, rc);
            }
        }
        return GT_OK;
    }

    if ( onlyPmDelete == GT_FALSE )
    {
        /*Delete fastlink configuration if already set by application*/
        /*Applies to BC3 & Aldrin2 devices only*/
        if (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
             (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)) &&
             (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].isFastLink))
        {
            if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortFastLinkDownSetFunc != NULL)
            {
                /*Clearing fastLink configuration*/
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortFastLinkDownSetFunc(CAST_SW_DEVNUM(devNum), portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortFastLinkDownSetFunc=%d", devNum, portNum, rc);
                }
            }
        }

        rc = prvCpssPortManagerStopAdaptive(devNum, portNum, tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling prvCpssPortManagerStopAdaptive failed %d ", devNum, portNum, rc);
        }

        /* restore loopback to defaults if enabled */
        /* loopback configuration - get from DB */
        rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, portType, &loopbackCfg, &anyLoopback);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d", devNum, portNum, rc);
        }

        /* reset to defualt setting for Auto-Negotiation sequence for slow modes except SGMII2_5 and _2500Base_X for SIP_6 devices */
        if (PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) || (SGMII2_5 != portMode && _2500Base_X != portMode)))
        {
            CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC autoNegotiationTmp;
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Reset to defualt setting for Auto-Negotiation for slow modes");

            if ( ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E )
            {
                autoNegotiationTmp.inbandEnable                 = GT_TRUE;
                autoNegotiationTmp.duplexEnable                 = GT_TRUE;
                autoNegotiationTmp.speedEnable                  = GT_TRUE;
                autoNegotiationTmp.byPassEnable                 = GT_FALSE;
                autoNegotiationTmp.flowCtrlEnable               = GT_FALSE;

                if ( PRV_CPSS_SIP_6_CHECK_MAC(devNum) )
                {
                    autoNegotiationTmp.flowCtrlAsmAdvertiseEnable   = GT_TRUE;
                    autoNegotiationTmp.flowCtrlPauseAdvertiseEnable = GT_TRUE;
                }
                else
                {
                    autoNegotiationTmp.flowCtrlAsmAdvertiseEnable   = GT_FALSE;
                    autoNegotiationTmp.flowCtrlPauseAdvertiseEnable = GT_FALSE;
                }
            }
            else if ( ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E )
            {
                if ( PRV_CPSS_SIP_6_CHECK_MAC(devNum) )
                {
                    autoNegotiationTmp.inbandEnable                 = GT_TRUE;
                    autoNegotiationTmp.byPassEnable                 = GT_FALSE;
                    autoNegotiationTmp.flowCtrlAsmAdvertiseEnable   = GT_TRUE;
                    autoNegotiationTmp.flowCtrlPauseAdvertiseEnable = GT_TRUE;
                }
                else
                {
                    autoNegotiationTmp.inbandEnable                 = GT_FALSE;
                    autoNegotiationTmp.byPassEnable                 = GT_TRUE;
                    autoNegotiationTmp.flowCtrlAsmAdvertiseEnable   = GT_FALSE;
                    autoNegotiationTmp.flowCtrlPauseAdvertiseEnable = GT_TRUE;
                }

                autoNegotiationTmp.duplexEnable                 = GT_FALSE;
                autoNegotiationTmp.speedEnable                  = GT_FALSE;
                autoNegotiationTmp.flowCtrlEnable               = GT_FALSE;
            }
            else
            {
                autoNegotiationTmp.inbandEnable                 = GT_FALSE;
                autoNegotiationTmp.duplexEnable                 = GT_TRUE;
                autoNegotiationTmp.speedEnable                  = GT_TRUE;
                autoNegotiationTmp.byPassEnable                 = GT_TRUE;
                autoNegotiationTmp.flowCtrlEnable               = GT_FALSE;
                autoNegotiationTmp.flowCtrlAsmAdvertiseEnable   = GT_FALSE;
                autoNegotiationTmp.flowCtrlPauseAdvertiseEnable = GT_FALSE;
            }

            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppAutoNeg1GSgmiiFunc(devNum, portNum, ifMode, &autoNegotiationTmp);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppAutoNeg1GSgmiiFunc returned %d \n ", devNum, portNum, rc);
            }
        }
        /* Disable port */
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Enable returned error code %d ", rc);
            return rc;
        }
        /* loopback configuration - now set to HW */
        if (anyLoopback == GT_TRUE)
        {
            if ((PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))  ||
                (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)))
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port enable with MAC loopback");
                /* mask low level interrupts (working on mac lvl only)*/
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                 if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
                }
            }

            /* initialize loopback mode db to proper loopback disable which is the initial state of HW */
            rc = prvCpssPortManagerLoopbackModeInit(portNum,loopbackCfg.loopbackType,&loopbackCfg);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModeInit failed=%d",
                                                           devNum, portNum, rc);
            }
            /* set disable state to HW */
            rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                           devNum, portNum, rc);
            }
        }
    }


    loopbackCfg.loopbackType = CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E;
    if ( stage == CPSS_PORT_MANAGER_EVENT_DELETE_E )
    {
        rc = prvCpssPortManagerLoopbackModeInit(portNum,
                                                CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E,
                                                &tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->loopbackDbCfg);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModeInit failed=%d",
                                                       devNum, portNum, rc);
        }

        /* when port is deleted, all of his stored attributes are deleted from Port Manager DB */
        rc = prvCpssPortManagerCleanPortParameters(CAST_SW_DEVNUM(devNum), portGroup, portNum, CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerCleanPortParameters failed=%d",
                                                       devNum, portNum, rc);
        }
    }

    if ( onlyPmDelete == GT_FALSE )
    {
        if (PRV_PORT_MANAGER_CHECK_XLG_CG_MAC_PORT_MAC(curPortParams))
        {
            rc = prvCpssPortXlgMacMaskConfig(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortXlgMacMaskConfig on port %d returned %d ", devNum, portNum, rc);
            }
        }
        /*mvHwsCm3SemOper(devNum,portMacNum, GT_TRUE);*/
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc(devNum, portModeSpeedSetBitmap, GT_FALSE, ifMode, speed, tmpPortManagerDbPtr->portMngSmDb[portNum]);
        /*mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);*/
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortModeSpeedSetFunc"
                                                       " returned error code %d ",
                                                       devNum, portNum, rc);
        }
        if(tmpPortManagerDbPtr->portMngSmDb[portNum]->preemptionParams.type!= CPSS_PM_MAC_PREEMPTION_DISABLED_E)
        {
            /*For port that is deleted preemption should be disabled*/
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPreemptionParamsSetFunc(devNum,portNum,
            &defaultPreemptionParams/*return to default*/);
            if (rc!=GT_OK)
            {
               CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssDxChTxqSip6_10PreemptionEnableSet failed=%d\n", devNum, portNum,rc);
            }
        }

        if ( PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPtpDelayParamsSetFunc(devNum,portNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortPtpDelayParamsSetFunc failed=%d\n", devNum, portNum,rc);
            }
        }

        /* return bandwidth mode to default - CPSS_PORT_PA_BW_MODE_REGULAR_E */
        if ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc != NULL) && !PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc(CAST_SW_DEVNUM(devNum), portNum, CPSS_PORT_PA_BW_MODE_REGULAR_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortBwTxFifoSetFunc returned error code %d ",
                                                           devNum, portNum, rc);
            }
        }

        rc = prvCpssPortManagerEnableRemoteLink(devNum,portNum,ifMode,speed,tmpPortManagerDbPtr,GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerEnableRemoteLink returned error code %d ", rc);
            return rc;
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* mask mac and low level interrupts */
            rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
            }
        }
        else
        {
            rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
            }
        }
    }



    return GT_OK;
}

/**
* @internal prvCpssPortManagerStatusChanged
* @endinternal
*
* @brief  function for handling status changed(low/mac) event
*           - Triggered by interrupts
*           - Mac event will checking the port link up status
*           - Low event will mask if needed the interrupts and
*             will call to init event
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] tmpPortManagerDbPtr   - port manager DB
*
* @param[inout] stagePtr           - PM stage
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortManagerStatusChanged
(
    IN GT_SW_DEV_NUM                  devNum,
    IN GT_PHYSICAL_PORT_NUM           portNum,
    IN PRV_CPSS_PORT_MNG_DB_STC       *tmpPortManagerDbPtr,
    INOUT CPSS_PORT_MANAGER_EVENT_ENT *stagePtr
)
{
    GT_STATUS                               rc;
    CPSS_PORT_SPEED_ENT                     speed;
    GT_U32                                  laneNum;
    MV_HWS_PORT_INIT_PARAMS                 curPortParams;
    GT_U32                                  portMacNum;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    MV_HWS_PORT_INIT_PARAMS                 currPortParams;
    GT_U32                                  portGroup,serdesNum;
    CPSS_PORT_FEC_MODE_ENT                  fecAbility;
    MV_HWS_PORT_STANDARD                    portMode;
    GT_BOOL                                 cdrLock;

    fecAbility = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
    speed  = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portMode = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;


    if (*stagePtr == CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E &&
        (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_UP_E ||
         tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E))
    {
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap & CPSS_PM_PORT_LINK_STATUS_CHANGED_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Ignore Mac event while interrupt is masked");
            return GT_OK;
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Checking link up status");

        rc = prvCpssPortManagerLinkUpCheck(devNum,portNum,GT_TRUE,tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
            prvCpssPortManagerModeToDownState(devNum,portNum);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
            return rc;
        }

        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E)
        {
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                if((CPSS_PORT_SPEED_25000_E == speed)
                        && (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode)
                        && (CPSS_PORT_RS_FEC_MODE_ENABLED_E == fecAbility))
                {
                    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
                    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
                    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, NON_SUP_MODE, &currPortParams);
                    /*Since 25G uses one lane*/
                    serdesNum = currPortParams.activeLanesList[0];
                    mvHwsAvagoSyncOkErrataFix(devNum, portGroup, serdesNum);
                }
            }
        }
    }
    else if (*stagePtr == CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E &&
             tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
    {
        /*read cdr */
        if (( PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCdrLockGetFunc != NULL ) && 
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E) &&
            ((PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))|| (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)))
        {
            PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCdrLockGetFunc(devNum, portNum, &cdrLock);
            if ( cdrLock == GT_FALSE )
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"CDR not locked");
                return GT_OK;
            }
        }
        /* PCS align lock(0x1) + PCS GB lock(0x2) + signal detect(0x4) */
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap & 0x7 )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Ignore low level event while interrupt is masked");
            return GT_OK;
        }
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"low_level changed portAdminSM %d", tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM);
        if ( !PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode) )
        {
            rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if(PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
        {
            portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;

            if(tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E)
            {
                prvCpssPortManagerStatTimeGet(tmpPortManagerDbPtr->portMngSmDb[portNum], &tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics.linkFailToSignalTime);
                CPSS_PM_START_TIME(tmpPortManagerDbPtr->portMngSmDb[portNum]);
                tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected = GT_FALSE;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal    = GT_TRUE;

                if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, NON_SUP_MODE, &curPortParams))
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                               "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
                }

                /* change to SerDes normal-power mode (training need to be in normal-power) */
                if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
                {
                    for (laneNum = 0; laneNum < curPortParams.numOfActLanes; laneNum++)
                    {
                        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLowPowerEnableFunc(CAST_SW_DEVNUM(devNum), portNum, laneNum, GT_FALSE);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDxChPortSerdesLowPowerModeEnable failed=%d", devNum, portNum, rc);
                        }
                    }
                }

                *stagePtr = CPSS_PORT_MANAGER_EVENT_INIT_E;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
                return GT_OK;
            }
            else
            {
                rc = prvCpssPortManagerStopAdaptive(devNum, portNum, tmpPortManagerDbPtr);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"prvCpssPortManagerStopAdaptive failed %d ", rc );
                    return rc;
                }
                prvCpssPortManagerModeToDownState(devNum,portNum);
                if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
                {
                    if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM != GT_TRUE)
                    {
                        if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortEnableWaWithLinkStatusSet != NULL)
                        {
                            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortEnableWaWithLinkStatusSet(devNum, portNum);
                            if (rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDxChPortEnableWaWithLinkStatusSet: error, rc=%d\n",
                                        devNum, portNum,rc);
                            }
                        }
                    }
                }
                return GT_OK;
            }
        }
        *stagePtr = CPSS_PORT_MANAGER_EVENT_INIT_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E;
    }

    return GT_OK;
}

static GT_STATUS prvCpssPortManagerCopyRegToApPort
(
    GT_SW_DEV_NUM               devNum,
    GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_MANAGER_EVENT_ENT stage
)
{
    CPSS_PM_PORT_PARAMS_STC  regularPortParams;
    CPSS_PM_PORT_PARAMS_STC  portParams;
    CPSS_PM_AP_LANE_SERDES_SPEED laneSpeed;
    GT_U32                       lanesArrIdx     = 0;
    GT_U32                       portMacNum;
    MV_HWS_PORT_INIT_PARAMS      curPortParams;
    GT_STATUS                    rc;
    PRV_CPSS_PORT_MNG_DB_STC     *tmpPortManagerDbPtr;
    MV_HWS_PORT_STANDARD         portMode=0;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT          speed;


    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);
    portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;

    /* move params from regular to ap*/
    if ((stage == CPSS_PORT_MANAGER_EVENT_CREATE_E) || (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E) ||
        (stage == CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E))
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                       devNum, portNum, rc, ifMode, speed);
        }
        if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                       "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw = GT_TRUE;
        prvCpssPmRegPortParamsGet(devNum, portNum,&regularPortParams);


        portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
        rc = internal_cpssPortManagerInitParamsStruct(portParams.portType, &portParams);

        CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_OPERATIONS_E);
        /* set rx train + fiber*/
        if ((ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR8_E))
        {
            portParams.portParamsType.apPort.apAttrs.portExtraOperation = MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E | MV_HWS_PORT_SERDES_OPERATION_IS_FIBER_E;
        }
        else /* set rx train*/
        {
            portParams.portParamsType.apPort.apAttrs.portExtraOperation = MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E;
        }

        /* set mandatory params*/
        portParams.portParamsType.apPort.numOfModes = 1;
        portParams.portParamsType.apPort.modesArr[0].ifMode = ifMode;
        portParams.portParamsType.apPort.modesArr[0].speed = speed;
        portParams.portParamsType.apPort.modesArr[0].fecRequested = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
        portParams.portParamsType.apPort.modesArr[0].fecSupported = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
        CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_NONCE_E);
        portParams.portParamsType.apPort.apAttrs.nonceDisable = GT_TRUE;
        /* set skip resolution*/
        CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_SKIP_RES_E);
        portParams.portParamsType.apPort.apAttrs.skipRes = GT_TRUE;
        /* set interconnect params */
        if (CPSS_PM_ATTR_IS_VALID(&regularPortParams, CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E))
        {
            CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);
            portParams.portParamsType.apPort.apAttrs.interconnectProfile = regularPortParams.portParamsType.regPort.portAttributes.interconnectProfile;
        }

        if ( HWS_PAM4_MODE_CHECK(portMode) )
        {
            laneSpeed = CPSS_PM_AP_LANE_SERDES_SPEED_PAM4_E;
        }
        else if ( HWS_25G_SERDES_MODE_CHECK(portMode) )
        {
            laneSpeed = CPSS_PM_AP_LANE_SERDES_SPEED_25000_E;
        }
        else
        {
            laneSpeed = CPSS_PM_AP_LANE_SERDES_SPEED_10000_E;
        }

        for (lanesArrIdx = 0; lanesArrIdx < curPortParams.numOfActLanes; lanesArrIdx++)
        {
            if (CPSS_PM_LANE_PARAM_IS_VALID(&regularPortParams, lanesArrIdx, CPSS_PM_LANE_PARAM_TX_E))
            {
                CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);
                CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, lanesArrIdx, CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E);
                portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][lanesArrIdx].txParams.type = CPSS_PORT_SERDES_AVAGO_E;
                cpssOsMemCpy(&(portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][lanesArrIdx].txParams.txTune.avago),
                             &(regularPortParams.portParamsType.regPort.laneParams[lanesArrIdx].txParams.txTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
            }
            if (CPSS_PM_LANE_PARAM_IS_VALID(&regularPortParams, lanesArrIdx, CPSS_PM_LANE_PARAM_RX_E))
            {
                CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);
                CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, lanesArrIdx, CPSS_PM_AP_LANE_OVERRIDE_RX_E);
                portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][lanesArrIdx].rxParams.type = CPSS_PORT_SERDES_AVAGO_E;
                cpssOsMemCpy(&(portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][lanesArrIdx].rxParams.rxTune.avago),
                             &(regularPortParams.portParamsType.regPort.laneParams[lanesArrIdx].rxParams.rxTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
            }

        }

        prvCpssPmApPortParamsSet(devNum, portNum, &portParams);
    }
    return GT_OK;
}

static GT_STATUS prvCpssPortManagerStateCheck
(
    GT_SW_DEV_NUM                    devNum,
    GT_PHYSICAL_PORT_NUM             portNum,
    CPSS_PORT_MANAGER_EVENT_ENT      stage,
    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC *db,
    GT_BOOL                          *exitFromFunc
)
{
    *exitFromFunc = GT_TRUE;
    if (db->portAdminSM == CPSS_PORT_MANAGER_STATE_RESET_E)
    {
        switch (stage)
        {
            case CPSS_PORT_MANAGER_EVENT_ENABLE_E:
            case CPSS_PORT_MANAGER_EVENT_DISABLE_E:
            case CPSS_PORT_MANAGER_EVENT_DELETE_E:
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling ENABLE/DISABLE/DELETE while port is in RESET state, returning GT_NOT_INITIALIZED");
                /* this is not error to the log as the setting speed and mode from LUA on port generate those 'errors' but operation is OK */
                return /*not error to the log*/ GT_NOT_INITIALIZED;
                break;
            case CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E:
            case CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E:
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Calling MAC/LOW level while port is in RESET state, returning GT_OK");
                return GT_OK;
                break;
            case CPSS_PORT_MANAGER_EVENT_PORT_DEBUG_E:
            case CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E:
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling DEBUG while port is in RESET state, returning GT_NOT_INITIALIZED");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "for port=%d",portNum);
                break;
            case CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E:
            case CPSS_PORT_MANAGER_EVENT_CREATE_E:
            case CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E:
                break;
            case CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E:
            case CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E:
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling FORCE/UNFORCE while port is in RESET state, returning GT_NOT_INITIALIZED");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "for port=%d",portNum);
                break;
            default:
                break;
        }
    }

    if (db->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        if ((stage != CPSS_PORT_MANAGER_EVENT_CREATE_E) && (stage != CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E) &&
            (stage != CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Calling event when port already deleted, returning GT_OK");
            return GT_OK;
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] **invalid portType %d stage %d",
                                                       devNum, portNum, db->portType, stage);
        }
    }

    if ((db->portAdminSM != CPSS_PORT_MANAGER_STATE_DEBUG_E ) && (stage == CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is not in debug mode ");
        return GT_OK;
    }

    else if (((stage == CPSS_PORT_MANAGER_EVENT_CREATE_E) ||
              (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E) ||
              (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E))&&
             (db->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] event CREATE not applicable when port is not in RESET", devNum, portNum);
    }

    else if ((db->portOperDisableSM == GT_TRUE) &&
        !((CPSS_PORT_MANAGER_EVENT_ENABLE_E == stage) || (CPSS_PORT_MANAGER_EVENT_DELETE_E == stage) || (CPSS_PORT_MANAGER_EVENT_DISABLE_E == stage)
          || (CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E == stage) || (CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E == stage)) &&
        !(PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
/*        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %2d] event %d not applicable when port is disabled", portNum, stage);*/
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enable operation called upon already enabled port, so just returning.");
        return GT_OK;
    }

    else if ((db->portOperDisableSM == GT_FALSE) && (CPSS_PORT_MANAGER_EVENT_ENABLE_E == stage) && (!db->isRemotePort))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enable operation called upon already enabled port, so just returning.");
        return GT_OK;
    }

    else if ((db->portOperDisableSM == GT_TRUE) && (CPSS_PORT_MANAGER_EVENT_DISABLE_E == stage) && (!db->isRemotePort))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"disable operation called upon already disabled port, so just returning.");
        return GT_OK;
    }

    else if ((db->portAdminSM != CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E ) && (stage == CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is not in force link mode ");
        return GT_OK;
    }
    else if ((db->portAdminSM == CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E ) &&
             ((stage == CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E) || (stage == CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E) ||
             (stage == CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E) || (stage == CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E)))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is already in force link mode ");
        return GT_OK;
    }
    else if ((db->portAdminSM == CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E ) &&
             ((stage != CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E) && (stage != CPSS_PORT_MANAGER_EVENT_DELETE_E) &&
              (stage != CPSS_PORT_MANAGER_EVENT_ENABLE_E) && (stage != CPSS_PORT_MANAGER_EVENT_DISABLE_E)))
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is in force link mode ");
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] event %d not applicable when port is not in FORCE_LINK_DOWN", devNum, portNum,stage);
    }


    *exitFromFunc = GT_FALSE;
    return GT_OK;
}

/**
* @internal internal_cpssPortManagerEventSetfunction
* @endinternal
*
* @brief  getting event and according the port manager state
*         machine, it's handling the "event functions"
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] portEventStc          - struct with the event
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerEventSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN CPSS_PORT_MANAGER_STC *portEventStc
)
{
    GT_STATUS                    rc;
    GT_STATUS                    rc_tmp;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT          speed;
    MV_HWS_PORT_STANDARD         portMode=0;
    GT_U32                       portGroup;
    CPSS_PORTS_BMP_STC           portModeSpeedSetBitmap;
    CPSS_PORT_MANAGER_EVENT_ENT  stage;
    PRV_CPSS_PORT_MNG_DB_STC     *tmpPortManagerDbPtr;
    GT_U32                       portMacNum;
    MV_HWS_PORT_INIT_PARAMS      curPortParams;
    GT_U32                       data;
    GT_U32                       result = 0;
    GT_BOOL                      rxSignalOk;
    CPSS_UNI_EV_CAUSE_ENT        uniEv = CPSS_UNI_RSRVD_EVENT_E; /* igonre uninitialised error */
    CPSS_PORT_MANAGER_PORT_TYPE_ENT portType;
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;
    GT_BOOL                      anyLoopback, bool = GT_FALSE;
    CPSS_PORT_FEC_MODE_ENT       fecAbility;
    GT_U16                       newUpdate;
    GT_BOOL linkUp = GT_FALSE;
    GT_BOOL isRemotePort = GT_FALSE, exitFromFunc;
    GT_BOOL preemptionRollback = GT_FALSE, alignLock;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    CPSS_PM_MAC_PREEMPTION_PARAMS_STC defaultPreemptionParams =
            {
                CPSS_PM_MAC_PREEMPTION_DISABLED_E,
                CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E,
                CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E
            };

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "[Port %d/%2d] port manager not supported for %d device",
                                                   devNum, portNum, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        /*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] failed ppCheckAndGetMacFunc %d",portNum, rc);*/
        return rc;
    }

    /* if DELETE event called on non allocated port, just return GT_OK */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, 0);

    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        if((portEventStc->portEvent != CPSS_PORT_MANAGER_EVENT_CREATE_E)                   &&
           (portEventStc->portEvent != CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E)       &&
           (portEventStc->portEvent != CPSS_PORT_MANAGER_EVENT_DISABLE_E)                  &&
           (portEventStc->portEvent != CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E ))
        {
            return GT_OK;
        }
        else
        {
            GT_BOOL accessEnable;
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                if(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E == tmpPortManagerDbPtr->portMngSmDb[portNum]->portType)
                {
                    /* verify allocation */
                    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

                    ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
                    speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
                    rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                                   devNum, portNum, rc, ifMode, speed);
                    }

                    rc = mvHwsPortAccessCheck(devNum, portMacNum, portMode, &accessEnable);
                    if((GT_OK != rc) || (accessEnable != GT_TRUE))
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d][ifMode_%d][speed_%d] does not exist during HA process", devNum, portNum, ifMode, speed);
                    }
                }
                else if(CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E == tmpPortManagerDbPtr->portMngSmDb[portNum]->portType)
                {
                    MV_HWS_AP_PORT_STATUS apStatus;
                    GT_U32 tmpMacPort;

                    PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);

                    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &tmpMacPort);
                    if (rc!=GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] port check and mac num get failed- %d", devNum, portNum, rc);
                    }

                    rc = mvHwsApPortCtrlStatusGet(devNum, 0, tmpMacPort, &apStatus);
                    if ((GT_OK != rc) || (apStatus.smState == 0))
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] AP does not exist during HA process", devNum, portNum);
                    }
                }
                else
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] does not exist during HA process", devNum, portNum);
                }
            }
        }
    }
    stage = portEventStc->portEvent;
    if (stage != CPSS_PORT_MANAGER_EVENT_INIT_E)
    {
        PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(stageInit)[portNum] = GT_FALSE;
    }

    rc = prvCpssPortManagerStateCheck(devNum, portNum, stage, tmpPortManagerDbPtr->portMngSmDb[portNum], &exitFromFunc);
    if ( (GT_OK != rc) || (exitFromFunc))
    {
        return rc;
    }
    if (stage ==  CPSS_PORT_MANAGER_EVENT_PORT_DEBUG_E )
    {
         tmpPortManagerDbPtr->portMngSmDb[portNum]->enterDebugState = tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM;
         tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_DEBUG_E;
         CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CPSS_PORT_MANAGER_EVENT_PORT_DEBUG_E ");
         /* stop cm3 */
         if((CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily) )
         {
             if(mvHwsPortApSerdesAnDisableSetIpc(devNum,0, portMacNum, GT_TRUE, GT_FALSE) != GT_OK)
             {
                 CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] disable cm3 failed", devNum, portNum);
             }
         }

         return GT_OK;
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_DEBUG_E )
    {
        if ((stage != CPSS_PORT_MANAGER_EVENT_DELETE_E) && (stage != CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is in debug mode ");
            return GT_OK;
        }
        else
        {
            if (stage == CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E ");
                if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->enterDebugState != CPSS_PORT_MANAGER_STATE_LINK_UP_E)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is in debug mode while link is not up");
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }else
                {   /*check pcs align to see if link is still up*/
                    rc = prvCpssPortPcsAlignLockStatusGet(CAST_SW_DEVNUM(devNum), portNum,portMacNum, &alignLock);
                    if ((rc != GT_OK) || (alignLock == GT_FALSE))
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is in debug mode and pcs is down");
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                    }

                }
            }
            tmpPortManagerDbPtr->portMngSmDb[portNum]->enterDebugState = CPSS_PORT_MANAGER_STATE_RESET_E;
            /* stop debug mode and start cm3*/
            if((CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily) )
            {
                if(mvHwsPortApSerdesAnDisableSetIpc(devNum,0, portMacNum, GT_FALSE, GT_FALSE) != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] enable cm3 failed", devNum, portNum);
                }
            }
            if (stage == CPSS_PORT_MANAGER_EVENT_PORT_NO_DEBUG_E)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port is out of debug mode ");
                return GT_OK;
            }
        }
    }
    portType = tmpPortManagerDbPtr->portMngSmDb[portNum]->portType;
    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    speed = CPSS_PORT_SPEED_NA_E;


    isRemotePort = tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort;
    rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, portType, &loopbackCfg, &anyLoopback);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                   devNum, portNum, rc);
    }

    if (portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        /* verify allocation */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 1);
        ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
        speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;

        /* debugger, every update on the state, enter to the log the current state */
        newUpdate = (GT_U16)((stage<<8) | (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM));
        if (tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->lastUpdate != newUpdate)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "stage %d AdminSM %d stabilityDone %d trainDone %d alignLockDone %d rxStabilityTrainDone %d signalDetected %d",
                                                  stage, tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM, tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone,
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone, tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone,
                                                  tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone, tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected);
            tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->lastUpdate = newUpdate;

        }
        rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
        if (rc != GT_OK)
        {
         CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                    devNum, portNum, rc, ifMode, speed);
        }

           /* working with CM3 instead of PM on host */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw)
        {
            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) && (anyLoopback == GT_FALSE) &&
                (isRemotePort == GT_FALSE) && (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) && (portMode != _5GBaseR))
            {
                /*check loopback and speed*/
                prvCpssPortManagerCopyRegToApPort(devNum, portNum, stage);
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] pmOverFw is not valid", devNum, portNum);
            }
        }

    }

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)|| PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            if (stage == CPSS_PORT_MANAGER_EVENT_DISABLE_E)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;
                return GT_OK;
            }
            else if (stage == CPSS_PORT_MANAGER_EVENT_ENABLE_E)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
                return GT_OK;
            }
        }
    }

    if (stage == CPSS_PORT_MANAGER_EVENT_REMOTE_FAULT_TX_CHANGE_E)
    {
        /* The following API is called for all port types.
           for non CG or XG ports it will return GT_NOT_SUPPORTED */
        if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortRemoteFaultConfigGetFunc != NULL)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortRemoteFaultConfigGetFunc(devNum, portNum, &bool);
            if ( (rc != GT_OK) && (rc != GT_NOT_SUPPORTED) )
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] failed ppPortRemoteFaultConfigGetFunc %d", devNum, portNum, rc);
            }

            /* printf("Doorbell\n"); */
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Port %d remote fault changed to %d", portNum, bool);
        }

        return GT_OK;
    }
    /* start timestamp for pm operation*/
    CHECK_STATUS(cpssOsTimeRT(&(PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampSec)), &(PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampNSec))));

    /* AP port will be configured only in this next scope */
    if ((portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E) || (tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw))
    {
        if(isRemotePort)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] **invalid for Remote Port ", devNum, portNum);
        }

        rc = prvCpssPortManagerApEventSet(CAST_SW_DEVNUM(devNum), portNum, stage, tmpPortManagerDbPtr);
        return rc;
    }

    portMode   = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;

    if(isRemotePort)
    {
        /* Check if cascade port is configured */
        PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(tmpPortManagerDbPtr, portMacNum, 1);
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC( GT_NOT_INITIALIZED, "[Port %d/%2d] hwsPortModeParamsGetToBuffer returned null portMacNum %d portMode %d portType %d ifMode %d speed %d stage %d",
                                                    devNum, portNum, portMacNum, portMode, portType, ifMode, speed, stage);
    }

/**--------------------------------------------------------EVENTS-HANDLER---------------------------------------------------------*/

    /**=========================*/
    /** EVENT: STATUS CHANGED   */
    /**=========================*/
    /* Those events will call when we have change in the low/mac level.
        * Triggered by interrupts
        * Mac event will checking the port link up status
        * Low event will mask if needed the interrupts and will call to init event */
    if (stage == CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E || stage == CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E)
    {
        if (stage == CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E");
        }
        else
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E");
        }

        rc = prvCpssPortManagerStatusChanged(devNum, portNum, tmpPortManagerDbPtr, &stage);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerStatusChanged returned error code %d ",devNum, portNum, rc);
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish STATUS CHANGED");
    }

    /**=========================*/
    /**      EVENT: ENABLE      */
    /**=========================*/
    /* Enable event resume the port after disable
       hint: In enable the ports using the saved database and just powering the hardware components.*/
    if (stage == CPSS_PORT_MANAGER_EVENT_ENABLE_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: PORT_MANAGER_EVENT_ENABLE_E");

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                return rc;
            }
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish ENABLE");
            return GT_OK;
        }

        if(isRemotePort)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                return rc;
            }

            rc = prvCpssPortManagerLinkUpCheck(devNum,portNum,GT_TRUE,tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                prvCpssPortManagerModeToDownState(devNum,portNum);
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
                return rc;
            }
            return GT_OK;
        }

        rc = prvCpssPortManagerEnable(devNum, portNum, &curPortParams, tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
             CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnable returned error code %d ", devNum, portNum, rc);
        }
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish ENABLE");
    }

    /**=========================*/
    /**      EVENT: INIT        */
    /**=========================*/
    /* After configure the port on the create, init event processing the port (according to the speed, mode and the other parameters), and checking:
       Signal stability, Gearbox lock, Align lock, Tx/Rx training, Calibration and more */
    if (stage == CPSS_PORT_MANAGER_EVENT_INIT_E)
    {
        if(PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode) && portMode != _1000Base_X)
        {
            rc = prvCpssPortManagerSimpleInitialize(devNum ,portNum ,tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerInitializeLowSpeeds returned error code %d ", devNum, portNum, rc);
            }
        }
        else
        {
            rc = prvCpssPortManagerInitialize(devNum, portNum, &curPortParams, tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerInitialize returned error code %d ", devNum, portNum, rc);
            }
        }
    }

    /**=========================*/
    /**      EVENT: DISABLE     */
    /**=========================*/
    /* Disable event putting the link to down.
       disable port high level unit as there will no be signal trigger that will perform stuff then allow link level
       hint: In disable only the hardware components are powered off but all the PM attributes are saved.*/
    if (stage == CPSS_PORT_MANAGER_EVENT_DISABLE_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: PORT_MANAGER_EVENT_DISABLE_E");

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) )
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                return rc;
            }
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish DISABLE");
            return GT_OK;
        }

        if(isRemotePort)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                return rc;
            }
            rc = prvCpssPortManagerLinkUpCheck(devNum,portNum,GT_TRUE,tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                prvCpssPortManagerModeToDownState(devNum,portNum);
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
                return rc;
            }
            return GT_OK;
        }

        rc = prvCpssPortManagerDisable(devNum,portNum,tmpPortManagerDbPtr);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerDisable returned error code %d ",devNum, portNum, rc);
        }

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish DISABLE");
    }

    /**==============================*/
    /**      EVENT: DELETE           */
    /**      EVENT: FORCE_LINK_DOWN  */
    /**==============================*/
    /* The port will be Deleted, all hardware components will be powered off and all the port attributes will be removed from both PM and CPSS data bases.
       In order to re-create the port, application must call to set parameters again before create.
        if event is CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E pm db will remain*/
    else if ((stage == CPSS_PORT_MANAGER_EVENT_DELETE_E) || (stage == CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E))
    {
        if (stage == CPSS_PORT_MANAGER_EVENT_DELETE_E)
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: CPSS_PORT_MANAGER_EVENT_DELETE_E");
        else
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E");

        rc = prvCpssPortManagerDelete(devNum, portNum, &curPortParams, tmpPortManagerDbPtr, stage);
        if (rc != GT_OK)
        {
            return rc;
        }
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Finish DELETE/FORCE_LINK_DOWN");
    }
    /**====================================*/
    /** EVENT: CREATE_AND_FORCE_LINK_DOWN  */
    /**====================================*/
    else if (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E");
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_TRUE;
        /* oper enable is the default */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish CREATE_AND_FORCE_LINK_DOWN");
    }
    /**==================================*/
    /**      EVENT: CREATE               */
    /**      EVENT: CREATE_AND_DISABLE   */
    /**      EVENT: UNFORCE_LINK_DOWN    */
    /**==================================*/
    /* include create and disable event */
    /* This event set start execute of the port manager, so when we working on port manager, this will be the first event for the port.
     - Un/masking interrupts
     - Fetching all serdes/parameters from DB and applying them to hardware.
     - Disable mac and enable remote fault transmition
     - Forcing link down
     - Checking loopback
     - Configure the port with port mode speed set
      */
    if ((stage == CPSS_PORT_MANAGER_EVENT_CREATE_E) ||/* Stage create */
        (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E) ||
        (stage == CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E))
    {
        if (stage == CPSS_PORT_MANAGER_EVENT_CREATE_E)
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: PORT_MANAGER_EVENT_CREATE *****");
        else if (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E)
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: %d PORT_MANAGER_EVENT_CREATE_AND_DISABLE *****");
        else
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Stage: %d PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN *****");

        portGroup = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup;

        /* oper enable is the default */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_CREATE_PORT_FAILED_E;
        if (stage != CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E)
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_FALSE;

        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portModeSpeedSetBitmap);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portModeSpeedSetBitmap, portNum);

        tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged = GT_TRUE;
        if ( !isRemotePort && !(PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) )
        {
            rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
            }
        }
        else if (PRV_CPSS_PORT_MANAGER_CHECKING_GIGE_PORTS_ON_AC3_BASED_DEVICE(devNum,portNum))
        {
            rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
            }
        }
        else if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            rc = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
            }
        }
        /* ensuring port will be in link down before portModeSpeedSet, this way when port will
          be brought up we ensure there will be no link (if we will perform this operation after
          portModeSpeedSet, then between portModeSpeedSet and a call to this API, peer can establish link) */
        if(!isRemotePort)
        {
            rc = prvCpssPortManagerEnableLinkLevel(devNum, portNum, ifMode, speed, GT_FALSE);
            if (rc != GT_OK)
            {
                if ( !PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode) )
                {
                    rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                    if (rc_tmp != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
                    }
                }
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerEnableLinkLevel"
                                                       " returned error code %d ",
                                                       devNum, portNum, rc);
            }
        }
        /* loopback configuration - get from DB */
        rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, portType, &loopbackCfg, &anyLoopback);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                       devNum, portNum, rc);
        }

        /*Set Bandwidth mode of TX_FIFO Unit Pizza Arbiter*/
        if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->bwMode != CPSS_PORT_PA_BW_MODE_REGULAR_E) &&
            (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc != NULL) &&
            (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc(CAST_SW_DEVNUM(devNum), portNum, tmpPortManagerDbPtr->portMngSmDb[portNum]->bwMode);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortBwTxFifoSetFunc returned error code %d ", devNum, portNum, rc);
            }
        }

       /* if user configured FEC mode, this is the place to set it */
        if (PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && ((HWS_PAM4_MODE_CHECK(portMode)) ||(HWS_IS_PORT_MODE_200G(portMode))))
            { /*sip 6 not support fec disable for some speeds*/
                fecAbility = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            }
            else
            {
                fecAbility = CPSS_PORT_FEC_MODE_DISABLED_E;
            }
        }
        else
        {
            fecAbility = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
        }

        /* serdes that override before port mode speed set */
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)
              && !PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            rc = prvCpssPortManagerSerdesOverride(devNum,portNum,curPortParams);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerSerdesOverride failed=%d\n", devNum, portNum,rc);
            }
        }

        if ((PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].preemptionSupported) &&
            (tmpPortManagerDbPtr->portMngSmDb[portNum]->preemptionParams.type != CPSS_PM_MAC_PREEMPTION_DISABLED_E))
        {
            /*TBD - check that speed and mode support preemption*/
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPreemptionParamsSetFunc(devNum,portNum,
            &(tmpPortManagerDbPtr->portMngSmDb[portNum]->preemptionParams));
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssDxChTxqSip6_10PreemptionEnableSet failed=%d\n",devNum, portNum,rc);
            }
            preemptionRollback = GT_TRUE;
        }

        /* bringing up the port using portModeSpeedSet */
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc(devNum, portModeSpeedSetBitmap, GT_TRUE, ifMode, speed,
                                                                                     tmpPortManagerDbPtr->portMngSmDb[portNum] /* fecAbility*/);
        if (rc != GT_OK)
        {
            if(GT_TRUE == preemptionRollback)
            {
                rc_tmp = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPreemptionParamsSetFunc(devNum,portNum,
                    &defaultPreemptionParams/*return to default*/);
                if (rc_tmp!=GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %d/%2d] prvCpssDxChTxqSip6_10PreemptionEnableSet failed=%d\n", devNum, portNum,rc);
                }
            }

            if (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))
            {
                rc_tmp = prvCpssPortManagerMaskEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                if (rc_tmp != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc_tmp, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
                }
            }

            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppPortModeSpeedSetFunc"
                                                       " returned error code %d ",
                                                       devNum, portNum, rc);
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !isRemotePort)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPtpDelayParamsSetFunc(devNum,portNum);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppPortPtpDelayParamsSetFunc failed=%d\n", devNum, portNum,rc);
            }
        }

        /* Auto-Negotiation sequence for slow modes except SGMII2_5 and _2500Base_X for SIP_6 devices */
        if (PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) || (SGMII2_5 != portMode && _2500Base_X != portMode)))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Configure Auto-Negotiation for slow modes");
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppAutoNeg1GSgmiiFunc(devNum, portNum, ifMode, &(tmpPortManagerDbPtr->portMngSmDb[portNum]->autoNegotiation));
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppAutoNeg1GSgmiiFunc returned %d \n ", devNum, portNum, rc);
            }
        }

        /* serdes that override after port mode speed set */
        if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) || PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            rc = prvCpssPortManagerSerdesOverride(devNum,portNum,curPortParams);
            if (rc!=GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerSerdesOverride failed=%d\n", devNum, portNum,rc);
            }
        }

        if((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc != NULL) &&
            (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperationsBitmap)))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc(CAST_SW_DEVNUM(devNum), portNum,
                                                                                               portGroup, ifMode, speed,
                                                                                               tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperationsBitmap,&result);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortExtraOperationsSetFunc returned error code %d ", devNum, portNum, rc);
            }
        }

        /* update port manager SM status */
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_DOWN_E;
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"setting port FEC parameter %d",
                                              tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility);

        if (isRemotePort)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
            rc = prvCpssPortManagerLinkUpCheck(devNum,portNum,GT_TRUE,tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E;
                tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_CREATE_PORT_FAILED_E;
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Calling prvCpssPortManagerLinkUpCheck returned %d ", rc);
                return rc;
            }
            return GT_OK;
        }

        /* if user configured FEC mode, this is the place to set it */
        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility != CPSS_PORT_FEC_MODE_LAST_E)
        {
#if 0
            if (PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
            {
                if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (HWS_PAM4_MODE_CHECK(portMode)))
                { /*sip 6 not support fec disable for some speeds*/
                    fecAbility = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                }
                else
                {
                    fecAbility = CPSS_PORT_FEC_MODE_DISABLED_E;
                }
            }
            else
            {
                fecAbility = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
            }
#endif
            if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeSetFunc(devNum, portNum, fecAbility);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppFecModeSetFunc returned error code returned error code %d ",
                                                               devNum, portNum, rc);
                }
            }
        }

        if (tmpPortManagerDbPtr->portManagerGlobalUseFlag == 0)
            {
                tmpPortManagerDbPtr->portManagerGlobalUseFlag = 1;
            }

        if (stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E && !(PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            /* config loopback */
            if (anyLoopback == GT_TRUE)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"config loopback type %d  enableRegularTrafic %d",loopbackCfg.loopbackType,loopbackCfg.enableRegularTrafic);
                rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                               devNum, portNum, rc);
                }
            }
            if (PRV_PORT_MANAGER_CHECK_XLG_CG_MAC_PORT_MAC(curPortParams))
            {
                CPSS_LOG_INFORMATION_MAC("handling %d XLG macs", curPortParams.numOfActLanes);
                rc = prvCpssPortXlgMacMaskConfig(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortXlgMacMaskConfig returned %d ", devNum, portNum, rc);
                }
            }

            rc = prvCpssPortManagerDisable(devNum,portNum,tmpPortManagerDbPtr);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerDisable returned error code %d ",
                                                           devNum, portNum, rc);
            }

            tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish CREATE_AND_DISABLE");
            return rc;
        }

        if (!(PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) || CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
        {
            /* clear cause bit of interrupt link_status_change */
            uniEv = CPSS_PP_PORT_LINK_STATUS_CHANGED_E;

            /*for AC5P a convert portNum needed for finding the address, in order to not override the port num, using temp port num */
            if ((HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum)) && PRV_NON_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssEventExtDataConvertFunc) && !isRemotePort)
            {
                GT_U32 portNumTmp = portNum;
                rc = PRV_NON_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssEventExtDataConvertFunc)(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                                      PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E,
                                      &portNumTmp);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform data convert", devNum, portNum);
                }
                portMacNum = portNumTmp;
            }

            CPSS_TBD_BOOKMARK
            /* Check if need the following code for QSGMII mode also */
            /*  || (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E))*/
            if ((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
            {
                rc = prvCpssPortManagerEventBitMaskingReading(devNum, portNum,portMacNum, &curPortParams, tmpPortManagerDbPtr, uniEv, &data);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] calling prvCpssPortManagerEventBitMaskingReading %d ", devNum, portNum, rc);
                }
            }

            /* convert port mac back for AC5P */
            if (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                portMacNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
            }

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"clearing LINK_STATUS and SIGNAL_DETECT interrupt for mac %d numOfActLanes %d", (portMacNum), curPortParams.numOfActLanes);

            if ((!PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg)) && ((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) ||
                 (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) && !CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))) )
            {
                /* unmask the mask we did prior to portModeSpeedSet */
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
                }
            }

            /* loopback configuration - now set to HW.
               Skip for serdes Rx2Tx since this one needs signal in its RX during the configuration, we will set it when we get low level change interrupt */
            if (anyLoopback == GT_TRUE && !(PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_DIGITAL_RX2TX_CHECK(loopbackCfg)))
            {
                rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
                {
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "config loopback type %d  enableRegularTrafic %d", loopbackCfg.loopbackType, loopbackCfg.enableRegularTrafic);
                    rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                                   devNum, portNum, rc);
                    }
                    if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) ||
                        (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)))
                    {
                        if (loopbackCfg.enableRegularTrafic == GT_FALSE)
                        {
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"In TX2RX mode serdes Tx is disable ");
                            rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, GT_FALSE);
                            if(rc != GT_OK)
                            {
                                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsPortTxEnable failed=%d", devNum, portNum, rc);
                            }
                        }
                    }

                    if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) && (loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E))
                    {
                        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortForceLinkPassSetFunc(devNum,portNum,GT_TRUE);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                            return rc;
                        }
                    }
                }
            }

        /* (checking o_core_status[4] for rx_signal_ok, if it is true, then pusing
              signal_detect interrupt to the event queue. */
            if (!(PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg) || PRV_PORT_MANAGER_POLLING_DEVICES(devNum)))
            {
                GT_U32  realMacNum = portMacNum;
                if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
                {
                    portMacNum &= 0xFFFFFFFC;
                }

                rxSignalOk = GT_FALSE;

                rc = mvHwsPortAvagoSerdesRxSignalOkStateGet(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, &rxSignalOk);
                portMacNum = realMacNum;
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] cpssDrvPpHwRegBitMaskRead failed=%d", devNum, portNum, rc);
                }
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"rx_signal_ok state=%d", rxSignalOk);
                /* pusing signal_detect interrupt (that we cleared) to the event queue.
                  Only a single event needed in multi and non-multi lanes interface as we only need
                  one interrupt for the port manager to continue job processing*/

                /* to push event anyways, need to enable macro CPSS_PORT_MANAGER_IGNORE_SIG_OK */
                if (rxSignalOk == GT_TRUE)
                {
                    /* Reading the cause register of the interrupt */
                    if (PRV_CPSS_PORT_MANAGER_CHECKING_GIGE_PORTS_ON_BOBK_CAELUM_BASED_DEVICE(devNum,portNum))
                    {/* bobk on gige ports don't have support for mpcs, use sync status */
                        uniEv = CPSS_PP_PORT_SYNC_STATUS_CHANGED_E;
                    }
                    else
                    {
                        uniEv = CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E;
                    }

                    /* This is used for clearing the regiseter and no for reading the data */
                    if (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
                    {
                        rc = prvCpssPortManagerEventBitMaskingReading(devNum, portNum,portMacNum, &curPortParams, tmpPortManagerDbPtr, uniEv, &data);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] calling prvCpssPortManagerEventBitMaskingReading %d ", devNum, portNum, rc);
                        }
                    }

                    rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum/*extraData*/, uniEv);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerPushEvent"
                                                                   " failed=%d", devNum, portNum, rc);
                    }
                }
            }

            if (!(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E) && !(PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg)) &&
                !(PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)))
            /* no need to check iCal for mac loopback */
            {
                /* for polling devices iCal without electrical idels detect will drop */
                if (!PRV_PORT_MANAGER_POLLING_DEVICES(devNum))
                {
                    rc = prvCpssPortManagerSigStableRxTraining(devNum,portNum);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerSigStableRxTraining returned error code %d ",
                                                                   devNum, portNum, rc);
                    }
                }
            }

            if (!PRV_CPSS_PORT_MANAGER_SLOW_MODE_CHECK(portMode) || (CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E == ifMode))
            {
                tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone = CPSS_PORT_MANAGER_IN_PROGRESS_E;
            }

        }
        else if (PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode))/* handling loopback for low speeds */
        {
            /* loopback configuration - get from DB */
            rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, portType, &loopbackCfg, &anyLoopback);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                           devNum, portNum, rc);
            }

            /* loopback configuration - now set to HW (not serdes Rx2Tx)*/
            if (anyLoopback == GT_TRUE)
            {
                if ((loopbackCfg.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) ||
                    (PRV_CPSS_PORT_MANAGER_SERDES_LOOPBACK_ANALOG_TX2RX_CHECK(loopbackCfg)))
                {
                    /* loopback configuration - now set to HW.
                    Skip for serdes Rx2Tx since this one needs signal in its RX during the configuration, we will set it when we get low level change interrupt */
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"config loopback type %d  enableRegularTrafic %d",loopbackCfg.loopbackType,loopbackCfg.enableRegularTrafic);
                    rc = prvCpssPortManagerLoopbackModesHwSet(devNum, portNum, ifMode, speed, &loopbackCfg);
                    if (rc != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesHwSet failed=%d",
                                                                   devNum, portNum, rc);
                    }

                    if(loopbackCfg.enableRegularTrafic == GT_FALSE)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"In TX2RX mode serdes Tx is disable ");
                        rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, GT_FALSE);
                        if(rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] mvHwsPortTxEnable failed=%d", devNum, portNum, rc);
                        }
                    }
                }
            }

            if (PRV_CPSS_PORT_MANAGER_CHECKING_GIGE_PORTS_ON_BOBK_CAELUM_BASED_DEVICE(devNum,portNum))
            {
                uniEv = CPSS_PP_PORT_SYNC_STATUS_CHANGED_E;
                rc = prvCpssPortManagerEventBitMaskingReading(devNum, portNum,portMacNum, &curPortParams, tmpPortManagerDbPtr, uniEv, &data);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] calling prvCpssPortManagerEventBitMaskingReading %d ", devNum, portNum, rc);
                }

                rc = prvCpssPortManagerPushEvent(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMacNum/*extraData*/, uniEv);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerPushEvent"
                                                               " failed=%d", devNum, portNum, rc);
                }
            }
        }

        /****************************************
            3. Unmasking events
        *****************************************/

        if (PRV_PORT_MANAGER_CHECK_XLG_CG_MAC_PORT_MAC(curPortParams))
        {
            CPSS_LOG_INFORMATION_MAC("handling %d XLG macs", curPortParams.numOfActLanes);
            rc = prvCpssPortXlgMacMaskConfig(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortXlgMacMaskConfig returned %d ", devNum, portNum, rc);
            }
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected      = GT_FALSE;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone = CPSS_PORT_MANAGER_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone     = CPSS_PORT_MANAGER_RESET_E;
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone = CPSS_PORT_MANAGER_RESET_E;

        if((PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))&& stage == CPSS_PORT_MANAGER_EVENT_CREATE_AND_DISABLE_E)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM = GT_TRUE;

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish CREATE_AND_DISABLE");
        }


        if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM == GT_TRUE)
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"CREATE_AND_DISABLE disable MAC");
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc);
                return rc;
            }
        }




        if (PRV_CPSS_PORT_MANAGER_MAC_LOOPBACK_CHECK(loopbackCfg))
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port enable with MAC loopback");

            if((!(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))) ||
                ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM == GT_FALSE) && (PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
            {

                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    rc_tmp = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc(devNum, portNum, GT_FALSE);
                    if (rc_tmp != GT_OK)
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"enabling port returned error code %d ", rc_tmp);
                    }
                }
            }
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
            {
                rc = prvCpssPortManagerMaskLowLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_MASK_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform maskUnmask operation", devNum, portNum);
                }
            }
            rc = prvCpssPortManagerMaskMacLevelEvents(CAST_SW_DEVNUM(devNum), portNum, CPSS_EVENT_UNMASK_E);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] could not perform mac level maskUnmask operation", devNum, portNum);
            }

            rc = prvCpssPortManagerLinkUpCheck(devNum, portNum, GT_TRUE, tmpPortManagerDbPtr);
            if(rc != GT_OK)
            {
                prvCpssPortManagerModeToDownState(devNum,portNum);
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling prvCpssPortManagerLinkUpCheck - rc %d ", devNum, portNum, rc);
            }
        }
        tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason = CPSS_PORT_MANAGER_FAILURE_NONE_E;

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Finish CREATE");
        CPSS_PM_START_TIME(tmpPortManagerDbPtr->portMngSmDb[portNum]);
    } /* EVENT_CREATE or EVENT_CREATE_AND_DISABLE */

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if  (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /*check if link is up */
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusGetFunc(devNum, portNum, &linkUp);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppLinkStatusGetFunc returned %d ", devNum, portNum, rc);
        }
        if (linkUp == GT_TRUE)
        {
            tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM = CPSS_PORT_MANAGER_STATE_LINK_UP_E;
        }
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"process HA - linkUp - %d",linkUp);
    }
    /* check timestamp for pm operation*/
    CHECK_STATUS(cpssOsTimeRT(&(PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampSecEnd)), &(PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampNSecEnd))));
    {
        int temp = PRV_PORT_MANAGER_TIME_DIFF(PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampSec),PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampNSec),PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampSecEnd),PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(globalTimeStampNSecEnd));
        if (temp > 5)
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"total time %d",temp);
    }
    return rc;
}

/**
* @internal internal_cpssPortManagerStatusGet
* @endinternal
*
* @brief  API to get the port manager status that configure on the port
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
*
* @param[out] portStagePtr         - struct with port PM data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerStatusGet
(
    IN GT_SW_DEV_NUM                  devNum,
    IN GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC  *portStagePtr
)
{
    GT_STATUS                rc;
    GT_U32                   portMacNum;
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;
    /*CPSS_PORT_AP_STATUS_STC  apStatus;*/
    CPSS_PORT_FEC_MODE_ENT   fecAbility;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);
    if (!CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc!=GT_OK)
    {
        /*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %2d] failed ppCheckAndGetMacFunc %d",portNum, rc);*/
        return rc;
    }

    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(portStagePtr);

    cpssOsMemSet(portStagePtr, 0, sizeof(CPSS_PORT_MANAGER_STATUS_STC));

    portStagePtr->portState            = CPSS_PORT_MANAGER_STATE_RESET_E;
    portStagePtr->portUnderOperDisable = GT_FALSE;
    portStagePtr->failure              = CPSS_PORT_MANAGER_FAILURE_NONE_E;
    portStagePtr->ifMode  = CPSS_PORT_INTERFACE_MODE_NA_E;
    portStagePtr->speed   = CPSS_PORT_SPEED_NA_E;
    portStagePtr->fecType = CPSS_PORT_FEC_MODE_LAST_E;

    /* verify allocation - if not allocated, returning GT_OK with above initial statuses (RESET...)*/
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(tmpPortManagerDbPtr, portNum, 0);
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(tmpPortManagerDbPtr, portNum, 0);
    }


    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged == GT_TRUE)
    {
        /*portStagePtr->isPortUnderManagement = GT_TRUE;*/
        portStagePtr->portState = tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM;
        portStagePtr->portUnderOperDisable = tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM;
        portStagePtr->failure = tmpPortManagerDbPtr->portMngSmDb[portNum]->portLastFailureReason;
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E )
        {
            portStagePtr->ifMode = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
            portStagePtr->speed = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
        }
        else
        {
            portStagePtr->ifMode = CPSS_PORT_INTERFACE_MODE_NA_HCD_E;
            portStagePtr->speed = CPSS_PORT_SPEED_NA_HCD_E;
        }
    }


    if (portStagePtr->portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
    {
        switch (tmpPortManagerDbPtr->portMngSmDb[portNum]->portType)
        {
        case CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E:
            /* Speed to be updated with actual speed if PHY does AN */
            if((CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(portStagePtr->ifMode)) || (portStagePtr->ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E))
            {
                CPSS_PORT_SPEED_ENT speed;
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc(devNum, portNum, &speed);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc ,"[Port %d/%2d] Calling ppSpeedGetFromExtFunc returned error code %d ",
                                                           devNum, portNum, rc);
                }
                portStagePtr->speed = speed;
            }

            if (tmpPortManagerDbPtr->portMngSmDb[portNum]->isRemotePort == GT_TRUE)
            {
                fecAbility = tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility;
            }
            else
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeGetFunc(devNum, portNum, &fecAbility);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling ppFecModeGetFunc returned error code returned error code %d ",
                                                           devNum, portNum, rc);
                }
            }
            portStagePtr->fecType = fecAbility;
            break;
        case CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E:
            /*rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApStatusGetFunc(CAST_SW_DEVNUM(devNum),portNum,&apStatus);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %2d] cpssPortApPortStatusGet:rc=%d\n",
                                                           portNum, rc);
            }

            portStagePtr->ifMode = apStatus.portMode.ifMode;
            portStagePtr->speed = apStatus.portMode.speed;
            portStagePtr->fecType =  apStatus.fecType;*/
            portStagePtr->ifMode =tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.ifModeDb;
            portStagePtr->speed = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.speedDb;
            portStagePtr->fecType = tmpPortManagerDbPtr->portsApAttributesDb[portNum]->portResolution.fecStatus;
            break;
        default:
            ;
        }
    }

    /* for non CG or XG ports this API will return GT_NOT_SUPPORTED */
    if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortRemoteFaultConfigGetFunc != NULL)
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortRemoteFaultConfigGetFunc(devNum, portNum, &portStagePtr->remoteFaultConfig);
        if ( (rc != GT_OK) && (rc != GT_NOT_SUPPORTED) )
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] failed ppPortRemoteFaultConfigGetFunc %d", devNum, portNum, rc);
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerDbgInitPorts
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_U32 timeOut
)
{
    /* Variables declarations */
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_STATUS rc;
    GT_BOOL  portsExistFlag;
    CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
    CPSS_PORT_MANAGER_STC portEventStc;
    GT_U32 dbgCounter;

    portsExistFlag = GT_TRUE;
    dbgCounter = 0;

    while (portsExistFlag == GT_TRUE)
    {
        portsExistFlag = GT_FALSE;

        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            /************************************************************************
              1. Get port status
            **********************************************************************/

            rc = internal_cpssPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
            if (rc != GT_OK)
            {
                continue;/*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, " Calling internal_cpssPortManagerStatusGet returned %d ", rc);*/
            }

            /************************************************************************
              2. If port state is INIT than continue to call INIT_EVENT
            **********************************************************************/
            if (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
            {
                /*cpssOsPrintf CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"performing DBG INIT");*/
                portsExistFlag = GT_TRUE;
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;
                rc = internal_cpssPortManagerEventSet(devNum, portNum, &portEventStc);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling internal_cpssPortManagerEventSet returned %d ",
                                                               devNum, portNum, rc);
                }
            }
        }

        dbgCounter++;

        if (timeOut>0)
        {
            cpssOsTimerWkAfter(timeOut);
        }

        /* for preventing GalTis from context hang */
        if (dbgCounter>10000)
        {
            prvCpssCyclicLoggerLogEntriesDump();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "[Port %d/%2d] prvCpssPortManagerDbgInitPorts reached time-out", devNum, portNum);
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssPortManagerDbgDumpPortState(GT_SW_DEV_NUM devNum, GT_PHYSICAL_PORT_NUM portNum)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
    char *stateStr;
    char *failStr;

    rc = internal_cpssPortManagerStatusGet(devNum, portNum, &portConfigOutParams);

    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Calling internal_cpssPortManagerStatusGet returned %d ", devNum, portNum, rc);
    }

    switch (portConfigOutParams.portState)
    {
    case CPSS_PORT_MANAGER_STATE_RESET_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_RESET_E";
        break;
    case CPSS_PORT_MANAGER_STATE_LINK_DOWN_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_LINK_DOWN_E";
        break;
    case CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E";
        break;
    case CPSS_PORT_MANAGER_STATE_LINK_UP_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_LINK_UP_E";
        break;
    case CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E";
        break;
    case CPSS_PORT_MANAGER_STATE_FAILURE_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_FAILURE_E";
        break;
    case CPSS_PORT_MANAGER_STATE_LAST_E:
        stateStr = "CPSS_PORT_MANAGER_STATE_LAST_E";
        break;
    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                                   "[Port %d/%2d] portState undefined %d ",devNum, portNum, portConfigOutParams.portState);
    }

    switch (portConfigOutParams.failure)
    {
    case CPSS_PORT_MANAGER_FAILURE_NONE_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_NONE_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_SIGNAL_STABILITY_FAILED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_SIGNAL_STABILITY_FAILED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_ALIGNMENT_TIMER_EXPIRED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_ALIGNMENT_TIMER_EXPIRED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_CONFIDENCE_INTERVAL_TIMER_EXPIRED_E:
        failStr = "CPSS_PORT_MANAGER_FAILURE_CONFIDENCE_INTERVAL_TIMER_EXPIRED_E";
        break;
    case CPSS_PORT_MANAGER_FAILURE_LAST_E:
    default:
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                                   "[Port %d/%2d] portFail undefined %d ", devNum, portNum, portConfigOutParams.failure);
    }
    cpssOsPrintf("[Port %2d] state: %s, fail: %s, operDisabled: %d ",
                 portNum,
                 stateStr,
                 failStr,
                 /*portConfigOutParams.isPortUnderManagement,*/
                 portConfigOutParams.portUnderOperDisable);
    return GT_OK;
}

/**
* @internal internal_cpssPortManagerGlobalParamsOverride function
* @endinternal
*
* @brief  Override global parameters, using flags to check if
*     override or using defaults
*
* @param[in] devNum                - device number
* @param[in] globalParamsStcPtr    - stc with the parameters to override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad physical port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerGlobalParamsOverride
(
    IN GT_U8                                 devNum,
    IN CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC   *globalParamsStcPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;


    /*CPSS_PORT_MANAGER_MODE_PARAMS_STC*/
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if (globalParamsStcPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Port manager not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    if (CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_SIGNAL_DETECT_E, globalParamsStcPtr->globalParamsBitmapType))
    {
        tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeInterval
        = globalParamsStcPtr->signalDetectDbCfg.sdChangeInterval;
        tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeMinWindowSize
        = globalParamsStcPtr->signalDetectDbCfg.sdChangeMinWindowSize;
        tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdCheckTimeExpired
        = globalParamsStcPtr->signalDetectDbCfg.sdCheckTimeExpired;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if (CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_HIGH_SPEED_PORTS_E, globalParamsStcPtr->globalParamsBitmapType))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortProprietaryHighSpeedPortsSetFunc(devNum, globalParamsStcPtr->propHighSpeedPortEnabled);
            if (rc != GT_OK)
            {
                if (rc == GT_NOT_APPLICABLE_DEVICE)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "High speed ports configuration is not valid to this device");
                }
                else
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Setting CPU ports resources for High Speed ports failed");
                }
            }

        }
    }


    if (CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_RX_TERM_E, globalParamsStcPtr->globalParamsBitmapType))
    {
        tmpPortManagerDbPtr->globalParamsCfg.rxTermination = globalParamsStcPtr->rxTermination;
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
        {
            if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                /* update AP table in CM3 */
                mvHwsApSerdesGlobalRxTerminationModeIpc(devNum, 0, globalParamsStcPtr->rxTermination);
            }
            /* update table in host CPU */
            mvHwsAvagoSerdesDefaultRxTerminationSet(devNum, 0, (GT_U8)globalParamsStcPtr->rxTermination);

        }
    } else
        tmpPortManagerDbPtr->globalParamsCfg.rxTermination = NA_16BIT;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_40G_AN_RES_BIT_E, globalParamsStcPtr->globalParamsBitmapType))
        {
            if(globalParamsStcPtr->anResolutionBit_40G == CPSS_PORT_MANAGER_AN_RESOLUTION_BIT_34_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            tmpPortManagerDbPtr->globalParamsCfg.anResolutionBit_40G = globalParamsStcPtr->anResolutionBit_40G;
            if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                /* update AP 40G_KR2 AN resolution bit in CM3 */
                mvHwsApResolutionBitSet(devNum, _40GBase_KR2,(GT_U32)globalParamsStcPtr->anResolutionBit_40G);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPortManagerPortParamsUpdate
*      function
* @endinternal
*
* @brief  update port parameters also when port is active.
*
* @param[in] devNum        - device number
* @param[in] updateParamsStcPtr  - stc with the parameters to
*       override
*
* @retval GT_OK          - on success
* @retval GT_BAD_PARAM       - on wrong port number or device
* @retval GT_HW_ERROR       - on hardware error
* @retval GT_BAD_PTR        - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerPortParamsUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *updateParamsStcPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC* tmpPortManagerDbPtr;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);
    if (updateParamsStcPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: PORT_PARAMETER_UPDATE bwMode %d",updateParamsStcPtr->bwMode);

    if ((tmpPortManagerDbPtr->portMngSmDb == NULL) ||
        (tmpPortManagerDbPtr->portMngSmDb[portNum] == NULL) ||
        (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_RESET_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] port not created yet.",devNum, portNum);
    }

    if (CPSS_PORT_MANAGER_UPDATE_PARAMETER_TYPE_CHECK_MAC(CPSS_PORT_MANAGER_UPDATE_PARAMS_BW_TYPE_E, updateParamsStcPtr->updateParamsBitmapType))
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[Port %d/%2d] UPDATE_PARAMS_BW not supported in sip 6 ",
                                                       devNum, portNum);
        }
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc(devNum, portNum, updateParamsStcPtr->bwMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"[Port %d/%2d] Calling ppPortBwTxFifoSetFunc returned error code %d ",
                                                       devNum, portNum, rc);
        }

        tmpPortManagerDbPtr->portMngSmDb[portNum]->bwMode = updateParamsStcPtr->bwMode;
    }
    return GT_OK;

}
/**
* @internal internal_cpssPortManagerInit function
* @endinternal
*
* @brief   Init the portManager
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerInit
(
    IN  GT_U8                                   devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    PRV_CPSS_PORT_PM_FUNC_PTRS cpssPmFunc;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PP_MAC(devNum)->lockLowLevelApi = GT_FALSE;

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Port manager not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssPmFunc.cpssPmIsFwFunc = prvCpssPortManagerApIsConfigured;

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMgrInitFunc(devNum, &cpssPmFunc);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Failed to Init Port Manager");
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {   /* Falcon Only */
        if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            /* update AP table in CM3 */
            mvHwsApSetGlobalHostModeIpc(devNum, 0, MV_HWS_HOST_CONTROL_CNS);
        }

    }

    PRV_CPSS_PP_MAC(devNum)->lockLowLevelApi = GT_TRUE;
    PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(isPortMgrEnable)[devNum] = GT_TRUE;
    return GT_OK;

}

/**
* @internal internal_cpssPortManagerInitParamsStruct
* @endinternal
*
* @brief  function for setting initialize values to the port manager struct
*
* @param[in] devNum                - device number
*
* @param[out] portParams           - pm struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerInitParamsStruct
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    OUT  CPSS_PM_PORT_PARAMS_STC *portParams
)
{
    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(portParams);

    /* Init for the time being */
    cpssOsMemSet(portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));
    portParams->magic    = CPSS_PM_MAGIC;
    portParams->portType = portType;

    switch (portType)
    {
    case  CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E:
        /* Mandatory params init*/
        portParams->portParamsType.regPort.ifMode                  = CPSS_PORT_INTERFACE_MODE_NA_E;
        portParams->portParamsType.regPort.speed                   = CPSS_PORT_SPEED_NA_E;
        portParams->portParamsType.regPort.portAttributes.fecMode  = CPSS_PORT_FEC_MODE_DISABLED_E;
        break;
    case  CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E:
        /* Mandatory params init*/
        portParams->portParamsType.apPort.modesArr[0].ifMode       = CPSS_PORT_INTERFACE_MODE_NA_E;
        portParams->portParamsType.apPort.modesArr[0].speed        = CPSS_PORT_SPEED_NA_E;
        portParams->portParamsType.apPort.modesArr[0].fecRequested = CPSS_PORT_FEC_MODE_DISABLED_E;
        portParams->portParamsType.apPort.modesArr[0].fecSupported = CPSS_PORT_FEC_MODE_DISABLED_E;
        break;
    default:
        CPSS_LOG_INFORMATION_MAC("Port type: %d", portType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPmRegPortParamsSet
* @endinternal
*
* @brief  function for setting the params into the port database
*         (this function is for non ap ports)
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port
* @param[in] portParams            - port parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPmRegPortParamsSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PM_PORT_PARAMS_STC  *portParams
)
{
    GT_STATUS                             rc          = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB        = NULL;
    GT_U32                                portMacNum  = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;

    GT_U32                                lanesArrIdx = 0;
    CPSS_PM_PORT_ATTR_STC                *portAttr;
    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC     *portSmDbPtr;
    GT_U32                                bm;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
    *confiData   = NULL;

    CPSS_PM_MODE_LANE_PARAM_STC          *laneParams  = NULL;
    GT_U32                                targetLane  = 0xffffffff;
    CPSS_PORT_SERDES_TUNE_STC             serdesParams;
    GT_U32                                dbValidBitMap = 0;
    GT_U32                                dbSdValidBitMap = 0, step = 1, portsInBundle = 0;
#ifndef ASIC_SIMULATION
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackCfg;
    GT_BOOL                      anyLoopback;
#endif
    if (   !portParams
           || portParams->magic != CPSS_PM_MAGIC
           || portParams->portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL/uninitialised or portType not valid.");
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, pmDB, rc);

    /* check state machine legality */
    if (   (pmDB != NULL)
           && (pmDB->portMngSmDb != NULL)
           && (pmDB->portMngSmDb[portNum] != NULL)
           && (pmDB->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] port not in reset state.", devNum, portNum);
    }

    /* verify allocation */
    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation SM_DB failed=%d", devNum, portNum, rc);
    }

    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation PHYSICAL failed=%d", devNum, portNum, rc);
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), portParams->portParamsType.regPort.ifMode,
                                               portParams->portParamsType.regPort.speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                   devNum, portNum, rc, portParams->portParamsType.regPort.ifMode, portParams->portParamsType.regPort.speed);
    }

    pmDB->portsAttributedDb.generalPortCfgPtr[portNum]->portMode = portMode;
    pmDB->portsAttributedDb.generalPortCfgPtr[portNum]->portGroup =
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);
    pmDB->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum  = portMacNum;

    if(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemotePortCheckFunc)
    {
        pmDB->portMngSmDb[portNum]->isRemotePort = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemotePortCheckFunc(CAST_SW_DEVNUM(devNum),portNum);
    }

    if ( mvHwsUsxModeCheck(devNum,portMacNum,portMode))
    {
        rc = mvHwsUsxExtSerdesGroupParamsGet(devNum,portMacNum,portMode, &portMacNum, &step, &portsInBundle);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "serdes group failed=%d",rc);
        }
    }
    else if ((CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(portParams->portParamsType.regPort.ifMode)) ||
        (CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E == portParams->portParamsType.regPort.ifMode))
    {
        /* provide to HWS first port in quadruplet and it will configure all other ports if needed */
        portMacNum &= 0xFFFFFFFC;
        portsInBundle = 4;
    }

    if(!pmDB->portMngSmDb[portNum]->isRemotePort)
    {
        /* port mode sanity check */
        rc = mvHwsPortValidate(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "MAC_PORT[%d]: port mode not supported=%d",portMacNum, rc);
        }
    }
    /* TODO: Q: Shouldn't we clean-up these DB entries, if the API fails? */

    /**
     ** Basic mandatory params
     **/

    pmDB->portMngSmDb[portNum]->portType                            = portParams->portType;
    pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb      = portParams->portParamsType.regPort.ifMode;
    pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb       = portParams->portParamsType.regPort.speed;

    /* now that hws ifMode exist, we can verify alocation of serdes, because serdes lane numbers exist in hws */
    /* verify serdes allocation */

    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_SERDES_E);
    if (rc!=GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation SERDES failed=%d",devNum, portNum, rc);
    }

    if(!pmDB->portMngSmDb[portNum]->isRemotePort)
    {
        /* verify allocation */
        rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum),portNum,PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_MAC_E);
        if (rc!=GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation MAC failed=%d",devNum, portNum, rc);
        }
    }

    if (portsInBundle>1)
    {
        /* when working with QSGMII, better to initialize the database to all the quad ports */
        GT_U32 i;
        GT_U32 tmpPort;

        for ( i = portMacNum; portsInBundle > 0; i += step, portsInBundle-- )
        {
            if ( PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppConvertMacToPortFunc != NULL)
            {
                rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppConvertMacToPortFunc(devNum, i, &tmpPort);
                if (rc!=GT_OK)
                {
                  CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(MAC_port %d)ppConvertMacToPortFunc failed=%d", i, rc);
                }
            }
            else
            {
                tmpPort = i;
            }

            if (pmDB->portMngSmDb == NULL || pmDB->portMngSmDb[tmpPort] == NULL)
            {
                rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), tmpPort, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation SM_DB failed=%d", devNum, tmpPort, rc);
                }

                rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), tmpPort, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PHYSICAL_PORT_E);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation PHYSICAL failed=%d", devNum, tmpPort, rc);
                }
            }
        }
    }

    /**
     ** Lane Params
     **/

    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                   "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
    }

    switch (curPortParams.portFecMode)
    {
        case FC_FEC:
            pmDB->portMngSmDb[portNum]->fecAbility = CPSS_PORT_FEC_MODE_ENABLED_E;
            break;
        case RS_FEC:
            pmDB->portMngSmDb[portNum]->fecAbility = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            break;
        case RS_FEC_544_514:
            pmDB->portMngSmDb[portNum]->fecAbility = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            break;
        case FEC_OFF:
            pmDB->portMngSmDb[portNum]->fecAbility = CPSS_PORT_FEC_MODE_DISABLED_E;
            break;
        default:
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(
                GT_FAIL,
                "[MacPort %d] wrong portFecMode [%d] ", portMacNum, curPortParams.portFecMode);
    }

    for (lanesArrIdx = 0; lanesArrIdx < curPortParams.numOfActLanes; lanesArrIdx++)
    {
        laneParams      = &(portParams->portParamsType.regPort.laneParams[lanesArrIdx]);
        targetLane      = 0xffffffff;
        bm              = laneParams->validLaneParamsBitMask;

        if (bm)
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "(lane %d): SET PARAMS valid_bm=0x%x", lanesArrIdx, bm);

            if (CPSS_PM_LANE_PARAM_IS_VALID_BITMASK (bm, CPSS_PM_LANE_PARAM_GLOBAL_LANE_NUM_E) )
            {
                targetLane = laneParams->globalLaneNum;
            }
            else
            {
                targetLane = curPortParams.activeLanesList[lanesArrIdx];
            }

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"(lane %d): SET PARAMS targetLane=0x%x", lanesArrIdx, targetLane);

            rc = prvCpssPortManagerLaneTuneDbGet(CAST_SW_DEVNUM(devNum), portNum, 0, targetLane,
                                                 CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &serdesParams, &dbSdValidBitMap);
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPortManagerLaneTuneDbGet failed=%d", rc);
            }

            if ( bm & CPSS_PM_LANE_PARAM_RX_E )
            {
                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    if (laneParams->rxParams.type != CPSS_PORT_SERDES_AVAGO_E)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong SerDes Type");
                    }

                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "(lane %d) : setting port serdes RX params BW %d, DC %d,"
                                                               "HF %d, LF %d, sqlch %d",
                                                               lanesArrIdx, laneParams->rxParams.rxTune.avago.BW, laneParams->rxParams.rxTune.avago.DC, laneParams->rxParams.rxTune.avago.HF,
                                                               laneParams->rxParams.rxTune.avago.LF, laneParams->rxParams.rxTune.avago.sqlch);
                    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "BFLF %d, BFHF %d",laneParams->rxParams.rxTune.avago.BFLF,laneParams->rxParams.rxTune.avago.BFHF);
                    }

                    serdesParams.type = CPSS_PORT_SERDES_AVAGO_E;
                    cpssOsMemCpy(&(serdesParams.rxTune.avago), &(laneParams->rxParams.rxTune.avago),
                                 sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
                }
                else
                {
                    if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_H_E)
                    {
                        cpssOsMemCpy(&(serdesParams.rxTune.comphy),
                            &(laneParams->rxParams.rxTune.comphy),
                            sizeof(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC));
                    }
                    else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.rxTune.comphy_C12G),
                            &(laneParams->rxParams.rxTune.comphy_C12G),
                            sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));
                    }
                    else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C28G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.rxTune.comphy_C28G),
                            &(laneParams->rxParams.rxTune.comphy_C28G),
                            sizeof(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC));
                    }
                    else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C112G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.rxTune.comphy_C112G),
                            &(laneParams->rxParams.rxTune.comphy_C112G),
                            sizeof(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC));
                    }
                    else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C56G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.rxTune.comphy_C56G),
                            &(laneParams->rxParams.rxTune.comphy_C56G),
                            sizeof(CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC));
                    }
                    else
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong SerDes Type");
                    }

                    serdesParams.type = laneParams->rxParams.type;
                }
                dbValidBitMap |= CPSS_PM_DB_LANE_OVERRIDE_RX_E;
            }

            if ( bm & CPSS_PM_LANE_PARAM_TX_E )
            {
                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    if (laneParams->txParams.type != CPSS_PORT_SERDES_AVAGO_E)
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong SerDes Type");
                    }

                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "(lane %d) : setting port serdes TX parameters Atten %d, "
                                                               "post %d, pre %d, pre2 %d, pre3 %d ",
                                                               lanesArrIdx, laneParams->txParams.txTune.avago.atten, laneParams->txParams.txTune.avago.post, laneParams->txParams.txTune.avago.pre,
                                                               laneParams->txParams.txTune.avago.pre2, laneParams->txParams.txTune.avago.pre3);
                    serdesParams.type = CPSS_PORT_SERDES_AVAGO_E;
                    cpssOsMemCpy(&(serdesParams.txTune.avago),
                                 &(laneParams->txParams.txTune.avago),
                                 sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
                }
                else
                {
                    if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_H_E)
                    {
                      CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "(lane %d) : setting port serdes TX parameters txAmp %d, "
                                                                 "txAmpAdjEn %d, emph0 %d, emph1 %d, txAmpShft %d ",
                                                                 lanesArrIdx, laneParams->txParams.txTune.comphy.txAmp, laneParams->txParams.txTune.comphy.txAmpAdjEn, laneParams->txParams.txTune.comphy.emph0,
                                                                 laneParams->txParams.txTune.comphy.emph1, laneParams->txParams.txTune.comphy.txAmpShft);

                        cpssOsMemCpy(&(serdesParams.txTune.comphy),
                            &(laneParams->txParams.txTune.comphy),
                            sizeof(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC));
                    }
                    else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.txTune.comphy_C12G),
                            &(laneParams->txParams.txTune.comphy_C12G),
                            sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
                    }
                    else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C28G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.txTune.comphy_C28G),
                            &(laneParams->txParams.txTune.comphy_C28G),
                            sizeof(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC));
                    }
                    else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C112G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.txTune.comphy_C112G),
                            &(laneParams->txParams.txTune.comphy_C112G),
                            sizeof(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC));
                    }
                    else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C56G_E)
                    {
                        cpssOsMemCpy(&(serdesParams.txTune.comphy_C56G),
                            &(laneParams->txParams.txTune.comphy_C56G),
                            sizeof(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC));
                    }
                    else
                    {
                        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong SerDes Type");
                    }

                     serdesParams.type = laneParams->txParams.type;
               }
                dbValidBitMap |= CPSS_PM_DB_LANE_OVERRIDE_TX_E;
            }

            if (dbValidBitMap)
            {
                rc = prvCpssPortManagerLaneTuneDbSet(devNum, portNum, 0, targetLane,
                                                     CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, dbValidBitMap, &serdesParams);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPortManagerLaneTuneDbSet failed=%d", rc);
                }
            }
        }
    }

    /**
     ** Port Attributes
     **/

    portAttr    = &(portParams->portParamsType.regPort.portAttributes);
    bm          = portAttr->validAttrsBitMask;
    portSmDbPtr = pmDB->portMngSmDb[portNum];

    if (bm)
    {
        if ( bm & CPSS_PM_ATTR_TRAIN_MODE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "trainMode = %d", portAttr->trainMode);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E);
            portSmDbPtr->userOverrideTrainMode = portAttr->trainMode;
        }

        if ( bm & CPSS_PM_ATTR_RX_TRAIN_SUPP_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "adaptRxTrainSuppMode = %d", portAttr->adaptRxTrainSupp);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E);
            portSmDbPtr->userOverrideAdaptiveRxTrainSupported = portAttr->adaptRxTrainSupp;
        }

        if ( bm & CPSS_PM_ATTR_EDGE_DETECT_SUPP_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "edgeDetectSuppMode = %d", portAttr->edgeDetectSupported);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E);
            portSmDbPtr->userOverrideEdgeDetectSupported = portAttr->edgeDetectSupported;
        }

        if ( bm & CPSS_PM_ATTR_LOOPBACK_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Port Attr Loopback type = %d", portAttr->loopback.loopbackType);

            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_LOOPBACK_E);
            rc = prvCpssPortManagerLoopbackModesDbSet(devNum, portNum, portParams->portType,
                                                      &(portAttr->loopback));
            if (rc != GT_OK)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "(portNum %d)prvCpssPortManagerLoopbackModesDbSet failed=%d", portNum, rc);
            }
        }

        if ( bm & CPSS_PM_ATTR_LKB_E )
        {
            /* get PmDb for pairPort */
            PRV_CPSS_PORT_MNG_PORT_SM_DB_STC *pairSmDbPtr = pmDB->portMngSmDb[portAttr->linkBinding.pairPortNum];

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "lkb enabled = %d, pair %d",portAttr->linkBinding.enabled, portAttr->linkBinding.pairPortNum);

            if (portNum == portAttr->linkBinding.pairPortNum)
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_SET_ERROR, "error %d: Link binding port & pair are the same (%d)", GT_SET_ERROR, portNum);
            }

            if ((pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb < CPSS_PORT_SPEED_20000_E) &&
                (pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb != CPSS_PORT_SPEED_10000_E))
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_SET_ERROR, "error %d: Link binding on port (%d) not supported for this speed (%d)",
                                                           GT_SET_ERROR, portNum, pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb);
            }

            /* if the PmDb for pairPort==NULL - i.e. the pair port still not set by PM, so dont need to check the pairPort */
            if (pairSmDbPtr != NULL)
            {
                /* check if the pair of this pairPort not equal to this port */
                if ( (portAttr->linkBinding.enabled == GT_TRUE) &&
                     (pairSmDbPtr->linkBinding.enabled == GT_TRUE) &&
                     (portNum != pairSmDbPtr->linkBinding.pairPortNum) )
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_SET_ERROR, "error %d: Link binding pair port %d points at other port (%d)",
                                                               GT_SET_ERROR, portAttr->linkBinding.pairPortNum, pairSmDbPtr->linkBinding.pairPortNum);
                }
            }

            /* copy the configuration to the PmDb */
            portSmDbPtr->linkBinding.enabled = portAttr->linkBinding.enabled;
            portSmDbPtr->linkBinding.pairPortNum = portAttr->linkBinding.pairPortNum;
        }

        if ( bm & CPSS_PM_ATTR_ET_OVERRIDE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "minLF = %d", portAttr->etOverride.minLF);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "maxLF = %d", portAttr->etOverride.maxLF);

            portSmDbPtr->overrideEtParams = GT_TRUE;
            portSmDbPtr->min_LF           = portAttr->etOverride.minLF;
            portSmDbPtr->max_LF           = portAttr->etOverride.maxLF;
        }

        if ( bm & CPSS_PM_ATTR_FEC_MODE_E )
        {
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "FEC mode = %d", portAttr->fecMode);
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_FEC_MODE_E);
            portSmDbPtr->fecAbility = portAttr->fecMode;
        }

        if ( bm & CPSS_PM_ATTR_CALIBRATION_E )
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_CALIBRATION_E);
            confiData = &(portAttr->calibrationMode.confidenceCfg);
            portSmDbPtr->calibrationMode.calibrationType = portAttr->calibrationMode.calibrationType;

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "calibration type = %d", portAttr->calibrationMode.calibrationType);

            if (portSmDbPtr->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
            {
                CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "CI - lfMin = %d, lfMax=%d, hfMin=%d, hfMax=%d, eoMin=%d, eoMax=%d bitmap=0x%08x",
                                                           confiData->lfMinThreshold, confiData->lfMaxThreshold, confiData->hfMinThreshold,
                                                           confiData->hfMaxThreshold, confiData->eoMinThreshold, confiData->eoMaxThreshold,
                                                           confiData->confidenceEnableBitMap);

                portSmDbPtr->calibrationMode.minLfThreshold   = confiData->lfMinThreshold;
                portSmDbPtr->calibrationMode.maxLfThreshold   = confiData->lfMaxThreshold;
                portSmDbPtr->calibrationMode.minHfThreshold   = confiData->hfMinThreshold;
                portSmDbPtr->calibrationMode.maxHfThreshold   = confiData->hfMaxThreshold;
                portSmDbPtr->calibrationMode.minEoThreshold   = confiData->eoMinThreshold;
                portSmDbPtr->calibrationMode.maxEoThreshold   = confiData->eoMaxThreshold;
                portSmDbPtr->calibrationMode.confidenceBitMap = confiData->confidenceEnableBitMap;
            }
        }

        if ( bm & CPSS_PM_ATTR_UNMASK_EV_MODE_E )
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_UNMASK_EV_MODE_E);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"unMask Events Mode  %d", portAttr->unMaskEventsMode);
            portSmDbPtr->unMaskEventsMode = portAttr->unMaskEventsMode;
        }

        if (bm & CPSS_PM_ATTR_BW_MODE_E)
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_BW_MODE_E);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"BW_MODE Mode  %d", portAttr->bwMode);
            portSmDbPtr->bwMode = portAttr->bwMode;
        }

        if (bm & CPSS_PM_ATTR_PORT_OPERATIONS_E)
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_PORT_OPERATIONS_E);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port extra operations  %d", portAttr->bwMode);
            portSmDbPtr->portOperationsBitmap = portAttr->portExtraOperation;
        }
        else
        {
            portSmDbPtr->portOperationsBitmap = 0;
        }

        if (bm & CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E)
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port inter connect %d", portAttr->interconnectProfile);
            portSmDbPtr->interconnectProfile = portAttr->interconnectProfile;
        }
        else
        {
            portSmDbPtr->interconnectProfile = 0;
        }

        if (bm & CPSS_PM_PORT_ATTR_PREEMPTION_E)
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_PREEMPTION_E);
            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port preemption %d", portAttr->preemptionParams.type);
            portSmDbPtr->preemptionParams.type = portAttr->preemptionParams.type;
            portSmDbPtr->preemptionParams.minFragSize = portAttr->preemptionParams.minFragSize;
            portSmDbPtr->preemptionParams.preemptionMethod= portAttr->preemptionParams.preemptionMethod;
        }
        else
        {
            portSmDbPtr->preemptionParams.type = CPSS_PM_MAC_PREEMPTION_DISABLED_E;
        }

        if (bm & CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE)
        {
            portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE);

            portSmDbPtr->autoNegotiation.inbandEnable                 = portAttr->autoNegotiation.inbandEnable;
            portSmDbPtr->autoNegotiation.duplexEnable                 = portAttr->autoNegotiation.duplexEnable;
            portSmDbPtr->autoNegotiation.speedEnable                  = portAttr->autoNegotiation.speedEnable;
            portSmDbPtr->autoNegotiation.flowCtrlEnable               = portAttr->autoNegotiation.flowCtrlEnable;
            portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable = portAttr->autoNegotiation.flowCtrlPauseAdvertiseEnable;
            portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable   = portAttr->autoNegotiation.flowCtrlAsmAdvertiseEnable;
            portSmDbPtr->autoNegotiation.byPassEnable                 = portAttr->autoNegotiation.byPassEnable;

            CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"port autoNegotiation: inband enable %d duplex enable: %d speed enable: %d flowCtrlEnable enable %d flowCtrlPauseAdvertiseEnable enable %d flowCtrlAsmAdvertiseEnable enable %d bypass enable %d",
                                           portSmDbPtr->autoNegotiation.inbandEnable,
                                           portSmDbPtr->autoNegotiation.duplexEnable,
                                           portSmDbPtr->autoNegotiation.speedEnable,
                                           portSmDbPtr->autoNegotiation.flowCtrlEnable,
                                           portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable,
                                           portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable,
                                           portSmDbPtr->autoNegotiation.byPassEnable);
        }
        else
        {
            if ( pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb == CPSS_PORT_INTERFACE_MODE_SGMII_E )
            {
                portSmDbPtr->autoNegotiation.inbandEnable                 = GT_TRUE;
                portSmDbPtr->autoNegotiation.duplexEnable                 = GT_TRUE;
                portSmDbPtr->autoNegotiation.speedEnable                  = GT_TRUE;
                portSmDbPtr->autoNegotiation.byPassEnable                 = GT_FALSE;
                portSmDbPtr->autoNegotiation.flowCtrlEnable               = GT_FALSE;

                if ( PRV_CPSS_SIP_6_CHECK_MAC(devNum) )
                {
                    portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable   = GT_TRUE;
                    portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable = GT_TRUE;
                }
                else
                {
                    portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable   = GT_FALSE;
                    portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable = GT_FALSE;
                }
            }
            else if ( pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E ||
                     pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb == CPSS_PORT_INTERFACE_MODE_2500BASE_X_E)
            {
                if ( PRV_CPSS_SIP_6_CHECK_MAC(devNum) )
                {
                    portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable   = GT_TRUE;
                    portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable = GT_TRUE;
                }
                else
                {
                    portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable   = GT_FALSE;
                    portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable = GT_TRUE;
                }

                portSmDbPtr->autoNegotiation.inbandEnable                 = GT_FALSE;
                portSmDbPtr->autoNegotiation.byPassEnable                 = GT_TRUE;
                portSmDbPtr->autoNegotiation.duplexEnable                 = GT_FALSE;
                portSmDbPtr->autoNegotiation.speedEnable                  = GT_FALSE;
                portSmDbPtr->autoNegotiation.flowCtrlEnable               = GT_FALSE;
            }
            else
            {
                portSmDbPtr->autoNegotiation.inbandEnable                 = GT_FALSE;
                portSmDbPtr->autoNegotiation.duplexEnable                 = GT_TRUE;
                portSmDbPtr->autoNegotiation.speedEnable                  = GT_TRUE;
                portSmDbPtr->autoNegotiation.byPassEnable                 = GT_TRUE;
                portSmDbPtr->autoNegotiation.flowCtrlEnable               = GT_FALSE;
                portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable   = GT_FALSE;
                portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable = GT_FALSE;
            }
        }

        portSmDbPtr->pmOverFw = GT_FALSE;
#ifndef ASIC_SIMULATION
        if (bm & CPSS_PM_PORT_ATTR_PM_OVER_FW_E)
        {
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, pmDB->portMngSmDb[portNum]->portType, &loopbackCfg, &anyLoopback);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                               devNum, portNum, rc);
                }
                if (anyLoopback == GT_FALSE)
                {
                    portSmDbPtr->pmOverFw = portAttr->pmOverFw;
                    portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_PM_OVER_FW_ENABLE_E);
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "Set port PM over FW/host:%d", portAttr->pmOverFw);
                }
                else
                {
                    portSmDbPtr->pmOverFw = GT_FALSE;
                    portSmDbPtr->userOverrideByteBitmap &= (~(PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_PM_OVER_FW_ENABLE_E)));
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port PM over host (loopback) %d", portSmDbPtr->pmOverFw);
                }
            }
            else
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "error: PM_OVER_FW not supported");
            }

        }
        else
        {
            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&
                (!pmDB->portMngSmDb[portNum]->isRemotePort) && (!PRV_CPSS_PORT_MANAGER_SIMPLE_MODES_CHECK_MAC(portMode)) && (portMode != _5GBaseR))
            {
                rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, pmDB->portMngSmDb[portNum]->portType, &loopbackCfg, &anyLoopback);
                if (rc != GT_OK)
                {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerLoopbackModesDbGet failed=%d",
                                                               devNum, portNum, rc);
                }
                if (anyLoopback == GT_FALSE)
                {
                    portSmDbPtr->pmOverFw = GT_TRUE;
                    portSmDbPtr->userOverrideByteBitmap |= PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_PM_OVER_FW_ENABLE_E);
                    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "port PM over FW %d", portSmDbPtr->pmOverFw);
                }

            }
        }
#endif
    }

    return rc;
}

/**
* @internal prvCpssPmRegPortParamsGet
* @endinternal
*
* @brief  function for getting the params from the port database
*         (this function is for non ap ports)
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port
*
* @param[out] portParams            - port parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPmRegPortParamsGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT  CPSS_PM_PORT_PARAMS_STC  *portParams
)
{
  /**
   ** Variables
   **/
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB            = NULL;
    GT_BOOL                               anyLb;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
    *confiData       = NULL;
    GT_STATUS                             rc              = GT_OK;
    GT_U32                                portMacNum      = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;

    GT_U32                                targetLane;
    GT_U32                                lanesArrIdx     = 0;
    CPSS_PM_PORT_ATTR_STC                *portAttr;
    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC     *portSmDbPtr;

    CPSS_PM_MODE_LANE_PARAM_STC          *laneParams      = NULL;
    CPSS_PORT_SERDES_TUNE_STC            *serdesCfgDbPtr  = NULL;
    /**
    ** Checks
    **/
    if (!portParams)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL.");
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, pmDB, rc);

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(pmDB, portNum, 1);

    if (pmDB->portMngSmDb[portNum]->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"[Port %d/%2d] port parameters not set on port", devNum, portNum);
    }

    cpssOsMemSet(portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));
    portParams->portType = pmDB->portMngSmDb[portNum]->portType;

    if (portParams->portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] PortType is not Regular. Is unexpextedly AP.",devNum, portNum);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb,
                                               pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb, &portMode);
    if (rc != GT_OK || portMode >= LAST_PORT_MODE)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d. portMode = %d",
                                                   devNum, portNum, portMode, rc);
    }

    /**
    ** Lane Params
    **/
    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                                                   "[MacPort %d] hwsPortModeParamsGetToBuffer returned null ", portMacNum);
    }

    if (PRV_PORT_MANAGER_CHECK_XLG_CG_MAC_PORT_MAC(curPortParams))
    {
        PRV_PORT_MANAGER_CHECK_ALLOCATION_MAC_PORT_MAC(pmDB, portMacNum, 1);
    }
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PHYSICAL_PORT_MAC(pmDB, portNum, 1);

    portParams->portParamsType.regPort.ifMode = pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portParams->portParamsType.regPort.speed  = pmDB->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;

    for (lanesArrIdx = 0; lanesArrIdx < curPortParams.numOfActLanes; lanesArrIdx++)
    {
        laneParams        = &(portParams->portParamsType.regPort.laneParams[lanesArrIdx]);
        if (CPSS_PM_LANE_PARAM_IS_VALID_BITMASK (laneParams->validLaneParamsBitMask, CPSS_PM_LANE_PARAM_GLOBAL_LANE_NUM_E) )
        {
            targetLane = laneParams->globalLaneNum;
        }
        else
        {
            targetLane = curPortParams.activeLanesList[lanesArrIdx];
        }

        serdesCfgDbPtr = &(pmDB->portsAttributedDb.perSerdesDbPtr[targetLane]->portSerdesParams.serdesCfgDb);

        if (pmDB->portsAttributedDb.perSerdesDbPtr[targetLane] != NULL)
        {
            laneParams->txParams.type = serdesCfgDbPtr->type;
            if (laneParams->txParams.type == CPSS_PORT_SERDES_AVAGO_E)
            {
                cpssOsMemCpy(&(laneParams->txParams.txTune.avago),
                             &(serdesCfgDbPtr->txTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
            }
            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_H_E)
            {
                cpssOsMemCpy(&(laneParams->txParams.txTune.comphy),
                             &(serdesCfgDbPtr->txTune.comphy),
                             sizeof(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC));
            }
            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
            {
                cpssOsMemCpy(&(laneParams->txParams.txTune.comphy_C12G),
                             &(serdesCfgDbPtr->txTune.comphy_C12G),
                             sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
            }
            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C28G_E)
            {
                cpssOsMemCpy(&(laneParams->txParams.txTune.comphy_C28G),
                             &(serdesCfgDbPtr->txTune.comphy_C28G),
                             sizeof(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC));
            }
            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C112G_E)
            {
                 cpssOsMemCpy(&(laneParams->txParams.txTune.comphy_C112G),
                             &(serdesCfgDbPtr->txTune.comphy_C112G),
                             sizeof(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC));
            }
            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C56G_E)
            {
                 cpssOsMemCpy(&(laneParams->txParams.txTune.comphy_C56G),
                             &(serdesCfgDbPtr->txTune.comphy_C56G),
                             sizeof(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC));
            }

            if (pmDB->portsAttributedDb.perSerdesDbPtr[targetLane]->portSerdesParams.txValid)
            {
                CPSS_PM_SET_VALID_LANE_PARAM(portParams, lanesArrIdx, CPSS_PM_LANE_PARAM_TX_E);
            }

            laneParams->rxParams.type = serdesCfgDbPtr->type;
            if (laneParams->rxParams.type == CPSS_PORT_SERDES_AVAGO_E)
            {
                cpssOsMemCpy(&(laneParams->rxParams.rxTune.avago),
                             &(serdesCfgDbPtr->rxTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
            }
            else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_H_E)
            {
                cpssOsMemCpy(&(laneParams->rxParams.rxTune.comphy),
                             &(serdesCfgDbPtr->rxTune.comphy),
                             sizeof(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC));
            }
            else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
            {
                cpssOsMemCpy(&(laneParams->rxParams.rxTune.comphy_C12G),
                             &(serdesCfgDbPtr->rxTune.comphy_C12G),
                             sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));
            }
            else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C28G_E)
            {
                cpssOsMemCpy(&(laneParams->rxParams.rxTune.comphy_C28G),
                             &(serdesCfgDbPtr->rxTune.comphy_C28G),
                             sizeof(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC));
            }
            else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C112G_E)
            {
                 cpssOsMemCpy(&(laneParams->rxParams.rxTune.comphy_C112G),
                             &(serdesCfgDbPtr->rxTune.comphy_C112G),
                             sizeof(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC));
            }

            if (pmDB->portsAttributedDb.perSerdesDbPtr[targetLane]->portSerdesParams.rxValid)
            {
                CPSS_PM_SET_VALID_LANE_PARAM(portParams, lanesArrIdx, CPSS_PM_LANE_PARAM_RX_E);
            }
        }
    }

    /**
     ** Port Attributes
     **/

    portAttr    = &(portParams->portParamsType.regPort.portAttributes);
    portSmDbPtr = pmDB->portMngSmDb[portNum];

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_TRAIN_MODE_E))
    {
        portAttr->trainMode = portSmDbPtr->userOverrideTrainMode;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_ADAPTIVE_RX_TRAIN_SUPPORTED_E))
    {
        portAttr->adaptRxTrainSupp = portSmDbPtr->userOverrideAdaptiveRxTrainSupported;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PORT_MANAGER_EDGE_DETECT_SUPPORTED_E))
    {
        portAttr->edgeDetectSupported = portSmDbPtr->userOverrideEdgeDetectSupported;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_EDGE_DETECT_SUPP_E);
    }


    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_LOOPBACK_E))
    {
        rc = prvCpssPortManagerLoopbackModesDbGet(devNum, portNum, portParams->portType,
                                                  &(portAttr->loopback), &anyLb);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Port %d/%2d] prvCpssPortManagerLoopbackModesDbSet failed=%d", devNum, portNum, rc);
        }
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_LOOPBACK_E);
    }

    if ( portSmDbPtr->linkBinding.enabled == GT_TRUE)
    {
        portAttr->linkBinding.enabled = portSmDbPtr->linkBinding.enabled;
        portAttr->linkBinding.pairPortNum = portSmDbPtr->linkBinding.pairPortNum;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_LKB_E);
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "lkb port enabled = %d, pair %d", portNum, portAttr->linkBinding.pairPortNum);
    }

    if ( portSmDbPtr->overrideEtParams == GT_TRUE)
    {
        portAttr->etOverride.minLF = portSmDbPtr->min_LF;
        portAttr->etOverride.maxLF = portSmDbPtr->max_LF;

        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_FEC_MODE_E))
    {
        portAttr->fecMode = portSmDbPtr->fecAbility;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_FEC_MODE_E );
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_CALIBRATION_E))
    {
        portAttr->calibrationMode.calibrationType = portSmDbPtr->calibrationMode.calibrationType;
        confiData = &(portAttr->calibrationMode.confidenceCfg);
        if (portSmDbPtr->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
        {
            confiData->lfMinThreshold           = portSmDbPtr->calibrationMode.minLfThreshold;
            confiData->lfMaxThreshold           = portSmDbPtr->calibrationMode.maxLfThreshold;
            confiData->hfMinThreshold           = portSmDbPtr->calibrationMode.minHfThreshold;
            confiData->hfMaxThreshold           = portSmDbPtr->calibrationMode.maxHfThreshold;
            confiData->eoMinThreshold           = portSmDbPtr->calibrationMode.minEoThreshold;
            confiData->eoMaxThreshold           = portSmDbPtr->calibrationMode.maxEoThreshold;
            confiData->confidenceEnableBitMap   = portSmDbPtr->calibrationMode.confidenceBitMap;
        }
        else
        {
            confiData->confidenceEnableBitMap = 0x00000000;
        }
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_CALIBRATION_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_UNMASK_EV_MODE_E))
    {
        portAttr->unMaskEventsMode = portSmDbPtr->unMaskEventsMode;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_BW_MODE_E))
    {
        portAttr->bwMode = portSmDbPtr->bwMode;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_BW_MODE_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E))
    {
        portAttr->interconnectProfile = portSmDbPtr->interconnectProfile;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_ATTR_PORT_OPERATIONS_E))
    {
        portAttr->portExtraOperation = portSmDbPtr->portOperationsBitmap;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_ATTR_PORT_OPERATIONS_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_PREEMPTION_E))
    {
       portAttr->preemptionParams.type = portSmDbPtr->preemptionParams.type;
       CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_PORT_ATTR_PREEMPTION_E);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE))
    {
       portAttr->autoNegotiation.inbandEnable  = portSmDbPtr->autoNegotiation.inbandEnable;
       portAttr->autoNegotiation.duplexEnable  = portSmDbPtr->autoNegotiation.duplexEnable;
       portAttr->autoNegotiation.speedEnable   = portSmDbPtr->autoNegotiation.speedEnable;
       portAttr->autoNegotiation.flowCtrlEnable   = portSmDbPtr->autoNegotiation.flowCtrlEnable;
       portAttr->autoNegotiation.flowCtrlPauseAdvertiseEnable   = portSmDbPtr->autoNegotiation.flowCtrlPauseAdvertiseEnable;
       portAttr->autoNegotiation.flowCtrlAsmAdvertiseEnable = portSmDbPtr->autoNegotiation.flowCtrlAsmAdvertiseEnable;
       portAttr->autoNegotiation.byPassEnable  = portSmDbPtr->autoNegotiation.byPassEnable;

       CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE);
    }

    if (portSmDbPtr->userOverrideByteBitmap & PRV_CPSS_PORT_MANAGER_BIT_MAC(PRV_CPSS_PM_PORT_ATTR_PM_OVER_FW_ENABLE_E))
    {
        portAttr->pmOverFw = portSmDbPtr->pmOverFw;
        CPSS_PM_SET_VALID_ATTR(portParams, CPSS_PM_PORT_ATTR_PM_OVER_FW_E);
    }
    if (rc == GT_OK)
    {
        portParams->magic = CPSS_PM_MAGIC;
    }

    return rc;
}

/**
* @internal prvCpssPmApPortParamsSet
* @endinternal
*
* @brief  function for setting the params into the port database
*         (this function is for ap ports)
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port
* @param[in] portParams            - port parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPmApPortParamsSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PM_PORT_PARAMS_STC  *portParams
)
{
    GT_STATUS                             rc          = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB        = NULL;
    GT_U32                                portMacNum  = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;

    GT_U32                                serdesSpeedArr[CPSS_PORT_AP_IF_ARRAY_SIZE_CNS] = {0};
    GT_U32                                i,bm,bmLanes;
    CPSS_PM_AP_PORT_ADV_STC              *mode;
    CPSS_PM_AP_PORT_ATTR_STC             *apAttrs;
    GT_U32                                minNumOfLanes = PORT_MANAGER_MAX_LANES;
    CPSS_PORT_SERDES_TUNE_STC             serdesParams;
    GT_U32                                lanesArrIdx = 0,speedLaneIdx =0, advertisedNum = 0;
    CPSS_PM_AP_MODE_LANE_PARAM_STC        *laneParams  = NULL;
    GT_U32                                dbValidBitMap = 0;

#if 0
    /* TODO: Q */
    GT_U32                              sdLanes;
#endif

    /**
     ** Checks
     **/

    if ((!portParams) ||
        (portParams->magic != CPSS_PM_MAGIC) ||
        (portParams->portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] Input Params is NULL/uninitialised or portType not valid.", devNum, portNum);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, pmDB, rc);

    if ((PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApConfigSetFunc == NULL) ||
        (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApStatusGetFunc == NULL) ||
        (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApSerdesTxParamsOffsetSetFunc == NULL))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] AP APIs not configured", devNum, portNum);
    }

    /* check state machine legality */
    if (   (pmDB != NULL)
           && (pmDB->portMngSmDb != NULL)
           && (pmDB->portMngSmDb[portNum] != NULL)
           && (pmDB->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E)
           && (pmDB->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] port not in reset state.", devNum, portNum);
    }

    /* verify allocation */
    rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_AP_PORT_E);
    if (rc!=GT_OK)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation AP failed=%d", devNum, portNum, rc);

    if (  (pmDB == NULL)
       || (pmDB->portMngSmDb == NULL)
       || (pmDB->portMngSmDb[portNum] == NULL)
       || (pmDB->portMngSmDb[portNum]->pmOverFw == GT_FALSE ))
    {
        /* verify allocation */
        rc = prvCpssPortManagerVerifyAllocation(CAST_SW_DEVNUM(devNum), portNum, PRV_CPSS_PORT_MANAGER_ALLOCATION_TYPE_PORT_SM_DB_E);
        if (rc!=GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssPortManagerVerifyAllocation SM_DB failed=%d", devNum, portNum, rc);
    }

    if (CPSS_PORT_AP_IF_ARRAY_SIZE_CNS < portParams->portParamsType.apPort.numOfModes)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"[Port %d/%2d] AP port too many parameters=%d",
                                                   devNum, portNum, portParams->portParamsType.apPort.numOfModes);
    }

    /* TODO: Q: Shouldn't we clean-up these DB entries, if the API fails? */
    if (   (pmDB == NULL)
           || (pmDB->portMngSmDb == NULL)
           || (pmDB->portMngSmDb[portNum] == NULL) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "[Port %d/%2d] port not allocated.", devNum, portNum);
    }

    /**
     ** Copying all advertised mode related info
     **/
    if ( pmDB->portMngSmDb[portNum]->pmOverFw == GT_FALSE )
    {
        pmDB->portMngSmDb[portNum]->portType = portParams->portType;
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);

    pmDB->portsApAttributesDb[portNum]->perPhyPortApNumOfModes = portParams->portParamsType.apPort.numOfModes;
    for (i = 0; i < portParams->portParamsType.apPort.numOfModes; i++)
    {
        mode = &(portParams->portParamsType.apPort.modesArr[i]);
        rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), mode->ifMode, mode->speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                       devNum, portNum, rc, mode->ifMode, mode->speed);
        }

        rc = mvHwsPortValidate (CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
        if (rc != GT_OK)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] port mode not supported=%d", devNum, portNum, rc);

        pmDB->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb           = mode->ifMode;
        pmDB->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb            = mode->speed;
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.fecRequestedArr[i] = mode->fecRequested;
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.fecAbilityArr[i]   = mode->fecSupported;

        /*
         * Lane Params
         */
        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "hwsPortModeParamsGet returned %d", rc );
        }

        if (minNumOfLanes > curPortParams.numOfActLanes)
            minNumOfLanes = curPortParams.numOfActLanes;

        serdesSpeedArr[i] = curPortParams.serdesSpeed;
    } /* for numOfModes */

    /**
     ** AP Port Attributes
     **/
    apAttrs = &(portParams->portParamsType.apPort.apAttrs);
    bm      = apAttrs->validAttrsBitMask;
    pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask = bm;

    if (bm & CPSS_PM_AP_PORT_ATTR_PARALLEL_DETECT_E)
    {
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartMaxNum  = apAttrs->parallelDetect.restartMaxNum;
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.enable  = apAttrs->parallelDetect.enable;
    }
    else
    {
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartMaxNum  = PRV_PORT_MANAGER_DEFAULT_PARALLEL_DETECT_RESTAR_NUM;
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.enable  = PRV_PORT_MANAGER_DEFAULT_PARALLEL_DETECT_EN;
    }

    if (bm & CPSS_PM_AP_PORT_ATTR_NONCE_E)
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable   = apAttrs->nonceDisable;
    else
    {
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            pmDB->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable   = PRV_PORT_MANAGER_DEFAULT_6_10_NONECE_DISABLE;
        }
        else
        {
            pmDB->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable   = PRV_PORT_MANAGER_DEFAULT_NONECE_DISABLE;
        }
    }

    if (bm & CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E)
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcPause         = apAttrs->fcPause;
    else
    {
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcPause     = PRV_PORT_MANAGER_DEFAULT_6_10_FC_PAUSE;
        }
        else
        {
            pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcPause     = PRV_PORT_MANAGER_DEFAULT_FC_PAUSE;
        }
    }


    if (bm & CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E)
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcAsmDir        = apAttrs->fcAsmDir;
    else
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcAsmDir        = PRV_PORT_MANAGER_DEFAULT_FC_ASM_DIR;

    if (bm & CPSS_PM_AP_PORT_ATTR_LANE_NUM_E)
    {
        if (apAttrs->negotiationLaneNum >= minNumOfLanes)
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] negotiationLaneNum %d is Invalid", devNum, portNum, apAttrs->negotiationLaneNum);

        pmDB->portsApAttributesDb[portNum]->apAttributesStc.laneNum         = apAttrs->negotiationLaneNum;
    }
    else
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.laneNum         = PRV_PORT_MANAGER_DEFAULT_LANE_NUM;

    if (bm & CPSS_PM_AP_PORT_ATTR_OPERATIONS_E)
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.portOperationsBitmap   = apAttrs->portExtraOperation;
    else
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.portOperationsBitmap   = 0;

    if (bm & CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E)
    {
        if ( apAttrs->interconnectProfile >=  HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES )
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] Interconnect Profile %d is Invalid", devNum, portNum, apAttrs->interconnectProfile );
        }
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.interconnectProfile    = apAttrs->interconnectProfile;
    }
    else
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.interconnectProfile    = 0;

    if (bm & CPSS_PM_AP_PORT_ATTR_SKIP_RES_E)
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.skipRes    = apAttrs->skipRes;
    else
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.skipRes    = PRV_PORT_MANAGER_DEFAULT_SKIP_RES;

    if (bm & CPSS_PM_AP_PORT_ATTR_LKB_E)
    {
        /* take the PmDb for pairPort */
        PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC *pairApDbPtr = pmDB->portsApAttributesDb[apAttrs->linkBinding.pairPortNum];

        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "lkb enabled = %d, pair %d", apAttrs->linkBinding.enabled, apAttrs->linkBinding.pairPortNum);

        if (portNum == apAttrs->linkBinding.pairPortNum)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_SET_ERROR, "error %d: Link binding port & pair are the same (%d)", GT_SET_ERROR, portNum);
        }

        /* if the PmDb for pairPort==NULL - i.e. the pair port still not set by PM, so dont need to check the pairPort */
        if (pairApDbPtr != NULL)
        {
            /* check if the pair of this pairPort not equal to this port */
            if ( (apAttrs->linkBinding.enabled == GT_TRUE) &&
                 (pairApDbPtr->apAttributesStc.linkBinding.enabled == GT_TRUE) &&
                 (portNum != pairApDbPtr->apAttributesStc.linkBinding.pairPortNum) )
            {
                CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_SET_ERROR, "error %d: Link binding pair port %d points at other port (%d)",
                                                           GT_SET_ERROR, apAttrs->linkBinding.pairPortNum, pairApDbPtr->apAttributesStc.linkBinding.pairPortNum);
            }
        }

        /* copy the configuration to the PmDb */
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.linkBinding.enabled = apAttrs->linkBinding.enabled;
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.linkBinding.pairPortNum = apAttrs->linkBinding.pairPortNum;
    }
    else
    {
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.linkBinding.enabled = GT_FALSE;
        pmDB->portsApAttributesDb[portNum]->apAttributesStc.linkBinding.pairPortNum = 0xFFFFFFFF;
    }

    if (bm)
    {
        CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum, "AP attrs set:  valid_bm = 0x%x, nonceDisa=%d, fcPause=%d, fcAsmDir=%d, negoLaneNum=%d",
                                                   bm, apAttrs->nonceDisable, apAttrs->fcPause,apAttrs->fcAsmDir, apAttrs->negotiationLaneNum);
    }

    bm      = apAttrs->validAttrsBitMask;

    if (bm & CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E)
    {
        for (advertisedNum = 0; advertisedNum < portParams->portParamsType.apPort.numOfModes; advertisedNum++)
        {
            if ((portParams->portParamsType.apPort.modesArr[advertisedNum].ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) ||
                (portParams->portParamsType.apPort.modesArr[advertisedNum].speed  != CPSS_PORT_SPEED_NA_E))
            {
                switch (serdesSpeedArr[advertisedNum])
                {
                    case _10_3125G:
                    case _10_3125G_SR_LR:
                    case _10_9375G:
                    case _12_1875G:
                        speedLaneIdx = CPSS_PM_AP_LANE_SERDES_SPEED_10000_E;
                        break;
                    case _25_78125G:
                    case _25_78125G_SR_LR:
                    case _27_34375G:
                        speedLaneIdx = CPSS_PM_AP_LANE_SERDES_SPEED_25000_E;
                        break;
                    case _26_5625G_PAM4:
                    case _26_5625G_PAM4_SR_LR:
                    case _27_1875_PAM4:
                    case _28_125G_PAM4:
                        speedLaneIdx = CPSS_PM_AP_LANE_SERDES_SPEED_PAM4_E;
                        break;
                    default:
                        continue;
                }

                for (lanesArrIdx = 0; lanesArrIdx < PORT_MANAGER_MAX_LANES; lanesArrIdx++)
                {
                    laneParams      = &(portParams->portParamsType.apPort.apAttrs.overrideLaneParams[speedLaneIdx][lanesArrIdx]);
                    bmLanes         = laneParams->validApLaneParamsBitMask;
                    dbValidBitMap   = 0;
                    cpssOsMemSet(&serdesParams, 0, sizeof(CPSS_PORT_SERDES_TUNE_STC));

                    if (bmLanes != 0)
                    {
                        /* RX */
                        if (bmLanes & CPSS_PM_AP_LANE_OVERRIDE_RX_E)
                        {
                            if (laneParams->rxParams.type == CPSS_PORT_SERDES_AVAGO_E)
                            {
                                serdesParams.type = laneParams->rxParams.type;
                                cpssOsMemCpy(&(serdesParams.rxTune.avago), &(laneParams->rxParams.rxTune.avago),
                                             sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
                            }
                            else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
                            {
                                serdesParams.type = laneParams->rxParams.type;
                                cpssOsMemCpy(&(serdesParams.rxTune.comphy_C12G),
                                             &(laneParams->rxParams.rxTune.comphy_C12G),
                                             sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));
                            }
                            else
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
                            }
                            dbValidBitMap |= CPSS_PM_DB_LANE_OVERRIDE_RX_E;
                        }

                        /* TX */
                        if (bmLanes & CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E)
                        {
                            if (laneParams->txParams.type == CPSS_PORT_SERDES_AVAGO_E)
                            {
                                serdesParams.type = laneParams->txParams.type;
                                cpssOsMemCpy(&(serdesParams.txTune.avago), &(laneParams->txParams.txTune.avago),
                                             sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
                            }
                            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_C12G_E)
                            {
                                serdesParams.type = laneParams->txParams.type;
                                cpssOsMemCpy(&(serdesParams.txTune.comphy_C12G),
                                             &(laneParams->txParams.txTune.comphy_C12G),
                                             sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
                            }
                            else
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
                            }
                            dbValidBitMap |= CPSS_PM_DB_LANE_OVERRIDE_TX_E;
                        }

                        rc = prvCpssPortManagerLaneTuneDbSet(devNum, portNum, advertisedNum, lanesArrIdx,
                                                             CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, dbValidBitMap, &serdesParams);
                        if (rc != GT_OK)
                        {
                            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPortManagerLaneTuneDbSet failed=%d", rc);
                        }
                    }
                }
            }
        }
    }

    return rc;
}

/**
* @internal prvCpssPmApPortParamsGet
* @endinternal
*
* @brief  function for getting the params from the port database
*         (this function is for ap ports)
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port
*
* @param[out] portParams           - port parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPmApPortParamsGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_PM_PORT_PARAMS_STC  *portParams
)
{
    /**
     ** Variables
     **/
    GT_STATUS                             rc          = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC             *pmDB        = NULL;
    GT_U32                                portMacNum  = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    GT_U32                                advertisedNum = 0;

    GT_U32                                i,speedLaneIdx = 0,lanesArrIdx = 0;
    CPSS_PM_AP_PORT_ADV_STC              *mode;
    CPSS_PM_AP_PORT_ATTR_STC             *apAttrs;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;
    GT_U32                                serdesSpeedArr[CPSS_PORT_AP_IF_ARRAY_SIZE_CNS] = {0};

    /**
     ** Checks
     **/
    if (!portParams)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL.");
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, pmDB, rc);

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);
    }

    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(pmDB, portNum, 1);
    PRV_PORT_MANAGER_CHECK_ALLOCATION_AP_PORT_MAC(pmDB, portNum, 1);

    if (pmDB->portMngSmDb[portNum]->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"[Port %d/%2d] port parameters not set on port", devNum, portNum);
    }

    portParams->portType = pmDB->portMngSmDb[portNum]->portType;

    /**
     ** Copying all advertised mode related info
     **/

    portParams->portParamsType.apPort.numOfModes = pmDB->portsApAttributesDb[portNum]->perPhyPortApNumOfModes;
    for (i = 0; i < portParams->portParamsType.apPort.numOfModes; i++)
    {
        mode = &(portParams->portParamsType.apPort.modesArr[i]);

        mode->ifMode        = pmDB->portsApAttributesDb[portNum]->perPhyPortDb[i].ifModeDb;
        mode->speed         = pmDB->portsApAttributesDb[portNum]->perPhyPortDb[i].speedDb;
        mode->fecRequested  = pmDB->portsApAttributesDb[portNum]->apAttributesStc.fecRequestedArr[i];
        mode->fecSupported  = pmDB->portsApAttributesDb[portNum]->apAttributesStc.fecAbilityArr[i];

        rc = prvCpssCommonPortIfModeToHwsTranslate (CAST_SW_DEVNUM(devNum), mode->ifMode, mode->speed, &portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                                                       devNum, portNum, rc, mode->ifMode, mode->speed);
        }

        rc = mvHwsPortValidate (CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] port mode not supported=%d", devNum, portNum, rc);
        }

        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "hwsPortModeParamsGet returned %d", rc );
        }

        serdesSpeedArr[i] = curPortParams.serdesSpeed;
    } /* for numOfModes */


    /**
     ** AP Port Attributes
     **/

    /*
       CPSS_PM_AP_PORT_ATTR_NONCE_E,
       CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E,
       CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E,
       CPSS_PM_AP_PORT_ATTR_LANE_NUM_E,
       CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E,
    */
    apAttrs                       = &(portParams->portParamsType.apPort.apAttrs);
    apAttrs->validAttrsBitMask    = pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask;

    apAttrs->parallelDetect.restartMaxNum = pmDB->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.restartMaxNum;
    apAttrs->parallelDetect.enable = pmDB->portsApAttributesDb[portNum]->apAttributesStc.parallelDetect.enable;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_PARALLEL_DETECT_E)
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_PARALLEL_DETECT_E);

    apAttrs->nonceDisable         = pmDB->portsApAttributesDb[portNum]->apAttributesStc.noneceDisable;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_NONCE_E)
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_NONCE_E);

    apAttrs->fcPause              = pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcPause;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E)
        CPSS_PM_SET_VALID_AP_ATTR (portParams,CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E);

    apAttrs->fcAsmDir             = pmDB->portsApAttributesDb[portNum]->apAttributesStc.fcAsmDir;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E)
        CPSS_PM_SET_VALID_AP_ATTR (portParams, CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E);

    apAttrs->negotiationLaneNum   = pmDB->portsApAttributesDb[portNum]->apAttributesStc.laneNum;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_LANE_NUM_E)
        CPSS_PM_SET_VALID_AP_ATTR (portParams, CPSS_PM_AP_PORT_ATTR_LANE_NUM_E);

    apAttrs->portExtraOperation   = pmDB->portsApAttributesDb[portNum]->apAttributesStc.portOperationsBitmap;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_OPERATIONS_E)
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_OPERATIONS_E);

    apAttrs->interconnectProfile  = pmDB->portsApAttributesDb[portNum]->apAttributesStc.interconnectProfile;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E)
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);

    apAttrs->skipRes  = pmDB->portsApAttributesDb[portNum]->apAttributesStc.skipRes;
    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_SKIP_RES_E)
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_SKIP_RES_E);
    /* Override Params */
    /*   CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E, */

    if (pmDB->portsApAttributesDb[portNum]->apAttributesStc.overrideAttrsBitMask & CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E)
    {
        CPSS_PM_SET_VALID_AP_ATTR(portParams, CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);
    }
    for (advertisedNum = 0; advertisedNum < portParams->portParamsType.apPort.numOfModes; advertisedNum++)
    {
       if ((portParams->portParamsType.apPort.modesArr[advertisedNum].ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) ||
            (portParams->portParamsType.apPort.modesArr[advertisedNum].speed  != CPSS_PORT_SPEED_NA_E))
       {
           switch (serdesSpeedArr[advertisedNum])
           {
               case _10_3125G:
               case _10_3125G_SR_LR:
               case _10_9375G:
               case _12_1875G:
                   speedLaneIdx = CPSS_PM_AP_LANE_SERDES_SPEED_10000_E;
                   break;
               case _25_78125G:
               case _25_78125G_SR_LR:
               case _27_34375G:
                   speedLaneIdx = CPSS_PM_AP_LANE_SERDES_SPEED_25000_E;
                   break;
               case _26_5625G_PAM4:
               case _26_5625G_PAM4_SR_LR:
               case _27_1875_PAM4:
               case _28_125G_PAM4:
                   speedLaneIdx = CPSS_PM_AP_LANE_SERDES_SPEED_PAM4_E;
                   break;
               default:
                   continue;
          }

          for (lanesArrIdx = 0; lanesArrIdx < PORT_MANAGER_MAX_LANES; lanesArrIdx++)
          {
               apAttrs->overrideLaneParams[speedLaneIdx][lanesArrIdx].txParams.type =
                        pmDB->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (lanesArrIdx)].portSerdesParams.serdesCfgDb.type;
               cpssOsMemCpy(&(apAttrs->overrideLaneParams[speedLaneIdx][lanesArrIdx].txParams.txTune),
                               &(pmDB->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (lanesArrIdx)].portSerdesParams.serdesCfgDb.txTune),
                               sizeof(CPSS_PORT_SERDES_TX_CONFIG_UNT));
               if (pmDB->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (lanesArrIdx)].portSerdesParams.txValid == GT_TRUE)
               {
                   apAttrs->overrideLaneParams[speedLaneIdx][lanesArrIdx].validApLaneParamsBitMask |= CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E;
               }

               apAttrs->overrideLaneParams[speedLaneIdx][lanesArrIdx].rxParams.type =
                        pmDB->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (lanesArrIdx)].portSerdesParams.serdesCfgDb.type;
               cpssOsMemCpy(&(apAttrs->overrideLaneParams[speedLaneIdx][lanesArrIdx].rxParams.rxTune),
                                &(pmDB->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (lanesArrIdx)].portSerdesParams.serdesCfgDb.rxTune),
                                sizeof(CPSS_PORT_SERDES_RX_CONFIG_UNT));
               if (pmDB->portsApAttributesDb[portNum]->perSerdesDb[(advertisedNum * PORT_MANAGER_MAX_LANES) + (lanesArrIdx)].portSerdesParams.rxValid == GT_TRUE)
               {
                   apAttrs->overrideLaneParams[speedLaneIdx][lanesArrIdx].validApLaneParamsBitMask |= CPSS_PM_AP_LANE_OVERRIDE_RX_E;
               }
            }
        }
    }

    return rc;
}

/**
* @internal internal_cpssPortManagerPortParamsSet
* @endinternal
*
* @brief  function for setting the params into the port database
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port
* @param[in] portParams            - port parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerPortParamsSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PM_PORT_PARAMS_STC  *portParams
)
{
    GT_STATUS rc;

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if (   !portParams
           || portParams->magic != CPSS_PM_MAGIC
           || portParams->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL/uninitialised or portType not valid.");
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "[Port %d/%2d] port manager not supported for %d device",
                                                   devNum, portNum, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    CPSS_PORT_MANAGER_LOG_PORT_INFORMATION_MAC(devNum, portNum,"Stage: PORT_PARAMETER_SET (portType=%d)*****", portParams->portType);

    if (portParams->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[Port %d/%2d] **invalid portType %d in parameters set",
                                                   devNum, portNum, portParams->portType);
    }

    if (portParams->portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        rc = prvCpssPmRegPortParamsSet(devNum, portNum, portParams);
    }
    else
    {
        rc = prvCpssPmApPortParamsSet(devNum, portNum, portParams);
    }

    return rc;
}

/**
* @internal internal_cpssPortManagerPortParamsGet
* @endinternal
*
* @brief  function for getting the params from the port database
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port
*
* @param[out] portParams           - port parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPortManagerPortParamsGet
(
    IN GT_SW_DEV_NUM             devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    OUT CPSS_PM_PORT_PARAMS_STC  *portParams
)
{
   /**
     ** Variables
     **/

    PRV_CPSS_PORT_MNG_DB_STC             *pmDB            = NULL;
    GT_STATUS                             rc              = GT_OK;
    GT_U32                                portMacNum      = 0;
    /**
     ** Checks
     **/
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    if (!portParams)
    {
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Input Params is NULL.");
    }

    if ( !(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)) )
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "[Port %d/%2d] port manager not supported for %d device",
                                                devNum, portNum, PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /* getting port manager database*/
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, pmDB, rc);

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);
    }

    /* verify allocation */
    PRV_PORT_MANAGER_CHECK_ALLOCATION_PORT_SM_DB_MAC(pmDB, portNum, 1);

    if (pmDB->portMngSmDb[portNum]->portType >= CPSS_PORT_MANAGER_PORT_TYPE_LAST_E)
    {
        rc = GT_NOT_INITIALIZED;
        /* not for log */
       return rc;
        /*CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"[Port %2d] port parameters not set on port", portNum);*/
    }

    cpssOsMemSet(portParams, 0, sizeof(CPSS_PM_PORT_PARAMS_STC));
    portParams->portType = pmDB->portMngSmDb[portNum]->portType;

    switch (portParams->portType)
    {
    case CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E:
        rc = prvCpssPmRegPortParamsGet(devNum, portNum, portParams);
        break;
    case CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E:
        rc = prvCpssPmApPortParamsGet(devNum, portNum, portParams);
        break;
    default:
        rc = GT_BAD_PARAM;
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "Problem with Port Type");
    }

    if (rc == GT_OK)
    {
        portParams->magic = CPSS_PM_MAGIC;
    }

    return rc;
}

/**
* @internal cpssPortManagerLuaSerdesTypeGet
* @endinternal
*
* @brief   getting the serdes type
*
* @param[in] devNum                - device number
*
* @param[out] serdesType           - serdes type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this function is for using port-manager.lua function
* don't use this function from other files than port-manager.lua
*
*/
GT_STATUS cpssPortManagerLuaSerdesTypeGet
(
    IN  GT_SW_DEV_NUM              devNum,
    OUT CPSS_PORT_SERDES_TYPE_ENT  *serdesType
)
{
    /*Check if device exists*/
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    /*Check for valid device family*/
    if (!(CPSS_PORT_MANAGER_CHECK_DEV_APPLICABLE_MAC(devNum)))
    {
        CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Port manager not supported for %d device",
                                                   PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        *serdesType = CPSS_PORT_SERDES_AVAGO_E;
    }
    else
    {
        *serdesType = CPSS_PORT_SERDES_COMPHY_H_E;
    }
    return GT_OK;
}


/**
* @internal internal_cpssPortManagerEnableGet
* @endinternal
*
* @brief  checks if portManager enabled/disabled per device
*
* @param[in] devNum                - device number
*
* @param[out] enablePtr            - Port-manager status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS internal_cpssPortManagerEnableGet
(
    IN  GT_U8 devNum,
    OUT GT_BOOL *enablePtr
)
{
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);
    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(enablePtr);

    *enablePtr = PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(isPortMgrEnable)[devNum];
    return GT_OK;
}

/**
* @internal internal_cpssPortManagerStatGet
* @endinternal
*
* @brief  get portManager statistics
*
* @param[in]  devNum               - device number
* @param[in]  portNum              - port number
* @param[out]  portStatStcPtr       - port statistic structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*
*
*/
GT_STATUS internal_cpssPortManagerStatGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT CPSS_PORT_MANAGER_STATISTICS_STC    *portStatStcPtr
)
{
    PRV_CPSS_PORT_MNG_DB_STC      *tmpPortManagerDbPtr;
    GT_STATUS rc;
    GT_U32                  phyPortNum; /* port number in local core */
    MV_HWS_AP_PORT_STATS    apStats;    /* AP statistics information in HWS format*/

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);
    CPSS_PORT_MANAGER_NULL_PTR_CHECK_MAC(portStatStcPtr);
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);
    if (( tmpPortManagerDbPtr == NULL ) || (tmpPortManagerDbPtr->portMngSmDb[portNum] == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw)
    {
        rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &phyPortNum);
        if (rc != GT_OK)
        {
            CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);
        }
        rc = mvHwsApPortCtrlStatsGet(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum),
                                     phyPortNum, &apStats);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlStatsGet: Hws return code is %d", rc);
            return rc;
        }
        portStatStcPtr->alignLockTime = apStats.linkUpTime;
        portStatStcPtr->linkFailCnt = apStats.linkFailCnt;
        portStatStcPtr->linkOkCnt = apStats.linkSuccessCnt;
        if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw )
        {
            portStatStcPtr->linkFailToSignalTime = apStats.hcdResoultionTime;
        }
    }
    else
    {
        cpssOsMemCpy(portStatStcPtr, &tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics, sizeof(CPSS_PORT_MANAGER_STATISTICS_STC));
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &phyPortNum);
            if (rc != GT_OK)
            {
                    CPSS_PORT_MANAGER_LOG_ERROR_AND_RETURN_MAC(rc, "[Port %d/%2d] ppCheckAndGetMacFunc failed=%d", devNum, portNum, rc);
            }
            rc = mvHwsApPortCtrlStatsGet(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum),
                    phyPortNum, &apStats);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlStatsGet: Hws return code is %d", rc);
                return rc;
            }
            portStatStcPtr->linkFailCnt    = apStats.linkFailCnt;
            portStatStcPtr->rxTrainingTime = apStats.rxTrainDuration;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPortManagerStatClear
* @endinternal
*
* @brief  clear portManager statistic db
*
* @param[in]  devNum               - device number
* @param[in]  portNum              - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
*
*
*/
GT_STATUS internal_cpssPortManagerStatClear
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    PRV_CPSS_PORT_MNG_DB_STC      *tmpPortManagerDbPtr;
    GT_STATUS rc;
    GT_U32                   phyPortNum; /* port number in local core */

    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);
    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);
    if (( tmpPortManagerDbPtr == NULL ) || (tmpPortManagerDbPtr->portMngSmDb[portNum] == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(&tmpPortManagerDbPtr->portMngSmDb[portNum]->statistics, 0, sizeof(CPSS_PORT_MANAGER_STATISTICS_STC) );
    tmpPortManagerDbPtr->portMngSmDb[portNum]->timeStampSec = 0;
    tmpPortManagerDbPtr->portMngSmDb[portNum]->timeStampNSec = 0;
    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->pmOverFw)
    {
        phyPortNum = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
        rc = mvHwsApPortCtrlStatsReset(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum), phyPortNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlStatsGet: Hws return code is %d", rc);
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal iprvCpssPortManagerPortStatusPrint
* @endinternal
*
* @brief  print info about the port
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssPortManagerPortStatusPrint
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_STATUS                     rc = GT_OK;
    PRV_CPSS_PORT_MNG_DB_STC      *tmpPortManagerDbPtr;
    GT_BOOL                       signalDetect,gbLock,alignLock;
    GT_U32                        portMacNum;
    CPSS_PORT_SPEED_ENT           speed;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    MV_HWS_PORT_STANDARD          portMode;
#if 0 /*TBD - lanes status*/
    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    CPSS_PORT_SERDES_TUNE_STC   serdesTuneRes;
    GT_U32                        laneNum;
#endif
    CPSS_PORT_MANAGER_DEV_CHECK_MAC(devNum);

    PRV_PORT_MANAGER_GET_PM_DATABASE(devNum, tmpPortManagerDbPtr, rc);
    if (( tmpPortManagerDbPtr == NULL ) || (tmpPortManagerDbPtr->portMngSmDb[portNum] == NULL))
    {
        cpssOsPrintf("devNum: %d  DB not allocated\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    }
    if ( tmpPortManagerDbPtr->portMngSmDb[portNum]->portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    speed        = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->speedDb;
    ifMode       = tmpPortManagerDbPtr->portsAttributedDb.perPhyPortDbPtr[portNum]->ifModeDb;
    portMacNum   = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum;
    portMode     = tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMode;

    cpssOsPrintf("devNum: %d  portNum:  %d MAC portNum: %d\n", devNum, portNum, portMacNum);
    cpssOsPrintf("---port-configuration---\n");
    cpssOsPrintf("  mode: %d  speed: %d fec: %d portMode: %d\n",speed, ifMode, tmpPortManagerDbPtr->portMngSmDb[portNum]->fecAbility,portMode);

    cpssOsPrintf("port manager %d\n",tmpPortManagerDbPtr->portMngSmDb[portNum]->portManaged);
    cpssOsPrintf(" oneShotIcal is: %d\n", tmpPortManagerDbPtr->portMngSmDb[portNum]->oneShotiCal);

    if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_LINK_UP_E\n");
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_LINK_DOWN_E\n");
    }
   else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_\n");
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E\n");
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_FAILURE_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_FAILURE_E\n");
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_LAST_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_LAST_E\n");
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_RESET_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_RESET_E\n");
        return rc;
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_DEBUG_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_DEBUG_E\n");
    }
    else if (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM == CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E)
    {
        cpssOsPrintf("state: CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E\n");
    }
    cpssOsPrintf("portmanger DB oper disable: %d\n",tmpPortManagerDbPtr->portMngSmDb[portNum]->portOperDisableSM);

    if ((tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_RESET_E) &&
        (tmpPortManagerDbPtr->portMngSmDb[portNum]->portAdminSM != CPSS_PORT_MANAGER_STATE_FORCE_LINK_DOWN_E))
    {
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSigDetGetFunc(devNum, portNum, &signalDetect);
        if(rc == GT_OK)
        {
            cpssOsPrintf("register signal Detect status: %s\n", (signalDetect == GT_TRUE ? "true" : "false"));
            cpssOsPrintf("portmanger DB signal Detect status: %s\n", (tmpPortManagerDbPtr->portMngSmDb[portNum]->signalDetected == GT_TRUE ? "true" : "false"));
        }
        if ( PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCdrLockGetFunc != NULL )
        {
            PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCdrLockGetFunc(devNum, portNum, &signalDetect);
            if(rc == GT_OK)
            {
                cpssOsPrintf("register cdr lock status: %s\n", (signalDetect == GT_TRUE ? "true" : "false"));
            }
        }
    }


    rc = prvCpssPortPcsGearBoxStatusGet(CAST_SW_DEVNUM(devNum), portNum,
                                        tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum, &gbLock);
    if (rc == GT_OK)
    {
        cpssOsPrintf("register gearbox lock status: %s\n", (gbLock == GT_TRUE ? "true" : "false"));
    }

    rc = prvCpssPortPcsAlignLockStatusGet(CAST_SW_DEVNUM(devNum), portNum,
                                          tmpPortManagerDbPtr->portsAttributedDb.generalPortCfgPtr[portNum]->portMacNum, &alignLock);
    if (rc == GT_OK)
    {
        cpssOsPrintf("register align lock status: %s\n", (alignLock == GT_TRUE ? "true" : "false"));
    }


    cpssOsPrintf("-------------\n");
    cpssOsPrintf("portDbStabilityDone: %s\n",
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == CPSS_PORT_MANAGER_IN_PROGRESS_E ? "IN PROGRESS" :
               (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbStabilityDone == CPSS_PORT_MANAGER_DONE_E ? "DONE" : "RESET")));

    cpssOsPrintf("rxStabilityTrainDone: %s\n",
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone == CPSS_PORT_MANAGER_IN_PROGRESS_E ? "IN PROGRESS" :
               (tmpPortManagerDbPtr->portMngSmDb[portNum]->rxStabilityTrainDone == CPSS_PORT_MANAGER_DONE_E ? "DONE" : "RESET")));

    cpssOsPrintf("portDbTrainDone:     %s\n",
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone == CPSS_PORT_MANAGER_IN_PROGRESS_E ? "IN PROGRESS" :
               (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbTrainDone == CPSS_PORT_MANAGER_DONE_E ? "DONE" : "RESET")));

    cpssOsPrintf("portDbAlignLockDone: %s\n",
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone == CPSS_PORT_MANAGER_IN_PROGRESS_E ? "IN PROGRESS" :
               (tmpPortManagerDbPtr->portMngSmDb[portNum]->portDbAlignLockDone == CPSS_PORT_MANAGER_DONE_E ? "DONE" : "RESET")));
    cpssOsPrintf("-------------\n");

    cpssOsPrintf("---low level mask---\n");
    cpssOsPrintf("  CPSS_PM_PCS_ALIGN_LOCK_LOST_MASKING_E is: %s\n",
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap && CPSS_PM_PCS_ALIGN_LOCK_LOST_MASKING_E == 0x1 ? "mask" : "unmask"));

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        cpssOsPrintf("  CPSS_PM_PCS_GB_LOCK_SYNC_CHANGE_MASKING_E is: %s\n",
                 (tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap && CPSS_PM_PCS_GB_LOCK_SYNC_CHANGE_MASKING_E == 0x2 ? "mask" : "unmask"));

        cpssOsPrintf("  CPSS_PM_MMPCS_SIGNAL_DETECT_CHANGE_MASKING_E is: %s\n",
                 (tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap && CPSS_PM_MMPCS_SIGNAL_DETECT_CHANGE_MASKING_E == 0x4 ? "mask" : "unmask"));
    }

    cpssOsPrintf("---mac level mask---\n");
    cpssOsPrintf("  CPSS_PM_PORT_LINK_STATUS_CHANGED_E is: %s\n",
             (tmpPortManagerDbPtr->portMngSmDb[portNum]->maskingBitMap && CPSS_PM_PORT_LINK_STATUS_CHANGED_E == 0x8 ? "mask" : "unmask"));

    return GT_OK;
}


