
/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortApCtrlDiagIf.c
*
* @brief
*
* @version   42
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApLogDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include "../portCtrl/h/mvHwsPortCtrlAp.h"
#include "../portCtrl/h/mvHwsPortCtrlAn.h"
#include "../portCtrl/h/mvHwsPortCtrlDefines.h"
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

#if defined(CPU_BE)
#include <cpssCommon/cpssPresteraDefs.h>
#endif


/**************************** Pre-Declaration ********************************************/

void mvApPortCtrlDebugInfoShowEntry(GT_U32 port, GT_U32 timestamp, GT_U32 info, MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT output);
void mvApPortCtrlDebugInfoPrint(char *buff, MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT output);

/**************************** Definition **************************************/

/**************************** Globals *****************************************/
/* Port Log Control Parameters */
GT_U32 mvHwsFwAPLogOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwAPLogCountOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwAPLogPointerOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwAPLogResetOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwAPLogPointer[SRVCPU_MAX_IPC_CHANNEL];

/* Port Log Control Parameters */
GT_U32 mvHwsFwLogOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwLogCountOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwLogPointerOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwLogResetOffsetAddr[SRVCPU_MAX_IPC_CHANNEL];
GT_U32 mvHwsFwLogPointer[SRVCPU_MAX_IPC_CHANNEL];

/* Port Log */
MV_HWS_PORT_CTRL_LOG_ENTRY portLog[MV_PORT_CTRL_LOG_SIZE+MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE];
/* FW Hws Log */
MV_HWS_LOG_ENTRY hwsLog[MV_FW_HWS_LOG_SIZE];

#ifdef AP_PORT_SM /*(M5)*/
/* AP Registers description */
char *regType[PORT_CTRL_MAX_AP_REGS] =
{
    "",   /* the values in MV_PORT_CTRL_AP_REGS enum started from 1 */
    /* [PORT_CTRL_AP_REG_CFG_0] */                                   "Cfg-0",
    /* [PORT_CTRL_AP_REG_CFG_1] */                                   "Cfg-1",
    /* [PORT_CTRL_AP_REG_ST_0] */                                    "St-0",
    /* [PORT_CTRL_AP_INT_REG_802_3_AP_CTRL] */                       "Ctrl",
    /* [PORT_CTRL_AP_INT_REG_802_3_AP_ST] */                         "St",
    /* [PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_1] */                  "AdvReg1",
    /* [PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_2] */                  "AdvReg2",
    /* [PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3] */                  "AdvReg3",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1] */         "LpReg1",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2] */         "LpReg2",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3] */         "LpReg3",
    /* [PORT_CTRL_AP_INT_REG_ANEG_CTRL_0] */                         "Aneg0",
    /* [PORT_CTRL_AP_INT_REG_ANEG_CTRL_1] */                         "Aneg1",
    /* [PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG] */                "NxtPg",
    /* [PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15] */          "NxtPg-0",
    /* [PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31] */         "NxtPg-16",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG] */        "LpNxtPg",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15] */  "LpNxtPg-0",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31] */ "LpNxtPg-16",
    /* [PORT_CTRL_AP_INT_REG_802_3_BP_ETH_ST_REG] */                 "BpSt",
    /* [PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG] */                    "LpAdv",
    /* [PORT_CTRL_AP_REG_ST_1] */                                    "St-1",
    /* [PORT_CTRL_AP_REG_CFG_2] */                                   "Cfg-2",
    /* [PORT_CTRL_AP_REG_O_CORE] */                                  "O_CoreStatus:"
};
#endif /*AP_PORT */

/* Message Type description */
char *msgType[MV_HWS_IPC_LAST_CTRL_MSG_TYPE] =
{
   /* [MV_HWS_IPC_PORT_AP_ENABLE_MSG] */                    "AP Enable",
   /* [MV_HWS_IPC_PORT_AP_DISABLE_MSG] */                   "AP Disable",
   /* [MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG] */             "AP Sys Cfg",
   /* [MV_HWS_IPC_PORT_AP_CFG_GET_MSG] */                   "AP Cfg Get",
   /* [MV_HWS_IPC_PORT_AP_STATUS_MSG] */                    "AP Status",
   /* [MV_HWS_IPC_PORT_AP_STATS_MSG] */                     "AP Stats",
   /* [MV_HWS_IPC_PORT_AP_STATS_RESET_MSG] */               "AP Reset",
   /* [MV_HWS_IPC_PORT_AP_INTROP_GET_MSG] */                "AP Introp Get",
   /* [MV_HWS_IPC_PORT_AP_INTROP_SET_MSG] */                "AP Introp Set",
   /* [MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG] */                 "AP Debug",
   /* [MV_HWS_IPC_PORT_PARAMS_MSG] */                       "Params Set",
   /* [MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG] */            "Avago Serdes Init",
   /* [MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG] */                "Avago GUI Set",
   /* [MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG]  */     "Avago Appl Data Reset",
   /* [MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG]  */         "VOS Override MSG",
   /* [MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG]  */            "VOS Override Mode Set",
   /* [MV_HWS_IPC_NOOP_MSG] */                              "Noop",
   /* [MV_HWS_IPC_PORT_INIT_MSG] */                         "Init",
   /* [MV_HWS_IPC_PORT_RESET_MSG] */                        "Reset",
   /* [MV_HWS_IPC_PORT_RESET_EXT_MSG] */                    "Reset Ext",
   /* [MV_HWS_IPC_HWS_LOG_GET_MSG] */                       "Hws LOG Get",
   /* [MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG] */            "Auto Tune Set",
   /* [MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG] */               "Auto Tune Stop",
   /* [MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG] */          "Auto Tune Check",
   /* [MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG] */              "Link Status",
   /* [MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG] */               "Fec Get",
   /* [MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG] */                "Tx Enable Get",
   /* [MV_HWS_IPC_PORT_SERDES_RESET_MSG] */                 "Serdes Reset",
   /* [MV_HWS_IPC_PORT_POLARITY_SET_MSG] */                 "Polarity",
   /* [MV_HWS_IPC_PORT_FEC_CONFIG_MSG] */                   "Fec Config",
   /* [MV_HWS_IPC_PORT_TX_ENABLE_MSG] */                    "Tx Enable",
   /* [MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG] */            "Signal Detect",
   /* [MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG] */          "CDR Lock Status",
   /* [MV_HWS_IPC_PORT_LOOPBACK_SET_MSG] */                 "Loopback",
   /* [MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG] */          "Loopback Status",
   /* [MV_HWS_IPC_PORT_PPM_SET_MSG] */                      "PPM Set",
   /* [MV_HWS_IPC_PORT_PPM_GET_MSG] */                      "PPM Get",
   /* [MV_HWS_IPC_PORT_IF_GET_MSG] */                       "If Get",
   /* [MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG] */      "Serdes Man Rx",
   /* [MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG] */      "Serdes Man Tx",
   /* [MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG] */ "PCS Act Status",
   /* [MV_HWS_IPC_PORT_FC_STATE_SET_MSG]  */                "FC State Set",
   /* [MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG] */              "Port Enable Ctrl Set",
   /* [MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG] */              "Port Enable Ctrl Get",
   /* [MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG] */ "Port Serdes TX Parameters Offset Set",
   /* [MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG] */    "Port Serdes RX Parameters Override Set",
   /* [MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_MSG] */         "Port Adaptive CTLE Enable Set",
   /* [MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_MSG] */            "Non AP Port Link Up Status Set",
   /* [MV_HWS_IPC_PORT_LINK_BINDING_SET_MSG] */             "Port Link Binding Set",
   /* [MV_HWS_IPC_PORT_LINK_BINDING_REGISTER_AP_MSG] */     "Port Link Binding Reg AP",
   /* [MV_HWS_IPC_PORT_OPERATION_MSG] */                    "Port Operation",
   /* [MV_HWS_IPC_PORT_SERDES_TX_CONFIG_MSG] */             "Port Serdes TX configuration",
   /* [MV_HWS_IPC_PORT_SERDES_RX_CONFIG_MSG] */             "Port Serdes RX configuration",
   /* [MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG] */           "Port Enable Wait Ack",
   /* [MV_HWS_IPC_AN_RES_BIT_SET_MSG] */                    "An Resolution Bit Set",


};

