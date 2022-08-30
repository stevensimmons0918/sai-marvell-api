/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file shrMemSharedData.c
*
* @brief This file contains common data for OS/SharedMemory modules
* which should be shared between different processes, thus should
* be linked into libcpss.so (only that object
* has shared BSS/DATA sections).
*
* @version   8
********************************************************************************
*/
#include <gtOs/gtEnvDep.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/gtOsSharedUtil.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*********************************************************************************************
*  The RxTxProcessParams struct used for interprocess communication in multi process appDemo
*  the RxTx parameters transmitted from appDemo process to RxTxProcess used to fill the 
*  the packet capture table
*  
*  function cpssMultiProcComSetRxTxParams used to set the RxTx parameters
*  function cpssMultiProcComGetRxTxParams used to get the RxTx parameters
*  function cpssMultiProcComSetTransmitParams used to set the Packet Transmit  parameters
*  function cpssMultiProcComGetTransmitParams used to get the Packet Transmit  parameters
*
**********************************************************************************************/
#ifdef SHARED_MEMORY

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtShrMemShrDataSrc

/**
* @internal cpssMultiProcComSetRxTxParams function
* @endinternal
*
* @brief   Used to set packet RxTx parameters for transmission from appDemo process
*         to RxTx process
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComSetRxTxParams
(
	CPSS_RX_TX_MP_MODE_PARAMS_STC *paramPtr
)
{
	PRV_SHARED_DB.RxTxProcessParams.valid = paramPtr->valid;
	PRV_SHARED_DB.RxTxProcessParams.copyRxInfoFunc = paramPtr->copyRxInfoFunc ;
	PRV_SHARED_DB.RxTxProcessParams.doCapture = paramPtr->doCapture;
	PRV_SHARED_DB.RxTxProcessParams.freeRxInfoFunc = paramPtr->freeRxInfoFunc;
	PRV_SHARED_DB.RxTxProcessParams.indexToInsertPacket = paramPtr->indexToInsertPacket;
	PRV_SHARED_DB.RxTxProcessParams.maxNumOfRxEntries = paramPtr->maxNumOfRxEntries;
	PRV_SHARED_DB.RxTxProcessParams.maxRxBufferSize = paramPtr->maxRxBufferSize;
	PRV_SHARED_DB.RxTxProcessParams.rxEntriesArr = paramPtr->rxEntriesArr;
	PRV_SHARED_DB.RxTxProcessParams.rxTableMode = paramPtr->rxTableMode;
	PRV_SHARED_DB.RxTxProcessParams.specificDeviceFormatPtr = paramPtr->specificDeviceFormatPtr;
	PRV_SHARED_DB.RxTxProcessParams.wasInitDone = paramPtr->wasInitDone;

	return GT_OK;
}

/**
* @internal cpssMultiProcComGetRxTxParams function
* @endinternal
*
* @brief   Used to get packet RxTx parameters for transmission from appDemo process and send it
*         to RxTx process
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComGetRxTxParams
(
	CPSS_RX_TX_MP_MODE_PARAMS_STC *paramPtr
)
{
	paramPtr->valid = PRV_SHARED_DB.RxTxProcessParams.valid;
	paramPtr->copyRxInfoFunc = PRV_SHARED_DB.RxTxProcessParams.copyRxInfoFunc;
	paramPtr->doCapture = PRV_SHARED_DB.RxTxProcessParams.doCapture;
	paramPtr->freeRxInfoFunc = PRV_SHARED_DB.RxTxProcessParams.freeRxInfoFunc;
	paramPtr->indexToInsertPacket = PRV_SHARED_DB.RxTxProcessParams.indexToInsertPacket;
	paramPtr->maxNumOfRxEntries = PRV_SHARED_DB.RxTxProcessParams.maxNumOfRxEntries;
	paramPtr->maxRxBufferSize = PRV_SHARED_DB.RxTxProcessParams.maxRxBufferSize;
	paramPtr->rxEntriesArr = PRV_SHARED_DB.RxTxProcessParams.rxEntriesArr;
	paramPtr->rxTableMode = PRV_SHARED_DB.RxTxProcessParams.rxTableMode;
	paramPtr->specificDeviceFormatPtr = PRV_SHARED_DB.RxTxProcessParams.specificDeviceFormatPtr;
	paramPtr->wasInitDone = PRV_SHARED_DB.RxTxProcessParams.wasInitDone;

	return GT_OK;
}

#ifdef CHX_FAMILY


/*********** TX  ************/

