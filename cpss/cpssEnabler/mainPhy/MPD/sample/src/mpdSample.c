/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdSample.c
*
* @brief This file contains sample functions code for driver
*  initialization and demo use of other MPD API's
*   For Reference Only.
*
***********************************************************************/
#include <mpd.h>
#include <mpdPrv.h>
#include <sample/h/mpdSample.h>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "time.h"
/**
 * @defgroup Example Example Code
 * @{
 */

#ifdef MPD_SMAPLE_WITH_CPSS
	#include <mainPpDrv/h/cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#endif

static MPD_SAMPLE_DEBUG_FLAG_INFO_STC mpdSampleDebugFlags[MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS];
static UINT_32 MPD_SAMPLE_DEBUG_FREE_INDEX = 0;
static MPD_SAMPLE_PORT_DB_STC mpdSamplePortDb[MPD_MAX_PORT_NUMBER_CNS] = {
/*	valid, dev	port	mdioType					phyType				phyNumber	phySlice	mdioBus		mdioAddress	        usxInfo*/
    {0,     0,  0,      0,                              0,                  0,          0,          0,          0,           MPD_PHY_USX_TYPE_NONE     },
    {1,     0,	0,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88X7120_E,		0,			0,			0,			0,           MPD_PHY_USX_TYPE_NONE     }, /* 1 */
    {1,     0,	1,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88X7120_E,		0,			1,			0,			4,           MPD_PHY_USX_TYPE_NONE     }, /* 2 */	
    {1,     0,	2,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88X7120_E,		0,			2,			0,			8,           MPD_PHY_USX_TYPE_NONE     }, /* 3 */
    {1,     0,	3,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88X7120_E,		0,			3,			0,			0xc,         MPD_PHY_USX_TYPE_NONE     }, /* 4 */	
    {1,     0,	4,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			0,			1,			0,           MPD_PHY_USX_TYPE_NONE     }, /* 5 */
    {1,     0,	5,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			2,			1,			2,           MPD_PHY_USX_TYPE_NONE     }, /* 6 */
    {1,     0,	6,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			1,			1,			1,           MPD_PHY_USX_TYPE_NONE     }, /* 7 */
    {1,     0,  7,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			3,			1,			3,           MPD_PHY_USX_TYPE_NONE     }, /* 8 */
    {1,     0,	8,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			4,			1,			4,           MPD_PHY_USX_TYPE_NONE     }, /* 9 */
    {1,     0,	9,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			6,			1,			6,           MPD_PHY_USX_TYPE_NONE     }, /* 10 */
    {1,     0,	10,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			5,			1,			5,           MPD_PHY_USX_TYPE_NONE     }, /* 11 */
    {1,     0,	11,		MPD_SAMPLE_MDIO_TYPE_SMI_E,	MPD_TYPE_88E1680_E,		1,			7,			1,			7,           MPD_PHY_USX_TYPE_NONE     }, /* 12 */
	{1,     0,  24,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		2,			0,			1,			0,           MPD_PHY_USX_TYPE_OXGMII   }, /* 13 */
	{1,     0,  25,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		2,			1,			1,			1,           MPD_PHY_USX_TYPE_OXGMII   }, /* 14 */
	{1,     0,  26,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		2,			2,			1,			2,           MPD_PHY_USX_TYPE_OXGMII   }, /* 15 */
	{1,     0,  27,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		2,			3,			1,			3,           MPD_PHY_USX_TYPE_OXGMII   },  /* 16 */
    {1,     0,  28,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		3,			0,			1,			4,           MPD_PHY_USX_TYPE_OXGMII   }, /* 13 */
    {1,     0,  29,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		3,			1,			1,			5,           MPD_PHY_USX_TYPE_OXGMII   }, /* 14 */
    {1,     0,  30,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		3,			2,			1,			6,           MPD_PHY_USX_TYPE_OXGMII   }, /* 15 */
    {1,     0,  31,		MPD_SAMPLE_MDIO_TYPE_XSMI_E,MPD_TYPE_88E2540_E,		3,			3,			1,			7,           MPD_PHY_USX_TYPE_OXGMII   }  /* 16 */
};

#define MPD_SAMPLE_NUM_OF_PORTS (sizeof(mpdSamplePortDb)/sizeof(MPD_SAMPLE_PORT_DB_STC))
/**
 * @defgroup example-callback Callbacks
 * @{
 */

/**
 * @brief   example implementation of OS delay
 *
 * @return Success/Fail
 */

