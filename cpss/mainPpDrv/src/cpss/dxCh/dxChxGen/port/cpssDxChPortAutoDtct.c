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
* @file cpssDxChPortAutoDtct.c
*
* @brief CPSS implementation for Port interface mode and speed autodetection
* facility.
*
* @version   17
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>

#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern GT_STATUS prvCpssDxChPortLionSerdesPowerUpSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

extern GT_STATUS serDesConfig
(
    IN  GT_U8 devNum,
    OUT PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY **serdesPowerUpSequencePtr,
    OUT PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  **serdesExtCfgPtr
);

/* #define AUTODETECT_DBG */
#ifdef AUTODETECT_DBG
#define AUTODETECT_DBG_PRINT_MAC(x)     cpssOsPrintf x
#else
#define AUTODETECT_DBG_PRINT_MAC(x)
#endif

static GT_STATUS prvCpssDxChInbandAutoNegRestartAndWait
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
);


/* default array of supported by XCAT stacking ports mode/speed auto detect
    and configure options */
static CPSS_PORT_MODE_SPEED_STC xcatDefaultStackPortModeSpeedOptionsArray[] =
{
    {CPSS_PORT_INTERFACE_MODE_XGMII_E,      CPSS_PORT_SPEED_10000_E},
    {CPSS_PORT_INTERFACE_MODE_RXAUI_E,      CPSS_PORT_SPEED_10000_E},
    {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, CPSS_PORT_SPEED_1000_E},
    {CPSS_PORT_INTERFACE_MODE_SGMII_E,      CPSS_PORT_SPEED_2500_E},
    {CPSS_PORT_INTERFACE_MODE_SGMII_E,      CPSS_PORT_SPEED_1000_E}
};

/* default array of supported by XCAT network ports mode/speed auto detect
    and configure options */
static CPSS_PORT_MODE_SPEED_STC xcatDefaultNetworkPortModeSpeedOptionsArray[] =
{
    {CPSS_PORT_INTERFACE_MODE_QSGMII_E,     CPSS_PORT_SPEED_1000_E},
    {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, CPSS_PORT_SPEED_1000_E},
    {CPSS_PORT_INTERFACE_MODE_SGMII_E,      CPSS_PORT_SPEED_2500_E},
    {CPSS_PORT_INTERFACE_MODE_SGMII_E,      CPSS_PORT_SPEED_1000_E}
};

static GT_STATUS prvCpssDxChXgmii10GDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxChRxauiDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxCh1000BaseXDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxChSGMII1GDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxChQsgmiiDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxChSGMII2_5GDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxChSGMIIDetectAlgorithm
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed,
    IN  GT_BOOL inbandAutonegEn,
    OUT GT_BOOL *statusPtr
);

static GT_STATUS prvCpssDxChGeDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/* array of functions for specific port mode/speed test
    NOTE !!! must be compatible with device/port_type specific
            ...DefaultPortModeSpeedOptionsArray
*/

/* array for XCAT stacking ports */
static PRV_CPSS_PORT_MODE_SPEED_TEST_FUN xcatStackPortModeSpeedTestFunctionsArray[] =
{
    prvCpssDxChXgmii10GDetect,
    prvCpssDxChRxauiDetect,
    prvCpssDxCh1000BaseXDetect,
    prvCpssDxChSGMII2_5GDetect,
    prvCpssDxChSGMII1GDetect
};
/* array for XCAT network ports */
static PRV_CPSS_PORT_MODE_SPEED_TEST_FUN xcatNetworkPortModeSpeedTestFunctionsArray[] =
{
    prvCpssDxChQsgmiiDetect,
    prvCpssDxCh1000BaseXDetect,
    prvCpssDxChSGMII2_5GDetect,
    prvCpssDxChSGMII1GDetect
};


/**
* @internal prvCpssDxChXpcsLinkStatus function
* @endinternal
*
* @brief   The function checks XPCS of GE or XG MAC has link synchronized
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] geXpcsStatusPtr          - pointer to GE XPCS MAC status result:
*                                      GT_TRUE - synchronized,
*                                      GT_FALSE - not synchronized
* @param[out] xgXpcsStatusPtr          - pointer to XG XPCS MAC status result:
*                                      GT_TRUE - synchronized,
*                                      GT_FALSE - not synchronized
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChXpcsLinkStatus
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *geXpcsStatusPtr,
    OUT GT_BOOL *xgXpcsStatusPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into the register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    CPSS_NULL_PTR_CHECK_MAC(geXpcsStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(xgXpcsStatusPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portNum,&regAddr);
    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChXpcsLinkStatus:PORT_STATUS_CTRL_REG addr=0x%x\n", regAddr));
    /* get SynkOk bit */
    if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                14, 1, &value)) != GT_OK)
        return rc;
    AUTODETECT_DBG_PRINT_MAC(("fieldValue=%d\n", value));
    *geXpcsStatusPtr = (value == 1) ? GT_TRUE : GT_FALSE;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].xgXpcsGlobalStatus;
    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChXpcsLinkStatus:xgXpcsGlobalStatus portNum=%d,addr=0x%x\n",
                                portNum, regAddr));
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {/* current device doesn't support XG on stack ports */
        *xgXpcsStatusPtr = GT_FALSE;
        return GT_OK;
    }

    /* get LinkUP bit */
    if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 1, &value)) != GT_OK)
        return rc;
    AUTODETECT_DBG_PRINT_MAC(("fieldValue=%d\n", value));
    *xgXpcsStatusPtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal prvCpssDxChSerdesSignalDetect function
* @endinternal
*
* @brief   The function checks if signal detected on required serdeses
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] startSerdes              - number of first serdes to check
* @param[in] numOfSerdesToCheck       - how many serdes must be tested to report success
*
* @param[out] statusPtr                - pointer to signal detect status result:
*                                      GT_TRUE - signal detected on all required serdeses
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChSerdesSignalDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  startSerdes,
    IN  GT_U32  numOfSerdesToCheck,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32  i;                  /* iterator */
    GT_U32  regAddr;            /* register address */
    GT_U32  serdesReset, value; /* value of field */
    GT_U32  portGroupId;        /* the port group Id - support multi-port-groups device */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    for(i = 0; i < numOfSerdesToCheck; i++)
    {
        /* Deactivate SerDes Reset - must be done otherwise device stuck when try to access serdes under reset */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[startSerdes+i].serdesExternalReg2;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 3, 1, &serdesReset)) != GT_OK)
            return rc;

        if(serdesReset != 1)
        {
            if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 3, 1, 1)) != GT_OK)
                return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[startSerdes+i].receiverReg0;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 11, 1, &value)) != GT_OK)
            return rc;

        if(serdesReset != 1)
        {/* Activate SerDes Reset - don't leave not connected serdeses active  */
            if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 3, 1, 0)) != GT_OK)
                return rc;
        }

        /* if on serdes no signal detected stop */
        if(value == 1)
        {
            AUTODETECT_DBG_PRINT_MAC(("serdes %d no signal\n", startSerdes+i));
            *statusPtr = GT_FALSE;
            return GT_OK;
        }
    }

    *statusPtr = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvCpssDxChXpcsLanesSyncStatus function
