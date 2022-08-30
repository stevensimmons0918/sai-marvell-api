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
* @file prvCpssDxChTxqFcGopUtils.c
*
* @brief CPSS SIP6 TXQ Flow Control L1 abstraction layer
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS

/*Use prvCpssFalconGopMtiRegsDump in order to debug adresses*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPfcc.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcUtils.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortDpIronman.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


typedef struct{

    GT_U32                  regAddrMti100;
    GT_U32                  regAddrEmacMti100;
    GT_U32                  regAddrMti400;
    GT_U32                  regAddrMtiCpu;
    GT_U32                  regAddrUsx;

} PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC;


#define PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_COMMAND_CONFIG(_devNum,_portMacNum,_addrDb) \
    do\
    {\
       cpssOsMemSet(&_addrDb,PRV_CPSS_SW_PTR_ENTRY_UNUSED,sizeof(PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC));\
      _addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100_MAC.commandConfig;\
      _addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI400_MAC.commandConfig;\
      _addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI_CPU_MAC.commandConfig;\
      _addrDb.regAddrUsx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI_USX_MAC.commandConfig;\
      _addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100_MAC.EMAC.emac_commandConfig;\
    }while(0);

#define PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_MAC0_MAC(_devNum,_portMacNum,_addrDb) \
    do\
    {\
       cpssOsMemSet(&_addrDb,PRV_CPSS_SW_PTR_ENTRY_UNUSED,sizeof(PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC));\
      _addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100_MAC.macAddr0;\
      _addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI400_MAC.macAddr0;\
      _addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI_CPU_MAC.macAddr0;\
      _addrDb.regAddrUsx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI_USX_MAC.macAddr0;\
      _addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100_MAC.EMAC.emac_macAddr0;\
    }while(0);


#define PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_MAC1_MAC(_devNum,_portMacNum,_addrDb) \
    do\
    {\
       cpssOsMemSet(&_addrDb,PRV_CPSS_SW_PTR_ENTRY_UNUSED,sizeof(PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC));\
      _addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100_MAC.macAddr1;\
      _addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI400_MAC.macAddr1;\
      _addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI_CPU_MAC.macAddr1;\
      _addrDb.regAddrUsx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI_USX_MAC.macAddr1;\
      _addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100_MAC.EMAC.emac_macAddr1;\
    }while(0);





#define PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_NO_ERROR(_devNum) \
    do\
    {\
      if(GT_TRUE==PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum))\
      {/*FCU is not supported for this device*/\
          return GT_OK;\
      }\
    }while(0);


#define PRV_TXQ_GOP_UTILS_CHECK_MIF_SUPPORT_NO_ERROR(_devNum) \
    do\
    {\
      if(GT_FALSE==PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum))\
      {/*MIF is not supported for this device*/\
          return GT_OK;\
      }\
    }while(0);



#define PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(_devNum) \
    do\
    {\
      if(GT_TRUE==PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum))\
      {/*FCU is not supported for this device*/\
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Device  %d does not support FCU unit",_devNum);\
      }\
    }while(0);

#define PRV_GOP_FIELD_GET_WITH_TYPE_MAC(_name,_rawData,_valueToGet,_type) _valueToGet =(_type) U32_GET_FIELD_MAC(_rawData,\
                                                                             _name##_FIELD_OFFSET,\
                                                                                _name##_FIELD_SIZE);

extern GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsParamsArrayGet
(
   IN GT_U8    devNum,
   IN GT_U32   portGroup,
   IN GT_U32   portNum
);

extern GT_STATUS hwsPortModeParamsGetToBuffer
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   OUT MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
);

