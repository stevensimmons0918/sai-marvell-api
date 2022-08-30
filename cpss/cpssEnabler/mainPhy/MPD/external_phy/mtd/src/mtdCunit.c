/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for
higher-level functions for controlling and getting status from 
the Control Unit (C Unit) of the Marvell 88X32X0, 88X33X0, 88X35X0, 88E20X0 
and 88E21X0 ethernet PHYs.
********************************************************************/
#include <mtdApiRegs.h>
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <mtdCunit.h>
#include <mtdHXunit.h>
#include <mtdHunit.h>

/*******************************************************************/
/*******************************************************************
  Resets
  Port SW Reset
  Chip HW Reset
 *******************************************************************/
/****************************************************************************/

MTD_STATUS mtdCunitSwReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_U16 currMacType, currMacLinkDownSpeed, currMacMaxIfSpeed;
    MTD_BOOL currMacIfPowerDown;
    MTD_U16 regValue;
    
    if (MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        /* check if MACTYPE is DXGMII or QXGMII and use instead other port soft reset */

        /* WARNING: */
        /* Assumes if MACTYPE has been changed, the software reset has been issued already */
        /* at the same time the change was made, otherwise the readback of the MACTYPE won't be correct */
        /* If mactype was changed, but port reset was not issued, the readback will be incorrect */
        /* and this code will change the mactype to the readback value (which will have been the previous mactype) */
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_E21X0_PORT_CTRL,&regValue));
        MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue,0,3,&currMacType));

        if (currMacType == MTD_MAC_TYPE_DXGMII || currMacType == MTD_MAC_TYPE_QXGMII)
        {
            /* should only come here on E2140 or E2180 port devices */
            if (!(devPtr->numPortsPerDevice == 4 || devPtr->numPortsPerDevice == 8))
            {
                /* something is wrong */
                MTD_DBG_ERROR("mtdCunitSwReset: Invalid mactype for this device %d\n", (unsigned int)currMacType);
                return MTD_FAIL;
            }
            
            /* use bit 4 port reset instead for these two cases */
            regValue |= (1<<4);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_E21X0_PORT_CTRL,regValue));
        }
        else
        {
            /* use bit 15 port reset for all other cases */
            regValue |= (1<<15);
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_E21X0_PORT_CTRL,regValue));
        }
    }
    else
    {
         if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
        {
            /* WARNING: */
            /* Assumes if MacType has been changed, the software reset has been issued already */
            /* at the same time the change was made, otherwise the readback of the MACTYPE won't be correct */
            /* If mactype was changed, but port reset was not issued, the readback will be incorrect */
            /* and this code will change the mactype to the readback value (which will have been the previous mactype) */
            MTD_ATTEMPT(mtdGetMacInterfaceCopperOnlyPhy(devPtr,port,&currMacType,&currMacIfPowerDown,&currMacLinkDownSpeed,&currMacMaxIfSpeed));

            /* Must pass in the first(Master) port for DXGMII/QXGMII/OXGMII modes */
            if (currMacType == MTD_MAC_TYPE_10G_DXGMII || currMacType == MTD_MAC_TYPE_20G_DXGMII ||
                currMacType == MTD_MAC_TYPE_10G_QXGMII || currMacType == MTD_MAC_TYPE_20G_QXGMII ||
                currMacType == MTD_MAC_TYPE_OXGMII || currMacType == MTD_MAC_TYPE_OXGMII_PARTNER)
            {
                MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,6,1,&regValue));
                if (regValue != 0x1)
                {
                    MTD_DBG_ERROR("mtdCunitSwReset: Must pass in the Master(first) port of the port group\n");
                    MTD_DBG_ERROR("mtdCunitSwReset: P0/P2 (P4/P6) for DXGMII, P0 (P4) for QXGMII/OXGMII/OXGMII_PARTNER)\n");
                    return MTD_FAIL;
                }
            }
        }

        /* On X3540/E2540/X3580, this will reset all 4 grouped ports for the DXGMII/QXGMII/OXGMII modes */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL,15,1,1));
    }

    return MTD_OK;
    
}

