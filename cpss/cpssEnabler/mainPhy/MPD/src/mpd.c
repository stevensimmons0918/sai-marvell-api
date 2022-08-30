/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpd.c
 *	@brief PHY and driver specific PHY implementation.
 *	For all (supported) Alaska 1G/100M PHYs
 *
 */


#include 		<mpd.h>
#include 		<mpdPrv.h>
#include        <mpdDebug.h>
#include        <math.h>

#undef __FUNCTION__
static char prvMpdDebugComponentName[] = "phy";
static char prvMpdDebugPckName[] = "drv";

static char prvMpdDebugTraceFlagName[] = "trace";
static char prvMpdDebugTraceFlagHelp[] = "Debug Trace for PHY ";
UINT_32 prvMpdDebugTraceFlagId;

static char prvMpdDebugConfigVctFlagName[] = "vct";
static char prvMpdDebugConfigVctFlagHelp[] = "debug PHY vct operations";
static UINT_32 prvMpdDebugConfigVctFlagId;

static char prvMpdDebugWriteFlagName[] = "write";
static char prvMpdDebugWriteFlagHelp[] = "trace register write per port ";
UINT_32 prvMpdDebugWriteFlagId;

static char prvMpdDebugReadFlagName[] = "read";
static char prvMpdDebugReadFlagHelp[] = "trace register read per port ";
UINT_32 prvMpdDebugReadFlagId;

static char prvMpdDebugSfpPckName[] = "sfp";
static char prvMpdDebugSfpFlagName[] = "state";
static char prvMpdDebugSfpFlagHelp[] = "debug PHY SFP operations";
UINT_32 prvMpdDebugSfpFlagId;

static char prvMpdDebugErrorFlagName[] = "error";
static char prvMpdDebugErrorFlagHelp[] = "debug PHY config errors";
UINT_32 prvMpdDebugErrorFlagId;

static char prvMpdDebugOperationFlagName[] = "operation";
static char prvMpdDebugOperationFlagHelp[] = "debug PHY operations";
UINT_32 prvMpdDebugOperationFlagId;

static PRV_MPD_FUNC_INFO_ENTRY_STC **prvMpdFuncDb_ARR = NULL;

static PRV_MPD_VCT_EXT_GAIN_INFO_STC prvMpdGainARR[MPD_TYPE_NUM_OF_TYPES_E + 1] = {
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_DIRECT_ATTACHED_FIBER_E	*/          
    {PRV_MPD_GAIN_COF_BITS1_CNS, 130, {174.160, 284.975, 180.705, 214.215, 181.475, 199.525, 0, 0}},      /* MPD_TYPE_88E1543_E				    */  
    {PRV_MPD_GAIN_COF_BITS1_CNS, 130, {174.160, 284.975, 180.705, 214.215, 181.475, 199.525, 0, 0}},      /* MPD_TYPE_88E1545_E				    */  
    {PRV_MPD_GAIN_COF_BITS1_CNS, 130, {174.160, 284.975, 180.705, 214.215, 181.475, 199.525, 0, 0}},      /* MPD_TYPE_88E1548_E		            */  
    /* 1680-A2 values (different values for 1680-A1) */                                                     
    {PRV_MPD_GAIN_COF_BITS2_CNS, 130, {180.990, 306.805, 191.100, 234.960, 186.495, 213.225, 0, 0}},      /* MPD_TYPE_88E1680_E 	            */  
    {PRV_MPD_GAIN_COF_BITS2_CNS, 130, {180.990, 306.805, 191.100, 234.960, 186.495, 213.225, 0, 0}},      /* MPD_TYPE_88E1680L_E 	            */  
    {PRV_MPD_GAIN_COF_BITS1_CNS, 130, {174.160, 284.975, 180.705, 214.215, 181.475, 199.525, 0, 0}},      /* MPD_TYPE_88E151x_E 	            */  
    {PRV_MPD_GAIN_COF_BITS2_CNS, 130, {180.990, 306.805, 191.100, 234.960, 186.495, 213.225, 0, 0}},      /* MPD_TYPE_88E3680_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88X32x0_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88X33x0_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88X20x0_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88X2180_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88E2540_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88X3540_E 	            */  
    {PRV_MPD_GAIN_COF_BITS1_CNS, 130, {183.99, 310.97, 187.66, 230.32, 191.34, 223.68, 205.50, 240.00}},  /* MPD_TYPE_88E1780_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88E2580_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},                                                                     /* MPD_TYPE_88X7120_E 	            */  
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}}, 
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}}, 
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}}, 
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}},
    {0, 0, {0, 0, 0, 0 ,0, 0, 0, 0}}
};

PRV_MPD_GLOBAL_DB_STC *prvMpdGlobalDb_PTR = NULL;

static PRV_MPD_CONVERT_IF_SPEED_TO_PHY_TYPE_STC prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[] = {
																								{ 	MPD_SPEED_CAPABILITY_10M_CNS,
																									MPD_TYPE_BITMAP_10BASETX_RESERVED_CNS },
																								{ 	MPD_SPEED_CAPABILITY_100M_CNS,
																									MPD_TYPE_BITMAP_100BASETX_CNS },
																								{ 	MPD_SPEED_CAPABILITY_1G_CNS,
																									MPD_TYPE_BITMAP_1000BASET_CNS },
																								{ 	MPD_SPEED_CAPABILITY_10G_CNS,
																									MPD_TYPE_BITMAP_10GBASET_CNS },
																								{ 	MPD_SPEED_CAPABILITY_2500M_CNS,
																									MPD_TYPE_BITMAP_2500BASET_CNS },
																								{ 	MPD_SPEED_CAPABILITY_5G_CNS,
																									MPD_TYPE_BITMAP_5GBASET_CNS }
};

#define max(a,b)  (((a) > (b)) ? (a) : (b))

MPD_CALLBACKS_STC prvMpdCallBacks;

static UINT_32 prvMpdGreenWaSleepMs = 100;

static PRV_MPD_SPECIFIC_FEATURE_STC prvMpdSpecificFeature_ARR[PRV_MPD_SPECIFIC_FEATURE_LAST_E];

static UINT_32 prvMpdEeeMasterEnableSsleep = PRV_MPD_EEE_MASTER_ENABLE_SLEEP_CNS;

/* portEntry_PTR->rel_ifIndex pset to accumulate all 1680 ports.
 ** Used for configuration of QSGMII WA required for 1680 */
PRV_MPD_PORT_LIST_TYP prvMpd88E1680PortList = { { 0 } };
/**
 * @internal mpdDriverInitDb function
 * @endinternal
 *
 * @brief   bind function
 * 			connect betweem PHY type & logical operation to PHY (type specific) implementation
 *
 * @param [in ]   	phyType   		- The PHY type
 * @param [in ]   	opType			- The logical operation
 * @param [in ]   	func_PTR		- The implementation of <opType> for <phyType>
 * @param [in ]		func_text_PTR	- Function name, for debug (may be NULL)
 *
 * @return MPD_RESULT_ENT
 */
extern BOOLEAN prvMpdBind(
	IN MPD_TYPE_ENT phyType,
	IN MPD_OP_CODE_ENT opType,
	IN MPD_OPERATIONS_FUN *func_PTR,
	IN char *func_text_PTR
)
{
	if (prvMpdFuncDb_ARR == NULL) {
		prvMpdFuncDb_ARR = (PRV_MPD_FUNC_INFO_ENTRY_STC**) PRV_MPD_CALLOC_MAC(	MPD_TYPE_NUM_OF_TYPES_E,
																				sizeof(PRV_MPD_FUNC_INFO_ENTRY_STC*));
	}
	if (prvMpdFuncDb_ARR == NULL) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_FATAL_E,
									"failed to allocate prvMpdFuncDb_ARR");
		return FALSE;
	}
	if (prvMpdFuncDb_ARR[phyType] == NULL) {
		prvMpdFuncDb_ARR[phyType] = (PRV_MPD_FUNC_INFO_ENTRY_STC*) PRV_MPD_CALLOC_MAC(	PRV_MPD_NUM_OF_OPS_E,
																						sizeof(PRV_MPD_FUNC_INFO_ENTRY_STC));
	}
	if (prvMpdFuncDb_ARR[phyType] == NULL) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_FATAL_E,
									"failed to allocate prvMpdFuncDb_ARR[phyType]");
		return FALSE;
	}
	if ((UINT_32)opType >= PRV_MPD_NUM_OF_OPS_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_ERROR_E,
									"mpdBind - illegal op type");
		return FALSE;
	}
	prvMpdFuncDb_ARR[phyType][opType].phyOperation_PTR = func_PTR;
	prvMpdFuncDb_ARR[phyType][opType].funcName_PTR = func_text_PTR;
	return TRUE;
}
/*$ END OF prvMpdBind */


/*****************************************************************************
* FUNCTION NAME: prvMpdVctSetTxPulseTwistedPair
*
* DESCRIPTION: Sets the pair to launch the pulse.
*
*****************************************************************************/
static MPD_RESULT_ENT prvMpdVctSetTxPulseTwistedPair(
	/*!     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16    twistedPairNum
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:             */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_16   val;       /* temporary value */
    MPD_RESULT_ENT status;       /* return code*/
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    /* HW values start from 4 up to 7 */
    twistedPairNum += 4;

    /* read for write */
    status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                23,
                                &val); 
        
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* channel is represented by bits 11-13 in reg23 */
    val &= ~(7 << 11);
    val |= (twistedPairNum << 11);

    /* update twistedPairNum */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                23,
                                val); 
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return status;
}
/*$ END OF  prvMpdVctSetTxPulseTwistedPair */


/*****************************************************************************
* FUNCTION NAME: prvMpdVctReady
*
* DESCRIPTION:  This function asserts the first state of virtual cable test.
* 
*****************************************************************************/
static MPD_RESULT_ENT prvMpdVctReady(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUPUTS:             */
    MPD_VCT_CABLE_STATUS_STC *cableStatus_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_16         val;         /* temporary data */
    MPD_RESULT_ENT  status;      /* return code    */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    cableStatus_PTR->normalCableLen = MPD_VCT_UNKNOWN_LEN_E;

    /* select page 5*/
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                0x5);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    
    PRV_MPD_SLEEP_MAC(1);
    
    /* setting launch pair */
    if ((status = prvMpdVctSetTxPulseTwistedPair(portEntry_PTR, 0)) != MPD_OK_E)
        return status;
    
    /* enable test */
    status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                23,
                                &val); 
    
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    val |= 0x8000;
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                23,
                                val);
    
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* save current launching pair */
    portEntry_PTR->runningData_PTR->vctInfo.currentTxPair = 0;

    return status;
}

/*$ END OF  prvMpdVctReady */

/*****************************************************************************
* FUNCTION NAME: prvMpdVctGetErrCableLen
*
* DESCRIPTION:  This function convert hw value of GE PHY cabel fault distance to length in meters.
*
*****************************************************************************/
static void prvMpdVctGetErrCableLen(
	/*!     INPUTS:             */
    UINT_32 dist,
    UINT_16 offset, 
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:             */
    UINT_8 *lengthPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_16 mm; /* actual distance in meters, may be negative value */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    mm = (8049 * dist + offset)/10000;
    *lengthPtr = (GT_U8)mm;
}
/*$ END OF  prvMpdVctGetErrCableLen */


/*****************************************************************************
* FUNCTION NAME: prvMpdVctStarted
*
* DESCRIPTION: This function asserts the second state of virtual cable test. 
*      
*****************************************************************************/
static MPD_RESULT_ENT prvMpdVctStarted(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC* portEntry_PTR,
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:             */
    MPD_VCT_CABLE_STATUS_STC *cableStatus_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
	UINT_16                     data;              /* temporary data */
    UINT_32                     distToFault;       /* distance to fault */
    UINT_32                     amplitude;         /* accepted amplitude value */
    UINT_32                     amplitudeThreshold;/* amplitude threshold to select fault type */
    UINT_32                     polarity;          /* accepted polarity value */
    UINT_32                     txPair,rxPair;     /* loop iterators */
    UINT_32                     rxDbIndex;         /* index in the Rx results DB */
    UINT_8                      errCableLen;
    MPD_RESULT_ENT              status;            /* return code */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    cableStatus_PTR->normalCableLen = MPD_VCT_UNKNOWN_LEN_E;
    
    /* select page 5 */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                5);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* check whether TDR test is finished */
    status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                23,
                                &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    
    /* the value 1 of bit 14 of register 23 page 5 indicates that VCT completed
       and TDR results in the registers are valid */
    if(!(data & 0x4000))
    {
        status = MPD_OP_FAILED_E;
        return status;
    }
    
    /* store accepted results */
    for (rxPair = 0; rxPair < MPD_VCT_MDI_PAIR_NUM_CNS; rxPair++)
    {
        rxDbIndex = rxPair + (MPD_VCT_MDI_PAIR_NUM_CNS * portEntry_PTR->runningData_PTR->vctInfo.currentTxPair);
        status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    (16 + rxPair),
                                    &portEntry_PTR->runningData_PTR->vctInfo.rxPairsRes[rxDbIndex]);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    }
    
    /* start analysing test results */
    if(portEntry_PTR->runningData_PTR->vctInfo.currentTxPair == (MPD_VCT_MDI_PAIR_NUM_CNS - 1))
    {
        /* outer loop running on all 4 pairs */
        for (txPair = 0; txPair < MPD_VCT_MDI_PAIR_NUM_CNS; txPair++)
        {
            for (rxPair = 0; rxPair < MPD_VCT_MDI_PAIR_NUM_CNS; rxPair++)
            {
                rxDbIndex = rxPair + (MPD_VCT_MDI_PAIR_NUM_CNS * txPair);

                data = portEntry_PTR->runningData_PTR->vctInfo.rxPairsRes[rxDbIndex];

                /* TDR test couldn't be completed */
                if(data == 0xFFFF){
                    PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                        "rel_ifIndex %d. Test couldn't be completed.",
                                                                        portEntry_PTR->rel_ifIndex);
                    return MPD_OP_FAILED_E;    
                }

                /* Good Termination */
                if((data & 0x00FF)  == 0x0)
                {
                    /* store results fro own Rx pair and
                      continue check next Rx one */
                    if(rxPair == txPair)
                    {
                        cableStatus_PTR->cableStatus[txPair].testStatus =
                                                       MPD_VCT_NORMAL_CABLE_E;
                        cableStatus_PTR->cableStatus[txPair].errCableLen = 0;
                    }
                }
                /* Polarity/Amplitude changes on the same pair */
                else
                {
                    /* calculate distance to fault */
                    distToFault = data & 0xff;
                    errCableLen = 0;
                    prvMpdVctGetErrCableLen(distToFault, portEntry_PTR->runningData_PTR->vct_offset, &errCableLen);
                    cableStatus_PTR->cableStatus[txPair].errCableLen = errCableLen;    
                    if (cableStatus_PTR->cableStatus[txPair].errCableLen >
                        MPD_VCT_MIN_110M_CABLE_LEN_DUE_TO_DEVIATION_CNS)
                    {
                        amplitudeThreshold = MPD_VCT_THRESHOLD_AMPLITUDE_VALUE_FOR_110M_CABLE_CNS;
                    }
                    else
                    {
                        amplitudeThreshold = MPD_VCT_THRESHOLD_AMPLITUDE_VALUE_CNS;
                    }

                    /* not same pair and cross pair short */
                    if(rxPair != txPair)
                    {
                        cableStatus_PTR->cableStatus[txPair].testStatus = MPD_VCT_SHORT_WITH_PAIR0_E + rxPair;
                    }
                    /* not same pair */
                    else
                    {
                        amplitude = (data >> 8) & 0x7F;
                        /* extract polarity - bit 15 */
                        polarity = ((data >> 15) & 0x1);
                        if(polarity)
                        {
                            if(amplitude > amplitudeThreshold)
                            {
                                 cableStatus_PTR->cableStatus[txPair].testStatus = MPD_VCT_OPEN_CABLE_E;
                            }
                            else
                            {
                                 cableStatus_PTR->cableStatus[txPair].testStatus = MPD_VCT_IMPEDANCE_MISMATCH_E;
                            }
                        }
                        else
                        {
                            if(amplitude > amplitudeThreshold)
                            { 
                                cableStatus_PTR->cableStatus[txPair].testStatus = MPD_VCT_SHORT_CABLE_E;
                            }
                            else
                            {
                                cableStatus_PTR->cableStatus[txPair].testStatus = MPD_VCT_IMPEDANCE_MISMATCH_E;
                            }
                        }/* polarity else */

                    }/* not same pair */

                    /* first failure defines final one */
                    break;

                }/* else */

            } /* for each Rx pair loop */

        }/* active pair Tx loop*/
    }
    else /* test not finished */
    {
        /* setting next launch pair */
        status = prvMpdVctSetTxPulseTwistedPair(portEntry_PTR, ++portEntry_PTR->runningData_PTR->vctInfo.currentTxPair);
        if (status != MPD_OK_E){
            return status;
        }        
       
        /* enable test */
        status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    23,
                                    &data);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        
        data |= 0x8000;
        status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    23,
                                    data);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    /* return to page 0 */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                0);
    
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    PRV_MPD_SLEEP_MAC(1);

    return MPD_OK_E;
}
/*$ END OF  prvMpdVctStarted */

/*****************************************************************************
* FUNCTION NAME: prvMpdVctCableStatusGetStart
*
* DESCRIPTION: This function start the vct test.
*                
*****************************************************************************/
extern MPD_RESULT_ENT prvMpdVctCableStatusGetStart(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC* portEntry_PTR,
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:             */
    MPD_VCT_CABLE_STATUS_STC  *cableStatus_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_16         val = 0;
    MPD_RESULT_ENT  status;            /* return code */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/ 
    if (cableStatus_PTR == NULL) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d. CableStatus_PTR - Null pointer",
                                                            portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;;
   }
   
   /* reset PHY */
   status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                0);
   PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

   status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_CTRLREG_OFFSET_CNS,
                                &val); 
   PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

   val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* software reset bit */
   
   status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_CTRLREG_OFFSET_CNS,
                                val);
   PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
   
   status = prvMpdVctReady(portEntry_PTR, cableStatus_PTR);
   if (status != MPD_OK_E){
       return status;
   }

   return MPD_OK_E;
}

/*$ END OF  prvMpdVctCableStatusGetStart */

/*****************************************************************************
* FUNCTION NAME: prvMpdVctCableStatusGetRes
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

extern MPD_RESULT_ENT  prvMpdVctCableStatusGetRes(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC* portEntry_PTR,
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUUPUTS:             */
    MPD_VCT_CABLE_STATUS_STC  *cableStatus_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    MPD_RESULT_ENT  status;                   /* return code */
    UINT_16         val = 0;     /* PHY ID register value */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (cableStatus_PTR == NULL) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d. CableStatus_PTR - Null pointer",
                                                            portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;;
    }
    
    /* select page 0 (cooper) */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    
    status = prvMpdVctStarted(portEntry_PTR, cableStatus_PTR);
    
    if (status == MPD_OK_E) 
    {
       /* Test has completed. Switch to READY state and reset DB.*/
       /* Enable soft reset to the port */
       
       status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                    0);
       PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

       status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_CTRLREG_OFFSET_CNS,
                                    &val);
       PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
       
       val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* software reset bit */

       status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_CTRLREG_OFFSET_CNS,
                                    val);
       PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    }
    
    return status;
}

/*$ END OF  prvMpdVctCableStatusGetRes */



/*!**************************************************RND Template version 4.1
*!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
*!==========================================================================
*!
*$ FUNCTION: prvMpdVctgetLengthSum
*!
*$ DESCRIPTION: This function get the read the cable length for each pair.
*!
*!***************************************************************************/
MPD_RESULT_ENT prvMpdVctgetLengthSum(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_32     index,
    /*!     OUTPUTS:            */
    UINT_16     *maxValue_PTR,
    UINT_32     *lengthSum_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    MPD_RESULT_ENT      status;
    UINT_16             minValue;
    UINT_16             value;                  /* temporary data */
    UINT_32             i,k;
    UINT_16             startVctValue;          /* register's value to start retrieving of cable length */
    UINT_8              startVctRegAddr;        /* register's address to start retrieving of cable length */
    UINT_8              lengthRegAddr;          /* register's address to get cable length */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    /* reset min/max values for each pair */
    minValue  = 0xFFFF;
    startVctRegAddr = 16;
    lengthRegAddr   = 18;
    startVctValue   = 0x1118;

    /* Read each pair's results at least 10 times.
    As workaround to avoid deviation, highest and lowest readingns should
    be thrown */
    for(k = 0; k < 10; k++){
        status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                    0x00FF);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        
        status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    startVctRegAddr,
                                    (UINT_16)(startVctValue + index));
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        
        /* keep polling reg16.15 until bit is set to 1 */
        for (i = 0; i < 1000; i++){
            status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        startVctRegAddr,
                                        &value);
            PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        #ifdef PHY_SIMULATION
            value |= 0x8000;
        #endif
            if ((value & 0x8000))
                break;
           PRV_MPD_SLEEP_MAC(1);
        }
        /* read length of relevant pair */
        status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    lengthRegAddr,
                                    &value);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        *lengthSum_PTR += value;

        /* save min and max to throw it and avoid deviation */
        if(value < minValue){
            minValue = value;
        }

        if(value > *maxValue_PTR){
            *maxValue_PTR = value;
        }
    }/* loop per each pair */

    
    /* remove deviations and get average */
    *lengthSum_PTR -= (*maxValue_PTR + minValue);
    *lengthSum_PTR /= 8;

    /* update statuses */
    
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0);

    return MPD_OK_E;
}
/*$ END OF prvMpdVctgetLengthSum */