static GT_STATUS prvCpssDxChPortSip6IsNwCpuPort
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                  *isNwCpuPortPtr
)
{
    GT_STATUS rc;
    GT_U32  tile,dp,localPort;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,
                                &tile,&dp,&localPort,&mappingType);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *isNwCpuPortPtr = GT_FALSE;

    if((localPort == CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)-1)&&
        (mappingType==CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E))
    {
        *isNwCpuPortPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSip6MacSet function
* @endinternal
*
* @brief   Write data to all  MACs.Same date is duplicated .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] regAddrMti100           - adress of MTI 100 MAC
* @param[in] regAddrMti400           -adress of MTI 400 MAC
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static  GT_STATUS prvCpssDxChPortSip6MacSet
(
    IN   GT_U8                                       devNum,
    IN   GT_PHYSICAL_PORT_NUM                        portNum,
    IN   PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC      *regAddrDbPtr,
    IN   GT_U32                                     fieldData,
    IN   GT_U32                                     fieldOffset,
    IN   GT_U32                                     fieldLength
)
{

    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(regAddrDbPtr);

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);



    if(regAddrDbPtr->regAddrMti100 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddrDbPtr->regAddrMti100;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = fieldLength;
    }

    if(regAddrDbPtr->regAddrMti400 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddrDbPtr->regAddrMti400;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = fieldLength;
    }


    if(regAddrDbPtr->regAddrMtiCpu != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddrDbPtr->regAddrMtiCpu;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = fieldLength;
    }

    if(regAddrDbPtr->regAddrUsx != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddrDbPtr->regAddrUsx;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = fieldLength;
    }

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc!=GT_OK)
    {
      return rc;
    }

    /*Note cleared again for BR setting only*/

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddrDbPtr->regAddrEmacMti100!= PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddrDbPtr->regAddrEmacMti100;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = fieldData;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = fieldOffset;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = fieldLength;

        rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
        if(rc!=GT_OK)
        {
          return rc;
        }
    }


    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Enable/disable PRIORITY flow control
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcModeEnable           -If equals GT_TRUE,the Core generates and processes PFC control frames according
*  to the Priority Flow Control Interface signals. If equals GT_FALSE (0 - Reset Value), the Core operates in legacy Pause Frame
*  mode and generates and processes standard Pause Frames.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6PfcModeEnableSet
(
  IN  GT_U8                    devNum,
  IN  GT_PHYSICAL_PORT_NUM     portNum,
  IN  GT_BOOL                  pfcModeEnable
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_COMMAND_CONFIG(devNum,portMacNum,addrDb)

    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        BOOL2BIT_MAC(pfcModeEnable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE
        );


    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Enable/disable  flow control message forwarding to user application.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcPauseFwdEnable           -Terminate / Forward Pause Frames. If set to 'GT_TRUE', pause frames are forwarded to the user application.
*  If set to 'GT_FALSE' (0- Reset value), pause frames are terminated and discarded within the MAC.
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6PfcPauseFwdEnableSet
 (
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  pfcPauseFwdEnable
 )
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_COMMAND_CONFIG(devNum,portMacNum,addrDb)


    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        BOOL2BIT_MAC(pfcPauseFwdEnable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE
        );


    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcPauseFwdEnableGet function
* @endinternal
*
* @brief   Get enable/disable  flow control message forwarding to user application.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcPauseFwdEnable           -Terminate / Forward Pause Frames. If set to 'GT_TRUE', pause frames are forwarded to the user application.
*  If set to 'GT_FALSE' (0- Reset value), pause frames are terminated and discarded within the MAC.
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseFwdEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *pfcPauseFwdEnablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(pfcPauseFwdEnablePtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_FWD_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *pfcPauseFwdEnablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcModeEnableSet function
* @endinternal
*
* @brief   Set "pause quanta" that will appear in pause frame sent by port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] pauseQuanta           -Pause quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseQuantaSet
    (
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   pauseQuanta
    )
{
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    cpssOsMemSet(&addrDb,PRV_CPSS_SW_PTR_ENTRY_UNUSED,sizeof(PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC));

    switch(tc)
    {
    case 0:
    case 1:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl01PauseQuanta;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01PauseQuanta;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl01PauseQuanta;
        addrDb.regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl01PauseQuanta;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl01PauseQuanta;
        break;
    case 2:
    case 3:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl23PauseQuanta;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23PauseQuanta;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl23PauseQuanta;
        addrDb.regAddrUsx     = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl23PauseQuanta;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl23PauseQuanta;
        break;
    case 4:
    case 5:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl45PauseQuanta;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45PauseQuanta;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl45PauseQuanta;
        addrDb.regAddrUsx     = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl45PauseQuanta;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl45PauseQuanta;
        break;
    case 6:
    case 7:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl67PauseQuanta;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67PauseQuanta;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl67PauseQuanta;
        addrDb.regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl67PauseQuanta;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl67PauseQuanta;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    /*Assumption same quanta value for MTI100 and MTI400 and MTI CPU*/

    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,&addrDb,
        pauseQuanta,
        (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
        PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE
        );

    return rc;


}


/**
* @internal prvCpssDxChPortSip6PfcQuantaThreshSet function
* @endinternal
*
* @brief   Set interval  that determines how often to "refresh the Xoff frame",
* Meaning, if an Xoff frame sent, the Threshold is reached, and the Xoff_gen input is still high,
* A new Xoff frame will be transmitted.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] quantaThresh           -Interval  quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcQuantaThreshSet
    (
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   quantaThresh
    )
{
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc;

    if(quantaThresh>=1<<PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    cpssOsMemSet(&addrDb,PRV_CPSS_SW_PTR_ENTRY_UNUSED,sizeof(PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC));

    switch(tc)
    {
    case 0:
    case 1:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl01QuantaThresh;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01QuantaThresh;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl01QuantaThresh;
        addrDb.regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl01QuantaThresh;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl01QuantaThresh;
        break;
    case 2:
    case 3:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl23QuantaThresh;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23QuantaThresh;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl23QuantaThresh;
        addrDb.regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl23QuantaThresh;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl23QuantaThresh;
        break;
    case 4:
    case 5:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl45QuantaThresh;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45QuantaThresh;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl45QuantaThresh;
        addrDb.regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl45QuantaThresh;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl45QuantaThresh;
        break;
    case 6:
    case 7:
        addrDb.regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl67QuantaThresh;
        addrDb.regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67QuantaThresh;
        addrDb.regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl67QuantaThresh;
        addrDb.regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl67QuantaThresh;
        addrDb.regAddrEmacMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.EMAC.emac_cl67QuantaThresh;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    /*Assumption same quanta value for MTI100 and MTI400 and MTI CPU*/

    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,&addrDb,
        quantaThresh,
        (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
        PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE
        );

    return rc;


}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlEnableSet function
* @endinternal
*
* @brief   FCU RX/TX  Channel Enable/Disable. Also set channel ID on demand.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           -If GT_TRUE flow control is enabled on TX  on port,else disabled.
* @param[in] rxEnable           -If GT_TRUE flow control is enabled on RX  on port,else disabled.
* @param[in] setChannelId           -If GT_TRUE then channel ID is configured ,else not congfigured.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlEnableSet
(
   GT_U8                   devNum,
   GT_U32                  portNum,
   GT_BOOL                 txEnable,
   GT_BOOL                 rxEnable,
   GT_BOOL                 segmentedChannel,
   GT_BOOL                 setChannelId
)
{
    GT_U32 mpfChannel;
    GT_U32 regAddrRx, regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_BOOL isNwCpuPort;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(devNum);

    /*For Hawk
          EPI/<MIF> MIF/MIF registers %a/MIF Tx registers/Mif Type 8 Tx Pfc Control*/

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;

    rc = prvCpssDxChPortSip6IsNwCpuPort(devNum,portNum,&isNwCpuPort);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if (GT_TRUE==isNwCpuPort)
    {
        mpfChannel = 8;
    }


    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];
    regAddrRx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelRXControl[mpfChannel];

    /*set enable*/

    rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_SIZE,BOOL2BIT_MAC(txEnable));

    if(rc!=GT_OK)
    {
        return rc;
    }


    rc = prvCpssHwPpSetRegField(devNum,regAddrRx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_SIZE,BOOL2BIT_MAC(rxEnable));

    if(rc!=GT_OK)
    {
        return rc;
    }

    /*set channel ID*/

    if(setChannelId == GT_TRUE)
    {
        rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM_FIELD_OFFSET,
            PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM_FIELD_SIZE,segmentedChannel?mpfChannel>>2:mpfChannel);
        if(rc!=GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_OFFSET,
            PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_SIZE,segmentedChannel?mpfChannel>>2:mpfChannel);

        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlSegmentedChannelSet function
* @endinternal
*
* @brief   Mark channel as segmented.Both Rx and Tx
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] setSegmentedChannel           -If GT_TRUE mark channel as segmented(200G and 400G)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlSegmentedChannelSet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 setSegmentedChannel
)
{
    GT_U32 mpfChannel;
    GT_U32 regAddrRx, regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_BOOL isNwCpuPort;

   PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(devNum);

   /*For Hawk
          EPI/<MIF> MIF/MIF registers %a/MIF Tx registers/Mif Type 8 Tx Pfc Control*/

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* only modes 200 and 400 are multiSeg
    if ((portMode == _200GBase_KR4 ) || (portMode == _200GBase_KR8 ) || (portMode == _400GBase_KR8 ))
    {
    multiSeg = GT_TRUE;
    }
    */

    mpfChannel = portMacNum % 8;


    rc = prvCpssDxChPortSip6IsNwCpuPort(devNum,portNum,&isNwCpuPort);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if (GT_TRUE==isNwCpuPort)
    {
        mpfChannel = 8;
    }

    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];
    regAddrRx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelRXControl[mpfChannel];

    rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE_FIELD_SIZE,BOOL2BIT_MAC(setSegmentedChannel));

    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpSetRegField(devNum,regAddrRx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_CHANNEL_ID_NUM_FIELD_SIZE,BOOL2BIT_MAC(setSegmentedChannel));

    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet function
