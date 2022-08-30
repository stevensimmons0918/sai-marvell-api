/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
 
/**
 * @file mpdMtd.c
 *	@brief PHY and driver specific PHY implementation.
 *	For all (supported) MTD PHYs
 *
 */

#include 	<mpdTypes.h>
#include    <mpd.h>
#include    <mpdPrv.h>
#include    <mpdDebug.h>

#include    <mtdApiTypes.h>
#include    <mtdAPI.h>
#include    <mtdAPIInternal.h>
#include    <mtdApiRegs.h>
#include    <mtdCunit.h>
#include    <mtdDiagnostics.h>
#include    <mtdEEE.h>
#include    <mtdFwDownload.h>
#include    <mtdHunit.h>
#include    <mtdHwCntl.h>
#include    <mtdHXunit.h>
#include    <mtdXgFastRetrain.h>
#include    <mtdXunit.h>
#include    <mtdInitialization.h>
#include    <fwimages/mtdFwDlSlave.h>

extern UINT_32 prvMpdDebugWriteFlagId;
PRV_MPD_MTD_OBJECT_STC *prvMpdMtdPortDb_ARR[MPD_MAX_PORT_NUMBER_CNS + 1];

/** DEBUG **/
static char prvMpdMtdCompName[] = "mtd";
static char prvMpdMtdPckName[] = "wrap";

static char prvMpdMtdFwFlagName[] = "fw";
static char prvMpdMtdFwFlagHelp[] = "debug FW load sequence";
static UINT_32 prvMpdMtdFwFlagId;

static char prvMpdMtdVctName[] = "vct";
static char prvMpdMtdVctHelp[] = "debug VCT sequence";
static UINT_32 prvMpdMtdVctFlagId;

static char prvMpdMtdLinkName[] = "link";
static char prvMpdMtdLinkHelp[] = "debug link change sequence";
UINT_32 prvMpdMtdLinkFlagId;

static char prvMpdMtdEeeName[] = "eee";
static char prvMpdMtdEeeHelp[] = "debug SF EEE";
static UINT_32 prvMpdMtdEeeFlagId;

static char prvMpdMtdConfigName[] = "config";
static char prvMpdMtdConfigHelp[] = "debug SF config sequence";
static UINT_32 prvMpdMtdConfigFlagId;

static char prvMpdMtdMtdPckName[] = "mtd";
static char prvMpdMtdInfoName[] = "info";
static char prvMpdMtdInfoHelp[] = "MTD informational";
UINT_32 prvMpdMtdInfoFlagId;

static char prvMpdMtdCriticalName[] = "critical";
static char prvMpdMtdCriticalHelp[] = "MTD critical info";
UINT_32 prvMpdMtdCriticalFlagId;

static char prvMpdMtdErrorName[] = "error";
static char prvMpdMtdErrorHelp[] = "MTD Error";
UINT_32 prvMpdMtdErrorFlagId;


static PRV_MPD_MTD_TO_MPD_CONVERT_STC prvMpdConvertMtdToMpdSpeed[] = {
																		{ 	MTD_SPEED_10M_HD,
																			MPD_AUTONEG_CAPABILITIES_TENHALF_CNS },
																		{ 	MTD_SPEED_10M_FD,
																			MPD_AUTONEG_CAPABILITIES_TENFULL_CNS },
																		{ 	MTD_SPEED_100M_HD,
																			MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS },
																		{ 	MTD_SPEED_100M_FD,
																			MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS },
																		{ 	MTD_SPEED_1GIG_HD,
																			MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS },
																		{ 	MTD_SPEED_1GIG_FD,
																			MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS },
																		{ 	MTD_SPEED_10GIG_FD,
																			MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS },
																		{ 	MTD_SPEED_2P5GIG_FD,
																			MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS },
																		{ 	MTD_SPEED_5GIG_FD,
																			MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS }
};

static PRV_MPD_MTD_TO_MPD_CONVERT_STC prvMpdConvertMtdToMpdEeeCapability[] = {
																				{ 	MTD_EEE_100M,
																					MPD_SPEED_CAPABILITY_100M_CNS },
																				{ 	MTD_EEE_1G,
																					MPD_SPEED_CAPABILITY_1G_CNS },
																				{ 	MTD_EEE_10G,
																					MPD_SPEED_CAPABILITY_10G_CNS },
																				{ 	MTD_EEE_2P5G,
																					MPD_SPEED_CAPABILITY_2500M_CNS },
																				{ 	MTD_EEE_5G,
																					MPD_SPEED_CAPABILITY_5G_CNS }
};

#ifndef PHY_SIMULATION 
static BOOLEAN prvMpdMtdPrintPhyFw_ARR[MPD_TYPE_NUM_OF_TYPES_E];

static  char *  prvMpdMtdConvertPhyNameToText_ARR[MPD_TYPE_NUM_OF_TYPES_E] = {
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "PHY-88X32x0",
    "PHY-88X33x0",
    "PHY-88X20X0",
    "PHY-88X2180",
    "PHY-88E2540",
    "PHY-88X3540",
    "NO FW",
    "PHY-88E2580",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
};
#endif
static UINT_16 prvMpdRepresentativeModulo_ARR [MPD_TYPE_NUM_OF_TYPES_E+1] = {0};

#define ERROR_STR(_x) #_x

#ifdef PHY_SIMULATION
#define PRV_MPD_MTD_CALL(_ret,_rel_ifIndex, _status) 												\
	_status = _ret;																					\
    PRV_MPD_DEBUG_LOG_PORT_MAC(prvMpdDebugWriteFlagId,_rel_ifIndex)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),	\
        "%s returned with status %d line %d",ERROR_STR(_ret), _status, __LINE__ );
#else
#define PRV_MPD_MTD_CALL(_ret,_rel_ifIndex, _status) 												\
	_status = _ret; 																				\
    PRV_MPD_DEBUG_LOG_PORT_MAC(prvMpdDebugWriteFlagId,_rel_ifIndex)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),	\
        "%s returned with status %d line %d",ERROR_STR(_ret), _status, __LINE__ );					\
	if (_status == MTD_FAIL){																		\
		PRV_MPD_HANDLE_FAILURE_MAC(_rel_ifIndex, MPD_ERROR_SEVERITY_ERROR_E, ERROR_STR(_ret));		\
		return MPD_OP_FAILED_E;																		\
	}
#endif
MTD_STATUS mtdRerunSerdesAutoInitialization
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit
);
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdConfigSerdesTx
 *
 * DESCRIPTION: ALL MTD
 *      
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdConfigSerdesTx(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	
    UINT_16 val, rel_ifIndex;
	MPD_RESULT_ENT status = MPD_OK_E;
	rel_ifIndex = (UINT_16) portEntry_PTR->rel_ifIndex;
	PRV_MPD_MTD_ASSERT_REL_IFINDEX(rel_ifIndex);
	if (portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.value_set) {
		val = 0;
		val |= (((portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.pre_tap) & 0x1F) << 11); /* 15:11 */
		val |= (((portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.main_tap) & 0x3F) << 5); /* 10:5  */
		val |= ((portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.post_tap) & 0x1F); /* 4:0 */

		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												3,
												0x29,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);
		if (status != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d. failed to write page 3, reg 0x29.",
                                                            portEntry_PTR->rel_ifIndex);
			return status;
		}
	}

	if (portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.value_set) {
		val = 0;
		val |= (((portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.pre_tap) & 0x1F) << 11); /* 15:11 */
		val |= (((portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.main_tap) & 0x3F) << 5); /* 10:5  */
		val |= ((portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.post_tap) & 0x1F); /* 4:0 */

		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												3,
												0x2A,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);
        if (status != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d. failed to write page 3, reg 0x2A.",
                                                            portEntry_PTR->rel_ifIndex);
            return status;
        }
	}

	if (portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.value_set || portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.value_set) {
		PRV_MPD_MTD_CALL(	mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 200),
							rel_ifIndex,
							status);
	}

	return status;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdValidateUsxSpeeds
 *
 * DESCRIPTION: validate speeds appropriate to usx mode.
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88E2580_E
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdValidateUsxSpeeds(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MTD_U16 mtdSpeed
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_PHY_USX_TYPE_ENT usxType;

    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;
    if (usxType == MPD_PHY_USX_TYPE_NONE){
        return MPD_OK_E;
    }

    switch (usxType){
    case MPD_PHY_USX_TYPE_OXGMII:
    case MPD_PHY_USX_TYPE_10G_QXGMII:
         /* The appropriate speed is up to MTD_SPEED_2P5GIG_FD */
        if ((mtdSpeed == MTD_SPEED_5GIG_FD) || (mtdSpeed == MTD_SPEED_10GIG_FD)){
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                  "rel_ifIndex %d desired speed 0x%x doesn't match usx mode capabilities",
                                  portEntry_PTR->rel_ifIndex,
                                  mtdSpeed);
            return MPD_OP_FAILED_E;
        }
        break;
    case MPD_PHY_USX_TYPE_10G_DXGMII:
    case MPD_PHY_USX_TYPE_20G_QXGMII:
         /* The appropriate speed is up to MTD_SPEED_5GIG_FD */
        if (mtdSpeed == MTD_SPEED_10GIG_FD){
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                  "rel_ifIndex %d desired speed 0x%x doesn't match usx mode capabilities",
                                  portEntry_PTR->rel_ifIndex,
                                  mtdSpeed);
            return MPD_OP_FAILED_E;
        }
        break;
    default:
        break;
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvConvertMpdUsxModeToMtdUsxMode
 *
 * DESCRIPTION: vconvert MPD usx mode to MTD usx mode.
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88E2580_E
 *****************************************************************************/
static MPD_RESULT_ENT prvConvertMpdUsxModeToMtdUsxMode(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    MTD_U16                    * mtdUsxType_PTR
)
{
    MPD_PHY_USX_TYPE_ENT usxType;
    MTD_U16              macType;

    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;
    if (usxType == MPD_PHY_USX_TYPE_NONE){
        return MPD_OK_E;
    }

    if (mtdUsxType_PTR == NULL)
        return MPD_OP_FAILED_E;

    switch (usxType){
    case MPD_PHY_USX_TYPE_OXGMII:
        macType = MTD_MAC_TYPE_OXGMII;
        break;
    case MPD_PHY_USX_TYPE_10G_QXGMII:
        macType = MTD_MAC_TYPE_10G_QXGMII;
        break;
    case MPD_PHY_USX_TYPE_10G_DXGMII:
        macType = MTD_MAC_TYPE_10G_DXGMII;
        break;
    case MPD_PHY_USX_TYPE_20G_QXGMII:
        macType = MTD_MAC_TYPE_20G_QXGMII;
        break;
    default:
        return MPD_OP_FAILED_E;
        break;
    }

    *mtdUsxType_PTR = macType;

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInitDebug
 *
 * DESCRIPTION: ALL MTD
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdInitDebug(
    /*     INPUTS:             */
    void
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdPckName,
                            prvMpdMtdFwFlagName,
                            prvMpdMtdFwFlagHelp,
                            &prvMpdMtdFwFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdPckName,
                            prvMpdMtdVctName,
                            prvMpdMtdVctHelp,
                            &prvMpdMtdVctFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdPckName,
                            prvMpdMtdLinkName,
                            prvMpdMtdLinkHelp,
                            &prvMpdMtdLinkFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdPckName,
                            prvMpdMtdEeeName,
                            prvMpdMtdEeeHelp,
                            &prvMpdMtdEeeFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdPckName,
                            prvMpdMtdConfigName,
                            prvMpdMtdConfigHelp,
                            &prvMpdMtdConfigFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdMtdPckName,
                            prvMpdMtdInfoName,
                            prvMpdMtdInfoHelp,
                            &prvMpdMtdInfoFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdMtdPckName,
                            prvMpdMtdCriticalName,
                            prvMpdMtdCriticalHelp,
                            &prvMpdMtdCriticalFlagId);

    PRV_MPD_DEBUG_BIND_MAC( prvMpdMtdCompName,
                            prvMpdMtdMtdPckName,
                            prvMpdMtdErrorName,
                            prvMpdMtdErrorHelp,
                            &prvMpdMtdErrorFlagId);


    return MPD_OK_E;

}

/*****************************************************************************
 * FUNCTION NAME: prvMpdMtdIsPhyReady
 *
 * DESCRIPTION: check if phy is ready after firmware download (max timeout 3 seconsds)
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdIsPhyReady(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC            *portEntry_PTR,
    PRV_MPD_PHY_READY_SEQUENCE_ENT          phySequence
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT ret_val = MPD_OK_E;
#ifndef PHY_SIMULATION
    MTD_BOOL phyReady = MTD_FALSE;
    UINT_32 rel_ifIndex;
    MTD_BOOL appStarted = MTD_FALSE;
    UINT_32 try, max_num_of_tries = 30;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    if (phySequence == PRV_MPD_PHY_READY_AFTER_FW_DOWNLOAD_E) {
        for (try = 1; try <= max_num_of_tries; try++) {
            PRV_MPD_MTD_CALL(   mtdDidPhyAppCodeStart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &appStarted),
                                                      rel_ifIndex,
                                                      mtdStatus);
            if (appStarted == MTD_TRUE)
            {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d, PHY apps start after FW download, try number %d",
                                                            rel_ifIndex,
                                                            try);
                break;
            }

            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, PHY apps didn't start after FW download, try number %d",
                                                        rel_ifIndex,
                                                        try);
            PRV_MPD_SLEEP_MAC(100);
        }
        if (try > max_num_of_tries) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, PHY apps didn't start after FW download, after %d tries",
                                                        rel_ifIndex,
                                                        max_num_of_tries);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_ERROR_E,
                                        "PHY apps didn't start after FW download\n");
            ret_val = MPD_OP_FAILED_E;
        }
    }
    
    for (try = 1; try <= max_num_of_tries; try++) {
        PRV_MPD_MTD_CALL(   mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady),
                            rel_ifIndex,
                            mtdStatus);
        if (phyReady == MTD_TRUE)
        {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, PHY ready after reset, try number %d",
                                                        rel_ifIndex,
                                                        try);
            break;
        }
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d, PHY not ready after reset, try number %d",
                                                    rel_ifIndex,
                                                    try);
        PRV_MPD_SLEEP_MAC(100);
    }
    if (try > max_num_of_tries) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d, PHY not ready after reset, after %d tries",
                                                    rel_ifIndex,
                                                    max_num_of_tries);
        PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                    MPD_ERROR_SEVERITY_ERROR_E,
                                    "PHY not ready after reset\n");
        ret_val = MPD_OP_FAILED_E;
    }
#endif
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(phySequence);
    return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit20x0
 *
 * DESCRIPTION: 
 *      
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit20x0(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_8 i;
	UINT_16 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MTD_BOOL is_tunit_ready;
	MTD_BOOL inLowPowerMode;
	MPD_UNUSED_PARAM(params_PTR);
    
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	PRV_MPD_MTD_CALL(	mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&inLowPowerMode),
						rel_ifIndex,
						mtdStatus);
	if (inLowPowerMode) {
		/* remove from low-power mode */
		PRV_MPD_MTD_CALL(	mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
							rel_ifIndex,
							mtdStatus);

		for (i = 0; i < 50; i++) {
			PRV_MPD_MTD_CALL(	mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &is_tunit_ready),
								rel_ifIndex,
								mtdStatus);
			if (is_tunit_ready == MTD_TRUE)
				break;
			PRV_MPD_SLEEP_MAC(20);
		}
#ifndef PHY_SIMULATION
		if (is_tunit_ready == MTD_FALSE) {
			PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"T Unit is not ready after 1 Sec\n");
		}
#endif

		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
		(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			"rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
			rel_ifIndex,
			(is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
			i + 1);

	}

	/* copper initialization */
	PRV_MPD_MTD_CALL(	mtdSetMacInterfaceControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_MAC_TYPE_XFI_SGMII_AN_EN,
						MTD_FALSE, /* TRUE = mac interface always power up */
						MTD_MAC_SNOOP_OFF,
						0,
						MTD_MAC_SPEED_10_GBPS,
						MTD_MAX_MAC_SPEED_5G,
						MTD_TRUE, MTD_TRUE),
						rel_ifIndex,
						mtdStatus);


	PRV_MPD_MTD_CALL(	mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_F2R_OFF,
						MTD_MS_CU_ONLY,
						MTD_FT_NONE,
						MTD_FALSE,
						MTD_FALSE,
						MTD_TRUE, MTD_TRUE),
						rel_ifIndex,
						mtdStatus);


#ifndef PHY_SIMULATION
	/* Enable Temperature Sensor*/
	PRV_MPD_MTD_CALL(	mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
						rel_ifIndex,
						mtdStatus);