/*!**************************************************RND Template version 4.1
*!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
*!==========================================================================
*!
*$ FUNCTION: prvMpdVctgetExtendedCable
*
*$ DESCRIPTION: This function calculate the cable length for each pair, using the coefficient and 
*!               the length obtained from prvMpdVctgetLengthSum.
*!
*!==========================================================================*/
MPD_RESULT_ENT prvMpdVctGetExtendedCable (
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC         *portEntry_PTR,
    /*!     INPUTS / OUTPUTS:   */
    MPD_VCT_CABLE_EXTENDED_STATUS_STC   *extendedCableStatus_PTR
    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    MPD_RESULT_ENT   status;                 /* return code */
    UINT_16          value;                  /* temporary data */
    UINT_16          gain;
    UINT_32          i;
    UINT_16          maxValue;               /* maximum value read from reg21 */
    UINT_32          lengthSum;              /* sum of all lengths of same pair */
    double           cableLength;
    MPD_TYPE_ENT     phyType;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    phyType = portEntry_PTR->initData_PTR->phyType;
    /* Procedure to get cable length utilising look up table */
    for(i = 0; i < MPD_VCT_MDI_PAIR_NUM_CNS; i++){
        /* reset min/max values for each pair */
        maxValue  = 0;
        lengthSum = 0;

        status = prvMpdVctgetLengthSum(portEntry_PTR, i, &maxValue, &lengthSum);
        if (status == MPD_OP_FAILED_E){
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex %d. prvMpdVctgetLengthSum failed.",
                portEntry_PTR->rel_ifIndex);
            return MPD_OP_FAILED_E;
        }
        extendedCableStatus_PTR->accurateCableLen.isValid[i] = TRUE;
  
        /* convert HW value */
        status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                    0x00FC);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

        /* Read the divider/multiplier coefficient for the DSP-based VCT */
        status = prvMpdMdioReadRegisterNoPage( PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                   portEntry_PTR,
                                   23,
                                   &value);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        gain = value & prvMpdGainARR[phyType].gainCoefficient;
       
        if ((gain < 3) || (prvMpdGainARR[phyType].gainVars[PRV_MPD_MAX_GAINS_NUMBER_CNS*2-1] != 0)){
            cableLength = prvMpdGainARR[phyType].gainVars[gain*2] * log10(lengthSum) - prvMpdGainARR[phyType].gainVars[gain*2+1];
        }
        else {
            cableLength = prvMpdGainARR[phyType].maxVal;
        }
        extendedCableStatus_PTR->accurateCableLen.cableLen[i] = (UINT_16)cableLength;
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
            "rel_ifIndex %d. value = 0x%x , gain = %d, lengthSum = %d, cableLength = %lf, cableLen[i] = %d \n ",
            portEntry_PTR->rel_ifIndex,
            value,
            gain,
            lengthSum,
            cableLength,
            extendedCableStatus_PTR->accurateCableLen.cableLen[i]);
    }/* loop on pairs */
    /* check if valid bit on */

    return MPD_OK_E;
}
/*$ END OF prvMpdVctgetExtendedCable */


/*!**************************************************RND Template version 4.1
*!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
*!==========================================================================
* FUNCTION: prvMpdVctGetExtendedCableStatus
*
* DESCRIPTION: This function perform the extended virtual cable test.
*
*!==========================================================================*/
MPD_RESULT_ENT prvMpdVctGetExtendedCableStatus(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC*             portEntry_PTR,
	/*!     INPUTS / OUTPUTS:   */
    MPD_VCT_CABLE_EXTENDED_STATUS_STC       *extendedCableStatus_PTR
	/*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/  
    MPD_RESULT_ENT               status;                 /* return code */
    UINT_16                      value;                  /* temporary data */
    UINT_32                      i;                      /* loop iterators */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (extendedCableStatus_PTR == NULL) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d. extendedCableStatus_PTR - Null pointer",
                                                            portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
    
    /* select page 0 (cooper) */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    PRV_MPD_SLEEP_MAC(1);
    
    /* read reg.17 value */
    status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                17,
                                &value);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    
    /* check if link is on */
    if( !(value & 0x0400))
        return MPD_OK_E;
    
    status = prvMpdVctGetExtendedCable(portEntry_PTR, extendedCableStatus_PTR);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
   
    /* check if valid bit on */
    
    /* select page 5 */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                5);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    PRV_MPD_SLEEP_MAC(1);
    
    status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                17,
                                &value);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* if bit not valid don't do anything */
    if( !(value & 0x0040))
    {
        /* return page register to page 0 */
        status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                    0);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        PRV_MPD_SLEEP_MAC(1);
        return status;
    }

    /* display extended statuses */
    extendedCableStatus_PTR->vctExtendedCableStatus.isValid = TRUE;

    /* read data from 21_page5 */
    /* display pair polarity, bits 0,1,2,3 */
    for(i = 0; i < MPD_VCT_MDI_PAIR_NUM_CNS; i++)
    {
        if( !((value >> i) & 0x01) )
            extendedCableStatus_PTR->vctExtendedCableStatus.pairPolarity[i] =
                                                           MPD_VCT_POSITIVE_POLARITY_E;
        else
            extendedCableStatus_PTR->vctExtendedCableStatus.pairPolarity[i] =
                                                           MPD_VCT_NEGATIVE_POLARITY_E;
    }

    /* display pair swap, bits 4,5 */
    for(i = 0; i < MPD_VCT_CHANNEL_PAIR_NUM_CNS; i++)
    {
        if( !((value >> (i+4)) & 0x01) )
            extendedCableStatus_PTR->vctExtendedCableStatus.pairSwap[i] =
                                                            MPD_VCT_CABLE_SWAP_STRAIGHT_E;
        else
            extendedCableStatus_PTR->vctExtendedCableStatus.pairSwap[i] =
                                                            MPD_VCT_CABLE_SWAP_CROSSOVER_E;
    }

    status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                20,
                                &value);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* display pair skew */
    extendedCableStatus_PTR->vctExtendedCableStatus.pairSkew.isValid = TRUE;

    for( i = 0; i < MPD_VCT_MDI_PAIR_NUM_CNS; i++)
        extendedCableStatus_PTR->vctExtendedCableStatus.pairSkew.skew[i] =
                                                (((value >> (i*4)) & 0x0F) * 8);

    /* return to page 0 */
    status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}
/*$ END OF prvMpdVctGetExtendedCableStatus */


/*!**************************************************RND Template version 4.1
*!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
*!==========================================================================
* FUNCTION NAME: prvMpdVctAnalyzeVctExtendedCable
*
* DESCRIPTION: This function returns the extended VCT status per MDI pair/channel pair.
*
*
============================================================================*/
MPD_RESULT_ENT prvMpdVctAnalyzeVctExtendedCable(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC*                 portEntry_PTR,
	/*!     INPUTS / OUTPUTS:   */
    MPD_VCT_CABLE_EXTENDED_STATUS_STC  *extendedCableStatus_PTR
	/*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32         i;              /* iterator*/
    MPD_RESULT_ENT  status;         /* return code */
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (extendedCableStatus_PTR == NULL) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d. extendedCableStatus_PTR - Null pointer",
                                                            portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    /* set default values */
    extendedCableStatus_PTR->vctExtendedCableStatus.isValid = FALSE;
    extendedCableStatus_PTR->vctExtendedCableStatus.pairSwap[0] = MPD_VCT_NOT_APPLICABLE_SWAP_E;
    extendedCableStatus_PTR->vctExtendedCableStatus.pairSwap[1] = MPD_VCT_NOT_APPLICABLE_SWAP_E;
    extendedCableStatus_PTR->vctExtendedCableStatus.pairSkew.isValid = FALSE;
    for (i=0; i < MPD_VCT_MDI_PAIR_NUM_CNS; i++){
        extendedCableStatus_PTR->vctExtendedCableStatus.pairPolarity[i] = MPD_VCT_NOT_APPLICABLE_POLARITY_E;
        extendedCableStatus_PTR->vctExtendedCableStatus.pairSkew.skew[i] = 0;
        extendedCableStatus_PTR->accurateCableLen.cableLen[i] = 0;
        extendedCableStatus_PTR->accurateCableLen.isValid[i] = FALSE;
    }

    status = prvMpdVctGetExtendedCableStatus(portEntry_PTR, extendedCableStatus_PTR);

    return status;
}

/*$ END OF prvMpdVctAnalyzeVctExtendedCable */

/*****************************************************************************
 * FUNCTION NAME: prvMpdAnalyzeVct
 *
 * DESCRIPTION: This function analyze the vct test results.
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdAnalyzeVct(
	/*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	BOOLEAN isTwoPairCable, /* for fast Ethernet ports */
	/*!     INPUTS / OUTPUTS:   */
    MPD_VCT_PARAMS_STC *params_PTR
    /*!     OUTPUTS:            */
)
{
	MPD_RESULT_ENT vct_stat;
	UINT_32 mdi_pair, mdi_pairMax, i, attempts = 0;
	BOOLEAN is_possible_two_pair_cable, is_active_two_pair_cable = FALSE;
	MPD_VCT_CABLE_STATUS_STC cableStatus;
	UINT_32 len;
	BOOLEAN is_short_cable = FALSE;
	MPD_VCT_TEST_STATUS_ENT pair_short[] =
			{
				MPD_VCT_SHORT_WITH_PAIR0_E,
				MPD_VCT_SHORT_WITH_PAIR1_E,
				MPD_VCT_SHORT_WITH_PAIR2_E,
				MPD_VCT_SHORT_WITH_PAIR3_E
			};

	/* in hardware spec the test is called TDR - time domain reflector */
	/* this funciton is performed solely on the PHY (not on the PP). */
	/* vct can be performed on copper ports only */
	memset(	&cableStatus,
			0,
			sizeof(MPD_VCT_CABLE_STATUS_STC));

	vct_stat = prvMpdVctCableStatusGetStart(portEntry_PTR, &cableStatus);
	if (vct_stat == MPD_OK_E) {
		vct_stat = prvMpdVctCableStatusGetRes(portEntry_PTR, &cableStatus);
        
        /* Check up to 100 times if vct_stat is not ready yet*/
		while (vct_stat != MPD_OK_E && attempts < 100) {
			attempts++;
			PRV_MPD_SLEEP_MAC((100));
			vct_stat = prvMpdVctCableStatusGetRes(portEntry_PTR, &cableStatus);
		}
		if (vct_stat != MPD_OK_E)
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "portEntry_PTR->rel_ifIndex %d. prvMpdVctCableStatusGetRes failed",
                                                                portEntry_PTR->rel_ifIndex);                                                    
			return MPD_OP_FAILED_E;
	}
	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d. vct_stat %d",
														portEntry_PTR->rel_ifIndex,
														vct_stat);

	if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(prvMpdDebugConfigVctFlagId)) {
		for (mdi_pair = 0; mdi_pair < 4; mdi_pair++) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"pair %d testStatus %d errCableLen %d normalCableLen %d",
																mdi_pair,
																cableStatus.cableStatus[mdi_pair].testStatus,
																cableStatus.cableStatus[mdi_pair].errCableLen,
																cableStatus.normalCableLen);
		}
	}

	if (vct_stat != MPD_OK_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"vct failed on portEntry_PTR->rel_ifIndex %d phy status is %d",
															portEntry_PTR->rel_ifIndex,
															vct_stat);

		return MPD_OP_FAILED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d vctGetCableDiag passed",
														portEntry_PTR->rel_ifIndex);

	/* ANALIZE THE VCT RESULTS */

	/* detect possible 2 pair cable */
	if (isTwoPairCable == FALSE) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex NOT isTwoPairCable",
															portEntry_PTR->rel_ifIndex);

		if (((cableStatus.cableStatus[PRV_MPD_VCT_2_PAIR_CABLE_CNS].testStatus == MPD_VCT_OPEN_CABLE_E) &&
				(cableStatus.cableStatus[PRV_MPD_VCT_2_PAIR_CABLE_CNS].errCableLen == 0)) &&
				((cableStatus.cableStatus[PRV_MPD_VCT_2_PAIR_CABLE_CNS + 1].testStatus == MPD_VCT_OPEN_CABLE_E) &&
						(cableStatus.cableStatus[PRV_MPD_VCT_2_PAIR_CABLE_CNS + 1].errCableLen == 0))) {
			is_possible_two_pair_cable = TRUE;
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d is_possible_two_pair_cable 1",
																portEntry_PTR->rel_ifIndex);
		}
		else if ((cableStatus.cableStatus[PRV_MPD_VCT_2_PAIR_CABLE_CNS].testStatus == MPD_VCT_SHORT_CABLE_E) &&
				(cableStatus.cableStatus[PRV_MPD_VCT_2_PAIR_CABLE_CNS + 1].testStatus == MPD_VCT_SHORT_CABLE_E)) {
			is_possible_two_pair_cable = TRUE;
			is_active_two_pair_cable = TRUE;
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d is_possible_two_pair_cable 2, is_active_two_pair_cable",
																portEntry_PTR->rel_ifIndex);
		}
		else {
			is_possible_two_pair_cable = FALSE;
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d is_possible_two_pair_cable 3",
																portEntry_PTR->rel_ifIndex);
		}
	}
	else {
		is_possible_two_pair_cable = TRUE;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d isTwoPairCable 4",
															portEntry_PTR->rel_ifIndex);
	}

	/* set max pair to check 2,4 */
	mdi_pairMax =
			(is_possible_two_pair_cable) ? PRV_MPD_VCT_2_PAIR_CABLE_CNS : PRV_MPD_VCT_4_PAIR_CABLE_CNS;

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"mdi_pairMax %d",
														mdi_pairMax);

	/* MPD_VCT_RESULT_CABLE_OK_CNS */
	/* check normal cable on all 2 or 4 pairs */
	for (mdi_pair = 0; mdi_pair < mdi_pairMax; mdi_pair++)
		if (cableStatus.cableStatus[mdi_pair].testStatus != MPD_VCT_NORMAL_CABLE_E)
			break;
	/* all 4 pair are normal */
	if (mdi_pair == PRV_MPD_VCT_4_PAIR_CABLE_CNS) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"all pair are normal. normal cable");

		params_PTR->testResult = MPD_VCT_RESULT_CABLE_OK_E;
		params_PTR->cableLength = 0;

		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d normal cable",
															portEntry_PTR->rel_ifIndex);

		return MPD_OK_E;
	}
	/* only 2 first pairs are normal */
	else if (mdi_pair == mdi_pairMax) {
		/* fast port so only 2 pairs */
		if (isTwoPairCable || is_active_two_pair_cable) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"fast port portEntry_PTR->rel_ifIndex %d normal cable",
																portEntry_PTR->rel_ifIndex);
			params_PTR->testResult = MPD_VCT_RESULT_CABLE_OK_E;
		}
		/* giga port 2 pair 1,2 are normal */
		else if (is_possible_two_pair_cable) {
			params_PTR->testResult = MPD_VCT_RESULT_2_PAIR_CABLE_E;

			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"giga port portEntry_PTR->rel_ifIndex %d 2 pair cable",
																portEntry_PTR->rel_ifIndex);
		}
		params_PTR->cableLength = 0;

		return MPD_OK_E;
	}

	/* MPD_VCT_RESULT_NO_CABLE_CNS */
	/* 2 upper pairs are open with length 0 */
	if (is_possible_two_pair_cable) {
		/* check first 2 pairs - open with length 0 */
		for (mdi_pair = 0; mdi_pair < 2; mdi_pair++)
			if (cableStatus.cableStatus[mdi_pair].testStatus != MPD_VCT_OPEN_CABLE_E ||
					cableStatus.cableStatus[mdi_pair].errCableLen != 0)
				break;
		if (mdi_pair == 2) {
			params_PTR->testResult = MPD_VCT_RESULT_NO_CABLE_E;
			params_PTR->cableLength = 0;
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d no cable",
																portEntry_PTR->rel_ifIndex);

			return MPD_OK_E;
		}
	}

	/* MPD_VCT_RESULT_BAD_CABLE_CNS */
	for (mdi_pair = 0; mdi_pair < mdi_pairMax; mdi_pair++)
		if (cableStatus.cableStatus[mdi_pair].testStatus == MPD_VCT_TEST_FAIL_E)
			break;
	if (mdi_pair < mdi_pairMax) {
		params_PTR->testResult = MPD_VCT_RESULT_BAD_CABLE_E;
		params_PTR->cableLength = 0;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d bad cable",
															portEntry_PTR->rel_ifIndex);

		return MPD_OK_E;
	}

	/* MPD_VCT_RESULT_SHORT_CABLE_CNS */
	for (len = 0xFFFF, mdi_pair = 0; mdi_pair < mdi_pairMax; mdi_pair++) {
		/* short or specific pair is short */
		is_short_cable = (cableStatus.cableStatus[mdi_pair].testStatus == MPD_VCT_SHORT_CABLE_E);
		if (!is_short_cable) {
			/*bug133965 fixing*/
			/*Check if polarity/amplitude changes on the not same pair (cross pair short circuit)*/
			for (i = 0; i < 4; i++) {
				if (pair_short[i] == cableStatus.cableStatus[mdi_pair].testStatus) {
					is_short_cable = TRUE;
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																		"portEntry_PTR->rel_ifIndex %d Check if polarity/amplitude changes on the not same pair (cross pair short circuit)",
																		portEntry_PTR->rel_ifIndex);
					break;
				}
			}
		}
		if (is_short_cable) {
			len = MIN(	len,
						cableStatus.cableStatus[mdi_pair].errCableLen);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d is_short_cable. MIN from: len %d, errCableLen %d",
																portEntry_PTR->rel_ifIndex,
																len,
																cableStatus.cableStatus[mdi_pair].errCableLen);
		}
	}

	if (len < 0xFFFF) {
		params_PTR->testResult = MPD_VCT_RESULT_SHORT_CABLE_E;
		params_PTR->cableLength = len;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d is_short_cable. len %d",
															portEntry_PTR->rel_ifIndex,
															len);

		return MPD_OK_E;
	}

	/* MPD_VCT_RESULT_OPEN_CABLE_CNS */
	for (len = 0xFFFF, mdi_pair = 0; mdi_pair < mdi_pairMax; mdi_pair++) {
		if (cableStatus.cableStatus[mdi_pair].testStatus == MPD_VCT_OPEN_CABLE_E) {
			len = MIN(	len,
						cableStatus.cableStatus[mdi_pair].errCableLen);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d is_OPEN_CABLE. len %d, errCableLen %d",
																portEntry_PTR->rel_ifIndex,
																len,
																cableStatus.cableStatus[mdi_pair].errCableLen);
		}
	}

	if (len < 0xFFFF) {
		params_PTR->testResult = MPD_VCT_RESULT_OPEN_CABLE_E;
		params_PTR->cableLength = len;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d open cable len is %d",
															portEntry_PTR->rel_ifIndex,
															len);

		return MPD_OK_E;
	}

	/* MPD_VCT_RESULT_IMPEDANCE_MISMATCH_CNS */
	for (len = 0xFFFF, mdi_pair = 0; mdi_pair < mdi_pairMax; mdi_pair++) {
		if (cableStatus.cableStatus[mdi_pair].testStatus == MPD_VCT_IMPEDANCE_MISMATCH_E) {
			len = MIN(	len,
						cableStatus.cableStatus[mdi_pair].errCableLen);
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d GT_IMPEDANCE_MISMATCH MIN from: len %d, errCableLenn %d",
																portEntry_PTR->rel_ifIndex,
																len,
																cableStatus.cableStatus[mdi_pair].errCableLen);
		}
	}
	if (len < 0xFFFF) {
		params_PTR->testResult = MPD_VCT_RESULT_IMPEDANCE_MISMATCH_E;
		params_PTR->cableLength = len;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d impedance mismatch cable len is %d",
															portEntry_PTR->rel_ifIndex,
															len);

		return MPD_OK_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d imop_not_supported",
														portEntry_PTR->rel_ifIndex);

	return MPD_NOT_SUPPORTED_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdCheckIfPhyIsInReqAutoneg
 *
 * DESCRIPTION: Check if the port is configured to the requested auto-negotiation configuration.
 *
 *
 *****************************************************************************/
