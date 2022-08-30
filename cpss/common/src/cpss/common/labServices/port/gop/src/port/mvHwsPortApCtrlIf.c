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
* @file mvHwsPortApCtrlIf.c
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
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

#include "../portCtrl/h/mvHwsPortCtrlAp.h"
#include "../portCtrl/h/mvHwsPortCtrlAn.h"
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>

/**
* @internal mvHwsApPortCtrlStart function
* @endinternal
*
* @brief   Init AP port capability.
*         Runs AP protocol(802.3ap Serdes AutoNeg) and configures the best port
*         mode and all it's elements accordingly.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStart
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApStartIpc(devNum, portGroup, phyPortNum, (GT_U32*)apCfg));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlStop function
* @endinternal
*
* @brief   Disable the AP engine on port and release all its resources.
*         Clears the port mode and release all its resources according to selected.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApStopIpc(devNum, portGroup, phyPortNum));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlSysAck function
* @endinternal
*
* @brief   Acknowledge port resources were allocated at application level
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlSysAck
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApSysAckIpc(devNum, portGroup, phyPortNum, portMode));

    return GT_OK;
}

/**
* @internal mvHwsApPortEnableWaitAck function
* @endinternal
*
* @brief   Send message to notify AP state machine that port was
*         disabled and ap machine can continue execution
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortEnableWaitAck
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApEnableWaitAckIpc(devNum, portGroup, phyPortNum));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlCfgGet function
* @endinternal
*
* @brief   Returns the AP port configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @param[out] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlCfgGet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

#if defined (CHX_FAMILY)
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsPortAnpConfigGet(devNum, phyPortNum, apCfg));
    }
    else
#endif
    {
        CHECK_STATUS(mvHwsPortApCfgGetIpc(devNum, portGroup, phyPortNum, (GT_U32 *)apCfg));
    }

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatus                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatusGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_STATUS *apStatus
)
{
    GT_STATUS rcode;

    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    apStatus->preApPortNum  = phyPortNum;
    apStatus->postApPortNum = phyPortNum;

#if defined (CHX_FAMILY)
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsAnpPortStatusGet( devNum,  phyPortNum, apStatus));
        return GT_OK;
    }
#endif
    rcode = mvHwsPortApStatusGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apStatus);
    if (rcode == GT_OK)
    {
        if ( !(apStatus->hcdResult.hcdMisc &0x1)) /*check if OPTICAL_MODE*/
        {
            switch (apStatus->hcdResult.hcdResult)
            {
            case Port_100GBASE_KR4:
                apStatus->postApPortMode = _100GBase_KR4;
                break;
            case Port_100GBASE_CR4:
                apStatus->postApPortMode = _100GBase_CR4;
                break;
            case Port_40GBase_R:
                apStatus->postApPortMode = _40GBase_KR4;
                break;
            case Port_40GBASE_CR4:
                apStatus->postApPortMode = _40GBase_CR4;
                break;
            case Port_10GBase_R:
                apStatus->postApPortMode = _10GBase_KR;
                break;
            case Port_10GBase_KX4:
                apStatus->postApPortMode = _10GBase_KX4;
                break;
            case Port_1000Base_KX:
                apStatus->postApPortMode = _1000Base_X;
                break;
            case Port_20GBASE_KR:
                apStatus->postApPortMode = _20GBase_KR;
                break;
            case Port_25GBASE_KR:
                apStatus->postApPortMode = _25GBase_KR;
                break;
            case Port_25GBASE_KR_S:
                apStatus->postApPortMode = _25GBase_KR_S;
                break;
            case Port_25GBASE_CR:
                apStatus->postApPortMode = _25GBase_CR;
                break;
            case Port_25GBASE_CR_S:
                apStatus->postApPortMode = _25GBase_CR_S;
                break;
            case Port_25GBASE_KR_C:
                apStatus->postApPortMode = _25GBase_KR_C;
                break;
            case Port_50GBASE_KR2_C:
                apStatus->postApPortMode = _50GBase_KR2_C;
                break;
            case Port_25GBASE_CR_C:
                apStatus->postApPortMode = _25GBase_CR_C;
                break;
            case Port_50GBASE_CR2_C:
                apStatus->postApPortMode = _50GBase_CR2_C;
                break;
                /*Raven*/
            case Port_40GBase_KR2:
                apStatus->postApPortMode = _40GBase_KR2;
                break;

            /* PM4 Raven speeds*/
            case Port_50GBase_KR:
                apStatus->postApPortMode = _50GBase_KR;
                break;
            case Port_50GBase_CR:
                apStatus->postApPortMode = _50GBase_CR;
                break;
            case Port_100GBase_KR2:
                apStatus->postApPortMode = _100GBase_KR2;
                break;
            case Port_100GBase_CR2:
                apStatus->postApPortMode = _100GBase_CR2;
                break;
            case Port_200GBase_KR4:
                apStatus->postApPortMode = _200GBase_KR4;
                break;
            case Port_200GBase_CR4:
                apStatus->postApPortMode = _200GBase_CR4;
                break;
            case Port_200GBase_KR8:
                apStatus->postApPortMode = _200GBase_KR8;
                break;
            case Port_200GBase_CR8:
                apStatus->postApPortMode = _200GBase_CR8;
                break;
            case Port_400GBase_KR8:
                apStatus->postApPortMode = _400GBase_KR8;
                break;
            case Port_400GBase_CR8:
                apStatus->postApPortMode = _400GBase_CR8;
                break;

            }
        }
        else
        { /* is OPTICAL_MODE == true*/
            switch (apStatus->hcdResult.hcdResult)
            {
            case Port_400GBase_KR8:
            case Port_400GBase_CR8:
                apStatus->postApPortMode = _400GBase_SR_LR8;
                break;
            case Port_200GBase_KR8:
            case Port_200GBase_CR8:
                apStatus->postApPortMode = _200GBase_SR_LR8;
                break;
            case Port_200GBase_KR4:
            case Port_200GBase_CR4:
                apStatus->postApPortMode = _200GBase_SR_LR4;
                break;
            case Port_100GBase_KR2:
            case Port_100GBase_CR2:
                apStatus->postApPortMode = _100GBase_SR_LR2;
                break;
            case Port_50GBase_KR:
            case Port_50GBase_CR:
                apStatus->postApPortMode = _50GBase_SR_LR;
                break;
            case Port_100GBASE_KR4:
            case Port_100GBASE_CR4:
                apStatus->postApPortMode = _100GBase_SR4;
                break;
            case Port_40GBase_R:
            case Port_40GBASE_CR4:
                apStatus->postApPortMode = _40GBase_SR_LR4;
                break;
            case Port_25GBASE_KR:
            case Port_25GBASE_KR_S:
            case Port_25GBASE_CR:
            case Port_25GBASE_CR_S:
            case Port_25GBASE_KR_C:
                apStatus->postApPortMode = _25GBase_SR;
                break;
            case Port_50GBASE_CR2_C:
                apStatus->postApPortMode = _50GBase_SR2;
                break;
            case Port_10GBase_R:
                apStatus->postApPortMode = _10GBase_SR_LR;
                break;
            }
        }
    }
    return rcode;
}