#endif

	if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                            rel_ifIndex,
                            mtdStatus);
    }

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit33x0And32x0
 *
 * DESCRIPTION:
 *
 * note: if PHY is strapped to work in fiber mode, init will fail
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit33x0And32x0(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_8 i;
	UINT_16 rel_ifIndex;
	MTD_BOOL is_tunit_ready, inLowPowerMode;
	MTD_STATUS mtdStatus;
	MPD_UNUSED_PARAM(params_PTR);

	rel_ifIndex = portEntry_PTR->rel_ifIndex;

	PRV_MPD_MTD_CALL(	mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&inLowPowerMode),
						rel_ifIndex,
						mtdStatus);
	if (inLowPowerMode) {
		/* remove from low-power mode */
		PRV_MPD_MTD_CALL(	mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
							rel_ifIndex,
							mtdStatus);

		for (i = 0; i < 50; i++) {
			mtdIsTunitResponsive(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&is_tunit_ready);
			if (is_tunit_ready == MTD_TRUE)
				break;
			PRV_MPD_SLEEP_MAC(20);
		}
#ifndef PHY_SIMULATION
		if (is_tunit_ready == MTD_FALSE)
			PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"T Unit is not ready after 1 Sec\n");
#endif
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
		(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			"rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
			rel_ifIndex,
			(is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
			i + 1);
	}

	if (portEntry_PTR->initData_PTR->shortReachSerdes) {
        /* enable the SERDES short reach mode by using the following setting: 7.800F.10 (hunit_shortReach) = 1 and 7.800F.11 (xunit_shortReach) = 1*/

        PRV_MPD_MTD_CALL(   mtdSetSerdesOverrideReach(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),MTD_H_UNIT,MTD_TRUE),
                            rel_ifIndex,
                            mtdStatus);

        PRV_MPD_MTD_CALL(   mtdSetSerdesOverrideReach(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),MTD_X_UNIT,MTD_TRUE),
                            rel_ifIndex,
                            mtdStatus);

        /* tunit software reset */
        PRV_MPD_MTD_CALL(   mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 0),
                            rel_ifIndex,
                            mtdStatus);
    }

	/* copper initialization */
	PRV_MPD_MTD_CALL(	mtdSetMacInterfaceControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_MAC_TYPE_XFI_SGMII_AN_EN,
						MTD_FALSE, /* TRUE = mac interface always power up */
						MTD_MAC_SNOOP_OFF,
						0,
						MTD_MAC_SPEED_10_GBPS,
						MTD_MAX_MAC_SPEED_10G,
						MTD_TRUE, MTD_FALSE),
						rel_ifIndex,
						mtdStatus);


	PRV_MPD_MTD_CALL(	mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_F2R_OFF,
						MTD_MS_CU_ONLY,
						MTD_FT_10GBASER,
						MTD_FALSE,
						MTD_FALSE,
						MTD_TRUE, MTD_FALSE),
						rel_ifIndex,
						mtdStatus);


	PRV_MPD_MTD_CALL(	mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_ENABLE_NFR_ONLY,
						MTD_FR_IDLE,
						MTD_FALSE),
						rel_ifIndex,
						mtdStatus);

#ifndef PHY_SIMULATION
	/* Enable Temperature Sensor*/
	PRV_MPD_MTD_CALL(	mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
						rel_ifIndex,
						mtdStatus);
#endif
	/** if downshift supported on the device (might be not supported), Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
	 * this configuration requires SW reset but we don't do this here,
	 * SW reset will be done when port becomes admin up.
	 */
	mtdAutonegSetNbaseTDownshiftControl(
        PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
        0 /* disable downhift */,
        MTD_TRUE /* re-enable all speeds when no energy is detected */,
        MTD_FALSE /* disable downhift */,
        MTD_FALSE /* don't restart autoneg */);

	if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                            rel_ifIndex,
                            mtdStatus);
    }

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit2180
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit2180(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	UINT_8 i;
	UINT_16 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MTD_BOOL is_tunit_ready, inLowPowerMode;
	MPD_UNUSED_PARAM(params_PTR);

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	PRV_MPD_MTD_CALL(	mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&inLowPowerMode),
						rel_ifIndex,
						mtdStatus);
	if (inLowPowerMode) {
		/* remove from low-power mode */
		PRV_MPD_MTD_CALL(	mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
							rel_ifIndex,
							mtdStatus);

		for (i = 0; i < 50; i++) {
			mtdIsTunitResponsive(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&is_tunit_ready);
			if (is_tunit_ready == MTD_TRUE)
				break;
			PRV_MPD_SLEEP_MAC(20);
		}
#ifndef PHY_SIMULATION
		if (is_tunit_ready == MTD_FALSE)
			PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"T Unit is not ready after 1 Sec\n");
#endif
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
		(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			"rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
			rel_ifIndex,
			(is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
			i + 1);
	}
#ifndef PHY_SIMULATION
	/* Enable Temperature Sensor*/
	PRV_MPD_MTD_CALL(	mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
						rel_ifIndex,
						mtdStatus);
#endif
	/* copper initialization */
	PRV_MPD_MTD_CALL(	mtdSetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_MAC_TYPE_5BR_SGMII_AN_EN,
						MTD_FALSE, /* TRUE = mac interface always power up */
						MTD_MAC_SPEED_10_MBPS,
						MTD_MAX_MAC_SPEED_5G),
						rel_ifIndex,
						mtdStatus);

	PRV_MPD_MTD_CALL(	mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_ENABLE_NFR_ONLY,
						MTD_FR_IDLE,
						MTD_TRUE),
						rel_ifIndex,
						mtdStatus);


	/** Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
	 * this configuration requires SW reset but we don't do this here,
	 * SW reset will be done when port becomes admin up.
	 */
	PRV_MPD_MTD_CALL(	mtdAutonegSetNbaseTDownshiftControl(
						PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						0 /* disable downhift */,
						MTD_TRUE /* re-enable all speeds when no energy is detected */,
						MTD_FALSE /* disable downhift */,
						MTD_FALSE /* don't restart autoneg */),
						rel_ifIndex,
						mtdStatus);

	if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                            rel_ifIndex,
                            mtdStatus);
    }

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit3540
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit3540(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
    UINT_8 i;
    UINT_16 rel_ifIndex;
    MTD_U16 macType, currMacType, currMacLinkDownSpeed, currMacMaxIfSpeed;
    MTD_STATUS mtdStatus;
    MTD_BOOL is_tunit_ready, inLowPowerMode, currMacIfPowerDown, hostCanPowerDown = MTD_FALSE;
    MPD_PHY_USX_TYPE_ENT    usxType;
    BOOLEAN configureInterfaceMode = FALSE;
    MPD_RESULT_ENT mpdStatus;
    char error_msg [256];
    
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    PRV_MPD_MTD_CALL(	mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&inLowPowerMode),
                        rel_ifIndex,
                        mtdStatus);
    if (inLowPowerMode) {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(	mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                            rel_ifIndex,
                            mtdStatus);

        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                    &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE)
                break;
            PRV_MPD_SLEEP_MAC(20);
        }
#ifndef PHY_SIMULATION
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
                                        MPD_ERROR_SEVERITY_ERROR_E,
                                        "T Unit is not ready after 1 Sec\n");
#endif
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
            (	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
                rel_ifIndex,
                (is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
                i + 1);
    }
#ifndef PHY_SIMULATION
    /* Enable Temperature Sensor*/
    PRV_MPD_MTD_CALL(	mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                        rel_ifIndex,
                        mtdStatus);
#endif
    /* copper initialization */
    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;         
    if (usxType == MPD_PHY_USX_TYPE_NONE){
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_FALSE;
        macType = MTD_MAC_TYPE_10GR_SGMII_AN_EN;
    }
    else if ((((usxType == MPD_PHY_USX_TYPE_10G_DXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_DXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 2) == 0) ||
        (((usxType == MPD_PHY_USX_TYPE_10G_QXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_QXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0))
    {
        configureInterfaceMode = TRUE;
        mpdStatus = prvConvertMpdUsxModeToMtdUsxMode(portEntry_PTR, &macType);
        if (mpdStatus != MPD_OK_E) {
            sprintf(error_msg, "rel_ifIndex %d - Failed to convert MPD usx mode [%d] to MTD usx mode\n", rel_ifIndex, usxType);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_FATAL_E,
                                        error_msg);
            return MPD_OP_FAILED_E;
        }
        hostCanPowerDown = MTD_TRUE;
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
                             (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                             "rel_ifIndex %d , usxType is %d. macType is 0x%x",
                             rel_ifIndex,
                             usxType,
                             macType);
    }
    else if (((usxType == MPD_PHY_USX_TYPE_OXGMII) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0)){
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_TRUE;
        /* checking if the phy is master/slave */
        PRV_MPD_MTD_CALL(	mtdGetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            &currMacType,
                            &currMacIfPowerDown,
                            &currMacLinkDownSpeed,
                            &currMacMaxIfSpeed),
                            rel_ifIndex,
                            mtdStatus);
        if (currMacType == MTD_MAC_TYPE_OXGMII){
            macType = MTD_MAC_TYPE_OXGMII;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
                                 (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                 "rel_ifIndex %d: is master OXGMII.  usxType is %d. currentMacType is 0x%x",
                                 rel_ifIndex,
                                 usxType,
                                 currMacType);
        }
        else {
            macType = MTD_MAC_TYPE_OXGMII_PARTNER;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
                                 (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                 "rel_ifIndex %d: is slave OXGMII.  usxType is %d. currentMacType is 0x%x",
                                 rel_ifIndex,
                                 usxType,
                                 currMacType);
        }
    }
    if (configureInterfaceMode){
        PRV_MPD_MTD_CALL(	mtdSetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            macType,
                            hostCanPowerDown,
                            MTD_MAC_SPEED_NOT_APPLICABLE,
                            MTD_MAX_MAC_SPEED_NOT_APPLICABLE),
                            rel_ifIndex,
                            mtdStatus);
    }
	PRV_MPD_MTD_CALL(	mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_ENABLE_NFR_ONLY,
						MTD_FR_IDLE,
						MTD_FALSE),
						rel_ifIndex,
						mtdStatus);

	/** Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
	 * this configuration requires SW reset but we don't do this here,
	 * SW reset will be done when port becomes admin up.
	 */
	PRV_MPD_MTD_CALL(	mtdAutonegSetNbaseTDownshiftControl(
						PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						0 /* disable downhift */,
						MTD_TRUE /* re-enable all speeds when no energy is detected */,
						MTD_FALSE /* disable downhift */,
						MTD_FALSE /* don't restart autoneg */),
						rel_ifIndex,
						mtdStatus);

	if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                            rel_ifIndex,
                            mtdStatus);
    }

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit25x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit25x0(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	UINT_8 i;
	UINT_16 rel_ifIndex;
    MTD_U16 macType, currMacType, currMacLinkDownSpeed, currMacMaxIfSpeed;
	MTD_STATUS mtdStatus;
	MTD_BOOL is_tunit_ready, inLowPowerMode, currMacIfPowerDown, hostCanPowerDown = MTD_FALSE;
	MPD_PHY_USX_TYPE_ENT    usxType;
    BOOLEAN configureInterfaceMode = FALSE;
    MPD_RESULT_ENT mpdStatus;
    char error_msg [256];
    
    MPD_UNUSED_PARAM(params_PTR);
    
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	PRV_MPD_MTD_CALL(	mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&inLowPowerMode),
						rel_ifIndex,
						mtdStatus);
	if (inLowPowerMode) {
		/* remove from low-power mode */
		PRV_MPD_MTD_CALL(	mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
							rel_ifIndex,
							mtdStatus);

		for (i = 0; i < 50; i++) {
			mtdIsTunitResponsive(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&is_tunit_ready);
			if (is_tunit_ready == MTD_TRUE)
				break;
			PRV_MPD_SLEEP_MAC(20);
		}
#ifndef PHY_SIMULATION
		if (is_tunit_ready == MTD_FALSE)
			PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"T Unit is not ready after 1 Sec\n");
#endif
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
		(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			"rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
			rel_ifIndex,
			(is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
			i + 1);
	}
#ifndef PHY_SIMULATION
	/* Enable Temperature Sensor*/
	PRV_MPD_MTD_CALL(	mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
						rel_ifIndex,
						mtdStatus);
#endif
    /* copper initialization */
    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;         
    if (usxType == MPD_PHY_USX_TYPE_NONE){
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_FALSE;
        macType = MTD_MAC_TYPE_5BR_SGMII_AN_EN;
    }
    else if ((((usxType == MPD_PHY_USX_TYPE_10G_DXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_DXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 2) == 0) ||
        (((usxType == MPD_PHY_USX_TYPE_10G_QXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_QXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0))
    {
        configureInterfaceMode = TRUE;
        mpdStatus = prvConvertMpdUsxModeToMtdUsxMode(portEntry_PTR, &macType);
        if (mpdStatus != MPD_OK_E) {
            sprintf(error_msg, "rel_ifIndex %d - Failed to convert MPD usx mode [%d] to MTD usx mode\n", rel_ifIndex, usxType);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_FATAL_E,
                                        error_msg);
            return MPD_OP_FAILED_E;
        }
        hostCanPowerDown = MTD_TRUE;
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
                             (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                             "rel_ifIndex %d , usxType is %d. macType is 0x%x",
                             rel_ifIndex,
                             usxType,
                             macType);
    }
    else if (((usxType == MPD_PHY_USX_TYPE_OXGMII) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0)){
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_TRUE;
        /* checking if the phy is master/slave */
        PRV_MPD_MTD_CALL(   mtdGetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            &currMacType,
                            &currMacIfPowerDown,
                            &currMacLinkDownSpeed,
                            &currMacMaxIfSpeed),
                            rel_ifIndex,
                            mtdStatus);
        if (currMacType == MTD_MAC_TYPE_OXGMII){
            macType = MTD_MAC_TYPE_OXGMII;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
                                 (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                 "rel_ifIndex %d: is master OXGMII.  usxType is %d. currentMacType is 0x%x",
                                 rel_ifIndex,
                                 usxType,
                                 currMacType);
        }
        else {
            macType = MTD_MAC_TYPE_OXGMII_PARTNER;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)
                                 (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                 "rel_ifIndex %d: is slave OXGMII.  usxType is %d. currentMacType is 0x%x",
                                 rel_ifIndex,
                                 usxType,
                                 currMacType);
        }
    }

    if (configureInterfaceMode){
        PRV_MPD_MTD_CALL(	mtdSetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            macType,
                            hostCanPowerDown,
                            MTD_MAC_SPEED_NOT_APPLICABLE,
                            MTD_MAX_MAC_SPEED_NOT_APPLICABLE),
                            rel_ifIndex,
                            mtdStatus);
    }
	PRV_MPD_MTD_CALL(	mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						MTD_ENABLE_NFR_ONLY,
						MTD_FR_IDLE,
						MTD_FALSE),
						rel_ifIndex,
						mtdStatus);

	/** Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
	 * this configuration requires SW reset but we don't do this here,
	 * SW reset will be done when port becomes admin up.
	 */
	PRV_MPD_MTD_CALL(	mtdAutonegSetNbaseTDownshiftControl(
						PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						0 /* disable downhift */,
						MTD_TRUE /* re-enable all speeds when no energy is detected */,
						MTD_FALSE /* disable downhift */,
						MTD_FALSE /* don't restart autoneg */),
						rel_ifIndex,
						mtdStatus);

	if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                            rel_ifIndex,
                            mtdStatus);
    }

	return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdReset32x0
 *
 * DESCRIPTION: Utility, Perform PHY reset for MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/

static MPD_RESULT_ENT prvMpdMtdReset32x0(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_32 sleep_time_ms
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_32 rel_ifIndex;
	MTD_STATUS rc;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;

	if (portEntry_PTR->runningData_PTR->sfpPresent) {
	    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex fiber mode, removed sleep time %d",
                                                    rel_ifIndex, sleep_time_ms);
	    sleep_time_ms = 0;
	}
    
	PRV_MPD_MTD_CALL(	mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), sleep_time_ms),
						rel_ifIndex,
						rc);
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex call to mtdSoftwareReset, rc=%d",
													rel_ifIndex,
													rc);
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetSpeed_2
 *
 * DESCRIPTION: Get PHY operational link & speed
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetSpeed_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	MPD_RESULT_ENT ret = MPD_OK_E;
#ifndef PHY_SIMULATION
	UINT_32 rel_ifIndex;
	UINT_16 val, speed_val, duplex_val, bit2 = 4, bit5 = 0x20, bits15_14 = 0x3,
			mask_0 = 1;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