* @endinternal
*
* @brief   Set Rx flow control resolution 8 bit /16 bit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxType16Bit           -If GT_TRUE then flow control resolution is 16bit ,otherwise 8 bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet
(
    GT_U8                   devNum,
    GT_U32                  portMacNum,
    GT_BOOL                 rxType16Bit
)
{
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(devNum);

    /*For Hawk  /EPI/<MIF> MIF/MIF registers %a/MIF Global registers/Mif Type8 Channel Mapping<<%n>> bit 17*/

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuControl;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_SIZE,BOOL2BIT_MAC(rxType16Bit));

    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortSip6_10PfcConfigMifControlRxTypeSet
(
    GT_U8                   devNum,
    GT_U32                  portMacNum,
    GT_BOOL                 rxType16Bit
)
{

    GT_STATUS rc = GT_BAD_PARAM;
    MV_HWS_PORT_INIT_PARAMS * supportedModes = NULL;
    GT_BOOL     usxModeConfigured = GT_FALSE;
    GT_BOOL     segmentModeConfigured = GT_FALSE;
    GT_BOOL     nonSegmentModeConfigured = GT_FALSE;
    GT_BOOL     configurationRequired;

    supportedModes = hwsPortsParamsArrayGet(devNum,0,portMacNum);
    /*set all supported modes*/
    if(NULL != supportedModes)
    {
        while(supportedModes->portStandard!=NON_SUP_MODE)
        {
            configurationRequired = GT_FALSE;
            if(mvHwsUsxModeCheck(devNum, portMacNum, supportedModes->portStandard)&&!usxModeConfigured)
            {
                configurationRequired = GT_TRUE;
                usxModeConfigured = GT_TRUE;
            }
            else if((portMacNum == MV_HWS_AC5P_GOP_PORT_CPU_CNS)&&!usxModeConfigured)
            {
                configurationRequired = GT_TRUE;
                usxModeConfigured = GT_TRUE;
            }
            else if(HWS_IS_PORT_MULTI_SEGMENT((supportedModes->portStandard))&&!segmentModeConfigured)
            {
                configurationRequired = GT_TRUE;
                segmentModeConfigured = GT_TRUE;
            }
            else if(!nonSegmentModeConfigured)
            {
                 configurationRequired = GT_TRUE;
                 nonSegmentModeConfigured = GT_TRUE;
            }

            if(GT_TRUE==configurationRequired)
            {
                rc = mvHwsMifPfcModeSet(devNum,portMacNum,supportedModes->portStandard,rxType16Bit);
                if(rc!=GT_OK)
                {
                    return rc;
                }
            }

            supportedModes++;
        }
    }

    return rc;

}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeGet function
* @endinternal
*
* @brief   Get Rx flow control resolution 8 bit /16 bit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[out] rxType16BitPtr           -(pointer to)If GT_TRUE then flow control resolution is 16bit ,otherwise 8 bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTypeGet
(
    GT_U8                   devNum,
    GT_U32                  portMacNum,
    GT_BOOL                 *rxType16BitPtr
)
{
    GT_U32 regAddr,regValue;
    GT_STATUS rc;

   PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(devNum);

   /*For Hawk  /EPI/<MIF> MIF/MIF registers %a/MIF Global registers/Mif Type8 Channel Mapping<<%n>> bit 17*/

   regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuControl;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TYPE_FIELD_SIZE,&regValue);

    if(rc!=GT_OK)
    {
        return rc;
    }

    *rxType16BitPtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

GT_STATUS prvCpssDxChPortSip6_10PfcConfigMifControlRxTypeGet
(
    GT_U8                   devNum,
    GT_U32                  portMacNum,
    GT_BOOL                 *rxType16Bit
)
{
    MV_HWS_PORT_INIT_PARAMS portParam;
    GT_STATUS rc;

    /*get current mode*/
    rc = hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, NON_SUP_MODE, &portParam);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = mvHwsMifPfcModeGet(devNum,portMacNum,portParam.portStandard,rxType16Bit);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;

}


/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTypeSet function
* @endinternal
*
* @brief   Enable/disable Rx timer (periodic report on flow control status between QFC and D2D)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxTimerEnable           -If GT_TRUE then rx timer enabled,otherwise disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTimerEnableSet
(
   GT_U8                   devNum,
   GT_U32                  portNum,
   GT_BOOL                 rxTimerEnable
)
{
    GT_U32 portMacNum; /* MAC number */
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(devNum);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuControl;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TIMER_MODE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TIMER_MODE_FIELD_SIZE,BOOL2BIT_MAC(rxTimerEnable));

    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuControlRxTimerValueSet function
* @endinternal
*
* @brief  Configure  Rx timer value (periodic report on flow control status between QFC and D2D)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxTimerValue           -Timer value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6PfcConfigFcuControlRxTimerValueSet
(
   GT_U8                   devNum,
   GT_U32                  portNum,
   GT_U32                  rxTimerValue
)
{
    GT_U32 portMacNum; /* MAC number */
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_WITH_ERROR(devNum);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuRXTimer;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,PRV_PFC_MSDB_FCU_RX_TIMER_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_TIMER_FIELD_SIZE,rxTimerValue);

    if(rc!=GT_OK)
    {
        return rc;
    }



    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet function
* @endinternal
*
* @brief  Configure  FCU TX channel control enable
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           - Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet
 (
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 txEnable
)
{
    GT_U32 mpfChannel;
    GT_U32  regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_BOOL isNwCpuPort;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_NO_ERROR(devNum);

    /*For Hawk
          EPI/<MIF> MIF/MIF registers %a/MIF Tx registers/Mif Type 8 Tx Pfc Control*/

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;

    rc = prvCpssDxChPortSip6IsNwCpuPort(devNum,portNum,&isNwCpuPort);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if (GT_TRUE==isNwCpuPort)
    {
        mpfChannel = 8;
    }



    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];


    /*set enable*/

    rc = prvCpssHwPpSetRegField(devNum,regAddrTx,PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE_FIELD_SIZE,BOOL2BIT_MAC(txEnable));

    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet function
