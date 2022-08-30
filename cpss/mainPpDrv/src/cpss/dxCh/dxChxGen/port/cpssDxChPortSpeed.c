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
* @file cpssDxChPortSpeed.c
*
* @brief CPSS implementation for Port speed configuration.
*
* The following APIs can run now "preliminary stage" and "post stage"
* callbacks, if the bind with port MAC object pointer engaged:
* - cpssDxChPortSpeedSet;
* - cpssDxChPortSpeedGet;
*
* @version   62
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSpeed.h>

/* TM glue */
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_CPSS_DXCH_PORT_SPEED_2_HW_VALUE_CONVERT(_speed, _speed_hw) \
    switch(_speed){                                                    \
        case CPSS_PORT_SPEED_10_E:                                     \
            _speed_hw = 10;                                            \
            break;                                                     \
        case CPSS_PORT_SPEED_100_E:                                    \
            _speed_hw = 100;                                           \
            break;                                                     \
        case CPSS_PORT_SPEED_1000_E:                                   \
            _speed_hw = 1000;                                          \
            break;                                                     \
        case CPSS_PORT_SPEED_2500_E:                                   \
            _speed_hw = 2500;                                          \
            break;                                                     \
        case CPSS_PORT_SPEED_5000_E:                                   \
            _speed_hw = 5000;                                          \
            break;                                                     \
        case CPSS_PORT_SPEED_10000_E:                                  \
            _speed_hw = 10000;                                         \
            break;                                                     \
        default:                                                       \
            _speed_hw = 0;                                             \
            break;                                                     \
    }

#define PRV_CPSS_DXCH_PORT_HW_VALUE_2_SPEED_CONVERT(_speed_hw, _speed) \
    switch(_speed_hw){                                                 \
        case 10:                                                       \
            _speed = CPSS_PORT_SPEED_10_E;                             \
            break;                                                     \
        case 100:                                                      \
            _speed = CPSS_PORT_SPEED_100_E;                            \
            break;                                                     \
        case 1000:                                                     \
            _speed = CPSS_PORT_SPEED_1000_E;                           \
            break;                                                     \
        case 2500:                                                     \
            _speed = CPSS_PORT_SPEED_2500_E;                           \
            break;                                                     \
        case 5000:                                                     \
            _speed = CPSS_PORT_SPEED_5000_E;                           \
            break;                                                     \
        case 10000:                                                    \
            _speed = CPSS_PORT_SPEED_10000_E;                          \
            break;                                                     \
        default:                                                       \
            _speed = CPSS_PORT_SPEED_NA_E;                             \
            break;                                                     \
    }

/* array defining serdes speed used in given interface mode for given port data speed */
/* APPLICABLE DEVICES:  DxChXcat and above. */
extern CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E];

extern PRV_CPSS_LION2_INT_CAUSE_ENT lion2PortUnitHglSumIndex
(
    IN GT_PHYSICAL_PORT_NUM localPort
);