/**
* @internal mvHwsApPortCtrlStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsGet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_AP_PORT_STATS *apStats
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

#if defined (CHX_FAMILY)
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsAnpPortStatsGet( devNum,  phyPortNum, apStats));
    }
    else
#endif
    {
        CHECK_STATUS(mvHwsPortApStatsGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apStats));
    }

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsReset
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

#if defined (CHX_FAMILY)
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsAnpPortStatsReset( devNum,  phyPortNum));
    }
    else
#endif
    {
        CHECK_STATUS(mvHwsPortApStatsResetIpc(devNum, portGroup, phyPortNum));
    }
    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlIntropSet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

#if defined (CHX_FAMILY)
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
         CHECK_STATUS(mvHwsAnpPortInteropSet(devNum, phyPortNum, apIntrop));
    }
    else
#endif
    {
        CHECK_STATUS(mvHwsPortApIntropSetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apIntrop));
    }
    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlIntropGet function
* @endinternal
*
* @brief   Return AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlIntropGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

#if defined (CHX_FAMILY)
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsAnpPortInteropGet(devNum, phyPortNum, apIntrop));
    }
    else
#endif
    {
        CHECK_STATUS(mvHwsPortApIntropGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apIntrop));
    }

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlDebugGet function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apDebug                  - AP debug parameters
*
* @param[out] apDebug                  - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlDebugGet
(
    GT_U8           devNum,
    GT_U32          portGroup,
    GT_U32          phyPortNum,
    MV_HWS_FW_LOG   *apDebug
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApDebugGetIpc(devNum, portGroup, phyPortNum, (GT_U32*)apDebug));

    return GT_OK;
}

/**
* @internal mvHwsFwHwsLogGet function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
* @param[in] fwHwsLogPtr              - FW HWS log parameters pointer
*
* @param[out] fwHwsLogPtr              - FW HWS log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFwHwsLogGet
(
    GT_U8           devNum,
    GT_U32          phyPortNum,
    MV_HWS_FW_LOG  *fwHwsLogPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsLogGetIpc(devNum, phyPortNum, (GT_U32*)fwHwsLogPtr));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlAvagoGuiSet function
* @endinternal
*
* @brief   Set AP state machine state when Avago GUI is enabled
*         Avago GUI access Avago Firmware as SBUS command level
*         Therefore it is required to stop the periodic behiviour of AP state
*         machine when Avago GUI is enabled
* @param[in] devNum                   - system device number
* @param[in] cpuId                    - the cpu id
* @param[in] state                    - Avago GUI state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlAvagoGuiSet
(
    GT_U8 devNum,
    GT_U8 cpuId,
    GT_U8 state
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApAvagoGuiSetIpc(devNum, cpuId, state));

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsApPortCtrlEnablePortCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlSet
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApEnablePortCtrlSetIpc(devNum, srvCpuEnable));

    return GT_OK;
}

/**
* @internal mvHwsApPortCtrlEnablePortCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlGet
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApEnablePortCtrlGetIpc(devNum, srvCpuEnablePtr));

    return GT_OK;
}
#endif

/**
* @internal mvHwsApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - serdes TX values offsets
* @param[in] serdesSpeed              - port speed assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxParametersOffsetSet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApSerdesTxParametersOffsetSetIpc(devNum, portGroup, phyPortNum, serdesNumber, offsets, serdesSpeed));

    return GT_OK;
}


/**
* @internal mvHwsApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesRxParametersManualSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE  *rxCfgPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (apPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }
    if ((rxCfgPtr->serdesSpeed != _10_3125G) && (rxCfgPtr->serdesSpeed != _25_78125G)) {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &apPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }
    CHECK_STATUS(mvHwsPortApSerdesRxParametersManualSetIpc(devNum, portGroup, apPortNum, rxCfgPtr));

    return GT_OK;
}


/**
* @internal mvHwsPortAdaptiveCtlePortEnableSet function
* @endinternal
*
* @brief   Set adaptive ctle port enable/ disable
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port number
* @param[in] enable                   - enale or disable
*       adaptive ctle
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAdaptiveCtlePortEnableSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          portNum,
    GT_BOOL                         enable
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (portNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &portNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortAdaptiveCtlePortEnableSetIpc(devNum, portGroup, portNum, enable));

    return GT_OK;
}

#if defined (PX_FAMILY)
/**
* @internal mvHwsPortLkbPortSet function
* @endinternal
*
* @brief   set link-binding on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - HWS port mode
* @param[in] pairNum                  - physical pair port number.
*                                       when the link on portNum is down -
*                                       the remote fault enabled on the pair port.
* @param[in] add                      - 1-enable link binding on port, 0-disable
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_BAD_PARAM                - on if error in devNum or portNum
*                                       or if portNum==pairPortNum
*/
GT_STATUS mvHwsPortLkbPortSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U16                          portNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN GT_U16                          pairNum,
    IN GT_BOOL                         add
)
{
    /* LKB_PRINT("mvHwsPortLkbPortSet\n"); */

    if (devNum >= HWS_MAX_DEVICE_NUM)
        return GT_BAD_PARAM;

    if (portNum >= HWS_CORE_PORTS_NUM(devNum))
        return GT_BAD_PARAM;

    CHECK_STATUS(mvHwsPortLkbPortSetIpc(devNum, portGroup, portNum, portMode, (GT_U8)pairNum, (GT_U8)add));

    return GT_OK;
}
#endif


