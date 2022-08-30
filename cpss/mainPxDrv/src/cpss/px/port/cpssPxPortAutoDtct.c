/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxPortAutoDtct.c
*
* @brief CPSS implementation for Port interface mode and speed autodetection
* facility.
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
/*#define CPSS_LOG_IN_MODULE_ENABLE*/

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyEomIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmEomIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>


/* #define AUTODETECT_DBG */
#ifdef AUTODETECT_DBG
#define AUTODETECT_DBG_PRINT_MAC(x)     cpssOsPrintf x
#else
#define AUTODETECT_DBG_PRINT_MAC(x)
#endif


/**
* @internal prvCpssPxAutoNegMasterModeSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation code word.
*         When set, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE   -  SW to define Auto-Negotiation code word
*                                      GT_FALSE  - ASIC defines Auto-Negotiation code word
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Master mode can be enabled on SGMII as well as
*       1000BASE-X. For normal operation, this bit must be cleared.
*
*/
static GT_STATUS prvCpssPxAutoNegMasterModeSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.
                                        perPortRegs[portNum].serdesCnfg;
    /* configure <Auto-Negotiation Master Mode Enable> */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 9, 1, BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }
    /* configure Force Link on Master Mode - This bit should be set to 1 when
        <Auto-Negotiation Master Mode Enable> is set to 1. The link is taken
        according to <TX Config Reg>[15]. */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 11, 1, BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal internal_cpssPxPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_VALUE             - on wrong speed value in the register
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS internal_cpssPxPortAutoNegAdvertismentConfigGet
(
    IN   GT_SW_DEV_NUM                            devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC    *portAnAdvertismentPtr
)
{
    GT_STATUS                       rc;         /* return code */
    GT_U32                          regAddr;    /* register address */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;     /* port interface */
    CPSS_PORT_SPEED_ENT             speed;      /* port speed */
    GT_U32                          data;       /* data from register */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    CPSS_NULL_PTR_CHECK_MAC(portAnAdvertismentPtr);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
        (speed != CPSS_PORT_SPEED_1000_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacMap].serdesCnfg2;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    if((data & 0x8000) > 0)
    {
        portAnAdvertismentPtr->link = GT_TRUE;
    }
    else
    {
        portAnAdvertismentPtr->link = GT_FALSE;
    }
    if((data & 0x1000) > 0)
    {
        portAnAdvertismentPtr->duplex = CPSS_PORT_FULL_DUPLEX_E;
    }
    else
    {
        portAnAdvertismentPtr->duplex = CPSS_PORT_HALF_DUPLEX_E;
    }
    switch(data & 0xC00)
    {
    case 0x800:
        portAnAdvertismentPtr->speed = CPSS_PORT_SPEED_1000_E;
        break;
    case 0x400:
        portAnAdvertismentPtr->speed = CPSS_PORT_SPEED_100_E;
        break;
    case 0x000:
        portAnAdvertismentPtr->speed = CPSS_PORT_SPEED_10_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        break;
    }
    return GT_OK;
}

