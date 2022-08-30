
/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for
higher-level functions using MDIO access to control and read 
status of the MACsec Legacy Mode Buffer energy efficient ethernet 
(EEE) functions of the Marvell 88X32X0, 88X33X0 and 88E20X0 ethernet PHYs.
********************************************************************/

#include <mtdFeatures.h>

#if MTD_LINKCRYPT_MSECPTP

#include <mtdApiTypes.h>
#include <mtdApiRegs.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <LinkCryptPtp/mtdHwMsecPtpCntl.h>
#include <LinkCryptPtp/mtdMsecEEE.h>
#include <mtdEEE.h>


MTD_STATUS mtdMACSECEeeBufferConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 EEE_Mode_Control, 
    IN MTD_U8 XGMII_Enter_Timeout, 
    IN MTD_U8 GMII_Enter_Timeout, 
    IN MTD_U8 MII_Enter_Timeout, 
    IN MTD_U8 XGMII_Exit_Timeout, 
    IN MTD_U8 GMII_Exit_Timeout, 
    IN MTD_U8 MII_Exit_Timeout, 
    IN MTD_BOOL Force_Sys_LPI, 
    IN MTD_BOOL Force_Wre_LPI 
)
{
    MTD_U16 macSecEnable;

    /* make sure macsec was already turned on */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,12,2,&macSecEnable));

    if (macSecEnable != 3)
    {
        return MTD_FAIL;
    }

    if (XGMII_Enter_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,4,8,(MTD_U32)XGMII_Enter_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,4,8,(MTD_U32)0xFF));
    }

    if (XGMII_Exit_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,0,8,(MTD_U32)XGMII_Exit_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,0,8,(MTD_U32)0x4B));
    }

    if (GMII_Enter_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,12,8,(MTD_U32)GMII_Enter_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,12,8,(MTD_U32)0x12));
    }

    if (GMII_Exit_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,8,8,(MTD_U32)GMII_Exit_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,8,8,(MTD_U32)0x12));
    }
    
    if (MII_Enter_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,20,8,(MTD_U32)MII_Enter_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,20,8,(MTD_U32)0x20));
    }

    if (MII_Exit_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,16,8,(MTD_U32)MII_Exit_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,16,8,(MTD_U32)0x20));
    }

    if (Force_Sys_LPI == MTD_TRUE)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,3,1,(MTD_U32)1));
    }
    else
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,3,1,(MTD_U32)0));
    }

    if (Force_Wre_LPI == MTD_TRUE)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,2,1,(MTD_U32)1));
    }
    else
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,2,1,(MTD_U32)0));
    }

    switch (EEE_Mode_Control)
    {
        case MTD_EEE_MODE_DISABLE:
            MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,0,2,(MTD_U32)0));
            break;

        case MTD_EEE_MODE_ENABLE_NO_LEGACY:
            MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,0,2,(MTD_U32)2));
            break;

        case MTD_EEE_MODE_ENABLE_WITH_LEGACY:
            MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809C,0,2,(MTD_U32)1));
            break;

        default:
            return MTD_FAIL;
            break;
    }

    return MTD_OK;
    
}

MTD_STATUS mtdMACSECEeeBufferConfig5G_2P5G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 FiveG_Enter_Timeout, 
    IN MTD_U8 TwoP5G_Enter_Timeout, 
    IN MTD_U8 FiveG_Exit_Timeout, 
    IN MTD_U8 TwoP5G_Exit_Timeout
)
{
    MTD_U16 macSecEnable;

    /* make sure macsec was already turned on */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,12,2,&macSecEnable));

    if (macSecEnable != 3)
    {
        return MTD_FAIL;
    }

    if (FiveG_Enter_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x80A0,0,8,(MTD_U32)FiveG_Enter_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x80A0,0,8,(MTD_U32)0xB4));
    }

    if (FiveG_Exit_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x80A0,8,8,(MTD_U32)FiveG_Exit_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x80A0,8,8,(MTD_U32)0x96));
    }    


    if (TwoP5G_Enter_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x80A0,16,8,(MTD_U32)TwoP5G_Enter_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x80A0,16,8,(MTD_U32)0x12));
    }

    if (TwoP5G_Exit_Timeout)
    {
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,24,8,(MTD_U32)TwoP5G_Exit_Timeout));
    }
    else
    {
        /* it was 0, so set it to default */
        MTD_ATTEMPT(mtdHwSetMsecPhyRegField(devPtr,port,31,0x809E,24,8,(MTD_U32)0x1E));
    }    

    return MTD_OK;
    
}