#endif

    
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
    params_PTR->phyInternalOperStatus.duplexMode = TRUE;
    params_PTR->phyInternalOperStatus.isOperStatusUp = FALSE;
#else
	rel_ifIndex = portEntry_PTR->rel_ifIndex;

	/* check media type */
	memset(	&phy_params,
			0,
			sizeof(phy_params));
	prvMpdPerformPhyOperation(	portEntry_PTR,
								MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
								&phy_params);

	params_PTR->phyInternalOperStatus.mediaType = phy_params.phyKindAndMedia.mediaType;

	if (phy_params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E) { /* handle fiber case for speed 10G only */
		/* read 7.1 - auto neg status. bit 2 = link status */
		mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						3,
						0x1001,
						&val);
		mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						3,
						0x1001,
						&val);
	}
	else
	{
		/* read 7.1 - auto neg status. bit 2 = link status */
		mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						7,
						1,
						&val);
		mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						7,
						1,
						&val);
	}

	params_PTR->phyInternalOperStatus.isOperStatusUp =
			(val & bit2) ? TRUE : FALSE;

	if (phy_params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E &&
			params_PTR->phyInternalOperStatus.isOperStatusUp == TRUE)
	{
		/* 7.1, bit 5 = is auto neg completed */
		params_PTR->phyInternalOperStatus.isAnCompleted =
				((val & bit5) != 0) ? TRUE : FALSE;

		params_PTR->phyInternalOperStatus.duplexMode = TRUE;
		mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						3,
						0x8008,
						&val);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdLinkFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"READ - rel_ifIndex %d device %d register 0x%x value 0x%x",
													rel_ifIndex,
													3,
													0x8008,
													val);
		/* read 3.0x8008, bits 14,15 = speed resolution */
		speed_val = (val >> 14) & bits15_14;
		switch (speed_val) {
			case 0:
				case 1:
				duplex_val = (val >> 13) & mask_0;
				params_PTR->phyInternalOperStatus.duplexMode =
						(duplex_val) ? TRUE : FALSE; /* TRUE - Full duplex*/
				if (speed_val == 0)
					params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10M_E;
				else
					params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_100M_E;
				break;
			case 2:
				params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
				break;
			case 3:
				params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
				break;
			default:
				params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
				PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdLinkFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"rel_ifIndex %d DEFAULT CASE val is 0x%x. fall-back to 10G",
															rel_ifIndex,
															val);
		}
	}
#endif
	return ret;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetSpeed_1
 *
 * DESCRIPTION: Get PHY operational link & speed
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetSpeed_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 val, bit2 = 4;
	UINT_16 speed_res, forcedSpeed;
	UINT_32 rel_ifIndex;
	MTD_BOOL speedIsForced;
	MTD_BOOL dummy, linkStatus;
	MPD_RESULT_ENT ret = MPD_OK_E;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;

#ifdef PHY_SIMULATION
    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
    params_PTR->phyInternalOperStatus.isOperStatusUp = FALSE;
#endif
	params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_LAST_E;

	/* if port is power down we should not check negotiation status -
	 * after shutdown and restart phy2180 negotiation status ended with oper status up while the port was in power down */
	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
		params_PTR->phyInternalOperStatus.isOperStatusUp = FALSE;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdLinkFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d:  admin down state",
													rel_ifIndex);
		return ret;
	}
	else {
		/* check media type */
		memset(	&phy_params,
				0,
				sizeof(phy_params));
		prvMpdPerformPhyOperation(	portEntry_PTR,
									MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
									&phy_params);

		params_PTR->phyInternalOperStatus.mediaType = phy_params.phyKindAndMedia.mediaType;

		if (phy_params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E) { /* handle fiber case */

			if (mtdGet10GBRStatus1(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									MTD_X_UNIT,
									&dummy,
									&dummy,
									&dummy,
									&dummy,
									&dummy,
									&dummy,
									&linkStatus) != MTD_OK){
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													            "rel_ifIndex %d:  mtdGet10GBRStatus1 failed",
													            rel_ifIndex);          
				return MPD_OP_FAILED_E;
            }
			if (linkStatus == MTD_TRUE) {
				params_PTR->phyInternalOperStatus.isOperStatusUp = TRUE;
				params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
			}
			else if (mtdGet1000BXSGMIIStatus(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
												MTD_X_UNIT,
												&dummy,
												&dummy,
												&dummy,
												&linkStatus) != MTD_OK){
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex %d:  mtdGet1000BXSGMIIStatus failed",
                                                                rel_ifIndex);
                return MPD_OP_FAILED_E;
            }

			if (linkStatus == MTD_TRUE) {
				params_PTR->phyInternalOperStatus.isOperStatusUp = TRUE;
				params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
			}

			else {
				params_PTR->phyInternalOperStatus.isOperStatusUp = FALSE;
			}

			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdLinkFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d:  Fiber media. Oper status is %s",
														rel_ifIndex,
														(params_PTR->phyInternalOperStatus.isOperStatusUp == TRUE) ? "UP" : "DOWN");
		}

		/* copper case */
		else {

			/* read 7.1 - auto neg status. bit 2 = link status */
			mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
							7,
							1,
							&val);
			mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
							7,
							1,
							&val);
			params_PTR->phyInternalOperStatus.isOperStatusUp =
					(val & bit2) ? TRUE : FALSE;

			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdLinkFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d:  Copper media. Oper status is %s",
														rel_ifIndex,
														(val & bit2) ? "UP" : "DOWN");

			if (params_PTR->phyInternalOperStatus.isOperStatusUp == TRUE) {
				if (mtdGetAutonegSpeedDuplexResolution(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
														&speed_res) != MTD_OK){
                    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                    "rel_ifIndex %d:  mtdGetAutonegSpeedDuplexResolution failed",
                                                                    rel_ifIndex);
                    ret = MPD_OP_FAILED_E;
                }

				params_PTR->phyInternalOperStatus.duplexMode = TRUE;

				switch (speed_res) {

					case MTD_SPEED_10M_HD: /* 0x0001 // 10BT half-duplex*/
						params_PTR->phyInternalOperStatus.duplexMode = FALSE;
						/* fall through */
					case MTD_SPEED_10M_FD: /*       0x0002 // 10BT full-duplex*/
						params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10M_E;
						break;
					case MTD_SPEED_100M_HD: /*  0x0004 // 100BASE-TX half-duplex*/
						params_PTR->phyInternalOperStatus.duplexMode = FALSE;
						/* fall through */
					case MTD_SPEED_100M_FD: /*  0x0008 // 100BASE-TX full-duplex*/
						params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_100M_E;
						break;
					case MTD_SPEED_1GIG_HD: /*      0x0010 // 1000BASE-T half-duplex*/
						params_PTR->phyInternalOperStatus.duplexMode = FALSE;
						/* fall through */
					case MTD_SPEED_1GIG_FD: /*      0x0020 // 1000BASE-T full-duplex*/
						params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
						break;
					case MTD_SPEED_10GIG_FD: /*     0x0040 // 10GBASE-T full-duplex*/
						params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
						break;
					case MTD_SPEED_2P5GIG_FD: /*    0x0800 // 2.5GBASE-T full-duplex, 88X33X0 family only*/
						params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_2500M_E;
						break;
					case MTD_SPEED_5GIG_FD: /*      0x1000 5Gig */
						params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_5000M_E;
						break;
					case MTD_ADV_NONE:
						/* check for forced speed */
						if (mtdGetForcedSpeed(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
												&speedIsForced,
												&forcedSpeed) != MTD_OK) {
                            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                            "rel_ifIndex %d:  mtdGetForcedSpeed failed",
                                                                            rel_ifIndex);
							ret = MPD_OP_FAILED_E;
						}
						else if (speedIsForced == MTD_TRUE) {
							if (forcedSpeed == MTD_SPEED_10M_HD_AN_DIS || forcedSpeed == MTD_SPEED_10M_FD_AN_DIS)
								params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10M_E;
							else
								/* MTD_SPEED_100M_HD_AN_DIS 0x0200 || MTD_SPEED_100M_FD_AN_DIS */
								params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_100M_E;

							params_PTR->phyInternalOperStatus.duplexMode =
									(forcedSpeed == MTD_SPEED_10M_HD_AN_DIS || forcedSpeed == MTD_SPEED_100M_HD_AN_DIS) ? FALSE : TRUE;
						}
                        else{
                            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                            "rel_ifIndex %d:  speedIsForced  = MTD_FALSE",
                                                                            rel_ifIndex);
                            ret = MPD_OP_FAILED_E;
                        }
							
						break;
				}
			}
		} /* copper case */
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdLinkFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d:  speed resolution: %#x, duplex: %s",
												rel_ifIndex,
												params_PTR->phyInternalOperStatus.phySpeed,
												(params_PTR->phyInternalOperStatus.duplexMode) ? "FULL" : "HALF");

	return ret;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetLpAnCapabilities
 *
 * DESCRIPTION: Get link partner auto negotiation capabilities
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetLpAnCapabilities(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 ieeeSpeed, nbSpeed;
	UINT_32 i;
	UINT_32 rel_ifIndex;
	UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdSpeed) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
	MTD_STATUS rc;
	MPD_RESULT_ENT ret = MPD_OK_E;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	params_PTR->phyRemoteAutoneg.capabilities = 0;

	rc = mtdGetLPAdvertisedSpeeds(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&ieeeSpeed,
									&nbSpeed);
	if (rc != MTD_OK || ieeeSpeed == MTD_ADV_NONE) {
		params_PTR->phyRemoteAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  prvMpdMtdGetLpAnCapabilities return not supported",
                                                        rel_ifIndex);
        ret = MPD_NOT_SUPPORTED_E;
	}

	for (i = 0; i < arr_size; i++) {
		if (ieeeSpeed & prvMpdConvertMtdToMpdSpeed[i].mtd_val) {
			params_PTR->phyRemoteAutoneg.capabilities |= prvMpdConvertMtdToMpdSpeed[i].hal_val;
		}
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d ieeeSpeed 0x%x and returned capabilities 0x%x",
													rel_ifIndex,
													ieeeSpeed,
													params_PTR->phyRemoteAutoneg.capabilities);

	return ret;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetMdixMode
 *
 * DESCRIPTION: Set MDI/X mode (mdi/midx/auto)
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetMdixMode(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
    BOOLEAN restartAn;
	UINT_16 val;
	UINT_16 sf_mdix_arr[] = { 	MTD_FORCE_MDI, /* MPD_MDI_MODE_MEDIA_E  */
								MTD_FORCE_MDIX, /* MPD_MDIX_MODE_MEDIA_E */
								MTD_AUTO_MDI_MDIX }; /* MPD_AUTO_MODE_MEDIA_E */
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdStatus = mtdGetMDIXControl(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&val);
	if (mtdStatus != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  mtdGetMDIXControl failed",
                                                        rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	if ((val & 3) != sf_mdix_arr[params_PTR->phyMdix.mode]) {

	    /* if port is UP - restart auto-neg */
	    restartAn = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? TRUE : FALSE;

		PRV_MPD_MTD_CALL(	mtdMDIXControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), sf_mdix_arr[params_PTR->phyMdix.mode], restartAn),
							rel_ifIndex,
							mtdStatus);

		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d config 0x%x restartAn %d",
														rel_ifIndex,
														sf_mdix_arr[params_PTR->phyMdix.mode],
														restartAn);
	}
	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetAdvertiseFc
 *
 * DESCRIPTION: Configre flow control advertisement capabilities
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetAdvertiseFc(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 val, forcedSpeed;
	MTD_BOOL isSpeedForced = MTD_FALSE, anRestart = MTD_TRUE;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdStatus = mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
					            7,
					            16,
					            &val);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
	PRV_MPD_MTD_CALL(	mtdGetForcedSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&isSpeedForced,&forcedSpeed),
						rel_ifIndex,
						mtdStatus);
	if (isSpeedForced == MTD_TRUE) {
		anRestart = MTD_FALSE;
	}

	/* symetric pause - bit 10 */
	if ((val & 0x400) && params_PTR->phyFc.advertiseFc == FALSE) {

		PRV_MPD_MTD_CALL(	mtdSetPauseAdvertisement(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_CLEAR_PAUSE, anRestart),
							rel_ifIndex,
							mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d clear pause",
														rel_ifIndex);
		if (mtdStatus != MTD_OK)
		{
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d:  mtdSetPauseAdvertisement failed",
                                                            rel_ifIndex);
			return MPD_OP_FAILED_E;
		}
	}
	else if (((val & 0x400) == FALSE) && params_PTR->phyFc.advertiseFc == TRUE) {

		PRV_MPD_MTD_CALL(	mtdSetPauseAdvertisement(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_SYM_PAUSE, anRestart),
							rel_ifIndex,
							mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d config sym pause",
														rel_ifIndex);
		if (mtdStatus != MTD_OK)
		{
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d:  mtdSetPauseAdvertisement failed",
                                                            rel_ifIndex);
			return MPD_OP_FAILED_E;
		}
	}

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetMdixAdminMode
 *
 * DESCRIPTION: Get admin (configured) MDI/X mode
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetMdixAdminMode(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 val;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdStatus = mtdGetMDIXControl(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&val);
	if (mtdStatus != MTD_OK){
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  mtdGetMDIXControl failed",
                                                        rel_ifIndex);
		return MPD_OP_FAILED_E;
	}
	switch (val) {
		case MTD_FORCE_MDI:
			params_PTR->phyMdix.mode = MPD_MDI_MODE_MEDIA_E;
			break;
		case MTD_FORCE_MDIX:
			params_PTR->phyMdix.mode = MPD_MDIX_MODE_MEDIA_E;
			break;
		case MTD_AUTO_MDI_MDIX:
			params_PTR->phyMdix.mode = MPD_AUTO_MODE_MEDIA_E;
			break;
		default:
			return MPD_OP_FAILED_E;

	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetMdixMode
 *
 * DESCRIPTION: Get operational (resolved) MDI/X mode
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetMdixMode(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_8 val;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdStatus = mtdGetMDIXResolution(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
										&val);
	if (mtdStatus != MTD_OK || val == MTD_MDI_MDIX_NOT_RESOLVED)
	{
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  mtdGetMDIXResolution failed",
                                                        rel_ifIndex);
        return MPD_OP_FAILED_E;
	}

	params_PTR->phyMdix.mode =
			(val == MTD_MDIX_RESOLVED) ? MPD_MDIX_MODE_MEDIA_E : MPD_MDI_MODE_MEDIA_E;

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d result 0x%x return mode %s",
													rel_ifIndex,
													val,
													(params_PTR->phyMdix.mode == MPD_MDIX_MODE_MEDIA_E) ? "MDIX" : "MDI");

	return MPD_OK_E;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetLpAdvFc
 *
 * DESCRIPTION: Get link partner advertised flow control capabilities
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetLpAdvFc(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_8 val;
	UINT_32 rel_ifIndex;
	MPD_RESULT_ENT ret = MPD_OK_E;
    
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	if (mtdGetLPAdvertisedPause(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								&val) != MTD_OK) {
		params_PTR->phyLinkPartnerFc.pauseCapable =
				((val & MTD_SYM_PAUSE) == 0) ? FALSE : TRUE;

		params_PTR->phyLinkPartnerFc.asymetricRequested =
				(val && (val & ~MTD_SYM_PAUSE) == 0) ? TRUE : FALSE;
	}
	else {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  prvMpdMtdGetLpAdvFc return not supported",
                                                        rel_ifIndex);
        ret = MPD_NOT_SUPPORTED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d result 0x%x return pause capable %s",
													rel_ifIndex,
													val,
													(params_PTR->phyLinkPartnerFc.pauseCapable == FALSE) ? "NO" : "YES");

	return ret;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdRestartAutoNeg
 *
 * DESCRIPTION: Reset auto-negotiation
 * 	Reset PHY will loose configuration, it is not exposed to application.
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdMtdRestartAutoNeg(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MPD_UNUSED_PARAM(params_PTR);

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	PRV_MPD_MTD_CALL(	mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
						rel_ifIndex,
						mtdStatus);
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d call to mtdAutonegRestart ",
													rel_ifIndex);
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDisable_2
 *
 * DESCRIPTION:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDisable_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 rel_ifIndex, i;
    MTD_STATUS mtdStatus;
    MTD_BOOL is_tunit_ready, enable;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "config rel_ifIndex %d, admin %s",
                                                            rel_ifIndex, (params_PTR->phyDisable.forceLinkDown == TRUE) ? "DOWN" : "UP");

    if (params_PTR->phyDisable.forceLinkDown == TRUE) {
        /* put in low power mode */
        PRV_MPD_MTD_CALL(   mtdPutPhyInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                            rel_ifIndex,
                            mtdStatus);
    }
    else {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(   mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                            rel_ifIndex,
                            mtdStatus);

        if (!portEntry_PTR->runningData_PTR->sfpPresent) {
            PRV_MPD_MTD_CALL(   mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 500),
                            rel_ifIndex,
                            mtdStatus);

            /* check t unit is ready  */
            for (i = 0; i < 50; i++) {
                mtdIsTunitResponsive(   PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                        &is_tunit_ready);
                if (is_tunit_ready == MTD_TRUE)
                    break;
                PRV_MPD_SLEEP_MAC(20);
            }
    #ifndef PHY_SIMULATION
            if (is_tunit_ready == MTD_FALSE)
                PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                            MPD_ERROR_SEVERITY_ERROR_E,
                                            "T Unit is not ready after 1 Sec\n");
    #endif
        }
    }

    if (portEntry_PTR->initData_PTR->transceiverType != MPD_TRANSCEIVER_COPPER_E) {
        /* Disable/ enable Fiber transceiver through application */
        enable = (params_PTR->phyDisable.forceLinkDown == FALSE) ? TRUE : FALSE;
        PRV_MPD_TRANSCEIVER_ENABLE_MAC( portEntry_PTR->rel_ifIndex,
                                        enable);
    }

    return mtdStatus == MTD_OK ? MPD_OK_E : MPD_OP_FAILED_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDisable_3
 *
 * DESCRIPTION:
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *					MPD_TYPE_88X20x0_E
 *					MPD_TYPE_88X2180_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDisable_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 rel_ifIndex, i;
	MTD_STATUS mtdStatus;
	MTD_BOOL is_tunit_ready, enable;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "config rel_ifIndex %d, admin %s",
                                                            rel_ifIndex, (params_PTR->phyDisable.forceLinkDown == TRUE) ? "DOWN" : "UP");

	if (params_PTR->phyDisable.forceLinkDown == TRUE) {
        /* put in low power mode */
        PRV_MPD_MTD_CALL(	mtdPutTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                            rel_ifIndex,
                            mtdStatus);
	}
	else {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(   mtdRemoveTunitLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                            rel_ifIndex,
                            mtdStatus);

        PRV_MPD_MTD_CALL(   mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 500),
                            rel_ifIndex,
                            mtdStatus);

        /* check t unit is ready  */
        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(   PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                    &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE)
                break;
            PRV_MPD_SLEEP_MAC(20);
        }
