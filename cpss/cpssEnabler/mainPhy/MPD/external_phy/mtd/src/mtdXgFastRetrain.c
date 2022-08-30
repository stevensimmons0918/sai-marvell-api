/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions using MDIO access to control and get
status of the 10GBASE-T fast retrain functionality for the 
Marvell 88X32X0, 88X33X0, 88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.

It also allows configuring and checking the 2.5/5GBASE-T fast retrain.
********************************************************************/
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <mtdXgFastRetrain.h>
#include <mpdTypes.h>
/*******************************************************************
 Fast Retrain Control/Status - Top Level Functions

 Use these functions to configure/enable the NFR/10GBTFR and to 
 check the resolution to see which fast retrain type will be used
 following auto-negotiation.

 It is suggested to use only the top-level function for configuration.

 After the link is up, the function mtdGetFastRetrainResolution()
 should be called to determine the type of fast retrain which is being
 used. 

 While the link is up, mtdGetFastRetrainStatus() can be used to read
 the status (which will clear bits in 1.147) and the corresponding
 set of functions can be called to test the status (see the list of 
 functions in the description for mtdGetFastRetrainStatus()).

 If only the 10GBTFR is desired, the functions in the 10GBTFR section
 plus mtdGetFastRetrainStatus() can be used instead of the top-level
 functions.

 If only the NFR is desired, the functions in the NFR section plus
 mtdGetFastRetrainResolution() and mtdGetFastRetrainStatus() can 
 be used instead of the top-level configuration function.

 Most 10GBTFR functions are shared with the 2.5/5G NFR.

 If NFR is advertised for 10GBT, it is also being advertised for 2.5/5G.

 *******************************************************************/
MTD_STATUS mtdConfigureFastRetrain
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 fr_mode, 
    IN MTD_U16 frSignalType,
    IN MTD_BOOL anRestart
)
{
    MTD_U16 speeds_available, speeds_not_available;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr,port,&speeds_available));
    speeds_not_available = ~(speeds_available);

    /* tried to enable 2.5/5G option on a device that doesn't support both of them */
    if ((speeds_not_available & MTD_SPEED_2P5GIG_FD) && (speeds_not_available & MTD_SPEED_5GIG_FD) && (fr_mode & 0xF0))
    {
        MTD_DBG_ERROR("mtdConfigureFastRetrain: fr_mode 0x%04X is not supported on this device, 2.5G or 5G is not supported on this device\n", fr_mode);
        return MTD_FAIL;
    }

    /* set 10GBT fast retrain options, allow on all PHYs on purpose, some use this even for 2.5/5G */
    switch ((fr_mode & 0x0F))
    {
        case MTD_DISABLE_FR:
            MTD_ATTEMPT(mtdDoNotAdvertNFR(devPtr,port,MTD_FALSE));
            MTD_ATTEMPT(mtdDoNotAdvert10GBTFR(devPtr,port,MTD_FALSE));
            MTD_ATTEMPT(mtdSetFrSignalType(devPtr,port,frSignalType));
            break;

        case MTD_ENABLE_10GBTFR_ONLY:
            MTD_ATTEMPT(mtdDoNotAdvertNFR(devPtr,port,MTD_FALSE));
            MTD_ATTEMPT(mtdSetFrSignalType(devPtr,port,frSignalType));
            MTD_ATTEMPT(mtdAdvert10GBTFR(devPtr,port,MTD_FALSE));
            break;

        case MTD_ENABLE_NFR_ONLY:
            MTD_ATTEMPT(mtdDoNotAdvert10GBTFR(devPtr,port,MTD_FALSE));
            MTD_ATTEMPT(mtdAdvertNFR(devPtr,port,frSignalType, MTD_FALSE));
            break;

        case MTD_ENABLE_BOTH_FR:
            MTD_ATTEMPT(mtdAdvertNFR(devPtr,port,frSignalType,MTD_FALSE));
            MTD_ATTEMPT(mtdAdvert10GBTFR(devPtr,port,MTD_FALSE));
            break;
            
        default:
            return MTD_FAIL; /* some kind of error occurred*/
    }

    /* now set 2.5/5G options on devices that have it, skip on those that don't have it */
    if (speeds_available & (MTD_SPEED_2P5GIG_FD | MTD_SPEED_5GIG_FD))
    {
        fr_mode &= 0xF0;

        if (fr_mode == MTD_DISABLE_MBT_FR)
        {
            /* all are disabled */
            MTD_ATTEMPT(mtdDoNotAdvert2P5GBTFR(devPtr,port,MTD_FALSE));
            MTD_ATTEMPT(mtdDoNotAdvert5GBTFR(devPtr,port,MTD_FALSE));
        }
        else
        {
            /* some options are enabled */
            if (fr_mode & MTD_ENABLE_2P5G_FR)
            {
                MTD_ATTEMPT(mtdAdvert2P5GBTFR(devPtr,port,((fr_mode & MTD_SEND_2P5G_THP_REQ) ? MTD_TRUE : MTD_FALSE), MTD_FALSE));
            }

            if (fr_mode & MTD_ENABLE_5G_FR)
            {
                MTD_ATTEMPT(mtdAdvert5GBTFR(devPtr,port,((fr_mode & MTD_SEND_5G_THP_REQ) ? MTD_TRUE : MTD_FALSE), MTD_FALSE));
            }
        }
    }

    if(anRestart)
    {
        MTD_ATTEMPT(mtdAutonegEnable(devPtr,port));
        MTD_ATTEMPT(mtdAutonegRestart(devPtr,port));
    }   

    return MTD_OK;
}