/* array for translation of given interface mode and serdes speed of flex link to data speed of port */
/* APPLICABLE DEVICES:  Xcat, Xcat2. */
static const CPSS_PORT_SPEED_ENT ifModeSerdesToPortSpeed[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_DXCH_PORT_SERDES_SPEED_NA_E] =
{                                               /* 1.25G                  3.125G                      3.75G                       6.25G                         5G                          4.25G                       2.5G                  5.156G  */
/* CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E  */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_MII_E           */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_SGMII_E         */  {CPSS_PORT_SPEED_1000_E , CPSS_PORT_SPEED_2500_E    , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_XGMII_E         */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_10000_E   , CPSS_PORT_SPEED_12000_E   , CPSS_PORT_SPEED_20000_E   , CPSS_PORT_SPEED_16000_E   , CPSS_PORT_SPEED_13600_E   , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_MGMII_E         */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,   */  {CPSS_PORT_SPEED_1000_E , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_GMII_E,         */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_MII_PHY_E,      */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_QX_E,           */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_2500_E    , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_5000_E    , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_HX_E,           */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_5000_E    , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_10000_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_RXAUI_E,        */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_10000_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,   */  {CPSS_PORT_SPEED_1000_E , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_QSGMII_E,       */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_1000_E    , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    },
/* CPSS_PORT_INTERFACE_MODE_XLG_E,          */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E      , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_40000_E },
/* CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E   */  {CPSS_PORT_SPEED_NA_E   , CPSS_PORT_SPEED_10000_E   , CPSS_PORT_SPEED_12000_E   , CPSS_PORT_SPEED_20000_E   , CPSS_PORT_SPEED_16000_E   , CPSS_PORT_SPEED_13600_E   , CPSS_PORT_SPEED_NA_E, CPSS_PORT_SPEED_NA_E    }
};

/* matrix [serdes speed][serdes ref clock] = reg value */
static const GT_U32 speedRegValue[4][3] =
{{0x205C,0x200C,0   }, /* 1.25G */
 {0     ,0x245E,0x2421}, /* 3.125G */
 {0     ,0x245D,0x2420}, /* 6.25G */
 {0x255C,0x2435,0x2410}  /* 3.75G */
};

/**
* @internal prvCpssDxChPortSpeedForCutThroughWaCalc function
* @endinternal
*
* @brief   Calculate port speed HW value and field offsets in RX and TX DMAs
*         for Cut Throw WA - packet from slow to fast port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] localPortNum             - local port number
* @param[in] speed                    - port speed
*
* @param[out] hwSpeedPtr               - pointer to 2-bit HW speed value.
* @param[out] rxRegAddrPtr             - pointer to address of relevant RX DMA register.
* @param[out] txRegAddrPtr             - pointer to address of relevant TX DMA register.
* @param[out] rxRegOffPtr              - pointer to bit offset of field in relevant RX DMA register.
* @param[out] txRegOffPtr              - pointer to bit offset of field in relevant TX DMA register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPortSpeedForCutThroughWaCalc
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM localPortNum,
    IN  CPSS_PORT_SPEED_ENT  speed,
    OUT GT_U32               *hwSpeedPtr,
    OUT GT_U32               *rxRegAddrPtr,
    OUT GT_U32               *txRegAddrPtr,
    OUT GT_U32               *rxRegOffPtr,
    OUT GT_U32               *txRegOffPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    if (PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(devNum) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(hwSpeedPtr);
    CPSS_NULL_PTR_CHECK_MAC(rxRegOffPtr);
    CPSS_NULL_PTR_CHECK_MAC(txRegOffPtr);
    if (localPortNum >= 12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (speed)
    {
        /* three speed GIGE */
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            *hwSpeedPtr = 0;
            break;
        case CPSS_PORT_SPEED_10000_E:
            *hwSpeedPtr = 1;
            break;
        case CPSS_PORT_SPEED_20000_E:
            *hwSpeedPtr = 2;
            break;
        case CPSS_PORT_SPEED_40000_E:
            *hwSpeedPtr = 3;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* RXDMA DFX metal fix register */
    *rxRegAddrPtr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaDfxMetalFix;

    /* TXDMA Buffer memory clear type register */
    *txRegAddrPtr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBufMemClearType;

    *rxRegOffPtr  = (2 * localPortNum) + 2;
    *txRegOffPtr  = (2 * localPortNum);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMacSerdesSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*/
static GT_STATUS prvCpssDxChPortMacSerdesSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;   /* return code */
    PRV_CPSS_DXCH_PORT_SPEED_SET_FUN speedSetFuncPtr; /* ptr to speed set function per devFamily
                                                            per port options */
    GT_PHYSICAL_PORT_NUM    portMacNum; /* MAC to which mapped current physical port */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacNum);

    if((!PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
       && (CPSS_CPU_PORT_NUM_CNS == portNum))
    {
        speedSetFuncPtr = PORT_OBJ_FUNC(devNum).
                        setPortSpeed[PRV_CPSS_GE_PORT_GE_ONLY_E];
    }
    else
    {
        speedSetFuncPtr = PORT_OBJ_FUNC(devNum).
                        setPortSpeed[PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portMacNum)];
    }

    if(speedSetFuncPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* configure requested speed on port */
    rc = speedSetFuncPtr(devNum, portNum, speed);

    return rc;
}