#ifndef PHY_SIMULATION
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_ERROR_E,
                                        "T Unit is not ready after 1 Sec\n");
#endif
	}

	if (portEntry_PTR->initData_PTR->transceiverType != MPD_TRANSCEIVER_COPPER_E) {
        /* Disable/ enable Fiber transceiver through application */
        enable = (params_PTR->phyDisable.forceLinkDown == FALSE) ? TRUE : FALSE;
        PRV_MPD_TRANSCEIVER_ENABLE_MAC( portEntry_PTR->rel_ifIndex,
                                        enable);
    }

	return mtdStatus == MTD_OK ? MPD_OK_E : MPD_OP_FAILED_E;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdVctUtility
 *
 * DESCRIPTION: Utility, Set SolarFlare VCT test - utility used for test2 and test3
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdVctUtility(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_8 terminitationCode[4] = { 0,
									0,
									0,
									0 }, length[4] = {	0,
														0,
														0,
														0 };
	UINT_8 max_termination_code = 0;
	MTD_BOOL testDone = MTD_FALSE;
	UINT_32 i, num_of_tries = 60, num_of_pairs = 4, sleep_time_ms = 100,
			len = 0;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus = MTD_FAIL;
	MPD_RESULT_ENT ret_val = MPD_OK_E;
	MPD_VCT_RESULT_ENT vct_res_convert_arr[] = { 	MPD_VCT_RESULT_BAD_CABLE_E,
													MPD_VCT_RESULT_CABLE_OK_E,
													MPD_VCT_RESULT_OPEN_CABLE_E,
													MPD_VCT_RESULT_SHORT_CABLE_E,
													MPD_VCT_RESULT_SHORT_CABLE_E };

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	/* Perform VCT test - then restore the WA  */
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"start VCT test on rel_ifIndex %d",
												rel_ifIndex);
	/* start VCT test - TDR */
	PRV_MPD_MTD_CALL(	mtdRun_CableDiagnostics(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_TRUE/*break link*/, MTD_FALSE/*disableInterPairShort*/),
						rel_ifIndex,
						mtdStatus);

	/* sleep and check up to 5 seconds */
	for (i = 0; i < num_of_tries; i++) {
		/* sleep 100 ms and check whether the test has finished */
		PRV_MPD_SLEEP_MAC(sleep_time_ms);
		mtdStatus = mtdGet_CableDiagnostics_Status(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
													&testDone);
		if ((mtdStatus == MTD_OK) && (testDone == MTD_TRUE)) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"Exit in mtdGet_CableDiagnostics_Status. rel_ifIndex %d, i = %d",
														rel_ifIndex,
														i);
			break;
		}
	}

	/* test succeeded */
	if ((i < num_of_tries) && (testDone == MTD_TRUE)) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"VCT test ended on rel_ifIndex %d, i=%d, test duration: %d[ms]",
													rel_ifIndex,
													i,
													(i + 1) * sleep_time_ms);

		PRV_MPD_MTD_CALL(mtdGet_CableDiagnostics_Results(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
												terminitationCode,
												length),rel_ifIndex,mtdStatus);

		for (i = 0; i < num_of_pairs; i++) {
			if (length[i] >= 0xF0) {
				PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"rel_ifIndex [%d], pair No. [%d], Length=[%d], override with 1 terminitationCode=[%d]",
															rel_ifIndex,
															i + 1,
															length[i],
															terminitationCode[i]);
				length[i] = 0;

			}
			else {
				PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"rel_ifIndex [%d], pair No. [%d], Length=[%d], terminitationCode=[%d]",
															rel_ifIndex,
															i + 1,
															length[i],
															terminitationCode[i]);
			}
			len += length[i];
			if (max_termination_code < terminitationCode[i]) {
				max_termination_code = terminitationCode[i];
			}
		}

		/* average of all pairs length */
		params_PTR->phyVct.cableLength = len / (num_of_pairs);

		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d, i=%d, Total Length: %d, Avg Len: %d",
													rel_ifIndex,
													i,
													len,
													params_PTR->phyVct.cableLength);

		if (max_termination_code > PRV_MPD_vctResult_interPairShort_CNS) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d. phy_op_failed");

			ret_val = MPD_OP_FAILED_E;
		}

		params_PTR->phyVct.testResult = vct_res_convert_arr[max_termination_code];
	}
	else /* test failed */
	{
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"VCT test failed on rel_ifIndex %d",
													rel_ifIndex);

		ret_val = MPD_OP_FAILED_E; /* VCT failed */
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"End of VCT test on rel_ifIndex %d",
												rel_ifIndex);

	/* END WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */
	return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetVctTest
 *
 * DESCRIPTION: Set SolarFlare VCT test
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetVctTest(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	BOOLEAN energyDetetctEnabled = FALSE, exitReset = FALSE;
	UINT_16 val = 0;
	UINT_32 i, num_of_tries = 60, sleep_time_ms = 100;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus = MTD_FAIL;
	MPD_RESULT_ENT ret_val = MPD_OK_E;
	MTD_BOOL phyReady = MTD_FALSE;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	/* if port is down, VCT should not run */
	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d admin is Down - VCT should not run",
													rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"START VCT WA - disable energy detect on rel_ifIndex %d",
												rel_ifIndex);

	/* START WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */
	/* disable energy detect - set 3.8000.9:8 to 00 */
	PRV_MPD_MTD_CALL(	prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, 3,0x8000, &val),
						rel_ifIndex,
						mtdStatus);
	energyDetetctEnabled = ((val >> 8) & 0x3) ? TRUE : FALSE;

	if (energyDetetctEnabled) {
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 0),
							rel_ifIndex,
							mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"WA - rel_ifIndex %d energy detect disabled",
													rel_ifIndex);

		/* software reset to PHY - set 1.0000.15 to 1 (soft reset) and wait until 1.0000.15 self clears */
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0x0, 15, 1, 1),
							rel_ifIndex,
							mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"WA - rel_ifIndex %d reset PHY",
													rel_ifIndex);

		/* sleep and check up to 5 seconds */
		for (i = 0; i < num_of_tries; i++) {
			/* sleep 100 ms and check whether the test has finished */
			PRV_MPD_SLEEP_MAC(sleep_time_ms);
			PRV_MPD_MTD_CALL(   mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady),
                                rel_ifIndex,
                                mtdStatus);
            if (phyReady == MTD_TRUE) {
				PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"Exit after reset. rel_ifIndex %d, i = %d",
															rel_ifIndex,
															i);
				exitReset = TRUE;
				break;
			}
		}
		if (exitReset == FALSE)
			PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"prvMpdMtdSetVctTest - PHY not exit from RESET");
	}

	/* run VCT test */
	ret_val = prvMpdMtdVctUtility(	portEntry_PTR,
									params_PTR);
	if (ret_val != MPD_OK_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"VCT test result:  rel_ifIndex %d, ret_val = %d",
													rel_ifIndex,
													ret_val);
	}
	else {
		/* if length is greater than 110 - rerun test */
		if (params_PTR->phyVct.cableLength > 110 ||
				(params_PTR->phyVct.cableLength == 0 && params_PTR->phyVct.testResult == MPD_VCT_RESULT_CABLE_OK_E)) {
			prvMpdMtdVctUtility(portEntry_PTR,
								params_PTR);
		}
	}

	/* restore WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */
	if (energyDetetctEnabled) {

		/* restore energy detect - set 3.8000.9:8 back to 2b?11 */
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 3),
							rel_ifIndex,
							mtdStatus);

		/* software reset to PHY - set 1.0000.15 to 1 (soft reset) and wait until 1.0000.15 self clears */
		/*    EXTHWP_SFPhyReset(rel_ifIndex, PRV_MPD_sleep_time_forDownload_CNS);*/
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0x0, 15, 1, 1),
							rel_ifIndex,
							mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"WA - rel_ifIndex %d reset PHY",
													rel_ifIndex);

	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"RESTORE VCT WA - DONE- on rel_ifIndex %d",
												rel_ifIndex);
	/* END WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */

	return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetCableLenghNoBreakLink
 *
 * DESCRIPTION: Get cable length without breaking the link
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetCableLenghNoBreakLink(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_8 terminitationCode[4] = { 0,
									0,
									0,
									0 }, length[4] = {	0,
														0,
														0,
														0 };
	MTD_BOOL testDone = MTD_FALSE;
	UINT_32 rel_ifIndex;
	UINT_32 i, num_of_pairs = 4, len = 0;
	UINT_32 num_of_tries = 100, sleep_time_ms = 20;
	MTD_STATUS mtdStatus;
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"Get passive cable length on rel_ifIndex %d",
												rel_ifIndex);

	/* start VCT test - DSP */
	PRV_MPD_MTD_CALL(	mtdRun_CableDiagnostics(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_FALSE/*break link*/, MTD_TRUE/*disableInterPairShort*/),
						rel_ifIndex,
						mtdStatus);



	for (i = 0; i < num_of_tries; i++) {
		mtdStatus = mtdGet_CableDiagnostics_Status(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
													&testDone);
		if ((mtdStatus == MTD_OK) && (testDone == MTD_TRUE)) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"Exit in mtdGet_CableDiagnostics_Status. rel_ifIndex %d, i = %d",
														rel_ifIndex,
														i);
			break;
		}
		/* sleep 100 ms and check whether the test has finished */
		PRV_MPD_SLEEP_MAC(sleep_time_ms);
	}

	if ((i < num_of_tries) && (testDone == MTD_TRUE)) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d, i=%d, test duration: %d[ms]",
													rel_ifIndex,
													i,
													i * sleep_time_ms);

		PRV_MPD_MTD_CALL(mtdGet_CableDiagnostics_Results(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
												terminitationCode,
												length),rel_ifIndex,mtdStatus);

		for (i = 0; i < num_of_pairs; i++) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d, pair No. %d, Length=%d, terminitationCode=%d",
														rel_ifIndex,
														i + 1,
														length[i],
														terminitationCode[i]);

			len += length[i];
			/* in case a pair is not OK - break and return this result */
			if (terminitationCode[i] != PRV_MPD_vctResult_pairOk_CNS) {
				PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"rel_ifIndex %d, pair No. %d, pair is not OK",
															rel_ifIndex,
															i + 1);

				return MPD_NOT_SUPPORTED_E;
			}
		}

		/* average of 4 pairs length */
		len /= num_of_pairs;

		if (len <= 50) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_LESS_THAN_50M_E;
		}
		else if (len <= 80) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_50M_80M_E;
		}
		else if (len <= 110) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_80M_110M_E;
		}
		else if (len <= 140) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_110M_140M_E;
		}
		else {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_MORE_THAN_140M_E;
		}
		params_PTR->phyCableLen.accurateLength = len;

		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d, Length=%d, phy_cableLen.accurateLength=%d",
													rel_ifIndex,
													len,
													params_PTR->phyCableLen.accurateLength);
	}
	else {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"VCT-DSP (no-break link) test failed on rel_ifIndex %d",
													rel_ifIndex);

		return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdUpdateParallelList
 *
 * DESCRIPTION: Update parallel list to download fw into the flash/ram:
 *              To flash - The number of ports to download should be one per phy.
 *              To ram - The number of ports to download should be all the ports in the same phyType,InterfaceId, device \ 
 *                       because it done per port.
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X2580_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdUpdateParallelList(
	/*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_FW_DOWNLOAD_TYPE_ENT    fwDownloadType,
	/*     INPUTS / OUTPUTS:   */
    BOOLEAN *repPerPhyNumber_ARR
	/*     OUTPUTS:            */
)
{
    UINT_32 phyNumber;
    BOOLEAN isRepresentative = FALSE;
    MPD_TYPE_ENT    phyType;
      
    phyType = portEntry_PTR->initData_PTR->phyType;
    if (phyType >=  MPD_TYPE_NUM_OF_TYPES_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									"prvMpdMtdUpdateParallelList - Illegal PHY type");
		return MPD_OP_FAILED_E;
    }
    phyNumber = portEntry_PTR->initData_PTR->phyNumber;
    if (prvMpdRepresentativeModulo_ARR[phyType] != 0){
        if (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % prvMpdRepresentativeModulo_ARR[phyType] == 0){
            isRepresentative = TRUE;
        }
    }
    else if (repPerPhyNumber_ARR[phyNumber] == FALSE){
        isRepresentative = TRUE;
        repPerPhyNumber_ARR[phyNumber] = TRUE;
    }
    
    if (fwDownloadType == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        isRepresentative = TRUE;
    }
    if (isRepresentative) {
        portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list[portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports] = 
                                        portEntry_PTR->initData_PTR->mdioInfo.mdioAddress;
        portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports++;
        if (portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
    		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
    									MPD_ERROR_SEVERITY_ERROR_E,
    									"prvMpdMtdUpdateParallelList - too many ports to download to");
    		return MPD_OP_FAILED_E;

        }
        portEntry_PTR->runningData_PTR->isRepresentative = TRUE;
    }
    
    return MPD_OK_E;
}