static BOOLEAN prvMpdCheckIfPhyIsInReqAutoneg(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_AUTO_NEGOTIATION_ENT req_autoneg,
	GT_U8 phyPage
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	GT_U16 val;
	MPD_RESULT_ENT status;
	BOOLEAN ret_val;

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									phyPage,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&val);
	if (status != MPD_OK_E) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "portEntry_PTR->rel_ifIndex %d prvMpdCheckIfPhyIsInReqAutoneg failed to read phyPage %d",
                                                        portEntry_PTR->rel_ifIndex,
                                                        phyPage);
		ret_val = FALSE;
	}
	else {
		switch (req_autoneg) {
			case MPD_AUTO_NEGOTIATION_ENABLE_E:
				if (val & PRV_MPD_CTRLREG_AUTONEG_MASK_CNS) {
					ret_val = TRUE;

					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"already in auto neg enable");

				}
				else {
					ret_val = FALSE;
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"not in auto neg enable");
				}
				break;

			case MPD_AUTO_NEGOTIATION_DISABLE_E:
				if (!(val & PRV_MPD_CTRLREG_AUTONEG_MASK_CNS)) {
					ret_val = TRUE;
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"already in auto neg disable");
				}
				else {
					ret_val = FALSE;
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"not in auto neg disable");
				}
				break;

			default:
				PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
											MPD_ERROR_SEVERITY_ERROR_E,
											"wrong req_autoneg");
				ret_val = FALSE;
				break;
		}
	}
	return (ret_val);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetAutoNegotiation_1
 *
 * DESCRIPTION: 88E3680
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetAutoNegotiation_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	BOOLEAN in_req_autoneg;
	MPD_RESULT_ENT status;
	GT_U16 val = 0;
	GT_U16 capabilities1 = 0;
	GT_U16 control_reg_val, pause_advertise_bit = 0;

	switch (params_PTR->phyAutoneg.enable) {
		case MPD_AUTO_NEGOTIATION_ENABLE_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set auto_neg enable, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_10_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_10_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_100_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_100_T_FULL_CNS;
			}

			in_req_autoneg = prvMpdCheckIfPhyIsInReqAutoneg(portEntry_PTR,
															MPD_AUTO_NEGOTIATION_ENABLE_E,
															PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS);
			if (in_req_autoneg) {
				/* Already in the requested auto neg mode. So there is no need to change it again */
				/* check if we had advertised all */
				status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_AUTNEGADVER_OFFSET_CNS,
												&val);
				PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

				if (((val & PRV_MPD_ADVERTISE_10_100_CNS) == capabilities1)) {
					return MPD_OK_E;
				}
			}

			if (val & 0x0400) {
				pause_advertise_bit = 0x0400;
			}
			/* First set advertise bits */
			val = capabilities1 | 0x0001 | pause_advertise_bit;
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_AUTNEGADVER_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
			/* Reset, reset auto neg and no power down*/
			val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* reset*/
			val |= PRV_MPD_CTRLREG_AUTO_NEG_CNS; /* enable and restart auto-neg */
			val &= ~PRV_MPD_CTRLREG_POWER_DOWN_CNS; /* no power down*/
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
			break;

		case MPD_AUTO_NEGOTIATION_DISABLE_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set auto_neg disable, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			/* Set the control register according to the specific status register. */
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&control_reg_val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
			/* Set auto_neg and restart_auto_neg bits */
			control_reg_val &= ~PRV_MPD_CTRLREG_AUTO_NEG_CNS;

			/* This PHY requires reset after setting a new value */
			control_reg_val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;

			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"write values to cntrl reg with reset");

			/* Write back control reg */
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												control_reg_val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
			break;

		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"invalid command");
			return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetDuplexMode_8
 *
 * DESCRIPTION: 88E3680
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetDuplexMode_8(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 val;
	MPD_RESULT_ENT status;

	/* Read the cntrl reg. */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	switch (params_PTR->phyDuplex.mode) {
		case MPD_DUPLEX_ADMIN_MODE_FULL_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set full duplex, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);
			/* check if the phy is already in full duplex mode */
			if (val & PRV_MPD_CTRLREG_FULL_DPLX_CNS) {
				PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"already in full duplex");
				return MPD_OK_E;
			}
			val |= PRV_MPD_CTRLREG_FULL_DPLX_CNS;
			break;
		case MPD_DUPLEX_ADMIN_MODE_HALF_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set half duplex, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);
			/* check if the phy is already in half duplex mode */
			if (!(val & PRV_MPD_CTRLREG_FULL_DPLX_CNS)) {
				PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"already in half duplex");
				return MPD_OK_E;
			}
			val &= ~PRV_MPD_CTRLREG_FULL_DPLX_CNS;
			break;
		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"Invalid duplex mode");
			return MPD_OP_FAILED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"force the new duplex mode with reset");

	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) {
		/* This PHY requires reset after setting a new value */
		val = val | PRV_MPD_CTRLREG_RESET_BIT_CNS;
	}
	/* Force the duplex mode */
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										(GT_U16) val);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetSpeed_1
 *
 * DESCRIPTION: 88E3680
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetSpeed_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	/* *********************   ******************************************************/
	MPD_RESULT_ENT status = MPD_OK_E;
	GT_U16 cntrl_val;
	/* Read the current value of the control register */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&cntrl_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* speed is set in 0.0.6, 0.0.13 */
	cntrl_val &= ~(1 << 6); /* Zero Bit 6 */
	cntrl_val &= ~(1 << 13); /* Zero Bit 13 */
	switch (params_PTR->phySpeed.speed) {
		case MPD_SPEED_10M_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set 10M, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);
			break;

		case MPD_SPEED_100M_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set 100M, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			cntrl_val |= PRV_MPD_CTRLREG_100BT_CNS;
			break;

		case MPD_SPEED_1000M_E:
			default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"Not supported speed");
			return MPD_NOT_SUPPORTED_E;
	}

	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) {
		/* This PHY requires reset after setting a new value */
		cntrl_val = cntrl_val | PRV_MPD_CTRLREG_RESET_BIT_CNS;
	}
	/* force the requested speed */
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										(GT_U16) cntrl_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetMdixAdminMode_1
 *
 * DESCRIPTION: 88E3680
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetMdixAdminMode_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	GT_U16 val, new_val;

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	new_val = val;
	/* MDI/X is configured in 0.16.5:6 */
	new_val &= ~(1 << 5); /* Zero Bit 5 */
	new_val &= ~(1 << 6); /* Zero Bit 6 */
	switch (params_PTR->phyMdix.mode) {
		case MPD_MDI_MODE_MEDIA_E:
			/* set mdi mode*/
			new_val |= PRV_MPD_MANUAL_MDI_CONFIGURATION_CNS;
			break;

		case MPD_MDIX_MODE_MEDIA_E:
			/* set mdix mode*/
			new_val |= PRV_MPD_MANUAL_MDIX_CONFIGURATION_CNS;
			break;

		case MPD_AUTO_MODE_MEDIA_E:
			/* set auto mdix mode*/
			new_val |= PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS;
			break;

		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										"Invalid MDI/X mode");
			return MPD_NOT_SUPPORTED_E;
	}
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (new_val != val) {
		/* Write MDI/X configuration */
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											new_val);

		if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) {
			/* reset the phy (commit the changes) */
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
			val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												0,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);
		}

		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdResetPhy_7
 *
 * DESCRIPTION: 88E3680
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdResetPhy_7(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 cntrl_val;
	MPD_RESULT_ENT status;
    MPD_UNUSED_PARAM(params_PTR);

	/* if port is admin down - don't reset PHY (it will cancel the admin down) */
	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
		return MPD_OK_E;
	}
	/* Read the current value of the control register */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&cntrl_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	cntrl_val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										cntrl_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDisableOperation_2
 *
 * DESCRIPTION: 	Adminstrative shutdown of PHY
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdDisableOperation_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 val;
	MPD_RESULT_ENT status;

	/* read control register */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (params_PTR->phyDisable.forceLinkDown == TRUE) {
		/* force port to link down */
		val |= PRV_MPD_LINK_DOWN_CNS;
	}
	else {
		/* un-force link down */
		val &= PRV_MPD_LINK_UP_CNS;
		/* reset PHY */
		val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
	}

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* disable power down */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									16,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* Zero 0.16.2 */
	val &= ~(1 << 2);

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
										16,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetPhyPageSelect
 *
 * DESCRIPTION: change PHY page to selected page, used for legacy phys
 *
 * APPLICABLE PHYS:
 *                  MPD_TYPE_88E1543_E
 *                  MPD_TYPE_88E1545_E
 *                  MPD_TYPE_88E1548_E
 *                  MPD_TYPE_88E151x_E
 *                  MPD_TYPE_88E1680_E
 *                  MPD_TYPE_88E1680L_E
 *                  MPD_TYPE_88E3680_E
 *                  MPD_TYPE_88E1780_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetPhyPageSelect(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT status;
    UINT_16 page = params_PTR->internal.phyPageSelect.page, prev_page;
    /* read previous page */
    if (params_PTR->internal.phyPageSelect.readPrevPage) {
        status = PRV_MPD_MDIO_READ_MAC( portEntry_PTR->rel_ifIndex,
                                    PRV_MPD_IGNORE_PAGE_CNS,
                                    PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                    &prev_page);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        params_PTR->internal.phyPageSelect.prevPage = prev_page;
    }
    /* set to new page */
    status = PRV_MPD_MDIO_WRITE_MAC (portEntry_PTR->rel_ifIndex,
                                 PRV_MPD_IGNORE_PAGE_CNS,
                                 PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                 page);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetPhyIdentifier
 *
 * DESCRIPTION: get phy type and identifier
 *
 * APPLICABLE PHYS:
 *                  MPD_TYPE_88E1543_E
 *                  MPD_TYPE_88E1545_E
 *                  MPD_TYPE_88E1548_E
 *                  MPD_TYPE_88E151x_E
 *                  MPD_TYPE_88E1680_E
 *                  MPD_TYPE_88E1680L_E
 *                  MPD_TYPE_88E3680_E
 *                  MPD_TYPE_88E1780_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetPhyIdentifier(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT status;
    UINT_16 regVal;

    /* read PHY identifier (page 0 register offset 3 - BITS 10:15- OUI, 4:9-Model Number, 0:3-Revision Number) */
    status = PRV_MPD_MDIO_READ_MAC( portEntry_PTR->rel_ifIndex,
                                PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
                                PRV_MPD_COPPER_GIG_PHY_IDENTIFIER_CNS,
                                &regVal);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    params_PTR->phyRevision.phyType = portEntry_PTR->initData_PTR->phyType;
    params_PTR->phyRevision.revision = regVal;

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdMydGetPhyIdentifier
 *
 * DESCRIPTION: get phy type and identifier
 *
 * APPLICABLE PHYS: all MPD & MYD phys
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdMydGetPhyIdentifier(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_16 regVal;
    MPD_RESULT_ENT mpdStatus;

    /* read PHY identifier (device 1 register 3 - BITS 10:15- OUI, 4:9-Model Number, 0:3-Revision Number) */
    mpdStatus = PRV_MPD_MDIO_READ_MAC( portEntry_PTR->rel_ifIndex,
                                    1,
                                    3,
                                    &regVal);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mpdStatus, portEntry_PTR->rel_ifIndex);

    params_PTR->phyRevision.phyType = portEntry_PTR->initData_PTR->phyType;
    params_PTR->phyRevision.revision = regVal;

    return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetLoopback
 *
 * DESCRIPTION: Debug, Set PHY Lookback
 *
 * APPLICABLE PHYS:
 *                  MPD_TYPE_88E1543_E
 *                  MPD_TYPE_88E1545_E
 *                  MPD_TYPE_88E1548_E
 *                  MPD_TYPE_88E151x_E
 *                  MPD_TYPE_88E1680_E
 *                  MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetLoopback(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 val;
	MPD_RESULT_ENT status;
	if (params_PTR->internal.phyLoopback.enable) {
		/* write 2.21 = 0x1046 default mac interface speed = 1000Mbs */
		val = 0x1046;
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											2,
											21,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/* write 0.0 = 0x4140 Force 1000M Full-duplex and Enable Mac Loopback */
		val = 0x4140;
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											0,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	else {
		/* 0.0.14 = '0' Disable Mac Loopback and phy reset*/
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										0,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		val &= ~(1 << 14);
		val |= 0x9340;
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											0,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetLoopback_2
 *
 * DESCRIPTION: Debug, Set PHY Lookback
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetLoopback_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 val, mask, good_vaule;
	BOOLEAN failure = FALSE;
	MPD_RESULT_ENT status;
	if (params_PTR->internal.phyLoopback.enable) {
		/* write 2.21 = 0x1045 default mac interface speed = 100Mbs */
		val = 0x1045;
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											2,
											21,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/* 0.0 = 0xA100 Force 100M Full */
		val = 0xA100;
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											0,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/* 0.16.10 = '1' - Force link good */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										16,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		val |= (1 << 10);
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											16,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/* Check for copper link: 0.17.14:15 = '01', 0.17.13 = '1' 0.17.10 = '1' */
		good_vaule = (1 << 10) | (1 << 13) | (1 << 14);
		mask = good_vaule | (1 << 15);

		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										16,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		if ((val & mask) != good_vaule) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"+++ No Copper link on %d. 0.17 = %#x",
															portEntry_PTR->rel_ifIndex,
															val);
			failure = TRUE;
		}

		/* Check for qsgmii link: 4.17.14:15 = '01', 4.17.13 = '1' 4.17.10 = '1' */
		good_vaule = (1 << 10) | (1 << 13) | (1 << 14);
		mask = good_vaule | (1 << 15);

		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										4,
										16,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		if ((val & mask) != good_vaule) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"+++ No QSGMII link on %d. 4.17 = %#x",
															portEntry_PTR->rel_ifIndex,
															val);
			failure = TRUE;
		}

		/* 0.0.14 = '1' Enable Mac Loopback */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										0,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		val |= (1 << 14);
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											0,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		PRV_MPD_SLEEP_MAC((100));
		/* 0.16.10 = '1' - Disable Force link good */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										16,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		val &= ~(1 << 10);
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											16,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/* Check for qsgmii link: 4.17.14:15 = '01', 4.17.13 = '1' 4.17.10 = '1' */
		good_vaule = (1 << 10) | (1 << 13) | (1 << 14);
		mask = good_vaule | (1 << 15);

		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										4,
										16,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		if ((val & mask) != good_vaule) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"--- No QSGMII link -- LoopBack %d. 4.17 = %#x",
															portEntry_PTR->rel_ifIndex,
															val);
			failure = TRUE;
		}

		if (failure == FALSE) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"Loopback successful in PHY %d",
															portEntry_PTR->rel_ifIndex);
		}
	}
	else {
		/* 0.0.14 = '0' Disable Mac Loopback */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										0,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		val &= ~(1 << 14);
		val |= 0x9340;
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											0,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetPhyKindCopper
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetPhyKindCopper(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyKindAndMedia.phyKind = MPD_KIND_COPPER_E;
	params_PTR->phyKindAndMedia.isSfpPresent = FALSE;
	params_PTR->phyKindAndMedia.mediaType = MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;
	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetPhyKindSfp
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetPhyKindSfp(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	params_PTR->phyKindAndMedia.phyKind = MPD_KIND_SFP_E;
	params_PTR->phyKindAndMedia.isSfpPresent = portEntry_PTR->runningData_PTR->sfpPresent;
	params_PTR->phyKindAndMedia.mediaType = MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E;

	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetPhyKindCombo
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetPhyKindCombo(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	params_PTR->phyKindAndMedia.phyKind = MPD_KIND_COMBO_E;
	params_PTR->phyKindAndMedia.isSfpPresent = portEntry_PTR->runningData_PTR->sfpPresent;
	params_PTR->phyKindAndMedia.mediaType =
			(params_PTR->phyKindAndMedia.isSfpPresent) ? MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E : MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;

	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetPhyKindByType
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetPhyKindByType(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {
		params_PTR->phyKindAndMedia.phyKind = MPD_KIND_COMBO_E;
		params_PTR->phyKindAndMedia.isSfpPresent = portEntry_PTR->runningData_PTR->sfpPresent;
		params_PTR->phyKindAndMedia.mediaType =
				(params_PTR->phyKindAndMedia.isSfpPresent) ? MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E : MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;
	}
	else if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		params_PTR->phyKindAndMedia.phyKind = MPD_KIND_SFP_E;
		params_PTR->phyKindAndMedia.isSfpPresent = portEntry_PTR->runningData_PTR->sfpPresent;
		params_PTR->phyKindAndMedia.mediaType = MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E;
	}
	else {
		params_PTR->phyKindAndMedia.phyKind = MPD_KIND_COPPER_E;
		params_PTR->phyKindAndMedia.isSfpPresent = FALSE;
		params_PTR->phyKindAndMedia.mediaType = MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdPortRunningDbUpdate
 *
 * DESCRIPTION: update port running DB
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdPortRunningDbUpdate(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OP_CODE_ENT op,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
)
{

    switch (op) {
    case MPD_OP_CODE_SET_PHY_DISABLE_OPER_E:
        portEntry_PTR->runningData_PTR->adminMode = (params_PTR->phyDisable.forceLinkDown == TRUE) ?
                MPD_PORT_ADMIN_DOWN_E : MPD_PORT_ADMIN_UP_E;
        break;
    case MPD_OP_CODE_SET_SPEED_E:
        portEntry_PTR->runningData_PTR->speed = params_PTR->phySpeed.speed;
        break;
    default:
        return MPD_OK_E;
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdPerformPhyOperation
 *
 * DESCRIPTION: perform PHY operaration
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdPerformPhyOperation(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OP_CODE_ENT op,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT ret_val = MPD_OP_FAILED_E;
	MPD_TYPE_ENT phyType;
	BOOLEAN prev_state = FALSE;
	/* no such operation */
	if ((UINT_32)op >= PRV_MPD_NUM_OF_OPS_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return MPD_OP_FAILED_E;
	}
	if ((UINT_32) portEntry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
		/*This is not fatal error because we might try to set operation for port that was disconnect duting init*/
		return MPD_NOT_SUPPORTED_E;
	}

	phyType = portEntry_PTR->initData_PTR->phyType;
	if (prvMpdFuncDb_ARR[phyType] == NULL) {
		return MPD_OP_FAILED_E;
	}

	/* operation not supported */
	if (prvMpdFuncDb_ARR[phyType][op].phyOperation_PTR == NULL) {
		return MPD_NOT_SUPPORTED_E;
	}
	/* 1G phy which support page select, disable PPU */
	if (prvMpdGetPhyOperationName(portEntry_PTR, PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E) != NULL) {
	    PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, TRUE, &prev_state);
	}

    prvMpdDebugPerformPhyOperation(portEntry_PTR, op, params_PTR, TRUE);

	ret_val = prvMpdFuncDb_ARR[phyType][op].phyOperation_PTR(	portEntry_PTR,
																params_PTR);

    prvMpdDebugPerformPhyOperation(portEntry_PTR, op, params_PTR, FALSE);

	/* 1G phy which support page select, revert disable PPU */
	if (prvMpdGetPhyOperationName(portEntry_PTR, PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E) != NULL) {
	    PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, prev_state, &prev_state);
    }

	/* update running DB if required */
	if (ret_val == MPD_OK_E) {
	    prvMpdPortRunningDbUpdate (portEntry_PTR, op, params_PTR);
	}
	return ret_val;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetPhyOperationName
 *
 * DESCRIPTION: for debug, get function name corresponding to operation and port
 *
 *
 *****************************************************************************/
char* prvMpdGetPhyOperationName(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_OP_CODE_ENT op
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	MPD_TYPE_ENT phyType;
	/* no such operation */
	if ((UINT_32)op >= PRV_MPD_NUM_OF_OPS_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_MINOR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return NULL;
	}
	if ((UINT_32) portEntry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
		/*This is not fatal error because we might try to set operation for port that was disconnect duting init*/
		return NULL;
	}

	phyType = portEntry_PTR->initData_PTR->phyType;
	if (prvMpdFuncDb_ARR[phyType] == NULL) {
		return NULL;
	}

	/* operation not supported */
	if (prvMpdFuncDb_ARR[phyType][op].phyOperation_PTR == NULL) {
		return NULL;
	}

	return prvMpdFuncDb_ARR[phyType][op].funcName_PTR;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdPortListInitSequence
 *
 * DESCRIPTION: Perform init sequence for port list
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdPortListInitSequence(
	/*     INPUTS:             */
	PRV_MPD_PORT_LIST_TYP *portsList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	BOOLEAN is_initPhy[MPD_TYPE_NUM_OF_TYPES_E];
	UINT_32 i, rel_ifIndex = 0;
	MPD_TYPE_ENT phyType;
	MPD_RESULT_ENT ret_val;
	PRV_MPD_PORT_LIST_TYP *tmp_portsList_PTR = NULL;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
	memset(	is_initPhy,
			0,
			sizeof(is_initPhy));
	/* if the port list received is NULL, we should configure all valid ports in local DB */
	if (portsList_PTR == NULL) {
		tmp_portsList_PTR = &(prvMpdGlobalDb_PTR->validPorts);
	}
	else {
		tmp_portsList_PTR = portsList_PTR;
	}


	while (prvMpdPortListGetNext(	tmp_portsList_PTR,
									&rel_ifIndex)) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (!portEntry_PTR) {
			continue;
		}

        /* Check mandatory callbacks have been initialized */
        if ((prvMpdCallBacks.mdioRead_PTR == NULL && portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR == NULL)  || 
            (prvMpdCallBacks.mdioWrite_PTR == NULL && portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR == NULL) ||
            (prvMpdCallBacks.alloc_PTR == NULL) || (prvMpdCallBacks.free_PTR == NULL) || (prvMpdCallBacks.sleep_PTR == NULL)) 
        {
            PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
                MPD_ERROR_SEVERITY_FATAL_E,
                PRV_MPD_DEBUG_FUNC_NAME_MAC());
            return MPD_OP_FAILED_E;
        }
        
		/* initialize phy DB */
		phyType = portEntry_PTR->initData_PTR->phyType;
		if (phyType >= MPD_TYPE_NUM_OF_TYPES_E) {
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_FATAL_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_OP_FAILED_E;
		}
        
		if (is_initPhy[phyType] == FALSE) {
			phy_params.internal.phyInit.initializeDb = TRUE;
			prvMpdBindInitFunc(phyType);
			ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
												PRV_MPD_OP_CODE_INIT_E,
												&phy_params);
			if (ret_val == MPD_OP_FAILED_E) {
				ret_val = PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
														MPD_ERROR_SEVERITY_ERROR_E,
														PRV_MPD_DEBUG_FUNC_NAME_MAC());
			}
			is_initPhy[phyType] = TRUE;
		}

		portEntry_PTR->runningData_PTR->macOnPhyState = PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E;
	}
	/* update pre & post configuration callbacks */
	rel_ifIndex = 0;
	while (prvMpdPortListGetNext(	tmp_portsList_PTR,
									&rel_ifIndex)) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (!portEntry_PTR) {
			continue;
		}
		ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
											PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
											&phy_params);
		/* init process is interrupted only in case PHY init reports failure.
		 ** it is acceptable for PHY not to have a specific features routine, in such case
		 ** ret_val will be "not_supported" and flow will continue normally
		 */
		if (ret_val == MPD_OP_FAILED_E) {
			ret_val = PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
													MPD_ERROR_SEVERITY_MINOR_E,
													PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return ret_val;
		}
	}
	/* run pre init configuration callback */
	for (i = 0; i < PRV_MPD_SPECIFIC_FEATURE_LAST_E; i++) {
		if (prvMpdSpecificFeature_ARR[i].supported && prvMpdSpecificFeature_ARR[i].preInitFeatureExecuteCallback_PTR) {
			prvMpdSpecificFeature_ARR[i].preInitFeatureExecuteCallback_PTR(&(prvMpdGlobalDb_PTR->validPorts));
		}
	}
	/* run init configuration on all active ports */
	rel_ifIndex = 0;
	while (prvMpdPortListGetNext(	tmp_portsList_PTR,
									&rel_ifIndex)) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (!portEntry_PTR) {
			continue;
		}
		/* initialize phy */
		phy_params.internal.phyInit.initializeDb = FALSE;

		ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
											PRV_MPD_OP_CODE_INIT_E,
											&phy_params);
		/* init process is interrupted only in case PHY init reports failure.
		 ** it is acceptable for PHY not to have an init routine, in such case
		 ** ret_val will be "not_supported" and flow will continue normally
		 */
		if (ret_val == MPD_OP_FAILED_E) {
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_FATAL_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_OP_FAILED_E;
		}

        /* Disable PHY on init so that network ports will not be active on system startup */
		if (portEntry_PTR->initData_PTR->disableOnInit) {
            memset( &phy_params,
                    0,
                    sizeof(phy_params));
            phy_params.phyDisable.forceLinkDown = TRUE;
            prvMpdPerformPhyOperation(  portEntry_PTR,
                                        MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                                        &phy_params);
		}
	}

	/* run post init configuration callback */
	for (i = 0; i < PRV_MPD_SPECIFIC_FEATURE_LAST_E; i++) {
		if (prvMpdSpecificFeature_ARR[i].supported && prvMpdSpecificFeature_ARR[i].postInitFeatureExecuteCallback_PTR) {
			prvMpdSpecificFeature_ARR[i].postInitFeatureExecuteCallback_PTR(&(prvMpdGlobalDb_PTR->validPorts));
		}
	}

    

	return MPD_OK_E;
}

