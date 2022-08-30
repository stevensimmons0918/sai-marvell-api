/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdMyd.c
 *	@brief PHY and driver specific PHY implementation.
 *	For all (supported) MYD PHYs
 *
 */


#include    <mpdTypes.h>
#include    <mpd.h>
#include    <mpdPrv.h>

#include    <mydApiTypes.h>
#include    <mydAPI.h>
#include    <mydIntrIOConfig.h>
#include    <mydAPIInternal.h>
#include    <mydApiRegs.h>
#include    <mydDiagnostics.h>
#include    <mydFwDownload.h>
#include    <mydHwCntl.h>
#include    <mydFEC.h>
#include    <mydHwSerdesCntl.h>
#include    <mydSerdes.h>
#include    <mydUtils.h>
#include    <mydInitialization.h>

#include <serdes.0x1093_208D.h>
#include <sbus_master.0x1021_2001.h>
#include <z80_Firmware_v203.nimb.h>

#define MPD_MYD_MAX_IF_IDX  12
#define MPD_MYD_MAX_PHY_MODE_IDX  2

PRV_MPD_MYD_OBJECT_STC *prvMpdMydPortDb_ARR[MPD_MAX_PORT_NUMBER_CNS + 1];
static PRV_MPD_PORT_LIST_TYP prvMpdMydDownloadModeEnableSet;

#define MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset) \
                       if((pDev != NULL) && (pDev->devEnabled == MYD_TRUE))                      \
                       {                                                                         \
                           mdioPort   = (laneNum / pDev->portCount) + pDev->mdioPort;            \
                           laneOffset = (laneNum % MYD_NUM_LANES);                               \
                       }
/* mpd to myd mode table for speed 25G*/
MYD_OP_MODE prvMyd25GModeTable[MPD_MYD_MAX_IF_IDX][MPD_MYD_MAX_PHY_MODE_IDX][MPD_FEC_MODE_LAST_E] =
{
	    /* Retimer Mode                       */          /* PCS Mode                             */
	   /*  FC-FEC       NO-FEC    RS-FEC       BOTH(AP)  RS-FEC(544,514)      FC-FEC		NO-FEC    RS-FEC    BOTH(AP) RS-FEC(544, 514) */
/*KR*/     {{0,        MYD_R25L,        0,         0,      0},         {MYD_P25LN, MYD_P25LF, MYD_P25LR,     0,     0}},
/*KR2*/    {{0,        0,               0,         0,      0},         {0,                 0,         0,     0,     0}},
/*KR4*/    {{0,        0,               0,         0,      0},         {MYD_P25HN, MYD_P25HF,         0,     0,     0}},
/*KR8*/    {{0,        0,               0,         0,      0},         {0,         0,                 0,     0,     0}},
/*CR*/     {{0,        MYD_R25C,        0,         0,      0},         {MYD_P25CN, MYD_P25CF, MYD_P25CR,     0,     0}},
/*CR2*/    {{0,        0,               0,         0,      0},         {MYD_P25ZN, MYD_P25ZF,         0,     0,     0}},
/*CR4*/    {{0,        0,               0,         0,      0},         {MYD_P25GN, MYD_P25GF,         0,     0,     0}},
/*CR8*/    {{0,        0,               0,         0,      0},         {0,         0,                 0,     0,     0}},
/*SR_LR*/  {{0,        MYD_R25L,        0,         0,      0},         {MYD_P25LN, MYD_P25LF, MYD_P25LF,     0,     0}},
/*SR_LR2*/ {{0,        0,               0,         0,      0},         {MYD_P25YN, MYD_P25YF,         0,     0,     0}},
/*SR_LR4*/ {{0,        0,               0,         0,      0},         {MYD_P25AN, 0,                 0,     0,     0}},
/*SR_LR8*/ {{0,        0,               0,         0,      0},         {0,         0,                 0,     0,     0}},
};

/* mpd to myd mode table for speed 50G*/
MYD_OP_MODE prvMyd50GModeTable[MPD_MYD_MAX_IF_IDX][MPD_MYD_MAX_PHY_MODE_IDX][MPD_FEC_MODE_LAST_E] =
{
	    /* Retimer Mode                       */          /* PCS Mode                             */
	   /* FC-FEC          NO-FEC      RS-FEC   BOTH(AP)  RS-FEC(544,514)  FC-FEC	NO-FEC  	RS-FEC      BOTH(AP)    RS-FEC(544, 514) */
/*KR*/     {{0,        MYD_R50U,        0,         0,      0},         {0,         0,         0,             0,   MYD_P50UP}},
/*KR2*/    {{0,        0,               0,         0,      0},         {MYD_P50JF, MYD_P50JN, MYD_P50JR,     0,   0}},
/*KR4*/    {{0,        0,               0,         0,      0},         {MYD_P50KF, MYD_P50KN, 0,             0,   0}},
/*KR8*/    {{0,        0,               0,         0,      0},         {0,         0,         0,             0,   0}},
/*CR*/     {{0,        MYD_R50U,        0,         0,      0},         {0,         0,         0,             0,   MYD_P50UP}},
/*CR2*/    {{0,        MYD_R50C,        0,         0,      0},         {MYD_P50BF, MYD_P50BN, MYD_P50BR,     0,   0}},
/*CR4*/    {{0,        0,               0,         0,      0},         {MYD_P50CF, MYD_P50CN, 0,             0,   0}},
/*CR8*/    {{0,        0,               0,         0,      0},         {0,         0,         0,             0,   0}},
/*SR_LR*/  {{0,        MYD_R50U,        0,         0,      0},         {0,         0,         0,             0,   MYD_P50UP}},
/*SR_LR2*/ {{0,        MYD_R50L,        0,         0,      0},         {MYD_P50MF, MYD_P50MN, MYD_P50MR,     0,   0}},
/*SR_LR4*/ {{0,        0,               0,         0,      0},         {MYD_P50LF, MYD_P50LN, 0,             0,   0}},
/*SR_LR8*/ {{0,        0,               0,         0,      0},         {0,         0,         0,             0,   0}},
};