* @endinternal
*
* @brief   The function checks if XG XPCS lanes RX synchronized
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to XE XPCS lanes RX sync status result:
*                                      GT_TRUE - synchronized,
*                                      GT_FALSE - not synchronized
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChXpcsLanesSyncStatus
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS   rc;          /* return code */
    GT_U8       laneNum;     /* iterator on serdes lanes */
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* value to write into the register */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    *statusPtr = GT_TRUE;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    for(laneNum = 0; laneNum < PRV_CPSS_DXCH_NUM_LANES_CNS; laneNum++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portNum].laneStatus[laneNum];
        /* get lane status SyncOK bit */
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                    4, 1, &value)) != GT_OK)
            return rc;

        if(value == 0)
        {
            AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChXpcsLanesSyncStatus:regAddr=0x%x\n", regAddr));
            AUTODETECT_DBG_PRINT_MAC(("SyncOK failed on port %d, lane %d\n",
                                        portNum, laneNum));
            *statusPtr = GT_FALSE;
            break;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSerdesConfig function
* @endinternal
*
* @brief   Configures serdes to frequency predefined by port data speed configuration
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode of current port (to save read it from DB)
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChSerdesConfig
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
{
    devNum = devNum;
    portNum = portNum;
    ifMode = ifMode;

    return GT_OK;
}

/**
* @internal prvCpssDxChXgmii10GDetect function
* @endinternal
*
* @brief   The function checks if XGMII (XAUI) interface mode and 10G data speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChXgmii10GDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL geXpcsStatus, /* link state of GE PCS */
            xgXpcsStatus, /* link state of XPCS */
            xpcsLanesSyncStatus, /* link state of XPCS */
            serdesSignalDetected; /* signal state on serdeses */
    GT_U32  numOfSerdesToCheck; /* number of serdeses to check */
    GT_U32  startSerdes;        /* first serdes to check */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChXgmii10GDetect\n"));

    /* init status to false - save few lines of code */
    *statusPtr = GT_FALSE;

    rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum,
                                                    CPSS_PORT_INTERFACE_MODE_XGMII_E,
                                                    &startSerdes, &numOfSerdesToCheck);
    if(rc != GT_OK)
    {
        return rc;
    }

    AUTODETECT_DBG_PRINT_MAC(("startSerdes=%d,numOfSerdesToCheck=%d\n", startSerdes, numOfSerdesToCheck));

    rc = prvCpssDxChSerdesSignalDetect(devNum, portNum, startSerdes, numOfSerdesToCheck, &serdesSignalDetected);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("serdesSignalDetected=%d\n", serdesSignalDetected));

    if(!serdesSignalDetected)
        return GT_OK;

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_XGMII_E)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, CPSS_PORT_SPEED_10000_E)) != GT_OK)
        return rc;

    if((rc = prvCpssDxChSerdesConfig(devNum, portNum, CPSS_PORT_INTERFACE_MODE_XGMII_E)) != GT_OK)
        return rc;

    /* Sync OK ? */
    rc = prvCpssDxChXpcsLanesSyncStatus(devNum, portNum, &xpcsLanesSyncStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcsLanesSyncStatus=%d\n", xpcsLanesSyncStatus));

    if(GT_FALSE == xpcsLanesSyncStatus)
        return GT_OK;

    /* xpcs link ? */
    rc = prvCpssDxChXpcsLinkStatus(devNum, portNum, &geXpcsStatus, &xgXpcsStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcs link ? geXpcsStatus=%d,xgXpcsStatus=%d\n",
                                geXpcsStatus, xgXpcsStatus));

    if(GT_FALSE == xgXpcsStatus)
        return GT_OK;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChXgmii10GDetect success\n"));

    *statusPtr = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvCpssDxChRxauiDetect function
* @endinternal
*
* @brief   The function checks if RXAUI interface mode and 10G data speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChRxauiDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL geXpcsStatus, /* link state of GE PCS */
            xgXpcsStatus, /* link state of XPCS */
            xpcsLanesSyncStatus, /* link state of XPCS */
            serdesSignalDetected; /* signal state on serdeses */
    GT_U32  numOfSerdesToCheck; /* number of serdeses to check */
    GT_U32  startSerdes;        /* first serdes to check */
    GT_U32  i;  /* iterator */
    GT_U8   activeSerdesNum;    /* number of serdeses where signal detected */
    GT_U32  regAddr,            /* register address */
            transmitterReg1Val, /* address of transmitter reg. 1 */
            ffeRegVal;          /* address of FFE register */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChRxauiDetect\n"));

    /* init status to false - save few lines of code */
    *statusPtr = GT_FALSE;

    rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum,
                                                    CPSS_PORT_INTERFACE_MODE_RXAUI_E,
                                                    &startSerdes, &numOfSerdesToCheck);
    if(rc != GT_OK)
    {
        return rc;
    }

    AUTODETECT_DBG_PRINT_MAC(("startSerdes=%d,numOfSerdesToCheck=%d\n", startSerdes, numOfSerdesToCheck));

    for(i = startSerdes, activeSerdesNum = 0; i < startSerdes+numOfSerdesToCheck; i++)
    {
        rc = prvCpssDxChSerdesSignalDetect(devNum, portNum, i, 1, &serdesSignalDetected);
        if(rc != GT_OK)
            return rc;

        if(serdesSignalDetected)
            activeSerdesNum++;
    }

    AUTODETECT_DBG_PRINT_MAC(("Signal Detected on %d serdeses\n", activeSerdesNum));
    if(activeSerdesNum < 2)
        return GT_OK;
    else if(activeSerdesNum > 2)
    {
        AUTODETECT_DBG_PRINT_MAC(("Warning: Expect signal detected on just two serdeses\n"));
    }

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_RXAUI_E)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, CPSS_PORT_SPEED_10000_E)) != GT_OK)
        return rc;

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;

        serdesPowerUpSequencePtr = (PRV_CPSS_PP_MAC(devNum)->revision > 2) ?
                        xcatSerdesPowerUpSequence : xcatA1SerdesPowerUpSequence;

        for(i = startSerdes; i < startSerdes + numOfSerdesToCheck; i++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].transmitterReg1;
            rc = prvCpssDrvHwPpReadRegister(devNum,regAddr,&transmitterReg1Val);
            if (rc != GT_OK)
                return rc;
            AUTODETECT_DBG_PRINT_MAC(("transmitterReg1Val=0x%x\n", transmitterReg1Val));
            /* if HW default kept in register - update it otherwise some XPCS lanes won't
                synch, else keep value configured by application */
            if(0x5503 == transmitterReg1Val)
            {
                transmitterReg1Val = serdesPowerUpSequencePtr[PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E]
                                                                [CPSS_DXCH_PORT_SERDES_SPEED_6_25_E];
                AUTODETECT_DBG_PRINT_MAC(("new transmitterReg1Val=0x%x\n", transmitterReg1Val));
                rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, transmitterReg1Val);
                if (rc != GT_OK)
                    return rc;
            }

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].ffeReg;
            rc = prvCpssDrvHwPpReadRegister(devNum,regAddr,&ffeRegVal);
            if (rc != GT_OK)
                return rc;
            AUTODETECT_DBG_PRINT_MAC(("ffeRegVal=0x%x\n", ffeRegVal));
            if(0x240 == ffeRegVal)
            {
                ffeRegVal = serdesPowerUpSequencePtr[PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E]
                                                        [CPSS_DXCH_PORT_SERDES_SPEED_6_25_E];
                AUTODETECT_DBG_PRINT_MAC(("new ffeRegVal=0x%x\n", ffeRegVal));
                rc = prvCpssDrvHwPpWriteRegister(devNum,regAddr,ffeRegVal);
                if (rc != GT_OK)
                    return rc;
            }
        }
    }

    if((rc = prvCpssDxChSerdesConfig(devNum, portNum, CPSS_PORT_INTERFACE_MODE_RXAUI_E)) != GT_OK)
        return rc;

    /* Sync OK ? */
    rc = prvCpssDxChXpcsLanesSyncStatus(devNum, portNum, &xpcsLanesSyncStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcsLanesSyncStatus=%d\n", xpcsLanesSyncStatus));
    if(GT_FALSE == xpcsLanesSyncStatus)
        return GT_OK;

    /* xpcs link ? */
    rc = prvCpssDxChXpcsLinkStatus(devNum, portNum, &geXpcsStatus, &xgXpcsStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcs link ? geXpcsStatus=%d,xgXpcsStatus=%d\n", geXpcsStatus, xgXpcsStatus));
    if(GT_FALSE == xgXpcsStatus)
        return GT_OK;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChRxauiDetect success\n"));

    *statusPtr = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvCpssDxCh1000BaseXDetect function
* @endinternal
*
* @brief   The function checks if 1000BaseX interface mode and 1G data speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxCh1000BaseXDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL serdesSignalDetected; /* signal state on serdeses */
    GT_U32  numOfSerdesToCheck; /* number of serdeses to check */
    GT_U32  startSerdes;        /* first serdes to check */
    GT_U32  i;  /* iterator */
    GT_U8   activeSerdesNum;    /* number of serdeses where signal detected */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode; /* detected port interface mode */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxCh1000BaseXDetect\n"));

    /* init status to false - save few lines of code */
    *statusPtr = GT_FALSE;

    rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum,
                                                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
                                                    &startSerdes, &numOfSerdesToCheck);
    if(rc != GT_OK)
    {
        return rc;
    }

    AUTODETECT_DBG_PRINT_MAC(("startSerdes=%d,numOfSerdesToCheck=%d\n", startSerdes, numOfSerdesToCheck));

    for(i = startSerdes, activeSerdesNum = 0; i < startSerdes+numOfSerdesToCheck; i++)
    {
        rc = prvCpssDxChSerdesSignalDetect(devNum, portNum, i, 1, &serdesSignalDetected);
        if(rc != GT_OK)
            return rc;

        if(serdesSignalDetected)
            activeSerdesNum++;
    }

    AUTODETECT_DBG_PRINT_MAC(("Signal Detected on %d serdeses\n", activeSerdesNum));
    if(activeSerdesNum == 0)
        return GT_OK;
    else if(activeSerdesNum > 1)
    {
        AUTODETECT_DBG_PRINT_MAC(("Warning: Expect signal detected on just one serdes\n"));
    }

    rc = prvCpssDxChGeDetect(devNum, portNum, &ifMode);
    if(rc != GT_OK)
        return rc;

    if(CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode)
    {
        AUTODETECT_DBG_PRINT_MAC(("prvCpssDxCh1000BaseXDetect success\n"));
        *statusPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChQsgmiiDetect function
* @endinternal
*
* @brief   The function checks if QSGMII interface mode and 1G data speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChQsgmiiDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL geXpcsStatus, /* link state of GE PCS */
            xgXpcsStatus, /* link state of XPCS */
            serdesSignalDetected; /* signal state on serdeses */
    GT_U32  startSerdes;        /* first serdes to check */
    GT_U32  regAddr,            /* register address */
            transmitterReg1Val, /* address of transmitter reg. 1 */
            ffeRegVal;          /* address of FFE register */
    GT_U32  portGroupId; /*the port group Id - support multi-port-groups device */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                    /* pointer to serdes power up sequence */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
        external registers serdes power up configuration (just dummy here) */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChQsgmiiDetect\n"));

    *statusPtr = GT_FALSE;

    startSerdes = portNum/4;

    AUTODETECT_DBG_PRINT_MAC(("startSerdes=%d\n", startSerdes));

    rc = prvCpssDxChSerdesSignalDetect(devNum, portNum, startSerdes, 1, &serdesSignalDetected);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("Serdes signal %s detected\n", serdesSignalDetected ? "" : "not"));
    if(!serdesSignalDetected)
        return GT_OK;

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_QSGMII_E)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, CPSS_PORT_SPEED_1000_E)) != GT_OK)
        return rc;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[startSerdes].transmitterReg1;
    rc = prvCpssDrvHwPpReadRegister(devNum,regAddr,&transmitterReg1Val);
    if (rc != GT_OK)
        return rc;
    AUTODETECT_DBG_PRINT_MAC(("transmitterReg1Val=0x%x\n", transmitterReg1Val));

    rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if HW default kept in register - update it otherwise some XPCS lanes won't
        synch, else keep value configured by application */
    if(0x5503 == transmitterReg1Val)
    {
        transmitterReg1Val = serdesPowerUpSequencePtr[PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E]
                                                        [CPSS_DXCH_PORT_SERDES_SPEED_5_E];
        AUTODETECT_DBG_PRINT_MAC(("new transmitterReg1Val=0x%x\n", transmitterReg1Val));
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, transmitterReg1Val);
        if (rc != GT_OK)
            return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[startSerdes].ffeReg;
    rc = prvCpssDrvHwPpReadRegister(devNum,regAddr,&ffeRegVal);
    if (rc != GT_OK)
        return rc;
    AUTODETECT_DBG_PRINT_MAC(("ffeRegVal=0x%x\n", ffeRegVal));
    if(0x240 == ffeRegVal)
    {
        ffeRegVal = serdesPowerUpSequencePtr[PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E]
                                                [CPSS_DXCH_PORT_SERDES_SPEED_5_E];
        AUTODETECT_DBG_PRINT_MAC(("new ffeRegVal=0x%x\n", ffeRegVal));
        rc = prvCpssDrvHwPpWriteRegister(devNum,regAddr,ffeRegVal);
        if (rc != GT_OK)
            return rc;
    }

    if((rc = prvCpssDxChSerdesConfig(devNum, portNum, CPSS_PORT_INTERFACE_MODE_QSGMII_E)) != GT_OK)
        return rc;

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,PRV_CPSS_PORT_GE_E,&regAddr);
    /* de-assert GE mac reset */
    if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr,
                                                6, 1, 0)) != GT_OK)
        return rc;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    /* xpcs link ? delay required before PCS status stabilized) */
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,300);
    rc = prvCpssDxChXpcsLinkStatus(devNum, portNum, &geXpcsStatus, &xgXpcsStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcs link ? geXpcsStatus=%d,xgXpcsStatus=%d\n",
                                                    geXpcsStatus, xgXpcsStatus));
    if(GT_FALSE == geXpcsStatus)
        return GT_OK;

    /* delay required before link status in MAC updated */
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,100);
    rc = cpssDxChPortLinkStatusGet(devNum,portNum,statusPtr);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChQsgmiiDetect succeeded\n"));

    return GT_OK;
}