/**
 *
 * @brief Perform PHY operation
 * @param [in] 		rel_ifIndex - The port on which to perform operation
 * @param [in] 		op			- The logical operation
 * @param [inout]	params_PTR	- Data for operation (Set & Get)
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdPerformPhyOperation(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex,
	MPD_OP_CODE_ENT op,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT phy_ret = MPD_OP_FAILED_E;
	PRV_MPD_PORT_HASH_ENTRY_STC *port_entry_PTR = NULL;
	port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (port_entry_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}

	phy_ret = prvMpdPerformPhyOperation(port_entry_PTR,
										op,
										params_PTR);

	return phy_ret;
}

/* general callback functions */
/* ***************************************************************************
 * FUNCTION NAME: prvMpdSfpPresentNotification
 *
 * DESCRIPTION: 	Called by application in order to indicate SFP present (LOS)
 *
 * APPLICABLE PHYS:
 * 				MPD_TYPE_88E1543_E
 * 				MPD_TYPE_DIRECT_ATTACHED_FIBER_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSfpPresentNotification(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	BOOLEAN isPresent, sfpWa;
	MPD_RESULT_ENT res;
	MPD_OPERATIONS_PARAMS_UNT comboMode;
	MPD_OPERATIONS_PARAMS_UNT enable_fiber;

	isPresent = params_PTR->phySfpPresentNotification.isSfpPresent;
	sfpWa = params_PTR->phySfpPresentNotification.sfpWa;

	switch (portEntry_PTR->initData_PTR->transceiverType) {
		case MPD_TRANSCEIVER_COMBO_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"1. paramsType == MPD_PARAMS_TYPE_COMBO_E");

			portEntry_PTR->runningData_PTR->sfpPresent = isPresent;

			switch (portEntry_PTR->runningData_PTR->comboMode) {
				case MPD_COMBO_MODE_FORCE_FIBER_E:
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"1.1 comboMode == MPD_COMBO_MODE_FORCE_FIBER_E");
					return MPD_OK_E;

				case MPD_COMBO_MODE_FORCE_COPPER_E:
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"1.2 MPD_COMBO_MODE_FORCE_COPPER_E");
					return MPD_OK_E;

				case MPD_COMBO_MODE_PREFER_COPPER_E:
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"1.3 MPD_COMBO_MODE_PREFER_COPPER_E");
					/* prefer copper not supported*/
					PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
												MPD_ERROR_SEVERITY_ERROR_E,
												PRV_MPD_DEBUG_FUNC_NAME_MAC());
					return MPD_NOT_SUPPORTED_E;
					break;

				default: /* MPD_COMBO_MODE_PREFER_FIBER_E */
					PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"1.4 MPD_COMBO_MODE_PREFER_FIBER_E");
					if ((isPresent == TRUE) && (sfpWa == FALSE)) {
						PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"1.4.1. isPresent == TRUE, sfpWa == FALSE, RX loss == TRUE");

						if (portEntry_PTR->runningData_PTR->sfpPresent) {
							return MPD_OK_E;
						}

						PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"1.4.1.2 MPD_COMBO_MODE_PREFER_FIBER_E, Update db");
					comboMode.internal.phyFiberParams.fiberPresent = isPresent;
						comboMode.internal.phyFiberParams.comboMode = MPD_COMBO_MODE_PREFER_FIBER_E;

						res = prvMpdPerformPhyOperation(portEntry_PTR,
														PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
														&comboMode);
						if (res != MPD_OK_E) {
							PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																		"1.4.1.3 prvMpdPerformPhyOperation FAILED!");
							return MPD_OK_E;
						}

						portEntry_PTR->runningData_PTR->sfpPresent = TRUE;

					}
					else { /* isPresent == FALSE */
						PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	"1.4.2 isPresent == FALSE, RX loss == FALSE");
						if (sfpWa) {
							isPresent = FALSE;
							PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																		"1.4.2.1 sfpWa == TRUE, set isPresent = FALSE");
						}
						if (portEntry_PTR->runningData_PTR->sfpPresent == FALSE) {
							PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																		"1.4.2.2 mediaType == HALC_configPhy_copper_media_E, update db");
							return MPD_OK_E;
						}

						comboMode.internal.phyFiberParams.fiberPresent = isPresent;
						comboMode.internal.phyFiberParams.comboMode = MPD_COMBO_MODE_PREFER_FIBER_E;

						res = prvMpdPerformPhyOperation(portEntry_PTR,
														PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
														&comboMode);
						if (res != MPD_OK_E) {
							PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																		"1.4.1.3 prvMpdPerformPhyOperation FAILED!");
							return MPD_OK_E;
						}
						portEntry_PTR->runningData_PTR->sfpPresent = FALSE;
					}
					break;
			} /* @suppress("No break at end of case") */
			MPD_FALLTHROUGH
		case MPD_TRANSCEIVER_SFP_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"2. paramsType == MPD_PARAMS_TYPE_SFP_E");
			portEntry_PTR->runningData_PTR->sfpPresent = isPresent;
			enable_fiber.internal.phyFiberParams.fiberPresent = isPresent;
			enable_fiber.internal.phyFiberParams.comboMode = MPD_COMBO_MODE_FORCE_FIBER_E;
			res = prvMpdPerformPhyOperation(portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E,
											&enable_fiber);
			break;

		case MPD_TRANSCEIVER_COPPER_E:
			default:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"3. paramsType == MPD_PARAMS_TYPE_COPPER_E");
			return MPD_NOT_SUPPORTED_E;
	}
	if (isPresent == TRUE) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugSfpFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													"4. isPresent == TRUE, update db");
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME:    prvMpdMdioAccess
 *
 * DESCRIPTION:      smi read/write
 *
 * APPLICABLE PHYS:
 * 				all SMI accessed PHYs
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioAccess(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_8 						deviceOrPage, offset;
	UINT_16 					data;
	GT_STATUS 					rc;
	MPD_MDIO_ACCESS_TYPE_ENT 	type;


	type = params_PTR->phyMdioAccess.type;
	deviceOrPage = params_PTR->phyMdioAccess.deviceOrPage;
	offset = params_PTR->phyMdioAccess.address;
	data = params_PTR->phyMdioAccess.data;
	if (type == MPD_MDIO_ACCESS_READ_E) {
		rc = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									deviceOrPage,
									offset,
									&data);
		params_PTR->phyMdioAccess.data = data;
	}
	else {
		rc = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										deviceOrPage,
										offset,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										data);
	}
	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);
	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetMdixOperType_3
 *
 * DESCRIPTION: 	Get the mdix operational status, the status can be mdi/mdix.
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetMdixOperType_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 		val;
	GT_STATUS 	status;

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - mdi admin not supported */
		return MPD_NOT_SUPPORTED_E;
	}

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_SPECIFIC_STATUS_REG_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);


	if (val & PRV_MPD_MDI_CROSSOVER_STATUS_CNS) {
		params_PTR->phyMdix.mode = MPD_MDIX_MODE_MEDIA_E;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"mdixMode");
	}
	else {
		params_PTR->phyMdix.mode = MPD_MDI_MODE_MEDIA_E;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"mdiMode");
	}

    if (portEntry_PTR->initData_PTR->invertMdi == TRUE) {
		params_PTR->phyMdix.mode = (params_PTR->phyMdix.mode == MPD_MDIX_MODE_MEDIA_E)?MPD_MDI_MODE_MEDIA_E:MPD_MDIX_MODE_MEDIA_E;
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetMdixOperType_3
 *
 * DESCRIPTION: 	Set the mdix mode, mdi/mdix/auto.
 *
 * register 16.6:5 MDI Crossover Mode 00 = Manual MDI configuration
 *                                    01 = Manual MDIX configuration
 *                                    10 = Reserved
 *                                    11 = Enable automatic crossover for all modes
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdSetMdixAdminMode_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	GT_U16 val;
	BOOLEAN is_value_different;
	unsigned int phyLink_st;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - mdi configuration is not supported */
		return MPD_NOT_SUPPORTED_E;
	}

	is_value_different = FALSE;
	switch (params_PTR->phyMdix.mode) {
		case MPD_MDI_MODE_MEDIA_E:
			/* set mdi mode*/
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
											&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
			if ((val & PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS) != PRV_MPD_MANUAL_MDI_CONFIGURATION_CNS) {
				val &= ~PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS;
				val |= PRV_MPD_MANUAL_MDI_CONFIGURATION_CNS;
				is_value_different = TRUE;
				status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													0,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													val);
			}

			break;

		case MPD_MDIX_MODE_MEDIA_E:
			/* set mdix mode*/
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
											&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

			if ((val & PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS) != PRV_MPD_MANUAL_MDIX_CONFIGURATION_CNS) {
				val &= ~PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS;
				val |= PRV_MPD_MANUAL_MDIX_CONFIGURATION_CNS;
				is_value_different = TRUE;
				status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													0,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													val);
			}

			break;

		case MPD_AUTO_MODE_MEDIA_E:
			/* set auto mdix mode*/
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
											&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

			if ((val & PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS) != PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS) {
				val |= PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS;
				is_value_different = TRUE;
				status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													0,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													val);
			}

			break;

		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_NOT_SUPPORTED_E;
	}
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (is_value_different) {
		/* reset the phy (commit the changes) */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		phyLink_st = val & PRV_MPD_LINK_DOWN_CNS; /*phy bit 11 == 0 - enabled, else disabled */

		val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* software reset bit */
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);

		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
		phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
		status = prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ERRATA_E,
											&phy_params);
		if (status == MPD_OP_FAILED_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d. Set Errata failed",
                                                            portEntry_PTR->rel_ifIndex);
			return MPD_OP_FAILED_E;
		}

		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/*set the phy bit 11 as it was before the test*/
		if (phyLink_st) { /*phy link was disabled*/
			val |= PRV_MPD_LINK_DOWN_CNS;
		}
		else { /*phy link was enabled*/
			val &= PRV_MPD_LINK_UP_CNS;
		}
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	}
	return MPD_OK_E;
}

/*****************************************************************************
 * FUNCTION NAME: prvMpdSetVctTest_4
 *
 * DESCRIPTION:  Run vct test for Alaska PHYs
 *
 * ALGORITHM:  1. Run vct test and get results.
 *             2. Find the closest fail in the 4 pairs.
 *             3. If there is a test fail, fail all the test.
 *             4. If there are more than 1 fails in the same distance the
 *                priority is fail (highest), short , open (lowest).This is done
 *                to assure consistence results
 *             5. The results are separated to two. The first two pairs and the last two pars.
 *                It is done because a 2 pair cable (the first two) can be a good cable
 *                for FE or 10M (but it is not good for Giga).
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetVctTest_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 reg_0_val;
	UINT_16 phyLink_st;
	BOOLEAN isTwoPairCable = FALSE;
	GT_STATUS status;
	MPD_RESULT_ENT vct_stat;
	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d",
														portEntry_PTR->rel_ifIndex);

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - VCT is not supported */
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. port is fiber only - VCT is not supported. phy_op_failed 1",
															portEntry_PTR->rel_ifIndex);
		return MPD_NOT_SUPPORTED_E;
	}

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&reg_0_val);

	phyLink_st = reg_0_val & PRV_MPD_LINK_DOWN_CNS; /*phy bit 11 == 0 - enabled, else disabled */
	if (phyLink_st) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. port is Down - VCT is not supported. phy_op_not_supported",
															portEntry_PTR->rel_ifIndex);
		return MPD_NOT_SUPPORTED_E;
	}

	reg_0_val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* software reset bit */
	/* reset the phy (commit the changes) */
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										reg_0_val);

	if (status != MPD_OK_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. phy_op_failed 2",
															portEntry_PTR->rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	/*checks if this function was called from prvMpdSetVctTest_7 (for phy 3680)*/
	if (params_PTR->phySpeed.speed == MPD_SPEED_100M_E) {
		isTwoPairCable = TRUE;
	}

	vct_stat = prvMpdAnalyzeVct(portEntry_PTR,
								isTwoPairCable,
								&(params_PTR->phyVct));

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"after analyze: portEntry_PTR->rel_ifIndex %d done. testResult %d, cableLen %d",
														portEntry_PTR->rel_ifIndex,
														params_PTR->phyVct.testResult,
														params_PTR->phyVct.cableLength);

	if (vct_stat == MPD_OP_FAILED_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. phy_op_failed 3",
															portEntry_PTR->rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	/* reset the phy (commit the changes) */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&reg_0_val);

	if (status != MPD_OK_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return MPD_OP_FAILED_E;
	}

	reg_0_val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* software reset bit */

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										reg_0_val);

	if (status != MPD_OK_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return MPD_OP_FAILED_E;
	}
	/*read before set to phy link state that was at the beginning*/
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&reg_0_val);

	if (status != MPD_OK_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return MPD_OP_FAILED_E;
	}

	reg_0_val &= PRV_MPD_LINK_UP_CNS;

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										reg_0_val);

	if (status != MPD_OK_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return MPD_OP_FAILED_E;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d done. vct_stat %d, phyLink_st 0x%x",
														portEntry_PTR->rel_ifIndex,
														vct_stat,
														phyLink_st);

	return vct_stat;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetVctTest_7
 *
 * DESCRIPTION: 	Run vct test
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetVctTest_7(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	params_PTR->phySpeed.speed = MPD_SPEED_100M_E;
	return prvMpdSetVctTest_4(	portEntry_PTR,
								params_PTR);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetExtVctParams_1
 *
 * DESCRIPTION: 	Eetrieve extended VCT parametes
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *                  MPD_TYPE_88E1780_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetExtVctParams_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	MPD_VCT_CABLE_EXTENDED_STATUS_STC cableExtStatus;

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d",
														portEntry_PTR->rel_ifIndex);

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - VCT is not supported */
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. port is fiber only - VCT is not supported",
															portEntry_PTR->rel_ifIndex);

		return MPD_NOT_SUPPORTED_E;
	}

    memset( &cableExtStatus,
            0,
            sizeof(MPD_VCT_CABLE_EXTENDED_STATUS_STC));
	status = prvMpdVctAnalyzeVctExtendedCable(portEntry_PTR, &cableExtStatus);

	/* ANALIZE THE VCT RESULTS */
	if (status != MPD_OK_E || cableExtStatus.vctExtendedCableStatus.isValid == FALSE) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. phy_op_failed",
															portEntry_PTR->rel_ifIndex);

		return MPD_OP_FAILED_E;
	}

	switch (params_PTR->phyExtVct.testType) {
		case MPD_VCT_TEST_TYPE_CABLECHANNEL1_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSwap[0];
			break;
		case MPD_VCT_TEST_TYPE_CABLECHANNEL2_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSwap[0];
			break;
		case MPD_VCT_TEST_TYPE_CABLECHANNEL3_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSwap[1];
			break;
		case MPD_VCT_TEST_TYPE_CABLECHANNEL4_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSwap[1];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPOLARITY1_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairPolarity[0];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPOLARITY2_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairPolarity[1];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPOLARITY3_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairPolarity[2];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPOLARITY4_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairPolarity[3];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPAIRSKEW1_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSkew.skew[0];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPAIRSKEW2_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSkew.skew[1];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPAIRSKEW3_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSkew.skew[2];
			break;
		case MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E:
			params_PTR->phyExtVct.result = cableExtStatus.vctExtendedCableStatus.pairSkew.skew[3];
			break;
	}

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d. testType %d, result %d",
														portEntry_PTR->rel_ifIndex,
														params_PTR->phyExtVct.testType,
														params_PTR->phyExtVct.result);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetCableLen_4
 *
 * DESCRIPTION: 	Get cable length in 30m resolution
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetCableLen_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 pairNum;
	UINT_32 totalCableLen;
	BOOLEAN isValid = TRUE;
	MPD_RESULT_ENT status;
	MPD_RESULT_ENT retVal = MPD_OK_E;
	MPD_VCT_CABLE_EXTENDED_STATUS_STC cableExtStatus;
	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d",
														portEntry_PTR->rel_ifIndex);

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - VCT is not supported */
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. port is fiber only - VCT is not supported",
															portEntry_PTR->rel_ifIndex);

		return MPD_NOT_SUPPORTED_E;
	}

	memset(	&cableExtStatus,
			0,
			sizeof(MPD_VCT_CABLE_EXTENDED_STATUS_STC));
	status = prvMpdVctAnalyzeVctExtendedCable(portEntry_PTR, &cableExtStatus);

	/* ANALIZE THE VCT RESULTS */
	if (status != MPD_OK_E) {
		if (status == MPD_NOT_SUPPORTED_E) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"portEntry_PTR->rel_ifIndex %d. phy_op_not_supported",
																portEntry_PTR->rel_ifIndex);

			return MPD_NOT_SUPPORTED_E;
		}
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. phy_op_failed",
															portEntry_PTR->rel_ifIndex);

		return MPD_OP_FAILED_E;
	}

	totalCableLen = 0;

	/*calculate average from all 4 pairs*/
	for (pairNum = 0; pairNum < 4; pairNum++) {
		if (!cableExtStatus.accurateCableLen.isValid[pairNum]) {
			isValid = FALSE;
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"pair %d Not valid",
																pairNum);
			break;
		}
		totalCableLen += cableExtStatus.accurateCableLen.cableLen[pairNum];
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. totalCableLen %d, pair %d len %d",
															portEntry_PTR->rel_ifIndex,
															totalCableLen,
															pairNum,
															cableExtStatus.accurateCableLen.cableLen[pairNum]);
	}
	if (isValid) {
		totalCableLen = totalCableLen / 4;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. totalCableLen %d",
															portEntry_PTR->rel_ifIndex,
															totalCableLen);

		if (totalCableLen <= 50) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_LESS_THAN_50M_E;
		}
		else if (totalCableLen <= 80) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_50M_80M_E;
		}
		else if (totalCableLen <= 110) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_80M_110M_E;
		}
		else if (totalCableLen <= 140) {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_110M_140M_E;
		}
		else {
			params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_MORE_THAN_140M_E;
		}
	}
	else {
		params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_UNKNOWN_E;

		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d.phy_cableLength_unknown !!",
															portEntry_PTR->rel_ifIndex);

		retVal = MPD_OP_FAILED_E;
	}
	return retVal;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetCableLen_4
 *
 * DESCRIPTION: 	Perfor PHY reset
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1548_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdResetPhy_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_8 phy_control_page;
	UINT_16 cntrl_val;
	UINT_16 phyLink_st;
	UINT_32 first_page, last_page;
	MPD_RESULT_ENT status;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	MPD_UNUSED_PARAM(params_PTR);
	first_page = last_page = PRV_MPD_PAGE_0_COPPER_CONTROL_CNS;

	switch (portEntry_PTR->initData_PTR->transceiverType) {
		case MPD_TRANSCEIVER_COPPER_E:
			break;
		case MPD_TRANSCEIVER_SFP_E:
			first_page = PRV_MPD_PAGE_1_FIBER_CONTROL_CNS;
			/* fall through */
			MPD_FALLTHROUGH
		case MPD_TRANSCEIVER_COMBO_E:
			last_page = PRV_MPD_PAGE_1_FIBER_CONTROL_CNS;
			break;
        case MPD_TRANSCEIVER_MAX_TYPES_E:
        default:
            return MPD_OP_FAILED_E;
            break;
	}

	for (phy_control_page = first_page; phy_control_page <= last_page;
			phy_control_page++) {
		phyLink_st = cntrl_val = 0;
		/* Read the current value of the control register */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phy_control_page,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&cntrl_val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		phyLink_st = cntrl_val & PRV_MPD_LINK_DOWN_CNS; /*phy bit 11 == 0 - enabled, else disabled */

		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phy_control_page,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											(GT_U16) (cntrl_val | PRV_MPD_CTRLREG_RESET_BIT_CNS));
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
		phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
		status = prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ERRATA_E,
											&phy_params);
		if (status == MPD_OP_FAILED_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                            portEntry_PTR->rel_ifIndex);
            return MPD_OP_FAILED_E;
		}

		/* Read the current value of the control register */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phy_control_page,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&cntrl_val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/*set the phy bit 11 as it was before the test*/
		if (phyLink_st) { /*phy link was disabled*/
			cntrl_val |= PRV_MPD_LINK_DOWN_CNS;
		}
		else { /*phy link was enabled*/
			cntrl_val &= PRV_MPD_LINK_UP_CNS;
		}
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phy_control_page,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											cntrl_val);
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDisableOperation_5
 *
 * DESCRIPTION: 	Adminstrative shutdown of PHY
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1548_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdDisableOperation_5(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 phy_control_page;
	UINT_16 val;
	BOOLEAN enable;
	MPD_RESULT_ENT status;

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only */
		phy_control_page = PRV_MPD_PAGE_1_FIBER_CONTROL_CNS;
	}
	else {
		phy_control_page = PRV_MPD_PAGE_0_COPPER_CONTROL_CNS;
	}

	/* set power down */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									phy_control_page,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	switch (params_PTR->phyDisable.forceLinkDown) {
		case TRUE: /*perform disable operation*/
			val = val | PRV_MPD_LINK_DOWN_CNS;
			enable = FALSE;
			break;
		case FALSE: /*enable the port*/
			val = val & PRV_MPD_LINK_UP_CNS;
			enable = TRUE;
			break;
		default:
			val = val & PRV_MPD_LINK_UP_CNS;
			enable = TRUE;
			break;
	}

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phy_control_page,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	PRV_MPD_TRANSCEIVER_ENABLE_MAC(	portEntry_PTR->rel_ifIndex,
									enable);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetVctCapability_5
 *
 * DESCRIPTION: Get speeds in which VCT is supported
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetVctCapability_5(

	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyVctCapab.vctSupportedSpeedsBitmap = MPD_SPEED_CAPABILITY_1G_CNS | MPD_SPEED_CAPABILITY_2500M_CNS | MPD_SPEED_CAPABILITY_5G_CNS;
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetVctCapability_1
 *
 * DESCRIPTION: Get speeds in which VCT is supported
 *
 * APPLICABLE PHYS:
 *				MPD_TYPE_88E3680_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetVctCapability_1(

	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyVctCapab.vctSupportedSpeedsBitmap = MPD_SPEED_CAPABILITY_100M_CNS;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetVctCapability_2
 *
 * DESCRIPTION: Get speeds in which VCT is supported
 *
 * APPLICABLE PHYS:
 * 					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetVctCapability_2(

	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyVctCapab.vctSupportedSpeedsBitmap = MPD_SPEED_CAPABILITY_1G_CNS;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetVctCapability_3
 *
 * DESCRIPTION: Get speeds in which VCT is supported
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88X32x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetVctCapability_3(

	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyVctCapab.vctSupportedSpeedsBitmap = MPD_SPEED_CAPABILITY_10G_CNS;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetVctCapability_4
 *
 * DESCRIPTION: Get speeds in which VCT is supported
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88X33x0_E
 *					MPD_TYPE_88X20x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetVctCapability_4(

	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyVctCapab.vctSupportedSpeedsBitmap = (MPD_SPEED_CAPABILITY_1G_CNS | MPD_SPEED_CAPABILITY_2500M_CNS |
	MPD_SPEED_CAPABILITY_5G_CNS | MPD_SPEED_CAPABILITY_10G_CNS);
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugVctGetOffset
 *
 * DESCRIPTION: Utility, get VCT offset (length without cable)
 *
 *
 *****************************************************************************/
static void prvMpdDebugVctGetOffset(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *offset_PTR
)
{
	UINT_16 val;
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
						0x00FA);
	prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						13,
						&val);

	val &= 0x3F00;
	*(offset_PTR) = val >> 8;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpd_debug_vct_write_offset
 *
 * DESCRIPTION:	Utility, Set VCT offset (so length without cable will be 0)
 *
 *
 *****************************************************************************/
static void prvMpdDebugVctWriteOffset(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_32 offset
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	UINT_16 val;
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
						0x00FA);
	prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						13,
						&val);
	/* clear vct offset value */
	val &= 0xC0FF;
	/* set new offset */
	val |= ((offset & 0x3f) << 8);
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						13,
						val);
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
						0);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugVctGetLen
 *
 * DESCRIPTION:	Utility, debug, get length in order to test offset configuration
 *
 *
 *****************************************************************************/
static UINT_32 prvMpdDebugVctGetLen(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
#define num_of_retries 20
	UINT_16 val = 0;
	UINT_32 tries, len = 0, i;
	BOOLEAN ready = FALSE;

	/* reset PHY */
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
						0);
	prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						0,
						&val);
	val |= 0x8000;
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						0,
						val);

	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
						0x5);
	prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						23,
						&val);
	val = val | 0x8000; /* Bit 15 turned ON = Enable test */
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						23,
						val); /* Enable test */

	/* wait for test completion */
	tries = 0;
	do {
		prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							23,
							&val);
		ready = ((val & 0x4000) == 0x4000); /* Bit 14 turned ON = test completed */
		if (!ready) {
			PRV_MPD_SLEEP_MAC((300));
		}
		tries++;
	} while (!ready && tries < num_of_retries);

	if (!ready) {
		return 0xFFFF;
	}
	/* Check results in registers 16-19 */
	for (i = 16; i <= 19; i++) {
		prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							i,
							&val);
		val = val & 0xFF;
		len = max(	len,
					val);
	}
	return len;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetVctOffset_1
 *
 * DESCRIPTION:	Get offset, value that should be set in order to
 * 	get VCT length = 0 when no cable connected
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetVctOffset_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 initial_val;
    INT_32 offset, len;

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - VCT is not supported */
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. port is fiber only - VCT is not supported",
															portEntry_PTR->rel_ifIndex);
		return MPD_NOT_SUPPORTED_E;
	}

	prvMpdDebugVctGetOffset(portEntry_PTR,
							&initial_val);
	for (offset = 0; offset < 63; offset++) {
		prvMpdDebugVctWriteOffset(	portEntry_PTR,
									offset);
		len = prvMpdDebugVctGetLen(portEntry_PTR);
		if (len == 0) {
			break;
		}
		offset = max(offset, (len - 1));
	}
    portEntry_PTR->runningData_PTR->vct_offset = offset;
	if (params_PTR->internal.phyVctOffset.do_set == FALSE) {
		/* Returns the original offset */
		prvMpdDebugVctWriteOffset(	portEntry_PTR,
									initial_val);
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetDteStatus_1
 *
 * DESCRIPTION: Get DTE power status (detected/not detected)
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E

 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetDteStatus_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 val;
	MPD_RESULT_ENT status;

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_SPECIFIC_STATUS_REG_CNS,
									&val);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	params_PTR->phyDteStatus.dteDetetcted =
			(val & PRV_MPD_DTE_POWER_STATUS_STATUS_CNS) ? TRUE : FALSE;

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetLinkPartnerPauseCapable_4
 *
 * DESCRIPTION: Get link partner pause (FC) capability
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E

 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetLinkPartnerPauseCapable_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_8 phyPage;
	UINT_16 val, fc_good_value, asym_good_value;
	BOOLEAN is_fiber_combo;
	MPD_RESULT_ENT status;

	is_fiber_combo = (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
			(portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
			(portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E));

	if (is_fiber_combo) {
		phyPage = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
		fc_good_value = 0xA0;
		asym_good_value = 0x100;
	}
	else {
		phyPage = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
		fc_good_value = 0x400;
		asym_good_value = 0x800;
	}

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									phyPage,
									PRV_MPD_LINK_PARTNER_ABILITY_REG_CNS,
									&val);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (val & fc_good_value) {
		params_PTR->phyLinkPartnerFc.pauseCapable = TRUE;
	}
	else {
		params_PTR->phyLinkPartnerFc.pauseCapable = FALSE;
	}

	if (val & asym_good_value) {
		params_PTR->phyLinkPartnerFc.asymetricRequested = TRUE;
	}
	else {
		params_PTR->phyLinkPartnerFc.asymetricRequested = FALSE;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadinessValue
 *
 * DESCRIPTION:	Utility, Get green readiness (support method) value
 *
 *
 *****************************************************************************/
static MPD_GREEN_READINESS_TYPE_ENT prvMpdGetGreenReadinessValue(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_GREEN_READINESS_TYPE_ENT copper_val
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)
		return MPD_GREEN_READINESS_FIBER_E;

	else if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {
		if (portEntry_PTR->runningData_PTR->comboMode == MPD_COMBO_MODE_FORCE_COPPER_E ||
				portEntry_PTR->runningData_PTR->sfpPresent == FALSE)
			return copper_val;
		else
			return MPD_GREEN_READINESS_COMBO_FIBER_E;

	}
	else
		/* copper port */
		return copper_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadiness_5
 *
 * DESCRIPTION: Get green readiness (support method) value
 *
 * APPLICABLE PHYS:
 * 				MPD_TYPE_88X20x0_E
 * 				MPD_TYPE_88X33x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenReadiness_5(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	if (params_PTR->phyGreen.type == MPD_GREEN_READINESS_TYPE_SR_E) {
		params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																		MPD_GREEN_READINESS_ALWAYS_ENABLED_E);
	}
	else {
		params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																		MPD_GREEN_READINESS_OPRNORMAL_E);
	}

	params_PTR->phyGreen.srSpeeds = (MPD_SPEED_CAPABILITY_1G_CNS |
	MPD_SPEED_CAPABILITY_2500M_CNS |
	MPD_SPEED_CAPABILITY_5G_CNS |
	MPD_SPEED_CAPABILITY_10G_CNS);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadiness_3
 *
 * DESCRIPTION: Get green readiness (support method) value
 *
 * APPLICABLE PHYS:
 * 				MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenReadiness_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	if (params_PTR->phyGreen.type == MPD_GREEN_READINESS_TYPE_SR_E) {
		params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																		MPD_GREEN_READINESS_ALWAYS_ENABLED_E);
	}
	else {
		params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																		MPD_GREEN_READINESS_OPRNORMAL_E);
	}

	params_PTR->phyGreen.srSpeeds = MPD_SPEED_CAPABILITY_10G_CNS;

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadiness_1
 *
 * DESCRIPTION: Get green readiness (support method) value
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenReadiness_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	/* get port status */
	params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																	MPD_GREEN_READINESS_OPRNORMAL_E);

	params_PTR->phyGreen.srSpeeds = MPD_SPEED_CAPABILITY_1G_CNS;

	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadiness_4
 *
 * DESCRIPTION: Get green readiness (support method) value
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenReadiness_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyGreen.readiness = MPD_GREEN_READINESS_OPRNORMAL_E;
	params_PTR->phyGreen.srSpeeds = MPD_SPEED_CAPABILITY_100M_CNS;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadiness_6
 *
 * DESCRIPTION: Get green readiness (support method) value
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenReadiness_6(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	if (params_PTR->phyGreen.type == MPD_GREEN_READINESS_TYPE_SR_E) {
		params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																		MPD_GREEN_READINESS_ALWAYS_ENABLED_E);
	}
	else {
		params_PTR->phyGreen.readiness = prvMpdGetGreenReadinessValue(	portEntry_PTR,
																		MPD_GREEN_READINESS_OPRNORMAL_E);
	}
	params_PTR->phyGreen.srSpeeds = (MPD_SPEED_CAPABILITY_1G_CNS |
	MPD_SPEED_CAPABILITY_2500M_CNS |
	MPD_SPEED_CAPABILITY_5G_CNS);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenReadiness_8
 *
 * DESCRIPTION: Get green readiness (support method) value
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_DIRECT_ATTACHED_FIBER_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenReadiness_8(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->phyGreen.readiness = MPD_GREEN_READINESS_FIBER_E;
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetCableLenNoRange_1
 *
 * DESCRIPTION: 	Get accurate cable length
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetCableLenNoRange_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
#ifndef PHY_SIMULATION
	MPD_VCT_CABLE_EXTENDED_STATUS_STC cableExtStatus;
	MPD_RESULT_ENT status;
	UINT_32 pairNum;
	UINT_32 totalCableLen;
	BOOLEAN isValid = TRUE;
#endif
	MPD_RESULT_ENT retVal = MPD_OK_E;

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d",
														portEntry_PTR->rel_ifIndex);

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - mdi admin not supported */
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"rel_ifIndex %d. port is fiber only - mdi admin not supported",
															portEntry_PTR->rel_ifIndex);

		return MPD_NOT_SUPPORTED_E;
	}

	params_PTR->phyCableLenNoRange.cableLen = 0;