/*****************************************************************************
 * FUNCTION NAME: prvMpdMtdPrintFwVersion
 *
 * DESCRIPTION: 
 *      
 *
 *****************************************************************************/
 MPD_RESULT_ENT prvMpdMtdPrintFwVersion(
     /*!     INPUTS:             */
     PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
     BOOLEAN    beforeFwDownload
     /*!     INPUTS / OUTPUTS:   */
     /*!     OUTPUTS:            */ 
 )
 {
 /*!****************************************************************************/
 /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
 /*!****************************************************************************/
#ifndef PHY_SIMULATION 
     MPD_TYPE_ENT   phyType;
     UINT_8         major, minor, inc, test;
     char           fw_msg [256] = {0};
     MTD_STATUS     mtdStatus = MTD_OK;
 /*!****************************************************************************/
 /*!                      F U N C T I O N   L O G I C                          */
 /*!****************************************************************************/
    phyType = portEntry_PTR->initData_PTR->phyType;
    mtdStatus = mtdGetFirmwareVersion(	PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex),
                                        &major,
                                        &minor,
                                        &inc,
                                        &test);


    if (mtdStatus != MTD_OK){
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
            "rel_ifIndex %d, mtdGetFirmwareVersion -FAIL!", 
            portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    if (beforeFwDownload){
        if (!prvMpdMtdPrintPhyFw_ARR[phyType]){
            sprintf(fw_msg, "\n%s - FW Version: %u.%u.%u.%u\n",
                    prvMpdMtdConvertPhyNameToText_ARR[phyType], 
                    major, 
                    minor, 
                    inc, 
                    test);
            prvMpdMtdPrintPhyFw_ARR[phyType] = TRUE;
        }
    }
    else if (prvMpdMtdPrintPhyFw_ARR[phyType]){
        sprintf(fw_msg, "\n%s - FW Version: %u.%u.%u.%u\n",
                prvMpdMtdConvertPhyNameToText_ARR[phyType], 
                major, 
                minor, 
                inc, 
                test);
        prvMpdMtdPrintPhyFw_ARR[phyType] = FALSE;
    }

    PRV_MPD_LOGGING_MAC(fw_msg);
#else
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(beforeFwDownload);
#endif
    return MTD_OK;
 }


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPreFwDownload
 *
 * DESCRIPTION: Pre FW download
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdPreFwDownload(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_8 major = 0, minor = 0, inc = 0, test = 0;
	UINT_32                  rel_ifIndex;
    PRV_MPD_sw_version_UNT fwPhy_version;
	MTD_STATUS mtdStatus = MTD_OK;
	MPD_UNUSED_PARAM(params_PTR);

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	
	memset( &fwPhy_version, 0, sizeof(fwPhy_version));
    /* need to wait till phy is ready before reading phy fw version */
    if (prvMpdGlobalDb_PTR->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_FLASH_E){
        if (prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_RESET_E) != MPD_OK_E){
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, prvMpdMtdIsPhyReadyAfterFWDownload - FAIL, PHY IS NOT READY!", 
                                                     rel_ifIndex);
        }
        /* get current FW version - only in flash, otherwise all version numbers are 0 */
        mtdStatus = mtdGetFirmwareVersion(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 
                                          &major, 
                                          &minor, 
                                          &inc, 
                                          &test);
        if (mtdStatus != MTD_OK){
#ifndef PHY_SIMULATION
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, mtdGetFirmwareVersion -FAIL!", 
                                                     rel_ifIndex);
            return MPD_OP_FAILED_E;
#endif			
        }
		/* print current FW version */
		prvMpdMtdPrintFwVersion(portEntry_PTR, TRUE);
		
		fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS] = (UINT_8) major;
		fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS] = (UINT_8) minor;
		fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS] = (UINT_8) inc;
		fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS] = (UINT_8) test;

		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d, current FW version on PHY: major = %d, minor = %d inc = %d test = %d, in dWord = 0x%x",
													rel_ifIndex,
													fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS],
													fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS],
													fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS],
													fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS],
													fwPhy_version.ver_num);
    } else { /* RAM download */
		/* HW reset on the representative port of the phy - this is done to cover the case in which the device has no reset circuit */		
		if (portEntry_PTR->runningData_PTR->isRepresentative) {
			PRV_MPD_MTD_CALL(   mtdChipHardwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
								rel_ifIndex,
								mtdStatus);
			PRV_MPD_SLEEP_MAC(100);
		}		
	}
	
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"FW version from Application: major = %d, minor = %d inc = %d test = %d, in dWord = 0x%x",
												prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS],
												prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS],
												prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS],
												prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS],
												prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_num);

    if ((portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE) && 
        (fwPhy_version.ver_num != prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_num)){
            portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired = TRUE;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, Phy required fw download",
                                                        rel_ifIndex);                                                 
    } 
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdFwDownloadByType
 *
 * DESCRIPTION: Help to apply FW download
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdFwDownloadByType(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_FW_DOWNLOAD_TYPE_ENT downloadType,
    MTD_U16 portsToDownload[],
    UINT_16 numOfPortsToDownload
)
{

    UINT_16 error_code = 0, reg_val;
    UINT_16 error_code_ports[MPD_MAX_PORTS_TO_DOWNLOAD_CNS] = { 0 };
    char error_msg [256];
    UINT_32 rel_ifIndex;
    MTD_STATUS ret_val = MTD_OK;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    if (numOfPortsToDownload > MPD_MAX_PORTS_TO_DOWNLOAD_CNS ) {
        PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                    MPD_ERROR_SEVERITY_ERROR_E,
									"Too many ports to download to");
    	return MPD_OP_FAILED_E;
    }
    if (downloadType == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        if ((prvMpdMtdPortDb_ARR[portEntry_PTR->rel_ifIndex]->fw_main.data_PTR == NULL) || 
            (prvMpdMtdPortDb_ARR[portEntry_PTR->rel_ifIndex]->fw_main.dataSize == 0)){
                PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex, 
                                            MPD_ERROR_SEVERITY_FATAL_E, 
                                            "No fw to download on ram");
                return MPD_OP_FAILED_E;
        }
        ret_val = mtdParallelUpdateRamImage( PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex),
                                             portsToDownload,
                                             prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.data_PTR,
                                             prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.dataSize,
											 numOfPortsToDownload,
                                             error_code_ports,
                                             &error_code);
        if (ret_val != MTD_OK) {
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, 1 ,0XC050 ,&reg_val);
            sprintf(error_msg, "rel_ifIndex %d: RAM download. num of parallel ports= %d, error_code = 0X%X, reg 0XC050 = 0X%X\n",
                    rel_ifIndex,
                    numOfPortsToDownload,
                    error_code,
                    reg_val);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_ERROR_E,
                                        error_msg);
        } else {
            portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded = TRUE;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, downloaded to %d ports on PHY's ram",
                                                        rel_ifIndex,
                                                        numOfPortsToDownload);
        }
    } else {
        ret_val = mtdParallelUpdateFlash( PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex),
                                          portsToDownload,
                                          prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.data_PTR,
                                          prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.dataSize,
										  numOfPortsToDownload,
                                          error_code_ports,
                                          &error_code);
        if (ret_val != MTD_OK) {
            sprintf(error_msg, "rel_ifIndex %d: Flash download. error_code = 0X%X\n",
                    rel_ifIndex,
                    error_code);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_ERROR_E,
                                        error_msg);
        }
        else {
            portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded = TRUE;
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, downloaded to %d ports on PHY's flash, ret_val = 0x%x error_code = 0x%x",
                                                        rel_ifIndex,
                                                        numOfPortsToDownload,
                                                        ret_val,
                                                        error_code);
        }

    }

    return ret_val;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdFwDownload
 *
 * DESCRIPTION: FW download
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdFwDownload(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	static BOOLEAN write_msg = TRUE;
	UINT_32 rel_ifIndex;
	MTD_STATUS ret_val = MTD_OK;
	MPD_UNUSED_PARAM(params_PTR);
    
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE) 
    {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d, no need to download FW",
													rel_ifIndex);
		return TRUE;
	}

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex, MPD_ERROR_SEVERITY_FATAL_E, "num of parallel ports exceed maximum supported");
        return MPD_OP_FAILED_E;
    }
	if (prvMpdGlobalDb_PTR->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_RAM_E){
		if (write_msg == TRUE) {
		    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		            "\nThe 10G PHY firmware is being downloaded to RAM. This process might take a few minutes. The ports are not operational during this period");
			write_msg = FALSE;
		}
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d: download FW to RAM. representative %s",
													rel_ifIndex,
													(portEntry_PTR->runningData_PTR->isRepresentative) ? "TRUE" : "FALSE");
        if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) {
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                    MPD_FW_DOWNLOAD_TYPE_RAM_E,
                    portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list,
                    portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports);
            if (ret_val != MTD_OK){
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d: prvMpdMtdFwDownloadByType - download FW to RAM. FAILED",
                                                            rel_ifIndex);
            }
        }
	}
	else {
		if (write_msg == TRUE) {
		    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		                "\nThe 10G PHY firmware is being updated. This process might take a few minutes. The ports are not operational during this period");
			write_msg = FALSE;
		}
		if ((portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) && (portEntry_PTR->runningData_PTR->isRepresentative)) {
		    ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                    MPD_FW_DOWNLOAD_TYPE_FLASH_E,
                    portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list,
                    portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports);
            if (ret_val != MTD_OK){
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d: prvMpdMtdFwDownloadByType - download FW to FLASH. FAILED",
                                                            rel_ifIndex);
            }
        }
	}
#ifdef PHY_SIMULATION
	ret_val = MTD_OK;
#endif

	return (ret_val == MTD_OK) ? MPD_OK_E : MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdFwDownload_1
 *
 * DESCRIPTION: FW download
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88E2580_E
 *                  MPD_TYPE_88X3580_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdFwDownload_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    static BOOLEAN write_msg = TRUE;
    UINT_32 rel_ifIndex, idx, lower_idx = 0, upper_idx = 0;
    MTD_STATUS ret_val = MTD_OK;
    UINT_16 mdio_addr = 0;
    UINT_16 parallel_list_upper_ports[MPD_MAX_PORTS_TO_DOWNLOAD_CNS];
    UINT_16 parallel_list_lower_ports[MPD_MAX_PORTS_TO_DOWNLOAD_CNS];

    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE)
    {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d, no need to download FW",
                                                    rel_ifIndex);
        return TRUE;
    }

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex, MPD_ERROR_SEVERITY_FATAL_E, "num of parallel ports exceed maximum supported");
        return MPD_OP_FAILED_E;
    }
    if (prvMpdGlobalDb_PTR->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        if (write_msg == TRUE) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "\nThe 10G PHY firmware is being downloaded to RAM. This process might take a few minutes. The ports are not operational during this period");
            write_msg = FALSE;
        }
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d: download FW to RAM. representative %s",
                                                    rel_ifIndex,
                                                    (portEntry_PTR->runningData_PTR->isRepresentative) ? "TRUE" : "FALSE");

        if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) {
            /* Special sequence for X3580/E2580 starts here .... needed only for RAM download due to HW issue */
            for (idx = 0; idx < portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports; idx++) {
                mdio_addr = portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list[idx];
                if (mdio_addr % 8 >= 4) {
                    parallel_list_upper_ports[upper_idx] = mdio_addr;
                    upper_idx++;
                } else {
                    parallel_list_lower_ports[lower_idx] = mdio_addr;
                    lower_idx++;
                }
            }

            /* Chip reset upper ports in each PHY */
            for (idx = 0; idx < upper_idx; idx++) {
                mdio_addr = parallel_list_upper_ports[idx];
                if (mdio_addr % 4 == 0) {
                    PRV_MPD_MTD_CALL(   mtdHwXmdioWrite(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), mdio_addr, 31, 0xF001,(1<<14)),
                                    rel_ifIndex,
                                    ret_val);
                    /* Very important, delay needed after this or will fail */
                    PRV_MPD_SLEEP_MAC(10); /* Wait 10ms before any MDIO access */
                }
            }
            if (upper_idx >= MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
            	return MPD_OP_FAILED_E;
            }
            /* download FW to upper ports */
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                    MPD_FW_DOWNLOAD_TYPE_RAM_E,
                    parallel_list_upper_ports,
                    upper_idx);

            if (ret_val == MTD_OK) {
                /* Chip reset lower ports in each PHY */
                for (idx = 0; idx < lower_idx; idx++) {
                    mdio_addr = parallel_list_lower_ports[idx];
                    if (mdio_addr % 4 == 0) {
                        /* Special sequence for X3580/E2580 starts here .... */
                        /* Chip reset upper ports */
                        PRV_MPD_MTD_CALL(   mtdHwXmdioWrite(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), mdio_addr, 31, 0xF001,(1<<14)),
                                        rel_ifIndex,
                                        ret_val);
                        /* Very important, delay needed after this or will fail */
                        PRV_MPD_SLEEP_MAC(10); /* Wait 10ms before any MDIO access */
                    }
                }
                if (lower_idx >= MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
                	return MPD_OP_FAILED_E;
                }
                /* download FW to lower ports */
                ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                        MPD_FW_DOWNLOAD_TYPE_RAM_E,
                        parallel_list_lower_ports,
                        lower_idx);
                if (ret_val!= MTD_OK){
                    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex %d, Failed to downloaded on lower ports on PHY's ram",
                                                                rel_ifIndex);
                }
            } else {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, Failed to downloaded on upper ports on PHY's ram",
                                                        rel_ifIndex);
            }
        }
    }
    else {
        if (write_msg == TRUE) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                        "\nThe 10G PHY firmware is being updated. This process might take a few minutes. The ports are not operational during this period");
            write_msg = FALSE;
        }
        if ((portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) && (portEntry_PTR->runningData_PTR->isRepresentative)) {
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                    MPD_FW_DOWNLOAD_TYPE_FLASH_E,
                    portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list,
					portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports);
            if (ret_val != MTD_OK){
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d: prvMpdMtdFwDownloadByType - download FW to FLASH. FAILED",
                                                            rel_ifIndex);
            }
            /* this call Trigger the FW loading from the flash to the ram */
            PRV_MPD_MTD_CALL(	mtdChipHardwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                                rel_ifIndex,
                                ret_val);
        }
    }
#ifdef PHY_SIMULATION
    ret_val = MTD_OK;
#endif

    return (ret_val == MTD_OK) ? MPD_OK_E : MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPostFwDownload
 *
 * DESCRIPTION: Post FW download
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdPostFwDownload(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 rel_ifIndex;
#ifndef PHY_SIMULATION    
    PRV_MPD_sw_version_UNT fwPhy_version;
    UINT_8 major = 0, minor = 0, inc = 0, test = 0;
#endif
	MPD_RESULT_ENT ret_val = MPD_OK_E;
	MPD_UNUSED_PARAM(params_PTR);

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	if (prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.data_PTR != NULL) {
		PRV_MPD_FREE_MAC(prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.data_PTR);
		prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_main.data_PTR = NULL;
	}

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE){
        ret_val = prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_RESET_E);
        if (ret_val != MPD_OK_E){
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d, phy not ready!",
                                                        rel_ifIndex);
        }
        prvMpdMtdConfigSerdesTx(portEntry_PTR);
        return MPD_OK_E;
    }
          
	ret_val = prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_FW_DOWNLOAD_E);
	if (ret_val == MPD_OK_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdFwFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d, Post download succeeded!",
													rel_ifIndex);
	}
	else {
		return ret_val;
	}

#ifndef PHY_SIMULATION
    mtdGetFirmwareVersion(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            &major,
                            &minor,
                            &inc,
                            &test);
    
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS] = (UINT_8) major;
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS] = (UINT_8) minor;
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS] = (UINT_8) inc;
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS] = (UINT_8) test;

    /* print current FW version */
    prvMpdMtdPrintFwVersion(portEntry_PTR, FALSE);

    if (fwPhy_version.ver_num != prvMpdMtdPortDb_ARR[rel_ifIndex]->fw_version.ver_num) {
        PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
                                    MPD_ERROR_SEVERITY_ERROR_E,
                                    "The fw version that load to flash is not one that running \n");
    }
#endif
	prvMpdMtdConfigSerdesTx(portEntry_PTR);
	
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeCapabilities
 *
 * DESCRIPTION: Get EEE capabilities
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetEeeCapabilities(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_16 mtd_eee_supported_bits;
	UINT_32 i;
	UINT_32 rel_ifIndex;
	UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
#ifndef PHY_SIMULATION
	MTD_STATUS ret_val;
#endif

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
#ifdef PHY_SIMULATION
    mtd_eee_supported_bits = 0xE; /* set bits 1-3 --> 100m, 1g, 10g */
#else
	ret_val = mtdGetEEESupported(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&mtd_eee_supported_bits);
	if (ret_val != MTD_OK){
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdGetEEESupported failed",
                                                        rel_ifIndex);
        
		return MPD_OP_FAILED_E;
    }
#endif

	for (i = 0; i < arr_size; i++) {
		if (mtd_eee_supported_bits & prvMpdConvertMtdToMpdEeeCapability[i].mtd_val) {
			params_PTR->phyEeeCapabilities.enableBitmap |= prvMpdConvertMtdToMpdEeeCapability[i].hal_val;
		}
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d read capabilities 0x%x returned capabilities 0x%x",
												rel_ifIndex,
												mtd_eee_supported_bits,
												params_PTR->phyEeeCapabilities.enableBitmap);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiEnterTimer_2
 *
 * DESCRIPTION: Set EEE LPI Enter timer
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiEnterTimer_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	MPD_UNUSED_PARAM(params_PTR);

	return MPD_OK_E;
	/* OriB, 19.02.17: according to PHY team instructions - we don't need this configuration (Ts is HW parameter); leave it with PHYs defaults */