/* Port Type description */
char *portType[] =
{
    "Reg",
    "AP"
};

/* Supervisour state description */
char *superState[] =
{
    "High Pri Msg",
    "Low Pri Msg",
    "Delay"
};

/* Port Management state description */
char *portMngState[] =
{
    "Msg Process",
    "Delay"
};

/* Port State machine state description */
char *portMngSmState[] =
{
    "Idle",
    "Serdes Config",
    "Serdes Training",
    "Mac/Pcs Config",
    "Active",
    "Delete"
};

#if (defined(AP_PORT_SM) /*(M4)*/ || defined(M3_PORT_SM)/*M3*/)
/* Port State machine status description */
char *portMngSmStatus[] =
{
    "Not Running",
    "Start Execute",
    "Serdes Config In Progress",
    "Serdes Config Success",
    "Serdes Config Failure",
    "Serdes Training In Progress",
    "Serdes Training Success",
    "Serdes Training Failure",
    "Serdes Training Config Failure",
    "Serdes Training End Wait for Ack",
    "Mac/Pcs Config In Progress",
    "Mac/Pcs Config Success",
    "Mac/Pcs Config Failure",
    "Delete In Progress",
    "Delete Success",
    "Delete Failure",
    "FEC Config In Progress",
    "FEC Config Success",
    "FEC Config Failure",
    "FC State Set In Progress",
    "FC State Set Success",
    "FC State Set Failure",
    "Serdes Training Not Ready",
    "Serdes Polarity Set In Progress",
    "Serdes Polarity Set Success",
    "Serdes Polarity Set Failure",
    "Serdes Training End Received Ack",
    "Port Enable Failure",
    "Serdes Adaptive Training Start Failure",
    "Serdes Enhance Tune Lite Failure",
    "Serdes Enhance Tune Lite In Progress",
    "Serdes Enhance Tune Lite Success",
    "Serdes CTLE or ETL config Failure",
    "Serdes Adaptive Rx Training Success"

};
#endif /*(defined(AP_PORT)|| defined(M3_PORT_SM))*/
/* AP HCD Found description */
char* apPortMngHcdFound[] =
{
    "Not Found",
    "Found"
};

/* AP HCD Type description */
char* apPortMngHcdType[Port_AP_LAST] =
{
    "1000Base KX",
    "10GBase KX4",
    "10GBase KR",
    "25GBASE_KR_S",
    "25GBASE_KR",
    "40GBase KR4",
    "40GBASE_CR4",
    "100GBASE_CR10",
    "100GBASE_KP4",
    "100GBASE_KR4",
    "100GBASE_CR4",
    "25GBASE_KR1_CONSORTIUM",
    "25GBASE_CR1_CONSORTIUM",
    "50GBASE_KR2_CONSORTIUM",
    "50GBASE_CR2_CONSORTIUM",
    "25GBASE_CR_S",
    "25GBASE_CR"
};

/* AP General filed description */
char* apPortMngGen[] =
{
    "Off",
    "On"
};
/* AP Port State machine description */
char *apPortMngSmState[] =
{
    "Idle",
    "Init",
    "Tx Disable",
    "Resolution",
    "Active",
    "Delete",
    "Disable",
    "Max" /* since state machine enum has values 0-6 the description should contain matching list */
};

/* AP Port State machine status description */
char *apPortMngSmStatus[] =
{
    "Start Execute",
    "Serdes Init Failure",
    "Init Success",
    "Tx Disable In Progress",
    "Tx Disable Failure",
    "Tx Disable Success",
    "Resolution In Progress",
    "Parallel Detect Resolution In Progress",
    "Parallel Detect Resolution Failure",
    "AP Resolution In Progress",
    "AP No Resolution",
    "AP Resolution Timer Failure",
    "AP Resolution Max Interval Failure",
    "Resolution Success",
    "Link Check Start",
    "Link Check Validate",
    "Link Check Failure",
    "Link Check Success",
    "Delete In Progress",
    "Delete Success",
    "Unknown HCD Found",
    "AP Resolution check consortium",
    "AP Resolution check ACK2",
    "AP Resolution no more pages",
    "Msg Send Failure",
    "Adaptive CTLE init",
    "Adaptive CTLE running",
    "AP port enable wait"
};


/* AP Port State machine description */
char *anPortSmState[] =
{
    "Idle",
    "An",
    "Serdes Configuration",
    "Link Status",
    "Delete",
    "Disable",
    "Max" /* since state machine enum has values 0-6 the description should contain matching list */
};

/* AP Port State machine status description */
char *anPortSmStatus[] =
{
    "Start Execute",
    "Serdes Init Failure",
    "Init Success",
    "Set An Parameters",
    "Resolution In Progress",
    "Resolution Success",
    "Resolution Failure",
    "Serdes Config In Progress",
    "Serdes Config Success",
    "Serdes Config Failure",
    "Serdes Training In Progress",
    "Serdes Training Not Ready",
    "Serdes Training Success",
    "Serdes Training Failure",
    "Serdes Training End Wait for Ack",
    "Serdes Training End Received Ack",
    "Mac/Pcs Config In Progress",
    "Mac/Pcs Config Success",
    "Mac/Pcs Config Failure",
    "FEC Config In Progress",
    "FEC Config Success",
    "FEC Config Failure",
    "Port Enable Failure",
    "FC State Set Success",
    "FC State Set Failure",
    "Assert Link in progress",
    "Assert Link in Success",
    "Assert Link in Failure",
    "complete in progress",
    "complete in Success",
    "complete in Failure",
    "Link Check Success",
    "Link Check Failure",
    "Disable In Progress",
    "Disable Success",
    "Delete In Progress",
    "Delete Success",
    "Delete Failure",
    "Serdes Power Up",
    "Resolution Check Consortium",
    "Resolution Check Ack2",
    "Resolution No More Pages",
    "Enable Wait",
    "Serdes Training wait",
};