/**
* @internal prvCpssDxChPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssDxChPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*       4.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvCpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    CPSS_PORT_SPEED_ENT  origSpeed = speed;     /* speed to configure on port */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;        /* port callback pointer */
    GT_STATUS rc = GT_OK;                       /* return code */
    GT_BOOL doPpMacConfig = GT_TRUE;            /* run switch mac code indicator */
    GT_U32               portGroupId;           /* Port Group Id */
    GT_U32               localPort;             /* Local Port Num */
    GT_U32               hwSpeed;               /* Cut Through WA HW port speed value */
    GT_U32               rxRegAddr;             /* Cut Through WA RX DMA register address */
    GT_U32               txRegAddr;             /* Cut Through WA TX DMA register address */
    GT_U32               rxRegOff;              /* Cut Through WA bit offset in RX DMA register */
    GT_U32               txRegOff;              /* Cut Through WA bit offset in TX DMA register */
    GT_U32               portMacNum;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* system recovery info*/

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        && (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable))
    {/* do only if we during recovery */
        CPSS_PORT_SPEED_ENT   currentSpeed;

        rc = prvCpssDxChPortSpeedGet(devNum, portNum, &currentSpeed);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(speed == currentSpeed)
        {
            return GT_OK;
        }
    }

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacNum);

    /* Get PHY MAC object ptr
       if it is CPU port or the pointer is NULL (no bind) - return NULL */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run callback*/
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacSpeedSetFunc(devNum,portNum,
                                  origSpeed,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,&origSpeed);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* doPpMacConfig value defined by callback; if it is TRUE -
       run prvCpssDxChPortSpeedSet that means do all required
       confiruration actions for switch MAC */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortMacSerdesSpeedSet(devNum,portNum,origSpeed);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* if required the post stage callback can run*/
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacSpeedSetFunc(devNum,portNum,
                                       origSpeed,CPSS_MACDRV_STAGE_POST_E,
                                       &doPpMacConfig,&origSpeed);
        if(rc!=GT_OK)
            {
            return rc;
        }
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_LION2_CUT_THROUGH_SLOW_TO_FAST_WA_E.
            enabled == GT_TRUE)
    {
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

        rc = prvCpssDxChPortSpeedForCutThroughWaCalc(
            devNum, localPort, speed,
            &hwSpeed, &rxRegAddr, &txRegAddr, &rxRegOff, &txRegOff);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* RX DMA register */
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, portGroupId, rxRegAddr, rxRegOff, 2 /*fieldLength*/, hwSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* TX DMA register */
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, portGroupId, txRegAddr, txRegOff, 2 /*fieldLength*/, hwSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal internal_cpssDxChPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssDxChPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*       4.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
static GT_STATUS internal_cpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_PHYSICAL_PORT_NUM portMacNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacNum);

    if(PRV_CPSS_PORT_NOT_EXISTS_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* check input parameter */
    if (speed >= CPSS_PORT_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChPortSpeedSet(devNum, portNum, speed);
}

/**
* @internal cpssDxChPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssDxChPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*       4.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSpeedSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speed));

    rc = internal_cpssDxChPortSpeedSet(devNum, portNum, speed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal geMacUnitSpeedSet function
* @endinternal
*
* @brief   Configure GE MAC unit of specific device and port to required speed
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong speed
*/
GT_STATUS geMacUnitSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;               /* return code */
    PRV_CPSS_DXCH_PORT_STATE_STC    portStateStc;        /* current port state */
    GT_U32    gmiiOffset;       /* gmii speed bit offset */
    GT_U32    miiOffset;        /* mii speed bit offset */
    GT_U32    portSpeedRegAddr; /* address of GE port speed register */
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */

    GT_U32 portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    /* disable port if we need */
    rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
    if(rc != GT_OK)
        return rc;

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&portSpeedRegAddr);

    gmiiOffset = 6;
    miiOffset  = 5;

    switch (speed)
    {
        case CPSS_PORT_SPEED_10_E:
            if(prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, portSpeedRegAddr, gmiiOffset, 1, 0) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, portSpeedRegAddr, miiOffset, 1, 0);
            if(rc != GT_OK)
                return rc;
            break;

        case CPSS_PORT_SPEED_100_E:
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, portSpeedRegAddr, gmiiOffset, 1, 0) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, portSpeedRegAddr, miiOffset, 1, 1);
            if(rc != GT_OK)
                return rc;
            break;

        case CPSS_PORT_SPEED_1000_E:
        case CPSS_PORT_SPEED_2500_E:
            if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_GE_E)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, portSpeedRegAddr, gmiiOffset, 1, 1);
                if(rc != GT_OK)
                    return rc;

                if((PRV_CPSS_PP_MAC(devNum)->devFamily > CPSS_PP_FAMILY_CHEETAH3_E)
                    && ((CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum))
                        || (CPSS_PORT_SPEED_2500_E == speed)))
                {
                    GT_BOOL linkDownStatus;

                    rc = prvCpssDxChPortForceLinkDownEnable(devNum,portNum,&linkDownStatus);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* configure 1000BaseX Port type, although the
                        interface mode called SGMII from historic reasons */
                    rc = prvCpssDxChPortGePortTypeSet(devNum, portNum,
                                                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = prvCpssDxChPortInbandAutonegMode(devNum,portNum, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* usually it's application's decision enable/disable inband auto-neg.,
                        but to make 1000BaseX and 2.5G establish link it must be enabled */
                    rc = cpssDxChPortInbandAutoNegEnableSet(devNum,portNum,GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = prvCpssDxChPortForceLinkDownDisable(devNum,portNum,linkDownStatus);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }

                break;
            }
            /* else
                continue toward failure */
            GT_ATTR_FALLTHROUGH;

        default:
            /* enable port if we need */
            rc = prvCpssDxChPortStateRestore(devNum,portNum,&portStateStc);
            if(rc != GT_OK)
                return rc;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); /* GT_BAD_PARAM; */
    }

    /* enable port if we need */
    rc = prvCpssDxChPortStateRestore(devNum,portNum,&portStateStc);

    return rc;
}