BOOLEAN mpdSampleOsDelay (
    IN UINT_32 delay /* in nanoseconds */
)
{
	struct timespec sleep, remain;
	if (1000000000 < delay) {
		/* sleep time to big */
		return 0;
	}

	sleep.tv_nsec = delay;
	sleep.tv_sec = 0;
	if (nanosleep(	&sleep, &remain)) {
		return TRUE;
	}

	return FALSE;
}

/**
 * @brief   example implementation of OS Malloc
 *
 * @return Pointer to allocated memory or NULL in case of failure
 */

void * mpdSampleOsMalloc (
    IN UINT_32 size
)
{
    return malloc(size);
}
/**
 * @brief   example implementation of OS Free
 *
 */

void mdpSampleOsFree(
	IN void*	data_PTR
)
{
    free(data_PTR);
}
/**
 * @brief   example implementation of debug bind function
 *
 * @return Success/Fail
 */
BOOLEAN mpdSampleDebugBind (
    IN const char   * comp_PTR,
	IN const char   * pkg_PTR,
	IN const char   * flag_PTR,
	IN const char   * help_PTR,
    OUT UINT_32		* flagId_PTR
)
{
	UINT_32 len = 0,index;
	MPD_UNUSED_PARAM(flag_PTR);
	if (MPD_SAMPLE_DEBUG_FREE_INDEX >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS) {
		/* no more debug flags */
		return FALSE;
	}
	index = MPD_SAMPLE_DEBUG_FREE_INDEX++;
	mpdSampleDebugFlags[index].isFree = FALSE;
	mpdSampleDebugFlags[index].flagStatus = FALSE;

	len += (comp_PTR != NULL)?strlen(comp_PTR):0;
	len += (pkg_PTR != NULL)?strlen(pkg_PTR):0;
	len += (flag_PTR != NULL)?strlen(flag_PTR):0;

	if (len) {
		len += 6;
		mpdSampleDebugFlags[index].flagName_PTR = mpdSampleOsMalloc(len);
		mpdSampleDebugFlags[index].flagName_PTR[0] = '\0';
		strcat(mpdSampleDebugFlags[index].flagName_PTR,comp_PTR);
		strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
		strcat(mpdSampleDebugFlags[index].flagName_PTR,pkg_PTR);
		strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
		strcat(mpdSampleDebugFlags[index].flagName_PTR,flag_PTR);

	}
	len = (help_PTR != NULL)?strlen(help_PTR):0;
	if (len) {
		mpdSampleDebugFlags[index].flagHelp_PTR = mpdSampleOsMalloc(len);
		mpdSampleDebugFlags[index].flagHelp_PTR[0] = '\0';
		strncpy(mpdSampleDebugFlags[index].flagHelp_PTR,help_PTR,len);
	}
	*flagId_PTR = index;
	return TRUE;
}
/**
 * @brief   example implementation of debug check is Flag active
 *
 * @return Active / InActive
 */
BOOLEAN mpdSampleDebugIsActive (
    IN UINT_32	flagId
)
{
	if (flagId >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS) {
		return FALSE;
	}
	if (mpdSampleDebugFlags[flagId].isFree == TRUE) {
		return FALSE;
	}
	return mpdSampleDebugFlags[flagId].flagStatus;
}

/**
 * @brief   example implementation of debug log (print)
 *
 */

void mpdSampleDebugLog (
    IN const char    * funcName_PTR,
    IN const char    * format_PTR,
    IN ...
)
{

	va_list      argptr;

	printf("%s:",funcName_PTR);
	va_start(argptr, format_PTR);
	vprintf( format_PTR, argptr);
	va_end(argptr);
	printf("\n");
}

/**
 * @brief   example implementation of failure handling function
 *
 * @return Success/Fail (of failure handler)
 */

MPD_RESULT_ENT mpdSampleHandleFailure(
	IN UINT_32      			rel_ifIndex,
	IN MPD_ERROR_SEVERITY_ENT	severity,
	IN UINT_32		 			line,
	IN const char 				* calling_func_PTR,
	IN const char				* error_text_PTR
)
{
	char * severityText[MPD_ERROR_SEVERITY_MINOR_E+1] = {"Fatal", "Error", "Minor"};
	printf("Failure Level [%s] on port [%d] line [%d] called by [%s].  %s\n",
		(severity<=MPD_ERROR_SEVERITY_MINOR_E)?severityText[severity]:"Unknown",
				rel_ifIndex,
				line,
				calling_func_PTR,
				error_text_PTR);

	if (severity == MPD_ERROR_SEVERITY_FATAL_E){
		exit(0);
	}

	return MPD_OK_E;
}