char *anPortSedesDebug[] =
{
    "oCore status:",
    "oCore2 status:",
    "DFE status:",
    "EO status:",
    "TAP2 status:",
    "resolution found",
    "PCAL started",
    "Operation was called"
};



/**
* @internal mvHwsApPortCtrlDebugParamsSet function
* @endinternal
* @brief   Set AP debug information
*
* @param[in] apFwLogBaseAddr          - AP Firmware Real-time log base address
* @param[in] apFwLogCountAddr         - AP Firmware Real-time log count address
* @param[in] apFwLogPointerAddr       - AP Firmware Real-time log pointer address
* @param[in] apFwLogResetAddr         - AP Firmware Real-time log reset address
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsApPortCtrlDebugParamsSet
(
    GT_U32 cpuId,
    GT_U32 apFwLogBaseAddr,
    GT_U32 apFwLogCountAddr,
    GT_U32 apFwLogPointerAddr,
    GT_U32 apFwLogResetAddr
)
{
    if (cpuId < SRVCPU_MAX_IPC_CHANNEL)
    {
        mvHwsFwAPLogOffsetAddr[cpuId]        = apFwLogBaseAddr;
        mvHwsFwAPLogCountOffsetAddr[cpuId]   = apFwLogCountAddr;
        mvHwsFwAPLogPointerOffsetAddr[cpuId] = apFwLogPointerAddr;
        mvHwsFwAPLogResetOffsetAddr[cpuId]   = apFwLogResetAddr;
        mvHwsFwAPLogPointer[cpuId]           = 0;
    }
}

/**
* @internal mvHwsLogParamsSet function
* @endinternal
*
* @brief   Set Hws log information
*
* @param[in] fwLogBaseAddr            - Firmware HWS Real-time log base address
* @param[in] fwLogCountAddr           - Firmware HWS Real-time log count address
* @param[in] fwLogPointerAddr         - Firmware HWS Real-time log pointer address
* @param[in] fwLogResetAddr           - Firmware HWS Real-time log reset address
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsLogParamsSet
(
    GT_U32 cpuId,
    GT_U32 fwLogBaseAddr,
    GT_U32 fwLogCountAddr,
    GT_U32 fwLogPointerAddr,
    GT_U32 fwLogResetAddr
)
{
    if (cpuId < SRVCPU_MAX_IPC_CHANNEL)
    {
        mvHwsFwLogOffsetAddr[cpuId] = fwLogBaseAddr;
        mvHwsFwLogCountOffsetAddr[cpuId]   = fwLogCountAddr;
        mvHwsFwLogPointerOffsetAddr[cpuId] = fwLogPointerAddr;
        mvHwsFwLogResetOffsetAddr[cpuId]   = fwLogResetAddr;
        mvHwsFwLogPointer[cpuId]           = 0;
    }
}
#ifdef AP_PORT_SM /*(M5)*/
/**
* @internal arbSmStateDesc function
* @endinternal
*
* @brief   Print AP ARB State machine status description
*/
void arbSmStateDesc(GT_U32 mask, GT_U32 statusId, MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT output)
{
    char buffer[50] = {0};

    if(statusId == AP_PORT_MSG_SEND_FAILURE)
    {
        if(mask == MV_HWS_IPC_PORT_RESET_MSG)
        {
            hwsSprintf(buffer,"RESET\n");
        }
        else if (mask == MV_HWS_IPC_PORT_INIT_MSG)
        {
            hwsSprintf(buffer,"INIT\n");
        }
        else
        {
            hwsSprintf(buffer,"\n");
        }
    }
    else
    {
        if (mask != 0)
        {
            if      (mask & ST_AN_ENABLE)    hwsSprintf(buffer,"ARB Status AN_ENABLE\n");
            else if (mask & ST_TX_DISABLE)   hwsSprintf(buffer,"ARB Status TX_DISABLE\n");
            else if (mask & ST_LINK_STAT_CK) hwsSprintf(buffer,"ARB Status LINK_STATE_CHECK\n");
            else if (mask & ST_PARALLEL_FLT) hwsSprintf(buffer,"ARB Status PARALLEL_FLT\n");
            else if (mask & ST_ABILITY_DET)  hwsSprintf(buffer,"ARB Status ABILITY_DET\n");
            else if (mask & ST_ACK_DETECT)   hwsSprintf(buffer,"ARB Status ST_ACK_DETECT\n");
            else if (mask & ST_COMPLETE_ACK) hwsSprintf(buffer,"ARB Status ST_COMPLETE_ACK\n");
            else if (mask & ST_NP_WAIT)      hwsSprintf(buffer,"ARB Status ST_NP_WAIT\n");
            else if (mask & ST_AN_GOOD_CK)   hwsSprintf(buffer,"ARB Status AN_GOOD_CK\n");
            else if (mask & ST_AN_GOOD)      hwsSprintf(buffer,"ARB Status AN_GOOD\n");
            else if (mask & ST_SERDES_WAIT)  hwsSprintf(buffer,"ARB Status SERDES WAIT\n");
        }
        else
        {
            hwsSprintf(buffer,"\n");
        }
    }

    mvApPortCtrlDebugInfoPrint(buffer, output);

}


/**
* @internal mvApPortCtrlStatsShow function
* @endinternal
*
* @brief   Print AP port statistics information stored in system
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlStatsShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
)
{
    MV_HWS_AP_PORT_STATS apStats;

    CHECK_STATUS(mvHwsApPortCtrlStatsGet(devNum, portGroup, apPortNum, &apStats));

    hwsOsPrintf("======== AP Port %d stats ========\n", apPortNum);
    hwsOsPrintf("Tx Disable cnt        %08d\n", apStats.txDisCnt);
    hwsOsPrintf("Ability check cnt     %08d\n", apStats.abilityCnt);
    hwsOsPrintf("Resolution Detect cnt %08d\n", apStats.abilitySuccessCnt);
    hwsOsPrintf("Link Fail cnt         %08d\n", apStats.linkFailCnt);
    hwsOsPrintf("Link Success cnt      %08d\n", apStats.linkSuccessCnt);
    hwsOsPrintf("HCD Time              %08d\n", apStats.hcdResoultionTime);
    hwsOsPrintf("link Up Time          %08d\n", apStats.linkUpTime);
    hwsOsPrintf("Total Exec Time       %08d\n", apStats.hcdResoultionTime + apStats.linkUpTime);

    return GT_OK;
}


#endif /*AP_PORT*/
/**
* @internal mvApPortCtrlStatusShow function
* @endinternal
*
* @brief   Print AP port status information stored in system.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlStatusShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
)
{
    MV_HWS_AP_PORT_STATUS apStatus;

    CHECK_STATUS(mvHwsApPortCtrlStatusGet(devNum, portGroup, apPortNum, &apStatus));

    hwsOsPrintf("======== AP Port %d, Lane %d info ========\n", apPortNum, apStatus.apLaneNum);
    hwsOsPrintf("AP Port State           - %s\n", apPortMngSmState[apStatus.smState]);
    hwsOsPrintf("AP Port Status          - %s\n", apPortMngSmStatus[apStatus.smStatus]);
    hwsOsPrintf("AP Port ARB State       - ");

#ifdef AP_PORT_SM /*(M5)*/
    switch (apStatus.arbStatus)
    {
    case ST_AN_ENABLE:    hwsOsPrintf("ST_AN_ENABLE\n");    break;
    case ST_TX_DISABLE:   hwsOsPrintf("ST_TX_DISABLE\n");   break;
    case ST_LINK_STAT_CK: hwsOsPrintf("ST_LINK_STAT_CK\n"); break;
    case ST_PARALLEL_FLT: hwsOsPrintf("ST_PARALLEL_FLT\n"); break;
    case ST_ABILITY_DET:  hwsOsPrintf("ST_ABILITY_DET\n");  break;
    case ST_ACK_DETECT:   hwsOsPrintf("ST_ACK_DETECT\n");   break;
    case ST_COMPLETE_ACK: hwsOsPrintf("ST_COMPLETE_ACK\n"); break;
    case ST_NP_WAIT:      hwsOsPrintf("ST_NP_WAIT\n");      break;
    case ST_AN_GOOD_CK:   hwsOsPrintf("ST_AN_GOOD_CK\n");   break;
    case ST_AN_GOOD:      hwsOsPrintf("ST_AN_GOOD\n");      break;
    case ST_SERDES_WAIT:  hwsOsPrintf("ST_SERDES_WAIT\n");  break;
    default:              hwsOsPrintf("Unknown\n");         break;
    }
