/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for
higher-level functions for controlling and getting status from 
the Host Unit (H Unit) of the Marvell 88X32X0, 88X33X0, 88X35X0, 88E20X0 
and 88E21X0 ethernet PHYs.
********************************************************************/
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <mtdApiRegs.h>
#include <mtdCunit.h>
#include <mtdHunit.h>
#include <mtdHXunit.h>

/******************************************************************************
 Mac Interface functions
******************************************************************************/

MTD_STATUS mtdSetMacInterfaceControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 macType,
    IN MTD_BOOL macIfPowerDown,
    IN MTD_U16 macIfSnoopSel,
    IN MTD_U16 macIfActiveLaneSelect,
    IN MTD_U16 macLinkDownSpeed,
    IN MTD_U16 macMaxIfSpeed,
    IN MTD_BOOL doSwReset,
    IN MTD_BOOL rerunSerdesInitialization
)
{
    MTD_U16 cunitPortCtrl, cunitModeConfig;
    MTD_U16 mediaSelect;
    MTD_U16 tunitinswreset,tunitinwaitpowerdown;

    if (MTD_IS_E21X0_BASE(devPtr->deviceId) || MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        return mtdSetMacInterfaceCopperOnlyPhy(devPtr,port,macType,macIfPowerDown, macLinkDownSpeed,macMaxIfSpeed);
    }

    /* do range checking on parameters */
    if ((macType > MTD_MAC_LEAVE_UNCHANGED))
    {
        return MTD_FAIL;
    }     

    if (macType == MTD_MAC_TYPE_USXGMII && (!(MTD_IS_X33X0_BASE(devPtr->deviceId))))
    {
        MTD_DBG_ERROR("mtdSetMacInterfaceControl() MTD_MAC_TYPE_USXGMII only supported on X33X0 devices.\n");
        return MTD_FAIL;
    }

    if ((macIfSnoopSel > MTD_MAC_SNOOP_LEAVE_UNCHANGED) || 
        (macIfSnoopSel == 1))
    {
        return MTD_FAIL;
    }
    
    if (macIfActiveLaneSelect > 1)
    {
        return MTD_FAIL;
    }

    if (macLinkDownSpeed > MTD_MAC_SPEED_LEAVE_UNCHANGED)
    {
        return MTD_FAIL;
    }

    if (!(macMaxIfSpeed == MTD_MAX_MAC_SPEED_10G || macMaxIfSpeed == MTD_MAX_MAC_SPEED_5G || macMaxIfSpeed == MTD_MAX_MAC_SPEED_2P5G ||
          macMaxIfSpeed == MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED || macMaxIfSpeed == MTD_MAX_MAC_SPEED_NOT_APPLICABLE))
    {
        return MTD_FAIL;
    }
        
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_PORT_CTRL,&cunitPortCtrl));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_MODE_CONFIG,&cunitModeConfig))

    /* Because writes of some of these bits don't show up in the register on a read
       until after the software reset, we can't do repeated read-modify-writes
       to the same register or we will lose those changes.

       This approach also cuts down on IO and speeds up the code
    */

    if (macType < MTD_MAC_LEAVE_UNCHANGED)
    {
        MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitPortCtrl,macType,0,3,&cunitPortCtrl));
    }

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitModeConfig,(MTD_U16)macIfPowerDown,3,1,&cunitModeConfig));

    if (macIfSnoopSel < MTD_MAC_SNOOP_LEAVE_UNCHANGED)
    {
        MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitModeConfig,macIfSnoopSel,8,2,&cunitModeConfig));
    }

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitModeConfig,macIfActiveLaneSelect,10,1,&cunitModeConfig));

    if (macLinkDownSpeed < MTD_MAC_SPEED_LEAVE_UNCHANGED)
    {
        MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitModeConfig,macLinkDownSpeed,6,2,&cunitModeConfig));
    }

    /* Now write changed values*/
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_PORT_CTRL,cunitPortCtrl));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_MODE_CONFIG,cunitModeConfig));

    if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        if (macMaxIfSpeed != MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,31,0xF0A8,0,2,macMaxIfSpeed));
        }
    }

    if (doSwReset == MTD_TRUE)
    {
        MTD_ATTEMPT(mtdCunitSwReset(devPtr,port));

        if (macLinkDownSpeed < MTD_MAC_SPEED_LEAVE_UNCHANGED)
        {
            MTD_ATTEMPT(mtdCunitSwReset(devPtr,port)); /* need 2x for changes to macLinkDownSpeed */
        }

        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,0,3,&mediaSelect));

        if (mediaSelect == MTD_MS_FBR_ONLY)
        {
            /* check if T unit is already in fiber only mode/T unit software reset is already in process */
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,15,1,&tunitinswreset));
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_TUNIT_BIST_STATUS_REG,7,1,&tunitinwaitpowerdown));

            if ((tunitinswreset == 1 && tunitinwaitpowerdown == 0) ||
                (tunitinswreset == 0 && tunitinwaitpowerdown == 1))
            {
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
                /* T unit is already in fiber only mode, i.e. both are 1 */
            }
        }

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

MTD_STATUS mtdGetMacInterfaceControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *macType,
    OUT MTD_BOOL *macIfPowerDown,
    OUT MTD_U16 *macIfSnoopSel,
    OUT MTD_U16 *macIfActiveLaneSelect,
    OUT MTD_U16 *macLinkDownSpeed,
    OUT MTD_U16 *macMaxIfSpeed /* only applicable to X33X0/E20X0/E21X0 devices */
)
{
    MTD_U16 temp;

    if (MTD_IS_E21X0_BASE(devPtr->deviceId) || MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        *macIfSnoopSel = MTD_MAC_SNOOP_OFF;
        *macIfActiveLaneSelect = 0;
        return mtdGetMacInterfaceCopperOnlyPhy(devPtr,port,macType,macIfPowerDown,macLinkDownSpeed,macMaxIfSpeed);
    }
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL,0,3,macType));
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,3,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *macIfPowerDown);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,8,2,macIfSnoopSel));                        
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,10,1,macIfActiveLaneSelect)); 
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,6,2,macLinkDownSpeed))

    if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,31,0xF0A8,0,2,macMaxIfSpeed));
    }
    else
    {
        *macMaxIfSpeed = MTD_MAX_MAC_SPEED_NOT_APPLICABLE;
    }

    
    return MTD_OK;
}

/******************************************************************************
 For Copper-Only PHY E20X0 or E21X0 - Simplified Control/Status Interface
******************************************************************************/

#define MTD_PROH 0 /* this from/to mac change is prohibited or this is a reserved mactype */
#define MTD_ALW0 1 /* this from/to mac change is allowed, normal software reset needs to be done */
#define MTD_ALW1 2 /* this from/to mac change is allowed, PCS scrambler bypass must be changed */
#define MTD_ALW2 3 /* this from/to mac change is allowed, bit 4 sw should be used though and NOT 15 */

MTD_STATUS mtdSetMacInterfaceCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 macType,
    IN MTD_BOOL macIfPowerDown,
    IN MTD_U16 macLinkDownSpeed,
    IN MTD_U16 macMaxIfSpeed
)
{
    MTD_U16 currMacType, prevMacType, prevMacLinkDownSpeed, prevMacMaxIfSpeed;
    MTD_BOOL prevMacIfPowerDown;
    MTD_U16 cunitPortCtrl, temp;
    MTD_U16 groupedPortNum, portIndex, firstPortInQuad;
    MTD_U16 fromToPermission; /* whether this change is allowed and how to do it */

    /* check which PHY type this is, handle E20X0 by calling old function to simplify this one */
    if (MTD_IS_E20X0_DEVICE(devPtr->deviceId))
    {
        return mtdSetMacInterfaceControl(devPtr,port,macType,macIfPowerDown,MTD_MAC_SNOOP_OFF,
                                         0,macLinkDownSpeed,macMaxIfSpeed,MTD_TRUE,MTD_TRUE);
    }

    if (MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        /* This table is for E2180. E2110 is a subset of these options. */
        const MTD_U16 allowedMactypeChange[8][8] = /* first index is FROM, second index is TO */
        {                                          
            
        /* FROM    TO:  000b      001b      010b      011b      100b      101b      110b      111b */
        /* 000b */ {MTD_ALW0, MTD_PROH, MTD_PROH, MTD_PROH, MTD_ALW0, MTD_ALW0, MTD_ALW0, MTD_PROH},
        /* 001b */ {MTD_ALW1, MTD_ALW2, MTD_PROH, MTD_PROH, MTD_ALW1, MTD_ALW1, MTD_ALW1, MTD_PROH},
        /* 010b */ {MTD_ALW1, MTD_PROH, MTD_ALW2, MTD_PROH, MTD_ALW1, MTD_ALW1, MTD_ALW1, MTD_PROH},

        /* 011b */ {MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH},

        /* 100b */ {MTD_ALW0, MTD_PROH, MTD_PROH, MTD_PROH, MTD_ALW0, MTD_ALW0, MTD_ALW0, MTD_PROH},
        /* 101b */ {MTD_ALW0, MTD_PROH, MTD_PROH, MTD_PROH, MTD_ALW0, MTD_ALW0, MTD_ALW0, MTD_PROH},
        /* 110b */ {MTD_ALW0, MTD_PROH, MTD_PROH, MTD_PROH, MTD_ALW0, MTD_ALW0, MTD_ALW0, MTD_PROH},
        
        /* 111b */ {MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH, MTD_PROH},      
        };

        if ((macType == MTD_MAC_TYPE_DXGMII) || (macType == MTD_MAC_TYPE_QXGMII))
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: Not allowed macType. Must be set by strap pin!\n");
            return MTD_FAIL;
        }

        if (macLinkDownSpeed > MTD_MAC_SPEED_LEAVE_UNCHANGED)
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: macLinkDownSpeed out of range.\n");
            return MTD_FAIL;
        }  

        if (macLinkDownSpeed <= MTD_MAC_SPEED_1000_MBPS && 
            !(macType == MTD_MAC_TYPE_5BR_SGMII_AN_DIS || macType == MTD_MAC_TYPE_5BR_SGMII_AN_EN))
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: macLinkDownSpeed invalid for this mac type\n");
            return MTD_FAIL;        
        }

        if ((macMaxIfSpeed > MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED || macMaxIfSpeed == 1) ||
            (macType == MTD_MAC_TYPE_SXGMII && !(macMaxIfSpeed == MTD_MAX_MAC_SPEED_10G || macMaxIfSpeed == MTD_MAX_MAC_SPEED_5G
                                                || macMaxIfSpeed == MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED)))
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: macMaxIfSpeed out of range.\n");
            return MTD_FAIL;
        }

        if (macType == MTD_MAC_TYPE_SXGMII && macIfPowerDown == MTD_FALSE) 
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: SXGMII requires macIfPowerDown be MTD_TRUE on this device.\n");
            return MTD_FAIL;        
        }

        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_E21X0_PORT_CTRL,&temp));
        MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp,0,3,&currMacType));

        /* range check on new mactype */
        if (macType == MTD_MAC_LEAVE_UNCHANGED)
        {
            macType = currMacType; 
        }
        else if (macType > MTD_MAC_LEAVE_UNCHANGED)
        {
            MTD_DBG_ERROR("Mac Type out of range.\n");
            return MTD_FAIL;
        }

        if (macType == MTD_MAC_TYPE_DXGMII || macType == MTD_MAC_TYPE_QXGMII)
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: no changes allowed for mac types DXGMII/QXGMII or to DXGMII/QXGMII\n");
            return MTD_FAIL;
        }

        /*  on E2110 device only 4 mac types supported */
        if ((devPtr->numPortsPerDevice == 1) && 
            !(macType == MTD_MAC_TYPE_5BR_SGMII_AN_EN || macType == MTD_MAC_TYPE_5BR_SGMII_AN_DIS || macType == MTD_MAC_TYPE_SXGMII || macType == MTD_MAC_TYPE_10GR_RATE_ADAP) )    
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: invalid mac type for E2110 device\n");
            return MTD_FAIL;
        }
        /* KW */
        if (macType >= 8) {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: invalid mac type %d\n", macType);
            return MTD_FAIL;
        }
        /* End KW */
        /* check from/to combination */
        fromToPermission = allowedMactypeChange[currMacType][macType];

        if (fromToPermission == MTD_PROH)
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: mac type reserved or change not allowed.\n");
            return MTD_FAIL;
        }

        MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,macType,0,3,&temp)); /* set macType in temp for 1.C04A.2:0 */    


        /* set which bit will be used for soft reset, this doesn't do the soft reset yet */
        if (fromToPermission == MTD_ALW2)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,1,4,1,&temp)); /* Use 1.C04A.4 port soft reset */
        }
        else
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,1,15,1,&temp)); /* Use 1.C04A.15 port soft reset */
        }

        if (macLinkDownSpeed < MTD_MAC_SPEED_LEAVE_UNCHANGED)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,6,2,macLinkDownSpeed)); /* do this early will save us 1x soft reset */
        }

        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_E21X0_PORT_CTRL,temp)); /* write mactype and do soft reset at same time */

        if (fromToPermission == MTD_ALW1)
        {
            /* need to disable the PCS scrambler bypass when leaving DXGMII or QXGMII mode */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0x9003,1,1,0));
        }


        if (macMaxIfSpeed < MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED)
        {
            if (macType == MTD_MAC_TYPE_10GR_RATE_ADAP)
            {
                MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,4,2,macMaxIfSpeed));
            } 
            else if (macType == MTD_MAC_TYPE_SXGMII)
            {
                /* can only be 10G or 5G, valid range check was done earlier */
                MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,2,2,((macMaxIfSpeed == MTD_MAX_MAC_SPEED_10G)?0:1)));
            }
            else
            {
                /* other mac types link up follows media, link down follows 31.F000.7:6 and 31.F0A8.1:0 */
                MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,31,0xF0A8,0,2,macMaxIfSpeed));
            }        
        }
        
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,3,1,(MTD_U16)macIfPowerDown));

        MTD_ATTEMPT(mtdCunitSwReset(devPtr,port)); /* do one more in case other stuff changed, also takes care of second one */
                                            /* in case macLinkDownSpeed changed and this function uses correct sw reset */