/**
* @internal prvCpssDxChSGMII2_5GDetect function
* @endinternal
*
* @brief   The function checks if SGMII interface mode and 2.5G data speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChSGMII2_5GDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    /* init status to false - save few lines of code */
    *statusPtr = GT_FALSE;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMII2_5GDetect: Try Inband Auto-neg. enabled\n"));

    rc = prvCpssDxChSGMIIDetectAlgorithm(devNum, portNum,
                                            CPSS_PORT_SPEED_2500_E, GT_TRUE,
                                            statusPtr);
    if(rc != GT_OK)
        return rc;

    if(*statusPtr == GT_TRUE)
        return GT_OK;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMII2_5GDetect: Try Inband Auto-neg. disabled\n"));

    return prvCpssDxChSGMIIDetectAlgorithm(devNum, portNum,
                                            CPSS_PORT_SPEED_2500_E, GT_FALSE,
                                            statusPtr);
}

/**
* @internal prvCpssDxChSGMIIDetectAlgorithm function
* @endinternal
*
* @brief   The function checks if SGMII interface mode and required speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - required  to test, must be
*                                      CPSS_PORT_SPEED_1000_E or CPSS_PORT_SPEED_2500_E
* @param[in] inbandAutonegEn          - enable/disable inband auto-neg.
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChSGMIIDetectAlgorithm
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed,
    IN  GT_BOOL inbandAutonegEn,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL geXpcsStatus, /* link state of GE PCS */
            xgXpcsStatus, /* link state of XPCS */
            serdesSignalDetected; /* signal state on serdeses */
    GT_U32  startSerdes;        /* first serdes to check */
    GT_U32  numOfSerdesToCheck; /* number of serdeses to check */
    GT_U32  i;  /* iterator */
    GT_U8   activeSerdesNum;    /* number of serdeses where signal detected */
    GT_U32  portGroupId, /* port group number */
            regAddr;    /* register address */
    GT_BOOL isLinkUp;   /* link status */
    CPSS_PORT_SPEED_ENT   currentSpeed; /* current port speed */
    CPSS_PORT_DUPLEX_ENT  currentDMode; /* current port duplex */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    if((speed != CPSS_PORT_SPEED_1000_E) &&
        (speed != CPSS_PORT_SPEED_2500_E))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMIIDetectAlgorithm:%s\n",
                                (speed == CPSS_PORT_SPEED_1000_E) ?
                                        "CPSS_PORT_SPEED_1000_E" :
                                            "CPSS_PORT_SPEED_2500_E"));

    /* init status to false - save few lines of code */
    *statusPtr = GT_FALSE;

    rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum,
                                                    CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                                    &startSerdes, &numOfSerdesToCheck);
    if(rc != GT_OK)
    {
        return rc;
    }

    AUTODETECT_DBG_PRINT_MAC(("startSerdes=%d,numOfSerdesToCheck=%d\n", startSerdes, numOfSerdesToCheck));

    for(i = startSerdes, activeSerdesNum = 0; i < startSerdes+numOfSerdesToCheck; i++)
    {
        rc = prvCpssDxChSerdesSignalDetect(devNum, portNum, i, 1, &serdesSignalDetected);
        if(rc != GT_OK)
            return rc;

        if(serdesSignalDetected)
            activeSerdesNum++;
    }

    AUTODETECT_DBG_PRINT_MAC(("Signal Detected on %d serdeses\n", activeSerdesNum));
    if(activeSerdesNum == 0)
        return GT_OK;
    else if(activeSerdesNum > 1)
    {
        AUTODETECT_DBG_PRINT_MAC(("Warning: Expect signal detected on just one serdes\n"));
    }

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_SGMII_E)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, speed)) != GT_OK)
        return rc;

    if((rc = prvCpssDxChSerdesConfig(devNum, portNum, CPSS_PORT_INTERFACE_MODE_SGMII_E)) != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,PRV_CPSS_PORT_GE_E,&regAddr);
    /* de-assert GE mac reset */
    if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                6, 1, 0)) != GT_OK)
        return rc;

    /* xpcs link ? */
    rc = prvCpssDxChXpcsLinkStatus(devNum, portNum, &geXpcsStatus, &xgXpcsStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcs link ? geXpcsStatus=%d,xgXpcsStatus=%d\n",
                                                    geXpcsStatus, xgXpcsStatus));
    if(GT_FALSE == geXpcsStatus)
        return GT_OK;

    if((rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum, (!inbandAutonegEn))) != GT_OK)
        return rc;

    if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, inbandAutonegEn)) != GT_OK)
        return rc;

    rc = prvCpssDxChInbandAutoNegRestartAndWait(devNum,portNum);
    if((rc != GT_OK) && (rc != GT_TIMEOUT))
    {
        return rc;
    }

    if((speed == CPSS_PORT_SPEED_1000_E) && (rc == GT_TIMEOUT))
    {
        AUTODETECT_DBG_PRINT_MAC(("SGMII:isPortAnDone false\n"));
        return GT_OK;
    }

    /* delay required before link status in MAC updated */
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,200);
    rc = cpssDxChPortLinkStatusGet(devNum,portNum,&isLinkUp);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMIIDetectAlgorithm %s link status %d\n",
                                (speed == CPSS_PORT_SPEED_1000_E) ?
                                        "CPSS_PORT_SPEED_1000_E" :
                                            "CPSS_PORT_SPEED_2500_E", isLinkUp));

    if(GT_FALSE == isLinkUp)
    {
        if(speed == CPSS_PORT_SPEED_1000_E)
        {
            if((rc = cpssDxChPortSpeedGet(devNum, portNum, &currentSpeed)) != GT_OK)
                return rc;

            if((rc = cpssDxChPortDuplexModeGet(devNum, portNum, &currentDMode)) != GT_OK)
                return rc;

            AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMIIDetectAlgorithm currentSpeed=%d,currentDMode=%d\n",
                                        currentSpeed, currentDMode));

            if((CPSS_PORT_SPEED_10_E != currentSpeed) || (CPSS_PORT_HALF_DUPLEX_E != currentDMode))
            {
                *statusPtr = GT_TRUE;
            }
        }
    }
    else
    {
        *statusPtr = GT_TRUE;
    }

    return GT_OK;
}