/**
* @internal prvCpssDxChLion2PortTypeSet function
* @endinternal
*
* @brief   Sets port type (mostly means which mac unit used) on a specified port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - Interface mode.
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the speed is not supported on the port
*/
GT_STATUS prvCpssDxChLion2PortTypeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc; /* return code */
    GT_BOOL     noExtendedMac = GT_FALSE;/* if device has just 3 mini-GOP's */
    GT_PHYSICAL_PORT_NUM localPort;   /* number of port in port group */

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if((9 == localPort) || (11 == localPort))
    {
        switch(PRV_CPSS_PP_MAC(devNum)->devType)
        {
            case CPSS_LION_2_THREE_MINI_GOPS_DEVICES_CASES_MAC:
                noExtendedMac = GT_TRUE;
                break;
            case CPSS_LION_2_FOUR_MINI_GOPS_DEVICES_CASES_MAC:
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_GE_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            if((speed != CPSS_PORT_SPEED_10000_E) && (speed != CPSS_PORT_SPEED_20000_E)
                && (speed != CPSS_PORT_SPEED_12000_E) && (speed != CPSS_PORT_SPEED_16000_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            if(speed != CPSS_PORT_SPEED_1000_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_GE_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_HX_E:
            if(speed != CPSS_PORT_SPEED_10000_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            if(speed != CPSS_PORT_SPEED_10000_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E:
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10000_E:
                    if(CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode )
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                    break;
                case CPSS_PORT_SPEED_20000_E:
                    if(noExtendedMac || CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode )
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    if(noExtendedMac)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    if(CPSS_PORT_INTERFACE_MODE_KR4_E != ifMode || CPSS_PORT_INTERFACE_MODE_CR4_E != ifMode)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }

            if(CPSS_PORT_SPEED_10000_E == speed)
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XG_E;
            }
            else
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_HGL_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_15000_E:
                case CPSS_PORT_SPEED_16000_E:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType =
                        PRV_CPSS_PORT_HGL_E;
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    /* 40G is XLHGL supported by XLG MAC */
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_CHGL_12_E:
            if(speed != CPSS_PORT_SPEED_100G_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            break;
        case CPSS_PORT_INTERFACE_MODE_XHGS_E:
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
            switch (speed)
            {
                case CPSS_PORT_SPEED_11800_E:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XG_E;
                break;
                case CPSS_PORT_SPEED_47200_E:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XLG_E;
                break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_NA_E:
            /* when port is powered down must set it to XG not XLG(!) to allow
                neighbour ports be configured. NA - used only for loops,
                NOT_EXIST - means port physically not exists */
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = PRV_CPSS_PORT_XG_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType)
    {
        case PRV_CPSS_PORT_GE_E:
            rc = prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet(devNum,
                                portNum, CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PRV_CPSS_PORT_XG_E:
        case PRV_CPSS_PORT_XLG_E:
        case PRV_CPSS_PORT_HGL_E:
            rc = prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet(devNum,
                                portNum, CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


GT_STATUS bcat2MacGESpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;                        /* return code */
    GT_PHYSICAL_PORT_NUM portMacNum;    /* MAC to which port mapped */
    GT_U32    portSpeedRegAddr; /* address of GE port speed register */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,  portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) != PRV_CPSS_PORT_GE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &portSpeedRegAddr);

    switch (speed)
    {
        case CPSS_PORT_SPEED_10_E:
            rc = prvCpssDrvHwPpSetRegField(devNum, portSpeedRegAddr, 5, 2, 0);
            break;

        case CPSS_PORT_SPEED_100_E:
            rc = prvCpssDrvHwPpSetRegField(devNum, portSpeedRegAddr, 5, 2, 1);
            break;

        case CPSS_PORT_SPEED_1000_E:
            rc = prvCpssDrvHwPpSetRegField(devNum, portSpeedRegAddr, 6, 1, 1);
            break;

        default:
            rc = GT_BAD_PARAM;
            break;
    }

    if(GT_OK == rc)
    {
        /* save new port speed in DB */
        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum) = speed;
    }
    return rc;
}

/**
* @internal bcat2SpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified bcat2 device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the speed is not supported on the port
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS bcat2SpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc, rc1;                        /* return code */
    GT_PHYSICAL_PORT_NUM portMacNum;    /* MAC to which port mapped */
    PRV_CPSS_DXCH_PORT_STATE_STC    portStateStc;        /* current port state (enable/disable) */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,  portMacNum);

    if((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) &&
       (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) != PRV_CPSS_PORT_GE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* disable port if we need */
    rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = bcat2MacGESpeedSet(devNum,portNum,speed);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable port if we need */
    rc1 = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
    if(rc1 != GT_OK)
    {
        return rc1;
    }

    return rc;
}

/**
* @internal lion2SpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified xcat/xcat2 device.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the speed is not supported on the port
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS lion2SpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;                        /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current i/f mode of port */
    GT_U32               portGroup; /* number of local core */
    GT_PHYSICAL_PORT_NUM localPort;   /* number of port in port group */
    PRV_CPSS_PORT_TYPE_ENT originalPortType; /* keep original port type to restore on failure */
    MV_HWS_PORT_STANDARD portMode;  /* port interface in hwServices format */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32               macNum;     /* number of MAC used by port for given ifMode */
    GT_U8                sdVecSize;  /* size of serdes redundancy array */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if(CPSS_CPU_PORT_NUM_CNS == portNum)
    {
        return geMacUnitSpeedSet(devNum,portNum,speed);
    }

    if(CPSS_NULL_PORT_NUM_CNS == portNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPizzaArbiterIfCheckSupport(devNum,portNum,speed);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* save original port type to restore it if new configuration fails */
    originalPortType = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType;

    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
    rc = prvCpssDxChLion2PortTypeSet(devNum,portNum,ifMode,speed);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    portGroup = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = originalPortType;
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    macNum      = curPortParams.portMacNumber;
    sdVecSize   = curPortParams.numOfActLanes;

    if((macNum%2 != 0) && (sdVecSize > 1))
    {/* 1 serdes modes already checked */
        MV_HWS_PORT_INIT_PARAMS tmpPortParams;
        GT_U32 localPortTmp;
        GT_U8  sdVecSizeTmp;
        GT_PHYSICAL_PORT_NUM firstInCouple;

        firstInCouple = portNum-1;
        /* if even neighbor port not configured check not needed */
        if((PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,firstInCouple) !=
                                            CPSS_PORT_INTERFACE_MODE_NA_E)
           && (PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,firstInCouple) !=
                                                        CPSS_PORT_SPEED_NA_E))
        {
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,firstInCouple),
                        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,firstInCouple),
                        &portMode);
            if(rc != GT_OK)
                return rc;

            localPortTmp = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,
                                                            firstInCouple);
            if(localPortTmp >= 12)
            {/* to prevent coverity warning OVERRUN_STATIC */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPortTmp, portMode, &tmpPortParams);
            if (GT_OK != rc)
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = originalPortType;
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            sdVecSizeTmp = tmpPortParams.numOfActLanes;

            if(sdVecSizeTmp >= 2)
            {/* i.e. current port's serdeses occupied by its couple even port */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = originalPortType;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    if((macNum%4 != 0) && (sdVecSize > 1))
    {/* 1 serdes modes already checked */
        MV_HWS_PORT_INIT_PARAMS tmpPortParams;
        GT_U32 localPortTmp;
        GT_U8  sdVecSizeTmp;
        GT_PHYSICAL_PORT_NUM firstInQuadruplet;

        firstInQuadruplet = (macNum != 14) ? (portNum&0xFFFFFFFC) :
                                            (portNum-2) /* i.e. local port 9 */;
        /* if first port of quadruplet not configured check not needed */
        if((PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,firstInQuadruplet) !=
                                            CPSS_PORT_INTERFACE_MODE_NA_E)
           && (PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,firstInQuadruplet) !=
                                                        CPSS_PORT_SPEED_NA_E))
        {
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,firstInQuadruplet),
                        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,firstInQuadruplet),
                        &portMode);
            if(rc != GT_OK)
                return rc;

            localPortTmp = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,
                                                            firstInQuadruplet);
            if(localPortTmp >= 12)
            {/* to prevent coverity warning OVERRUN_STATIC */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPortTmp, portMode, &tmpPortParams);
            if (GT_OK != rc)
            {
                return rc;
            }
            sdVecSizeTmp = tmpPortParams.numOfActLanes;

            if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
            {
                GT_U32 sliceNumUsed; /* number od pizza slices used by neighbor port */

                rc = cpssDxChPortPizzaArbiterIfPortStateGet(devNum, portGroup,
                                                            localPortTmp, &sliceNumUsed);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if(sliceNumUsed > 2)
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = originalPortType;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
            else if(sdVecSizeTmp >= 4)
            {/* i.e. current port's serdeses occupied by first port in its quadruplet */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType = originalPortType;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
    rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum, ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_LION2_HGL_MAC_INT_WA_E))
       && (PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->ppEventsCfg[portGroup].intVecNum !=
           CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS))
    {/* mask the HGL MAC interrupts if the portInterface != HGL*/

        /* unmask - i.e. event enable.                */
        /* prvCpssDrvEventsMask may return not GT_OK  */
        /* if interrupt was not binded by application */
        /* or interrupts are not used.                */
        /* Need to ignore this fail.                  */
        (GT_VOID)prvCpssDrvEventsMask(devNum,
            lion2PortUnitHglSumIndex(localPort),
            ((ifMode != CPSS_PORT_INTERFACE_MODE_HGL_E)
             || (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType !=
                        PRV_CPSS_PORT_HGL_E)));
    }

    /* save new port speed in DB */
    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum) = speed;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSpeedGet function
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
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS prvCpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS rc;   /* return code */
    PRV_CPSS_DXCH_PORT_SPEED_GET_FUN speedGetFuncPtr; /* pointer to
            per device_family function returning current port speed */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);

    if((speedGetFuncPtr = PORT_OBJ_FUNC(devNum).getPortSpeed) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = speedGetFuncPtr(devNum, portNum, speedPtr);

    return rc;
}