MTD_STATUS mtdTunitHwReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_CUNIT_PORT_CTRL, 12, 1, 1));

    return MTD_OK;
}


MTD_STATUS mtdChipHardwareReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_U16 lastPortMdioAddr, basePort, upperHalfFirstPort;

    if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        /* 88X33X0 for chip reset must use last port for full reset */
        /* redirect to last port*/
        if (devPtr->numPortsPerDevice == 0) /* check for divide by 0 */
        {
            return MTD_FAIL;
        }
        basePort = (port/(devPtr->numPortsPerDevice)) * (devPtr->numPortsPerDevice);
        lastPortMdioAddr = (basePort + (devPtr->numPortsPerDevice)) - 1;
        return (mtdHwSetPhyRegField(devPtr,lastPortMdioAddr,MTD_CUNIT_PORT_CTRL,14,1,1));
    }
    else
    {
        if (MTD_IS_X3580_DEVICE(devPtr))
        {
            /* Need to reset both sides of device */
            if (devPtr->numPortsPerDevice == 0) /* check for divide by 0 */
            {
                return MTD_FAIL;
            }        
            basePort = (port/(devPtr->numPortsPerDevice)) * (devPtr->numPortsPerDevice);
            upperHalfFirstPort = basePort + 4;
          
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,upperHalfFirstPort,MTD_CUNIT_PORT_CTRL,(1<<14)));             
            MTD_ATTEMPT(mtdWait(devPtr, 225)); /* need to wait 225ms to reset other 4 ports */
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,basePort,MTD_CUNIT_PORT_CTRL,(1<<14)));
            MTD_ATTEMPT(mtdWait(devPtr, 10)); /* need to wait 10ms before any MDIO activity after reset */            
            return MTD_OK;
        }
        /* else/all others */
       
        return (mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL,14,1,1));
    }

    return MTD_FAIL; /* should never come to here */
}

/*******************************************************************
  Macsec Configuration
  Macsec Set Control
  Macsec Get Control
 *******************************************************************/


MTD_STATUS mtdSetMacsecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 powerControl,
    IN MTD_U16 faultForwardControl
)
{
    if (powerControl == MTD_MACSEC_POWERON ||
        powerControl == MTD_MACSEC_RESET ||
        powerControl == MTD_MACSEC_SLEEP)        
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,12,2,powerControl));
    }
    else
    {
        return MTD_FAIL;
    }

    switch (faultForwardControl)
    {
        case MTD_FAULT_FORWARDING_OFF:
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,14,1,0));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,11,1,0));
            break;
            
        case MTD_FORWARD_LINK_FAULT:
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,14,1,1));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,11,1,0));            
            break;
            
        case MTD_FORWARD_NON_LINK_FAULT:
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,14,1,0));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,11,1,1));            
            break;
            
        case MTD_FORWARD_ALL_FAULT:
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,14,1,1));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,11,1,1));            
            break;
            
        default:
            return MTD_FAIL; /* invalid value passed in */
            break;
    }

    return MTD_OK;
    
}

MTD_STATUS mtdGetMacsecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *powerStatus,
    OUT MTD_U16 *faultForwardStatus
)
{
    MTD_U16 bit14, bit11;
    MTD_U16 regValue;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_MODE_CONFIG,&regValue));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue,12,2,powerStatus));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue,14,1,&bit14));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue,11,1,&bit11));

    if (bit11 && bit14)
    {
        *faultForwardStatus = MTD_FORWARD_ALL_FAULT;
    }
    else if (bit11 && !bit14)
    {
        *faultForwardStatus = MTD_FORWARD_NON_LINK_FAULT;
    }
    else if (!bit11 && bit14)
    {
        *faultForwardStatus = MTD_FORWARD_LINK_FAULT;
    }
    else
    {
        /* must be !bit11 && !bit14 */
        *faultForwardStatus = MTD_FAULT_FORWARDING_OFF;
    }

    return MTD_OK;
}

/*******************************************************************
  C Unit Port Power Down
 *******************************************************************/