static GT_STATUS prvCpssDxCh1000BaseXConfig
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      portGroupId;    /* number of group of port for multi-port-group devices */

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)) != GT_OK)
        return rc;

    /* for 1000BaseX speed and duplex auto-neg has no sence */
    if((rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE)) != GT_OK)
        return rc;

    /* in devices of Lion family half-duplex not supported in any port mode/speed,
        so don't touch duplex auto-neg. to prevent unpredictable behaviour */
    if((rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE)) != GT_OK)
        return rc;

    /* to avoid possible problems caused by FC lets disable it */
    if((rc = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, portNum, GT_FALSE, GT_FALSE)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortFlowControlEnableSet(devNum, portNum, CPSS_PORT_FLOW_CONTROL_DISABLE_E)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, CPSS_PORT_SPEED_1000_E)) != GT_OK)
        return rc;

    if((rc = prvCpssDxChSerdesConfig(devNum, portNum, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)) != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,PRV_CPSS_PORT_GE_E,&regAddr);
    /* de-assert GE mac reset */
    if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                6, 1, 0)) != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal prvCpssDxChInbandAutoNegRestartAndWait function
* @endinternal
*
* @brief   Restart inband auto-negotiation and wait till it's done.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChInbandAutoNegRestartAndWait
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      fieldData; /* register field value */
    GT_U32      timeoutCntr;    /* count timeout of inband auto-neg. */

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChInbandAutoNegRestartAndWait\n"));

    if((rc = cpssDxChPortInbandAutoNegRestart(devNum,portNum)) != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].macStatus;
    timeoutCntr = 0;
    do
    {
        /* should check with delays of 30msec => 300msec overall
            length of auto-neg. process according to protocol
            here 60 msec to be sure */
        HW_WAIT_MILLISECONDS_MAC(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 60);
        /* read AnDone bit */
        if (prvCpssDrvHwPpGetRegField(devNum, regAddr, 11, 1, &fieldData) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        timeoutCntr++;
    } while((fieldData != 1) && (timeoutCntr < 10));

    if(10 == timeoutCntr)
    {
        AUTODETECT_DBG_PRINT_MAC(("InbandAutoNeg timeout\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
GT_STATUS internal_cpssDxChPortAutoNegAdvertismentConfigGet
(
    IN   GT_U8                                    devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)
{
    GT_STATUS                       rc;         /* return code */
    GT_U32                          regAddr;    /* register address */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;     /* port interface */
    CPSS_PORT_SPEED_ENT             speed;      /* port speed */
    GT_U32                          data;       /* data from register */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(portAnAdvertismentPtr);
        speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
        if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
            (speed != CPSS_PORT_SPEED_1000_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                             perPortRegs[portMacMap].serdesCnfg2;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 16, &data);
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
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssDxChPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
GT_STATUS cpssDxChPortAutoNegAdvertismentConfigGet
(
    IN   GT_U8                                    devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortAutoNegAdvertismentConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portAnAdvertismentPtr));

    rc = internal_cpssDxChPortAutoNegAdvertismentConfigGet(devNum, portNum, portAnAdvertismentPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portAnAdvertismentPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
GT_STATUS internal_cpssDxChPortAutoNegAdvertismentConfigSet
(
    IN   GT_U8                                    devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    IN   CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)
{
    GT_STATUS                       rc;            /* return code */
    GT_U32                          regAddr;       /* register address */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;        /* port interface */
    CPSS_PORT_SPEED_ENT             speed;         /* port speed */
    GT_U32                          data = 0x4001; /* advertisment data */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */
    PRV_CPSS_DXCH_PP_CONFIG_STC     *pDev;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(portAnAdvertismentPtr);
        speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
        if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
            (speed != CPSS_PORT_SPEED_1000_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                            perPortRegs[portMacMap].serdesCnfg2;
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

        if((pDev->genInfo.devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E) || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            /* perform MAC RESET */
            rc = prvCpssDxChPortMacResetStateSet(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortMacResetStateSet: error for portNum = %d\n", portNum);
            }

            /* perform PCS RESET */
            rc = cpssDxChPortPcsResetSet(devNum, portNum, CPSS_PORT_PCS_RESET_MODE_ALL_E, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortPcsResetSet: error for portNum = %d\n", portNum);
            }
        }



        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 16, data);
        if(rc != GT_OK)
        {
            return rc;
        }

        if ((pDev->genInfo.devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E) || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            /* perform PCS UNRESET */
            rc = cpssDxChPortPcsResetSet(devNum, portNum, CPSS_PORT_PCS_RESET_MODE_ALL_E, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortPcsResetSet: error for portNum = %d\n", portNum);
            }

            /* perform MAC UNRESET */
            rc = prvCpssDxChPortMacResetStateSet(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortMacResetStateSet: error for portNum = %d\n", portNum);
            }
        }

        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

}

/**
* @internal cpssDxChPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
GT_STATUS cpssDxChPortAutoNegAdvertismentConfigSet
(
    IN   GT_U8                                    devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    IN   CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortAutoNegAdvertismentConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portAnAdvertismentPtr));

    rc = internal_cpssDxChPortAutoNegAdvertismentConfigSet(devNum, portNum, portAnAdvertismentPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portAnAdvertismentPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
static GT_STATUS internal_cpssDxChPortAutoNegMasterModeEnableGet
(
    IN   GT_U8                    devNum,
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

   PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
   PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
   if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(enablePtr);
        speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
        if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
            (speed != CPSS_PORT_SPEED_1000_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                                perPortRegs[portMacMap].serdesCnfg;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 11, 1, &bitValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        *enablePtr = BIT2BOOL_MAC(bitValue);
        if(GT_TRUE == *enablePtr)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                                perPortRegs[portMacMap].serdesCnfg;
            rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 9, 1, &bitValue);
            if (rc != GT_OK)
            {
                return rc;
            }
            *enablePtr = BIT2BOOL_MAC(bitValue);
        }
        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

}

/**
* @internal cpssDxChPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
GT_STATUS cpssDxChPortAutoNegMasterModeEnableGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortAutoNegMasterModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortAutoNegMasterModeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChAutoNegMasterModeSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation code word.
*         When set, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
static GT_STATUS prvCpssDxChAutoNegMasterModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg;
    /* configure <Auto-Negotiation Master Mode Enable> */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 9, 1, BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }
    /* configure Force Link on Master Mode - This bit should be set to 1 when
        <Auto-Negotiation Master Mode Enable> is set to 1. The link is taken
        according to <TX Config Reg>[15]. */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 1, BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
static GT_STATUS internal_cpssDxChPortAutoNegMasterModeEnableSet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_BOOL                  enable
)
{
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;     /* port interface */
    CPSS_PORT_SPEED_ENT             speed;      /* port speed */
    GT_U32                          portMacMap; /* number of mac mapped to this physical port */
    GT_STATUS                       rc;         /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
        if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if((speed != CPSS_PORT_SPEED_10_E) && (speed != CPSS_PORT_SPEED_100_E) &&
            (speed != CPSS_PORT_SPEED_1000_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChAutoNegMasterModeSet(devNum, portMacMap, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
        if(GT_TRUE == enable)
        {
            rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChPortFlowControlEnableSet(devNum, portNum, CPSS_PORT_FLOW_CONTROL_DISABLE_E);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssDxChPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
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
GT_STATUS cpssDxChPortAutoNegMasterModeEnableSet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortAutoNegMasterModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortAutoNegMasterModeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortConfig function
* @endinternal
*
* @brief   Sets Interface mode and speed on a specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - Interface mode
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For constrains see cpssDxChPortInterfaceModeSet, cpssDxChPortSpeedSet,
*       serdesPowerStatusSet
*
*/
static GT_STATUS prvCpssDxChPortConfig
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT            speed
)
{
    GT_STATUS rc;

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, ifMode)) != GT_OK)
    {
        return rc;
    }

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, speed)) != GT_OK)
    {
        return rc;
    }

    if((rc = prvCpssDxChSerdesConfig(devNum, portNum, ifMode)) != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChGeByHwDetect function
* @endinternal
*
* @brief   The function tries to find out which GE interface 1000BaseX or SGMII
*         compatible with given port interface.
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ifModePtr                - pointer to acheaved interface mode:
*                                      CPSS_PORT_INTERFACE_MODE_SGMII_E or CPSS_PORT_INTERFACE_MODE_1000BASE_X_E
*                                      CPSS_PORT_INTERFACE_MODE_NA_E - if failed to find appropriate configuration
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*
* @note This function uses new HW mechanism to distinguish SGMII or 1000BaseX
*
*/
static GT_STATUS prvCpssDxChGeByHwDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      fieldData;  /* register field value */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    /* Try 1000BaseX interface */
    if((rc = prvCpssDxChPortConfig(devNum, portNum,
                                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
                                    CPSS_PORT_SPEED_1000_E)) != GT_OK)
    {
        return rc;
    }

    if((rc = cpssDxChPortDuplexModeSet(devNum, portNum, CPSS_PORT_FULL_DUPLEX_E)) != GT_OK)
    {
        return rc;
    }

    if((rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE)) != GT_OK)
    {
        return rc;
    }

    if((rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE)) != GT_OK)
    {
        return rc;
    }

    /* enable overwrite of the Auto-Negotiation code word */
    rc = prvCpssDxChAutoNegMasterModeSet(devNum, portNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* <TX Config Reg> = 0xD801 (1G FD) */
    /* <TX Config Reg> is the register notion used in the guidelines to deal */
    /* with this AN issue, actually it is the Serdes Configuration Register. */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0xD801);
    if(rc != GT_OK)
    {
        goto cleanExit;
    }

    /* enable <Bypass Auto-Negotiation during data> */
    /* It�s a more loose requirement to reach AnBypass
        When bit unset � device will not reach AnBypass stage if link partner
                        is sending traffic
        When bit set � device can reach AnBypass stage if link partner
                        is sending traffic
        There is no standard definition for AnBypass, so both options are
        equally correct to do. In the auto-detect process, the link partner
        may be in force mode, and already sending traffic. In this case,
        we need to set this bit to allow ourselves to reach AnBypass and
        force our own settings.
    */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 7, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum, GT_TRUE)) != GT_OK)
    {
        goto cleanExit;
    }

    if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, GT_TRUE)) != GT_OK)
    {
        goto cleanExit;
    }

    /* read <SquelchNot Detected> to find out if signal present on serdes */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].macStatus;
    if (prvCpssDrvHwPpGetRegField(devNum, regAddr, 15, 1, &fieldData) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    if(1 == fieldData)
    {/* no signal - exit */
        rc = GT_OK;
        goto cleanExit;
    }

    if((rc = prvCpssDxChInbandAutoNegRestartAndWait(devNum,portNum)) != GT_OK)
    {
        goto cleanExit;
    }

    /* Read <InBandAnByPassAct> */
    if (prvCpssDrvHwPpGetRegField(devNum, regAddr, 12, 1, &fieldData) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    if(1 == fieldData)
    {/* Done 1000BASE-X or force mode*/
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
        rc = GT_OK;
        goto cleanExit;
    }

    /* Read <RX Config Reg>[0] */
    /* NOTE:This register bitwise inverted. For example, reading 0xFFFF
        means a value of 0x0000. */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg3;
    if (prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 1, &fieldData) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    if(1 == fieldData)
    {/* Done 1000BASE-X or force mode*/
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
        rc = GT_OK;
        goto cleanExit;
    }

    /* Try to function as PHY side */
    if((rc = prvCpssDxChPortConfig(devNum, portNum,
                                    CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                    CPSS_PORT_SPEED_1000_E)) != GT_OK)
    {
        goto cleanExit;
    }

    if((rc = prvCpssDxChInbandAutoNegRestartAndWait(devNum,portNum)) != GT_OK)
    {
        goto cleanExit;
    }

    /* Get result of autonegotiation from <RX Config Reg> */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg3;
    if (prvCpssDrvHwPpReadRegister(devNum, regAddr, &fieldData) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    if(0xFFFF == fieldData)
    {/* auto-negotiation failed - something wrong */
        goto cleanExit;
    }

    /* content of <RX Config Reg> is inverted */
    fieldData=(~fieldData);
    if(0x4001 == fieldData)
    {
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_SGMII_E;
        rc = GT_OK;
        goto cleanExit;
    }

    /* Remote side is PHY - force local according to <RX Config Reg> */
    /* force duplex */
    if((rc = cpssDxChPortDuplexModeSet(devNum, portNum,
                                        ((U32_GET_FIELD_MAC(fieldData, 12, 1) == 1) ?
                                            CPSS_PORT_FULL_DUPLEX_E :
                                            CPSS_PORT_HALF_DUPLEX_E))) != GT_OK)
    {
        goto cleanExit;
    }

    /* force speed if agreed by negotiation is not 1000 */
    fieldData = U32_GET_FIELD_MAC(fieldData, 10, 2);
    if(fieldData != 0x2)
    {
        if((rc = prvCpssDxChPortConfig(devNum, portNum,
                                        CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                        ((1 == fieldData) ?
                                            CPSS_PORT_SPEED_100_E :
                                            CPSS_PORT_SPEED_10_E))) != GT_OK)
        {
            goto cleanExit;
        }
    }

    /* disable overwrite of the Auto-Negotiation code word */
    rc = prvCpssDxChAutoNegMasterModeSet(devNum, portNum, GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* disable <Bypass Auto-Negotiation during data> */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 7, 1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    *ifModePtr = CPSS_PORT_INTERFACE_MODE_SGMII_E;
    return GT_OK;

cleanExit:
    /* disable overwrite of the Auto-Negotiation code word */
    (GT_VOID)prvCpssDxChAutoNegMasterModeSet(devNum, portNum, GT_FALSE);

    /* disable <Bypass Auto-Negotiation during data> */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                        perPortRegs[portNum].serdesCnfg;
    (GT_VOID)prvCpssDrvHwPpSetRegField(devNum, regAddr, 7, 1, 0);

    return rc;
}

/**
* @internal prvCpssDxChGeDetect function
* @endinternal
*
* @brief   The function tries to find out which GE interface
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ifModePtr                - pointer to acheaved interface mode:
*                                      CPSS_PORT_INTERFACE_MODE_SGMII_E or CPSS_PORT_INTERFACE_MODE_1000BASE_X_E
*                                      CPSS_PORT_INTERFACE_MODE_NA_E - if failed to find appropriate configuration
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*
* @note Warning:
*       Still there is no possibility to distinguish if partner has SGMII
*       or 1000BaseX, when auto-negotiation disabled.
*       Preference to 1000BaseX given, because when SGMII on network ports
*       defined, executed auto-polling of PHY, even if auto-negotiation
*       disabled, and so, link never will come up in MAC-to-MAC connection.
*       Assumtions:
*       When we try SGMII with auto-negotiation enabled we assume that
*       partner emulates the PHY auto-negotiation control work.
*       Application notes:
*       The algorithm will run every time applications provides in options
*       array SGMII 1G or 1000BaseX, after finish CPSS will check if
*       synchronized with wanted interface and stop or continue to next option.
*       But customer must be aware that in case of auto-negotiation disabled
*       for both interfaces � result will be 1000BaseX.
*
*/
static GT_STATUS prvCpssDxChGeDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS   rc; /* return code */
    CPSS_PORT_MAC_STATUS_STC portMacStatus; /* port status - isPortAnDone */
    GT_BOOL     isLinkUp;   /* MAC link status */
    GT_BOOL     sgmiiDetectStatus;  /* is SGMII auto-neg. enabled get link with partner */
    GT_BOOL     geXpcsStatus,   /* sync ok on GE PCS */
                xgXpcsStatus;   /* sync ok on XG PCS */
    GT_U32      portGroupId;    /* port-group number */

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChGeDetect start\n"));

    CPSS_NULL_PTR_CHECK_MAC(ifModePtr);

    *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
        && (PRV_CPSS_PP_MAC(devNum)->revision >= 4))
    {
        if(((portNum%4)==0)||(portNum>=24))
        {/* feature supported on network ports only on first port in quadruplet */
            return prvCpssDxChGeByHwDetect(devNum, portNum, ifModePtr);
        }
    }

    AUTODETECT_DBG_PRINT_MAC(("Configure 1000BaseX inband auto-neg. enabled as default\n"));

    if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, GT_TRUE)) != GT_OK)
    {
        return rc;
    }

    if((rc = prvCpssDxCh1000BaseXConfig(devNum,portNum)) != GT_OK)
    {
        return rc;
    }

    /* random sleep up to 200 msec */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,cpssOsRand() % 200);

    /* sync OK ? */
    rc = prvCpssDxChXpcsLinkStatus(devNum, portNum, &geXpcsStatus, &xgXpcsStatus);
    if(rc != GT_OK)
        return rc;

    AUTODETECT_DBG_PRINT_MAC(("xpcs link ? geXpcsStatus=%d,xgXpcsStatus=%d\n", geXpcsStatus, xgXpcsStatus));
    if(GT_FALSE == geXpcsStatus)
        return GT_OK;

    if((rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_TRUE)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_TRUE)) != GT_OK)
        return rc;

    rc = prvCpssDxChSGMIIDetectAlgorithm(devNum,portNum,CPSS_PORT_SPEED_1000_E,GT_TRUE,&sgmiiDetectStatus);
    if(rc != GT_OK)
        return rc;

    if(GT_TRUE == sgmiiDetectStatus)
    {
        AUTODETECT_DBG_PRINT_MAC(("SGMII succeeded\n"));
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_SGMII_E;
        return GT_OK;
    }

    rc = cpssDxChPortMacStatusGet(devNum,portNum,&portMacStatus);
    if(rc != GT_OK)
        return rc;

    if(GT_FALSE == portMacStatus.isPortAnDone)
    {
        AUTODETECT_DBG_PRINT_MAC(("SGMII:isPortAnDone false, try 1000BaseX no inband auto-neg.\n"));
        goto autonegDisCase;
    }

    AUTODETECT_DBG_PRINT_MAC(("Try 1000BaseX inband auto-neg. enabled\n"));
    if((rc = prvCpssDxCh1000BaseXConfig(devNum,portNum)) != GT_OK)
    {
        return rc;
    }

    /* delay required before link status in MAC updated */
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,200);
    rc = cpssDxChPortLinkStatusGet(devNum,portNum,&isLinkUp);
    if(rc != GT_OK)
        return rc;

    if(GT_TRUE == isLinkUp)
    {
        AUTODETECT_DBG_PRINT_MAC(("Link up with 1000BaseX\n"));
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
        return GT_OK;
    }

    /* if auto-neg. done on port then no sence to try further */
    return GT_OK;

    /* try 1000BaseX with auto-neg. disabled */