/**
* @internal flexLinkSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified flex link on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
static GT_STATUS flexLinkSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS   rc; /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* interface mode */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed;    /* current serdes frequency */
    GT_U32      serdes; /* serdes number */
    GT_U32      serdesNum;  /* serdes quantity */
    GT_U32      refClockId; /* reference clock */
    GT_U32      regValue;   /* register value */
    GT_U32      portGroupId;    /* port group Id for multi-port-group support */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacNum; /* MAC number connected to port */
    GT_BOOL     apEnable;
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacNum);
    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* NA interface */
    if (ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        *speedPtr = CPSS_PORT_SPEED_NA_E;
        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_HCD_E)
            *speedPtr = CPSS_PORT_SPEED_NA_HCD_E;
        return GT_OK;
    }

    /* NA interface */

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                             portMacNum);

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        || (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum)))
    {
        *speedPtr = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
        {
            return GT_OK;
        }
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        if((rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode, &serdes,
                                                    &serdesNum)) != GT_OK)
        {
            return rc;
        }

        rc = prvGetLpSerdesSpeed(devNum,portGroupId,serdes,&serdesSpeed);
        if (rc != GT_OK)
        {
            if ((GT_NOT_SUPPORTED == rc) || (GT_NOT_INITIALIZED == rc))
            {/* if port/serdes doesn't support speed parameter don't fail the function */
             /* or serdes was not initialized */
                *speedPtr = CPSS_PORT_SPEED_NA_E;
                return GT_OK;
            }
            else
                return rc;
        }
        *speedPtr = ifModeSerdesToPortSpeed[ifMode][serdesSpeed];
    }
    else /* xCat A0 */
    {
        switch (PRV_CPSS_DXCH_PP_MAC(devNum)->port.serdesRefClock)
        {
            case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E:
                refClockId = 0;
                break;
            case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E:
            case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E:
            case CPSS_DXCH_PP_SERDES_REF_CLOCK_INTERNAL_125_E:
                refClockId = 1;
                break;
            case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E:
            case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E:
                refClockId = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        /* Get SERDES speed configuration */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        macRegs.perPortRegs[portNum].serdesSpeed1[0];
        if (prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regValue) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
        if (regValue == speedRegValue[0][refClockId])
        {
            serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
        }
        else if (regValue == speedRegValue[1][refClockId])
        {
            serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_125_E;
        }
        else if (regValue == speedRegValue[2][refClockId])
        {
            serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_6_25_E;
        }
        else if (regValue == speedRegValue[3][refClockId])
        {
            serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_75_E;
        }
        else
        {
            serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_NA_E;
            *speedPtr = CPSS_PORT_SPEED_NA_E;
            return GT_OK;
        }

        *speedPtr = ifModeSerdesToPortSpeed[ifMode][serdesSpeed];
    }

    if ( *speedPtr <= CPSS_PORT_SPEED_1000_E )
    {
        rc = cpssDxChPortApPortEnableGet(devNum,portNum,&apEnable);
        if (rc != GT_OK)
        {
           apEnable = GT_FALSE;
        }
        if (!apEnable)
        {
            /* for all types of ASIC's for FE/GE ports get speed from HW for case
            auto-negotiation is enabled */
            PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &regAddr);
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 1,
                                                    2, &regValue);
            if (rc != GT_OK)
            {
                return rc;
            }

            if ((regValue & 0x1) == 0)
            {
                *speedPtr = ((regValue >> 1) == 1) ? CPSS_PORT_SPEED_100_E:
                    CPSS_PORT_SPEED_10_E;
            }
            else
            {
                *speedPtr = CPSS_PORT_SPEED_1000_E;
            }
        }
    }

    return GT_OK;
}