#if 0
	UINT_8 numPorts, thisPort;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MTD_DEVICE_ID phyRev;
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdGetPhyRevision(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						&phyRev,
						&numPorts,
						&thisPort);
	if (phyRev == MTD_REV_3240P_Z2)
			{
		return MPD_OK_E;
	}

	if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E)
	{
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF004, 8, 8, params_PTR->phyEeeLpiTime.time_us),
							rel_ifIndex,
							mtdStatus);
	}
	else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E)
	{
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF004, 0, 8, params_PTR->phyEeeLpiTime.time_us),
							rel_ifIndex,
							mtdStatus);
	}
	else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E)
	{
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0xC033, 8, 8, params_PTR->phyEeeLpiTime.time_us),
							rel_ifIndex,
							mtdStatus);
	}
	else
	{
		return MPD_NOT_SUPPORTED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d set lpi_enter_time %d to speed 0x%x",
												rel_ifIndex,
												params_PTR->phyEeeLpiTime.time_us,
												params_PTR->phyEeeLpiTime.speed);

	return MPD_OK_E;
#endif
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeAdvertise_2
 *
 * DESCRIPTION: Set EEE advertisiement
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeAdvertise_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_8 numPorts, thisPort;
	UINT_16 current_adv, input_adv, i;
	BOOLEAN restartAn;
	UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MTD_DEVICE_ID phyRev;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdGetPhyRevision(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						&phyRev,
						&numPorts,
						&thisPort);
	if (phyRev == MTD_REV_3240P_Z2)
			{
		return MPD_OK_E;
	}

	for (i = 0, input_adv = 0; i < arr_size; i++) {
		if (params_PTR->phyEeeAdvertize.speedBitmap & prvMpdConvertMtdToMpdEeeCapability[i].hal_val) {
			input_adv |= prvMpdConvertMtdToMpdEeeCapability[i].mtd_val;
		}
	}

	/* supports 100M, 1G and 10G */
	if (input_adv & ~(MTD_EEE_ALL))
		return MPD_NOT_SUPPORTED_E;

	/* if port is UP - restart auto-neg */
	restartAn = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? TRUE : FALSE;

	/* get the current advertisement */
	mtdStatus = mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
					            7,
					            0x003C,
					            &current_adv);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);

	if ((params_PTR->phyEeeAdvertize.advEnable == TRUE) && (current_adv != input_adv)) {
		PRV_MPD_MTD_CALL(	mtdAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), input_adv, restartAn),
							rel_ifIndex,
							mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d advertise EEE on speeds 0x%x and restart AN - %s",
													rel_ifIndex,
													params_PTR->phyEeeAdvertize.speedBitmap,
													(restartAn == FALSE) ? "NO" : "YES");
	}
	else if ((params_PTR->phyEeeAdvertize.advEnable == FALSE) && (current_adv != 0)) {
		PRV_MPD_MTD_CALL(	mtdHwXmdioWrite(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 7, 0x003C, 0),
							rel_ifIndex,
							mtdStatus);
		if (restartAn) {
			PRV_MPD_MTD_CALL(	mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
								rel_ifIndex,
								mtdStatus);
		}
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d not advertise EEE on any speed restart AN - %s",
													rel_ifIndex,
													(restartAn == FALSE) ? "NO" : "YES");
	}

	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeAdvertise_1
 *
 * DESCRIPTION: Set EEE advertisiement
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X33x0_E
 *					MPD_TYPE_88X20x0_E
 *      			MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeAdvertise_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	MTD_U16 mtd_eee_advertise_bits = 0, current_adv;
	MTD_BOOL anRestart;
	UINT_32 i;
	UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
	UINT_32 rel_ifIndex;
	MTD_STATUS ret_val;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	if (params_PTR->phyEeeAdvertize.advEnable == TRUE) {
		for (i = 0, mtd_eee_advertise_bits = 0; i < arr_size; i++) {
			if (params_PTR->phyEeeAdvertize.speedBitmap & prvMpdConvertMtdToMpdEeeCapability[i].hal_val) {
				mtd_eee_advertise_bits |= prvMpdConvertMtdToMpdEeeCapability[i].mtd_val;
			}
		}
	}

	anRestart = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? TRUE : FALSE;

	PRV_MPD_MTD_CALL(mtdGetAdvert_EEE(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								&current_adv), rel_ifIndex, ret_val);

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)
	(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		"rel_ifIndex %d: advertise [%s], input hal-speeds [%#x], current phy config [%#x], admin mode [%s]",
		rel_ifIndex,
		(params_PTR->phyEeeAdvertize.advEnable == TRUE) ? "ENABLE" : "DISBALE",
		params_PTR->phyEeeAdvertize.speedBitmap,
		current_adv,
		(portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? "UP" : "DOWN");

	if ((params_PTR->phyEeeAdvertize.advEnable == TRUE) && (current_adv != mtd_eee_advertise_bits)) {

		ret_val = mtdAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								mtd_eee_advertise_bits,
								anRestart);
		if (ret_val != MTD_OK){
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d: mtdAdvert_EEE failed",
                                                            rel_ifIndex);
            return MPD_OP_FAILED_E;
        }
	}
	else if ((params_PTR->phyEeeAdvertize.advEnable == FALSE) && (current_adv != MTD_EEE_NONE)) {
		ret_val = mtdAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								MTD_EEE_NONE,
								anRestart);
		if (ret_val != MTD_OK)
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d: mtdAdvert_EEE failed",
                                                            rel_ifIndex);
			return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLegacyEnable_2
 *
 * DESCRIPTION: Set EEE legacy (master) enable
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLegacyEnable_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_8 numPorts, thisPort;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MTD_DEVICE_ID phyRev;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdGetPhyRevision(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						&phyRev,
						&numPorts,
						&thisPort);
	if (phyRev == MTD_REV_3240P_Z2)
			{
		return MPD_OK_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)
	(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		"rel_ifIndex %d: Set EEE legacy %s",
		rel_ifIndex,
		(params_PTR->phyEeeMasterEnable.masterEnable == TRUE) ? "Enable" : "Disable");

	if (params_PTR->phyEeeMasterEnable.masterEnable == TRUE) {
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0xC033, 0, 2, (MTD_U16)3),
							rel_ifIndex,
							mtdStatus);
	}
	else {
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0xC033, 0, 2, (MTD_U16)0),
							rel_ifIndex,
							mtdStatus);
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeConfig
 *
 * DESCRIPTION:  Utility, Set EEE configuration
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeConfig(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OP_CODE_ENT phyOperation,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	MTD_U8 eeeMode, xg_enter, xg_exit, g_enter, g_exit, m_enter, m_exit, ipgLen;
	UINT_32 rel_ifIndex;
	MTD_STATUS ret_val;
	MTD_BOOL restart_auto_neg = MTD_FALSE;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	/* read current configuration */
	ret_val = mtdGetEeeBufferConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&eeeMode,
									&xg_enter,
									&g_enter,
									&m_enter,
									&xg_exit,
									&g_exit,
									&m_exit,
									&ipgLen);
	if (ret_val != MTD_OK){
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdGetEeeBufferConfig failed",
                                                        rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)
	(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		"rel_ifIndex %d, EEE oper %d. current configuration: eeeMode %s, xg_enter %d, xg_exit %d, g_enter %d, g_exit %d, m_enter %d, m_exit %d, ipgLen %d",
		rel_ifIndex,
		phyOperation,
		(eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) ? "ENABLE-LEGACY" : "DISABLE",
		xg_enter,
		xg_exit,
		g_enter,
		g_exit,
		m_enter,
		m_exit,
		ipgLen);

	switch (phyOperation) {
		case MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E:
			eeeMode =
					(params_PTR->phyEeeMasterEnable.masterEnable == TRUE) ? MTD_EEE_MODE_ENABLE_WITH_LEGACY : MTD_EEE_MODE_DISABLE;

			if (params_PTR->phyEeeMasterEnable.masterEnable) {
                memset( &phy_params, 0, sizeof(phy_params));
                prvMpdPerformPhyOperation(  portEntry_PTR,
                                            PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                                            &phy_params);

                if (phy_params.internal.phyEeeEnableMode.enableMode == MPD_EEE_ENABLE_MODE_ADMIN_ONLY_E) {
                    restart_auto_neg = MTD_TRUE;
                }
            }
			break;

		case MPD_OP_CODE_SET_LPI_ENTER_TIME_E:
			if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E) {
				m_enter = params_PTR->phyEeeLpiTime.time_us;
			}
			else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E) {
				g_enter = params_PTR->phyEeeLpiTime.time_us;
			}
			else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E) {
				xg_enter = params_PTR->phyEeeLpiTime.time_us;
			}
			else {
				return MPD_NOT_SUPPORTED_E;
			}
			break;

		case MPD_OP_CODE_SET_LPI_EXIT_TIME_E:
			if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E) {
				m_exit = params_PTR->phyEeeLpiTime.time_us;
			}
			else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E) {
				g_exit = params_PTR->phyEeeLpiTime.time_us;
			}
			else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E) {
				xg_exit = params_PTR->phyEeeLpiTime.time_us;
			}
			else {
				return MPD_NOT_SUPPORTED_E;
			}
			break;

		default:
			return MPD_NOT_SUPPORTED_E;
			break;
	}

	/* configure new configuration */
	ret_val = mtdEeeBufferConfig(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									eeeMode,
									xg_enter,
									g_enter,
									m_enter,
									xg_exit,
									g_exit,
									m_exit,
									ipgLen);

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)
	(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		"rel_ifIndex %d: new configuration: eeeMode %s, xg_enter %d, xg_exit %d, g_enter %d, g_exit %d, m_enter %d, m_exit %d, ipgLen %d, ret_val: %d. restartAutoNeg %d",
		rel_ifIndex,
		(eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) ? "ENABLE-LEGACY" : "DISABLE",
		xg_enter,
		xg_exit,
		g_enter,
		g_exit,
		m_enter,
		m_exit,
		ipgLen,
		ret_val,
		restart_auto_neg);

	if (ret_val != MTD_OK){
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdGetEeeBufferConfig failed",
                                                        rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
	if (restart_auto_neg) {
	    PRV_MPD_MTD_CALL(   mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                        rel_ifIndex,
                        ret_val);
	}

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiEnterTimer_1
 *
 * DESCRIPTION: PHY 33x0, 20x0
 *
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiEnterTimer_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	/* OriB, 19.02.17: according to PHY team instructions - we don't need this configuration (Ts is HW parameter); leave it with PHYs defaults
	 return prvMpdMtdSetEeeConfig(rel_ifIndex, MPD_OP_CODE_SET_LPI_ENTER_TIME_E, params_PTR);*/
	MPD_UNUSED_PARAM(portEntry_PTR);
	MPD_UNUSED_PARAM(params_PTR);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLegacyEnable_1
 *
 * DESCRIPTION: PHY 33x0
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLegacyEnable_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	return prvMpdMtdSetEeeConfig(	portEntry_PTR,
									MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
									params_PTR);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeRemoteStatus_2
 *
 * DESCRIPTION: Get EEE remote status
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdMtdGetEeeRemoteStatus_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_16 val, temp;
	UINT_32 bit_map_ARR[4] = {	0,
								MPD_SPEED_CAPABILITY_100M_CNS,
								MPD_SPEED_CAPABILITY_1G_CNS,
								MPD_SPEED_CAPABILITY_10G_CNS };
	UINT_32 rel_ifIndex, i;
    MTD_STATUS 	mtdStatus = MPD_OK_E;
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
#ifdef PHY_SIMULATION
    val = 8; /* WM - remote advertise EEE 10G capability */
#else
	mtdStatus = mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
					            7,
					            61,
					            &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
#endif
	for (i = 1; i < 4; i++)
			{
		temp = val & (1 << i);
		if (temp != 0)
				{
			params_PTR->phyEeeCapabilities.enableBitmap |= bit_map_ARR[i];
		}
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d result 0x%x returned value 0x%x",
												rel_ifIndex,
												val,
												params_PTR->phyEeeCapabilities.enableBitmap);

	return mtdStatus;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeRemoteStatus_1
 *
 * DESCRIPTION: PHY 33x0
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetEeeRemoteStatus_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_16 eee_lp_advert;
	UINT_32 i;
	UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
	UINT_32 rel_ifIndex;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	params_PTR->phyEeeCapabilities.enableBitmap = 0;
	if (mtdGetLP_Advert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
							&eee_lp_advert) != MTD_OK){

        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdGetLP_Advert_EEE failed",
                                                        rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
		
	for (i = 0; i < arr_size; i++) {
		if (eee_lp_advert & prvMpdConvertMtdToMpdEeeCapability[i].mtd_val) {
			params_PTR->phyEeeCapabilities.enableBitmap |= prvMpdConvertMtdToMpdEeeCapability[i].hal_val;
		}
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d result 0x%x returned value 0x%x",
												rel_ifIndex,
												eee_lp_advert,
												params_PTR->phyEeeCapabilities.enableBitmap);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeConfig_1
 *
 * DESCRIPTION: Get EEE configuration from MTD driver.
 *               PHYs: 3220, 33x0
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetEeeConfig_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	MTD_U8 eeeMode, xg_enter, xg_exit, g_enter, g_exit, m_enter, m_exit, ipgLen;
	UINT_32 rel_ifIndex = portEntry_PTR->rel_ifIndex;
	MTD_STATUS rc;
    
#ifdef PHY_SIMULATION
    eeeMode=MTD_EEE_MODE_ENABLE_WITH_LEGACY;
    xg_enter=0x1A;
    xg_exit=8;
    g_enter=0x12;
    g_exit=0x12;
    m_enter=0x20;
    m_exit=0x20;
    ipgLen=0xc;
    rc = MTD_OK;
#else    
	rc = mtdGetEeeBufferConfig(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								&eeeMode,
								&xg_enter,
								&g_enter,
								&m_enter,
								&xg_exit,
								&g_exit,
								&m_exit,
								&ipgLen);
#endif
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)
	(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		"rel_ifIndex %d: eeeMode %s, xg_enter %d, xg_exit %d, g_enter %d, g_exit %d, m_enter %d, m_exit %d, ipgLen %d",
		rel_ifIndex,
		(eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) ? "ENABLE-LEGACY" : "DISABLE",
		xg_enter,
		xg_exit,
		g_enter,
		g_exit,
		m_enter,
		m_exit,
		ipgLen);

	if (rc != MTD_OK){
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdGetEeeBufferConfig failed",
                                                        rel_ifIndex);
		return MPD_OP_FAILED_E;
    }
	params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_100M_E] = m_exit;
	params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_1G_E] = g_exit;
	params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_10G_E] = xg_exit;
	/* PATCH!!! - need values from PHY driver!!! */
	params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_2500M_E] = xg_exit;
	params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_5G_E] = xg_exit;

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetNearEndLoopback_2
 *
 * DESCRIPTION: PHY 3240
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetNearEndLoopback_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_32 rel_ifIndex;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	if (params_PTR->internal.phyLoopback.enable) {
		mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
							3,
							0,
							14,
							1,
							1);
	}
	else {
		prvMpdMtdReset32x0(	portEntry_PTR,
							50);
	}
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDuplexMode_2
 *
 * DESCRIPTION: Set Duplex mode
 * 			Relevant only if AN is disabled in 10M or 100M
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDuplexMode_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	char *duplex_text = "FULL";
	UINT_16 speed_val, phy_in_shutdown, new_val;
	MTD_BOOL speedIsForced;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MPD_RESULT_ENT ret = MPD_OK_E;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						31,
						0xF001,
						11,
						1,
						&phy_in_shutdown);
	/*if (phy_in_shutdown)
	 {
	 restart_autoneg = FALSE;
	 }*/

	/* check if works in 100M - maybe need to enable parallel detect and disable it in other speeds */

	mtdGetForcedSpeed(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						&speedIsForced,
						&speed_val);

	if (speedIsForced == MTD_TRUE)
	{
		if ((((speed_val == MTD_SPEED_10M_HD_AN_DIS) || (speed_val == MTD_SPEED_100M_HD_AN_DIS)) && (params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_FULL_E)) ||
				(((speed_val == MTD_SPEED_10M_FD_AN_DIS) || (speed_val == MTD_SPEED_100M_FD_AN_DIS)) && (params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_HALF_E)))
				{
			new_val = speed_val;
			if (params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_FULL_E) {
				new_val = new_val << 1;
			}
			else {
				new_val = new_val >> 1;
				duplex_text = "HALF";
			}

			PRV_MPD_MTD_CALL(	mtdForceSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), new_val),
								rel_ifIndex,
								mtdStatus);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"rel_ifIndex %d config %s DUPLEX",
															rel_ifIndex,
															duplex_text);

			if (phy_in_shutdown)
			{
				/* need to return the PHY to shutdown - TBD */
				PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"rel_ifIndex %d return PHY to SHUTDOWN");
			}
		}
	}

	return ret;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiExitTimer_2
 *
 * DESCRIPTION: Set EEE LPI Exit timer
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiExitTimer_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_8 numPorts, thisPort;
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	MTD_DEVICE_ID phyRev;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	mtdGetPhyRevision(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
						&phyRev,
						&numPorts,
						&thisPort);
	if (phyRev == MTD_REV_3240P_Z2)
			{
		return MPD_OK_E;
	}

	if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E)
	{
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF006, 8, 8, params_PTR->phyEeeLpiTime.time_us),
							rel_ifIndex,
							mtdStatus);
	}
	else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E)
	{
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF006, 0, 8, params_PTR->phyEeeLpiTime.time_us),
							rel_ifIndex,
							mtdStatus);
	}
	else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E)
	{
		PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF005, 8, 8, params_PTR->phyEeeLpiTime.time_us),
							rel_ifIndex,
							mtdStatus);
	}
	else
	{
		return MPD_NOT_SUPPORTED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d set lpi_exit_time %d to speed 0x%x",
												rel_ifIndex,
												params_PTR->phyEeeLpiTime.time_us,
												params_PTR->phyEeeLpiTime.speed);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiExitTimer_1
 *
 * DESCRIPTION: Set EEE LPI Exit timer
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiExitTimer_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	return prvMpdMtdSetEeeConfig(	portEntry_PTR,
									MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
									params_PTR);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetPowerModules
 *
 * DESCRIPTION: Enable Green Ethernet Short Reach
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetPowerModules(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus;
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	/*ED treatment*/
	if (params_PTR->phyPowerModules.energyDetetct != MPD_GREEN_NO_SET_E)
	{
		if (params_PTR->phyPowerModules.energyDetetct == MPD_GREEN_ENABLE_E) {

			PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 3),
								rel_ifIndex,
								mtdStatus);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d Energy detect enable",
														rel_ifIndex);
		} /* enable ED */
		else if (params_PTR->phyPowerModules.energyDetetct == MPD_GREEN_DISABLE_E) {
			PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 0),
								rel_ifIndex,
								mtdStatus);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d Energy detect disable",
														rel_ifIndex);
		} /* disable ED */
		if (params_PTR->phyPowerModules.performPhyReset) {
			prvMpdMtdReset32x0(	portEntry_PTR,
								50);
		}
	} /* ED */

	/*SR treatment*/
	if (params_PTR->phyPowerModules.shortReach != MPD_GREEN_NO_SET_E)
	{
		if (params_PTR->phyPowerModules.shortReach == MPD_GREEN_ENABLE_E)
		{
			PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0x0083, 0, 1, 1),
								rel_ifIndex,
								mtdStatus);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d Short reach enable",
														rel_ifIndex);
		} /* enable SR */
		else if (params_PTR->phyPowerModules.shortReach == MPD_GREEN_DISABLE_E)
		{
			PRV_MPD_MTD_CALL(	mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0x0083, 0, 1, 0),
								rel_ifIndex,
								mtdStatus);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdEeeFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d Short reach disable",
														rel_ifIndex);
		} /* disable SR */
	} /* SR */

	return MPD_OK_E;
}