* @endinternal
*
* @brief  Configure  FCU TX channel control enable
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnablePtr           - (Pointer to)Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 *txEnablePtr,
    GT_BOOL                 *isSegmented,
    GT_U32                  *channelIdPtr
)
{
    GT_U32 mpfChannel;
    GT_U32  regAddrTx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 data;
    GT_BOOL isNwCpuPort;

    CPSS_NULL_PTR_CHECK_MAC(txEnablePtr);

    *txEnablePtr = GT_TRUE;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_NO_ERROR(devNum);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;


    rc = prvCpssDxChPortSip6IsNwCpuPort(devNum,portNum,&isNwCpuPort);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if (GT_TRUE==isNwCpuPort)
    {
        mpfChannel = 8;
    }

    regAddrTx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelTXControl[mpfChannel];

    /*get enable*/

    rc = prvCpssHwPpReadRegister(devNum,regAddrTx,&data);

    if(rc!=GT_OK)
    {
        return rc;
    }

    PRV_GOP_FIELD_GET_WITH_TYPE_MAC(PRV_PFC_MSDB_FCU_TX_CHANNEL_ENABLE,data, *txEnablePtr,GT_BOOL);
    PRV_GOP_FIELD_GET_WITH_TYPE_MAC(PRV_PFC_MSDB_FCU_TX_CHANNEL_SEGMENTED_MODE,data, *isSegmented,GT_BOOL);
    PRV_GOP_FIELD_GET_WITH_TYPE_MAC(PRV_PFC_MSDB_FCU_TX_CHANNEL_ID_NUM,data, *channelIdPtr,GT_U32);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet function
* @endinternal
*
* @brief  Get configure  of FCU RX channel control enable
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] rxEnablePtr           - (Pointer to)Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet
 (
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 *rxEnablePtr
)
{
    GT_U32 mpfChannel;
    GT_U32  regAddrRx;
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 data;
    GT_BOOL isNwCpuPort;

    CPSS_NULL_PTR_CHECK_MAC(rxEnablePtr);

    *rxEnablePtr = GT_TRUE;

    PRV_TXQ_GOP_UTILS_CHECK_FCU_SUPPORT_NO_ERROR(devNum);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    mpfChannel = portMacNum % 8;



    rc = prvCpssDxChPortSip6IsNwCpuPort(devNum,portNum,&isNwCpuPort);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if (GT_TRUE==isNwCpuPort)
    {
        mpfChannel = 8;
    }

    regAddrRx = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MSDB[portMacNum].fcuChannelRXControl[mpfChannel];

    /*get enable*/

    rc = prvCpssHwPpGetRegField(devNum,regAddrRx,PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_OFFSET,
        PRV_PFC_MSDB_FCU_RX_CHANNEL_ENABLE_FIELD_SIZE,&data);

    if(rc!=GT_OK)
    {
        return rc;
    }

    *rxEnablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}



/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet function
* @endinternal
*
* @brief  Configure  port flow control mode (Both TxQ and L1)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           - Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6FlowControlModeSet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
)
{
        GT_STATUS rc;
        GT_U32                          tileNum,dpNum,localdpPortNum,index;
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
        CPSS_DXCH_PORT_FC_MODE_ENT      currentFcMode;
        PRV_CPSS_PFCC_CFG_ENTRY_STC     entry;
        GT_U32                          tcBitVector=0x1;

        /*Find the port*/

        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
        }

        if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6FlowControlModeSet  failed for portNum  %d due to wrong mapping type ",portNum);
        }

        /*Get current configured mode*/
        rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,dpNum,localdpPortNum,NULL,&currentFcMode);

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortGet failed for portNum %d",portNum);
        }

        if(currentFcMode == fcMode)
        {
            return GT_OK;
        }

        index= (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*localdpPortNum+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));



         /*Configure PFCC -entry type= port ,only TC0 is set to avoid overhead*/

        if(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)
        {
            /*Check if the PFCC entry exist already*/
            rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,index,&entry);
            if(rc!=GT_OK)
            {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntryGet failed for index %d",index);
            }

            if(entry.entryType ==PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
            {
                if(fcMode == CPSS_DXCH_PORT_FC_MODE_PFC_E)
                {
                    /*check on which TC threshold is configured*/
                    rc = prvCpssFalconTxqPfcValidGet(devNum, tileNum,dpNum, localdpPortNum,&tcBitVector);
                    if (rc != GT_OK)
                    {
                         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssFalconTxqPfcValidGet failed");
                    }
                }
                rc = prvCpssFalconTxqPffcTableSyncSet(devNum,tileNum,dpNum,localdpPortNum,tcBitVector);
            }

        }
        else
        {
             rc = prvCpssFalconTxqPffcTableSyncSet(devNum,tileNum,dpNum,localdpPortNum,0);
        }

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPffcTableSyncSet failed for portNum %d",portNum);
        }

        /*Configure QFC - only port type ,mapping is done at the init*/

        rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum,tileNum,dpNum,localdpPortNum,0/*don't care*/,fcMode,GT_TRUE);

       if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortSet  failed for portNum  %d  ",portNum);
        }

        if(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)
        {
              /*Configure L1 MTI X COMMAND_CONFIG*/

             rc = prvCpssDxChPortSip6PfcModeEnableSet(devNum,portNum,(fcMode == CPSS_DXCH_PORT_FC_MODE_PFC_E)?GT_TRUE:GT_FALSE);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcModeEnableSet  failed for portNum  %d  ",portNum);
            }
       }

        /*handle xoff_gen bus towards MAC*/
        rc = prvCpssDxChPortSip6XoffOverrideEnableSet(devNum,portNum,(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)?GT_FALSE:GT_TRUE);
        if (rc != GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6XoffOverrideEnableSet failed for port %d \n",portNum);
        }

        /*Configure L1 MSDB FCU  for  SIP6 devices ,otherwise GT_OK is returned immediately */
        rc = prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet(devNum,portNum,(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)?GT_TRUE:GT_FALSE);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableSet  failed for portNum  %d  ",portNum);
        }

        /*Configure L1 MIF- SIP6_10  devices ,otherwise GT_OK is returned immediately */
        rc = prvCpssDxChPortPfcConfigMifEnableSet(devNum,portNum,(fcMode!= CPSS_DXCH_PORT_FC_MODE_DISABLE_E)?GT_TRUE:GT_FALSE);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortPfcConfigMifEnableSet  failed for portNum  %d  ",portNum);
        }

        return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet function