/**
* @internal xcatPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified flex link on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS xcatPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS   rc;   /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register value */
    GT_U32      startSerdes;/* number of first serdes used by port */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT  serdesSpeed;   /* speed of serdes */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if(CPSS_CPU_PORT_NUM_CNS == portNum)
    {
        *speedPtr = CPSS_PORT_SPEED_1000_E;
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) == GT_TRUE)
    {
        rc = flexLinkSpeedGet(devNum,portNum,speedPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portNum,&regAddr);

        /* read GMII Speed */
        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr, 1, 1, &value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        if (value == 1)
        {
            if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_FE_E)
            {/* MAC of FE port of xcat can be configured to 1G speed, but there won't be
                traffic because lack of synchronization with lower layer */
                *speedPtr = CPSS_PORT_SPEED_1000_E;
                return GT_OK;
            }

            startSerdes = portNum/4;
            rc = prvGetLpSerdesSpeed(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,startSerdes,&serdesSpeed);
            if (GT_NOT_INITIALIZED == rc)
            {
                /* or serdes was not initialized */
                *speedPtr = CPSS_PORT_SPEED_NA_E;
                return GT_OK;
            }
            else if (rc != GT_OK)
            {
                return rc;
            }
            /* can't determine serdes speed */
            if (serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_NA_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            else
            {
                switch(serdesSpeed)
                {
                    case CPSS_DXCH_PORT_SERDES_SPEED_1_25_E: /* sgmii */
                    case CPSS_DXCH_PORT_SERDES_SPEED_5_E:    /* qsgmii */
                        *speedPtr = CPSS_PORT_SPEED_1000_E;
                    break;

                    case CPSS_DXCH_PORT_SERDES_SPEED_3_125_E: /* sgmii 2.5G */
                        *speedPtr = CPSS_PORT_SPEED_2500_E;
                    break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }
        }
        else
        {
            /* read MII Speed */
            if (prvCpssDrvHwPpPortGroupGetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                    regAddr, 2, 1, &value) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

            *speedPtr = (value == 1) ? CPSS_PORT_SPEED_100_E : CPSS_PORT_SPEED_10_E;
        }
    }

    return GT_OK;
}

/**
* @internal lionPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified flex link on specified device.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS sip6_10_PortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS                       rc;          /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;      /* interface mode */
    MV_HWS_PORT_STANDARD            portMode;    /* port interface in hwServices format */
    CPSS_PORT_SPEED_ENT             speed;       /* current port speed */
    GT_U32                          speed_hw;    /* numerical value of port speed */
    GT_U32                          portMacNum;  /* MAC number connected to port */
    GT_U32                          gmiiRep;
    GT_U32                          gmii_2_5;

    if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacNum);
    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* NA interface */
    if (ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        *speedPtr = CPSS_PORT_SPEED_NA_E;
        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_HCD_E)
            *speedPtr = CPSS_PORT_SPEED_NA_HCD_E;
        return GT_OK;
    }

    *speedPtr = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);
    speed = *speedPtr;

    /* Check if stored in DB speed should be updated accoring to
       USX autonegotiation results */

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    rc = mvHwsMtiUsxReplicationGet(devNum, 0, portMacNum, portMode, &gmiiRep, &gmii_2_5);
    if(rc == GT_NOT_SUPPORTED)
    {
        /* return speed from DB */
        return GT_OK;
    }
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if(gmii_2_5 == 1)
    {
        PRV_CPSS_DXCH_PORT_SPEED_2_HW_VALUE_CONVERT(speed, speed_hw);
        speed_hw = (speed_hw * 10) / 25;
        PRV_CPSS_DXCH_PORT_HW_VALUE_2_SPEED_CONVERT(speed_hw, speed);
        *speedPtr = speed;
        return GT_OK;
    }
    else if(gmiiRep == 1)
    {
        /*no need to update*/
        return GT_OK;
    }
    else
    {
        if(gmiiRep == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
            case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
                speed = CPSS_PORT_SPEED_1000_E;
                break;
            case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
                speed = CPSS_PORT_SPEED_2500_E;
                break;
            case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
                speed = CPSS_PORT_SPEED_5000_E;
                break;
            case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:
            case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
                speed = CPSS_PORT_SPEED_10000_E;
                break;
            default:
                break;
        }

        PRV_CPSS_DXCH_PORT_SPEED_2_HW_VALUE_CONVERT(speed, speed_hw);
        if(speed_hw == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        speed_hw = (GT_U32)(speed_hw / gmiiRep);
        PRV_CPSS_DXCH_PORT_HW_VALUE_2_SPEED_CONVERT(speed_hw, speed);
        *speedPtr = speed;
    }

    return GT_OK;
}