/**
 * @brief   example implementation of get PHY firmware files
 *
 * @return Success/Fail
 */
BOOLEAN mpdSampleGetFwFiles  (
	IN  MPD_TYPE_ENT	  	  phyType,
	INOUT MPD_FW_FILE_STC	* mainFile_PTR
)
{
	char * fwFileName_PTR = NULL;
	FILE * fp;
	UINT_32	fileSize;

	if (phyType == MPD_TYPE_88E2540_E) {
		fwFileName_PTR = "samples/resources/v0A030000_10979_10954_e2540.hdr";
	}
	if (phyType == MPD_TYPE_88X3540_E) {
		fwFileName_PTR = "samples/resources/v0A030000_10979_10954_x3540.hdr";
	}
	if (fwFileName_PTR == NULL) {
		return FALSE;
	}	
	fp = fopen(fwFileName_PTR,"r");
	if (fp) {
	fseek(fp, 0 , SEEK_END);
	  fileSize = ftell(fp);
	  fseek(fp, 0 , SEEK_SET);
	  mainFile_PTR->dataSize = fileSize;
	  mainFile_PTR->data_PTR = mpdSampleOsMalloc(fileSize);

	  if (mainFile_PTR->data_PTR == NULL) {
		  fclose(fp);
		  return FALSE;
	  }
	  fread(mainFile_PTR->data_PTR, sizeof(char), fileSize, fp);
	  fclose(fp);
	}
	return TRUE;
}

/**
 * @brief   example to print logging information
 *
 * @return Success/Fail
 */
MPD_RESULT_ENT mpdSampleLogging(
    const char * log_text_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    printf(log_text_PTR);
    
    return MPD_OK_E;
}

#ifdef MPD_SMAPLE_WITH_CPSS
BOOLEAN mpdSampleDisableSmiAutoNeg(
    /*     INPUTS:             */
    UINT_8                          dev,
    UINT_8                          port,
    BOOLEAN                         disable,
	BOOLEAN						  * prev_status_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
	GT_BOOL				prev_state, new_status;
	GT_STATUS 			gt_status;

    gt_status = cpssDxChPhyAutonegSmiGet(dev, port, &prev_state);
    if (gt_status != GT_OK) {
    	return FALSE;
    }

    (* prev_status_PTR) = (prev_state == GT_TRUE)?FALSE:TRUE;
    new_status = ( disable == TRUE ) ? GT_FALSE : GT_TRUE;
    if (prev_state != new_status) {
    	gt_status = cpssDxChPhyAutonegSmiSet(dev, port, new_status );
    	if (gt_status != GT_OK) {
    		return FALSE;
    	}
    }

    return TRUE;
}


/**
 * @brief MPD_MDIO_WRITE_FUNC \n
 * 	prototype function for write to PHY
 */
BOOLEAN mpdSampleMdioWrite (
	IN UINT_32      rel_ifIndex,
	IN UINT_8		mdioAddress,
	IN UINT_16      deviceOrPage,
	IN UINT_16      address,
	IN UINT_16      value
)
{
    UINT_8      							smiDev, port;
    UINT_8      							smiInterface;
    UINT_32     							portGroup, portGroupsBmp;
	MPD_RESULT_ENT							result;
	PRV_MPD_PORT_HASH_ENTRY_STC 		  * port_entry_PTR;

	port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }
    port = port_entry_PTR->initData_PTR->port;
    smiDev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

	if (mpdSamplePortDb[rel_ifIndex].mdioType == MPD_SAMPLE_MDIO_TYPE_SMI_E) {	
		result = cpssSmiRegisterWriteShort(	smiDev,
											portGroupsBmp,
											smiInterface,
											mdioAddress,
											address,
											value);	
	}
	else {
		result = cpssXsmiPortGroupRegisterWrite(	smiDev,
														portGroupsBmp,
														smiInterface,
														mdioAddress,
														address,
														deviceOrPage,
														value);
	}
	return TRUE;
}

/**
 * @brief MPD_MDIO_READ_FUNC \n
 * 	prototype function for read from PHY
 */