/* applicable to 10G and 2.5/5G */
MTD_STATUS mtdGetFastRetrainResolution
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *frResolution
)
{
    MTD_U16 speed, tempResolution;
    MTD_BOOL linkStat;

    *frResolution = MTD_FR_DISABLED;
    
    /* link has to be up to check 2.5/5G at least */
    MTD_ATTEMPT(mtdIsBaseTUp(devPtr,port,&speed,&linkStat));

    if (linkStat == MTD_FALSE)
    {
        return MTD_FAIL;
    }

    if (speed <= MTD_SPEED_10GIG_FD)
    {
        return (mtdHwGetPhyRegField(devPtr,port,7,
                32781,10,3,frResolution)); /* for 10G result is in this register */
    }
    else
    {
        MTD_BOOL frResolved,ldRequest,lpRequest;
        
        /* it's up at 2.5G or 5G, have to look at what was sent/received in info field during training */
        switch (speed)
        {
            case MTD_SPEED_2P5GIG_FD:
                MTD_ATTEMPT(mtdIs2P5GBTFRNegotiated(devPtr,port,&frResolved,&ldRequest,&lpRequest));
                if (frResolved)
                {
                    *frResolution = MTD_2P5G_FR_RESOLVED;
                }
                break;
                
            case MTD_SPEED_5GIG_FD:
                MTD_ATTEMPT(mtdIs5GBTFRNegotiated(devPtr,port,&frResolved,&ldRequest,&lpRequest));
                if (frResolved)
                {
                    *frResolution = MTD_5G_FR_RESOLVED;
                }
                break;
                
            default:
                return MTD_FAIL;
                break;
        }

        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,\
                32781,10,3,&tempResolution)); /* check if NFR was resolved or not */

        if ((*frResolution == MTD_FR_DISABLED) && (tempResolution == MTD_NFR_RESOLVED))
        {
            *frResolution = MTD_NFR_RESOLVED; /* if 2.5/5G did not resolve to standard fast retrain */
                                              /* but NFR was advertised by both, then NFR is being used for 2.5/5G */
        }
    }    

    return MTD_OK;
}

/* most bits are shared between 10G and 2.5/5G (all but 10GBT FR negotiated) */
MTD_STATUS mtdGetFastRetrainStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 *currentValue
)
{

    return(mtdHwXmdioRead(devPtr,port,1,147,currentValue));
}

MTD_STATUS mtdSetFrSignalType
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 frSignalType
)
{
    if (frSignalType == MTD_FR_IDLE || frSignalType == MTD_FR_LOCAL_FAULT ||
        frSignalType == MTD_FR_LINK_INTRRUPTION)
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,1,147,1,2,frSignalType));
    }
    else
    {
        MTD_DBG_ERROR("mtdSetFrSignalType: Incorrect fast retrain signal type selected\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/*******************************************************************
 Fast Retrain Control/Status - NFR-specific Functions

 Use these functions to configure/enable and check the status of
 only the NFR (negotiated fast retrain).

 NFR applies to both 10G and 2.5/5G.
 *******************************************************************/
MTD_STATUS mtdAdvertNFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 frSignalType,
    IN MTD_BOOL anRestart
)
{    
    /* set the signalling type for what gets sent to the MAC on a fast retrain */
    /* this will also clear the counts since it reads 1.147 */
    if (frSignalType == MTD_FR_IDLE || frSignalType == MTD_FR_LOCAL_FAULT ||
        frSignalType == MTD_FR_LINK_INTRRUPTION)
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,1,147,1,2,frSignalType));
    }
    else
    {
        return MTD_FAIL;
    }

    /* disable the 10GBTFR */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,1,147,0,1,0));
    
    /* enable NFR to be advertised */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x800C,0,1,1));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }

    return MTD_OK;

}

MTD_STATUS mtdDoNotAdvertNFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    /* disable NFR to be advertised */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x800C,0,1,0));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }

    return MTD_OK;
}

MTD_STATUS mtdLPNFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *lpNFRAbility
)
{
    MTD_U16 val;

    *lpNFRAbility = MTD_FALSE;
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,0x800D, 0,1, &val));

    if (val)
    {
        *lpNFRAbility = MTD_TRUE;
    }

    return MTD_OK;
}

MTD_STATUS mtdGetLDNFRCount
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 currentValue,
    OUT MTD_U16 *ldCount
)
{    
	MPD_UNUSED_PARAM(devPtr);
    /* Bits 10:6 in reg 1.147 give the LD fast retrain count */
    return(mtdHwGetRegFieldFromWord(currentValue,6,5,ldCount));
}


/*******************************************************************
 Fast Retrain Control/Status - 10GBTFR-specific Functions

 Use these functions to configure/enable and check the status of
 only the 10GBTFR (standard-defined fast retrain). Some are
 applicable to 2.5/5G, see comments.
 *******************************************************************/

/* next two are applicable to 10G and also 2.5/5G */
MTD_STATUS mtdGetLP10GBTFRCount
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 currentValue,
    OUT MTD_U16 *lpCount
)
{    
	MPD_UNUSED_PARAM(devPtr);
	/*Bits 15:11 in reg 1.147 give the LP fast retrain count.*/
    return(mtdHwGetRegFieldFromWord(currentValue,11,5,lpCount));
}

MTD_STATUS mtdGetLD10GBTFRCount
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 currentValue,
    OUT MTD_U16 *ldCount
)
{
	MPD_UNUSED_PARAM(devPtr);
    /*Bits 10:6 in reg 1.147 give the LD fast retrain count.*/
    return(mtdHwGetRegFieldFromWord(currentValue,6,5,ldCount));
}

/* 10GBT only */
MTD_STATUS mtdIs10GBTFRNegotiated
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 currentValue,
    OUT MTD_BOOL *isNegotiated
)
{
    MTD_U16 result;
    MPD_UNUSED_PARAM(devPtr);
    /*Bit 3 in reg 1.147 give status if 10GBTFR was negotiated. If '1' it 
      was negotiated else it was not negotiated*/
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(currentValue,3,1,&result));

    result ? (*isNegotiated = MTD_TRUE) : (*isNegotiated = MTD_FALSE);

    return MTD_OK;
}

/* 10GBASE-T only */
MTD_STATUS mtdAdvert10GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    /* 7.32.1 = 1 */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,32,1,1,1));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }

    return MTD_OK;
}

/* 10GBASE-T only */
MTD_STATUS mtdDoNotAdvert10GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 anRestart
)
{
    /* 7.32.1 = 1 */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,32,1,1,0));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }

    return MTD_OK;
}

/* Does Link partner have 10GBT Fast Retrain ability, 10GBASE-T only  */ 
MTD_STATUS mtdLP10GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *lp10BTFRAble
)
{
    MTD_U16 val;

    *lp10BTFRAble = MTD_FALSE;

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,33,1,1,&val));

    if (val)
    {
        *lp10BTFRAble = MTD_TRUE;
    }

    return MTD_OK;
}

/*******************************************************************
 Fast Retrain Control/Status for IEEE 2.5G and 5G fast retrain
 (multigbase-t) fast retrain.
 *******************************************************************/

MTD_STATUS mtdAdvert2P5GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL thpRequest,
    IN MTD_BOOL anRestart
)
{
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    if ((availableSpeeds & MTD_SPEED_2P5GIG_FD) == 0)
    {
        MTD_DBG_ERROR("mtdAdvert2P5GBTFR: 2.5G is not supported on this device.\n");
        return MTD_FAIL;
    }
    
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,32,5,1,1));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x0040,3,1,(thpRequest ? 1 : 0)));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }
    
    return MTD_OK;    
}