autonegDisCase:
    if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, GT_FALSE)) != GT_OK)
        return rc;

    if((rc = prvCpssDxCh1000BaseXConfig(devNum,portNum)) != GT_OK)
    {
        return rc;
    }
    /* delay required before link status in MAC updated */
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,200);
    rc = cpssDxChPortLinkStatusGet(devNum,portNum,&isLinkUp);
    if(rc != GT_OK)
        return rc;

    if(GT_TRUE == isLinkUp)
    {
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSGMII1GDetect function
* @endinternal
*
* @brief   The function checks if SGMII interface mode and 1G data speed
*         compatible with given port interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - pointer to test result:
*                                      GT_TRUE - suggested mode/speed pair compatible with given port interface,
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvCpssDxChSGMII1GDetect
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *statusPtr
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    /* init status to false - save few lines of code */
    *statusPtr = GT_FALSE;

    AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMII1GDetect: Try Inband Auto-neg. enabled\n"));

    rc = prvCpssDxChGeDetect(devNum, portNum, &ifMode);
    if(rc != GT_OK)
        return rc;

    if(CPSS_PORT_INTERFACE_MODE_SGMII_E == ifMode)
    {
        AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChSGMII1GDetect success\n"));
        *statusPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortModeSpeedAutoDetectAndConfig function
* @endinternal
*
* @brief   Autodetect and configure interface mode and speed of a given port
*         If the process succeeded the port gets configuration ready for link,
*         otherwise (none of provided by application options valid) the port
*         is left in reset mode.
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port not supported, FE ports not supported)
* @param[in] portModeSpeedOptionsArrayPtr � array of port mode and speed options
*                                      preferred for application,
*                                      NULL � if any of supported options acceptable
* @param[in] optionsArrayLen          � length of options array (must be 0 if
* @param[in] portModeSpeedOptionsArrayPtr == NULL)
*
* @param[out] currentModePtr           � if succeeded ifMode and speed that were configured
*                                      on the port,
*                                      otherwise previous ifMode and speed restored
*
* @retval GT_OK                    - if process completed without errors (no matter if mode
*                                       configured or not)
* @retval GT_BAD_PARAM             - on wrong port number or device,
*                                       optionsArrayLen more than possible options
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_NOT_SUPPORTED     � ifMode/speed pair not supported,
*                                       wrong port type
* @retval GT_HW_ERROR              - HW error
*
* @note For list of supported modes see:
*       lionDefaultPortModeSpeedOptionsArray,
*       xcatDefaultNetworkPortModeSpeedOptionsArray,
*       xcatDefaultStackPortModeSpeedOptionsArray
*       There is no possibility to distinguish if partner has SGMII or
*       1000BaseX, when auto-negotiation disabled. Preference to 1000BaseX given
*       by algorithm.
*       Responsibility of application to mask link change interrupt during
*       auto-detection algorithm run.
*
*/
static GT_STATUS internal_cpssDxChPortModeSpeedAutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   CPSS_PORT_MODE_SPEED_STC *portModeSpeedOptionsArrayPtr,
    IN   GT_U8                    optionsArrayLen,
    OUT  CPSS_PORT_MODE_SPEED_STC *currentModePtr
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_MODE_SPEED_STC *optionsPtr;   /* ptr array of pairs to test */
    GT_U8 option, i; /* iterators */
    GT_U8 optionsNum; /* number of options to test */
    GT_U8 maxOptionsNum; /* possible maximum of pairs to test for device/port type */
    GT_BOOL status = GT_FALSE; /* test resutl for ifMode/speed pair */
    GT_U32  numOfSerdesLanes = 8; /* default to currently possible max number of
                                     serdes lanes */
    CPSS_PORT_INTERFACE_MODE_ENT originalIfMode;    /* original ifMode of port if
                                all options fail and must return to prior state */
    CPSS_PORT_SPEED_ENT          originalSpeed; /* original speed of port if
                                all options fail and must return to prior state */
    GT_BOOL originalSerdesPowerUpState; /* original serdes power state of port if
                                all options fail and must return to prior state */
    GT_U32  startSerdes;    /* first serdes of port */
    GT_U32  lanesBmp;       /* serdes lanes bitmap */
    CPSS_PORT_MODE_SPEED_STC *defaultPortModeSpeedOptionsArray; /* ptr to default
        array of options for device/port type */
    PRV_CPSS_PORT_MODE_SPEED_TEST_FUN *portModeSpeedTestFunctionsArray; /* ptr to
        array of options testing functions */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    GT_BOOL originalInbandAutoNeg,
            originalInbandAutoNegBypass,
            originalSpeedAutoNeg,
            originalDuplexAutoNeg,
            originalFcAnState,
            originalPauseAdvertise;
    CPSS_PORT_FLOW_CONTROL_ENT originalFcStatus;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(currentModePtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) <= PRV_CPSS_PORT_FE_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum) == PRV_CPSS_GE_PORT_GE_ONLY_E)
    {
        defaultPortModeSpeedOptionsArray = xcatDefaultNetworkPortModeSpeedOptionsArray;
        portModeSpeedTestFunctionsArray = xcatNetworkPortModeSpeedTestFunctionsArray;
        maxOptionsNum = sizeof(xcatDefaultNetworkPortModeSpeedOptionsArray)/
                                        sizeof(CPSS_PORT_MODE_SPEED_STC);
    }
    else
    {
        defaultPortModeSpeedOptionsArray = xcatDefaultStackPortModeSpeedOptionsArray;
        portModeSpeedTestFunctionsArray = xcatStackPortModeSpeedTestFunctionsArray;
        maxOptionsNum = sizeof(xcatDefaultStackPortModeSpeedOptionsArray)/
                                        sizeof(CPSS_PORT_MODE_SPEED_STC);
    }

    if(optionsArrayLen > maxOptionsNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(portModeSpeedOptionsArrayPtr != NULL)
    {
        optionsPtr = portModeSpeedOptionsArrayPtr;
        optionsNum = optionsArrayLen;

        for(option = 0; option < optionsArrayLen; option++)
        {
            if(optionsPtr[option].ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

            if(optionsPtr[option].speed >= CPSS_PORT_SPEED_NA_E)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        optionsPtr = defaultPortModeSpeedOptionsArray;
        optionsNum = maxOptionsNum;
    }

    /* save original states of ifMode, speed, SerdesPowerUp */
    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&originalIfMode);
    if(rc != GT_OK)
    {
        originalIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    }

    rc = cpssDxChPortSpeedGet(devNum,portNum,&originalSpeed);
    if(rc != GT_OK)
    {
        originalSpeed = CPSS_PORT_SPEED_NA_E;
    }

    rc = prvCpssDxCh3PortSerdesPowerUpDownGet(devNum, portNum,
                                                &originalSerdesPowerUpState);
    if(rc != GT_OK)
    {
        originalSerdesPowerUpState = GT_FALSE;
    }

    AUTODETECT_DBG_PRINT_MAC(("original states:ifMode=%d,speed=%d,serdesPowerUp=%d\n",
                            originalIfMode, originalSpeed, originalSerdesPowerUpState));

    /* read and store original values of port parameters usually changed by auto-detect
        for case this mode will fail */
    if((rc = cpssDxChPortInbandAutoNegEnableGet(devNum, portNum, &originalInbandAutoNeg)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortInBandAutoNegBypassEnableGet(devNum, portNum, &originalInbandAutoNegBypass)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedAutoNegEnableGet(devNum, portNum, &originalSpeedAutoNeg)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortDuplexAutoNegEnableGet(devNum, portNum, &originalDuplexAutoNeg)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortFlowCntrlAutoNegEnableGet(devNum, portNum, &originalFcAnState,
                                                                &originalPauseAdvertise)) != GT_OK)
        return rc;
    if((rc = cpssDxChPortFlowControlEnableGet(devNum, portNum, &originalFcStatus)) != GT_OK)
        return rc;

    for(option = 0; option < optionsNum; option++)
    {
        if(portModeSpeedOptionsArrayPtr != NULL)
        {
            for(i = 0; i < maxOptionsNum; i++)
            {
                if((optionsPtr[option].ifMode == defaultPortModeSpeedOptionsArray[i].ifMode)
                    && (optionsPtr[option].speed == defaultPortModeSpeedOptionsArray[i].speed))
                {
                    break;
                }
            }

            if(i == maxOptionsNum)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        else
        {
            if((CPSS_PP_FAMILY_DXCH_XCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
               && (PRV_CPSS_PP_MAC(devNum)->revision < 3)
               && (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) != GT_TRUE)
               && (CPSS_PORT_SPEED_2500_E ==
                                defaultPortModeSpeedOptionsArray[option].speed))
            {/* xcat2 older then B0 don't support 2.5G on network ports */
                continue;
            }

            i = option;
        }

        AUTODETECT_DBG_PRINT_MAC(("test pair:ifMode=%d,speed=%d\n",
                                    defaultPortModeSpeedOptionsArray[i].ifMode,
                                    defaultPortModeSpeedOptionsArray[i].speed));

        if((rc = portModeSpeedTestFunctionsArray[i](devNum, portNum, &status)) != GT_OK)
            return rc;

        if( (GT_TRUE == status) && (i < maxOptionsNum) )
        {
            currentModePtr->ifMode = defaultPortModeSpeedOptionsArray[i].ifMode;
            currentModePtr->speed = defaultPortModeSpeedOptionsArray[i].speed;
            break;
        }
        else /* reset port units */
        {
            rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum,
                                                            optionsPtr[option].ifMode,
                                                            &startSerdes, &numOfSerdesLanes);
            if(rc != GT_OK)
            {
                return rc;
            }

            AUTODETECT_DBG_PRINT_MAC(("reset serdes and mac:portNum=%d,startSerdes=%d,numOfSerdesLanes=%d\n",
                                        portNum, startSerdes, numOfSerdesLanes));

            if((rc = prvCpssDxChPortSerdesResetStateSet(devNum, portNum, startSerdes,
                                                        numOfSerdesLanes, GT_TRUE)) != GT_OK)
                return rc;

            if((rc = prvCpssDxChPortMacResetStateSet(devNum, portNum, GT_TRUE)) != GT_OK)
                return rc;

            /*  happens that there is not enough time for signal on serdeses to restore
                and after fail of wrong option try, right option fails too, because of signal
                not detected on serdeses after reset
             */

            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,100);
        }
    }

    /* If API fails to establish link, port mode, speed and Serdes power
        set to original values */
    if(GT_FALSE == status)
    {
        AUTODETECT_DBG_PRINT_MAC(("restore original state:ifMode=%d,speed=%d,powerUp=%d\n",
                                    originalIfMode, originalSpeed, originalSerdesPowerUpState));

        if(originalIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            currentModePtr->ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        }
        else
        {
            rc = cpssDxChPortInterfaceModeSet(devNum,portNum,originalIfMode);
            if(rc != GT_OK)
                return rc;
            currentModePtr->ifMode = originalIfMode;
        }

        if(originalSpeed == CPSS_PORT_SPEED_NA_E)
        {
            currentModePtr->speed = CPSS_PORT_SPEED_NA_E;
        }
        else
        {
            rc = cpssDxChPortSpeedSet(devNum,portNum,originalSpeed);
            if(rc != GT_OK)
                return rc;
            currentModePtr->speed = originalSpeed;
        }

        if(GT_TRUE == originalSerdesPowerUpState)
        {
            lanesBmp = BIT_MASK_MAC(numOfSerdesLanes);
            if((rc = cpssDxChPortSerdesPowerStatusSet(devNum, portNum,
                                                        CPSS_PORT_DIRECTION_BOTH_E,
                                                        lanesBmp, GT_TRUE)) != GT_OK)
            {
                return rc;
            }
        }

        /* case failed - restore original values of port parameters usually changed by auto-detect algorithm */
        AUTODETECT_DBG_PRINT_MAC(("restore original state:originalSpeedAutoNeg=%d,\
                                    originalDuplexAutoNeg=%d,originalFcAnState=%d,\
                                    originalPauseAdvertise=%d,\
                                    originalInbandAutoNegBypass=%d,\
                                    originalInbandAutoNeg=%d\n",
                                    originalSpeedAutoNeg, originalDuplexAutoNeg,
                                    originalFcAnState, originalPauseAdvertise,
                                    originalInbandAutoNegBypass, originalInbandAutoNeg));

        if((rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, originalSpeedAutoNeg)) != GT_OK)
        {
            AUTODETECT_DBG_PRINT_MAC(("cpssDxChPortSpeedAutoNegEnableSet rc=%d\n", rc));
            return rc;
        }

        if((rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, originalDuplexAutoNeg)) != GT_OK)
        {
            AUTODETECT_DBG_PRINT_MAC(("cpssDxChPortDuplexAutoNegEnableSet rc=%d\n", rc));
            return rc;
        }

        if((rc = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, portNum, originalFcAnState,
                                                                    originalPauseAdvertise)) != GT_OK)
        {
            AUTODETECT_DBG_PRINT_MAC(("cpssDxChPortFlowCntrlAutoNegEnableSet rc=%d\n", rc));
            return rc;
        }

        if((rc = cpssDxChPortFlowControlEnableSet(devNum, portNum, originalFcStatus)) != GT_OK)
        {
            AUTODETECT_DBG_PRINT_MAC(("cpssDxChPortFlowControlEnableSet rc=%d\n", rc));
            return rc;
        }

        if((rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum, originalInbandAutoNegBypass)) != GT_OK)
        {
            AUTODETECT_DBG_PRINT_MAC(("cpssDxChPortInBandAutoNegBypassEnableSet rc=%d\n", rc));
            return rc;
        }

        if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, originalInbandAutoNeg)) != GT_OK)
        {
            AUTODETECT_DBG_PRINT_MAC(("cpssDxChPortInbandAutoNegEnableSet rc=%d\n", rc));
            return rc;
        }

        if(GT_TRUE == originalInbandAutoNeg)
        {/* restart inband auto-negotiation */
            rc = prvCpssDxChInbandAutoNegRestartAndWait(devNum,portNum);
            if((rc != GT_OK) && (rc != GT_TIMEOUT))
            {
                AUTODETECT_DBG_PRINT_MAC(("prvCpssDxChInbandAutoNegRestartAndWait rc=%d\n", rc));
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortModeSpeedAutoDetectAndConfig function
* @endinternal
*
* @brief   Autodetect and configure interface mode and speed of a given port
*         If the process succeeded the port gets configuration ready for link,
*         otherwise (none of provided by application options valid) the port
*         is left in reset mode.
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port not supported, FE ports not supported)
* @param[in] portModeSpeedOptionsArrayPtr � array of port mode and speed options
*                                      preferred for application,
*                                      NULL � if any of supported options acceptable
* @param[in] optionsArrayLen          � length of options array (must be 0 if
* @param[in] portModeSpeedOptionsArrayPtr == NULL)
*
* @param[out] currentModePtr           � if succeeded ifMode and speed that were configured
*                                      on the port,
*                                      otherwise previous ifMode and speed restored
*
* @retval GT_OK                    - if process completed without errors (no matter if mode
*                                       configured or not)
* @retval GT_BAD_PARAM             - on wrong port number or device,
*                                       optionsArrayLen more than possible options
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_NOT_SUPPORTED     � ifMode/speed pair not supported,
*                                       wrong port type
* @retval GT_HW_ERROR              - HW error
*
* @note For list of supported modes see:
*       lionDefaultPortModeSpeedOptionsArray,
*       xcatDefaultNetworkPortModeSpeedOptionsArray,
*       xcatDefaultStackPortModeSpeedOptionsArray
*       There is no possibility to distinguish if partner has SGMII or
*       1000BaseX, when auto-negotiation disabled. Preference to 1000BaseX given
*       by algorithm.
*       Responsibility of application to mask link change interrupt during
*       auto-detection algorithm run.
*
*/
GT_STATUS cpssDxChPortModeSpeedAutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   CPSS_PORT_MODE_SPEED_STC *portModeSpeedOptionsArrayPtr,
    IN   GT_U8                    optionsArrayLen,
    OUT  CPSS_PORT_MODE_SPEED_STC *currentModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortModeSpeedAutoDetectAndConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portModeSpeedOptionsArrayPtr, optionsArrayLen, currentModePtr));

    rc = internal_cpssDxChPortModeSpeedAutoDetectAndConfig(devNum, portNum, portModeSpeedOptionsArrayPtr, optionsArrayLen, currentModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portModeSpeedOptionsArrayPtr, optionsArrayLen, currentModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortInbandAutoNegRestart function
* @endinternal
*
* @brief   Restart inband auto-negotiation. Relevant only when inband auto-neg.
*         enabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortInbandAutoNegRestart
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* support multi-port-groups device */
    GT_U32      portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroupId =
    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 4, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Note: minimum time before switch bit off 15 ns */

    /* due to FE-21290 - InBandReStartAn bit does not reset automatically,
       so switch it off by SW  */
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 4, 1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortInbandAutoNegRestart function
* @endinternal
*
* @brief   Restart inband auto-negotiation. Relevant only when inband auto-neg.
*         enabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortInbandAutoNegRestart
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInbandAutoNegRestart);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssDxChPortInbandAutoNegRestart(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* --------------------- debug functions --------------------------------- */


/**
* @internal prvDebugCpssDxChPortModeSpeedAutoDetectAndConfig function
* @endinternal
*
* @brief   Simple wrapper for port mode and speed auto-detection function
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvDebugCpssDxChPortModeSpeedAutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_MODE_SPEED_STC *portModeSpeedOptionsArrayPtr = NULL;  /* array of
                                                            options to test */
    GT_U8                    optionsArrayLen = 0;   /* length of options array */
    CPSS_PORT_MODE_SPEED_STC currentMode;   /* final interface mode/speed */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    rc = cpssDxChPortModeSpeedAutoDetectAndConfig(devNum,portNum,
            portModeSpeedOptionsArrayPtr,optionsArrayLen,&currentMode);

    cpssOsPrintf("rc=%d,ifMode=%d,speed=%d\n", rc, currentMode.ifMode, currentMode.speed);

    return rc;
}