/**
* @internal mvHwsPortIsLinkUpStatusSet function
* @endinternal
*
* @brief   Set non AP port link up status
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] isLinkUp                 - port in link up or link
*                                       down
** @param[in] trainLfArr              - train LF from training
*                                       or enh training result.
* @param[in] enhTrainDelayArr        - enhtrain Delay from
*                                      enh training result.
* @param[in] serdesList              - port's serdeses arrray
* @param[in] numOfSerdes             - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortIsLinkUpStatusSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    GT_BOOL                 isLinkUp,
    GT_U16                  *trainLfArr,
    GT_U16                  *enhTrainDelayArr,
    GT_U16                  *serdesList,
    GT_U8                   numOfSerdeses
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (portNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &portNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortIsLinkUpStatusSetIpc(devNum, portGroup, portNum, isLinkUp,trainLfArr,enhTrainDelayArr,serdesList,numOfSerdeses));

    return GT_OK;
}

#if defined (PX_FAMILY)
/**
* @internal mvHwsPortLkbRegisterApPortSet function
* @endinternal
*
* @brief   set link-binding on AP port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] pairNum                  - physical pair port number.
*                                       when the link on portNum is down -
*                                       the remote fault enabled on the pair port.
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_BAD_PARAM                - on if error in devNum or portNum
*                                       or if portNum===pairPortNum
*/
GT_STATUS mvHwsPortLkbRegisterApPortSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U16                          apPortNum,
    IN GT_U16                          pairNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
        return GT_BAD_PARAM;

    if (apPortNum >= HWS_CORE_PORTS_NUM(devNum))
        return GT_BAD_PARAM;

    if (apPortNum == pairNum)
        return GT_BAD_PARAM;

    CHECK_STATUS(mvHwsPortLkbRegisterApPortIpc(devNum, portGroup, (GT_U8)apPortNum, (GT_U8)pairNum));

    return GT_OK;
}
#endif