/*****************************************************************************
 * FUNCTION NAME: prvMpdMtdUpdateMediaType
 *
 * DESCRIPTION:
 *
 * APPLICABLE PHY:

 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X32x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdUpdateMediaType(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_32                      speed
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */

)
{
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_U16 frameToRegister, mediaSelect, fiberType;
    MTD_BOOL npMediaEnergyDetect, maxPowerTunitAMDetect;
    UINT_32 fiberSpeed = MTD_FT_1000BASEX;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    /* check if we have X-UNIT */
    if (MTD_HAS_X_UNIT(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex)))
    {
        /* get current media */
        PRV_MPD_MTD_CALL(mtdGetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                    &frameToRegister, &mediaSelect, &fiberType, &npMediaEnergyDetect, &maxPowerTunitAMDetect),
                    rel_ifIndex,
                    mtdStatus);

        /* check if Fiber is present */
        if (portEntry_PTR->runningData_PTR->sfpPresent) {
            switch (speed) {
                case MPD_SPEED_1000M_E:
                    fiberSpeed = MTD_FT_1000BASEX;
                    break;
                case MPD_SPEED_2500M_E:
                    fiberSpeed = MTD_FT_2500BASEX;
                    break;
                case MPD_SPEED_5000M_E:
                    fiberSpeed = MTD_FT_5GBASER;
                    break;
                case MPD_SPEED_10000M_E:
                    fiberSpeed = MTD_FT_10GBASER;
                    break;
                default:
                    fiberSpeed = MTD_FT_NONE;
            }

            /* if desired speed was not selected, update media type will occur in set speed operation */
            if (fiberSpeed == MTD_FT_NONE) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d. No valid speed received %d",
                                                    rel_ifIndex, speed);
                return MPD_OK_E;
            }
            /* configured media type & speed are as desired */
            if ((mediaSelect == MTD_MS_FBR_ONLY) && (fiberSpeed == fiberType)) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d. No media change (fiber mode), configured speed %d",
                                                    rel_ifIndex, fiberSpeed);
            } else {
                if ((fiberSpeed == MTD_FT_NONE) && (fiberType != MTD_FT_NONE)) {
                    fiberSpeed = fiberType;
                    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d. media change (fiber mode), using configured speed %d",
                                                    rel_ifIndex, fiberSpeed);
                } else {
                    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d. media change (fiber mode), using new speed %d",
                                                    rel_ifIndex, fiberSpeed);
                }
                PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_F2R_OFF, MTD_MS_FBR_ONLY,
                            fiberSpeed, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE),
                            rel_ifIndex,
                            mtdStatus);
            }
        } else { /* if Fiber is not present, desired media is Copper */
            /* configured media is copper */
            if (mediaSelect == MTD_MS_CU_ONLY) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d. No media change, already in copper mode",
                                                    rel_ifIndex);

            } else {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. moving to copper mode",
                                                        rel_ifIndex);

                PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_F2R_OFF, MTD_MS_CU_ONLY,
                        MTD_FT_1000BASEX, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE),
                        rel_ifIndex,
                        mtdStatus);
            }
        }
    }

    return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetSpeed_5
 *
 * DESCRIPTION:
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X32x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetSpeed_5(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 duplexBit, speedVal = 0, autoNegEnabled;
    MTD_BOOL speedIsForced, config = FALSE;
    BOOLEAN restartAutoneg;
    UINT_32 speedConf;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_RESULT_ENT ret = MPD_OK_E;
    char error_msg [256];

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
    restartAutoneg =
            ((portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) && (!portEntry_PTR->runningData_PTR->sfpPresent)) ? TRUE : FALSE;

    switch (params_PTR->phySpeed.speed) {
        case MPD_SPEED_10M_E:
        case MPD_SPEED_100M_E:
            /* get forced speed and duplex -
             order is important-> speed bits must be set before AN is disabled that is done inside API mtdForceSpeed */
            mtdGetForcedSpeed(  PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                &speedIsForced,
                                &speedVal);
            /*if ( (speedIsForced == MTD_FALSE) || ((speed_val != MTD_SPEED_10M_HD_AN_DIS) && (speed_val != MTD_SPEED_10M_FD_AN_DIS)) ) {*/
            mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                7,
                                0x8000,
                                4,
                                1,
                                &duplexBit);
            if (duplexBit)
            {
                speedConf = (params_PTR->phySpeed.speed == MPD_SPEED_10M_E) ? MTD_SPEED_10M_FD_AN_DIS : MTD_SPEED_100M_FD_AN_DIS;
            } else {
                speedConf = (params_PTR->phySpeed.speed == MPD_SPEED_10M_E) ? MTD_SPEED_10M_HD_AN_DIS : MTD_SPEED_100M_HD_AN_DIS;
            }
            PRV_MPD_MTD_CALL(   mtdForceSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speedConf),
                                    rel_ifIndex,
                                    mtdStatus);
            if (!restartAutoneg)
            {
                /* need to return the PHY to shutdown - TBD */
            }
            if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(prvMpdMtdConfigFlagId)) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex %d, restartAutoneg - %s, MTD 0x%x, MPD %d speed %s",
                                                                rel_ifIndex,
                                                                (restartAutoneg) ? "YES" : "No",
                                                                speedConf,
                                                                params_PTR->phySpeed.speed,
                                                                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));
            }
            /*}*/
            break;
        case MPD_SPEED_1000M_E:
            speedConf = MTD_SPEED_1GIG_FD;
            config = TRUE;
            break;
        case MPD_SPEED_2500M_E:
            speedConf = MTD_SPEED_2P5GIG_FD;
            config = TRUE;
            break;
        case MPD_SPEED_5000M_E:
            speedConf = MTD_SPEED_5GIG_FD;
            config = TRUE;
            break;
        case MPD_SPEED_10000M_E:
            speedConf = MTD_SPEED_10GIG_FD;
            config = TRUE;
            break;

        default:
            return MPD_NOT_SUPPORTED_E;
    }
    if (config == TRUE) {
        ret = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, speedConf);
        if (ret != MPD_OK_E){
            sprintf(error_msg, "rel_ifIndex %d - desired MTD speed %d doesn't match usx mode capabilities\n", portEntry_PTR->rel_ifIndex, speedConf);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_MINOR_E,
                                        error_msg);
            return MPD_OP_FAILED_E;
        }
        /* get auto-negotiation value and advertise capabilities */
        mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            7,
                            0,
                            12,
                            1,
                            &autoNegEnabled);
        mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),  &speedVal);                      
        if ((autoNegEnabled == FALSE) || (speedVal != speedConf)) {

            prvMpdMtdUpdateMediaType(portEntry_PTR, params_PTR->phySpeed.speed);

            PRV_MPD_MTD_CALL(   mtdEnableSpeeds(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speedConf, restartAutoneg),
                                rel_ifIndex,
                                mtdStatus);
            if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(prvMpdMtdConfigFlagId)) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex %d restartAutoneg %s MTD %d MPD %d speed %s",
                                                                rel_ifIndex,
                                                                (restartAutoneg) ? "YES" : "No",
                                                                speedConf,
                                                                params_PTR->phySpeed.speed,
                                                                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));
            }
        }
    }


    return ret;
}

static MPD_RESULT_ENT prvMpdMtdGetHighestSpeed(
    /*     INPUTS:             */
    UINT_16 negCapabilities,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_32 * speed_PTR
)
{
    UINT_32 speed = MPD_SPEED_LAST_E;

    if (!speed_PTR) {
        return MPD_OP_FAILED_E;
    }

    if (negCapabilities & MPD_AUTONEG_CAPABILITIES_200G_FULL_CNS) {
        speed = MPD_SPEED_200G_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS) {
        speed = MPD_SPEED_100G_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS) {
        speed = MPD_SPEED_50G_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS) {
        speed = MPD_SPEED_40G_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS) {
        speed = MPD_SPEED_25G_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS) {
        speed = MPD_SPEED_10000M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS) {
        speed = MPD_SPEED_5000M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS) {
        speed = MPD_SPEED_2500M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS) {
        speed = MPD_SPEED_1000M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
        speed = MPD_SPEED_100M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
        speed = MPD_SPEED_100M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
        speed = MPD_SPEED_10M_E;
    } else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
        speed = MPD_SPEED_10M_E;
    }

    * speed_PTR = speed;

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetAutoNeg_5
 *
 * DESCRIPTION: PHY 2180
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetAutoNeg_5(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    typedef MTD_STATUS masterSlave_func(
        MTD_DEV_PTR,
        UINT_16,
        MTD_BOOL);

    masterSlave_func *func_PTR = NULL;
    UINT_16 val = 0, master_stat;
    UINT_16 capabilities = 0;
    BOOLEAN set_master = FALSE, restartAn = FALSE;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_RESULT_ENT status;
    UINT_32 speed = MPD_SPEED_LAST_E;
    char prvMpdCapabilitiesString[256];
    char error_msg [256];

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (params_PTR->phyAutoneg.enable == MPD_AUTO_NEGOTIATION_ENABLE_E) {

        if ((params_PTR->phyAutoneg.capabilities == 0) ||
             (params_PTR->phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS) ||
             (params_PTR->phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS)) {
            /* get all available supported speeds in the PHY */
            PRV_MPD_MTD_CALL(mtdGetSpeedsAvailable(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&capabilities),
                            rel_ifIndex,
                            mtdStatus);

            /* 1G Half Duplex is not supported */
            capabilities &= ~MTD_SPEED_1GIG_HD;

            if (capabilities & MTD_SPEED_5GIG_FD) {
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_5GIG_FD);
                if (status != MPD_OK_E) {
                    capabilities &= ~MTD_SPEED_5GIG_FD;
                }
            }
            if (capabilities & MTD_SPEED_10GIG_FD) {
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_10GIG_FD);
                if (status != MPD_OK_E) {
                    capabilities &= ~MTD_SPEED_10GIG_FD;
                }
            }
        } else {
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
                capabilities |= MTD_SPEED_10M_HD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
                capabilities |= MTD_SPEED_10M_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
                capabilities |= MTD_SPEED_100M_HD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
                capabilities |= MTD_SPEED_100M_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS) {
                capabilities |= MTD_SPEED_1GIG_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS) {
                capabilities |= MTD_SPEED_2P5GIG_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS) {
                capabilities |= MTD_SPEED_5GIG_FD;
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_5GIG_FD);
                if (status != MPD_OK_E) {
                    sprintf(error_msg, "rel_ifIndex %d - desired speed 5G doesn't match usx mode capabilities\n", portEntry_PTR->rel_ifIndex);
                    PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                                MPD_ERROR_SEVERITY_MINOR_E,
                                                error_msg);
                    return MPD_OP_FAILED_E;
                }
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS) {
                capabilities |= MTD_SPEED_10GIG_FD;
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_10GIG_FD);
                if (status != MPD_OK_E) {
                    sprintf(error_msg, "rel_ifIndex %d - desired speed 10G doesn't match usx mode capabilities\n", portEntry_PTR->rel_ifIndex);
                    PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                                MPD_ERROR_SEVERITY_MINOR_E,
                                                error_msg);
                    return MPD_OP_FAILED_E;
                }
            }
        }

        if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(prvMpdMtdConfigFlagId)) {
            prvMpdCapabilitiesString[0] = '\0';
            prvMpdDebugConvertMtdCapabilitiesToString(capabilities, prvMpdCapabilitiesString);
            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d MPD 0x%x MTD 0x%x capabilities %s",
                                                        rel_ifIndex,
                                                        params_PTR->phyAutoneg.capabilities,
                                                        capabilities,
                                                        prvMpdCapabilitiesString);
        }

        /* read master-slave preference status */
        mtdHwXmdioRead( PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                        7,
                        32,
                        &master_stat);
        /* check if need to set master/slave preferred */
        if (((master_stat & PRV_MPD_master_manual_mask_CNS) != 0) ||
                ((master_stat & PRV_MPD_master_preferred_mask_CNS) == 0 &&
                        params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) ||
                ((master_stat & PRV_MPD_master_preferred_mask_CNS) != 0 &&
                        params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_SLAVE_E)) {
            set_master = TRUE;
            func_PTR = (params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) ?
                    mtdAutonegSetMasterPreference : mtdAutonegSetSlavePreference;

            PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d set master/slave to %s",
                                                            rel_ifIndex,
                                                            (params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) ? "Preferred Master" : "Preferred Slave");
        }
    }
    else
    {
        /* possible for 10M or 100M forced speed only - the configuration will be done in prvMpdMtdSetSpeed_5 */
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d autoNeg disabled - configuration is done in set speed operation",
                                                        rel_ifIndex);
        return MPD_OK_E;
    }

    /* check the speeds that currently been advertised */
    mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &val);
                      
    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d configured MTD speeds 0x%x",
                                                    rel_ifIndex,
                                                    val);

    if (set_master) {
        /* call to master/salve preferred function + restart AN */
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d call to master-salve preferred function restart AN",
                                                    rel_ifIndex);
        PRV_MPD_MTD_CALL(   (func_PTR)(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), FALSE),
                            rel_ifIndex,
                            mtdStatus);
    }
    if (val != capabilities) {

        prvMpdMtdGetHighestSpeed(capabilities, &speed);
        prvMpdMtdUpdateMediaType(portEntry_PTR, speed);

        /* if port is UP and we are in copper mode - restart auto-neg */
        restartAn = ((portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) && (!portEntry_PTR->runningData_PTR->sfpPresent)) ? TRUE : FALSE;

        PRV_MPD_MTD_CALL(   mtdEnableSpeeds(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), capabilities, restartAn),
                            rel_ifIndex,
                            mtdStatus);

        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d update speeds & restart AN %d",
                                                    rel_ifIndex,
                                                    restartAn);
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdUtilConvertSpeedTypes
 *
 * DESCRIPTION: Convert speed types from MTD format to MPD format
 *
 *****************************************************************************/