* @endinternal
*
* @brief  Get Configure  port flow control mode .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] txEnable           - Flow control enable on MSDB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6FlowControlModeGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT            *fcModePtr
)
{
    GT_STATUS rc;
    GT_U32 tileNum,dpNum,localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32 dummy;

    /*Find the port*/

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6FlowControlModeSet  failed for portNum  %d due to wrong mapping type ",portNum);
    }

    /*Get configuration from QFC*/

    rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,dpNum,localdpPortNum,&dummy,fcModePtr);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortSet  failed for portNum  %d  ",portNum);
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6PfcModeEnableGet function
* @endinternal
*
* @brief   Get enable/disable PRIORITY flow control
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[out] pfcModeEnablePtr -(Pointer to)If equals GT_TRUE,the Core generates and processes PFC control frames according
*  to the Priority Flow Control Interface signals. If equals GT_FALSE (0 - Reset Value), the Core operates in legacy Pause Frame
*  mode and generates and processes standard Pause Frames.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcModeEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                  *pfcModeEnablePtr,
    OUT GT_U32                   *regAddrPtr

)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);



    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);


    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PFC_MODE_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *pfcModeEnablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
   *regAddrPtr = regAddr;

    return rc;
}



/**
* @internal prvCpssDxChPortSip6PfcPauseQuantaGet function
* @endinternal
*
* @brief  Get "pause quanta" that will appear in pause frame sent by port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] pauseQuantaPtr           -(pointer to)Pause quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseQuantaGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   *pauseQuantaPtr
)
{
    GT_U32      regAddrMti100,regAddrMti400,regAddrMtiCpu,regAddrUsx;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc = GT_FAIL;
    PRV_CPSS_PORT_TYPE_ENT macType;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    switch(tc)
    {
    case 0:
    case 1:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl01PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01PauseQuanta;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl01PauseQuanta;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl01PauseQuanta;
        break;
    case 2:
    case 3:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl23PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23PauseQuanta;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl23PauseQuanta;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl23PauseQuanta;
        break;
    case 4:
    case 5:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl45PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45PauseQuanta;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl45PauseQuanta;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl45PauseQuanta;
        break;
    case 6:
    case 7:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl67PauseQuanta;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67PauseQuanta;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl67PauseQuanta;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl67PauseQuanta;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti100)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti100, (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
                    PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE,pauseQuantaPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti400)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti400, (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
                    PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE, pauseQuantaPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMtiCpu)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMtiCpu, (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
                    PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE, pauseQuantaPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrUsx)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrUsx, (tc%2)?PRV_PFC_MTI_X_PAUSE_QUANTA_REG_ODD_TC_PAUSE_QUANTA_FIELD_OFFSET:PRV_PFC_MTI_X_PAUSE_QUANTA_REG_EVEN_TC_PAUSE_QUANTA_FIELD_OFFSET,
                    PRV_PFC_MTI_X_PAUSE_QUANTA_REG_PAUSE_QUANTA_FIELD_SIZE, pauseQuantaPtr);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "macType %d is not supported ",macType);
    }

    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcQuantaThreshSet function
* @endinternal
*
* @brief   Set interval  that determines how often to "refresh the Xoff frame",
* Meaning, if an Xoff frame sent, the Threshold is reached, and the Xoff_gen input is still high,
* A new Xoff frame will be transmitted.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] tc           -Traffic class
* @param[in] quantaThresh           -Interval  quanta in 512 bit-time
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcQuantaThreshGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   tc,
    IN  GT_U32                   *quantaThreshPtr
)
{
    GT_U32      regAddrMti100,regAddrMti400,regAddrMtiCpu,regAddrUsx;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS rc = GT_FAIL;
    PRV_CPSS_PORT_TYPE_ENT macType;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    switch(tc)
    {
    case 0:
    case 1:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl01QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl01QuantaThresh;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl01QuantaThresh;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl01QuantaThresh;
        break;
    case 2:
    case 3:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl23QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl23QuantaThresh;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl23QuantaThresh;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl23QuantaThresh;
        break;
    case 4:
    case 5:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl45QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl45QuantaThresh;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl45QuantaThresh;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl45QuantaThresh;
        break;
    case 6:
    case 7:
        regAddrMti100 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.cl67QuantaThresh;
        regAddrMti400 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.cl67QuantaThresh;
        regAddrMtiCpu = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.cl67QuantaThresh;
        regAddrUsx    = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.cl67QuantaThresh;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tc %d is not supported ",tc);
        break;
    }

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti100)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti100, (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
                    PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE,quantaThreshPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrMti400)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMti400, (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
                    PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE, quantaThreshPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED !=  regAddrMtiCpu )
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrMtiCpu, (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
                    PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE, quantaThreshPtr);
        }
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddrUsx )
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddrUsx, (tc%2)?PRV_PFC_MTI_X_QUANTA_THRESH_REG_ODD_TC_QUANTA_THRESH_FIELD_OFFSET:PRV_PFC_MTI_X_QUANTA_THRESH_REG_EVEN_TC_QUANTA_THRESH_FIELD_OFFSET,
                    PRV_PFC_MTI_X_QUANTA_THRESH_REG_QUANTA_THRESH_FIELD_SIZE, quantaThreshPtr);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "macType %d is not supported ",macType);
    }

    return rc;

}
/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet function
* @endinternal
*
* @brief   Get enable/disable Link Pause Ignore Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enablePtr           -(pointer to)If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseIgnoreEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_COMMAND_CONFIG(devNum,portMacNum,addrDb)


    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        BOOL2BIT_MAC(enable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE
        );


    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet function
* @endinternal
*
* @brief   Enable/disable Link Pause Compatibility Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enable           -If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{

    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_COMMAND_CONFIG(devNum,portMacNum,addrDb)

    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        BOOL2BIT_MAC(enable),
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
        PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE
        );


    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableGet function
* @endinternal
*
* @brief   Get enable/disable Link Paus Compatibility Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enablePtr           -(pointer to)If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcLinkPauseCompatibilityEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *enablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_PFC_COMP_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal prvCpssDxChPortSip6PfcPauseIgnoreEnableGet function
* @endinternal
*
* @brief   Get enable/disable Link Pause Ignore Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enablePtr           -(pointer to)If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcPauseIgnoreEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *pfcPauseIgnoreEnablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_OFFSET,
                    PRV_PFC_MTI_X_COMMAND_CONFIG_REG_PAUSE_IGNORE_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

   *pfcPauseIgnoreEnablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}


/**
* @internal prvCpssDxChPortSip6PfcRxPauseStatusGet function
* @endinternal
*
* @brief   Get  Pause status .Status bit for software to read the current received pause status. One bit for each of the 16 classes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[out] rxPauseStatusPtr           -(pointer to)rx pause status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6PfcRxPauseStatusGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_U32                  *rxPauseStatusPtr,
    OUT  GT_U32                  *regAddrPtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.rxPauseStatus;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.rxPauseStatus;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.rxPauseStatus;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.rxPauseStatus;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

    *regAddrPtr = regAddr;

    if(rc==GT_OK)
    {
        *rxPauseStatusPtr = value;
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSip6XoffOverrideEnableSet function
* @endinternal
*
* @brief   This sets the value of the xoff_gen bus towards MAC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] enable           -enable/disable the override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6XoffOverrideEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

     if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(portMacNum >= PRV_CPSS_MAX_MAC_PORTS_NUM_CNS)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, \
             "portMacNum for portNum  %d  is %d (bigger then %d)",portNum,portMacNum,PRV_CPSS_MAX_MAC_PORTS_NUM_CNS);
    }

    if(GT_TRUE == enable)
    {
    /*
            0x1 = Xoff
            0x0 = Xon
            Actual generation of Xon will occur only on due to transition from 0x1 to 0x0.
            Note:  after setting it to 0x0 a flow control Xon frame may be sent or not,
            Other than that it will disable flow control generation.
    */
        value = 0xFF;/*bits  0-7  mask,bits  8-15  value*/
    }
    else
    {
        value = 0x0;/*bits  0-7  mask,bits  8-15  value*/
    }

    if(portMacNum>=PRV_CPSS_MAX_MAC_PORTS_NUM_CNS)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portMacNum for portNum  %d  is %d (bigger then %d)",portNum,portMacNum,PRV_CPSS_MAX_MAC_PORTS_NUM_CNS);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.xoffOverride;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.segXoffOverride;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_EXT.xoffOverride;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = 0;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_EXT.xoffOverride;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = 16;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = 0;
    }
    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal prvCpssDxChPortSip6XoffOverrideEnableGet function