#endif
    hwsOsPrintf("AP Port HCD             - %s\n", (apStatus.hcdResult.hcdFound) ? ("Found") : ("Not Found."));
    hwsOsPrintf("AP Port HCD Type        - ");
    switch (apStatus.postApPortMode)
    {
    case _100GBase_KR4: hwsOsPrintf("100GBase KR4\n"); break;
    case _40GBase_KR4:   hwsOsPrintf("40GBase KR4\n"); break;
    case _10GBase_KR:   hwsOsPrintf("10GBase KR\n");  break;
    case _10GBase_KX4:  hwsOsPrintf("10GBase KX4\n"); break;
    case _1000Base_X:   hwsOsPrintf("1000Base KX\n"); break;
    case _25GBase_KR:   hwsOsPrintf("25GBase KR\n"); break;
    case _25GBase_KR_S: hwsOsPrintf("25GBase KR-S\n"); break;
    case _25GBase_KR_C: hwsOsPrintf("25GBase KR consortium\n"); break;
    case _50GBase_KR2_C:hwsOsPrintf("50GBase KR2 consortium\n"); break;
    case _40GBase_CR4:  hwsOsPrintf("40GBase CR4\n"); break;
    case _100GBase_CR4: hwsOsPrintf("100GBase CR4\n"); break;
    case _25GBase_CR:   hwsOsPrintf("25GBase CR\n"); break;
    case _25GBase_CR_S: hwsOsPrintf("25GBase CR-S\n"); break;
    case _25GBase_CR_C: hwsOsPrintf("25GBase CR consortium\n"); break;
    case _50GBase_CR2_C:hwsOsPrintf("50GBase CR2 consortium\n"); break;
    case _50GBase_KR  : hwsOsPrintf("50GBase KR\n"); break;
    case _100GBase_KR2: hwsOsPrintf("100GBase KR2\n"); break;
    case _200GBase_KR4: hwsOsPrintf("200GBase KR4\n"); break;
    case _200GBase_KR8: hwsOsPrintf("200GBase KR8\n"); break;
    case _400GBase_KR8: hwsOsPrintf("400GBase KR8\n"); break;
    default:            hwsOsPrintf("Unknown\n");     break;
    }

    hwsOsPrintf("AP Port HCD FEC Result  - %s\n", (apStatus.hcdResult.hcdFecEn)       ? ("FEC enabled") : ("FEC disabled"));
    /* hcdFecEn used for modes which use old FEC advertisement/resolution mechanism like 10G_KR and 100G_KR4 */
    hwsOsPrintf("AP Port HCD FEC Type    - %s\n", (AP_ST_HCD_FEC_RES_NONE == apStatus.hcdResult.hcdFecType) ? ("FC NONE ")
                                                : ((apStatus.hcdResult.hcdFecType == AP_ST_HCD_FEC_RES_FC) ? ("FC FEC ") : ("RS FEC")));
    hwsOsPrintf("AP Port HCD FC Rx Pause - %s\n", (apStatus.hcdResult.hcdFcRxPauseEn) ? ("FC Rx enabled") : ("FC Rx disabled"));
    hwsOsPrintf("AP Port HCD FC Tx Pause - %s\n", (apStatus.hcdResult.hcdFcTxPauseEn) ? ("FC Tx enabled") : ("FC Tx disabled"));
    hwsOsPrintf("AP Port HCD Link Status - %s\n", (apStatus.hcdResult.hcdLinkStatus)  ? ("Link Up") : ("Link Down"));

    return GT_OK;
}

/**
* @internal mvApPortCtrlIntropShow function
* @endinternal
*
* @brief   Print AP port introp information stored in system
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlIntropShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
)
{
    MV_HWS_AP_PORT_INTROP apIntrop;

    CHECK_STATUS(mvHwsApPortCtrlIntropGet(devNum, portGroup, apPortNum, &apIntrop));

    hwsOsPrintf("======== AP Introp Parameters  ========\n");
#ifdef AP_PORT_SM /*(M5)*/
    hwsOsPrintf("Tx Disable duration        %08d msec\n",      apIntrop.txDisDuration);
    hwsOsPrintf("Ability duration           %08d msec\n",      apIntrop.abilityDuration);
    hwsOsPrintf("Ability max interval       %08d intervals\n", apIntrop.abilityMaxInterval);
    hwsOsPrintf("Ability fail max interval  %08d intervals\n", apIntrop.abilityFailMaxInterval);
    hwsOsPrintf("AP Link Duration           %08d msec\n",      apIntrop.apLinkDuration);
    hwsOsPrintf("AP Link Max interval check %08d intervals\n", apIntrop.apLinkMaxInterval);
    hwsOsPrintf("PD Link Duration           %08d msec\n",      apIntrop.pdLinkDuration);
    hwsOsPrintf("PD Link Max interval check %08d intervals\n", apIntrop.pdLinkMaxInterval);
#endif /*AP_PORT*/

#ifdef AN_PORT_SM /*(M7)*/
    hwsOsPrintf("Ability duration           %08d msec\n",      apIntrop.abilityDuration);
    hwsOsPrintf("Ability max interval       %08d intervals\n", apIntrop.abilityMaxInterval);
    hwsOsPrintf("AP Link Duration           %08d msec\n",      apIntrop.apLinkDuration);
    hwsOsPrintf("AP Link Max interval check %08d intervals\n", apIntrop.apLinkMaxInterval);
    hwsOsPrintf("PD Link Duration           %08d msec\n",      apIntrop.pdLinkDuration);
    hwsOsPrintf("PD Link Max interval check %08d intervals\n", apIntrop.pdLinkMaxInterval);
    hwsOsPrintf("AP Link Max interval PAM4  %08d intervals\n", apIntrop.anPam4LinkMaxInterval);