#ifndef PHY_SIMULATION
	memset(	&cableExtStatus,
			0,
			sizeof(MPD_VCT_CABLE_EXTENDED_STATUS_STC));

	status = prvMpdVctAnalyzeVctExtendedCable(portEntry_PTR, &cableExtStatus);
	/* ANALIZE THE VCT RESULTS */
	if (status != MPD_OK_E) {
		if (status == MPD_NOT_SUPPORTED_E) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"rel_ifIndex %d not supported",
																portEntry_PTR->rel_ifIndex);

			return MPD_NOT_SUPPORTED_E;
		}
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"test on rel_ifIndex %d returned with isValid == FALSE",
															portEntry_PTR->rel_ifIndex);

		return MPD_OP_FAILED_E;
	}

	totalCableLen = 0;
	/*calculate average from all 4 pairs*/
	for (pairNum = 0; pairNum < 4; pairNum++) {
		if (!cableExtStatus.accurateCableLen.isValid[pairNum]) {
			isValid = FALSE;
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"pair %d Not valid",
																pairNum);
			break;
		}
		totalCableLen += cableExtStatus.accurateCableLen.cableLen[pairNum];
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"totalCableLen %d, len %d for pair %d",
															totalCableLen,
															cableExtStatus.accurateCableLen.cableLen[pairNum],
															pairNum);
	}
	if (isValid) {
		totalCableLen = totalCableLen / 4;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"cable_legth %d",
															totalCableLen);

		params_PTR->phyCableLenNoRange.cableLen = totalCableLen;
	}
	else {
		params_PTR->phyCableLenNoRange.cableLen = 0;
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"cable_legth not valid %d",
															totalCableLen);

		retVal = MPD_OK_E;
	}
#else
        MPD_UNUSED_PARAM(params_PTR);