#if MTD_ORIGSERDES                                           
        MTD_ATTEMPT(mtdRerunSerdesAutoInitUseAutoMode(devPtr,port));
#else
        MTD_DBG_ERROR("Need to include MTD_ORIGSERDES to include serdes code\n");
        return MTD_FAIL;
#endif
    }
    else if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_ATTEMPT(mtdGetMacInterfaceCopperOnlyPhy(devPtr,port,&prevMacType,&prevMacIfPowerDown,&prevMacLinkDownSpeed,&prevMacMaxIfSpeed));

        if (macType == MTD_MAC_LEAVE_UNCHANGED)
        {
            macType = prevMacType;
        }
        else if (macType > MTD_MAC_LEAVE_UNCHANGED &&
                 !(macType == MTD_MAC_TYPE_20G_DXGMII || macType == MTD_MAC_TYPE_20G_QXGMII ||
                   macType == MTD_MAC_TYPE_OXGMII_PARTNER || macType == MTD_MAC_TYPE_10GR_RM_AMD_ON))
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: Mac Type out of range.\n");
            return MTD_FAIL;
        }

        if (macLinkDownSpeed != MTD_MAC_SPEED_NOT_APPLICABLE)
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: macLinkDownSpeed follows the last link up speed, please set to MTD_MAC_SPEED_NOT_APPLICABLE.\n");
            return MTD_FAIL;
        }

        if (macMaxIfSpeed != MTD_MAX_MAC_SPEED_NOT_APPLICABLE)
        {
            MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: macMaxIfSpeed cannot be set on this device.\n");
            return MTD_FAIL;
        }

        if (macType == MTD_MAC_TYPE_SXGMII ||
            macType == MTD_MAC_TYPE_10G_DXGMII || macType == MTD_MAC_TYPE_20G_DXGMII ||
            macType == MTD_MAC_TYPE_10G_QXGMII || macType == MTD_MAC_TYPE_20G_QXGMII ||
            macType == MTD_MAC_TYPE_OXGMII || macType == MTD_MAC_TYPE_OXGMII_PARTNER)
        {
            if (macIfPowerDown == MTD_FALSE)
            {
                MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: MP-USXGMII mode requires macIfPowerDown be MTD_TRUE on this device.\n");
                return MTD_FAIL;
            }

            /* Must pass in the first(Master) port to config DXGMII/QXGMII/OXGMII modes */
            if ((macType == MTD_MAC_TYPE_10G_DXGMII || macType == MTD_MAC_TYPE_20G_DXGMII) && (port%2 != 0))
            {
                MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: Must pass in the Master(first) port of the port group (P0/P2 for DXGMII)\n");
                return MTD_FAIL;
            }

            if ((macType == MTD_MAC_TYPE_10G_QXGMII || macType == MTD_MAC_TYPE_20G_QXGMII ||
                 macType == MTD_MAC_TYPE_OXGMII || macType == MTD_MAC_TYPE_OXGMII_PARTNER) &&
                (port%4 != 0))
            {
                MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: Must pass in the first port of the port group (P0 for P0-3) for QXGMII/OXGMII\n");
                return MTD_FAIL;
            }
        }


        /* Disable OXGMII_PARTNER Mode */
        if (prevMacType == MTD_MAC_TYPE_OXGMII_PARTNER && macType != MTD_MAC_TYPE_OXGMII_PARTNER)
        {            
            /* Setting any port on a chip from MTD_MAC_TYPE_OXGMII_PARTNER to not
               MTD_MAC_TYPE_OXGMII_PARTNER, clear all of the OXGMII partner bits
               on that quad (chip or upper/lower for 8-port device) */
            firstPortInQuad = ((port/4) * 4);
            for (portIndex = firstPortInQuad; 
                 portIndex < (firstPortInQuad + 4); 
                 portIndex++)
            {
                MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,portIndex,4,0xF0AC,13,1,0x0));
            }
        }

        if (macType == MTD_MAC_TYPE_OXGMII_PARTNER)
        {
            /* OXGMII Partner is running 10G QXGMII Mode with additional configuration bits set */
            macType = MTD_MAC_TYPE_10G_QXGMII;

            if (prevMacType != MTD_MAC_TYPE_OXGMII_PARTNER)
            {
                firstPortInQuad = ((port/4) * 4); /* This is actually guaranteed to be first port due to error check above */
                for (portIndex = firstPortInQuad; /* First port is on 0/4/8... boundary checked above, port == firstPortInQuad in this case */
                     portIndex < (firstPortInQuad + 4); 
                     portIndex++)
                {
                    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,portIndex,4,0xF0AC,13,1,0x1));
                }
            }
        }

        /* Disable AP AN */
        if (prevMacType == MTD_MAC_TYPE_10GR_1000BX_AP_AN && macType != MTD_MAC_TYPE_10GR_1000BX_AP_AN)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x1000,12,1,0x0));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,11,1,0x0)); /* Disable KR Training */
        }
        
        /* Enable AP AN */
        if (macType == MTD_MAC_TYPE_10GR_1000BX_AP_AN)
        {
            /* Enable AP AN */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x1000,12,1,0x1));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,11,1,0x1)); /* Enable KR Training */            
        }

        /* Turn AMD Off */
        if (prevMacType == MTD_MAC_TYPE_10GR_RM_AMD_ON && macType != MTD_MAC_TYPE_10GR_RM_AMD_ON)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,7,1,0x0));
        }

        if (macType == MTD_MAC_TYPE_10GR_RM_AMD_ON)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,7,1,0x1));
        }

        /* Set macIfPowerDown */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,3,1,(MTD_U16)macIfPowerDown));

        if (macType == MTD_MAC_TYPE_10G_DXGMII || macType == MTD_MAC_TYPE_20G_DXGMII)
        {
            groupedPortNum = 2; /* set 2 of grouped port */
        }
        else if (macType == MTD_MAC_TYPE_10G_QXGMII || macType == MTD_MAC_TYPE_20G_QXGMII || macType == MTD_MAC_TYPE_OXGMII)
        {
            groupedPortNum = 4; /* set 4 of grouped port */
        }
        else    /* All other modes */
        {
            groupedPortNum = 1;  /* Set each port individually */
        }

        /* Set each port individually for other modes */
        portIndex = port + groupedPortNum;
        do
        {
            portIndex--;
            /* Get Port Control Register */
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,portIndex,MTD_CUNIT_PORT_CTRL,&cunitPortCtrl));
            
            /* Set Mac Type with a sw reset */
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitPortCtrl,(macType&0x7),0,3,&cunitPortCtrl));    /* Use lower 3 bit to set macType in hardware */
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(cunitPortCtrl,0x1,15,1,&cunitPortCtrl)); /* including C Unit soft reset */

            /* Master and Slave ports all need to be set to 10G/20G */
            if (macType == MTD_MAC_TYPE_20G_DXGMII || macType == MTD_MAC_TYPE_20G_QXGMII || macType == MTD_MAC_TYPE_OXGMII)
            {
                macMaxIfSpeed = MTD_MAX_MAC_SPEED_20G;
            }
            else
            {
                macMaxIfSpeed = MTD_MAX_MAC_SPEED_10G;
            }

            /* Readback of 31.F007 is not what was written until after sw reset */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,portIndex,MTD_CUNIT_PORT_CTRL2,2,2,macMaxIfSpeed));

            /* Apply sw reset to make mode change and to take speed change for DXGMII/QXGMII/OXGMII */
            /* Going in reverse order makes master port be last in the reset chain */
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,portIndex,MTD_CUNIT_PORT_CTRL,cunitPortCtrl));

            /* Let the T unit firmware pick up any changes to mactypes */
            MTD_ATTEMPT(mtdSoftwareReset(devPtr, portIndex, 1000)); /* The firmware does another C unit sw reset for MP-USXGMII modes when it sets the port ids */
        }
        while (portIndex > port);
                
    }
    else
    {
        MTD_DBG_ERROR("mtdSetMacInterfaceCopperOnlyPhy: Device not supported or the device is not copper-only\n");
        return MTD_FAIL;
    }

    return MTD_OK;    
}

MTD_STATUS mtdGetMacInterfaceCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *macType,
    OUT MTD_BOOL *macIfPowerDown,
    OUT MTD_U16 *macLinkDownSpeed,
    OUT MTD_U16 *macMaxIfSpeed 
)
{
    MTD_U16 dummy1,dummy2,temp;

    /* check which PHY type this is, handle E20X0 by calling old function to simplify this one */    
    if (MTD_IS_E20X0_DEVICE(devPtr->deviceId))
    {
        return mtdGetMacInterfaceControl(devPtr,port,macType,macIfPowerDown,&dummy1,&dummy2,macLinkDownSpeed,macMaxIfSpeed);
    }

    if (MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_E21X0_PORT_CTRL,0,3,macType));
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,3,1,&dummy1));
        MTD_CONVERT_UINT_TO_BOOL(dummy1, *macIfPowerDown);
                        
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,6,2,macLinkDownSpeed));

        if (*macType == MTD_MAC_TYPE_10GR_RATE_ADAP)
        {
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,4,2,macMaxIfSpeed));
        }
        else if (*macType == MTD_MAC_TYPE_SXGMII)
        {
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,2,2,&temp));
            
            if (temp == 0)
            {
                *macMaxIfSpeed = MTD_MAX_MAC_SPEED_10G;
            }
            else if (temp == 1)
            {
                *macMaxIfSpeed = MTD_MAX_MAC_SPEED_5G;
            }
            else
            {
                *macMaxIfSpeed = temp;
                MTD_DBG_ERROR("mtdGetMacInterfaceCopperOnlyPhy: macMaxIfSpeed invalid value.\n");
                return MTD_FAIL;
            }
        }
        else
        {
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,31,0xF0A8,0,2,macMaxIfSpeed));
        }
    }
    else if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        /* Get Host Speed */
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,2,2,macMaxIfSpeed));

        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL,0,3,macType));
        if (*macType == MTD_MAC_TYPE_10G_DXGMII)
        {
            if (*macMaxIfSpeed == MTD_MAX_MAC_SPEED_20G)
            {
                /* MTD_MAC_TYPE_20G_DXGMII */
                *macType = *macType | (1<<3);
            }
        }

        if (*macType == MTD_MAC_TYPE_10G_QXGMII)
        {
            if (*macMaxIfSpeed == MTD_MAX_MAC_SPEED_20G)
            {
                /* MTD_MAC_TYPE_20G_QXGMII */
                *macType = *macType | (1<<3);
            }
            else    /* MTD_MAC_TYPE_10G_QXGMII */
            {
                MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,4,0xF0AC,13,1,&temp));
                if (temp == 1)
                {
                    *macType = MTD_MAC_TYPE_OXGMII_PARTNER;
                }
            }
        }

        if (*macType == MTD_MAC_TYPE_10GR_RM_AMD_OFF)
        {
            MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,7,1,&temp))
            if (temp == 1)
            {
                *macType = MTD_MAC_TYPE_10GR_RM_AMD_ON;
            }
        }

        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,3,1,&temp));
        *macIfPowerDown = MTD_GET_BIT_AS_BOOL(temp);

        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,6,2,macLinkDownSpeed));
    }
    else
    {
        MTD_DBG_ERROR("mtdGetMacInterfaceCopperOnlyPhy: Device not supported or the device is not copper-only\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}



/******************************************************************************
 For Copper-Only PHY E21X0 Only
******************************************************************************/

MTD_STATUS mtdSetMacInterfaceRateMatchingSpeedCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 rateMatchingSpeed    
)
{
    if (!MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        MTD_DBG_ERROR("This option unsupported on this PHY\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,4,2,rateMatchingSpeed));

    MTD_ATTEMPT(mtdCunitSwReset(devPtr,port));
    
    return MTD_OK;
}


MTD_STATUS mtdGetMacInterfaceRateMatchingSpeedCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *rateMatchingSpeed    
)
{
    if (!(MTD_IS_E21X0_BASE(devPtr->deviceId) || MTD_IS_X35X0_BASE(devPtr->deviceId)))
    {
        MTD_DBG_ERROR("This option unsupported on this PHY\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_PORT_CTRL2,4,2,rateMatchingSpeed));
    
    return MTD_OK;
}

/******************************************************************************
 USXGMII functions
******************************************************************************/

MTD_STATUS mtdGetMultiUSXGMIIConfigurationCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *usxgmiiPortId,
    OUT MTD_BOOL *usxgmiiPrimaryPort
)
{
    MTD_U16 tmp,tmp2;
    
    if (!(MTD_IS_E21X0_BASE(devPtr->deviceId) || MTD_IS_X35X0_BASE(devPtr->deviceId)))
    {
        MTD_DBG_ERROR("This option unsupported on this PHY\n");
        return MTD_FAIL;
    }
    else
    {
        if (devPtr->numPortsPerDevice == 1)
        {
            MTD_DBG_ERROR("This option only applicable to multi-port PHY\n");
            return MTD_FAIL;
        }
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_CUNIT_PORT_CTRL2,&tmp));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(tmp,0,2,usxgmiiPortId));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(tmp,6,1,&tmp2));
    *usxgmiiPrimaryPort = MTD_GET_BIT_AS_BOOL(tmp2);

    return MTD_OK;
    
}