/* mpd to myd mode table for speed 100G*/
MYD_OP_MODE prvMyd100GModeTable[MPD_MYD_MAX_IF_IDX][MPD_MYD_MAX_PHY_MODE_IDX][MPD_FEC_MODE_LAST_E] =
{
	    /* Retimer Mode                       */          /* PCS Mode                             */
	   /*  FC-FEC      NO-FEC     RS-FEC      BOTH(AP)   RS-FEC(544,514)   FC-FEC		NO-FEC    RS-FEC        BOTH(AP)  RS-FEC(544, 514) */
/*KR*/     {{0,        0,               0,        0,       0},         {0,         0,         0,             0,   0}},
/*KR2*/    {{0,        MYD_R100Q,       0,        0,       0},         {0,         0,         0,             0,   MYD_P100QP}},
/*KR4*/    {{0,        MYD_R100K,       0,        0,       0},         {0,         MYD_P100KN,MYD_P100KR,    0,   0}},
/*KR8*/    {{0,        0,               0,        0,       0},         {0,         0,         0,             0,   0}},
/*CR*/     {{0,        0,               0,        0,       0},         {0,         0,         0,             0,   0}},
/*CR2*/    {{0,        MYD_R100Q,       0,        0,       0},         {0,         0,         0,             0,   MYD_P100QP}},
/*CR4*/    {{0,        MYD_R100C,       0,        0,       0},         {0,         0,         MYD_P100CR,    0,   0}},
/*CR8*/    {{0,        0,               0,        0,       0},         {0,         0,         0,             0,   0}},
/*SR_LR*/  {{0,        MYD_R100L,       0,        0,       0},         {0,         0,         0,             0,   0}},
/*SR_LR2*/ {{0,        MYD_R100U,       0,        0,       0},         {0,         0,         0,             0,   MYD_P100UP}},
/*SR_LR4*/ {{0,        0,               0,        0,       0},         {0,         MYD_P100LN,MYD_P100LR,    0,   0}},
/*SR_LR8*/ {{0,        0,               0,        0,       0},         {0,         0,         0,             0,   0}},
};