/**
* @internal cpssPxPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_VALUE             - on wrong speed value in the register
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegAdvertismentConfigGet
(
    IN   GT_SW_DEV_NUM                            devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC   *portAnAdvertismentPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortAutoNegAdvertismentConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portAnAdvertismentPtr));

    rc = internal_cpssPxPortAutoNegAdvertismentConfigGet(devNum, portNum, portAnAdvertismentPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portAnAdvertismentPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
* @param[in] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS internal_cpssPxPortAutoNegAdvertismentConfigSet
(
    IN   GT_SW_DEV_NUM                          devNum,
    IN   GT_PHYSICAL_PORT_NUM                   portNum,
    IN   CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC  *portAnAdvertismentPtr
)
{
    GT_STATUS                       rc;            /* return code */
    GT_U32                          regAddr;       /* register address */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;        /* port interface */
    CPSS_PORT_SPEED_ENT             speed;         /* port speed */
    GT_U32                          data = 0x4001; /* advertisment data */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);


    CPSS_NULL_PTR_CHECK_MAC(portAnAdvertismentPtr);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
        (speed != CPSS_PORT_SPEED_1000_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacMap].serdesCnfg2;
    /* configure <Auto-Negotiation Master Mode Enable> */
    if(GT_TRUE == portAnAdvertismentPtr->link)
    {
        data |= 0x8000;
    }
    if(CPSS_PORT_FULL_DUPLEX_E == portAnAdvertismentPtr->duplex)
    {
        data |= 0x1000;
    }
    switch(portAnAdvertismentPtr->speed)
    {
    case CPSS_PORT_SPEED_1000_E:
        data |= 0x800;
        break;
    case CPSS_PORT_SPEED_100_E:
        data |= 0x400;
        break;
    case CPSS_PORT_SPEED_10_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* perform MAC RESET */
    rc = prvCpssPxPortMacResetStateSet(devNum, portNum, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortMacResetStateSet: error for portNum = %d\n", portNum);
    }
    /* perform PCS RESET */
    rc = cpssPxPortPcsResetSet(devNum, portNum, CPSS_PORT_PCS_RESET_MODE_ALL_E, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortPcsResetSet: error for portNum = %d\n", portNum);
    }

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, data);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* perform PCS UNRESET */
    rc = cpssPxPortPcsResetSet(devNum, portNum, CPSS_PORT_PCS_RESET_MODE_ALL_E, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortPcsResetSet: error for portNum = %d\n", portNum);
    }

    /* perform MAC UNRESET */
    rc = prvCpssPxPortMacResetStateSet(devNum, portNum, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortMacResetStateSet: error for portNum = %d\n", portNum);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegAdvertismentConfigSet
(
    IN   GT_SW_DEV_NUM                            devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    IN   CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC    *portAnAdvertismentPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortAutoNegAdvertismentConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portAnAdvertismentPtr));

    rc = internal_cpssPxPortAutoNegAdvertismentConfigSet(devNum, portNum, portAnAdvertismentPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portAnAdvertismentPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      enable  - GT_TRUE   - enable Auto-Negotiation Master Mode and code word
*                                      GT_FALSE  - device defines Auto-Negotiation code word
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
static GT_STATUS internal_cpssPxPortAutoNegMasterModeEnableGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
)
{
    GT_STATUS                       rc;         /* return code */
    GT_U32                          regAddr;    /* register address */
    GT_U32                          bitValue;   /* bit value        */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;     /* port interface */
    CPSS_PORT_SPEED_ENT             speed;      /* port speed */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
        (speed != CPSS_PORT_SPEED_1000_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacMap].serdesCnfg;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 11, 1, &bitValue);
    if (rc != GT_OK)
    {
       return rc;
    }
    *enablePtr = BIT2BOOL_MAC(bitValue);
    if(GT_TRUE == *enablePtr)
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.
                                                perPortRegs[portMacMap].serdesCnfg;
        rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 9, 1, &bitValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        *enablePtr = BIT2BOOL_MAC(bitValue);
    }
    return GT_OK;
}


/**
* @internal cpssPxPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegMasterModeEnableGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortAutoNegMasterModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortAutoNegMasterModeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE   -  Auto-Negotiation Master Mode and code word
*                                      GT_FALSE  - device defines Auto-Negotiation code word
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
static GT_STATUS internal_cpssPxPortAutoNegMasterModeEnableSet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_BOOL                  enable
)
{
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;     /* port interface */
    CPSS_PORT_SPEED_ENT             speed;      /* port speed */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */
    GT_STATUS                       rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
        (speed != CPSS_PORT_SPEED_1000_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssPxAutoNegMasterModeSet(devNum, portMacMap, enable);
    if(GT_OK != rc)
    {
        return rc;
    }
    if(GT_TRUE == enable)
    {
        rc = cpssPxPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
        rc = cpssPxPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
        rc = cpssPxPortFlowControlEnableSet(devNum, portNum, CPSS_PORT_FLOW_CONTROL_DISABLE_E);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  Pipe.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE   -  Auto-Negotiation Master Mode and code word
*                                      GT_FALSE  - device defines Auto-Negotiation code word
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssPxPortAutoNegMasterModeEnableSet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortAutoNegMasterModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortAutoNegMasterModeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortInbandAutoNegRestart function
* @endinternal
*
* @brief   Restart inband auto-negotiation. Relevant only when inband auto-neg.
*         enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortInbandAutoNegRestart
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return code */
    GT_U32      portMacNum;         /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 4, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Note: minimum time before switch bit off 15 ns */

    /* due to FE-21290 - InBandReStartAn bit does not reset automatically,
       so switch it off by SW  */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 4, 1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortInbandAutoNegRestart function
* @endinternal
*
* @brief   Restart inband auto-negotiation. Relevant only when inband auto-neg.
*         enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInbandAutoNegRestart
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInbandAutoNegRestart);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssPxPortInbandAutoNegRestart(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