* @endinternal
*
* @brief   This gets the value of the xoff_gen bus towards MAC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[out] enablePtr           -(pointer to) enable/disable the override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6XoffOverrideEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_U32                  *enablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.xoffOverride;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.segXoffOverride;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_EXT.xoffOverride;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_EXT.xoffOverride;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

    if(rc==GT_OK)
    {
        *enablePtr = (value == 0xFF)?GT_TRUE:GT_FALSE;
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSip6XoffStatusGet function
* @endinternal
*
* @brief   This function gets the value of the tx XOFF  MAC status .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[out] enablePtr           -(pointer to) XOFF status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6XoffStatusGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_U32                  *regAddrPtr,
    OUT  GT_U32                  *xoffStatusPtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.portXoffStatus;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.segPortXoffStatus;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_EXT.portXoffStatus;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_EXT.portXoffStatus;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

         rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

    if(regAddrPtr)
    {
        *regAddrPtr = regAddr;
    }

    if(rc==GT_OK)
    {
        *xoffStatusPtr = value;
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSip6MacSaSet function
* @endinternal
*
* @brief  Sets the Mac Address for a port. Port MAC addresses
*         are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[in] macPtr           - mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6MacSaSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_ETHERADDR             *macPtr
)
{

    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_MAC0_MAC(devNum,portMacNum,addrDb);


    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        ((macPtr->arEther[3] << 24) | (macPtr->arEther[2] << 16) | (macPtr->arEther[1] << 8) | (macPtr->arEther[0])),
        0,
        32
        );
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Failed to set MAC SA");
    }

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_MAC1_MAC(devNum,portMacNum,addrDb);

    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        (macPtr->arEther[4]),
        0,
        8
        );
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Failed to set MAC SA");
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSip6MacSaGet function
* @endinternal
*
* @brief  Gets the Mac Address of a port. Port MAC addresses are
*         used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[out] macPtr          - mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6MacSaGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_ETHERADDR             *macPtr
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_U32 value0=0, value1=0;
    GT_U32 regAddr0, regAddr1;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr0 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.macAddr0;
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.macAddr1;
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr0 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.macAddr0;
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.macAddr1;
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr0 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.macAddr0;
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.macAddr1;
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr0 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.macAddr0;
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.macAddr1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
                regAddr0,&value0);
    if(rc!=GT_OK)
    {
       return rc;
    }


    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
                regAddr1,&value1);
    if(rc!=GT_OK)
    {
       return rc;
    }

    macPtr->arEther[0] = value0 & 0xFF;
    macPtr->arEther[1] = (value0 >> 8 ) & 0xFF ;
    macPtr->arEther[2] = (value0 >> 16) & 0xFF;
    macPtr->arEther[3] = (value0 >> 24) & 0xFF;
    macPtr->arEther[4] = (value1) & 0xFF;
    macPtr->arEther[5] = 0 ;

    return rc;
}

/**
* @internal prvCpssDxChPortSip6MacSaSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[in] macSaLsb         - the ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6MacSaLsbSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U8                    macSaLsb
)
{

    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */
    PRV_TXQ_GOP_UTILS_REGISTER_PARAMS_STC addrDb;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_TXQ_GOP_UTILS_ADDR_DB_INIT_MAC1_MAC(devNum,portMacNum,addrDb)


    rc = prvCpssDxChPortSip6MacSet(devNum, portNum,
        &addrDb,
        macSaLsb,
        8,
        8
        );
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Failed to set MAC SA");
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSip6MacSaGet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[out] macSaLsb        - (Pointer to) the ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6MacSaLsbGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U8                    *macSaLsbPtr
)
{
    GT_STATUS rc;

    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_U32 value=0;
    GT_U32 regAddr;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.macAddr1;
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.macAddr1;
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.macAddr1;
    }
      else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.macAddr1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
                regAddr,&value);

    *macSaLsbPtr = (value >> 8) & 0xFF;

    return rc;
}