#endif
	return retVal;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetComboMediaType_3
 *
 * DESCRIPTION: Set combo media type
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1545_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetComboMediaType_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 mode = 7; /* mode[0-2]*/
	UINT_16 val;
	GT_STATUS status;
	MPD_SPEED_ENT phySpeed;

	/* get value of the general control register */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									6,
									20,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* zero mode field */
	val &= ~mode;

	/* Set PHY Mode */
	if (params_PTR->internal.phyFiberParams.fiberPresent == FALSE) {
		/* copper */
		/* QSGMII to Auto Media Detect 1000Base-X */
		mode = 7;

		/* Update BOX about set combo media type to copper
		 * todo - move to core
		 BOXG_port_action(portEntry_PTR->rel_ifIndex, BOXG_port_ledControlMode_CNS, BOXG_port_ledControl_setComboMediaType_copper_CNS);
		 */
		/* Change page to 0. */
		val = 0;
		status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
									val);
	}
	else { /* fiber */
		/* get port speed */
		/* only in case of sfd - port type is actual port speed */
		phySpeed = params_PTR->internal.phyFiberParams.phySpeed;/*HALCDb_convert_ifIndex_toType(ifIndex);*/
		if (phySpeed == MPD_SPEED_100M_E) {

			/* configure as 100 sfp - QSGMII (system mode) to 100Base-FX */
			mode = 3;

			/* Set duplex to full on fiber page */
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												1,
												0,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												(0xA100 | PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS));

		}
		else {
			/* configure as 1G sfp - QSGMII (system mode) to Auto Media Detect 1000Base-X */
			mode = 7;
		}

		/* Update BOX about set combo media type to copper
		 * todo - move to core
		 BOXG_port_action(portEntry_PTR->rel_ifIndex, BOXG_port_ledControlMode_CNS, BOXG_port_ledControl_setComboMediaType_fiber_CNS);
		 */
		/* Change page to 1. */
		val = 1;
		status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
									val);
	}

	/* Set Auto-Media Detect Preferred Media */

	switch (params_PTR->internal.phyFiberParams.comboMode) {
		case MPD_COMBO_MODE_FORCE_COPPER_E:
			/* set phy to prefer copper */
			val |= (1 << 4);
			val &= ~(1 << 5);
			break;
		case MPD_COMBO_MODE_PREFER_FIBER_E:
			case MPD_COMBO_MODE_FORCE_FIBER_E:
			/* set phy to prefer fiber*/
			val &= ~(1 << 4);
			val |= (1 << 5);
			break;
		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_NOT_SUPPORTED_E;
	}

	/* update DB */
	portEntry_PTR->runningData_PTR->sfpPresent = params_PTR->internal.phyFiberParams.fiberPresent;
	/* reset phy mode */
	val |= (1 << 15);
	val |= mode;
	/* set prefare fiber/copper*/
	/* 1322 support auto media select, so this should be enough.*/
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										6,
										20,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);

	/* reset phy */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									0,
									&val);
	val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										0,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									1,
									0,
									&val);
	/* write only if there is a change */
	if ((val & PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS) == 0) {
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											1,
											0,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											(GT_U16) (val | PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS));
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetAutoNegotiationRemoteCapabilities_4
 *
 * DESCRIPTION: Get auto negotiation remote capabilities
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetAutoNegotiationRemoteCapabilities_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	MPD_RESULT_ENT status;
	GT_U16 val;

	params_PTR->phyRemoteAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;

	status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_CTRLREG1_OFFSET_CNS,
								&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (val & 0x0020) { /* Auto-Negotiation process complete */

		status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									6,
									&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		if (val & 0x0001) { /* Link Partner is Auto-Negotiation able */

			status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_LINK_PARTNER_ABILITY_REG_CNS,
										&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

			params_PTR->phyRemoteAutoneg.capabilities = 0;

			if (val & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100F_CNS) {
				params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
			}
			if (val & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100H_CNS) {
				params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
			}
			if (val & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10F_CNS) {
				params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
			}
			if (val & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10H_CNS) {
				params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
			}
			return MPD_OK_E;
		}
	}
	return MPD_NOT_SUPPORTED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetAutoNegotiation_6
 *
 * DESCRIPTION: Set the auto negotiation configuration
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1548_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdSetAutoNegotiation_6(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	BOOLEAN in_req_autoneg;
	MPD_RESULT_ENT status;
	GT_U16 val = 0, val2;
	GT_U16 capabilities1 = 0;
	GT_U16 capabilities2 = 0;
	GT_U16 control_reg_val, pause_advertise_bit = 0;
	GT_U8 phyPage;
	BOOLEAN is_fiber_combo, do_errata = FALSE;
	GT_U32 an_counter = 0;
	MPD_TRANSCEIVER_TYPE_ENT paramType;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	/* split the variable becasue there is extra action for fiber ports*/
	paramType = portEntry_PTR->initData_PTR->transceiverType;
	is_fiber_combo = (((paramType == MPD_TRANSCEIVER_COMBO_E) &&
			(portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
			(paramType == MPD_TRANSCEIVER_SFP_E));

	if (is_fiber_combo) {
		phyPage = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
	}
	else {
		phyPage = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
	}

	switch (params_PTR->phyAutoneg.enable) {

		case MPD_AUTO_NEGOTIATION_ENABLE_E:
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_10_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_10_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_100_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_100_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS) {
				capabilities2 |= PRV_MPD_ADVERTISE_1000_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS) {
				capabilities2 |= PRV_MPD_ADVERTISE_1000_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) {
				capabilities2 |= PRV_MPD_1000BASE_AN_PREFERMASTER_CNS; /* set bit - prefer master */
			}

			in_req_autoneg = prvMpdCheckIfPhyIsInReqAutoneg(portEntry_PTR,
															MPD_AUTO_NEGOTIATION_ENABLE_E,
															phyPage);

			if (in_req_autoneg) {

				if (is_fiber_combo == TRUE) {

					status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													phyPage,
													PRV_MPD_AUTNEGADVER_OFFSET_CNS,
													&val);

					if (val == PRV_MPD_FIBER_ADVERTISE_CNS)
						return MPD_OK_E;
				}
				else {
					/* Already in the requested auto neg mode.
					 So there is no need to change it again */
					/* check if we had advertised all */
					/*val = PRV_MPD_ADVERTISE_BITS_CNS;*/
					status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													phyPage,
													PRV_MPD_AUTNEGADVER_OFFSET_CNS,
													&val);

					status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													phyPage,
													PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
													&val2);

					if (((val & PRV_MPD_ADVERTISE_10_100_CNS) == capabilities1) &&
							((val2 & PRV_MPD_1000BASE_AN_PREFER_MASK_CNS) == capabilities2))
						return MPD_OK_E;
				}
			}

			if (is_fiber_combo == TRUE) {

				val = PRV_MPD_FIBER_ADVERTISE_CNS;
				do {
					/* change mode from SGMII to fiber (and back) requires delay for HW configuration */
					/* sleep minimum time*/
					PRV_MPD_SLEEP_MAC(1);
					status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														portEntry_PTR,
														phyPage,
														PRV_MPD_AUTNEGADVER_OFFSET_CNS,
														PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
														val);

					status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													phyPage,
													PRV_MPD_AUTNEGADVER_OFFSET_CNS,
													&val2);
					an_counter++;
				} while ((val != val2) && (an_counter < prvMpd_autoneg_num_of_retries_CNS));
			}
			else {
				if (val & 0x0400) {
					pause_advertise_bit = 0x0400;
				}
				/* First set advertise bits */
				val = capabilities1 | 0x0001 | pause_advertise_bit;
				status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													phyPage,
													PRV_MPD_AUTNEGADVER_OFFSET_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													val);

				/* set advertise_1000 bits */
				/* read val before writing back*/
				status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												phyPage,
												PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
												&val);
				val &= ~PRV_MPD_ADVERTISE_1000_T_MASK_CNS;
				val &= ~PRV_MPD_1000BASE_AN_MANUALCONFIG_CNS; /* unset this bit - auto configuration */
				val &= ~PRV_MPD_1000BASE_AN_PREFERMASTER_CNS; /* unset bit - master-slave preference */
				val |= capabilities2;

				status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													phyPage,
													PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													val);
			}

			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phyPage,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&val);
			/* Reset, reset auto neg and no power down*/
			val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* reset*/
			val |= PRV_MPD_CTRLREG_AUTO_NEG_CNS; /* enable and restart auto-neg */
			val &= ~PRV_MPD_CTRLREG_POWER_DOWN_CNS; /* no power down*/
			do_errata = TRUE;

			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												phyPage,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);

			break;

		case MPD_AUTO_NEGOTIATION_DISABLE_E:
			/* if in fiber mode - don't disable auto negotiation in PHY */
			if (phyPage == PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS) {
				return MPD_OK_E;
			}
			/* Set the control register according to the specific status register. */
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phyPage,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&control_reg_val);

			/* Set auto_neg and restart_auto_neg bits */
			control_reg_val &= ~PRV_MPD_CTRLREG_AUTO_NEG_CNS;

			/* This PHY requires reset after setting a new value */
			control_reg_val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
			do_errata = TRUE;

			/* Write back control reg */
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												phyPage,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												control_reg_val);

			break;

		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_NOT_SUPPORTED_E;
	}

	if (do_errata == TRUE)
	{
		/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
		phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
		status = prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ERRATA_E,
											&phy_params);
		if (status == MPD_OP_FAILED_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                            portEntry_PTR->rel_ifIndex);
            return MPD_OP_FAILED_E;
		}
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetAutoNegotiationRemoteCapabilities_5
 *
 * DESCRIPTION: Get auto negotiation remote capabilities
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1548_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetAutoNegotiationRemoteCapabilities_5(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	GT_U16 val, reg5, reg10;
	GT_U8 phyPage;
	params_PTR->phyRemoteAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;

	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
			(portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
			(portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
		phyPage = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
	}
	else {
		phyPage = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
	}

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									phyPage,
									PRV_MPD_CTRLREG1_OFFSET_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (val & 0x0020) { /* Auto-Negotiation process complete */

		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phyPage,
										PRV_MPD_LINK_PARTNER_ABILITY_REG_CNS,
										&reg5);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
				(portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
				(portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {

			params_PTR->phyRemoteAutoneg.capabilities = 0;

			if (reg5 & PRV_MPD_LINK_PARTNER_FIBER_ABILITY_1000H_CNS) {
				params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS;
			}
			if (reg5 & PRV_MPD_LINK_PARTNER_FIBER_ABILITY_1000F_CNS) {
				params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
			}
		}
		else {

			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phyPage,
											6,
											&val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

			if (val & 0x0001) { /* Link Partner is Auto-Negotiation able */

				status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												phyPage,
												PRV_MPD_1000_BASE_T_STATUS_REG_CNS,
												&reg10);
				PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

				params_PTR->phyRemoteAutoneg.capabilities = 0;

				if (reg5 & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100F_CNS) {
					params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
				}
				if (reg5 & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100H_CNS) {
					params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
				}
				if (reg5 & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10F_CNS) {
					params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
				}
				if (reg5 & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10H_CNS) {
					params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
				}
				if (reg10 & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_1000F_CNS) {
					params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
				}
				if (reg10 & PRV_MPD_LINK_PARTNER_COPPER_ABILITY_1000H_CNS) {
					params_PTR->phyRemoteAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS;
				}
			}
		}
		return MPD_OK_E;
	}
	return MPD_NOT_SUPPORTED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdAdvertiseFc_5
 *
 * DESCRIPTION: Advertise flow control capability
 *
 * APPLICABLE PHYS:
 * 					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1548_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdAdvertiseFc_5(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U8 phyPage;
	GT_U16 val;
	UINT_32 first_page = PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
			last_page = PRV_MPD_PAGE_0_COPPER_CONTROL_CNS;
	MPD_RESULT_ENT status;

	switch (portEntry_PTR->initData_PTR->transceiverType) {
		case MPD_TRANSCEIVER_COPPER_E:
			break;
		case MPD_TRANSCEIVER_SFP_E:
			first_page = PRV_MPD_PAGE_1_FIBER_CONTROL_CNS;
			/* fall through */
			MPD_FALLTHROUGH
		case MPD_TRANSCEIVER_COMBO_E:
			last_page = PRV_MPD_PAGE_1_FIBER_CONTROL_CNS;
			break;
        case MPD_TRANSCEIVER_MAX_TYPES_E:
        default:
            return MPD_OP_FAILED_E;
            break;
	}
	for (phyPage = first_page; phyPage <= last_page; phyPage++) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"pause bit is: %d ifIndex: %d",
														params_PTR->phyFc.advertiseFc,
														portEntry_PTR->rel_ifIndex);

		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phyPage,
										PRV_MPD_AUTNEGADVER_OFFSET_CNS,
										&val);

		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		/* fiber page */
		if (phyPage == PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS) {
			if (params_PTR->phyFc.advertiseFc == TRUE) {
				if ((val & 0x0180) == 0x0080) {
					continue;
				}
				val &= 0xFEFF;
				val |= 0x0080;
			}
			else {
				if (!(val & 0x0180)) {
					continue;
				}
				val &= 0xFE7F;
			}
		}
		else { /* copper page */
			if (params_PTR->phyFc.advertiseFc == TRUE) {
				if (val & 0x0400) {
					continue;
				}
				val |= 0x0400;
			}
			else {
				if (!(val & 0x0400)) {
					continue;
				}
				val &= 0xFBFF;
			}
		}

		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phyPage,
											PRV_MPD_AUTNEGADVER_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}

	status = prvMpdPerformPhyOperation(	portEntry_PTR,
										MPD_OP_CODE_SET_RESET_PHY_E,
										params_PTR);
	return status;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetDuplexMode_5
 *
 * DESCRIPTION: Set port duplex mode (HALF, FULL)
 *
 * APPLICABLE PHYS:
 * 					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1548_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetDuplexMode_5(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	GT_U16 val;
	GT_U8 phyPage;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
			(portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
			(portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
		phyPage = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
	}
	else {
		phyPage = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
	}

	/* Read the cntrl reg. Will be in use inside the switch. */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									phyPage,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	switch (params_PTR->phyDuplex.mode) {
		case MPD_DUPLEX_ADMIN_MODE_FULL_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set full duplex, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			/* check if the phy is already in full duplex mode */
			if (val & PRV_MPD_CTRLREG_FULL_DPLX_CNS) {

				return MPD_OK_E;
			}
			val |= PRV_MPD_CTRLREG_FULL_DPLX_CNS;
			break;
		case MPD_DUPLEX_ADMIN_MODE_HALF_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set half duplex, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			/* check if the phy is already in half duplex mode */
			if (!(val & PRV_MPD_CTRLREG_FULL_DPLX_CNS)) {

				return MPD_OK_E;
			}
			val &= ~PRV_MPD_CTRLREG_FULL_DPLX_CNS;
			break;
		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_OP_FAILED_E;
	}
	/* This PHY requires reset after setting a new value */
	val = val | PRV_MPD_CTRLREG_RESET_BIT_CNS;
	/* Force the duplex mode */
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										(GT_U16) val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
	phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
	status = prvMpdPerformPhyOperation(	portEntry_PTR,
										PRV_MPD_OP_CODE_SET_ERRATA_E,
										&phy_params);
	if (status == MPD_OP_FAILED_E) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                        portEntry_PTR->rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetSpeed_6
 *
 * DESCRIPTION: Set port Speed
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdSetSpeed_6(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status = MPD_OK_E;
	GT_U16 cntrl_val;
	MPD_OPERATIONS_PARAMS_UNT combo_params;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
			(portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
			(portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {

		memset(	&combo_params, 0, sizeof(combo_params));
		combo_params.internal.phyFiberParams.fiberPresent = TRUE;
		combo_params.internal.phyFiberParams.comboMode = MPD_COMBO_MODE_PREFER_FIBER_E;
		combo_params.internal.phyFiberParams.phySpeed = params_PTR->phySpeed.speed;

		return prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
											&combo_params);
	}
	/* Read the current value of the control register */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&cntrl_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	switch (params_PTR->phySpeed.speed) {
		case MPD_SPEED_10M_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set 10M, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			cntrl_val &= ~PRV_MPD_CTRLREG_100BT_CNS;
			cntrl_val &= ~PRV_MPD_CTRLREG_SPEED_1000BT_CNS;
			break;

		case MPD_SPEED_100M_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set 100M, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			cntrl_val |= PRV_MPD_CTRLREG_100BT_CNS;
			cntrl_val &= ~PRV_MPD_CTRLREG_SPEED_1000BT_CNS;
			break;

		case MPD_SPEED_1000M_E:
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"set 1000M, portEntry_PTR->rel_ifIndex: %d",
															portEntry_PTR->rel_ifIndex);

			cntrl_val &= ~PRV_MPD_CTRLREG_100BT_CNS;
			cntrl_val |= PRV_MPD_CTRLREG_SPEED_1000BT_CNS;
			break;

		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_NOT_SUPPORTED_E;
	}

	/* This PHY requires reset after setting a new value */
	cntrl_val = cntrl_val | PRV_MPD_CTRLREG_RESET_BIT_CNS;
	/* force the requested speed */
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										(GT_U16) cntrl_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
	phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
	status = prvMpdPerformPhyOperation(	portEntry_PTR,
										PRV_MPD_OP_CODE_SET_ERRATA_E,
										&phy_params);
	if (status == MPD_OP_FAILED_E) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                        portEntry_PTR->rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpd1540BypassSet
 *
 * DESCRIPTION: Utility, Enable or disable bypass of mac on PHY
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpd1540BypassSet(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	BOOLEAN bypass /* TRUE - bypass mode, FALSE - mac on PHY mode */
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	GT_STATUS rc;
	UINT_16 value;
	UINT_32 data = 0;

	if (portEntry_PTR->runningData_PTR->macOnPhyState == bypass) {
		return MPD_OK_E;
	}

	portEntry_PTR->runningData_PTR->macOnPhyState = bypass;

	if (bypass == TRUE) {
		/* force mac link down */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x47,
								&data);
		data &= ~(1 << 6);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x47,
								data);

		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x57,
								&data);
		data &= ~(1 << 6);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x57,
								data);

		/* disable auto-neg in mac */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x43,
								&data);
		data &= ~(1 << 5);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x43,
								data);

		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x53,
								&data);
		data &= ~(1 << 5);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x53,
								data);

		/* reset IGR and EGR pre/post/fifo */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x70,
								&data);
		data |= 0xB980; /* set bits 7,8,11,12,13,15 = 1 */
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x70,
								data);

		/* reset IGR and EGR path */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x2,
								&data);
		data |= 0x6; /* set bits 1,2 = 1*/
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x2,
								data);
	}
	/* set page to 18 */
	rc = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
							0x0012);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);
	rc = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							27,
							&value);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);
	if (bypass) {
		value &= ~PRV_MPD_1540_BYPASS_DISABLE_BIT_CNS;
	}
	else {
		value |= PRV_MPD_1540_BYPASS_DISABLE_BIT_CNS;
	}
	rc = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							27,
							value);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);
	/* return to page 0*/
	rc = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
							0);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

	if (bypass != TRUE) {
		/* clear IGR and EGR pre/post/fifo */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x70,
								&data);
		data &= ~(0xB980); /* set bits 7,8,11,12,13,15 = 0 */
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x70,
								data);

		/* clear IGR and EGR path */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x2,
								&data);
		data &= ~(0x6); /* set bits 1,2 = 0*/
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x2,
								data);

		/* enable auto-neg in mac */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x43,
								&data);
		data |= (1 << 5);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x43,
								data);

		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x53,
								&data);
		data |= (1 << 5);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x53,
								data);

		/* unset force mac link down */
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x47,
								&data);
		data |= (1 << 6);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x47,
								data);

		prvMpdReadLinkCryptReg(	portEntry_PTR,
								0x57,
								&data);
		data |= (1 << 6);
		prvMpdWriteLinkCryptReg(portEntry_PTR,
								0x57,
								data);
	}

	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetAutoNegotiationSupport_1
 *
 * DESCRIPTION: Get auto negotiation supported modes
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1548_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetAutoNegotiationSupport_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->PhyAutonegCapabilities.capabilities = 	MPD_AUTONEG_CAPABILITIES_TENHALF_CNS |
														MPD_AUTONEG_CAPABILITIES_TENFULL_CNS |
														MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS |
														MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS |
														MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetAutoNegotiationSupport_2
 *
 * DESCRIPTION: Get auto negotiation supported modes
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetAutoNegotiationSupport_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->PhyAutonegCapabilities.capabilities = 	MPD_AUTONEG_CAPABILITIES_TENHALF_CNS |
														MPD_AUTONEG_CAPABILITIES_TENFULL_CNS |
														MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS |
														MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetAutoNegotiationAdmin_1
 *
 * DESCRIPTION: Get auto negotiation configuration
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1548_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetAutoNegotiationAdmin_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16			copperCtrl,copperAnAdv, gigCtrl;
	MPD_RESULT_ENT 	status;

	params_PTR->phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
	params_PTR->phyAutoneg.masterSlave 	= MPD_AUTONEGPREFERENCE_UNKNOWN_E;
	params_PTR->phyAutoneg.enable 		= FALSE;

	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
		 (portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
		 (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
		return MPD_NOT_SUPPORTED_E;
	}
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&copperCtrl);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_AUTNEGADVER_OFFSET_CNS,
									&copperAnAdv);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
									&gigCtrl);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	params_PTR->phyAutoneg.enable = (copperCtrl & PRV_MPD_CTRLREG_EN_AUTO_NEG_CNS)?TRUE:FALSE;

	if (params_PTR->phyAutoneg.enable == FALSE) {
		return MPD_OK_E;
	}

	params_PTR->phyAutoneg.masterSlave = (gigCtrl & PRV_MPD_1000BASE_AN_PREFERMASTER_CNS)?MPD_AUTONEGPREFERENCE_MASTER_E:MPD_AUTONEGPREFERENCE_SLAVE_E;
	params_PTR->phyAutoneg.capabilities = 0;

	if (copperAnAdv & PRV_MPD_ADVERTISE_10_T_HALF_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
	}
	if (copperAnAdv & PRV_MPD_ADVERTISE_10_T_FULL_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
	}
	if (copperAnAdv & PRV_MPD_ADVERTISE_100_T_HALF_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
	}
	if (copperAnAdv & PRV_MPD_ADVERTISE_100_T_FULL_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
	}
	if (gigCtrl & PRV_MPD_ADVERTISE_1000_T_HALF_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS;
	}
	if (gigCtrl & PRV_MPD_ADVERTISE_1000_T_FULL_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetAutoNegotiationAdmin_2
 *
 * DESCRIPTION: Get auto negotiation configuration
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetAutoNegotiationAdmin_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16			copperCtrl,copperAnAdv;
	MPD_RESULT_ENT 	status;
	params_PTR->phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
	params_PTR->phyAutoneg.masterSlave 	= MPD_AUTONEGPREFERENCE_UNKNOWN_E;
	params_PTR->phyAutoneg.enable 		= FALSE;

	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
		 (portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
		 (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
		return MPD_NOT_SUPPORTED_E;
	}
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&copperCtrl);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_AUTNEGADVER_OFFSET_CNS,
									&copperAnAdv);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);


	params_PTR->phyAutoneg.enable = (copperCtrl & PRV_MPD_CTRLREG_EN_AUTO_NEG_CNS)?TRUE:FALSE;

	if (params_PTR->phyAutoneg.enable == FALSE) {
		return MPD_OK_E;
	}

	params_PTR->phyAutoneg.capabilities = 0;

	if (copperAnAdv & PRV_MPD_ADVERTISE_10_T_HALF_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
	}
	if (copperAnAdv & PRV_MPD_ADVERTISE_10_T_FULL_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
	}
	if (copperAnAdv & PRV_MPD_ADVERTISE_100_T_HALF_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
	}
	if (copperAnAdv & PRV_MPD_ADVERTISE_100_T_FULL_CNS) {
		params_PTR->phyAutoneg.capabilities |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetAutoNegotiation_7
 *
 * DESCRIPTION: Set auto negotiation configuration
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetAutoNegotiation_7(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	BOOLEAN in_req_autoneg, do_errata = FALSE;
	MPD_RESULT_ENT status;
	GT_U16 val = 0, val2;
	GT_U16 capabilities1 = 0;
	GT_U16 capabilities2 = 0;
	GT_U16 control_reg_val, pause_advertise_bit = 0;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	switch (params_PTR->phyAutoneg.enable) {

		case MPD_AUTO_NEGOTIATION_ENABLE_E:

			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_10_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_10_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_100_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
				capabilities1 |= PRV_MPD_ADVERTISE_100_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS) {
				capabilities2 |= PRV_MPD_ADVERTISE_1000_T_HALF_CNS;
			}
			if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS) {
				capabilities2 |= PRV_MPD_ADVERTISE_1000_T_FULL_CNS;
			}
			if (params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) {
				capabilities2 |= PRV_MPD_1000BASE_AN_PREFERMASTER_CNS; /* set bit - prefer master */
			}

			in_req_autoneg = prvMpdCheckIfPhyIsInReqAutoneg(portEntry_PTR,
															MPD_AUTO_NEGOTIATION_ENABLE_E,
															PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS);
			if (in_req_autoneg) {
				/* Already in the requested auto neg mode. So there is no need to change it again */
				/* check if we had advertised all */
				status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_AUTNEGADVER_OFFSET_CNS,
												&val);

				status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
												&val2);

				if (((val & PRV_MPD_ADVERTISE_10_100_CNS) == capabilities1) && ((val2 & PRV_MPD_1000BASE_AN_PREFER_MASK_CNS) == capabilities2))
					return MPD_OK_E;
			}

			if (val & 0x0400) {
				pause_advertise_bit = 0x0400;
			}
			/* First set advertise bits */
			val = capabilities1 | 0x0001 | pause_advertise_bit;
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_AUTNEGADVER_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);

			/* set advertise_1000 bits */
			/* read val before writing back*/
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
											PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
											&val);
			val &= ~PRV_MPD_ADVERTISE_1000_T_MASK_CNS;
			val &= ~PRV_MPD_1000BASE_AN_MANUALCONFIG_CNS; /* unset this bit - auto configuration */
			val &= ~PRV_MPD_1000BASE_AN_PREFERMASTER_CNS; /* unset bit - master-slave preference */
			val |= capabilities2;

			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);

			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&val);

			/* Reset, reset auto neg and no power down*/
			if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) {
				val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* reset*/
				val &= ~PRV_MPD_CTRLREG_POWER_DOWN_CNS; /* no power down*/
			}

			val |= PRV_MPD_CTRLREG_AUTO_NEG_CNS; /* enable and restart auto-neg */
			do_errata = TRUE;

			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												val);

			break;

		case MPD_AUTO_NEGOTIATION_DISABLE_E:
			/* Set the control register according to the specific status register. */
			status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											&control_reg_val);

			/* Set auto_neg and restart_auto_neg bits */
			control_reg_val &= ~PRV_MPD_CTRLREG_AUTO_NEG_CNS;

			/* This PHY requires reset after setting a new value */
			if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) {
				val |= PRV_MPD_CTRLREG_RESET_BIT_CNS; /* reset*/
				val &= ~PRV_MPD_CTRLREG_POWER_DOWN_CNS; /* no power down*/
			}
			do_errata = TRUE;

			/* Write back control reg */
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												control_reg_val);

			break;

		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_NOT_SUPPORTED_E;
	}

	if (do_errata == TRUE)
	{
		/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
		phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
		status = prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ERRATA_E,
											&phy_params);
		if (status == MPD_OP_FAILED_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                            portEntry_PTR->rel_ifIndex);
			return MPD_OP_FAILED_E;
		}
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdRestartAutoNegotiation_1
 *
 * DESCRIPTION: Restart auto negotiation
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdRestartAutoNegotiation_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 			control_reg_val;
	MPD_RESULT_ENT	status;
	MPD_UNUSED_PARAM(params_PTR);

	if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
		 (portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
		 (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
		return MPD_NOT_SUPPORTED_E;
	}

	/* Read the control register  */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&control_reg_val);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* if auto_neg is disabled - nothing to do */
	if ((control_reg_val & PRV_MPD_CTRLREG_EN_AUTO_NEG_CNS) == 0) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"Auto Negotiation is disabled - nothing to do");
		return MPD_NOT_SUPPORTED_E;
	}
	/* if port is admin down - nothing to do */
	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"Port is admin down - nothing to do");
		return MPD_NOT_SUPPORTED_E;

	}

	/* restart auto nogotiation */
	control_reg_val |= PRV_MPD_CTRLREG_RESATRT_AUTO_NEG_CNS;


	/* Write back control reg */
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										control_reg_val);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetEeeAdvertize
 *
 * DESCRIPTION: Set Enable/Disable EEE advertizement
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetEeeAdvertize(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_8 device = 7;
	UINT_16 data = 0;
	UINT_32 i;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	UINT_32 array_size = sizeof(prvMpdConvertIfSpeedCapabToEeePhyBit_ARR) / sizeof(PRV_MPD_CONVERT_IF_SPEED_TO_PHY_TYPE_STC);

	if (params_PTR->phyEeeAdvertize.advEnable == TRUE) {
		for (i = 0; i < array_size; i++) {
			if (params_PTR->phyEeeAdvertize.speedBitmap & prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[i].ifSpeedCapabBit) {
				data |= prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[i].phyTypeBit;
			}
		}
	}

	/* advertise */
	prvMpdXmdioWriteReg(portEntry_PTR,
						device,
						0x3C,
						data);

	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) {
		/* requires SW reset */
		prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
								PRV_MPD_CTRLREG_OFFSET_CNS,
								&data);
		data |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
		prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
								PRV_MPD_CTRLREG_OFFSET_CNS,
								PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								data);
	}

	/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
	phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
	if (prvMpdPerformPhyOperation(	portEntry_PTR,
									PRV_MPD_OP_CODE_SET_ERRATA_E,
									&phy_params) == MPD_OP_FAILED_E) {
		return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetEeeAdvertize
 *
 * DESCRIPTION: Get EEE remote status
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetEeeRemStatus(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 data, device = 7;
	UINT_32 i;
	UINT_32 array_size = sizeof(prvMpdConvertIfSpeedCapabToEeePhyBit_ARR) / sizeof(PRV_MPD_CONVERT_IF_SPEED_TO_PHY_TYPE_STC);

	prvMpdXmdioReadReg(	portEntry_PTR,
						(UINT_8) device,
						0x3D,
						&data);
	/* Convert the bit map of the specific phy to generic bit map */
	params_PTR->phyEeeCapabilities.enableBitmap = 0;

	for (i = 0; i < array_size; i++) {
		if (prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[i].phyTypeBit & data) {
			params_PTR->phyEeeCapabilities.enableBitmap |= prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[i].ifSpeedCapabBit;
		}
	}
#ifdef PHY_SIMULATION
    params_PTR->phyEeeCapabilities.enableBitmap = 0xff;
#endif
	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetEeeAdvertize
 *
 * DESCRIPTION: Get EEE local capability
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetEeeCapability(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_8 device = 3;
	UINT_16 data;
	UINT_32 i;
	UINT_32 array_size = sizeof(prvMpdConvertIfSpeedCapabToEeePhyBit_ARR) / sizeof(PRV_MPD_CONVERT_IF_SPEED_TO_PHY_TYPE_STC);
	prvMpdXmdioReadReg(	portEntry_PTR,
						device,
						0x14,
						&data);

	/* Convert the bit map of the specific phy to generic bit map */
	params_PTR->phyEeeCapabilities.enableBitmap = 0;

	if (data) {
		for (i = 0; i < array_size; i++) {
			if (prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[i].phyTypeBit & data) {
				params_PTR->phyEeeCapabilities.enableBitmap |= prvMpdConvertIfSpeedCapabToEeePhyBit_ARR[i].ifSpeedCapabBit;
			}
		}
	}

#ifdef PHY_SIMULATION
    /* return support for 100M and 1G */
    params_PTR->phyEeeCapabilities.enableBitmap = 0x03;
#endif
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetPowerModules_3
 *
 * DESCRIPTION: Set Green (SR/ED) configuration
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetPowerModules_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status = MPD_OK_E;
	GT_U16 val = 0, tmp, control_reg_val_0, control_reg_val_2;
	BOOLEAN set_sr = FALSE;
	UINT_32 synch_time_in_ticks;
	MPD_OPERATIONS_PARAMS_UNT reset_params;
	synch_time_in_ticks = (prvMpdGreenWaSleepMs);

	/*ED treatment*/
	if (params_PTR->phyPowerModules.energyDetetct != MPD_GREEN_NO_SET_E) {

		status |= prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
											&control_reg_val_0);

		status |= prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											2,
											PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
											&control_reg_val_2);
		if (params_PTR->phyPowerModules.energyDetetct == MPD_GREEN_ENABLE_E) {
			tmp = control_reg_val_0 & PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS;
			/* write register 16 page 0 - ED */
			if (tmp != PRV_MPD_88E1240_ENERGY_DETECT_ON_CNS) {
				control_reg_val_0 &= ~PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS;
				control_reg_val_0 |= PRV_MPD_88E1240_ENERGY_DETECT_ON_CNS;
				status |= prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													0,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													control_reg_val_0);
			}

			tmp = control_reg_val_2 & PRV_MPD_88E1240_MAC_INTERFACE_POW_MASK_CNS;
			/* write register 16 page 2 - MAC power down */
			if (tmp != PRV_MPD_88E1240_MAC_INTERFACE_POW_ON_CNS) {
				control_reg_val_2 &= ~PRV_MPD_88E1240_MAC_INTERFACE_POW_MASK_CNS;
				control_reg_val_2 |= PRV_MPD_88E1240_MAC_INTERFACE_POW_ON_CNS;
				status |= prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													2,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													control_reg_val_2);
			}
		} /* enable ED */
		/* disable */
		else if (params_PTR->phyPowerModules.energyDetetct == MPD_GREEN_DISABLE_E) {
			tmp = control_reg_val_0 & PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS;
			/* write register 16 page 0 */
			if (tmp != PRV_MPD_88E1240_ENERGY_DETECT_OFF_CNS) {
				control_reg_val_0 &= ~PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS;
				control_reg_val_0 |= PRV_MPD_88E1240_ENERGY_DETECT_OFF_CNS;
				status |= prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													0,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													control_reg_val_0);

			}

			tmp = control_reg_val_2 & PRV_MPD_88E1240_MAC_INTERFACE_POW_MASK_CNS;
			/* write register 16 page 2 */
			if (tmp != PRV_MPD_88E1240_MAC_INTERFACE_POW_OFF_CNS) {
				control_reg_val_2 &= ~PRV_MPD_88E1240_MAC_INTERFACE_POW_MASK_CNS;
				control_reg_val_2 |= PRV_MPD_88E1240_MAC_INTERFACE_POW_OFF_CNS;

				status |= prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													portEntry_PTR,
													2,
													PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
													PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
													control_reg_val_2);
			}
		} /* disable ED */
	} /* ED */
	/*SR treatment*/
	if (params_PTR->phyPowerModules.shortReach != MPD_GREEN_NO_SET_E)
	{

		/* Move to page 0xFC */
		status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
									0x00FC);
		status |= prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									2,
									&val);

		if (params_PTR->phyPowerModules.shortReach == MPD_GREEN_ENABLE_E && ((val & 0x100) == FALSE))
				{
			val |= 0x100;
			set_sr = TRUE;
		}
		else if (params_PTR->phyPowerModules.shortReach == MPD_GREEN_DISABLE_E && ((val & 0x100) != FALSE))
				{
			val &= ~0x100;
			set_sr = TRUE;
		}

		if (set_sr == TRUE)
		{
			status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										2,
										val);
		}

		/* Return to page 0 */
		status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
									0);
	} /* SR */

	if (params_PTR->phyPowerModules.performPhyReset == TRUE)
	{
		memset(	&reset_params,
				0,
				sizeof(reset_params));
		status |= prvMpdPerformPhyOperation(portEntry_PTR,
											MPD_OP_CODE_SET_RESET_PHY_E,
											&reset_params);
	}

	/* wait for other side to synch */
	PRV_MPD_SLEEP_MAC(synch_time_in_ticks);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDisableOperation_2
 *
 * DESCRIPTION: 	Adminstrative shutdown of PHY
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdDisableOperation_11(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 read_val, shutdown_val, reset_val = 0;
	UINT_8 phy_page, first_page, last_page;
	BOOLEAN enable = TRUE;
	MPD_RESULT_ENT status;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	switch (portEntry_PTR->initData_PTR->transceiverType) {
		case MPD_TRANSCEIVER_COPPER_E:
			first_page = last_page = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
			break;
		case MPD_TRANSCEIVER_COMBO_E:
			first_page = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
			last_page = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
			break;
		case MPD_TRANSCEIVER_SFP_E:
			first_page = last_page = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
			break;
		default:
			return MPD_OP_FAILED_E;
	}
	for (phy_page = first_page; phy_page <= last_page; phy_page++) {
		read_val = shutdown_val = reset_val = 0;
		/* prepare power down */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phy_page,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&read_val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		switch (params_PTR->phyDisable.forceLinkDown) {
			case TRUE: /*perform disable operation*/
				shutdown_val = read_val | PRV_MPD_LINK_DOWN_CNS;
				/* reset without shut down */
				reset_val = read_val | 0x8000;
				enable = FALSE;
				break;
			case FALSE: /*enable the port*/
				shutdown_val = read_val & PRV_MPD_LINK_UP_CNS;
				/* reset without shut down */
				reset_val = shutdown_val | 0x8000;
				enable = TRUE;
				break;
			default:
				shutdown_val = read_val & PRV_MPD_LINK_UP_CNS;
				/* reset without shut down */
				reset_val = shutdown_val | 0x8000;
				enable = TRUE;
				break;
		}
		/* perform reset before shutting down.
		 ** save shutdown value that will be written to PHY
		 */
		/* do phy reset - for power down to take effect - gain wa
		 release notes alaska 88E1545/88E1543 section 3.3 */

		/* do reset for power down before power down */
		if (params_PTR->phyDisable.forceLinkDown) {
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												phy_page,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												reset_val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		}
		/* set power down */
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phy_page,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											shutdown_val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	/* reset for power up to take effect */
	if (params_PTR->phyDisable.forceLinkDown == FALSE) {
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&read_val);

		/* do reset */
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											(read_val | PRV_MPD_CTRLREG_RESET_BIT_CNS));
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
		phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
		status = prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ERRATA_E,
											&phy_params);
		if (status == MPD_OP_FAILED_E) {
			return MPD_OP_FAILED_E;
		}
	}

	PRV_MPD_TRANSCEIVER_ENABLE_MAC(	portEntry_PTR->rel_ifIndex,
									enable);

	return MPD_OK_E;

}