MTD_STATUS mtdDoNotAdvert2P5GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 anRestart
)
{
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,32,5,1,0));
        
    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }

    return MTD_OK;    
}


MTD_STATUS mtdAdvert5GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL thpRequest,
    IN MTD_BOOL anRestart
)
{
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    if ((availableSpeeds & MTD_SPEED_5GIG_FD) == 0)
    {
        MTD_DBG_ERROR("mtdAdvert5GBTFR: 5G is not supported on this device.\n");
        return MTD_FAIL;
    }
    
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,32,6,1,1));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,0x0040,2,1,(thpRequest ? 1 : 0)));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }
    
    return MTD_OK;    
}

MTD_STATUS mtdDoNotAdvert5GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 anRestart
)
{
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,7,32,6,1,0));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(devPtr,port) ||
                             mtdAutonegRestart(devPtr,port)));
    }

    return MTD_OK;    
}


MTD_STATUS mtdIs2P5GBTFRNegotiated
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,    
    OUT MTD_BOOL *isNegotiated,
    OUT MTD_BOOL *thpLdRequest,
    OUT MTD_BOOL *thpLpRequest
)
{
    MTD_U16 ld2p5g, lp2p5g, ldThp, lpThp;
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    if ((availableSpeeds & MTD_SPEED_2P5GIG_FD) == 0)
    {
        MTD_DBG_ERROR("mtdIs2P5GBTFRNegotiated: 2.5G is not supported on this device.\n");
        return MTD_FAIL;
    }

    /* assumes link is up at the speed of 2.5G */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,32,5,1,&ld2p5g));
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,33,3,1,&lp2p5g));
    *isNegotiated = (ld2p5g && lp2p5g);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,0x0040,3,1,&ldThp));
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,0x0041,3,1,&lpThp));
    *thpLdRequest = MTD_GET_BIT_AS_BOOL(ldThp);
    *thpLpRequest = MTD_GET_BIT_AS_BOOL(lpThp);

    return MTD_OK;    
}


MTD_STATUS mtdIs5GBTFRNegotiated
(
    IN MTD_DEV_PTR devPtr,   
    IN MTD_U16 port,
    OUT MTD_BOOL *isNegotiated,
    OUT MTD_BOOL *thpLdRequest,
    OUT MTD_BOOL *thpLpRequest
)
{
    MTD_U16 ld5g, lp5g, ldThp, lpThp;
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    if ((availableSpeeds & MTD_SPEED_5GIG_FD) == 0)
    {
        MTD_DBG_ERROR("mtdIs5GBTFRNegotiated: 5G is not supported on this device.\n");
        return MTD_FAIL;
    }

    /* assumes link is up at the speed of 5G */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,32,6,1,&ld5g));
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,33,4,1,&lp5g));
    *isNegotiated = (ld5g && lp5g);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,0x0040,2,1,&ldThp));
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,0x0041,2,1,&lpThp));
    *thpLdRequest = MTD_GET_BIT_AS_BOOL(ldThp);
    *thpLpRequest = MTD_GET_BIT_AS_BOOL(lpThp);

    return MTD_OK;    
}

MTD_STATUS mtdLP2P5GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *lp2P5GBTFRAble
)
{
    MTD_U16 lp2p5g;
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    if ((availableSpeeds & MTD_SPEED_2P5GIG_FD) == 0)
    {
        MTD_DBG_ERROR("mtdLP2P5GBTFR: 2.5G is not supported on this device.\n");
        return MTD_FAIL;
    }

    /* Assumes link is up/training has completed. Info is not available until then. */        
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,33,3,1,&lp2p5g));    
    *lp2P5GBTFRAble = MTD_GET_BIT_AS_BOOL(lp2p5g);
    
    return MTD_OK;    
}


MTD_STATUS mtdLP5GBTFR
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *lp5GBTFRAble
)
{
    MTD_U16 lp5g;
    MTD_U16 availableSpeeds;

    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &availableSpeeds));

    if ((availableSpeeds & MTD_SPEED_2P5GIG_FD) == 0)
    {
        MTD_DBG_ERROR("mtdLP2P5GBTFR: 2.5G is not supported on this device.\n");
        return MTD_FAIL;
    }

    /* Assumes link is up/training has completed. Info is not available until then. */    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,7,33,4,1,&lp5g));
    *lp5GBTFRAble = MTD_GET_BIT_AS_BOOL(lp5g);
    
    return MTD_OK;    
}