BOOLEAN mpdSampleMdioRead (
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		 mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
    OUT UINT_16    * value_PTR
)
{
	UINT_8      							smiDev, port;
	UINT_8      							smiInterface;
	UINT_32     							portGroup, portGroupsBmp;
	MPD_RESULT_ENT							result;
	PRV_MPD_PORT_HASH_ENTRY_STC 		  * port_entry_PTR;

	port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (port_entry_PTR == NULL) {
		return FALSE;
	}

	if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
		return FALSE;
	}
	port = port_entry_PTR->initData_PTR->port;
	smiDev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
	smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
	portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
	portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

	if (mpdSamplePortDb[rel_ifIndex].mdioType == MPD_SAMPLE_MDIO_TYPE_SMI_E) {	
		result = cpssSmiRegisterReadShort(	smiDev,
													portGroupsBmp,
													smiInterface,
													mdioAddress,
													address,
													value_PTR);
	}
	else {
		result = cpssXsmiPortGroupRegisterRead(	smiDev,
														portGroupsBmp,
														smiInterface,
														mdioAddress,
														address,
														deviceOrPage,
														value_PTR);
	}
	return TRUE;
}
#else

BOOLEAN mpdSampleMdioWrite (
	IN UINT_32      rel_ifIndex,
	IN UINT_8		mdioAddress,
	IN UINT_16      deviceOrPage,
	IN UINT_16      address,
	IN UINT_16      value
)
{
	MPD_UNUSED_PARAM(rel_ifIndex);
	MPD_UNUSED_PARAM(mdioAddress);
	MPD_UNUSED_PARAM(deviceOrPage);
	MPD_UNUSED_PARAM(address);
	MPD_UNUSED_PARAM(value);
	/* System-dependent MDIO Write function */
	return TRUE;
}
BOOLEAN mpdSampleMdioRead (
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
	OUT UINT_16    * value_PTR
)
{
	MPD_UNUSED_PARAM(rel_ifIndex);
	MPD_UNUSED_PARAM(mdioAddress);
	MPD_UNUSED_PARAM(deviceOrPage);
	MPD_UNUSED_PARAM(address);
	MPD_UNUSED_PARAM(value_PTR);
	/* System-dependent MDIO Read function */
	return TRUE;
}
#endif

/**
 * @} example-callback
 */


/**
 * @defgroup Example-main example reference functions
 * @{
 */