/* mpd to myd mode table for speed 200G*/
MYD_OP_MODE prvMyd200GModeTable[MPD_MYD_MAX_IF_IDX][MPD_MYD_MAX_PHY_MODE_IDX][MPD_FEC_MODE_LAST_E] =
{
	    /* Retimer Mode                       */          /* PCS Mode                             */
	   /* FC-FEC       NO-FEC     RS-FEC     BOTH(AP)  RS-FEC(544,514)   FC-FEC		NO-FEC   RS-FEC     BOTH(AP) RS-FEC(544, 514) */
/*KR*/     {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*KR2*/    {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*KR4*/    {{0,        MYD_R200Q,      0,        0,       0},         {0,         0,         0,          0,       MYD_P200QP}},
/*KR8*/    {{0,        MYD_R200K,      0,        0,       0},         {0,         0,         0,          0,       0}},
/*CR*/     {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*CR2*/    {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*CR4*/    {{0,        MYD_R200Q,      0,        0,       0},         {0,         0,         0,          0,       MYD_P200QP}},
/*CR8*/    {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*SR_LR*/  {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*SR_LR2*/ {{0,        0,              0,        0,       0},         {0,         0,         0,          0,       0}},
/*SR_LR4*/ {{0,        MYD_R200U,      0,        0,       0},         {0,         0,         0,          0,       MYD_P200UP}},
/*SR_LR8*/ {{0,        MYD_R200L,      0,        0,       0},         {0,         0,         0,          0,       0}},
};

/* mpd to myd mode table for speed 400G*/
MYD_OP_MODE prvMyd400GModeTable[MPD_MYD_MAX_IF_IDX][MPD_MYD_MAX_PHY_MODE_IDX][MPD_FEC_MODE_LAST_E] =
{
	    /* Retimer Mode                       */          /* PCS Mode                             */
	   /* FC-FEC		NO-FEC       RS-FEC   BOTH(AP) RS-FEC(544,514)   FC-FEC		NO-FEC    RS-FEC   BOTH(AP)    RS-FEC(544, 514) */
/*KR*/     {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*KR2*/    {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*KR4*/    {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*KR8*/    {{0,        MYD_R400Q,      0,         0,      0},         {0,         0,         0,             0,    0}},
/*CR*/     {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*CR2*/    {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*CR4*/    {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*CR8*/    {{0,        MYD_R400Q,      0,         0,      0},         {0,         0,         0,             0,    0}},
/*SR_LR*/  {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*SR_LR2*/ {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*SR_LR4*/ {{0,        0,              0,         0,      0},         {0,         0,         0,             0,    0}},
/*SR_LR8*/ {{0,        MYD_R400U,      0,         0,      0},         {0,         0,         0,             0,    0}},
};

/**
 * wrMydCtrlXsmiRead
 *
 *
 * @param pDev
 * @param mdioPort
 * @param mmd
 * @param reg
 * @param value
 *
 * @return MYD_STATUS
 */
MYD_STATUS wrMydCtrlXsmiRead(
    MYD_DEV_PTR pDev,
    MYD_U16 mdioPort,
    MYD_U16 mmd,
    MYD_U16 reg,
    MYD_U16* value)
{
    PRV_MPD_APP_DATA_STC  *phyInfo = (PRV_MPD_APP_DATA_STC*)pDev->hostContext;
    MPD_RESULT_ENT status = MPD_OP_FAILED_E;
    PRV_MPD_PORT_HASH_ENTRY_STC * portEntry_PTR;
    portEntry_PTR = prvMpdFindPortEntry(phyInfo, mdioPort);
    if (portEntry_PTR == NULL) {
    	return MYD_FAIL;
    }

#ifdef PHY_SIMULATION
    status = MPD_OK_E;
    MPD_UNUSED_PARAM(phyInfo);
    MPD_UNUSED_PARAM(mdioPort);
    /* trying to read Chip FW Revision */
    if ((mmd == MYD_PMA_DEV) && (reg == 0xD8F4)) {
        *value = 0x0103;
    }
#else

    status = PRV_MPD_MDIO_READ_MAC( portEntry_PTR->rel_ifIndex,
                                    mmd,
                                    reg,
                                   (UINT_16*)value);
#endif
    return status;
}

/**
 * wrMydCtrlXsmiWrite
 *
 *
 * @param pDev
 * @param mdioPort
 * @param mmd
 * @param reg
 * @param value
 *
 * @return MYD_STATUS
 */
MYD_STATUS wrMydCtrlXsmiWrite(
    MYD_DEV_PTR pDev,
    MYD_U16 mdioPort,
    MYD_U16 mmd,
    MYD_U16 reg,
    MYD_U16 value)
{
    PRV_MPD_APP_DATA_STC  *phyInfo = (PRV_MPD_APP_DATA_STC*)pDev->hostContext;
    MPD_RESULT_ENT status = MPD_OP_FAILED_E;
    PRV_MPD_PORT_HASH_ENTRY_STC * portEntry_PTR;
    portEntry_PTR = prvMpdFindPortEntry(phyInfo, mdioPort);
    if (portEntry_PTR == NULL) {
    	return MYD_FAIL;
    }


#ifdef PHY_SIMULATION
    status = MPD_OK_E;
    MPD_UNUSED_PARAM(phyInfo);
    MPD_UNUSED_PARAM(mdioPort);
    MPD_UNUSED_PARAM(mmd);
    MPD_UNUSED_PARAM(reg);
    MPD_UNUSED_PARAM(value);
#else

    status = PRV_MPD_MDIO_WRITE_MAC( portEntry_PTR->rel_ifIndex,
                                    mmd,
                                    reg,
                                   (UINT_16)value);
#endif

    return status;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydPreFwDownload
 *
 * DESCRIPTION: Pre FW download
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydPreFwDownload(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
#ifndef   PHY_SIMULATION
		UINT_32 rel_ifIndex;
        MYD_STATUS status = MYD_FAIL;
        MYD_U16 major = 0, minor = 0;
        rel_ifIndex = portEntry_PTR->rel_ifIndex;
#endif
        MPD_UNUSED_PARAM(params_PTR);

	/* Add the first mdio port to init the driver for phy devivce */
        if (portEntry_PTR->initData_PTR->phySlice != 0) {
            return MPD_OK_E;
        }
#ifndef PHY_SIMULATION
        status = mydGetChipFWRevision((MYD_DEV_PTR)PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),
                                        portEntry_PTR->initData_PTR->mdioInfo.mdioAddress,
                                        &major,
                                        &minor);
        if(status != MYD_OK)
        {
            return MPD_OP_FAILED_E;
        }

        if(major == 0)
        {
        /* Firmware not loaded, need to download */
            portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired = TRUE;
        }
        prvMpdPortListAdd(      rel_ifIndex,
                                                        &prvMpdMydDownloadModeEnableSet);
#endif
        return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydFwDownload
 *
 * DESCRIPTION: FW download
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydFwDownload(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
	UINT_32 rel_ifIndex;
	MYD_U16 mdioPort;
	MYD_DEV_PTR pDev;
	MYD_PVOID   pHostContext;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;
	mdioPort = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress;
	pDev = PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex);
	pHostContext = PRV_MPD_MYD_CONTEXT_MAC(rel_ifIndex);
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(params_PTR);
	pDev->devEnabled = MYD_TRUE;
	pDev->fmydReadMdio = wrMydCtrlXsmiRead;
    pDev->fmydWriteMdio = wrMydCtrlXsmiWrite;
	pDev->hostContext = pHostContext;
	pDev->portCount = 4;
	pDev->deviceId = MYD_DEV_X7120;
    pDev->mdioPort = mdioPort;

#else
    params_PTR = params_PTR;    

    MYD_U16         *pBusMasterImage = NULL;
    MYD_U16         busMasterSize;
    MYD_U16         *pSerdesImage = NULL;
    MYD_U16         serdesSize;
    MYD_U16         *pZ80Image = NULL;
    MYD_U16         z80Size;
    MYD_U16         errCode;

    pSerdesImage    = (MYD_U16  *)myd_serdes0x1093_2085Data;
    serdesSize      = AVAGO_SERDES_FW_IMAGE_SIZE;

    pBusMasterImage = (MYD_U16  *)myd_sbusMaster0x1021_2001Data;
    busMasterSize   = AVAGO_SBUS_MASTER_FW_IMAGE_SIZE;
    pZ80Image       = (MYD_U16  *)myd_z80_v203Data;
    z80Size         = Z80_v203_IMAGE_SIZE;
    

    if (prvMpdPortListIsMember(rel_ifIndex, &prvMpdMydDownloadModeEnableSet) == FALSE)
    {
        return TRUE;
    }

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE)
    {
	 /* Initialize without firmware download*/
         mydRc = mydInitDriver(wrMydCtrlXsmiRead,
                               wrMydCtrlXsmiWrite,
                               mdioPort,
                               NULL, 0, NULL, 0, NULL, 0,
                               pHostContext,
                               pDev);

	return TRUE;
    }

    if(prvMpdGlobalDb_PTR->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_RAM_E)
    {
         mydRc = mydInitDriver(wrMydCtrlXsmiRead,
                               wrMydCtrlXsmiWrite,
                               mdioPort,
                               pZ80Image,
                               z80Size,
                               pBusMasterImage,
                               busMasterSize,
                               pSerdesImage,
                               serdesSize,
                               pHostContext,
                               pDev);
    }
    else if(prvMpdGlobalDb_PTR->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_FLASH_E)
    {
         mydRc = mydUpdateEEPROMImage( pDev,
                                       mdioPort,
                                       pBusMasterImage,
                                       busMasterSize,
                                       pSerdesImage,
                                       serdesSize, NULL/*pZ80Image*/, 0/*z80Size*/, &errCode);
    }
    else
    {
         /* Initialize without firmware download*/
         mydRc = mydInitDriver(wrMydCtrlXsmiRead,
                               wrMydCtrlXsmiWrite,
                               mdioPort,
                               NULL, 0, NULL, 0, NULL, 0,
                               pHostContext,
                               pDev);
    }
#endif
    return (mydRc == MYD_OK) ? MPD_OK_E : MPD_OP_FAILED_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydPostFwDownload
 *
 * DESCRIPTION: Post FW download
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydPostFwDownload(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    UINT_32 rel_ifIndex;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    MPD_UNUSED_PARAM(params_PTR);

    prvMpdPortListRemove(   rel_ifIndex,
                                                        &prvMpdMydDownloadModeEnableSet);

    portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired = FALSE;
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: mpdMydInit
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/

extern MPD_RESULT_ENT mpdMydInit(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP *exhw_portList_PTR,
    MPD_TYPE_ENT *phyType_PTR
)
{
    UINT_32 rel_ifIndex;
    UINT_8  phyNumber, dev;
    MPD_TYPE_ENT phyType;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    PRV_MPD_APP_DATA_STC * hostContext_PTR;
    PRV_MPD_MYD_OBJECT_INFO_STC phys_obj_info[MPD_TYPE_NUM_OF_TYPES_E][MPD_MAX_PORT_NUMBER_CNS][PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS];

    memset( phys_obj_info,
                        0,
                        sizeof(phys_obj_info));
    for (rel_ifIndex = 0; prvMpdPortListGetNext(  exhw_portList_PTR,
                                                       &rel_ifIndex) == TRUE;)
    {
        phyType = phyType_PTR[rel_ifIndex];
        portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
        phyNumber = portEntry_PTR->initData_PTR->phyNumber;
        dev = portEntry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
        if (PRV_MPD_VALID_PHY_TYPE(phyType))
        {
            if (phys_obj_info[phyType][phyNumber][dev].exist == FALSE)
            {
                phys_obj_info[phyType][phyNumber][dev].exist = TRUE;
                phys_obj_info[phyType][phyNumber][dev].obj_PTR = (PRV_MPD_MYD_OBJECT_STC*) PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_MYD_OBJECT_STC));
	        if(phys_obj_info[phyType][phyNumber][dev].obj_PTR == NULL)
                {
                        PRV_MPD_HANDLE_FAILURE_MAC( 0,
                                                    MPD_ERROR_SEVERITY_FATAL_E,
                                                    PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }
                memset( phys_obj_info[phyType][phyNumber][dev].obj_PTR,
                                                0,
                                                sizeof(PRV_MPD_MYD_OBJECT_STC));
                phys_obj_info[phyType][phyNumber][dev].phyFw_PTR = (MPD_PHY_FW_PARAMETERS_STC*) PRV_MPD_ALLOC_MAC(sizeof(MPD_PHY_FW_PARAMETERS_STC));
                if  (phys_obj_info[phyType][phyNumber][dev].phyFw_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC( 0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }
                memset( phys_obj_info[phyType][phyNumber][dev].phyFw_PTR,
                        0,
                        sizeof(MPD_PHY_FW_PARAMETERS_STC));

                phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR = (MYD_DEV*) PRV_MPD_ALLOC_MAC(sizeof(MYD_DEV));
                if  (phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC( 0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }

                memset( phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR, 0,sizeof(MYD_DEV));

                phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR->hostContext = (void*) PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_APP_DATA_STC));
                if  (phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR->hostContext == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC( 0,
                                                MPD_ERROR_SEVERITY_FATAL_E,
                                                PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }
                memset( phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR->hostContext,
                        0,
                        sizeof(PRV_MPD_APP_DATA_STC));

                hostContext_PTR = (PRV_MPD_APP_DATA_STC *) phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR->hostContext;
                hostContext_PTR->mdioDev = portEntry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
                hostContext_PTR->mdioBus = portEntry_PTR->initData_PTR->mdioInfo.mdioBus;
                hostContext_PTR->phyType = phyType;


                phys_obj_info[phyType][phyNumber][dev].obj_PTR->mdioBus = portEntry_PTR->initData_PTR->mdioInfo.mdioBus;
                phys_obj_info[phyType][phyNumber][dev].obj_PTR->phyAddr = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress;
                phys_obj_info[phyType][phyNumber][dev].obj_PTR->phyType =  portEntry_PTR->initData_PTR->phyType;
                phys_obj_info[phyType][phyNumber][dev].obj_PTR->hostDevNum = portEntry_PTR->initData_PTR->mdioInfo.mdioDev;
                phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR->fmydReadMdio = wrMydCtrlXsmiRead;
                phys_obj_info[phyType][phyNumber][dev].obj_PTR->mydObject_PTR->fmydWriteMdio = wrMydCtrlXsmiWrite;
            }
            prvMpdMydPortDb_ARR[rel_ifIndex] = phys_obj_info[phyType][phyNumber][dev].obj_PTR;
            portEntry_PTR->runningData_PTR->phyFw_PTR = phys_obj_info[phyType][phyNumber][dev].phyFw_PTR;
        }
        else
        {
            prvMpdMydPortDb_ARR[rel_ifIndex] = NULL;
            portEntry_PTR->runningData_PTR->phyFw_PTR = NULL;
        }
    }
    prvMpdPortListClear(&prvMpdMydDownloadModeEnableSet);
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydIfModeIdxGet
 *
 * DESCRIPTION: get speed table index from interface mode;
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/

UINT_8 prvMpdMydIfModeIdxGet(
    MPD_INTERFACE_MODE_ENT ifMode
)
{
    switch(ifMode)
    {
        case MPD_INTERFACE_MODE_KR_E:
            return 0;
        case MPD_INTERFACE_MODE_KR2_E:
            return 1;
        case MPD_INTERFACE_MODE_KR4_E:
            return 2;
        case MPD_INTERFACE_MODE_KR8_E:
            return 3;
        case MPD_INTERFACE_MODE_CR_E:
            return 4 ;
        case MPD_INTERFACE_MODE_CR2_E:
            return 5;
        case MPD_INTERFACE_MODE_CR4_E:
            return 6;
        case MPD_INTERFACE_MODE_CR8_E:
            return 7;
        case MPD_INTERFACE_MODE_SR_LR_E:
            return 8;
        case MPD_INTERFACE_MODE_SR_LR2_E:
            return 9;
        case MPD_INTERFACE_MODE_SR_LR4_E:
            return 10;
        case MPD_INTERFACE_MODE_SR_LR8_E:
            return 11;
        default:
	    return MPD_INTERFACE_MODE_NA_E;
    }
    return MPD_INTERFACE_MODE_NA_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdModeToMydMode
 *
 * DESCRIPTION: Convert mpd mode to myd mode
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdModeToMydMode(
         MPD_SPEED_ENT              speed,
         MPD_INTERFACE_MODE_ENT     ifMode,
         MPD_FEC_MODE_SUPPORTED_ENT fecMode,
         BOOLEAN                    isRetimerMode,
         MYD_OP_MODE                *opMode
)
{
    UINT_8 idx = 0, modeIdx = 0;

    if(opMode == NULL)
    {
        return MPD_OP_FAILED_E;
    }
    if(speed >= MPD_SPEED_LAST_E  || ifMode >= MPD_INTERFACE_MODE_NA_E || fecMode >= MPD_FEC_MODE_LAST_E )
    {
        return MPD_OP_FAILED_E;
    }

    idx = (isRetimerMode == TRUE) ? 0 : 1;
    modeIdx = prvMpdMydIfModeIdxGet(ifMode);

    if(modeIdx >= MPD_INTERFACE_MODE_NA_E)
    {
        return MPD_OP_FAILED_E;
    }
    switch(speed)
    {
    case MPD_SPEED_25G_E:
        *opMode = prvMyd25GModeTable[modeIdx][idx][fecMode];
        break;
    case MPD_SPEED_50G_E:
        *opMode = prvMyd50GModeTable[modeIdx][idx][fecMode];
        break;
    case MPD_SPEED_100G_E:
        *opMode = prvMyd100GModeTable[modeIdx][idx][fecMode];
        break;
    case MPD_SPEED_200G_E:
        *opMode = prvMyd200GModeTable[modeIdx][idx][fecMode];
        break;
    case MPD_SPEED_400G_E:
        *opMode = prvMyd400GModeTable[modeIdx][idx][fecMode];
        break;
    default:
	*opMode = MYD_OP_MODE_UNKNOWN;
        return MPD_NOT_SUPPORTED_E;
    }

    if(*opMode == MYD_OP_MODE_UNKNOWN)
    {
        return MPD_NOT_SUPPORTED_E;
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMydModeToMpdMode
 *
 * DESCRIPTION: Convert myd mode to mpd mode
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMydModeToMpdMode
(
   IN  MYD_OP_MODE                     opMode,
   OUT MPD_SPEED_ENT                   *speed,
   OUT MPD_INTERFACE_MODE_ENT          *ifMode,
   OUT MPD_FEC_MODE_SUPPORTED_ENT      *fecMode
)
{
    MPD_INTERFACE_MODE_ENT          ifModeIdx;
    MPD_FEC_MODE_SUPPORTED_ENT      fecModeIdx;
    UINT_8                          phyModeIdx;

    if(speed == NULL || ifMode == NULL || fecMode == NULL)
    {
        return MPD_OP_FAILED_E;
    }
    if(opMode >= MYD_OP_MODE_COUNT)
    {
        return MPD_OP_FAILED_E;
    }

    for(ifModeIdx = 0 ; ifModeIdx < MPD_MYD_MAX_IF_IDX; ifModeIdx++)
    {
        for(phyModeIdx = 0 ; phyModeIdx < MPD_MYD_MAX_PHY_MODE_IDX; phyModeIdx++)
        {
            for(fecModeIdx = 0 ; fecModeIdx < MPD_FEC_MODE_LAST_E; fecModeIdx++)
            {
                if(prvMyd25GModeTable[ifModeIdx][phyModeIdx][fecModeIdx] == opMode)
                {
                    *speed   = MPD_SPEED_25G_E;
                    *ifMode  = ifModeIdx;
                    *fecMode = fecModeIdx;
                     return MPD_OK_E;
                }
                else if(prvMyd50GModeTable[ifModeIdx][phyModeIdx][fecModeIdx] == opMode)
                {
                    *speed   = MPD_SPEED_50G_E;
                    *ifMode  = ifModeIdx;
                    *fecMode = fecModeIdx;
                     return MPD_OK_E;
                }
                else if(prvMyd100GModeTable[ifModeIdx][phyModeIdx][fecModeIdx] == opMode)
                {
                    *speed   = MPD_SPEED_100G_E;
                    *ifMode  = ifModeIdx;
                    *fecMode = fecModeIdx;
                     return MPD_OK_E;
                }
                else if(prvMyd200GModeTable[ifModeIdx][phyModeIdx][fecModeIdx] == opMode)
                {
                    *speed   = MPD_SPEED_200G_E;
                    *ifMode  = ifModeIdx;
                    *fecMode = fecModeIdx;
                     return MPD_OK_E;
                }
                else if(prvMyd400GModeTable[ifModeIdx][phyModeIdx][fecModeIdx] == opMode)
                {
                    *speed   = MPD_SPEED_400G_E;
                    *ifMode  = ifModeIdx;
                    *fecMode = fecModeIdx;
                     return MPD_OK_E;
                }
            }
        }
    }
    return MPD_NOT_SUPPORTED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydSetPortMode
 *
 * DESCRIPTION: port speed and mode set
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydSetPortMode(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
    UINT_32 rel_ifIndex;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_SPEED_ENT                   speed;
    MPD_INTERFACE_MODE_ENT          ifMode;
    MPD_FEC_MODE_SUPPORTED_ENT      fecMode;
    MYD_OP_MODE     mydOpModeHost;
    MYD_OP_MODE     mydOpModeLine;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_U16         iCalNIMBStatus=0;
    MYD_U32         modeOption;
    MYD_U16         result;
    UINT_8          portNum;
    BOOLEAN         isRetimerMode;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    portNum = portEntry_PTR->initData_PTR->port;

    speed = params_PTR->phySpeedExt.hostSide.speed;
    ifMode = params_PTR->phySpeedExt.hostSide.ifMode;
    fecMode = params_PTR->phySpeedExt.hostSide.fecMode;
    isRetimerMode = params_PTR->phySpeedExt.isRetimerMode;

    ret = prvMpdModeToMydMode(speed, ifMode, fecMode, isRetimerMode, &mydOpModeHost);
    if(ret != MPD_OK_E)
    {
        return MPD_OP_FAILED_E;
    }

    speed = params_PTR->phySpeedExt.lineSide.speed;
    ifMode = params_PTR->phySpeedExt.lineSide.ifMode;
    fecMode = params_PTR->phySpeedExt.lineSide.fecMode;

    ret = prvMpdModeToMydMode(speed, ifMode, fecMode, isRetimerMode, &mydOpModeLine);
    if(ret != MPD_OK_E)
    {
        return MPD_OP_FAILED_E;
    }

    MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), portNum, mdioPort, laneOffset);

    mydRc = mydLanePowerup(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,MYD_HOST_SIDE,MYD_ALL_LANES);
    if(mydRc != MYD_OK)
    {
        return MPD_OP_FAILED_E;
    }

    mydRc = mydLanePowerup(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,MYD_LINE_SIDE,MYD_ALL_LANES);
    if(mydRc != MYD_OK)
    {
        return MPD_OP_FAILED_E;
    }
    if(mydOpModeHost > MYD_PCS_RESERVE_29) /* Workaroud for retimer modes */
    {
        mydNIMBGetOpStatus(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, laneOffset, &iCalNIMBStatus);
        if (iCalNIMBStatus) /* Stop the iCal polling and auto calibration is started */
        {
            mydRc = mydNIMBSetOp(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, laneOffset, 0);
            if(mydRc != MYD_OK)
            {
                return MPD_OP_FAILED_E;
            }
            mydWait(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), 500);
        }
     }

     modeOption = MYD_MODE_FORCE_RECONFIG | MYD_CHANNEL_SHORT;
     mydRc = mydSetModeSelection(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, laneOffset, mydOpModeHost, mydOpModeLine, modeOption, &result);
     if(mydRc != MYD_OK)
     {
         return MPD_OP_FAILED_E;
     }
     if(mydOpModeHost > MYD_PCS_RESERVE_29) /* Workaroud for retimer modes */
     {
            /* Set the eye range to trigger an iCal calibration; MYD_NIMB_EYE_DEFAULT will use the suggested values */
         mydRc = mydNIMBSetEyeCoeff(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, laneOffset, 0, 0, 0, MYD_NIMB_EYE_DELTA_DEFAULT);
         if(mydRc != MYD_OK)
         {
            return MPD_OP_FAILED_E;
         }
         /* mydNIMBSetICal must be stopped when calling API that issues interrupt calls to the SerDes
            and SBus master. All APIs that required MYD_ENABLE_SERDES_API definition issued interrupt. */
         /* start the iCal polling and auto calibration */
         mydRc = mydNIMBSetOp(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, laneOffset, 1);
         if(mydRc != MYD_OK)
         {
             return MPD_OP_FAILED_E;
         }
    }
    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydGetPortMode
 *
 * DESCRIPTION: get port speed
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydGetPortMode(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);
#else
    portEntry_PTR = portEntry_PTR;
    params_PTR = params_PTR;

    UINT_32 rel_ifIndex;
    MPD_RESULT_ENT ret   = MPD_OK_E;
    MPD_SPEED_ENT                   speed;
    MPD_FEC_MODE_SUPPORTED_ENT      fecMode;
    MPD_INTERFACE_MODE_ENT          ifMode;
    MYD_OP_MODE     mydOpModeHost;
    MYD_OP_MODE     mydOpModeLine;

    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    UINT_8          portNum;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    portNum = portEntry_PTR->initData_PTR->port;

    MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), portNum, mdioPort, laneOffset);

    mydRc = mydGetOpMode(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,MYD_HOST_SIDE,laneOffset,&mydOpModeHost);
    if(mydRc != MYD_OK)
    {
        return MPD_OP_FAILED_E;
    }

    mydRc = mydGetOpMode(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,MYD_LINE_SIDE,laneOffset,&mydOpModeLine);
    if(mydRc != MYD_OK)
    {
        return MPD_OP_FAILED_E;
    }

    ret = prvMydModeToMpdMode(mydOpModeHost, &speed, &ifMode, &fecMode);
    if(ret != MPD_OK_E)
    {
        return MPD_OP_FAILED_E;
    }

    params_PTR->phySpeedExt.hostSide.speed = speed;
    params_PTR->phySpeedExt.hostSide.ifMode = ifMode;
    params_PTR->phySpeedExt.hostSide.fecMode = fecMode;

    ret = prvMydModeToMpdMode(mydOpModeLine, &speed, &ifMode, &fecMode);
    if(ret != MPD_OK_E)
    {
        return MPD_OP_FAILED_E;
    }
    params_PTR->phySpeedExt.lineSide.speed = speed;
    params_PTR->phySpeedExt.lineSide.ifMode = ifMode;
    params_PTR->phySpeedExt.lineSide.fecMode = fecMode;

#endif
	return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydSetSerdesTune
 *
 * DESCRIPTION: set serdes rx and tx lane params
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydSetSerdesTune(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);
#else
    portEntry_PTR = portEntry_PTR;
    params_PTR    = params_PTR;
    UINT_32 rel_ifIndex;
    UINT_16 laneNum;

    MYD_U16        sAddr;

    MYD_U16        mydHostOrLineSide;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mydHostOrLineSide = (params_PTR->phyTune.hostOrLineSide == MPD_PHY_SIDE_HOST_E) ? MYD_HOST_SIDE : MYD_LINE_SIDE;

    if(params_PTR->phyTune.tuneParams.paramsType == MPD_SERDES_PARAMS_TX_E)
    {
        /* Tx Params */
        MYD_16         pre;
        MYD_16         pre2;
        MYD_16         pre3;
        MYD_16         atten;
        MYD_16         post;

        pre   =  params_PTR->phyTune.tuneParams.txTune.pre;
        pre2  =  params_PTR->phyTune.tuneParams.txTune.pre2;
        pre3  =  params_PTR->phyTune.tuneParams.txTune.pre3;
        atten =  params_PTR->phyTune.tuneParams.txTune.atten;
        post  =  params_PTR->phyTune.tuneParams.txTune.post;

        for (laneNum = 0; laneNum < (PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex)->portCount*MYD_NUM_LANES); laneNum++)
        {
            if ((params_PTR->phyTune.lanesBmp & (1<<laneNum)) == 0) continue;

            MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), laneNum, mdioPort, laneOffset);

            if(mydGetSerdesAddress(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,mydHostOrLineSide,laneOffset,&sAddr) == MYD_OK)
            {
                mydRc = mydSerdesSetTxFFE(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),sAddr,pre,pre2,pre3,atten,post);
                if(mydRc != MYD_OK)
                {
	            return MPD_OP_FAILED_E;
                }
            }
       }
    }
    else if(params_PTR->phyTune.tuneParams.paramsType == MPD_SERDES_PARAMS_RX_E)
    {
        /* Rx Params */
        MYD_U16        dc;
        MYD_U16        lf;
        MYD_U16        hf;
        MYD_U16        bw;

        dc = params_PTR->phyTune.tuneParams.rxTune.dc;
        lf = params_PTR->phyTune.tuneParams.rxTune.lf;
        hf = params_PTR->phyTune.tuneParams.rxTune.hf;
        bw = params_PTR->phyTune.tuneParams.rxTune.bw;

        for (laneNum = 0; laneNum < (PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex)->portCount*MYD_NUM_LANES); laneNum++)
        {
            /* portsBmp contains only valid ports - must be verfied by caller function */
            if ((params_PTR->phyTune.lanesBmp & (1<<laneNum)) == 0) continue;

            MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), laneNum, mdioPort, laneOffset);

            if(mydGetSerdesAddress(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,mydHostOrLineSide,laneOffset,&sAddr) == MYD_OK)
            {
                /* Check ical pcal order */
                mydRc = mydSerdesTuneCTLE(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),sAddr,dc,lf,hf,bw,MYD_FIXED_CTLE_DISABLED);
                if(mydRc != MYD_OK)
                {
	            return MPD_OP_FAILED_E;
                }
                mydRc = mydSerdesTune_pCal(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),sAddr,MYD_DFE_START_ADAPTIVE);
                if(mydRc != MYD_OK)
                {
	            return MPD_OP_FAILED_E;
                }
            }
        }
    }
    else
    {
        return MPD_NOT_SUPPORTED_E;
    }
#endif

    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydSetSerdesLanePolarity
 *
 * DESCRIPTION: set serdes lane polarity
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydSetSerdesLanePolarity(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
    MPD_UNUSED_PARAM(portEntry_PTR);
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(params_PTR);
#else
    UINT_32 rel_ifIndex;

    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_U16         mydHostOrLineSide;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    mydHostOrLineSide = (params_PTR->phyTune.hostOrLineSide == MPD_PHY_SIDE_HOST_E) ? MYD_HOST_SIDE : MYD_LINE_SIDE;


    MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), params_PTR->phySerdesPolarity.laneNum, mdioPort, laneOffset);
    mydRc = mydSetTxPolarity(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, mydHostOrLineSide, laneOffset, params_PTR->phySerdesPolarity.invertTx,1);
    if(mydRc != MYD_OK)
    {
        return MPD_OP_FAILED_E;
    }
    mydRc = mydSetRxPolarity(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, mydHostOrLineSide, laneOffset, params_PTR->phySerdesPolarity.invertRx,1);
    if(mydRc != MYD_OK)
    {
        return MPD_OP_FAILED_E;
    }
#endif
    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydSetAutoNeg
 *
 * DESCRIPTION: port speed and mode set
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydSetAutoNeg(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
	portEntry_PTR = portEntry_PTR;
	params_PTR    = params_PTR;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydGetLaneBmp
 *
 * DESCRIPTION: Get the port lanes based on port mode.
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydGetPortLaneBmp(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);
#else
    MPD_UNUSED_PARAM(params_PTR);
    UINT_32 rel_ifIndex;
    UINT_16 laneCount, laneIndex;


    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_DEV_PTR     pDev = NULL;
    UINT_8          portNum;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    portNum = portEntry_PTR->initData_PTR->port;
    pDev = PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex);

    MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), portNum, mdioPort, laneOffset);

    params_PTR->internal.phyLaneBmp.hostSideLanesBmp = 0;
    mdioPort = MYD_GET_PORT_IDX(pDev, mdioPort);
    if (mdioPort >= MYD_MAX_PORTS) {
    	PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex, MPD_ERROR_SEVERITY_FATAL_E, "mdioPort out of range");
    	return MPD_OP_FAILED_E;
    }
    laneCount = pDev->hostConfig[mdioPort][laneOffset].laneCount;

    for(laneIndex = 0; laneIndex < laneCount ; laneIndex++)
    {
        params_PTR->internal.phyLaneBmp.hostSideLanesBmp |= 1 << (portNum+laneIndex);
    }

    params_PTR->internal.phyLaneBmp.lineSideLanesBmp = 0;
    laneCount = pDev->lineConfig[mdioPort][laneOffset].laneCount;
    for(laneIndex = 0; laneIndex < laneCount ; laneIndex++)
    {
        params_PTR->internal.phyLaneBmp.lineSideLanesBmp |= 1 << (portNum+laneIndex);
    }
#endif
    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydGetPortLinkStatus
 *
 * DESCRIPTION: Get the port link status.
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydGetPortLinkStatus(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);
#else
    params_PTR    = params_PTR;
    UINT_32 rel_ifIndex;


    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    UINT_8          portNum;
    MYD_U16                     linkStatus;
    MYD_PCS_LINK_STATUS         statusDetail;
    MYD_U16                     latchedStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    portNum = portEntry_PTR->initData_PTR->port;

    MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), portNum, mdioPort, laneOffset);
    mydRc = mydReadPCSLinkStatus(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,laneOffset,&linkStatus,&latchedStatus,&statusDetail);
        /*osPrintf("\ncurrentStatus#%d latchedStatus#%d hCur#%d hLat#%d lCur#%d lLat#%d\n",*linkStatus,latchedStatus, \
                 statusDetail.hostCurrent,statusDetail.hostLatched,statusDetail.lineCurrent,statusDetail.lineLatched);*/

    if( mydRc == MYD_OK )
    {
        params_PTR->phyInternalOperStatus.isOperStatusUp = linkStatus;
    }
#endif
    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydSetDisable
 *
 * DESCRIPTION: port disable.
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydSetDisable(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);
#else
    UINT_32 rel_ifIndex;
    UINT_8          portNum;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    portNum = portEntry_PTR->initData_PTR->port;
    if(params_PTR->phyDisable.forceLinkDown == TRUE)
    {
        MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), portNum, mdioPort, laneOffset);
        mydRc = mydLanePowerdown(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,MYD_HOST_SIDE,MYD_ALL_LANES);
        if(mydRc != MYD_OK)
        {
            return MPD_OP_FAILED_E;
        }

        mydRc = mydLanePowerdown(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex),mdioPort,MYD_LINE_SIDE,MYD_ALL_LANES);
        if(mydRc != MYD_OK)
        {
            return MPD_OP_FAILED_E;
        }
    }
    MPD_UNUSED_PARAM(laneOffset);