#endif /*AN_PORT_SM*/
    return GT_OK;
}
extern GT_STATUS prvCpssDrvHwPpPortGroupReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
);

/**
* @internal mvApPortLogDump function
* @endinternal
*
* @brief   Print AP port log without using ipc
*
* @param[in] chipIndex                   - chipIndex
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortLogDump
(
    GT_U8     chipIndex
)
{
    GT_U32 i,address, timestamp,info,count;
    if(hwsDeviceSpecInfo[0].devType != Falcon)
        return GT_FAIL;

    address = (chipIndex/4) * 0x20000000;

     if( (chipIndex % 8) < 4 )
     {
         address += (chipIndex % 4) * 0x01000000;
     } else {
         address += (4 - (chipIndex % 4) - 1) * 0x01000000;
     }
     address +=  0x380000;
     prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,mvHwsFwAPLogCountOffsetAddr[chipIndex]+address,0xFFFFFFFF, &count);
     /*hwsRegisterGetFuncPtr(0, 0, mvHwsFwAPLogCountOffsetAddr[chipIndex]+address, &count, 0xFFFFFFFF);*/
     hwsOsPrintf("logCount address: 0x%x count %d\n",mvHwsFwAPLogCountOffsetAddr[chipIndex],count);
     prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,mvHwsFwAPLogPointerOffsetAddr[chipIndex]+address,0xFFFFFFFF, &count);
     /*hwsRegisterGetFuncPtr(0, 0, mvHwsFwAPLogPointerOffsetAddr[chipIndex]+address, &count, 0xFFFFFFFF);*/
     hwsOsPrintf("logOffset address: 0x%x offset %d\n",mvHwsFwAPLogPointerOffsetAddr[chipIndex],count);

     address += mvHwsFwAPLogOffsetAddr[chipIndex];
     hwsOsPrintf("portLog address: 0x%x (0x%x)\n",mvHwsFwAPLogOffsetAddr[chipIndex],address);

    for (i = 0; i <  MV_PORT_CTRL_LOG_SIZE; i++)
    {
        prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,address+i*8,0xFFFFFFFF, &timestamp);
        prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,address+i*8+4,0xFFFFFFFF, &info);
        /*hwsRegisterGetFuncPtr(0, 0, address+i*8, &timestamp, 0xFFFFFFFF);
        hwsRegisterGetFuncPtr(0, 0, address+i*8+4, &info, 0xFFFFFFFF);*/
        if ( timestamp == 0 ) {
            continue;
        }
        mvApPortCtrlDebugInfoShowEntry(0xffff, timestamp, info, 0);
    }
   /* print extra data ftom ap in falcon*/
    if(hwsDeviceSpecInfo[0].devType == Falcon)
    {

        /*read extra status log*/
        for (i = 0; i < MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-2; i++)
        {
            prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8,0xFFFFFFFF, &timestamp);
            prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8+4,0xFFFFFFFF, &info);
            /*hwsRegisterGetFuncPtr(0, 0, address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8, &timestamp, 0xFFFFFFFF);
            hwsRegisterGetFuncPtr(0, 0, address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8+4, &info, 0xFFFFFFFF);*/
            hwsOsPrintf("%08d: AN SM   Port %d state %d status %d\n",
                       timestamp,
                       i,
                       ((info>>8)&0xFF), (info&0xFF));
        }
        /* global time stamp */
        prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8,0xFFFFFFFF, &timestamp);
        prvCpssDrvHwPpPortGroupReadRegBitMask(0,0,address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8+4,0xFFFFFFFF, &info);
 /*       hwsRegisterGetFuncPtr(0, 0, address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8, &timestamp, 0xFFFFFFFF);
        hwsRegisterGetFuncPtr(0, 0, address+ MV_PORT_CTRL_LOG_SIZE*8 +i*8+4, &info, 0xFFFFFFFF);*/
        hwsOsPrintf("%08d: AN SM   CM3 time stamp %d\n",
                   timestamp,
                   info);

    }

    return GT_OK;
}

/**
* @internal mvApPortCtrlDebugInfoShow function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @param[in] devNum                 - system device number
* @param[in] apPortNum              - AP port number
* @param[in] output                 - enum indicate log output
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlDebugInfoShow
(
    GT_U8                           devNum,
    GT_U32                          apPortNum,
    MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT  output
)
{
    GT_U32 logIndex;
    GT_U32 logCount=0;
    GT_U32 logPointer=0;
    GT_U32 logReset=0;
    GT_U32 logActiveCount;
    GT_U32 logActivePointer;
    GT_U32 logInfo;
    GT_U32 timestamp, cpuId = 0;
    GT_U8 chipIndex;
    char buffer[200] = {0};

   /* GT_UINTPTR fwChannel = PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcChannelHandlers)[devNum];*/
#if defined(CPU_BE)
    int    i;
#endif
    if(hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        if ((apPortNum & 0xFFFF) != LOG_ALL_PORT_DUMP)
        {
            cpuId = apPortNum / 16;
            apPortNum = apPortNum % 16;
        }
        else
        {
            cpuId = apPortNum>>16;
            apPortNum = LOG_ALL_PORT_DUMP;
        }
        chipIndex = (GT_U8)((16 * devNum) + cpuId);
    }
    else
    {
        chipIndex =  (GT_U8)(devNum + cpuId);
    }

    /* Read log control parameters */
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwAPLogCountOffsetAddr[chipIndex],   (unsigned int*)&logCount,   4);
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwAPLogPointerOffsetAddr[chipIndex], (unsigned int*)&logPointer, 4);
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwAPLogResetOffsetAddr[chipIndex],   (unsigned int*)&logReset,   4);
    /* Read log content */
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwAPLogOffsetAddr[chipIndex] , (unsigned int*)portLog, sizeof(portLog));
#if defined(CPU_BE)
    logCount = CPSS_32BIT_LE(logCount);
    logPointer = CPSS_32BIT_LE(logPointer);
    logReset = CPSS_32BIT_LE(logReset);
    for (i = 0; i < MV_PORT_CTRL_LOG_SIZE; i++)
    {
        portLog[i].timestamp = CPSS_32BIT_LE(portLog[i].timestamp);
        portLog[i].info = CPSS_32BIT_LE(portLog[i].info);
    }
#endif
    /* Mark log count reset (by Service CPU) */
    logReset++;
#if defined(CPU_BE)
    logReset = CPSS_32BIT_LE(logReset);