/**
 * @brief   sample implementation of application initialization of MPD
 *
 * @return MPD_RESULT_ENT
 */
 BOOLEAN mpdSampleInit(
    /*     INPUTS:             */
    void
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32           				rel_ifIndex, i;
    MPD_RESULT_ENT 					rc = MPD_OK_E;
    MPD_CALLBACKS_STC				phy_callbacks;
    MPD_PORT_INIT_DB_STC			phy_entry;
    MPD_SAMPLE_PORT_DB_STC		*	db_entry_PTR;

    memset(mpdSampleDebugFlags,0,sizeof(mpdSampleDebugFlags));
    /* bind basic callbacks needed from host application */
    memset(&phy_callbacks,0,sizeof(phy_callbacks));

    phy_callbacks.sleep_PTR =               mpdSampleOsDelay;
    phy_callbacks.alloc_PTR =               mpdSampleOsMalloc;
    phy_callbacks.free_PTR =				mdpSampleOsFree;
	phy_callbacks.debug_bind_PTR =          mpdSampleDebugBind;
    phy_callbacks.is_active_PTR =           mpdSampleDebugIsActive;
    phy_callbacks.debug_log_PTR =           mpdSampleDebugLog;
    phy_callbacks.txEnable_PTR =            NULL;
    phy_callbacks.handle_failure_PTR =      mpdSampleHandleFailure;
    phy_callbacks.getFwFiles_PTR = 			mpdSampleGetFwFiles;
    phy_callbacks.mdioRead_PTR =            mpdSampleMdioRead;
    phy_callbacks.mdioWrite_PTR =           mpdSampleMdioWrite;
#ifdef MPD_SMAPLE_WITH_CPSS
    phy_callbacks.smiAn_disable_PTR =       mpdSampleDisableSmiAutoNeg;
#endif
	phy_callbacks.logging_PTR = mpdSampleLogging;

    /* initialize DBs & bind debug log ids */
    rc = mpdDriverInitDb(&phy_callbacks);
   if (rc != MPD_OK_E) {
        PRV_MPD_HANDLE_FAILURE_MAC( 0, MPD_ERROR_SEVERITY_FATAL_E, "mpdSampleInit failed in mpdDriverInitDb");
        return FALSE;
    }
   for (i = 0; i < MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS; i++) {
	   mpdSampleDebugFlags[i].flagStatus = TRUE;
   }

    for (rel_ifIndex = 0,i = 0;i < MPD_SAMPLE_NUM_OF_PORTS && rel_ifIndex < MPD_MAX_PORT_NUMBER_CNS; rel_ifIndex++) {
    	if (mpdSamplePortDb[rel_ifIndex].valid == FALSE) {
    		continue;
    	}
    	db_entry_PTR = &mpdSamplePortDb[rel_ifIndex];
    	memset(&phy_entry,0,sizeof(phy_entry));
    	phy_entry.port 							= db_entry_PTR->port;
    	phy_entry.phyNumber 					= db_entry_PTR->phyNumber;
    	phy_entry.phyType 						= db_entry_PTR->phyType;
    	phy_entry.transceiverType				= MPD_TRANSCEIVER_COPPER_E;
    	phy_entry.phySlice 						= db_entry_PTR->phySlice;
        phy_entry.usxInfo.usxType               = db_entry_PTR->usxInfo;
    	phy_entry.mdioInfo.mdioAddress 			= db_entry_PTR->mdioAddress;
    	phy_entry.mdioInfo.mdioDev 				= db_entry_PTR->dev;
    	phy_entry.mdioInfo.mdioBus 				= db_entry_PTR->mdioBus;
    	/* Disable PHY on init so that network ports will not be active on system startup */
    	phy_entry.disableOnInit = TRUE;
    	if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
            PRV_MPD_HANDLE_FAILURE_MAC( 0, MPD_ERROR_SEVERITY_FATAL_E, "mpdSampleInit failed in mpdPortDbUpdate");
    		return FALSE;
    	}
    	i++;
    }

    /* init phy driver */
    /* initialize PHY FW download method: burn to the PHY flash, or copy it to the PHY RAM - update in get fw file*/

    for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++){
        prvMpdGlobalDb_PTR->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_FLASH_E;
    }

    if (mpdDriverInitHw() != MPD_OK_E) {
    	PRV_MPD_HANDLE_FAILURE_MAC( 0, MPD_ERROR_SEVERITY_FATAL_E, "mpdSampleInit failed in mpdDriverInitHw");
    }
    return TRUE;
}

 MPD_RESULT_ENT mpdSampleSetPortAdminOn(
	IN UINT_32 rel_ifIndex
)
{
	 MPD_RESULT_ENT				result;
	 MPD_OPERATIONS_PARAMS_UNT	phyParams;

	if (mpdSamplePortDb[rel_ifIndex].valid == FALSE) {
		return FALSE;
	}

	memset(&phyParams,0,sizeof(phyParams));
	phyParams.phyDisable.forceLinkDown = FALSE;
	result = mpdPerformPhyOperation(	rel_ifIndex,
										MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
										&phyParams);
	return result;

}
int main() {

	UINT_32						rel_ifIndex;
	MPD_RESULT_ENT				result;
	MPD_OPERATIONS_PARAMS_UNT	phyParams;

	mpdSampleInit();
	printf("======================== INIT DONE ========================\n");
	/* set all ports to admin up */
	for (rel_ifIndex = 0; prvMpdPortListGetNext(&prvMpdGlobalDb_PTR->validPorts, &rel_ifIndex);) {
		mpdSampleSetPortAdminOn(rel_ifIndex);
	}
	/* read link status on port 1 */
	memset(&phyParams,0,sizeof(phyParams));
	result = mpdPerformPhyOperation(	1,
										MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
										&phyParams);
	if (result == MPD_OK_E) {
		printf(	"rel_ifIndex [%d] reports link [%s] Auto-negotiation is [%s Complete]\n",
				1,
				((phyParams.phyInternalOperStatus.isOperStatusUp)?"UP":"Down"),
				phyParams.phyInternalOperStatus.isAnCompleted?"":"Not");
	}
	else {
		printf("read internal status on rel_ifIndex [%d] failed\n", 1);
	}
	/* set 200G speed on 88X7120 */
	phyParams.phySpeedExt.hostSide.speed = MPD_SPEED_200G_E;
    phyParams.phySpeedExt.hostSide.ifMode = MPD_INTERFACE_MODE_SR_LR4_E;
    phyParams.phySpeedExt.hostSide.fecMode = MPD_FEC_MODE_DISABLED_E;
    phyParams.phySpeedExt.isRetimerMode = TRUE;
    phyParams.phySpeedExt.lineSide.speed = MPD_SPEED_200G_E;
    phyParams.phySpeedExt.lineSide.ifMode = MPD_INTERFACE_MODE_CR4_E;
    phyParams.phySpeedExt.lineSide.fecMode = MPD_FEC_MODE_DISABLED_E;	
	mpdPerformPhyOperation(	1,
										MPD_OP_CODE_SET_SPEED_EXT_E,
										&phyParams);
    printf("set rel_ifIndex [%d] speed to 200G\n", 9);	
	
	return 1;
}
/**
 * @} Example-main
 * @} Example
 */