/* *************************************************************************
 * FUNCTION: prvMpdChangeGigPhyLedOperation
 *
 * GENERAL DESCRIPTION: Utility, Change the led operation of the phy
 *
 * REMARKS:     can be used for 1G PHYs
 ***************************************************************************
 **/
MPD_RESULT_ENT prvMpdChangeGigPhyLedOperation(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	MPD_GIG_PHY_LED_OPERATION_ENT ledOperation
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	GT_U16 read_val, write_val = 0;
	MPD_RESULT_ENT status;
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_88E151X_PAGE_LED_CTRL_CNS,
									PRV_MPD_88E151X_LED_CTRL_REG_CNS,
									&read_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	switch (ledOperation) {
		case MPD_GIG_PHY_LED_ON_E:
			write_val = 0x17;
			break;
		case MPD_GIG_PHY_LED_OFF_E:
			write_val = 0x88;
			break;
		default:
			return MPD_NOT_SUPPORTED_E;
	}

	read_val &= 0xFF00;
	write_val |= read_val;
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_88E151X_PAGE_LED_CTRL_CNS,
										PRV_MPD_88E151X_LED_CTRL_REG_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										write_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDisableOperation_16
 *
 * DESCRIPTION: 	Adminstrative shutdown of PHY
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151X_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdDisableOperation_16(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 read_val, shutdown_val, reset_val = 0;
	UINT_8 phy_page, first_page, last_page;
	BOOLEAN enable = TRUE;
	MPD_RESULT_ENT status;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	switch (portEntry_PTR->initData_PTR->transceiverType) {
		case MPD_TRANSCEIVER_COPPER_E:
			first_page = last_page = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
			break;
		case MPD_TRANSCEIVER_COMBO_E:
			first_page = PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS;
			last_page = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
			break;
		case MPD_TRANSCEIVER_SFP_E:
			first_page = last_page = PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS;
			break;
		default:
			return MPD_OP_FAILED_E;
	}
	for (phy_page = first_page; phy_page <= last_page; phy_page++) {
		read_val = shutdown_val = reset_val = 0;
		/* prepare power down */
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										phy_page,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&read_val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		switch (params_PTR->phyDisable.forceLinkDown) {
			case TRUE: /*perform disable operation*/
				shutdown_val = read_val | PRV_MPD_LINK_DOWN_CNS;
				/* reset without shut down */
				reset_val = read_val | 0x8000;
				enable = FALSE;
				break;
			case FALSE: /*enable the port*/
				shutdown_val = read_val & PRV_MPD_LINK_UP_CNS;
				/* reset without shut down */
				reset_val = shutdown_val | 0x8000;
				enable = TRUE;
				break;
			default:
				shutdown_val = read_val & PRV_MPD_LINK_UP_CNS;
				/* reset without shut down */
				reset_val = shutdown_val | 0x8000;
				enable = TRUE;
				break;
		}
		/* perform reset before shutting down.
		 ** save shutdown value that will be written to PHY
		 */
		/* do phy reset - for power down to take effect - gain wa
		 release notes alaska 88E1545/88E1543 section 3.3 */

		/* do reset for power down before power down */
		if (params_PTR->phyDisable.forceLinkDown) {
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												phy_page,
												PRV_MPD_CTRLREG_OFFSET_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												reset_val);
			PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		}

		if ((params_PTR->phyDisable.forceLinkDown == FALSE) && (phy_page == last_page)) {
			prvMpdChangeGigPhyLedOperation(	portEntry_PTR,
											MPD_GIG_PHY_LED_ON_E);
		}

		/* set power down */
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											phy_page,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											shutdown_val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		if ((params_PTR->phyDisable.forceLinkDown == TRUE) && (phy_page == last_page)) {
			prvMpdChangeGigPhyLedOperation(	portEntry_PTR,
											MPD_GIG_PHY_LED_OFF_E);
		}
	}
	/* reset for power up to take effect */
	if (params_PTR->phyDisable.forceLinkDown == FALSE) {
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										0,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										&read_val);

		/* do reset */
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											0,
											PRV_MPD_CTRLREG_OFFSET_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											(read_val | PRV_MPD_CTRLREG_RESET_BIT_CNS));
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		/* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
		phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
		status = prvMpdPerformPhyOperation(	portEntry_PTR,
											PRV_MPD_OP_CODE_SET_ERRATA_E,
											&phy_params);
		if (status == MPD_OP_FAILED_E) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                            portEntry_PTR->rel_ifIndex);
			return MPD_OP_FAILED_E;
		}
	}

	PRV_MPD_TRANSCEIVER_ENABLE_MAC(	portEntry_PTR->rel_ifIndex,
									enable);

	return MPD_OK_E;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdEeeIpg_1
 *
 * DESCRIPTION: 	Utility,
 * 				Change default IPG of EEE buffer control reg 1 (from 6) to 0xC.
 * 				When the IPG is too short, it cause link flapping in high speed traffic,
 *               when EEE is enabled on the link.
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdEeeIpg_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	GT_STATUS rc;
	UINT_16 val;
	/* move to page 18 */
	rc = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
							18);
	/* read reg 0 - EEE buffer control reg 1 */
	rc |= prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							0,
							&val);
	val &= 0xff;
	val |= 0xc00;
	/* write 0xc to IPG bits (8-15) */
	rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							0,
							val);
	/* move back to page 0 */
	rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
							0);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetComboMediaType_4
 *
 * DESCRIPTION: 	Set active media type on combo port
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1543_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetComboMediaType_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 mode = PRV_MPD_MODE_SGMII_TO_AUTO_MEDIA_COPPER_1000_BASEX_CNS | PRV_MPD_CTRLREG_AMD_COPPER_100FX_CNS; /* mode[0-2]*/
	UINT_16 val;
	MPD_RESULT_ENT status;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	/* Set PHY Mode */
	if (params_PTR->internal.phyFiberParams.fiberPresent == FALSE) {
		/* QSGMII to Auto Media Detect 1000Base-X */
		mode = PRV_MPD_MODE_SGMII_TO_AUTO_MEDIA_COPPER_1000_BASEX_CNS;

		/* Enable auto detect coper / 100BASEFX*/
		mode |= PRV_MPD_CTRLREG_AMD_COPPER_100FX_CNS;

		/* Update BOX about set combo media type to copper
		 * todo - move to core
		 BOXG_port_action(portEntry_PTR->rel_ifIndex, BOXG_port_ledControlMode_CNS, BOXG_port_ledControl_setComboMediaType_copper_CNS);
		 */
		/* Change page to 0. */
		status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
									PRV_MPD_PAGE_0_COPPER_CONTROL_CNS);
	}
	else {
		/* get port speed */
		/* only in case of sfd - port type is actual port speed */
		if (params_PTR->internal.phyFiberParams.phySpeed == MPD_SPEED_100M_E) {
			/* Configure as 100 sfp - QSGMII (system mode) to 100Base-FX */
			mode = PRV_MPD_MODE_SGMII_TO_100_FX_CNS;
			/* Enable auto detect coper / 100BASEFX*/
			mode |= PRV_MPD_CTRLREG_AMD_COPPER_100FX_CNS;

			/* Set duplex to full on fiber page */
			status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												portEntry_PTR,
												PRV_MPD_PAGE_1_FIBER_CONTROL_CNS,
												PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS,
												PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
												(0xA100 | PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS));
		}
		else {
			/* Configure as 1G sfp - QSGMII (system mode) to Auto Media Detect 1000Base-X */
			mode = PRV_MPD_MODE_SGMII_TO_AUTO_MEDIA_COPPER_1000_BASEX_CNS;
			/* Enable auto detect coper / 100BASEFX*/
			mode |= PRV_MPD_CTRLREG_AMD_COPPER_100FX_CNS;
		}

		/* Update BOX about set combo media type
		 * todo - move to core
		 BOXG_port_action(portEntry_PTR->rel_ifIndex, BOXG_port_ledControlMode_CNS, BOXG_port_ledControl_setComboMediaType_fiber_CNS);
		 */
		/* Change page to 1. */
		status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
									PRV_MPD_PAGE_1_FIBER_CONTROL_CNS);
	}

	val = 0;
	/* Get value of the general control register */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
									PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* Zero mode field */
	val &= ~PRV_MPD_MODE_MASK_CNS;

	/* Set Auto-Media Detect Preferred Media */
	switch (params_PTR->internal.phyFiberParams.comboMode) {
		case MPD_COMBO_MODE_FORCE_COPPER_E:
			case MPD_COMBO_MODE_PREFER_COPPER_E:
			/* set phy to prefer copper */
			val |= (1 << 4);
			val &= ~(1 << 5);
			break;
		case MPD_COMBO_MODE_FORCE_FIBER_E:
			case MPD_COMBO_MODE_PREFER_FIBER_E:
			/* set phy to prefer fiber*/
			val &= ~(1 << 4);
			val |= (1 << 5);
			break;
		default:
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_ERROR_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			return MPD_NOT_SUPPORTED_E;
	}

	/* Reset phy mode */
	val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
	val |= mode;
	/* Set prefer fiber/copper*/
	/* 1543 support auto media select, so this should be enough.*/
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
										PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);
	/* Always reset phy  - alert link partner */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
									PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS,
									&val);
	val |= PRV_MPD_CTRLREG_RESET_BIT_CNS;
	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
										PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);

	/* Call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
	phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS;
	status = prvMpdPerformPhyOperation(	portEntry_PTR,
										PRV_MPD_OP_CODE_SET_ERRATA_E,
										&phy_params);
	if (status == MPD_OP_FAILED_E) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "portEntry_PTR->rel_ifIndex %d.set errata failed!",
                                                        portEntry_PTR->rel_ifIndex);
		return MPD_OP_FAILED_E;
	}

	/* Fiber auto-negotiation should always be on - reason unknown */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_1_FIBER_CONTROL_CNS,
									PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS,
									&val);
	/* Write only if there is a change */
	if ((val & PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS) == 0) {
		status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
											portEntry_PTR,
											PRV_MPD_PAGE_1_FIBER_CONTROL_CNS,
											PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS,
											PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
											(GT_U16) (val | PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS));
	}
	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdEnableFiberPortStatus_4
 *
 * DESCRIPTION: 	Enable fiber on combo port
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1543_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdEnableFiberPortStatus_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT res;
	MPD_OPERATIONS_PARAMS_UNT comboMode;
	MPD_OPERATIONS_PARAMS_UNT auto_neg;

	if (params_PTR->internal.phyFiberParams.fiberPresent) { /* isPresent == TRUE */
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugTraceFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"got RX loss TRUE");

		comboMode.internal.phyFiberParams.comboMode = params_PTR->internal.phyFiberParams.comboMode;
		comboMode.internal.phyFiberParams.phySpeed = params_PTR->internal.phyFiberParams.phySpeed;
		comboMode.internal.phyFiberParams.fiberPresent = TRUE;

		res = prvMpdSetComboMediaType_4(portEntry_PTR,
										&comboMode);

		PRV_MPD_CHECK_RETURN_STATUS_MAC(res, portEntry_PTR->rel_ifIndex);
		/* port type is allways 1G */
		auto_neg.phyAutoneg.enable = MPD_AUTO_NEGOTIATION_ENABLE_E;
		auto_neg.phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS;
		res = prvMpdSetAutoNegotiation_6(	portEntry_PTR,
											&auto_neg);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(res, portEntry_PTR->rel_ifIndex);
	}
	return MPD_OK_E;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetEeeMasterEnable_1
 *
 * DESCRIPTION: 	Set EEE master enable
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetEeeMasterEnable_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 data;
	prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
							0,
							&data);
	if (params_PTR->phyEeeMasterEnable.masterEnable == TRUE)
	{
		data |= 0x0001;
		PRV_MPD_SLEEP_MAC((prvMpdEeeMasterEnableSsleep));
	}
	else
		data &= 0xFFFE;

	prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
							0,
							PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
							data);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetLpiExitTime_1
 *
 * DESCRIPTION: 	Set LPI Exit time (EEE)
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E

 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetLpiExitTime_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 data, dataUpdate = 0, dataUpdate_mask = 0;

	if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E)
	{
		dataUpdate = params_PTR->phyEeeLpiTime.time_us & 0x00FF;
		dataUpdate_mask = 0xFF00;
	}
	else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E)
	{
		dataUpdate = params_PTR->phyEeeLpiTime.time_us << 8;
		dataUpdate_mask = 0x00FF;
	}
	else
		return MPD_NOT_SUPPORTED_E;

	prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
							1,
							&data);
	data &= dataUpdate_mask;
	data |= dataUpdate;
	prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
							1,
							PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
							data);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetLpiEnterTime_1
 *
 * DESCRIPTION: 	Set LPI Enter time (EEE)
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E

 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetLpiEnterTime_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
#ifndef PHY_SIMULATION
	UINT_16 data, dataUpdate = 0, dataUpdate_mask = 0;
#endif
	MPD_UNUSED_PARAM(portEntry_PTR);
	MPD_UNUSED_PARAM(params_PTR);
#ifndef PHY_SIMULATION
	if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E)
	{
		dataUpdate = params_PTR->phyEeeLpiTime.time_us & 0x00FF;
		dataUpdate_mask = 0xFF00;
	}
	else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E)
	{
		dataUpdate = params_PTR->phyEeeLpiTime.time_us << 8;
		dataUpdate_mask = 0x00FF;
	}
	else
		return MPD_NOT_SUPPORTED_E;

	prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
							2,
							&data);
	data &= dataUpdate_mask;
	data |= dataUpdate;
	prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
							2,
							PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
							data);
#endif
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetEeeMaxTxVal_1
 *
 * DESCRIPTION: 	EEE, Get max Tx (exit) value
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E

 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetEeeMaxTxVal_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	UINT_16 max_tx_val[MPD_EEE_SPEED_MAX_E] = {
													#ifndef PHY_SIMULATION
													0,
#else
        255,
#endif
													112,
													32,
													0,
													0
	};
	MPD_EEE_SPEED_ENT speed_index;
	MPD_UNUSED_PARAM(portEntry_PTR);

	for (speed_index = 0; speed_index < MPD_EEE_SPEED_MAX_E; speed_index++) {
		params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[speed_index] = max_tx_val[speed_index];
	}

	return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetFastLinkDownEnable_1
 *
 * DESCRIPTION: 	Enable fast link down indication (towards host)
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetFastLinkDownEnable_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
#define PRV_MPD_FASTLINKDOWN_DELAY_MASK_CNS  0xC00 /* 26_0.11:10 */
	UINT_16 read_val;
	MPD_RESULT_ENT status;

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
									PRV_MPD_SPECIFIC_CONTROL_REG_3_CNS,
									&read_val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* Per the IEEE 802.3 Clause 40 standard, a 1000BASE-T PHY is required to wait 750 milliseconds or more
	 ** to report that link is down after detecting a problem with the link.
	 ** For Metro Ethernet applications, a Fast Fail over in 50 ms is specified,
	 ** which cannot be met if the PHY follows the 750 ms wait time.
	 ** This delay can be reduced intentionally violating the IEEE standard by
	 **     setting register 26_0.9 to 1
	 ** in our system we use this setting only for cascade ports, only when FFO is supported.
	 ** The delay at which link down is to be reported can be selected by
	 **      setting register 26_0.11:10 00 = 0ms, 01 = 10+-2ms, 10 = 20+-2ms, 11 = 40+-2ms
	 */
	if (params_PTR->internal.phyFastLinkDown.enable == TRUE) {
		read_val |= (1 << 9);
		/* set minimum delay */
		read_val &= ~(PRV_MPD_FASTLINKDOWN_DELAY_MASK_CNS);
	}
	else {
		read_val &= ~(1 << 9);
	}

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
										PRV_MPD_SPECIFIC_CONTROL_REG_3_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										read_val);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetEeeGetEnableMode_1
 *
 * DESCRIPTION: 	Whether the PHY support mode that EEE can be enabled
 *               by admin and link change or by admin only.
 *               This function is for PHYs that their mode is link change and admin.
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E3680_E
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *					MPD_TYPE_88X2180_E
 *					MPD_TYPE_88X20x0_E
 *					MPD_TYPE_88X33x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetEeeGetEnableMode_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->internal.phyEeeEnableMode.enableMode = MPD_EEE_ENABLE_MODE_LINK_CHANGE_E;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDisableOperation_9
 *
 * DESCRIPTION: 	Adminstrative shutdown for fiber phyless ports
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_DIRECT_ATTACHED_FIBER_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdDisableOperation_9(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	BOOLEAN enable;
	enable = (params_PTR->phyDisable.forceLinkDown == FALSE) ? TRUE : FALSE;
	PRV_MPD_TRANSCEIVER_ENABLE_MAC(	portEntry_PTR->rel_ifIndex,
									enable);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdFwLoadSequence
 *
 * DESCRIPTION: 	utility function
 *               - read firmware from flash to RAM
 *               - determine file format
 *               - pre-configure PHY to accept new firmware
 *               - load firmware to PHY
 *               - post-configure PHY to work with new firmware
 *               - verify firmware load was successful
 *               - free allocated memory
 *
 *****************************************************************************/
void prvMpdFwLoadSequence(
	/*     INPUTS:             */
	PRV_MPD_PORT_LIST_TYP *fw_port_list_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	UINT_32 rel_ifIndex;
	MPD_RESULT_ENT ret_val = MPD_OK_E;
	MPD_OPERATIONS_PARAMS_UNT params;
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR = NULL;

	/* pre-load configuration */
	/* portEntry_PTR->rel_ifIndex 0 isn't valid */
	for (rel_ifIndex = 0; prvMpdPortListGetNext(fw_port_list_PTR,
												&rel_ifIndex);) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		/* pre-download validation */
		ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
											PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
											&params);
		if (ret_val == MPD_OP_FAILED_E) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"pre-download failed on portEntry_PTR->rel_ifIndex %d",
															portEntry_PTR->rel_ifIndex);
			/*break;*/
		}
	}

	/* Load FW */
	for (rel_ifIndex = 0; prvMpdPortListGetNext(fw_port_list_PTR,
												&rel_ifIndex);) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		/* download doesn't require parameters */
		memset(	&params,
				0,
				sizeof(params));
		ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
											PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
											&params);
		if (ret_val == MPD_OP_FAILED_E) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"Download failed on portEntry_PTR->rel_ifIndex %d ",
															portEntry_PTR->rel_ifIndex);
			/*break;*/
		}
	}

	/* post load configuration */
	for (rel_ifIndex = 0; prvMpdPortListGetNext(fw_port_list_PTR,
												&rel_ifIndex);) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
											PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
											&params);
		if (ret_val == MPD_OP_FAILED_E) {
			PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"failed post download portEntry_PTR->rel_ifIndex %d ",
															portEntry_PTR->rel_ifIndex);
			/*break;*/
		}
	}
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdHandleFw
 *
 * DESCRIPTION: 	Utility, Specific feature callback for PHYs having fw.
 *
 *****************************************************************************/