GT_STATUS prvCpssDxChPortPfcConfigMifEnableSet
(
    GT_U8                   devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    GT_BOOL                 enable
)
{

    GT_STATUS rc = GT_BAD_PARAM;
    MV_HWS_PORT_INIT_PARAMS * supportedModes = NULL;
    GT_BOOL     usxModeConfigured = GT_FALSE;
    GT_BOOL     segmentModeConfigured = GT_FALSE;
    GT_BOOL     nonSegmentModeConfigured = GT_FALSE;
    GT_BOOL     configurationRequired;
    GT_U32      portMacNum;

    PRV_TXQ_GOP_UTILS_CHECK_MIF_SUPPORT_NO_ERROR(devNum);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    supportedModes = hwsPortsParamsArrayGet(devNum,0,portMacNum);
    /*set all supported modes*/
    if(NULL != supportedModes)
    {
        while(supportedModes->portStandard!=NON_SUP_MODE)
        {
            configurationRequired = GT_FALSE;
            if(mvHwsUsxModeCheck(devNum, portMacNum, supportedModes->portStandard)&&!usxModeConfigured)
            {
                configurationRequired = GT_TRUE;
                usxModeConfigured = GT_TRUE;
            }
            else if((portMacNum == MV_HWS_AC5P_GOP_PORT_CPU_CNS)&&!usxModeConfigured)
            {
                configurationRequired = GT_TRUE;
                usxModeConfigured = GT_TRUE;
            }
            else if(HWS_IS_PORT_MULTI_SEGMENT((supportedModes->portStandard))&&!segmentModeConfigured)
            {
                configurationRequired = GT_TRUE;
                segmentModeConfigured = GT_TRUE;
            }
            else if(!nonSegmentModeConfigured)
            {
                 configurationRequired = GT_TRUE;
                 nonSegmentModeConfigured = GT_TRUE;
            }

            if(GT_TRUE==configurationRequired)
            {
                if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
                {
                    /* Ironman */
                    rc = prvCpssDxChPortDpIronmanPortMifPfcEnableSet(
                        devNum, portMacNum,
                        enable/*enableTx*/, enable/*enableRx*/);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                else
                {
                    /*TX*/
                    rc = mvHwsMifPfcEnable(devNum,portMacNum,supportedModes->portStandard,GT_TRUE,GT_TRUE,&enable);
                    if(rc!=GT_OK)
                    {
                        return rc;
                    }
                    /*RX*/
                    rc = mvHwsMifPfcEnable(devNum,portMacNum,supportedModes->portStandard,GT_FALSE,GT_TRUE,&enable);
                    if(rc!=GT_OK)
                    {
                        return rc;
                    }
                }
            }

            supportedModes++;
        }
    }

    return rc;

}

GT_STATUS prvCpssDxChPortPfcConfigMifEnableGet
(
    GT_U8                   devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    GT_BOOL                 *txEnablePtr,
    GT_BOOL                 *rxEnablePtr
)
{
    MV_HWS_PORT_INIT_PARAMS portParam;
    GT_STATUS               rc;
    GT_U32                  portMacNum;

    PRV_TXQ_GOP_UTILS_CHECK_MIF_SUPPORT_NO_ERROR(devNum);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /*get current mode*/
    rc = hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, NON_SUP_MODE, &portParam);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        /* Ironman */
        rc = prvCpssDxChPortDpIronmanPortMifPfcEnableGet(
            devNum, portMacNum,
            txEnablePtr/*enableTxPtr*/, rxEnablePtr/*enableRxPtr*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = mvHwsMifPfcEnable(devNum,portMacNum,portParam.portStandard,GT_TRUE,GT_FALSE,txEnablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }

        rc = mvHwsMifPfcEnable(devNum,portMacNum,portParam.portStandard,GT_FALSE,GT_FALSE,rxEnablePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChPortSip6BrDataGet function
* @endinternal
*
* @brief  Get preemption relative data from GOP
*
* @note   APPLICABLE DEVICES:    AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon.

*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[out] brDataPtr        - (Pointer to) preemption MAC data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6BrDataGet
(
   IN  GT_U8                            devNum,
   IN  GT_PHYSICAL_PORT_NUM             portNum,
   OUT  PRV_TXQ_GOP_UTILS_BR_PARAMS_STC *brDataPtr
)
{

   GT_STATUS rc;

   GT_U32 portMacNum; /* MAC number */
   PRV_CPSS_PORT_TYPE_ENT macType;
   GT_U32 value;
   GT_U32 regAddr0, regAddr1;

   CPSS_NULL_PTR_CHECK_MAC(brDataPtr);

   brDataPtr->paramValid= GT_FALSE;

   PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

   macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

   if (TXQ_IS_PREEMPTIVE_DEVICE(devNum)&& (macType == PRV_CPSS_PORT_MTI_100_E))
   {
       regAddr0 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brCntrl ;
       regAddr1 = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brStatus ;

       if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr0)
       {
           rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
               PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                   regAddr0,&value);
           if(rc!=GT_OK)
           {
              return rc;
           }

           brDataPtr->control = value;


           /*if we have brCntrl we must have brStatus*/
           if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr1)
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

           rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                   PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                       regAddr1,&value);
           if(rc!=GT_OK)
           {
              return rc;
           }

           brDataPtr->status = value;
           brDataPtr->paramValid = GT_TRUE;
       }
   }

   return GT_OK;
}