static void prvMpdMtdUtilConvertSpeedTypes(
	/*     INPUTS:             */
	UINT_16 speeds,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	MPD_AUTONEG_CAPABILITIES_TYP *capabilities_PTR
)
{
	(*capabilities_PTR) = 0;
	if (speeds & MTD_SPEED_10M_HD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
	}
	if (speeds & MTD_SPEED_10M_FD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
	}
	if (speeds & MTD_SPEED_100M_HD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
	}
	if (speeds & MTD_SPEED_100M_FD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
	}
	if (speeds & MTD_SPEED_1GIG_HD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS;
	}
	if (speeds & MTD_SPEED_1GIG_FD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
	}
	if (speeds & MTD_SPEED_10GIG_FD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS;
	}
	if (speeds & MTD_SPEED_2P5GIG_FD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS;
	}
	if (speeds & MTD_SPEED_5GIG_FD) {
		(*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS;
	}
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetAutonegSupport
 *
 * DESCRIPTION: Get supported auto-negotiation capabilities
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetAutonegSupport(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 	speedsVal;
	UINT_32 	rel_ifIndex;
	MTD_STATUS 	mtdStatus;
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	PRV_MPD_MTD_CALL( mtdGetSpeedsAvailable(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speedsVal),
					  rel_ifIndex,
					  mtdStatus);

	prvMpdMtdUtilConvertSpeedTypes(speedsVal, &params_PTR->PhyAutonegCapabilities.capabilities);
	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetAutonegAdmin_1
 *
 * DESCRIPTION: Get admin (configured) auto-negotiation
 *
 * APPLICABLE PHY:
 * 					MPD_TYPE_88X32x0_E
 * 					MPD_TYPE_88X33x0_E
 * 					MPD_TYPE_88X20x0_E
 * 					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetAutonegAdmin_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 	rel_ifIndex;
	UINT_16 	masterPrefer, autoNegReg, speedsVal;
	MTD_STATUS 	mtdStatus;
    
	params_PTR->phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
	params_PTR->phyAutoneg.masterSlave 	= MPD_AUTONEGPREFERENCE_UNKNOWN_E;
	params_PTR->phyAutoneg.enable 		= FALSE;

	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
			 (portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
			 (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
		return MPD_NOT_SUPPORTED_E;
	}

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	/* check if auto neg enabled - dev 7, reg 0, bit 12 */
	mtdStatus = mtdHwXmdioRead(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								7,
								0,
								&autoNegReg);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
	if ((autoNegReg & 0x1000) == 0) {
		return MPD_OK_E;
	}

	mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
							   7,
							   32,
							   &masterPrefer);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);

	masterPrefer &= PRV_MPD_MTD_MASTER_PREFERENCE_MASK_CNS;
	if (masterPrefer == PRV_MPD_MTD_MASTER_PREFERENCE_MASTER_CNS) {
		params_PTR->phyAutoneg.masterSlave 	= MPD_AUTONEGPREFERENCE_MASTER_E;
	}
	if (masterPrefer == PRV_MPD_MTD_MASTER_PREFERENCE_SLAVE_CNS) {
		params_PTR->phyAutoneg.masterSlave 	= MPD_AUTONEGPREFERENCE_SLAVE_E;
	}

    mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speedsVal);

	prvMpdMtdUtilConvertSpeedTypes(speedsVal, &params_PTR->phyAutoneg.capabilities);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetPresentNotification
 *
 * DESCRIPTION:	Set to copper media type for combo PHY
 *
 * APPLICABLE PHY:
 * 				MPD_TYPE_88X32x0_E
 *              MPD_TYPE_88X33x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetPresentNotification(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    portEntry_PTR->runningData_PTR->sfpPresent = params_PTR->phySfpPresentNotification.isSfpPresent;
    if (params_PTR->phySfpPresentNotification.isSfpPresent) {
        portEntry_PTR->runningData_PTR->opMode = params_PTR->phySfpPresentNotification.opMode;
    } else {
        portEntry_PTR->runningData_PTR->opMode = MPD_OP_MODE_UNKNOWN_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d opMode %d isPresent %d speed %d",
                    portEntry_PTR->rel_ifIndex,
                    portEntry_PTR->runningData_PTR->opMode,
                    portEntry_PTR->runningData_PTR->sfpPresent,
                    portEntry_PTR->runningData_PTR->speed);

    prvMpdMtdUpdateMediaType(portEntry_PTR, portEntry_PTR->runningData_PTR->speed);

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetCheckLinkUp
 *
 * DESCRIPTION:     MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetCheckLinkUp(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 local, ieeeSpeed, nbSpeed, speed_res, lpAnd_local;
	UINT_32 retries = 0;
	UINT_32 rel_ifIndex;
	MTD_STATUS rc;
    
	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	params_PTR->internal.phyLinkStatus.phyValidStatus = TRUE;
	/* check the speeds that currently been advertised */
	rc = mtdGetSpeedsEnabled(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								&local);
	if (rc != MTD_OK) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"mtdGetSpeedsEnabled on rel_ifIndex %d failed",
													rel_ifIndex);
		return MPD_OK_E;
	}
	rc = mtdGetLPAdvertisedSpeeds(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									&ieeeSpeed,
									&nbSpeed);
	if (rc != MTD_OK) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"mtdGetLPAdvertisedSpeeds on rel_ifIndex %d failed",
													rel_ifIndex);
		return MPD_OK_E;
	}
	rc = MTD_FAIL;
	while (rc != MTD_OK && retries < 20) {
		rc = mtdGetAutonegSpeedDuplexResolution(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
												&speed_res);
		retries++;
		if (rc != MTD_OK) {
			PRV_MPD_SLEEP_MAC(1);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"mtdGetAutonegSpeedDuplexResolution on rel_ifIndex %d failed retry %d",
														rel_ifIndex,
														retries);
		}
	}

	if (rc != MTD_OK) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"mtdGetLPAdvertisedSpeeds on rel_ifIndex %d failed",
													rel_ifIndex);
		return MPD_OK_E;
	}

	lpAnd_local = local & ieeeSpeed;

	if (((lpAnd_local & ~speed_res) > speed_res) && speed_res != MTD_SPEED_10GIG_FD) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"%d Resolved to NOT highest possible speed. lpAnd_local[%#x] resolved[%#x]",
													rel_ifIndex,
													lpAnd_local,
													speed_res);

		params_PTR->internal.phyLinkStatus.phyValidStatus = FALSE;
		mtdAutonegRestart(	PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex) );
	}
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												"rel_ifIndex %d local[%#x] remote_ieeeSpeed[%#x] lpAnd_local[%#x] resolved[%#x]",
												rel_ifIndex,
												local,
												ieeeSpeed,
												lpAnd_local,
												speed_res);

	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetTemperature
 *
 * DESCRIPTION:	Get temperature from PHY temperature sensoe
 *
 * APPLICABLE PHY:
 * 				MPD_TYPE_88X32x0_E
 * 				MPD_TYPE_88X33x0_E
 * 				MPD_TYPE_88X20x0_E
 * 				MPD_TYPE_88X2180_E
 *				MPD_TYPE_88E2540_E
 *				MPD_TYPE_88X3540_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdMtdGetTemperature(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	MTD_S16 temperature = 50;
#ifndef PHY_SIMULATION
	UINT_32 rel_ifIndex;
	MTD_STATUS mtdStatus = MTD_OK;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
    
	PRV_MPD_MTD_CALL(	mtdReadTemperature(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),&temperature),
						rel_ifIndex,
						mtdStatus);
	if (mtdStatus != MTD_OK)
	{
        PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdReadTemperature failed",
                                                        rel_ifIndex);
        return MPD_OP_FAILED_E;
	}
	PRV_MPD_DEBUG_LOG_MAC(prvMpdMtdConfigFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"rel_ifIndex %d temperature is 0x%x",
													rel_ifIndex,
													temperature);
#endif
	MPD_UNUSED_PARAM(portEntry_PTR);

	params_PTR->phyTemperature.temperature = temperature;
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: mpdMtdMdioReadWrap
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MTD_STATUS mpdMtdMdioReadWrap(
	/*     INPUTS:             */
	MTD_DEV *dev_PTR,
	UINT_16 phy_addr,
	UINT_16 dev,
	UINT_16 reg,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *value_PTR
)
{
	PRV_MPD_APP_DATA_STC * appData_PTR;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;


    MPD_UNUSED_PARAM(phy_addr);
    appData_PTR = (PRV_MPD_APP_DATA_STC *) dev_PTR->appData;
    portEntry_PTR = prvMpdFindPortEntry(appData_PTR,phy_addr);
    if (portEntry_PTR == NULL) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_ERROR_E,
									"couldn't find entry\n");
    	return MTD_FAIL;
    }
    return prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									dev,
									reg,
									value_PTR);

}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdWaitWrap
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MTD_STATUS mpdMtdWaitWrap(
	IN	MTD_DEV_PTR dev,
    IN 	MTD_UINT milliSeconds
)
{
	MPD_UNUSED_PARAM(dev);
	PRV_MPD_SLEEP_MAC(milliSeconds);
	return MTD_OK;
}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdMdioWriteWrap
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MTD_STATUS mpdMtdMdioWriteWrap(
	/*     INPUTS:             */
	MTD_DEV *dev_PTR,
	UINT_16 phy_addr,
	UINT_16 dev,
	UINT_16 reg,
	UINT_16 value
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
    PRV_MPD_APP_DATA_STC * appData_PTR;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;

    appData_PTR = (PRV_MPD_APP_DATA_STC *) dev_PTR->appData;
    portEntry_PTR = prvMpdFindPortEntry(appData_PTR, phy_addr);
    if (portEntry_PTR == NULL) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_ERROR_E,
									"couldn't find entry\n");
    	return MTD_FAIL;
    }

    return prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									dev,
									reg,
									PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
									value);
	
}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdInit
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/

extern MPD_RESULT_ENT mpdMtdInit(
	/*     INPUTS:             */
	PRV_MPD_PORT_LIST_TYP *fw_portList_PTR,
	MPD_TYPE_ENT *phyType_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
	MPD_TYPE_ENT phyType;
	UINT_32 rel_ifIndex;
	PRV_MPD_MTD_OBJECT_INFO_STC phys_obj_info [MPD_TYPE_NUM_OF_TYPES_E][MPD_MAX_INTERFACE_ID_NUMBER][PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS];
    PRV_MPD_APP_DATA_STC * appData_PTR;
    UINT_8  dev, xsmiInterface;

#ifndef PHY_SIMULATION
	UINT_16 err_code;
	MTD_STATUS mtdStat;
#endif

	prvMpdMtdInitDebug();

	/* fill PHY attributes */
	memset(	phys_obj_info,
			0,
			sizeof(phys_obj_info));

	for (rel_ifIndex = 0; prvMpdPortListGetNext(fw_portList_PTR,
												&rel_ifIndex) == TRUE;) {
		phyType = phyType_PTR[rel_ifIndex];
		
        /* todo - call init external driver or re-design */
		if (phyType == MPD_TYPE_INVALID_E) {
			/* check whether this is a user defined */
			if (phyType_PTR[rel_ifIndex] >= MPD_TYPE_FIRST_USER_DEFINED_E &&
				phyType_PTR[rel_ifIndex] <= MPD_TYPE_LAST_USER_DEFINED_E) {
				phyType = phyType_PTR[rel_ifIndex];
			}
		}

		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
        xsmiInterface = portEntry_PTR->initData_PTR->mdioInfo.mdioBus;
        dev = portEntry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
        if (PRV_MPD_VALID_PHY_TYPE(phyType)) {
			if (phys_obj_info[phyType][xsmiInterface][dev].exist == FALSE) {
				/* handle MTD PHYs family (10G copper PHYs) object initialization */
                phys_obj_info[phyType][xsmiInterface][dev].exist = TRUE;
				phys_obj_info[phyType][xsmiInterface][dev].obj_PTR = (PRV_MPD_MTD_OBJECT_STC*) PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_MTD_OBJECT_STC));
                if  (phys_obj_info[phyType][xsmiInterface][dev].obj_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(	0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                }
                memset(	phys_obj_info[phyType][xsmiInterface][dev].obj_PTR,
						0,
						sizeof(PRV_MPD_MTD_OBJECT_STC));
                phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR = (MPD_PHY_FW_PARAMETERS_STC*) PRV_MPD_ALLOC_MAC(sizeof(MPD_PHY_FW_PARAMETERS_STC));
                if  (phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(	0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }    
                memset(	phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR,
                        0,
                        sizeof(MPD_PHY_FW_PARAMETERS_STC));
                phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR = (MTD_DEV*) PRV_MPD_ALLOC_MAC(sizeof(MTD_DEV));
                if  (phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(	0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }    
                memset(	phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR,
						0,
						sizeof(MTD_DEV));
                
                phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData = (void*) PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_APP_DATA_STC));
                if  (phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(	0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }    
                memset(	phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData,
                        0,
                        sizeof(PRV_MPD_APP_DATA_STC));
                
                appData_PTR = (PRV_MPD_APP_DATA_STC *) phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData;
                appData_PTR->mdioDev = portEntry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
                appData_PTR->mdioBus = portEntry_PTR->initData_PTR->mdioInfo.mdioBus;
                appData_PTR->phyType = phyType;
                
#ifndef PHY_SIMULATION
                mtdStat = mtdLoadDriver(mpdMtdMdioReadWrap,
										mpdMtdMdioWriteWrap,
										mpdMtdWaitWrap,
										(UINT_16) portEntry_PTR->initData_PTR->mdioInfo.mdioAddress,
										FALSE,
										phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR,
										&err_code);
				if (mtdStat != MTD_OK) {
					PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
												MPD_ERROR_SEVERITY_FATAL_E,
												"failed to load mtd driver (mtdLoadDriver)");
					return MPD_OP_FAILED_E;
				}
#else

                switch (phyType){
                case MPD_TYPE_88X33x0_E:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_3340_A1;
                    break;
                case MPD_TYPE_88X32x0_E:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_3240_A1;
                    break;
                case MPD_TYPE_88X20x0_E:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2040_A1;
                    break;
                case MPD_TYPE_88X2180_E:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2180_A0;
                    break;
                case MPD_TYPE_88E2540_E:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2540_B0;
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 4;
                    break;
                 case MPD_TYPE_88X3540_E:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_X3540_B0;
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 4;
                    break;
                 case MPD_TYPE_88E2580_E:
                     phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2580_A0;
                     phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 8;
                     break;
                default:
                    phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_3340_A1;
                    break;
                }            
                phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->fmtdReadMdio = mpdMtdMdioReadWrap;
                phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->fmtdWriteMdio = mpdMtdMdioWriteWrap;
                phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->devEnabled = MTD_TRUE;
	#endif
				phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->phyType = phyType;

				if (portEntry_PTR->initData_PTR->usxInfo.usxType != MPD_PHY_USX_TYPE_NONE) {
				    prvMpdRepresentativeModulo_ARR[phyType] = 4;
				}

#ifndef PHY_SIMULATION
                /* get PHY FW files */
                if (PRV_MPD_GET_FW_FILES_MAC(	phyType,
                    &phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_main)) {                            
                        mtdGetTunitFwVersionFromImage(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_main.data_PTR,
                            		phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_main.dataSize,
                                    &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS]),
                                    &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS]),
                                    &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS]),
                                    &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS]));
                        
                }
#endif
			}
			
            prvMpdMtdPortDb_ARR[rel_ifIndex] = phys_obj_info[phyType][xsmiInterface][dev].obj_PTR;
            portEntry_PTR->runningData_PTR->phyFw_PTR = phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR;
            prvMpdMtdUpdateParallelList(    portEntry_PTR,
                                            prvMpdGlobalDb_PTR->fwDownloadType_ARR[phyType],
                                            phys_obj_info[phyType][xsmiInterface][dev].representativePerPhyNumber_ARR);
		}
		else {
			prvMpdMtdPortDb_ARR[rel_ifIndex] = NULL;
            portEntry_PTR->runningData_PTR->phyFw_PTR = NULL;
		}
	}


	return MPD_OK_E;

}