#endif
    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydGetTemperature
 *
 * DESCRIPTION: port disable.
 *
 * APPLICABLE PHY:
 *                                      MPD_TYPE_88X7120_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydGetTemperature(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        /*     INPUTS / OUTPUTS:   */
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     OUTPUTS:            */
)
{
    MYD_STATUS     mydRc = MYD_OK;
#ifdef PHY_SIMULATION
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);
#else
    UINT_32 rel_ifIndex;
    UINT_8          portNum;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_32          thermal = 0;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    portNum = portEntry_PTR->initData_PTR->port;

    MPD_MYD_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), portNum, mdioPort, laneOffset);

    mydRc = mydSerdesGetTemperature(PRV_MPD_MYD_OBJECT_MAC(rel_ifIndex), mdioPort, &thermal );
    if(mydRc == MYD_OK)
    {
        params_PTR->phyTemperature.temperature = thermal;
    }

    MPD_UNUSED_PARAM(laneOffset);
#endif
    return (mydRc == MYD_OK) ? MPD_OK_E: MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMydInit7120
 *
 * DESCRIPTION: Init for 7120
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMydInit7120(
        /*     INPUTS:             */
        PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
        MPD_OPERATIONS_PARAMS_UNT *params_PTR
        /*     INPUTS / OUTPUTS:   */
        /*     OUTPUTS:            */
)
{

    MPD_UNUSED_PARAM(params_PTR);
    MPD_UNUSED_PARAM(portEntry_PTR);
    /* Disable PHY on init so that network ports will not be active on system startup */
#if 0
    /* TODO: there is issue with link up on some ports during power up , if disable is done at init */
    MPD_OPERATIONS_PARAMS_UNT phy_params;
    memset( &phy_params,
                      0,
                      sizeof(phy_params));
    phy_params.phyDisable.forceLinkDown = TRUE;
    prvMpdMydSetDisable(  portEntry_PTR,
                                                        &phy_params);
#endif
    return MPD_OK_E;
}