/**
* @internal mvHwsApSerdesRxManualConfigSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place before TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesSpeed              - serdes speed
* @param[in] serdesNumber             - serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxConfigPtr            - serdes RX values per speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesRxManualConfigSet
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT *rxConfigPtr
)
{
    GT_U8 speedIdx = UNPERMITTED_SD_SPEED_INDEX;
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }
    speedIdx = mvHwsSerdesSpeedToIndex(devNum,serdesSpeed);
    if (speedIdx == UNPERMITTED_SD_SPEED_INDEX)
    {
         return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvHwsPortApSerdesRxConfigSetIpc(devNum, portGroup, phyPortNum, serdesSpeed, serdesNumber, serdesType, rxConfigPtr));

    return GT_OK;
}

/**
* @internal mvHwsApSerdesTxManualConfigSet function
* @endinternal
*
* @brief   Set serdes TX parameters . Those offsets
*         will take place before TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesSpeed              - serdes speed
* @param[in] serdesNumber             - serdes number
* @param[in] serdesType               - serdes type
* @param[in] txConfigPtr            - serdes TX values per speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxManualConfigSet
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT *txConfigPtr
)
{
    GT_U8 speedIdx = UNPERMITTED_SD_SPEED_INDEX;
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    speedIdx = mvHwsSerdesSpeedToIndex(devNum,serdesSpeed);
    if (speedIdx == UNPERMITTED_SD_SPEED_INDEX)
    {
         return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApSerdesTxConfigSetIpc(devNum, portGroup, phyPortNum, serdesSpeed, serdesNumber, serdesType, txConfigPtr));

    return GT_OK;
}

/**
* @internal mvHwsApSerdesTxPresetSet function
* @endinternal
*
* @brief  Set preset in serdes. Will take place before TRX
*         training.
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] laneNumber             - lane number
* @param[in] portMode               - port mode
* @param[in] devNum                   - system device number
* @param[in] pre                      - preset pre value
* @param[in] atten                    - preset atten value
* @param[in] post                     - preset post value
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxPresetSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            portGroup,
    IN  GT_U32                            phyPortNum,
    IN  GT_U32                            laneNumber,
    IN  MV_HWS_PORT_STANDARD              portMode,
    IN  GT_U8                             pre,
    IN  GT_U8                             atten,
    IN  GT_U8                             post
)
{
    MV_HWS_PORT_INIT_PARAMS  curPortParams;
    GT_U8  speedIdx = UNPERMITTED_SD_SPEED_INDEX;
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum,portGroup,phyPortNum,portMode,&curPortParams));

    speedIdx = mvHwsSerdesSpeedToIndex(devNum, curPortParams.serdesSpeed);
    if (speedIdx == UNPERMITTED_SD_SPEED_INDEX)
    {
         return GT_BAD_PARAM;
    }

    if(laneNumber >= curPortParams.numOfActLanes)
    {
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvHwsPortApTxPresetSetIpc(devNum, portGroup, curPortParams.activeLanesList[laneNumber], curPortParams.serdesSpeed, pre, atten, post));

    return GT_OK;
}

/**
* @internal mvHwsApSerdesGlobalRxTerminationModeIpc function
* @endinternal
*
* @brief   Set serdes global rx termination mode.
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] globalRxTermination    - global rx termination mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesGlobalRxTerminationModeIpc
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                globalRxTermination
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortApSerdesGlobalRxTerminationModeIpc(devNum, portGroup, globalRxTermination));

    return GT_OK;
}

/**
* @internal mvHwsApSetGlobalHostModeIpc function
* @endinternal
*
* @brief   Indicate to AP that it is controled by HOST CPU and
*          not MI (relevant for Falcon Only).
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] globalHostMode      - global Control Mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSetGlobalHostModeIpc
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                globalHostMode
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsSetHostModeIpc(devNum, portGroup, globalHostMode));

    return GT_OK;
}


/**
* @internal mvHwsApSerdesInterconnectProfileConfigSet function
* @endinternal
*
* @brief   Set interconnect profile
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] numOfActLanes          - num of active lanes
* @param[in] interconnectProfile    - interconnect profile
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesInterconnectProfileConfigSet
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                phyPortNum,
    GT_U8                                 numOfActLanes,
    GT_U32                                interconnectProfile
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    CHECK_STATUS(mvHwsPortApConfigInterconnectProfileIpc(devNum, portGroup, phyPortNum, numOfActLanes, interconnectProfile));

    return GT_OK;
}

/**
* @internal mvHwsApResolutionBitSet function
* @endinternal
*
* @brief   Get AP AN resolution bit for 40G_KR2
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] anResBit               - resolution bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApResolutionBitSet
(
    GT_U8                        devNum,
    MV_HWS_PORT_STANDARD         portMode,
    GT_U32                       anResBit
)
{
    GT_U32 portNum;
    GT_U32 portGroup = 0, step = 16;
#if defined (CHX_FAMILY)
    MV_HWS_PORT_INIT_PARAMS  curPortParams;
#endif

    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        step = 1;
    }
    for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum ; portNum += step)
    {
#if defined (CHX_FAMILY)
        if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
        {
            if (GT_OK == hwsPortModeParamsGetToBuffer(devNum, 0, portNum, _10GBase_KR, &curPortParams))
            {
                CHECK_STATUS(mvHwsAnpPortResolutionBitSet( devNum, portNum, portMode, anResBit));
            }
            continue;
        }
#endif
        if(!mvHwsMtipIsReducedPort(devNum,portNum))
        {
            CHECK_STATUS(mvHwsApResolutionBitSetIpc(devNum,portGroup,portNum,portMode,anResBit));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsCm3SemOper function
* @endinternal
*
* @brief   Set AP AN resolution bit for 40G_KR2
*
* @param[in] devNum                 - system device number
* @param[in] portMacNum             - physical port number
* @param[in] takeFree               True- Take/False-Free
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCm3SemOper
(
    GT_U8                       devNum,
    GT_U32                      portMacNum,
    GT_BOOL                     takeFree /*true - take,false - free*/
)
{
    GT_U8   chipIdx;

    if ( hwsDeviceSpecInfo[devNum].devType != Falcon )
    {
        return GT_OK;
    }
    if ( mvHwsMtipIsReducedPort(devNum, portMacNum) )
    {
         chipIdx = (GT_U8)(portMacNum - (hwsDeviceSpecInfo[devNum].portsNum - hwsDeviceSpecInfo[devNum].numOfTiles*4));
    }
    else
    {
         chipIdx = portMacNum/16;
    }
    if ( takeFree ) {
        mvHwsHWAccessLock(devNum, chipIdx, MV_SEMA_PM_CM3);

    }
    else
    {
        mvHwsHWAccessUnlock(devNum, chipIdx, MV_SEMA_PM_CM3);
    }
    return GT_OK;
}