/**
* @internal prvDebugCpssDxChPortSgmii_1G_AutoDetectAndConfig function
* @endinternal
*
* @brief   Simple wrapper for port mode and speed auto-detection function
*         with priority for SGMII 1G mode, needed because at the moment we
*         can't distinguish between 1000BaseX and SGMII
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvDebugCpssDxChPortSgmii_1G_AutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;   /* return code */
    GT_U8                    optionsArrayLen = 5;   /* length of options array */
    CPSS_PORT_MODE_SPEED_STC currentMode;   /* final interface mode/speed */

    /* array of options to test */
    CPSS_PORT_MODE_SPEED_STC testPortModeSpeedOptionsArray[] =
    {
        {CPSS_PORT_INTERFACE_MODE_XGMII_E,      CPSS_PORT_SPEED_20000_E},
        {CPSS_PORT_INTERFACE_MODE_XGMII_E,      CPSS_PORT_SPEED_10000_E},
        {CPSS_PORT_INTERFACE_MODE_RXAUI_E,      CPSS_PORT_SPEED_10000_E},
        {CPSS_PORT_INTERFACE_MODE_SGMII_E,      CPSS_PORT_SPEED_1000_E},
        {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, CPSS_PORT_SPEED_1000_E}
    };

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    rc = cpssDxChPortModeSpeedAutoDetectAndConfig(devNum,portNum,
            testPortModeSpeedOptionsArray,optionsArrayLen,&currentMode);

    cpssOsPrintf("rc=%d,ifMode=%d,speed=%d\n", rc, currentMode.ifMode, currentMode.speed);

    return rc;
}