MTD_STATUS mtdSetPortPowerDown
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL powerDown
)
{
    return(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL,11,1,(powerDown == MTD_TRUE ? 1 : 0)));
}

MTD_STATUS mtdGetPortPowerDown
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *powerDown
)
{
    MTD_U16 bit11;

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL,11,1,&bit11));

    if (bit11)
    {
        *powerDown = MTD_TRUE;
    }
    else
    {
        *powerDown = MTD_FALSE;
    }

    return MTD_OK;
}

/*******************************************************************
  C Unit Top Configuration
 *******************************************************************/

MTD_STATUS mtdSetCunitTopConfig
(
 IN MTD_DEV_PTR devPtr,
 IN MTD_U16 port,
 IN MTD_U16 frameToRegister,
 IN MTD_U16 mediaSelect,
 IN MTD_U16 fiberType,
 IN MTD_BOOL npMediaEnergyDetect,
 IN MTD_BOOL maxPowerTunitAMDetect,
 IN MTD_BOOL softwareReset,
 IN MTD_BOOL rerunSerdesInitialization
)
{
    MTD_U16 regValue1,tunitinswreset,tunitinwaitpowerdown;

    if (MTD_IS_E21X0_BASE(devPtr->deviceId) || MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        MTD_DBG_ERROR("This function does not support MTD_E21X0_BASE and MTD_X35X0_BASE devices\n");
        return MTD_FAIL;
    }
    
    /* check error check of ranges/values of the different parameters */
    if (frameToRegister > MTD_F2R_8BHDR)
    {
        MTD_DBG_ERROR("Invalid F2R selection = %d\n", (unsigned int)frameToRegister);
        return MTD_FAIL;
    }

    if (mediaSelect == 5 || mediaSelect == 6 || mediaSelect > MTD_MS_AUTO_FIRST_LINK)
    {
        MTD_DBG_ERROR("Invalid media select value = %d\n", (unsigned int)mediaSelect);
        return MTD_FAIL;
    }

    if (MTD_IS_E20X0_DEVICE(devPtr->deviceId))
    {
        /* allow MTD_FT_1000BASEX for backward compatability, MTD_FT_NONE was added later */
        if (fiberType != MTD_FT_NONE && fiberType != MTD_FT_1000BASEX)
        {
            MTD_DBG_ERROR("Invalid fiber type = %d selected for E20X0 device\n", (unsigned int)fiberType);
            return MTD_FAIL; /* no fiber interface on these */
        }
        else
        {
            fiberType = 0; /* for MTD_FT_NONE OR MTD_FT_1000BASEX, should write 0 */
        }
    } else if (MTD_IS_X32X0_BASE(devPtr->deviceId))
    {
        if (fiberType > MTD_FT_10GBASER)
        {
            MTD_DBG_ERROR("Invalid fiber type on X32X0 value = %d\n", (unsigned int)fiberType);
            return MTD_FAIL; /* guarantee only lower 2 bits are set on 32X0 devices */
        }
    }
    else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        if ((fiberType > MTD_FT_2500BASEX) && (fiberType != MTD_FT_5GBASER))
        {
            MTD_DBG_ERROR("Invalid fiber type on X33X0 value = %d\n", (unsigned int)fiberType);
            return MTD_FAIL;
        }        
    }
    
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_MODE_CONFIG,&regValue1));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue1,frameToRegister,4,2,&regValue1));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue1,mediaSelect,0,3,&regValue1));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_MODE_CONFIG,regValue1));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_PORT_CTRL,&regValue1));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue1,(npMediaEnergyDetect?1:0),10,1,&regValue1));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue1,(maxPowerTunitAMDetect?1:0),9,1,&regValue1));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue1,fiberType,3,3,&regValue1)); /* on 32X0 devices only low 2 bits are valid, on 33X0 devices 3 bits are valid */
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_PORT_CTRL,regValue1));

    /* do a software reset so the new values take effect, several of the above require it */
    if (softwareReset)
    {
        MTD_ATTEMPT(mtdCunitSwReset(devPtr,port)); 

        if (mediaSelect == MTD_MS_FBR_ONLY)
        {
            /* check if T unit is already in fiber only mode/T unit software reset is already in process */
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,15,1,&tunitinswreset));
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_TUNIT_BIST_STATUS_REG,7,1,&tunitinwaitpowerdown));

            if ((tunitinswreset == 1 && tunitinwaitpowerdown == 0) ||
                (tunitinswreset == 0 && tunitinwaitpowerdown == 1))
            {
                MTD_DBG_ERROR("T unit in unexpected state\n");
                return MTD_FAIL; /* either a T unit software reset is already in progress, or T unit is in a strange state */
            }

            if ((tunitinswreset == 0 && tunitinwaitpowerdown == 0))
            {
                /* tunit is in not in fiber only mode yet and no sw reset is in progress */
            
                MTD_ATTEMPT(mtdWait(devPtr,10));
                MTD_ATTEMPT(mtdSoftwareReset(devPtr,port,0)); /* T unit held in low power mode in fiber only, so it will not be ready until taken out of fiber only mode */
                MTD_ATTEMPT(mtdWait(devPtr,10)); /* let T unit go into low power mode, BIST status should be 1.C00C.7 = 1, MTD_BIST_WAIT_POWERDOWN before continuing */

                MTD_ATTEMPT(mtdHwWaitForRegFieldValue(devPtr,port,MTD_TUNIT_BIST_STATUS_REG,7,1,1,500)); /* wait for MTD_BIST_WAIT_POWERDOWN */
            }
            else
            {
                /* T unit is already in fiber only mode (actually it's stuck in sw reset and in waiting for power down to be removed, i.e. both are 1) */
            }
        }

        /* other modes don't need a T unit soft reset, but need to rerun the serdes init */
        /* doesn't hurt to do it in all other cases and this function works for any mode (might result in it running twice) */
        if (rerunSerdesInitialization == MTD_TRUE)
        {
#if MTD_ORIGSERDES        
            MTD_ATTEMPT(mtdRerunSerdesAutoInitUseAutoMode(devPtr,port));
#else
            MTD_DBG_ERROR("Need to include MTD_ORIGSERDES to include serdes code\n");
            return MTD_FAIL;
#endif /* MTD_ORIGSERDES */
        }        
    }

    return MTD_OK;
}