/**
* @internal cpssMultiProcComSetTransmitParams function
* @endinternal
*
* @brief   Used to set packet transmit parameters for transmission to RxTx process from appDemo process
*
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComSetTransmitParams
(
	CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC *paramPtr
)
{
	PRV_SHARED_DB.transmitParams.valid = paramPtr->valid;
	PRV_SHARED_DB.transmitParams.cpssDxChTxPacketDescTbl = paramPtr->cpssDxChTxPacketDescTbl;
	PRV_SHARED_DB.transmitParams.flagStopTransmit = paramPtr->flagStopTransmit;
	PRV_SHARED_DB.transmitParams.modeSettings.cyclesNum = paramPtr->modeSettings.cyclesNum;
	PRV_SHARED_DB.transmitParams.modeSettings.gap = paramPtr->modeSettings.gap;
	PRV_SHARED_DB.transmitParams.txDxChTblCapacity = paramPtr->txDxChTblCapacity;
	PRV_SHARED_DB.transmitParams.txPoolPtr = paramPtr->txPoolPtr;

	return GT_OK;
}


/**
* @internal cpssMultiProcComGetTransmitParams function
* @endinternal
*
* @brief   Used to get packet transmit parameters to RxTx process from appDemo process
*
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComGetTransmitParams
(
	CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC *paramPtr
)
{
	paramPtr->valid = PRV_SHARED_DB.transmitParams.valid;
	paramPtr->cpssDxChTxPacketDescTbl = PRV_SHARED_DB.transmitParams.cpssDxChTxPacketDescTbl;
	paramPtr->flagStopTransmit = PRV_SHARED_DB.transmitParams.flagStopTransmit;
	paramPtr->modeSettings.cyclesNum = PRV_SHARED_DB.transmitParams.modeSettings.cyclesNum;
	paramPtr->modeSettings.gap = PRV_SHARED_DB.transmitParams.modeSettings.gap;
	paramPtr->txDxChTblCapacity = PRV_SHARED_DB.transmitParams.txDxChTblCapacity;
	paramPtr->txPoolPtr = PRV_SHARED_DB.transmitParams.txPoolPtr;
	
	return GT_OK;
}

/************ Command transmit ***********/

/**
* @internal cpssMultiProcComGetRxTxCommand function
* @endinternal
*
* @brief   Used to get if the Galtis command cmdCpssRxPktClearTbl was run by appDemo process
*/
GT_STATUS cpssMultiProcComGetRxTxCommand
(
		CPSS_RX_TX_PROC_COM_CMD_STC *cmdParamPtr
)
{

		cmdParamPtr->valid = PRV_SHARED_DB.rxTxGaltisCommands.valid;
		cmdParamPtr->mpCmdCpssRxPktClearTbl = PRV_SHARED_DB.rxTxGaltisCommands.mpCmdCpssRxPktClearTbl;

		return GT_OK;
}

/**
* @internal cpssMultiProcComSetRxTxCommand function
* @endinternal
*
* @brief   Used to set if the Galtis command cmdCpssRxPktClearTbl was run by appDemo process
*/
GT_STATUS cpssMultiProcComSetRxTxCommand
(
	CPSS_RX_TX_PROC_COM_CMD_STC *cmdParamPtr
)
{
	PRV_SHARED_DB.rxTxGaltisCommands.valid = cmdParamPtr->valid;
	PRV_SHARED_DB.rxTxGaltisCommands.mpCmdCpssRxPktClearTbl = cmdParamPtr->mpCmdCpssRxPktClearTbl;

	return GT_OK;
}


/**************** SMI ETH port transmit *************/

static CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC txKeepSmiTransmitParams;


GT_STATUS cpssMultiProcComGetSmiTransmitParams
(
	CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC *paramPtr
)
{
	paramPtr->appDemoSysConfig = txKeepSmiTransmitParams.appDemoSysConfig;
	paramPtr->valid = txKeepSmiTransmitParams.valid;

	return 0;
}

GT_STATUS cpssMultiProcComSetSmiTransmitParams
(
	CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC *paramPtr
)
{
	txKeepSmiTransmitParams.appDemoSysConfig = paramPtr->appDemoSysConfig;
	txKeepSmiTransmitParams.valid = paramPtr->valid;

	return 0;
}



#endif
#endif