#endif
    prvCpssGenericSrvCpuWriteCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwAPLogResetOffsetAddr[chipIndex] , (unsigned int*)&logReset, 4);

    /* Process Log */
    /* =========== */
    logActiveCount   = logCount;
    logActivePointer = logPointer;

    /* Adjust log control parameters */
    if (logActivePointer >= MV_PORT_CTRL_LOG_SIZE)
    {
        logActivePointer = 0;
    }

    if (logActiveCount >= MV_PORT_CTRL_LOG_SIZE)
    {
        logActiveCount = MV_PORT_CTRL_LOG_SIZE;
    }

    /* The pointer overlapped and we need to read from the start of the newer
       entries which is prior to the overlapping */
    if (logActiveCount > logActivePointer)
    {
        logActivePointer = MV_PORT_CTRL_LOG_SIZE - (logActiveCount - logActivePointer);
    }
    /* Set pointer to the first of the newer lines */
    else
    {
        logActivePointer = logActivePointer - logActiveCount;
    }

    /* Dump log */
    if ((mvHwsFwAPLogPointer[chipIndex] == logPointer) && (output == MV_HWS_AP_DEBUG_LOG_OUTPUT_CPSS_LOG_E))/*No new records*/
    {
        hwsSprintf(buffer,"\nPort Control Realtime Log - No new records to read\n");
        logActiveCount = 0;
    }
    else
    {
        hwsSprintf(buffer,"\nPort Control Realtime Log CpuID %d\n"
                          "=========================\n"
                          "Num of log entries  %03d\n"
                          "Current entry index %03d\n",
                          cpuId, logActiveCount, logActivePointer);
    }
    mvApPortCtrlDebugInfoPrint(buffer, output);

    mvHwsFwAPLogPointer[chipIndex]  = logPointer;

    for (logIndex = 0; logIndex < logActiveCount; logIndex++)
    {
        logInfo   = portLog[logActivePointer].info;
        timestamp = portLog[logActivePointer].timestamp;

        mvApPortCtrlDebugInfoShowEntry(apPortNum, timestamp, logInfo, output);
        logActivePointer++;
        if (logActivePointer >= MV_PORT_CTRL_LOG_SIZE)
        {
            logActivePointer = 0;
        }
    }

    /* print extra data ftom ap in falcon*/
    if(hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        /*read extra status log*/
        for (logIndex = 0; logIndex < MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-2; logIndex++)
        {
            logInfo   = portLog[MV_PORT_CTRL_LOG_SIZE + logIndex].info;
            timestamp = portLog[MV_PORT_CTRL_LOG_SIZE + logIndex].timestamp;
            hwsSprintf(buffer,"%08d: AN SM   Port %d state %d status %d\n",
                               timestamp,
                               logIndex,
                               ((logInfo>>8)&0xFF), (logInfo&0xFF));
             mvApPortCtrlDebugInfoPrint(buffer, output);
        }
        /* global time stamp */
        logInfo   = portLog[MV_PORT_CTRL_LOG_SIZE + MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-2].info;
        timestamp = portLog[MV_PORT_CTRL_LOG_SIZE + MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-2].timestamp;
        hwsSprintf(buffer,"%08d: AN SM   CM3 time stamp %d\n",
                           timestamp,
                           logInfo);
        mvApPortCtrlDebugInfoPrint(buffer, output);

        /*version number*/
        logInfo   = portLog[MV_PORT_CTRL_LOG_SIZE + MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-1].info;
        timestamp = portLog[MV_PORT_CTRL_LOG_SIZE + MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-1].timestamp;
        hwsSprintf(buffer,"%08d: AN SM   AP version number %02x/%02x_%02x\n",
                           0,
                           ((logInfo>>16)&0xFF), ((logInfo>>8)&0xFF), (logInfo&0xFF));
        mvApPortCtrlDebugInfoPrint(buffer, output);
    }

    return GT_OK;
}

/**
* @internal mvHwsLogInfoShow function
* @endinternal
*
* @brief   Print FW Hws real-time log information stored in system
*
* @param[in] devNum             - system device number
* @param[in] cpuId              - cpu Index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogInfoShow
(
    IN GT_U8  devNum,
    IN GT_U32 cpuId
)
{
    GT_U32 iteration=0;
    GT_U32 logPointer=0;
    GT_U32 idx = 0;
    GT_U32 logCount=0;
   /* GT_UINTPTR fwChannel = PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcChannelHandlers)[0];*/
#if defined(CPU_BE)
    int i;
#endif

    /* Read counter */
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwLogCountOffsetAddr[cpuId],   (unsigned int*)&logCount, sizeof(GT_U32));
    /* Read log content */
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwLogOffsetAddr[cpuId], (unsigned int*)hwsLog, sizeof(hwsLog));
    /* Read log pointer */
    prvCpssGenericSrvCpuReadCpuId(devNum, (GT_U8)cpuId/*fwChannel*/, mvHwsFwLogPointerOffsetAddr[cpuId], (unsigned int*)&logPointer, sizeof(GT_U32));
#if defined(CPU_BE)
    logCount = CPSS_32BIT_LE(logCount);
    logPointer = CPSS_32BIT_LE(logPointer);
    for (i = 0; i < MV_FW_HWS_LOG_SIZE; i++)
    {
        hwsLog[i].timestamp = CPSS_32BIT_LE(hwsLog[i].timestamp);
    }
#endif

    hwsOsPrintf("\n HWS Realtime Log (up to %d characters per message)\n"
                "==================================================\n", MV_FW_HWS_LOG_STRING_SIZE);

    /* If there was overlap, start from the current pointer and read whole log (with overlapping) */
    if (logCount >= MV_FW_HWS_LOG_SIZE)
    {
        for (iteration=0, idx=logPointer; iteration < MV_FW_HWS_LOG_SIZE; idx++, iteration++)
        {
            if (idx == MV_FW_HWS_LOG_SIZE )
            {
                idx = 0;
            }
            hwsOsPrintf("[%d] %s \n",
                     hwsLog[idx].timestamp, hwsLog[idx].info);
        }
    }
    else
    {
        /* No overlap happend yet */
        for (idx=0; idx < logPointer; idx++)
        {
            hwsOsPrintf("[%d] %s \n",
                     hwsLog[idx].timestamp, hwsLog[idx].info);
        }
    }

    hwsOsPrintf("\n");

    return GT_OK;
}

/**
* @internal mvApPortCtrlDebugInfoPrint function
* @endinternal
*
* @brief   Print Log content to the selected output
*/
void mvApPortCtrlDebugInfoPrint(char *buff, MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT output)
{
    switch(output)
    {
        case MV_HWS_AP_DEBUG_LOG_OUTPUT_CPSS_LOG_E:
            CPSS_LOG_INFORMATION_MAC(buff);
            break;
        case MV_HWS_AP_DEBUG_LOG_OUTPUT_CPSS_OS_LOG_E:
            cpssOsLog(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ALL_E,buff);
            break;
        case MV_HWS_AP_DEBUG_LOG_OUTPUT_TERMINAL_E:
        default:
            hwsOsPrintf(buff);
            break;
    }
}