/**
* @internal prvCpssDxChPortSip6BrDataGet function
* @endinternal
*
* @brief  Get preemption relative data (one parameter) from GOP.
*  Required for LUA since the structure PRV_TXQ_GOP_UTILS_BR_PARAMS_STC is private
*
* @note   APPLICABLE DEVICES:    AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon.

*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[in] inBrPrm          - BR parameter to get(see PRV_TXQ_GOP_UTILS_BR_PARAM_X defenition)
* @param[out] brDataPtr        - (Pointer to) preemption MAC data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChPortSip6BrSingleParameterGet
(
   IN  GT_U8                            devNum,
   IN  GT_PHYSICAL_PORT_NUM             portNum,
   IN  GT_U32                           inBrPrm,
   OUT GT_U32                           *outBrPrm
)
{
    GT_STATUS rc = GT_OK;
    PRV_TXQ_GOP_UTILS_BR_PARAMS_STC brData;

    CPSS_NULL_PTR_CHECK_MAC(outBrPrm);

    rc = prvCpssDxChPortSip6BrDataGet(devNum,portNum,&brData);
    if(rc!=GT_OK)
    {
       return rc;
    }

    switch(inBrPrm)
    {
        case PRV_TXQ_GOP_UTILS_BR_PARAM_VALID:
            *outBrPrm =(brData.paramValid==GT_TRUE)?1:0;
            break;
        case PRV_TXQ_GOP_UTILS_BR_PARAM_CONTROL:
            *outBrPrm = brData.control;
            break;
        case PRV_TXQ_GOP_UTILS_BR_PARAM_STATUS:
            *outBrPrm = brData.status;
            break;
        default:
            rc = GT_NOT_SUPPORTED;
            break;
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSip6GopFifoStatusGet function
* @endinternal
*
* @brief  Check status of MAC TX/RX fifo.
*
* @note   APPLICABLE DEVICES:    AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon.
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[out] txIdlePtr          (Pointer to)  MAC fifo Tx idle status
* @param[out] rxIdlePtr        - (Pointer to)  MAC fifo Rx idle status
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChPortSip6GopFifoStatusGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  *txIdlePtr,
    IN  GT_BOOL                  *rxIdlePtr
)
{
    GT_U32 regAddr;
    GT_U32 value=0;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;
    GT_U32   preemptionEnabledBit = 0;
    GT_U32   emacTxIdle = 1,pmacTxIdle;
    GT_U32   emacRxIdle = 1,pmacRxIdle;
    PRV_TXQ_GOP_UTILS_BR_PARAMS_STC brData;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    MV_HWS_PORT_STANDARD portMode = NON_SUP_MODE;
    GT_BOOL accessEnable;

    CPSS_NULL_PTR_CHECK_MAC(txIdlePtr);
    CPSS_NULL_PTR_CHECK_MAC(rxIdlePtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /*check systemRecovery */
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ( ( (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) ||
           (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) )&&
         (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E ) )
    {
        /*writes are disable ,under trafic TX/RX wont be idle so just skip the check and "act" as they are idle  */
        *txIdlePtr = GT_TRUE;
        *rxIdlePtr = GT_TRUE;
        return GT_OK;
    }


    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.status;
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.status;
    }
    else
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " unsupported mac type %d for portNum  %d \n ",macType, portNum);
    }

    if((CPSS_PORT_INTERFACE_MODE_NA_E != PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum))
       && (CPSS_PORT_SPEED_NA_E != PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum)))
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                                        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                                        &portMode);
    }

    /*protect mac clock enable*/
    mvHwsCm3SemOper(devNum,portMacNum, GT_TRUE);
    if(( rc == GT_OK) &&(portMode != NON_SUP_MODE))
    {
        value = 0xFFFFFFFF;
        rc = mvHwsPortAccessCheck(devNum, portMacNum, portMode, &accessEnable);
        if(accessEnable == GT_TRUE)
        {
            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                                portMacNum),
                        regAddr,&value);
        }
    }
    mvHwsCm3SemOper(devNum,portMacNum, GT_FALSE);

    if(rc == GT_OK)
    {
       pmacTxIdle = U32_GET_FIELD_MAC(value,PRV_PFC_MTI_X_COMMAND_TX_FIFO_IDLE_STATUS_FIELD_OFFSET,
         PRV_PFC_MTI_X_COMMAND_TX_FIFO_IDLE_STATUS_FIELD_SIZE);
       pmacRxIdle= U32_GET_FIELD_MAC(value,PRV_PFC_MTI_X_COMMAND_RX_FIFO_IDLE_STATUS_FIELD_OFFSET,
         PRV_PFC_MTI_X_COMMAND_RX_FIFO_IDLE_STATUS_FIELD_SIZE);

       rc = prvCpssDxChPortSip6BrDataGet(devNum,portNum,&brData);
       if(rc!=GT_OK)
       {
          return rc;
       }

       if(GT_TRUE ==brData.paramValid)
       {
          preemptionEnabledBit = U32_GET_FIELD_MAC(brData.control,PRV_PFC_MTI_X_COMMAND_TX_PREEMPT_EN_FIELD_OFFSET,
             PRV_PFC_MTI_X_COMMAND_TX_PREEMPT_EN_FIELD_SIZE);
       }


       /*check also emac status*/
       if(1==preemptionEnabledBit )
       {
            emacTxIdle = U32_GET_FIELD_MAC(value,PRV_PFC_MTI_X_COMMAND_EMAC_TX_FIFO_IDLE__STATUS_FIELD_OFFSET,
             PRV_PFC_MTI_X_COMMAND_EMAC_TX_FIFO_IDLE__STATUS_FIELD_SIZE);

            emacRxIdle = U32_GET_FIELD_MAC(value,PRV_PFC_MTI_X_COMMAND_EMAC_RX_FIFO_IDLE_STATUS_FIELD_OFFSET,
             PRV_PFC_MTI_X_COMMAND_EMAC_RX_FIFO_IDLE_STATUS_FIELD_SIZE);
        }
#ifndef ASIC_SIMULATION
        *txIdlePtr = BIT2BOOL_MAC((pmacTxIdle&emacTxIdle));
        *rxIdlePtr = BIT2BOOL_MAC((pmacRxIdle&emacRxIdle));
#else
/*simulation does not support this*/
        *txIdlePtr = GT_TRUE;
        *rxIdlePtr = GT_TRUE;
/*avoid warning*/
        emacTxIdle &=emacRxIdle;
        pmacTxIdle &=pmacRxIdle;
#endif

    }


    return rc;
}

/**
* @internal prvCpssDxChPortSip6XonEnableSet function
* @endinternal
*
* @brief  Enable/disable XON generation
*
* @note   APPLICABLE DEVICES:    Falcon;AC5P; AC5X;Harrier;Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[in] enable           -if equal GT_TRUE XON is generated ,otherwise XON is not generated
*
* @retval GT_OK               - on success
* @retval GT_BAD_PTR          - on NULL ptr
* @retval GT_HW_ERROR         - if write failed
*/
GT_STATUS prvCpssDxChPortSip6XonEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

     if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

     /* Pulse mode:
             When set to 1 the input xoff_gen is sampled on the rising edge and no XON frames are generated.
             Set it to 0 for normal PFC operation.
          */

    if(GT_TRUE == enable)
    {
      value = 0x0;
    }
    else
    {
      value = 0x1;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.xifMode;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_100_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldLength = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE;
        regDataArray[PRV_CPSS_PORT_MTI_100_E].fieldOffset = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.xifMode;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_400_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldLength = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE;
        regDataArray[PRV_CPSS_PORT_MTI_400_E].fieldOffset = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.xifMode;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldLength = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE;
        regDataArray[PRV_CPSS_PORT_MTI_CPU_E].fieldOffset = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.xifMode;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldLength = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE;
        regDataArray[PRV_CPSS_PORT_MTI_USX_E].fieldOffset = PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET;
    }
    return prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);
}
/**
* @internal prvCpssDxChPortSip6XonEnableGet function
* @endinternal
*
* @brief  Get enable/disable XON generation
*
* @note   APPLICABLE DEVICES:    Falcon;AC5P; AC5X;Harrier;Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum           - device number.
* @param[in] portNum          - physical port number
* @param[out] enablePtr       -(pointer to)if equal GT_TRUE XON is generated ,otherwise XON is not generated
*
* @retval GT_OK               - on success
* @retval GT_BAD_PTR          - on NULL ptr
* @retval GT_HW_ERROR         - if write failed
*/
GT_STATUS prvCpssDxChPortSip6XonEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.xifMode;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET,
                    PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.xifMode;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET,
                    PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.xifMode;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET,
                    PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE, &value);
    }
    else if (macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.xifMode;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr, PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_FIELD_OFFSET,
                    PRV_PFC_MAC_XIF_MODE_PFC_PULSE_MODE_EN_FIELD_SIZE, &value);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unsupported MAC type %d",macType);
    }


     /* Pulse mode:
             When set to 1 the input xoff_gen is sampled on the rising edge and no XON frames are generated.
             Set it to 0 for normal PFC operation.
          */

   *enablePtr = (value == 1) ?  GT_FALSE:GT_TRUE;

    return rc;
}