/******************************************************************************
 SGMII functions
******************************************************************************/

MTD_STATUS mtdGetSGMIIAutoneg_Advertisement
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *link_status,
    OUT MTD_BOOL *full_duplex,
    OUT MTD_U16  *speed,
    OUT MTD_BOOL *tx_pause,
    OUT MTD_BOOL *rx_pause,
    OUT MTD_BOOL *fiber_media,
    OUT MTD_BOOL *eee_enabled,
    OUT MTD_BOOL *clock_stop_lpi
)
{
    MTD_U16 temp;

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,15,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *link_status);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,12,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *full_duplex);
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,10,2,speed));
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,9,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *tx_pause);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,8,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *rx_pause);
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,7,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *fiber_media);
    

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,6,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *eee_enabled);
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_1000X_SGMII_AN_ADV,5,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *clock_stop_lpi);
        
    return MTD_OK;
}

#if MTD_ORIGSERDES

/******************************************************************************
 XFI functions
******************************************************************************/

MTD_STATUS mtdSetANKRControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL enableANKR
)
{
    if (MTD_IS_X32X0_BASE(devPtr->deviceId))
    {
        if (enableANKR)
        {
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,7,0x9006,0x0000)); /* device will drop the serdes link when this changes */
        }
        else
        {
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,7,0x9006,0x0100)); /* device will drop the serdes link when this changes */            
        }
    }
    else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        if (enableANKR)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF084,15,1,1));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF07C,14,2,1));
            
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x1000,15,1,1)); /* restarts the link, bit self clears */
        }
        else
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF084,15,1,0));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF07C,14,2,2));
            
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x1000,15,1,1)); /* restarts the link, bit self clears */
        }
    }  
    else if (MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        if (enableANKR)
        {
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,7,0x1000,0x1200)); /* Enable and restart AP AN */
        }
        else
        {
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,7,0x1000,0x0200)); /* Disable and restart AP AN */            
        }
    }
    else
    {
        return MTD_FAIL;
    }
    
    return MTD_OK;
}