/**
* @internal mvApPortCtrlDebugInfoShowEntry function
* @endinternal
*
* @brief   Print Log content according to Log count
*         The order of print is from the oldest to the newest message
*/
void mvApPortCtrlDebugInfoShowEntry(GT_U32 port, GT_U32 timestamp, GT_U32 logInfo, MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT output)
{
    GT_U32 msgId;
    GT_U32 stateId;
    GT_U32 portId;
    GT_U32 regId;
    GT_U32 regVal;
    GT_U32 hcdId;
    GT_U32 arbId;
    GT_U32 statusId;
    GT_U32 dbgFlag;
    char buffer[200] = {0};

    switch (LOG_MOD_GET(logInfo))
    {
    case M0_DEBUG:
        portId  = REG_LOG_PORT_GET(logInfo);
        stateId = REG_LOG_STATE_GET(logInfo);
        regId   = REG_LOG_IDX_GET(logInfo);
        regVal  = REG_LOG_VAL_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (stateId >= AP_PORT_SM_MAX_STATE) ||
             (regId >= PORT_CTRL_MAX_AP_REGS))
        {
            break;
        }
#if 0
        /* Process message
        ** ===============
        ** Print registers in sections:
        **   Cfg-0, Cfg-1, St-0
        **   Ctrl, St, AdvReg1, AdvReg2, AdvReg3
        **   LpReg1, LpReg2, LpReg3, Aneg0, Aneg1
        **   NxtPg, NxtPg-0, NxtPg-16
        **   LpNxtPg, LpNxtPg-0, LpNxtPg-16, BpSt, LpAdv
        */
        if (regId == PORT_CTRL_AP_REG_CFG_0)
        {
            if(apCpssLogEnable == GT_TRUE)
            {
                CPSS_LOG_INFORMATION_MAC("%08d: AP REG  Port %d, State O%d: ", timestamp, portId, stateId);
            }
            else
            {
                hwsOsPrintf("%08d: AP REG  Port %d, State O%d: ", timestamp, portId, stateId);
            }
        }
        else if ((regId == PORT_CTRL_AP_INT_REG_802_3_AP_CTRL) ||
                 (regId == PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1) ||
                 (regId == PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG) ||
                 (regId == PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG))
        {
            if(apCpssLogEnable == GT_TRUE)
            {
                CPSS_LOG_INFORMATION_MAC("%08d: AP REG  Port %d, State O%d, AP Int: ", timestamp, portId, stateId);
            }
            else
            {
                hwsOsPrintf("%08d: AP REG  Port %d, State O%d, AP Int: ", timestamp, portId, stateId);
            }
        }

        if(apCpssLogEnable == GT_TRUE)
        {
            CPSS_LOG_INFORMATION_MAC("%s 0x%04x ", regType[regId], regVal);
        }
        else
        {
            hwsOsPrintf("%s 0x%04x ", regType[regId], regVal);
        }

        if ((regId == PORT_CTRL_AP_REG_ST_0) ||
            (regId == PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3) ||
            (regId == PORT_CTRL_AP_INT_REG_ANEG_CTRL_1) ||
            (regId == PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31) ||
            (regId == PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG))
        {
            if(apCpssLogEnable == GT_TRUE)
            {
                CPSS_LOG_INFORMATION_MAC("\n");
            }
            else
            {
                hwsOsPrintf("\n");
            }
        }
#endif
        /*print On Demand Registers*/
        hwsSprintf(buffer,"%08d: AP REG  Port %d, %s 0x%04x\n", timestamp, portId, regType[regId], regVal);
        mvApPortCtrlDebugInfoPrint(buffer, output);
        break;

     case M1_SUPERVISOR:
        portId  = SPV_LOG_PORT_GET(logInfo);
        msgId   = SPV_LOG_MSG_GET(logInfo);
        stateId = SPV_LOG_STATE_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (msgId >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE) ||
             (stateId >= SPV_MAX_STATE))
        {
            break;
        }

        /* Process message */
        hwsSprintf(buffer,"%08d: Super   Port %02d, %s, %s Msg Sent to execute, O%d(%s)\n",
                          timestamp,
                          portId,
                          portType[SPV_LOG_TYPE_GET(logInfo)],
                          msgType[msgId],
                          stateId + 1,
                          superState[stateId]);
        mvApPortCtrlDebugInfoPrint(buffer, output);
        break;
    case M2_PORT_MNG:
        dbgFlag = AN_PORT_DET_LOG_DBG_FLAG_GET(logInfo);

        if (dbgFlag == 0)
        {
#ifdef REG_PORT_TASK
            portId  = PORT_MNG_LOG_PORT_GET(logInfo);
            msgId   = PORT_MNG_MSG_GET(logInfo);
            stateId = PORT_MNG_LOG_STATE_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (msgId >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE) ||
                 (stateId >= PORT_MNG_MAX_STATE))
            {
                break;
            }

            /* Process message */
            hwsSprintf(buffer,"%08d: PortMng Port %02d, %s Msg start execute, O%d(%s)\n",
                              timestamp,
                              portId,
                              msgType[msgId],
                              stateId,
                              portMngState[stateId])
            mvApPortCtrlDebugInfoPrint(buffer, output);