static void prvMpdHandleFw(
	/*     INPUTS:             */
	PRV_MPD_PORT_LIST_TYP *portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	static BOOLEAN first_time = TRUE;
	UINT_32 rel_ifIndex;
	MPD_TYPE_ENT *phyType_PTR;
	PRV_MPD_PORT_LIST_TYP mtdFwPortList, mydFwPortList;
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR = NULL;

	if (first_time) {
		first_time = FALSE;
	}
	else {
		return;
	}
	prvMpdPortListClear(&mtdFwPortList);
    prvMpdPortListClear(&mydFwPortList);

	/* allocate port-phy db memory for external-hw PHYs */
	phyType_PTR =
			(MPD_TYPE_ENT*) PRV_MPD_CALLOC_MAC(	MPD_MAX_PORT_NUMBER_CNS + 1,
												sizeof(MPD_TYPE_ENT));

	if (phyType_PTR == NULL) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_FATAL_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
		return;
	}

	/* set PHY type */
	for (rel_ifIndex = 0;
			prvMpdPortListGetNext(	portList_PTR,
									&rel_ifIndex) == TRUE;) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		/* Checking if the phy has a FW to load */
        if (prvMpdFuncDb_ARR[portEntry_PTR->initData_PTR->phyType][PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E].phyOperation_PTR != NULL) {
            phyType_PTR[portEntry_PTR->rel_ifIndex] = portEntry_PTR->initData_PTR->phyType; 
            /* check phy type is MYD */
            if (portEntry_PTR->initData_PTR->phyType == MPD_TYPE_88X7120_E) {
                prvMpdPortListAdd(	portEntry_PTR->rel_ifIndex,
                                    &mydFwPortList);
            } else {
                prvMpdPortListAdd(	portEntry_PTR->rel_ifIndex,
                                    &mtdFwPortList);
            }
        }
	}

    if (!prvMpdPortListIsEmpty(&mtdFwPortList)) {
        if (mpdMtdInit(	&mtdFwPortList,
                        phyType_PTR) != MPD_OK_E) {
            PRV_MPD_HANDLE_FAILURE_MAC(	0,
                                        MPD_ERROR_SEVERITY_FATAL_E,
                                        "mpdMtdInit failed");
        }
        /* handle FW downloading */
        prvMpdFwLoadSequence(&mtdFwPortList);
    }

    if (!prvMpdPortListIsEmpty(&mydFwPortList)) {
        if (mpdMydInit(	&mydFwPortList,
                        phyType_PTR) != MPD_OK_E) {
            PRV_MPD_HANDLE_FAILURE_MAC(	0,
                                        MPD_ERROR_SEVERITY_FATAL_E,
                                        "mpdMydInit failed");
        }
        /* handle FW downloading */
        prvMpdFwLoadSequence(&mydFwPortList);
    }

	PRV_MPD_FREE_MAC(phyType_PTR);
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdHandleSf
 *
 * DESCRIPTION: 	Set Specific feature callback for SolarFlare PHYs
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetSpecificFeatures_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */

)
{
	UINT_32 idx = PRV_MPD_SPECIFIC_FEATURE_FW_LOAD_SF_E;
	MPD_UNUSED_PARAM(portEntry_PTR);
	MPD_UNUSED_PARAM(params_PTR);
	if (prvMpdSpecificFeature_ARR[idx].supported == FALSE) {
		prvMpdSpecificFeature_ARR[idx].supported = TRUE;
		prvMpdSpecificFeature_ARR[idx].preInitFeatureExecuteCallback_PTR = prvMpdHandleFw;
		prvMpdSpecificFeature_ARR[idx].postInitFeatureExecuteCallback_PTR = NULL;
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDisableOperation_14
 *
 * DESCRIPTION: 	Adminstrative shutdown of PHY
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdDisableOperation_14(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	GT_U16 val, val_reg3;
	BOOLEAN enable;
	MPD_RESULT_ENT status;

	/* set power down */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
									PRV_MPD_CTRLREG_OFFSET_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if (params_PTR->phyDisable.forceLinkDown == TRUE)
	{
		val = val | PRV_MPD_LINK_DOWN_CNS;
		enable = FALSE;
	}
	else
	{
		val = val & PRV_MPD_LINK_UP_CNS;
		enable = TRUE;
	}

	status = prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										PRV_MPD_PAGE_0_COPPER_CONTROL_CNS,
										PRV_MPD_CTRLREG_OFFSET_CNS,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	/* if not force link down - do soft reset to QSGMII - see RN of 1680 rev A1 */
	if (enable == TRUE)
	{
		prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							3,
							&val_reg3);
		if (val_reg3 == 0x0ED2)
				{ /* revision A1 */
			prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									4,
									0,
									&val);
			val |= (1 << 15); /* write 1 to bit 15 - QSGMII soft reset */
			prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									4,
									0,
									PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
									val);
		}
		prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							16,
							&val);
		val &= 0xFFFB;
		prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							16,
							val);
	}
	return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdQsgmiiWa
 *
 * DESCRIPTION: 	Utility, Specific feature callback for 1680 PHY
 * 				implement QSGMII WA
 *
 *****************************************************************************/
static void prvMpdQsgmiiWa(
	/*     INPUTS:             */
	PRV_MPD_PORT_LIST_TYP *portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	UINT_32 rel_ifIndex;
	UINT_16 reg_data;
	MPD_RESULT_ENT rc = MPD_OK_E;
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR = NULL;
	MPD_UNUSED_PARAM(portList_PTR);
	/* if no members, there is nothing to do */
	if (prvMpdPortListNumOfMembers(&prvMpd88E1680PortList) == 0) {
		return;
	}
	/* configuration that need to be done only on one port in each quad - see 88E1680 Rev A1 RN from 6.4.12 */
	for (rel_ifIndex = 0;
			prvMpdPortListGetNext(	&prvMpd88E1680PortList,
									&rel_ifIndex);) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								0x00FD);
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								8,
								0x10F7); /* QSGMII Link Down Workaround */
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								7,
								0x2006);
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								4);
		rc |= prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								26,
								&reg_data);
		reg_data |= 0x0800; /* write 1 to bit 11 - QSGMII block power down */
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								26,
								reg_data);
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								0);
	}

#ifndef PHY_SIMULATION
	PRV_MPD_SLEEP_MAC((300));
#endif

	for (rel_ifIndex = 0;
			prvMpdPortListGetNext(	&prvMpd88E1680PortList,
									&rel_ifIndex);) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								4);
		rc |= prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								26,
								&reg_data);
		reg_data &= ~0x0800; /* write 0 to bit 11 - QSGMII block power up */
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								26,
								reg_data);
		rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								0);
	}

#ifndef PHY_SIMULATION
	PRV_MPD_SLEEP_MAC((200));
#endif

	/* QSGMII soft reset - per port */
	for (rel_ifIndex = 0;
			prvMpdPortListGetNext(	&prvMpd88E1680PortList,
									&rel_ifIndex);) {
		portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (portEntry_PTR == NULL) {
			continue;
		}
		rc |= prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										4,
										0,
										&reg_data);
		reg_data |= (1 << 15); /* write 1 to bit 15 - QSGMII soft reset */

		rc |= prvMpdMdioWriteRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										portEntry_PTR,
										4,
										0,
										PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
										reg_data);
	}

	if (rc == MPD_OP_FAILED_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,
									MPD_ERROR_SEVERITY_ERROR_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
	}
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetSpecificFeatures_4
 *
 * DESCRIPTION: 	Set Specific feature
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetSpecificFeatures_4(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 idx = PRV_MPD_SPECIFIC_FEATURE_WA_1680_E;
	MPD_UNUSED_PARAM(portEntry_PTR);
	MPD_UNUSED_PARAM(params_PTR);
	if (prvMpdSpecificFeature_ARR[idx].supported == FALSE)
	{
		prvMpdSpecificFeature_ARR[idx].supported = TRUE;
		prvMpdSpecificFeature_ARR[idx].preInitFeatureExecuteCallback_PTR = NULL;
		prvMpdSpecificFeature_ARR[idx].postInitFeatureExecuteCallback_PTR = prvMpdQsgmiiWa;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetEeeGetEnableMode_2
 *
 * DESCRIPTION: 	Whether the PHY support mode that EEE can be enabled
 *               by admin and link change or by admin only.
 *               This function is for PHYs that their mode is link change and admin.
 * APPLICABLE PHYS:
 *					MPD_TYPE_88X32x0_E
 *					MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetEeeGetEnableMode_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
       MPD_UNUSED_PARAM(portEntry_PTR);
	params_PTR->internal.phyEeeEnableMode.enableMode = MPD_EEE_ENABLE_MODE_ADMIN_ONLY_E;
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetInternalOperStatus_3
 *
 * DESCRIPTION:	Get PHY operational status
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetInternalOperStatus_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 						copper_specific_status, copper_status;
	MPD_RESULT_ENT 					status;
	MPD_INTERNAL_OPER_STATUS_STC  * oper_stat_PTR;


	oper_stat_PTR = &(params_PTR->phyInternalOperStatus);
	memset(	oper_stat_PTR,
			0,
			sizeof(MPD_INTERNAL_OPER_STATUS_STC));

	oper_stat_PTR->mediaType = MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_SPECIFIC_STATUS_REG_CNS,
									&copper_specific_status);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	if ((copper_specific_status & PRV_MPD_PORT_STATUS_LINK_MASK_CNS) == 0) {
		oper_stat_PTR->isOperStatusUp = FALSE;
		oper_stat_PTR->phySpeed = MPD_SPEED_LAST_E;
		return MPD_OK_E;
	}

	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_COPPER_STATUS_REGISTER_CNS,
									&copper_status);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	if ((copper_status & PRV_MPD_COPPER_AUTONEG_COMPLETE_CNS) != 0) {
		oper_stat_PTR->isAnCompleted = TRUE;
	}

	oper_stat_PTR->isOperStatusUp = TRUE;
	/* link is UP - get speed */
	if ((copper_specific_status & PRV_MPD_PORT_STATUS_SPEED_AND_DUPLEX_RESOLVED_CNS) == 0) {
		oper_stat_PTR->phySpeed = MPD_SPEED_LAST_E;
	}
	else {
		if ((copper_specific_status & PRV_MPD_PORT_STATUS_SPEED_10M_CNS) == PRV_MPD_PORT_STATUS_SPEED_10M_CNS) {
			oper_stat_PTR->phySpeed = MPD_SPEED_10M_E;
		}
		if ((copper_specific_status & PRV_MPD_PORT_STATUS_SPEED_100M_CNS) == PRV_MPD_PORT_STATUS_SPEED_100M_CNS) {
			oper_stat_PTR->phySpeed = MPD_SPEED_100M_E;
		}
		if ((copper_specific_status & PRV_MPD_PORT_STATUS_SPEED_1000M_CNS) == PRV_MPD_PORT_STATUS_SPEED_1000M_CNS) {
			oper_stat_PTR->phySpeed = MPD_SPEED_1000M_E;
		}
		if ((copper_specific_status & PRV_MPD_PORT_STATUS_DPLX_MOD_MASK_CNS) == PRV_MPD_PORT_STATUS_DPLX_MOD_MASK_CNS) {
			oper_stat_PTR->duplexMode = TRUE;
		}
		else {
			oper_stat_PTR->duplexMode = FALSE;
		}
	}

	oper_stat_PTR->mediaType = MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetCableLenNoRange_2
 *
 * DESCRIPTION: get accurate cable length for SF PHYs
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E2540_E
 *					MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetCableLenNoRange_2(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT ret_val;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d",
														portEntry_PTR->rel_ifIndex);

	memset(	&phy_params,
			0,
			sizeof(phy_params));
	ret_val = prvMpdPerformPhyOperation(portEntry_PTR,
										MPD_OP_CODE_GET_CABLE_LEN_E,
										&phy_params);
	if (ret_val != MPD_OK_E) {
		PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"portEntry_PTR->rel_ifIndex %d. phy_op_not_supported. ret_val %d",
															portEntry_PTR->rel_ifIndex,
															ret_val);

		return MPD_NOT_SUPPORTED_E;
	}

	params_PTR->phyCableLenNoRange.cableLen = phy_params.phyCableLen.accurateLength;

	PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugConfigVctFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"portEntry_PTR->rel_ifIndex %d. cableLen %d",
														portEntry_PTR->rel_ifIndex,
														params_PTR->phyCableLenNoRange.cableLen);

	return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSfpPresentNotification_6
 *
 * DESCRIPTION: 	Set present (LOS) notification
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1548_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSfpPresentNotification_6(
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
	}
	else {
		portEntry_PTR->runningData_PTR->opMode = MPD_OP_MODE_UNKNOWN_E;
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetGreenConsumption
 *
 * DESCRIPTION: 	Get results of power measurements (by PHY team)
 *
 * APPLICABLE PHYS:	ALL
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetGreenConsumption(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 retConsumption = 0;
	MPD_TYPE_ENT phyType;
	PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;
	if ((UINT_32) portEntry_PTR->initData_PTR->phyType >= MPD_TYPE_INVALID_E) {
		/*This is not fatal error because we might try to set operation for port that was disconnect during init*/
		return MPD_NOT_SUPPORTED_E;
	}

	phyType = portEntry_PTR->initData_PTR->phyType;

	entry_PTR = &prvMpdPowerConsumptionPerPhy[phyType];

	if (params_PTR->phyPowerConsumptions.getMax) {
		params_PTR->phyPowerConsumptions.greenConsumption = entry_PTR->max;
		return MPD_OK_E;
	}

	if (params_PTR->phyPowerConsumptions.portUp) {
		if (params_PTR->phyPowerConsumptions.portSpeed < MPD_SPEED_LAST_E) {

			if (params_PTR->phyPowerConsumptions.shortReach == MPD_GREEN_ENABLE_E) {
				retConsumption = entry_PTR->up.sr_on[params_PTR->phyPowerConsumptions.portSpeed];
			}
			else {
				retConsumption = entry_PTR->up.sr_off[params_PTR->phyPowerConsumptions.portSpeed];
			}
		}
		else {
			/* illegal speed */
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "portEntry_PTR->rel_ifIndex %d. illegal speed",
                                                                portEntry_PTR->rel_ifIndex);
			return MPD_NOT_SUPPORTED_E;
		}
	}
	else {
		if (params_PTR->phyPowerConsumptions.energyDetetct == MPD_GREEN_ENABLE_E) {
			retConsumption = entry_PTR->down.ed_on;
		}
		else {
			retConsumption = entry_PTR->down.ed_off;
		}
	}

	params_PTR->phyPowerConsumptions.greenConsumption = retConsumption;
	if (params_PTR->phyPowerConsumptions.greenConsumption == 0) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "portEntry_PTR->rel_ifIndex %d. greenConsumption = 0.",
                                                        portEntry_PTR->rel_ifIndex);
		return MPD_NOT_SUPPORTED_E;
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetEeeMaxTxVal_1
 *
 * DESCRIPTION: 	EEE, Get max Tx (exit) value
 *				in this PHY EEE Exit timer is not configurable --> MAX Tx = MIN Tx
 * APPLICABLE PHYS:
 *					MPD_TYPE_88x32x0_E

 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdGetEeeMaxTxVal_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/* value[0] - speed 10m; value[1] - speed 100m;
	 value[2] - speed 1g; value[3] - speed 10g */
	/*     OUTPUTS:            */
)
{

	UINT_16 max_tx_val[MPD_EEE_SPEED_MAX_E] = {
													0,
													0,
													0,
													PRV_MPD_3140_EEE_LPI_EXIT_TIME_CNS,
													0
	};
	MPD_EEE_SPEED_ENT speed_index;
	MPD_UNUSED_PARAM(portEntry_PTR);

	for (speed_index = 0; speed_index < MPD_EEE_SPEED_MAX_E; speed_index++) {
		params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[speed_index] = max_tx_val[speed_index];
	}

	return MPD_OK_E;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetSpeed_9
 *
 * DESCRIPTION: 	Set Speed
 *				on this PHY et speed on fiber only port
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E1548_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetSpeed_9(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 mode = 7; /* mode[0-2]*/
	UINT_16 control_reg_val;
	/* get value of the general control register */
	prvMpdMdioReadRegister(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							18,
							20,
							&control_reg_val);
	switch (params_PTR->phySpeed.speed) {
		case MPD_SPEED_100M_E:
			/* configure as 100 sfp - QSGMII (system mode) to 100Base-FX */
			mode = 3;
			/* Set Auto media detect copper / 100Base-FX to 100Base-FX only */
			control_reg_val &= ~(1 << 6);
			break;
		case MPD_SPEED_1000M_E:
			/* config as 1G sfp - QSGMII (system mode) to 1000Base-X */
			mode = 2;
			/* Set Auto media detect copper / 100Base-FX to auto */
			control_reg_val |= (1 << 6);
			break;
		default:
			return MPD_NOT_SUPPORTED_E;
	}

	/* Set Auto-Media Detect Preferred Media - fiber */
	control_reg_val &= ~(1 << 4);
	control_reg_val |= (1 << 5);
	/* reset phy mode */
	control_reg_val |= (1 << 15);
	/* set mode */
	control_reg_val |= mode;
	/* write back to PHY */
	prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							portEntry_PTR,
							18,
							20,
							PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
							control_reg_val);

	/* Change page to 1. */
	prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						portEntry_PTR,
						PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
						1);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetMdixAdmin_1
 *
 * DESCRIPTION: Get configured MDIX mode
 *
 * APPLICABLE PHYS:
 *					MPD_TYPE_88E151x_E
 *					MPD_TYPE_88E1543_E
 *					MPD_TYPE_88E1545_E
 *					MPD_TYPE_88E1680_E
 *					MPD_TYPE_88E1680L_E
 *					MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdGetMdixAdmin_1(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	GT_U16 			val;
	MPD_RESULT_ENT	status;

	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
		/* port is fiber only - mdi configuration is not supported */
		return MPD_NOT_SUPPORTED_E;
	}
	/* read MDIX configuration */
	status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									portEntry_PTR,
									0,
									PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
									&val);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	val &= PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS;
	switch (val) {
		case PRV_MPD_MANUAL_MDI_CONFIGURATION_CNS:
			params_PTR->phyMdix.mode = MPD_MDI_MODE_MEDIA_E;
		break;
		case PRV_MPD_MANUAL_MDIX_CONFIGURATION_CNS:
			params_PTR->phyMdix.mode = MPD_MDIX_MODE_MEDIA_E;
		break;
		case PRV_MPD_AUTOMATIC_CROSSOVER_CNS:
			params_PTR->phyMdix.mode = MPD_AUTO_MODE_MEDIA_E;
		break;
		default:
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "portEntry_PTR->rel_ifIndex %d. prvMpdGetMdixAdmin_1 failed.",
                                                            portEntry_PTR->rel_ifIndex);
			return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: mpdBindExternalPhyCallback
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT mpdBindExternalPhyCallback(
	/*     INPUTS:             */
	MPD_TYPE_ENT phyType,
	MPD_OP_CODE_ENT op,
	MPD_OPERATIONS_FUN *func_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{

	BOOLEAN status = FALSE;

	if (!((phyType > MPD_TYPE_LAST_SUPPERTED_E) && (phyType < MPD_TYPE_NUM_OF_TYPES_E))) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_FATAL_E,
									"failed to bind external phy type");
		return MPD_OP_FAILED_E;
	}

	status = PRV_MPD_BIND_MAC(	phyType,
								op,
								func_PTR);
	if (!status) {
		return MPD_OP_FAILED_E;
	}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdUpdatePhyPowerConsumption
 *
 * DESCRIPTION: update power consumption phy entry
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdUpdatePhyPowerConsumption(
	/*     INPUTS:             */
	MPD_TYPE_ENT phyType,
	PRV_MPD_POWER_CONSUMPTION_STC *powerConsumption_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;
	if (!((phyType > MPD_TYPE_LAST_SUPPERTED_E) && (phyType < MPD_TYPE_NUM_OF_TYPES_E))) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_FATAL_E,
									"failed to update power consumption");
		return MPD_OP_FAILED_E;
	}

	entry_PTR = &prvMpdPowerConsumptionPerPhy[phyType];
	memcpy(	entry_PTR,
			powerConsumption_PTR,
			sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

	return MPD_OK_E;
}

/**
 *
 * @brief   initialize PHY driver for all ports
 *          initialize PHY (HW) for all ports
 *
 *          @note Initialization is done based on information
 *				gathered through prvMpdPortHashCreate & mpdPortDbUpdate
 *
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdDriverInitHw(
	void
)
{
	MPD_RESULT_ENT ret_val;

	ret_val = prvMpdPortListInitSequence(NULL);

	if (ret_val == MPD_OP_FAILED_E) {
		PRV_MPD_HANDLE_FAILURE_MAC(	0,
									MPD_ERROR_SEVERITY_FATAL_E,
									PRV_MPD_DEBUG_FUNC_NAME_MAC());
	}
	return ret_val;
}

/**
 *
 * @brief   initialize PHY driver DB
 * @param [in ]   callbacks_PTR                - application callbacks
 *
 *
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdDriverInitDb(
	IN MPD_CALLBACKS_STC *callbacks_PTR
)
{
	if (callbacks_PTR == NULL) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "callbacks_PTR = NULL.");
		return MPD_OP_FAILED_E;
	}
	memcpy(	&prvMpdCallBacks,
			callbacks_PTR,
			sizeof(prvMpdCallBacks));
	prvMpdCallBacks.initialized = TRUE;

	/* initialize DB */
	if (prvMpdPortHashCreate() == MPD_OP_FAILED_E) {
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugErrorFlagId)(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "Failed to create portHash.");
		return MPD_OP_FAILED_E;
	}

	memset(	prvMpdSpecificFeature_ARR,
			0,
			sizeof(prvMpdSpecificFeature_ARR));

	/* bind debug */

	PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
							prvMpdDebugPckName,
							prvMpdDebugWriteFlagName,
							prvMpdDebugWriteFlagHelp,
							&prvMpdDebugWriteFlagId);

	PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
							prvMpdDebugPckName,
							prvMpdDebugReadFlagName,
							prvMpdDebugReadFlagHelp,
							&prvMpdDebugReadFlagId);

	PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
							prvMpdDebugPckName,
							prvMpdDebugConfigVctFlagName,
							prvMpdDebugConfigVctFlagHelp,
							&prvMpdDebugConfigVctFlagId);

	PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
							prvMpdDebugSfpPckName,
							prvMpdDebugSfpFlagName,
							prvMpdDebugSfpFlagHelp,
							&prvMpdDebugSfpFlagId);

	PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
							prvMpdDebugPckName,
							prvMpdDebugErrorFlagName,
							prvMpdDebugErrorFlagHelp,
							&prvMpdDebugErrorFlagId);

	PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
							prvMpdDebugPckName,
							prvMpdDebugTraceFlagName,
							prvMpdDebugTraceFlagHelp,
							&prvMpdDebugTraceFlagId);

    PRV_MPD_DEBUG_BIND_MAC(	prvMpdDebugComponentName,
                            prvMpdDebugPckName,
                            prvMpdDebugOperationFlagName,
                            prvMpdDebugOperationFlagHelp,
                            &prvMpdDebugOperationFlagId);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdGetTemperature
 *
 * DESCRIPTION:	Get temperature from PHY temperature sensor
 *
 * APPLICABLE PHY:
 * 				MPD_TYPE_88E1543_E
 *              MPD_TYPE_88E1545_E
 *              MPD_TYPE_88E1548_E
 *              MPD_TYPE_88E151x_E
 *              MPD_TYPE_88E1680_E
 * 				MPD_TYPE_88E1680L_E
 *              MPD_TYPE_88E3680_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdGetTemperature(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT status;
    UINT_16 val;
	INT_16 temperature;

    status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    6,
                                    27,
                                    &val);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    val &= 0xFF;
    temperature = val - 25;
	params_PTR->phyTemperature.temperature = temperature;
	return MPD_OK_E;
}