MTD_STATUS mtdGetANKRControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *enableANKR
)
{
    MTD_U16 temp;

    *enableANKR = MTD_FALSE;
    
    if (MTD_IS_X32X0_BASE(devPtr->deviceId))
    {
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,7,0x9006,&temp));
        
        if (temp == 0x0000)
        {
            *enableANKR = MTD_TRUE;
        }
        else if (temp == 0x0100)
        {
            *enableANKR = MTD_FALSE;
        }
        else
        {
            return MTD_FAIL;
        }
    }
    else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,4,0xF084,15,1,&temp));

        if (temp)
        {
            *enableANKR = MTD_TRUE;            
        }
        else
        {
            *enableANKR = MTD_FALSE;
        }        
    }  
    else if (MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,0x1000,12,1,&temp));

        if (temp)
        {
            *enableANKR = MTD_TRUE;            
        }
        else
        {
            *enableANKR = MTD_FALSE;
        }  
    }
    else
    {
        return MTD_FAIL;
    }
    
    return MTD_OK;
}


/******************************************************************************
 X2 (RXAUI) functions
******************************************************************************/

MTD_STATUS mtdSetX2SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL invert_input_pol_l0,
    IN MTD_BOOL invert_output_pol_l0,
    IN MTD_BOOL invert_input_pol_l1,
    IN MTD_BOOL invert_output_pol_l1
)
{
    MTD_U8 input_pol_l0_bit, output_pol_l0_bit, input_pol_l1_bit, output_pol_l1_bit;
    MTD_U16 temp;

    if (MTD_IS_X32X0_BASE(devPtr->deviceId))
    {
        input_pol_l1_bit = 15;
        input_pol_l0_bit = 14;
        output_pol_l1_bit = 13;
        output_pol_l0_bit = 12;
    }
    else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        input_pol_l1_bit = 13;        
        input_pol_l0_bit = 12;
        output_pol_l1_bit = 9;        
        output_pol_l0_bit = 8;
    }
    else
    {
        return MTD_FAIL; /* other devices don't have this control */
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL2,&temp));
    
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(invert_input_pol_l1),input_pol_l1_bit,1,&temp));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(invert_input_pol_l0),input_pol_l0_bit,1,&temp));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(invert_output_pol_l1),output_pol_l1_bit,1,&temp));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(invert_output_pol_l0),output_pol_l0_bit,1,&temp));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL2,temp));
    
    return MTD_OK;
}