/**
* @internal lionPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified flex link on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS lionPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    if(!PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        if(CPSS_CPU_PORT_NUM_CNS == portNum)
        {
            *speedPtr = CPSS_PORT_SPEED_1000_E;
            return GT_OK;
        }
    }

    return flexLinkSpeedGet(devNum, portNum, speedPtr);
}

/**
* @internal internal_cpssDxChPortSpeedGet function
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
static GT_STATUS internal_cpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    CPSS_MACDRV_OBJ_STC *portMacObjPtr; /* port callback pointer */
    GT_STATUS rc = GT_OK;               /* return code */
    GT_BOOL doPpMacConfig = GT_TRUE;    /* do switch mac code indicator */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_NOT_EXISTS_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL) {
        rc = portMacObjPtr->macDrvMacSpeedGetFunc(devNum,portNum,
                                  speedPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,speedPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* if callback set doPpMacConfig as TRUE - run prvCpssDxChPortSpeedGet - switch MAC configuration code */
    if (doPpMacConfig == GT_TRUE)
    {
        rc = prvCpssDxChPortSpeedGet(devNum,portNum,speedPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* run "post stage" callback */
    if (portMacObjPtr != NULL)  {
        rc = portMacObjPtr->macDrvMacSpeedGetFunc(devNum,portNum,
                                  speedPtr,CPSS_MACDRV_STAGE_POST_E,
                                  &doPpMacConfig,speedPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortSpeedGet function
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
GT_STATUS cpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSpeedGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speedPtr));

    rc = internal_cpssDxChPortSpeedGet(devNum, portNum, speedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLion2PortSpeedHwGet function
* @endinternal
*
* @brief   Gets from HW speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS prvCpssDxChLion2PortSpeedHwGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_U32 portGroupId; /*the port group Id - support multi-port-groups device */
    GT_STATUS   rc;     /* return code */
    MV_HWS_PORT_STANDARD    portMode = NON_SUP_MODE; /* port mode in HWS format */
    GT_U32  portMacMap;
    GT_U32 localPort;   /* number of port in local core */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    if(PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portMacMap))
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, portMacMap&0xFFFFFFFC);
        rc = mvHwsPortInterfaceGet(devNum, portGroupId, portMacMap&0xFFFFFFFC, &portMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if (QSGMII == portMode)
        {
            goto speedGet;
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, localPort);
    rc = mvHwsPortInterfaceGet(devNum, portGroupId, localPort, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

speedGet:
    switch(portMode)
    {
        case _100Base_FX:
            *speedPtr = CPSS_PORT_SPEED_100_E;
            break;

        case SGMII:
        case QSGMII:
            {/* HWS doesn't support 10/100M configuration */
                GT_U32 regAddr;
                GT_U32 value;

                /* read configured speed - not status */
                PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacMap, &regAddr);
                rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 5,
                                                                            2, &value);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if ((value & 0x2) == 0)
                {
                    *speedPtr = ((value & 0x1) == 1) ? CPSS_PORT_SPEED_100_E:
                        CPSS_PORT_SPEED_10_E;
                }
                else
                {
                    *speedPtr = CPSS_PORT_SPEED_1000_E;
                }
            }

            break;

        case SGMII2_5:
            *speedPtr = CPSS_PORT_SPEED_2500_E;
            break;

        case _1000Base_X:
            *speedPtr = CPSS_PORT_SPEED_1000_E;
            break;

        case _2500Base_X:
            *speedPtr = CPSS_PORT_SPEED_2500_E;
            break;

        case _5GBaseR:
           *speedPtr = CPSS_PORT_SPEED_5000_E;
            break;

        case _10GBase_KX4:
        case _10GBase_KX2:
        case _10GBase_KR:
        case RXAUI:
        case _10GBase_SR_LR:
        case INTLKN_4Lanes_3_125G:
            *speedPtr = CPSS_PORT_SPEED_10000_E;
            break;

        case _20GBase_KX4:
        case _20GBase_KR2:
        case _20GBase_SR_LR2:
        case INTLKN_8Lanes_3_125G:
        case INTLKN_4Lanes_6_25G:
        case _20GBase_KR:
            *speedPtr = CPSS_PORT_SPEED_20000_E;
            break;

        case _40GBase_KR4:
        case _40GBase_KR2:
        case _40GBase_CR4:
        case XLHGL_KR4:
        case _40GBase_SR_LR4:
        case INTLKN_8Lanes_6_25G:
            *speedPtr = CPSS_PORT_SPEED_40000_E;
            break;

        case _100GBase_KR10:
        case _100GBase_SR10:
        case CHGL:
        case INTLKN_12Lanes_6_25G:
        case INTLKN_12Lanes_10_3125G:
        case INTLKN_12Lanes_12_5G:
        case INTLKN_16Lanes_6_25G:
        case INTLKN_16Lanes_10_3125G:
        case INTLKN_16Lanes_12_5G:
        case INTLKN_16Lanes_3_125G:
        case INTLKN_24Lanes_6_25G:
        case INTLKN_24Lanes_3_125G:
            *speedPtr = CPSS_PORT_SPEED_100G_E;
            break;

        case _12_1GBase_KR:
            *speedPtr = CPSS_PORT_SPEED_12000_E;
            break;

        case _12_5GBase_KR:
            *speedPtr = CPSS_PORT_SPEED_12500_E;
            break;

        case _24GBase_KR2:
            *speedPtr = CPSS_PORT_SPEED_23600_E;
            break;

        case _25GBase_KR2:
            *speedPtr = CPSS_PORT_SPEED_25000_E;
            break;

        case _50GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_50000_E;
            break;

        case _25GBase_KR:
        case _25GBase_KR_C:
        case _25GBase_CR:
        case _25GBase_CR_C:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
            *speedPtr = CPSS_PORT_SPEED_25000_E;
            break;

        case _26_7GBase_KR:
            *speedPtr =  CPSS_PORT_SPEED_26700_E;
            break;

        case _50GBase_CR:
        case _50GBase_KR2:
        case _50GBase_CR2:
        case _50GBase_KR2_C:
        case _50GBase_CR2_C:
        case _50GBase_KR:
        case _50GBase_SR_LR:
            *speedPtr = CPSS_PORT_SPEED_50000_E;
            break;

        case _52_5GBase_KR2:
            *speedPtr = CPSS_PORT_SPEED_52500_E;
            break;

        case _100GBase_KR4:
        case _100GBase_CR4:
        case _100GBase_KR2:
        case _100GBase_CR2:
        case _100GBase_SR_LR2:
            *speedPtr = CPSS_PORT_SPEED_100G_E;
            break;

        case _107GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_107G_E;
            break;

        case _102GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_102G_E;
            break;

        case _25GBase_SR:
            *speedPtr = CPSS_PORT_SPEED_25000_E;
            break;

        case _50GBase_SR2:
            *speedPtr = CPSS_PORT_SPEED_50000_E;
            break;

        case _100GBase_SR4:
            *speedPtr = CPSS_PORT_SPEED_100G_E;
            break;

        case _100GBase_MLG:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
                ((localPort % 2) == 0)) /* portMacNum is even */
            {
                *speedPtr = CPSS_PORT_SPEED_40000_E;
            }
            else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
                       ((localPort % 2) == 1)) /* portMacNum is odd */
            {
                *speedPtr = CPSS_PORT_SPEED_10000_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case HGL:
            *speedPtr = CPSS_PORT_SPEED_15000_E;
            break;

        case HGL16G:
            *speedPtr = CPSS_PORT_SPEED_16000_E;
            break;

        case _12GBase_SR:
            *speedPtr = CPSS_PORT_SPEED_11800_E;
            break;

        case _48GBase_SR4:
            *speedPtr = CPSS_PORT_SPEED_47200_E;
            break;

        case _22GBase_SR:
            *speedPtr = CPSS_PORT_SPEED_22000_E;
            break;

        case _29_09GBase_SR4:
            *speedPtr = CPSS_PORT_SPEED_29090_E;
            break;

        case _200GBase_KR4:
        case _200GBase_KR8:
        case _200GBase_CR4:
        case _200GBase_CR8:
        case _200GBase_SR_LR4:
        case _200GBase_SR_LR8:
            *speedPtr = CPSS_PORT_SPEED_200G_E;
            break;

        case _400GBase_CR8:
        case _400GBase_KR8:
        case _400GBase_SR_LR8:
            *speedPtr = CPSS_PORT_SPEED_400G_E;
            break;

        case _424GBase_KR8:
            *speedPtr = CPSS_PORT_SPEED_424G_E;
            break;

        case _212GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_212G_E;
            break;

        default:
            *speedPtr = CPSS_PORT_SPEED_NA_E;
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSpeedHwGet function
* @endinternal
*
* @brief   Gets from HW speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS prvCpssDxChPortSpeedHwGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regValue;   /* register value */
    GT_U32      portGroupId;    /* port group Id for multi-port-group support */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);

    *speedPtr = CPSS_PORT_SPEED_NA_E;
    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(CPSS_CPU_PORT_NUM_CNS == portMacMap)
        {
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacMap,&regAddr);

            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);

            /* read GMII Speed */
            if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,
                                                    regAddr, 6, 1, &regValue) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

            if (regValue == 1)
            {
                *speedPtr = CPSS_PORT_SPEED_1000_E;
            }
            else
            {
                /* read MII Speed */
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,
                                            regAddr, 5, 1, &regValue) != GT_OK)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

                *speedPtr = (regValue == 1) ? CPSS_PORT_SPEED_100_E :
                                                        CPSS_PORT_SPEED_10_E;
            }
            return GT_OK;
        }
    }
    else
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E == portMapShadowPtr->portMap.mappingType)
        {/* return dummy speed to support legacy behavior */
            *speedPtr = CPSS_PORT_SPEED_1000_E;
            return GT_OK;
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return prvCpssDxChLion2PortSpeedHwGet(devNum,portNum,speedPtr);
    }

    rc = prvCpssDxChLion2PortSpeedHwGet(devNum,portNum,speedPtr);

    return rc;
}