#endif
        }
        else
        {   /* print log as debug */
            stateId  = AN_PORT_DET_LOG_STATE_GET(logInfo);
            switch ( stateId ) {
            case LOG_DEBUG_SERDES_OCORE      :
            case LOG_DEBUG_SERDES_OCORE2     :
            case LOG_DEBUG_SERDES_DFE        :
            case LOG_DEBUG_SERDES_EO         :
            case LOG_DEBUG_SERDES_TAP2       :
            case LOG_DEBUG_SERDES_OPERATION  :
                hwsSprintf(buffer,"%08d: AN SM   Port %02d, %s 0x%x  \n",
                                  timestamp,AN_PORT_DET_LOG_DBG_PORT_GET(logInfo),
                                  anPortSedesDebug[stateId], AN_PORT_DET_LOG_DBG16_GET(logInfo));
                mvApPortCtrlDebugInfoPrint(buffer, output);
                break;
            case LOG_DEBUG_SERDES_RESOLUTION :
                hwsSprintf(buffer,"%08d: AN SM   Port %02d, %s  fec %d port mode %d  \n",
                                   timestamp,AN_PORT_DET_LOG_DBG_PORT_GET(logInfo),
                                   anPortSedesDebug[stateId], ((AN_PORT_DET_LOG_DBG16_GET(logInfo)>>8)&0xff),(AN_PORT_DET_LOG_DBG16_GET(logInfo)& 0xff));
                mvApPortCtrlDebugInfoPrint(buffer, output);
                break;

            case LOG_DEBUG_SERDES_PCAL       :
                hwsSprintf(buffer,"%08d: AN SM   Port %02d, %s  %d\n",
                                  timestamp,AN_PORT_DET_LOG_DBG_PORT_GET(logInfo),
                                  anPortSedesDebug[stateId], AN_PORT_DET_LOG_DBG16_GET(logInfo));
                mvApPortCtrlDebugInfoPrint(buffer, output);
                break;

            }
        }

        break;

    case M3_PORT_SM:
        portId   = PORT_SM_LOG_PORT_GET(logInfo);
        statusId = PORT_SM_LOG_STATUS_GET(logInfo);
        stateId  = PORT_SM_LOG_STATE_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (statusId >= PORT_SM_MAX_STATUS) ||
             (stateId >= PORT_SM_MAX_STATE))
        {
            break;
        }

         /* Process message */
        hwsSprintf(buffer,"%08d: Port SM Port %02d, %s, O%d(%s)\n",
                          timestamp,
                          portId,
                          portMngSmStatus[statusId],
                          stateId,
                          portMngSmState[stateId]);
        mvApPortCtrlDebugInfoPrint(buffer, output);
        break;

    case M4_AP_PORT_MNG:
        portId   = AP_PORT_MNG_LOG_PORT_GET(logInfo);
        statusId = AP_PORT_MNG_LOG_STATUS_GET(logInfo);
        stateId  = AP_PORT_MNG_LOG_STATE_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (statusId >= PORT_SM_MAX_STATUS) ||
             (stateId >= PORT_SM_MAX_STATE))
        {
            break;
        }

        hwsSprintf(buffer,"%08d: Port SM Port %02d, %s, O%d(%s)\n",
                          timestamp,
                          portId,
                          portMngSmStatus[statusId],
                          stateId,
                          portMngSmState[stateId]);
        mvApPortCtrlDebugInfoPrint(buffer, output);
        break;

    case M5_AP_PORT_DET:
        portId   = AP_PORT_DET_LOG_PORT_GET(logInfo);
        statusId = AP_PORT_DET_LOG_STATUS_GET(logInfo);
        stateId  = AP_PORT_DET_LOG_STATE_GET(logInfo);
        arbId    = AP_PORT_DET_LOG_HW_SM_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (statusId >= AP_PORT_MAX_STATUS) ||
             (stateId >= AP_PORT_SM_MAX_STATE))
        {
            break;
        }

        hwsSprintf(buffer,"%08d: AP SM   Port %02d, %s, O%d(%s) ",
                          timestamp,
                          portId,
                          apPortMngSmStatus[statusId],
                          stateId,
                          apPortMngSmState[stateId]);
        mvApPortCtrlDebugInfoPrint(buffer, output);

        arbSmStateDesc(arbId, statusId, output);
        break;

    case M5_AP_PORT_DET_EXT:
        portId = AP_PORT_DET_EXT_LOG_PORT_GET(logInfo);
        hcdId  = AP_PORT_DET_EXT_LOG_HCD_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (hcdId >= 4))
        {
            break;
        }

        if(AP_PORT_DET_EXT_LOG_HCD_GET(logInfo) >= Port_AP_LAST)
        {
            hwsSprintf(buffer,"hcdType not supported %d\n", AP_PORT_DET_EXT_LOG_HCD_GET(logInfo));
            mvApPortCtrlDebugInfoPrint(buffer, output);
            return;
        }

        /* Process message */
        hwsSprintf(buffer,"%08d: AP SM   Port %02d, HCD[%s %s Link %s, FEC Result %s] FC[Tx %s, RX %s] Int[HCD %s, Link %s] \n",
                          timestamp,
                          portId,
                          apPortMngHcdFound[AP_PORT_DET_EXT_LOG_FOUND_GET(logInfo)],
                          apPortMngHcdType[AP_PORT_DET_EXT_LOG_HCD_GET(logInfo)],
                          apPortMngGen[AP_PORT_DET_EXT_LOG_LINK_GET(logInfo)],
                          apPortMngGen[AP_PORT_DET_EXT_LOG_FEC_RES_GET(logInfo)],
                          apPortMngGen[AP_PORT_DET_EXT_LOG_TX_FC_GET(logInfo)],
                          apPortMngGen[AP_PORT_DET_EXT_LOG_RX_FC_GET(logInfo)],
                          apPortMngGen[AP_PORT_DET_EXT_LOG_HCD_INT_GET(logInfo)],
                          apPortMngGen[AP_PORT_DET_EXT_LOG_LINK_INT_GET(logInfo)]);
        mvApPortCtrlDebugInfoPrint(buffer, output);

        break;

#ifdef AP_GENERAL_TASK
    case M6_GENERAL:
        portId = PORT_GENERAL_LOG_PORT_GET(logInfo);
        msgId  = PORT_GENERAL_LOG_MSG_GET(logInfo);

        /* Validate inputs params */
        if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
             (msgId >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE))
        {
            break;
        }

        hwsSprintf(buffer,"%08d: General Port %02d, Executing %s Msg",
                          timestamp,
                          portId,
                          msgType[msgId]);
        mvApPortCtrlDebugInfoPrint(buffer, output);

        switch (msgId)
        {
        case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
            if (PORT_GENERAL_LOG_RES_GET(logInfo))
            {
                hwsSprintf(buffer," %s", (PORT_GENERAL_LOG_DETAIL_GET(logInfo) == MV_GEN_LINK_DOWN) ? "Fail" :"OK");
                mvApPortCtrlDebugInfoPrint(buffer, output);
            }
            break;
        case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
            if (PORT_GENERAL_LOG_RES_GET(logInfo))
            {
                hwsSprintf(buffer," %s", (PORT_GENERAL_LOG_DETAIL_GET(logInfo) == MV_GEN_TUNE_FAIL) ? "Fail" :"Pass");
                mvApPortCtrlDebugInfoPrint(buffer, output);
            }
            break;
        }
        hwsSprintf(buffer,"\n")
        mvApPortCtrlDebugInfoPrint(buffer, output);
#endif

    case M7_AN_PORT_SM:
        stateId  = AN_PORT_DET_LOG_STATE_GET(logInfo);
        dbgFlag = AN_PORT_DET_LOG_DBG_FLAG_GET(logInfo);

        if (dbgFlag == 0)
        {
            portId   = AN_PORT_DET_LOG_PORT_GET(logInfo);
            statusId = AN_PORT_DET_LOG_STATUS_GET(logInfo);
            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (statusId >= AN_PORT_MAX_STATUS) ||
                 (stateId >= AN_PORT_SM_MAX_STATE))
            {
                break;
            }
            hwsSprintf(buffer,"%08d: AN SM   Port %02d, %s, O%d(%s)\n",
                              timestamp,
                              portId,
                              anPortSmStatus[statusId],
                              stateId,
                              anPortSmState[stateId]);
        }
        else
        {   /* print log as debug */
            hwsSprintf(buffer,"%08d: AN SM   Port %02d, DEBUG %d) %d %d\n",
                              timestamp,AN_PORT_DET_LOG_DBG_PORT_GET(logInfo),
                              stateId,AN_PORT_DET_LOG_DBG1_GET(logInfo),
                              AN_PORT_DET_LOG_DBG2_GET(logInfo));
        }
        /*arbSmStateDesc(arbId, statusId, apCpssLogEnable);*/
        mvApPortCtrlDebugInfoPrint(buffer, output);

        break;

    }
}