MTD_STATUS mtdGetX2SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL *invert_input_pol_l0,
    IN MTD_BOOL *invert_output_pol_l0,
    IN MTD_BOOL *invert_input_pol_l1,
    IN MTD_BOOL *invert_output_pol_l1    
)
{
    MTD_U16 temp,temp2;
    MTD_U8 input_pol_l0_bit, output_pol_l0_bit, input_pol_l1_bit, output_pol_l1_bit;

    if (MTD_IS_X32X0_BASE(devPtr->deviceId))
    {
        input_pol_l1_bit = 15;
        input_pol_l0_bit = 14;
        output_pol_l1_bit = 13;
        output_pol_l0_bit = 12;
    }
    else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
    {
        input_pol_l1_bit = 13;        
        input_pol_l0_bit = 12;
        output_pol_l1_bit = 9;        
        output_pol_l0_bit = 8;
    }   
    else
    {
        return MTD_FAIL; /* other devices don't have this control */
    }
    

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL2,&temp2));

    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp2,input_pol_l1_bit,1,&temp));   
    MTD_CONVERT_UINT_TO_BOOL(temp, *invert_input_pol_l1);
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp2,input_pol_l0_bit,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *invert_input_pol_l0);
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp2,output_pol_l1_bit,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *invert_output_pol_l1);
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp2,output_pol_l0_bit,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *invert_output_pol_l0);
    
    return MTD_OK;
}

MTD_STATUS mtdSetX4SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 inputPolarityMask,
    IN MTD_U16 outputPolarityMask
)
{
    MTD_U16 temp;

    if (MTD_IS_X33X0_BASE(devPtr->deviceId) && devPtr->numPortsPerDevice == 1)
    {
    
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL2,&temp));

        MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,inputPolarityMask,12,4,&temp));
        MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,outputPolarityMask,8,4,&temp));

        
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL2,temp));
    }
    else
    {
        /* only 33X0 single port devices have this control */
        return MTD_FAIL;
    }

    return MTD_OK;
}

MTD_STATUS mtdGetX4SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 *inputPolarityMask,
    IN MTD_U16 *outputPolarityMask
)
{
    MTD_U16 temp;
    
    if (MTD_IS_X33X0_BASE(devPtr->deviceId) && devPtr->numPortsPerDevice == 1)
    {
    
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL2,&temp));

        MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp,12,4,inputPolarityMask));    

        MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp,8,4,outputPolarityMask));        
    }
    else
    {
        /* only 33X0 single port devices have this control */
        return MTD_FAIL;
    }

    return MTD_OK;
    
}



#endif /* MTD_ORIGSERDES */