MTD_STATUS mtdGetMACSECEeeBufferConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *EEE_Mode_Control, 
    OUT MTD_U8 *XGMII_Enter_Timeout, 
    OUT MTD_U8 *GMII_Enter_Timeout, 
    OUT MTD_U8 *MII_Enter_Timeout, 
    OUT MTD_U8 *XGMII_Exit_Timeout, 
    OUT MTD_U8 *GMII_Exit_Timeout, 
    OUT MTD_U8 *MII_Exit_Timeout, 
    OUT MTD_BOOL *Force_Sys_LPI, 
    OUT MTD_BOOL *Force_Wre_LPI 
)
{
    MTD_U32 U32EEE_Mode_Control,
        U32XGMII_Enter_Timeout,
        U32GMII_Enter_Timeout,
        U32MII_Enter_Timeout,
        U32XGMII_Exit_Timeout,
        U32GMII_Exit_Timeout,
        U32MII_Exit_Timeout,
        U32Force_Sys_LPI,
        U32Force_Wre_LPI;
    MTD_U16 macSecEnable;

    /* make sure macsec was already turned on */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,12,2,&macSecEnable));

    if (macSecEnable != 3)
    {
        return MTD_FAIL;
    }
    
    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809C,4,8,&U32XGMII_Enter_Timeout));
    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809E,0,8,&U32XGMII_Exit_Timeout));
    *XGMII_Enter_Timeout = (MTD_U8)U32XGMII_Enter_Timeout;
    *XGMII_Exit_Timeout = (MTD_U8)U32XGMII_Exit_Timeout;

    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809C,12,8,&U32GMII_Enter_Timeout));
    *GMII_Enter_Timeout = (MTD_U8)U32GMII_Enter_Timeout;

    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809E,8,8,&U32GMII_Exit_Timeout));
    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809C,20,8,&U32MII_Enter_Timeout));
    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809E,16,8,&U32MII_Exit_Timeout));
    *GMII_Exit_Timeout = (MTD_U8)U32GMII_Exit_Timeout;
    *MII_Enter_Timeout = (MTD_U8)U32MII_Enter_Timeout;
    *MII_Exit_Timeout = (MTD_U8)U32MII_Exit_Timeout;

    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809C,2,1,&U32Force_Wre_LPI));
    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809C,3,1,&U32Force_Sys_LPI));
    *Force_Sys_LPI = (MTD_BOOL)U32Force_Sys_LPI;
    *Force_Wre_LPI = (MTD_BOOL)U32Force_Wre_LPI;

    MTD_ATTEMPT(mtdHwGetMsecPhyRegField(devPtr,port,31,0x809C,0,2,&U32EEE_Mode_Control));
    *EEE_Mode_Control = (MTD_U8)U32EEE_Mode_Control;
    if (*EEE_Mode_Control == 3)
    {
        return MTD_FAIL; /* This setting doesn't make sense */
    }
    else if (*EEE_Mode_Control == 2)
    {
        *EEE_Mode_Control = MTD_EEE_MODE_ENABLE_NO_LEGACY;
    }
    else if (*EEE_Mode_Control == 1)
    {
        *EEE_Mode_Control = MTD_EEE_MODE_ENABLE_WITH_LEGACY;
    }
    /* else it's 0, disabled, don't do anything to it, returns MTD_EEE_MODE_DISABLE */

    return MTD_OK;  
    
}

MTD_STATUS mtdGetMACSECEeeBufferConfig5G_2P5G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *FiveG_Enter_Timeout, 
    OUT MTD_U8 *TwoP5G_Enter_Timeout, 
    OUT MTD_U8 *FiveG_Exit_Timeout, 
    OUT MTD_U8 *TwoP5G_Exit_Timeout
)
{
    MTD_U32 U32FiveG_Enter_Timeout, U32TwoP5G_Enter_Timeout, U32FiveG_Exit_Timeout, 
            U32TwoP5G_Exit_Timeout, tempVal;
    MTD_U16 macSecEnable;

    /* make sure macsec was already turned on */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_CUNIT_MODE_CONFIG,12,2,&macSecEnable));

    if (macSecEnable != 3)
    {
        return MTD_FAIL;
    }  

    MTD_ATTEMPT(mtdHwMsecRead(devPtr,port,31,0x80A0,&tempVal));
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromLongWord(tempVal,0,8,&U32FiveG_Enter_Timeout));
    *FiveG_Enter_Timeout = (MTD_U8)U32FiveG_Enter_Timeout;
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromLongWord(tempVal,8,8,&U32FiveG_Exit_Timeout));
    *FiveG_Exit_Timeout = (MTD_U8)U32FiveG_Exit_Timeout;
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromLongWord(tempVal,16,8,&U32TwoP5G_Enter_Timeout));
    *TwoP5G_Enter_Timeout = (MTD_U8)U32TwoP5G_Enter_Timeout;

    MTD_ATTEMPT(mtdHwMsecRead(devPtr,port,31,0x809E,&tempVal));
    
    MTD_ATTEMPT(mtdHwGetRegFieldFromLongWord(tempVal,24,8,&U32TwoP5G_Exit_Timeout));
    *TwoP5G_Exit_Timeout = (MTD_U8)U32TwoP5G_Exit_Timeout;

    return MTD_OK;
    
}

#endif  /* MTD_LINKCRYPT_MSECPTP */