MTD_STATUS mtdGetCunitTopConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *frameToRegister,
    OUT MTD_U16 *mediaSelect,
    OUT MTD_U16 *fiberType,
    OUT MTD_BOOL *npMediaEnergyDetect,
    OUT MTD_BOOL *maxPowerTunitAMDetect
)
{
    MTD_U16 regValue1,bitValue;

    if (MTD_IS_E21X0_BASE(devPtr->deviceId) || MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        MTD_DBG_ERROR("This function does not support MTD_E21X0_BASE and MTD_X35X0_BASE devices\n");
        return MTD_FAIL;
    }    
    
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_MODE_CONFIG,&regValue1));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue1,4,2,frameToRegister));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue1,0,3,mediaSelect));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_PORT_CTRL,&regValue1));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue1,10,1,&bitValue));
    if (bitValue)
    {
        *npMediaEnergyDetect = MTD_TRUE;
    }
    else
    {
        *npMediaEnergyDetect = MTD_FALSE;
    }
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue1,9,1,&bitValue));
    if (bitValue)
    {
        *maxPowerTunitAMDetect= MTD_TRUE;
    }
    else
    {
        *maxPowerTunitAMDetect = MTD_FALSE;
    }
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(regValue1,3,3,fiberType));/* on 32X0 devices only low 2 bits are valid, on 33X0 devices 3 bits are valid */
    if (MTD_IS_E20X0_DEVICE(devPtr->deviceId))
    {
        *fiberType = MTD_FT_NONE;
        *npMediaEnergyDetect = MTD_FALSE;
        *maxPowerTunitAMDetect = MTD_FALSE;
    }

    return MTD_OK;
}




