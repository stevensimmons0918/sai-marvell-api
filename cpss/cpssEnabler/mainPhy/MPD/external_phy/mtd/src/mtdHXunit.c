/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data defines/data for
higher-level functions that are shared by the H Unit (host/interface
to the MAC) and the X Unit (media/fiber interface) for the 
Marvell 88X32X0, 88X33X0, 88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#include <mtdFeatures.h>
#include <mtdApiTypes.h>
#include <mtdHwCntl.h>
#include <mtdAPI.h>
#include <mtdApiRegs.h>
#include <mtdHXunit.h>
#include <mtdHunit.h>
#include <mtdDiagnostics.h>
#include <mtdHwSerdesCntl.h>

#if MTD_PKG_CE_SERDES
#include <serdes/mcesd/mcesdTop.h>

#if MTD_CE_SERDES28X2
#include	<serdes/mcesd/C28GP4X2/mcesdC28GP4X2_Defs.h>
#include	<serdes/mcesd/C28GP4X2/mcesdC28GP4X2_API.h>
#endif  /* MTD_CE_SERDES28X2 */

#endif  /* MTD_PKG_CE_SERDES */

#if MTD_ORIGSERDES

/******************************************************************************
 SERDES control (common)
******************************************************************************/

#define MTD_SERDES_SHORT_REACH (1)
#define MTD_SERDES_LONG_REACH  (0)
MTD_STATUS mtdSetSerdesOverrideReach
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL reachFlag
)
{
    const MTD_U8 bitPosition = ((HorXunit == MTD_H_UNIT) ? (10):(11));

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetSerdesOverrideReach: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,bitPosition,1,MTD_GET_BOOL_AS_BIT(reachFlag)));

    return MTD_OK;    
}

MTD_STATUS mtdGetSerdesOverrideReach
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *reachFlag
)
{

    const MTD_U8 bitPosition = ((HorXunit == MTD_H_UNIT) ? (10):(11));
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetSerdesOverrideReach: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,bitPosition,1,&temp));

    MTD_CONVERT_UINT_TO_BOOL(temp, *reachFlag);

    return MTD_OK;        
}

#define MTD_SERDES_DISABLE_AUTO_INIT (1)
#define MTD_SERDES_ENABLE_AUTO_INIT  (0)
MTD_STATUS mtdSetSerdesAutoInitialization
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL autoInitFlag
)
{    
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,13,1,MTD_GET_BOOL_AS_BIT(autoInitFlag)));

    return MTD_OK;       
}

MTD_STATUS mtdGetSerdesAutoInitialization
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *autoInitFlag
)
{
    MTD_U16 temp;

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,13,1,&temp));

    MTD_CONVERT_UINT_TO_BOOL(temp, *autoInitFlag);

    return MTD_OK;    
}

MTD_STATUS mtdRerunSerdesAutoInitialization
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit
)
{
    const MTD_U16 bit12Value = ((HorXunit == MTD_H_UNIT) ? (1):(0));
    MTD_U16 temp,temp2,temp3;
    MTD_U16 waitCounter;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdRerunSerdesAutoInitialization: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_SERDES_CTRL_STATUS,&temp));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,7,13,3,&temp2));  /* override, disable, and execute bits set */
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp2,bit12Value,12,1,&temp2)); /* set if this is H or X unit */

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_SERDES_CTRL_STATUS,temp2));

    /* wait for it to be done */
    waitCounter = 0;
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_SERDES_CTRL_STATUS,&temp3));
    while((temp3 & 0x8000) && (waitCounter < 100))
    {
        MTD_ATTEMPT(mtdWait(devPtr,1));
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_SERDES_CTRL_STATUS,&temp3));
        waitCounter++;
    }

    /* put it back only override and disable whether timed out or not */    
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,13,2,temp));

    if (waitCounter >= 100)
    {
        return MTD_FAIL; /* execute timed out */
    }

    return MTD_OK;
    
}


MTD_STATUS mtdRerunSerdesAutoInitUseAutoMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_U16 temp,temp2,temp3;
    MTD_U16 waitCounter;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_SERDES_CTRL_STATUS,&temp));

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,1,15,1,&temp2));  /* execute bits and disable bits set */
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp2,1,13,1,&temp2));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,MTD_SERDES_CTRL_STATUS,temp2));

    /* wait for it to be done */
    waitCounter = 0;
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_SERDES_CTRL_STATUS,&temp3));
    while((temp3 & 0x8000) && (waitCounter < 100))
    {
        MTD_ATTEMPT(mtdWait(devPtr,1));
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,MTD_SERDES_CTRL_STATUS,&temp3));
        waitCounter++;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,13,1,temp>>13)); /* set disable bit back the way it was */

    /* if speed changed, let it stay. that's the speed that it ended up changing to/serdes was initialied to */

    if (waitCounter >= 100)
    {
        return MTD_FAIL; /* execute timed out */
    }

    return MTD_OK;
    
    
}



MTD_STATUS mtdGetSerdesAutoInitSpeed
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_U16 *autoSpeedDetected
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetSerdesAutoInitSpeed: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    if (HorXunit == MTD_H_UNIT)
    {
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,0,4,autoSpeedDetected));
    }
    else
    {
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,MTD_SERDES_CTRL_STATUS,4,4,autoSpeedDetected));
    }

    return MTD_OK;
}


MTD_STATUS mtdSetSerdesControl1
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL loopback,
    IN MTD_BOOL rx_powerdown,
    IN MTD_BOOL block_tx_on_loopback
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetSerdesControl1: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL1,12,1,MTD_GET_BOOL_AS_BIT(loopback)));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL1,8,1,MTD_GET_BOOL_AS_BIT(rx_powerdown)));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL1,6,1,MTD_GET_BOOL_AS_BIT(block_tx_on_loopback)));
    
    return MTD_OK;
}

MTD_STATUS mtdGetSerdesControl1
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *loopback,
    OUT MTD_BOOL *rx_powerdown,
    OUT MTD_BOOL *block_tx_on_loopback
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetSerdesControl1: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL1,12,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *loopback);
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL1,8,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *rx_powerdown);
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL1,6,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *block_tx_on_loopback);

    return MTD_OK;
}


MTD_STATUS mtdSetSerdesInterruptEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL fifo_overflow,
    IN MTD_BOOL fifo_underflow,
    IN MTD_BOOL pkt_check_crc
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetSerdesInterruptEnable: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_ENABLE,1,1,MTD_GET_BOOL_AS_BIT(fifo_overflow)));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_ENABLE,0,1,MTD_GET_BOOL_AS_BIT(fifo_underflow)));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_ENABLE,2,1,MTD_GET_BOOL_AS_BIT(pkt_check_crc)));
    
    return MTD_OK;
}

MTD_STATUS mtdGetSerdesInterruptEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *fifo_overflow,
    OUT MTD_BOOL *fifo_underflow,
    OUT MTD_BOOL *pkt_check_crc
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetSerdesInterruptEnable: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_ENABLE,1,1,&temp));    
    MTD_CONVERT_UINT_TO_BOOL(temp, *fifo_overflow);
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_ENABLE,0,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *fifo_underflow);
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_ENABLE,2,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp, *pkt_check_crc);
    
    return MTD_OK;
}

MTD_STATUS mtdGetSerdesInterruptStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *fifo_overflow,
    OUT MTD_BOOL *fifo_underflow,
    OUT MTD_BOOL *pkt_check_crc
)
{
    MTD_U16 temp,temp2;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetSerdesInterruptStatus: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    /* clears latch bits, so need to read register in one operation */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_FIFO_CRC_INTR_STATUS,&temp));

    mtdHwGetRegFieldFromWord(temp,1,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2, *fifo_overflow);

    mtdHwGetRegFieldFromWord(temp,0,1,&temp2);    
    MTD_CONVERT_UINT_TO_BOOL(temp2, *fifo_underflow);

    mtdHwGetRegFieldFromWord(temp,2,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2, *pkt_check_crc);
        
    return MTD_OK;
}

MTD_STATUS mtdSetGetSerdesPPMFifo
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,    
    INOUT MTD_U16 *fifo_offset
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetGetSerdesPPMFifo: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    if (*fifo_offset > MTD_SERDES_PPM_FIFO_GETONLY)
    {
        return MTD_FAIL; /* out of range */
    }

    if (MTD_IS_X32X0_BASE(devPtr->deviceId) ||
        MTD_IS_X33X0_BASE(devPtr->deviceId) ||
        MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        /* only get is allowed */
        if (*fifo_offset != MTD_SERDES_PPM_FIFO_GETONLY)
        {
            return MTD_FAIL; /* these devices cannot be changed they are forced to 11b */
        }
    }

    if (*fifo_offset < MTD_SERDES_PPM_FIFO_GETONLY)
    {
        /* update it in the register, leave input parameter the same */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CONTROL1,14,2,*fifo_offset));            
        
    }

    /* if get only, read don't change the register setting */
    /* set/get, read it back after writing it to make sure it was able to be set to desired setting */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_FIFO_CONTROL1,14,2,fifo_offset));            
    
    return MTD_OK;
}

#endif /* MTD_ORIGSERDES */

MTD_STATUS mtdSetSerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL invert_input_pol,
    IN MTD_BOOL invert_output_pol
)
{
#if MTD_PKG_CE_SERDES
        MCESD_DEV_PTR pSerdesDev;
#endif
        if (MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        if (HorXunit == MTD_X_UNIT)
        {
            MTD_DBG_ERROR("mtdSetSerdesLanePolarity: This device doesn't have X Unit(Media/Fiber Interface).\n");
            return MTD_FAIL;
        }

#if MTD_PKG_CE_SERDES
        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr,port,&pSerdesDev));

        if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
        {
#if MTD_CE_SERDES28X2
            MCESD_U8 lane;
            E_C28GP4X2_POLARITY txPolarity;
            E_C28GP4X2_POLARITY rxPolarity;

            lane = port % 2;
            txPolarity = (invert_output_pol==MTD_TRUE)?C28GP4X2_POLARITY_INVERTED:C28GP4X2_POLARITY_NORMAL;
            rxPolarity = (invert_input_pol==MTD_TRUE)?C28GP4X2_POLARITY_INVERTED:C28GP4X2_POLARITY_NORMAL;

            MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_SetTxRxPolarity(pSerdesDev,lane,txPolarity,rxPolarity));
#else
            MTD_DBG_ERROR("mtdSetSerdesLanePolarity: Serdes 28G X2 code not selected.\n");
            return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
        }
#else
        MTD_DBG_ERROR("mtdSetSerdesLanePolarity: COMPHY Serdes(28G X2) code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_PKG_CE_SERDES */
    }
    else
    {
#if MTD_ORIGSERDES
        MTD_U8 input_pol_bit, output_pol_bit;

        if (HorXunit == MTD_H_UNIT)
        {
            if (MTD_IS_X32X0_BASE(devPtr->deviceId))
            {
                input_pol_bit = 14;
                output_pol_bit = 12;
            }
            else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
            {
                input_pol_bit = 12;
                output_pol_bit = 8;
            }
            else if (MTD_IS_E21X0_BASE(devPtr->deviceId))
            {
                MTD_U16 tempVal;

                MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_SERDES_CTRL_STATUS, &tempVal));
                MTD_ATTEMPT(mtdHwSetRegFieldToWord(tempVal, MTD_GET_BOOL_AS_BIT(invert_output_pol), 8, 1, &tempVal));            
                MTD_ATTEMPT(mtdHwSetRegFieldToWord(tempVal, MTD_GET_BOOL_AS_BIT(invert_input_pol), 9, 1, &tempVal));
                MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_SERDES_CTRL_STATUS, tempVal)); /* takes effect immediately */

                return MTD_OK;
            }
            else
            {
                MTD_DBG_ERROR("mtdSetSerdesLanePolarity: Device not supported.\n");
                return MTD_FAIL;
            }
        }
        else
        {
            if (MTD_HAS_X_UNIT(devPtr))
            {        
                input_pol_bit = 15;
                output_pol_bit = 13;
            }
            else
            {
                MTD_DBG_ERROR("mtdSetSerdesLanePolarity: This device doesn't have X Unit(Media/Fiber Interface).\n");
                return MTD_FAIL;
            }
        }

        /* Handling non-E21X0 cases */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL2,input_pol_bit,1,MTD_GET_BOOL_AS_BIT(invert_input_pol)));
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL2,output_pol_bit,1,MTD_GET_BOOL_AS_BIT(invert_output_pol)));
#endif  /* MTD_ORIGSERDES */
    }

    return MTD_OK;
}

MTD_STATUS mtdGetSerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *invert_input_pol,
    OUT MTD_BOOL *invert_output_pol
)
{
#if MTD_PKG_CE_SERDES
        MCESD_DEV_PTR pSerdesDev;
#endif
    if (MTD_IS_X35X0_BASE(devPtr->deviceId))
    {
        if (HorXunit == MTD_X_UNIT)
        {
            MTD_DBG_ERROR("mtdGetSerdesLanePolarity: This device doesn't have X Unit(Media/Fiber Interface).\n");
            return MTD_FAIL;
        }

#if MTD_PKG_CE_SERDES
        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr,port,&pSerdesDev));

        if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
        {
#if MTD_CE_SERDES28X2
            MCESD_U8 lane;
            E_C28GP4X2_POLARITY txPolarity;
            E_C28GP4X2_POLARITY rxPolarity;

            lane = port % 2;

            MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_GetTxRxPolarity(pSerdesDev,lane,&txPolarity,&rxPolarity));

            *invert_input_pol = (rxPolarity==C28GP4X2_POLARITY_INVERTED)?MTD_TRUE:MTD_FALSE;
            *invert_output_pol = (txPolarity==C28GP4X2_POLARITY_INVERTED)?MTD_TRUE:MTD_FALSE;
#else
            MTD_DBG_ERROR("mtdGetSerdesLanePolarity: Serdes 28G X2 Package not selected.\n");
            return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
        }
#else
        MTD_DBG_ERROR("mtdGetSerdesLanePolarity: COMPHY Serdes(28G X2) code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_PKG_CE_SERDES */
    }
    else
    {
#if MTD_ORIGSERDES
        MTD_U8 input_pol_bit, output_pol_bit;
        MTD_U16 temp;

        if (HorXunit == MTD_H_UNIT)
        {
            if (MTD_IS_X32X0_BASE(devPtr->deviceId))
            {
                input_pol_bit = 14;
                output_pol_bit = 12;
            }
            else if (MTD_IS_X33X0_BASE(devPtr->deviceId))
            {
                input_pol_bit = 12;
                output_pol_bit = 8;
            }
            else if (MTD_IS_E21X0_BASE(devPtr->deviceId))
            {
                MTD_U16 tempVal;

                MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_SERDES_CTRL_STATUS, &tempVal));
                MTD_ATTEMPT(mtdHwGetRegFieldFromWord(tempVal, 8, 1, &temp));
                MTD_CONVERT_UINT_TO_BOOL(temp, *invert_output_pol);
                MTD_ATTEMPT(mtdHwGetRegFieldFromWord(tempVal, 9, 1, &temp));
                MTD_CONVERT_UINT_TO_BOOL(temp, *invert_input_pol);
                
                return MTD_OK;    
            }
            else
            {
                MTD_DBG_ERROR("mtdSetSerdesLanePolarity: Device not supported.\n");
                return MTD_FAIL;
            }
        }
        else
        {
            if (MTD_HAS_X_UNIT(devPtr))
            {
                input_pol_bit = 15;
                output_pol_bit = 13;
            }
            else
            {
                MTD_DBG_ERROR("mtdGetSerdesLanePolarity: This device doesn't have X Unit(Media/Fiber Interface).\n");
                return MTD_FAIL;
            }
        }

        /* Handling non-E21X0 devices */
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL2,input_pol_bit,1,&temp));
        MTD_CONVERT_UINT_TO_BOOL(temp, *invert_input_pol);
        MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_SERDES_CONTROL2,output_pol_bit,1,&temp));        
        MTD_CONVERT_UINT_TO_BOOL(temp, *invert_output_pol);
#endif  /* MTD_ORIGSERDES */
    }

    return MTD_OK;
}


MTD_STATUS mtdConfigurePktGeneratorChecker
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL readToClear,
    IN MTD_BOOL dontuseSFDinChecker,
    IN MTD_U16 pktPatternControl,
    IN MTD_BOOL generateCRCoff,
    IN MTD_U32 initialPayload,
    IN MTD_U16 frameLengthControl,
    IN MTD_U16 numPktsToSend,
    IN MTD_BOOL randomIPG,
    IN MTD_U16 ipgDuration
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdConfigurePktGeneratorChecker: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    if (pktPatternControl > MTD_PKT_RANDOM_WORD || pktPatternControl == 1)
    {
        return MTD_FAIL;
    }

    if (frameLengthControl == 6 || frameLengthControl == 7)
    {
        return MTD_FAIL;
    }
    
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL1,15,1,MTD_GET_BOOL_AS_BIT(readToClear)));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL1,2,1,MTD_GET_BOOL_AS_BIT(dontuseSFDinChecker)));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL2,4,4,pktPatternControl));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL2,3,1,MTD_GET_BOOL_AS_BIT(generateCRCoff)));

    /* load up initial payload */
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PKT_GEN_PAYLOAD1,(MTD_U16)initialPayload));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PKT_GEN_PAYLOAD2,(MTD_U16)(initialPayload>>16)));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PKT_GEN_LENGTH,frameLengthControl));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PKT_GEN_BURSTSEQ,numPktsToSend));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_IPG,15,1,MTD_GET_BOOL_AS_BIT(randomIPG)));
    
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_IPG,0,15,ipgDuration));
    
    return MTD_OK;
}

MTD_STATUS mtdGetPktGeneratorCheckerConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXUnit,
    OUT MTD_BOOL *readToClear,
    OUT MTD_BOOL *dontuseSFDinChecker,
    OUT MTD_U16 *pktPatternControl,
    OUT MTD_BOOL *generateCRCoff,
    OUT MTD_U32 *initialPayload,
    OUT MTD_U16 *frameLengthControl,
    OUT MTD_U16 *numPktsToSend,
    OUT MTD_BOOL *randomIPG,
    OUT MTD_U16 *ipgDuration
)
{

    MTD_U16 temp;

    if (HorXUnit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetPktGeneratorCheckerConfig: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXUnit,MTD_PKT_GEN_CONTROL1,15,1,&temp));
    *readToClear = MTD_GET_BIT_AS_BOOL(temp);
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXUnit,MTD_PKT_GEN_CONTROL1,2,1,&temp));
    *dontuseSFDinChecker = MTD_GET_BIT_AS_BOOL(temp);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXUnit,MTD_PKT_GEN_CONTROL2,4,4,pktPatternControl));
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXUnit,MTD_PKT_GEN_CONTROL2,3,1,&temp));
    *generateCRCoff = MTD_GET_BIT_AS_BOOL(temp);

    /* lower byte */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXUnit,MTD_PKT_GEN_PAYLOAD1,&temp));
    *initialPayload = (MTD_U16)temp;
    /* upper byte */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXUnit,MTD_PKT_GEN_PAYLOAD2,&temp));
    *initialPayload |= (MTD_U32)temp<<16;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXUnit,MTD_PKT_GEN_LENGTH,frameLengthControl));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXUnit,MTD_PKT_GEN_BURSTSEQ,numPktsToSend));

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXUnit,MTD_PKT_GEN_IPG,15,1,&temp));
    *randomIPG = MTD_GET_BIT_AS_BOOL(temp);
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXUnit,MTD_PKT_GEN_IPG,0,15,ipgDuration));

    return MTD_OK;
}

MTD_STATUS mtdEnablePktGeneratorChecker
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL enableGenerator,
    IN MTD_BOOL enableChecker
)
{
    MTD_U16 temp = 0,numPacket = 0;
    MTD_U16 currGeneratorStatus = MTD_FALSE, currCheckerStatus = MTD_FALSE;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdEnablePktGeneratorChecker: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, HorXunit, MTD_PKT_GEN_CONTROL1, &temp));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp, 1, 1, &currGeneratorStatus));
    MTD_ATTEMPT(mtdHwGetRegFieldFromWord(temp, 0, 1, &currCheckerStatus));

    /* better to start/stop at the same time if possible, use a single write to limit delays between them */
    
    temp = ((MTD_GET_BOOL_AS_BIT(enableGenerator))<<1) | (MTD_GET_BOOL_AS_BIT(enableChecker));

    if (currGeneratorStatus != enableGenerator)
    {
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_BURSTSEQ,&numPacket));
        /* need to write X.F017 = 0 to insure no fragments are sent when enabling/disabling the generator */
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PKT_GEN_BURSTSEQ,0x0000));
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL1,0,2,temp));

    if (currGeneratorStatus != enableGenerator)
    {
        /* restore X.F017 to the configured value */
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PKT_GEN_BURSTSEQ,numPacket));
    }

    return MTD_OK;    
}

MTD_STATUS mtdStartStopPktGeneratorTraffic
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXUnit,
    IN MTD_U16 numPktsToSend
)
{
    if (HorXUnit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetPktGeneratorCheckerConfig: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXUnit,MTD_PKT_GEN_BURSTSEQ,numPktsToSend));

    return MTD_OK;
}

MTD_STATUS mtdPktGeneratorCounterReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit    
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdPktGeneratorCounterReset: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL1,6,1,1));   
    /* assumes delay is long enough to clear counters, this makes an assumption about the speed */
    /* of the MDIO as being relatively slow compared to the number of cycles it takes to clear the */
    /* clear the counters, relatively fast. may need a delay here for F2R, or really fast MDIO */
    /* speeds */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PKT_GEN_CONTROL1,6,1,0));    

    return MTD_OK;    
}

MTD_STATUS mtdPktGeneratorGetCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_U16 whichCounter,
    OUT MTD_U64 *packetCount,
    OUT MTD_U64 *byteCount    
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdPktGeneratorGetCounter: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    *packetCount = *byteCount = 0;
    
    switch (whichCounter)
    {
        case MTD_PKT_GET_TX:
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_TXPKTCTR1,&temp));
            *packetCount = temp;
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_TXPKTCTR2,&temp));
            *packetCount |= (((MTD_U64)(temp))<<16);
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_TXPKTCTR3,&temp));
            *packetCount |= (((MTD_U64)(temp))<<32);

            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_TXBYTCTR1,&temp));
            *byteCount = temp;
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_TXBYTCTR2,&temp));
            *byteCount |= (((MTD_U64)(temp))<<16);
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_TXBYTCTR3,&temp));
            *byteCount |= (((MTD_U64)(temp))<<32);            
            break;

        case MTD_PKT_GET_RX:
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_RXPKTCTR1,&temp));
            *packetCount = temp;
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_RXPKTCTR2,&temp));
            *packetCount |= (((MTD_U64)(temp))<<16);
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_RXPKTCTR3,&temp));
            *packetCount |= (((MTD_U64)(temp))<<32);

            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_RXBYTCTR1,&temp));
            *byteCount = temp;
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_RXBYTCTR2,&temp));
            *byteCount |= (((MTD_U64)(temp))<<16);
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_RXBYTCTR3,&temp));
            *byteCount |= (((MTD_U64)(temp))<<32);            
            break;

        case MTD_PKT_GET_ERR:
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_ERRPKTCTR1,&temp));
            *packetCount = temp;
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_ERRPKTCTR2,&temp));
            *packetCount |= (((MTD_U64)(temp))<<16);
            MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PKT_GEN_ERRPKTCTR3,&temp));
            *packetCount |= (((MTD_U64)(temp))<<32);            
            break;

        default:
            return MTD_FAIL;
            break;            
    }
    
    return MTD_OK;
}

MTD_STATUS mtdPktGeneratorCheckerGetLinkDrop
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_U16 *linkDropCounter        
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdPktGeneratorCheckerGetLinkDrop: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_LINK_DROP_COUNTER,linkDropCounter));
    
    return MTD_OK;
}


/***************************** PRBS Control/Status/Counters *******************/

/* Bit Positions for PRBS Control/Status */
#define PRBS_READ_CLEAR_BIT 13
#define PRBS_LOCKED_BIT      8
#define PRBS_WAIT_LOCK_BIT   7
#define PRBS_CTR_RESET_BIT   6
#define PRBS_TX_EN_BIT       5
#define PRBS_RX_EN_BIT       4
#define PRBS_PATTERN_SEL_BIT 0 /* 4-bit field */

/*******************************************************************************
 MTD_STATUS mtdSetPRBSPattern
    Selects the PRBS pattern
*******************************************************************************/
MTD_STATUS mtdSetPRBSPattern
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,
    IN MTD_U16  HorXunit,
    IN MTD_U16  pattSel
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetPRBSPattern: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_PATTERN_SEL_BIT,4,(MTD_U16)pattSel));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdSetPRBSEnableTxRx
    Controls the transmit and receive
*******************************************************************************/
MTD_STATUS mtdSetPRBSEnableTxRx
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,    
    IN MTD_U16  HorXunit,
    IN MTD_U16  txEnable,
    IN MTD_U16  rxEnable
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetPRBSEnableTxRx: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {        
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,0xF097,&temp));
    }
    else
    {        
        MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,0xF074,&temp));
    }

    if (rxEnable == MTD_ENABLE)
    {
        if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,0x2,12,2,&temp));
        }
        else
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,0x0,13,1,&temp));
        }
    }
    else
    {
        if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,0x1,12,2,&temp));
        }
        else
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,0x1,13,1,&temp));
            
        }
    }

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,0xF097,temp));
    }
    else
    {
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,0xF074,temp));
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_CONTROL,&temp));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,txEnable,PRBS_TX_EN_BIT,1,&temp));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,rxEnable,PRBS_RX_EN_BIT,1,&temp));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_PRBS_CONTROL,temp));
        
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdPRBSCounterReset
    Resets the PRBS counter when it's been set to manual reset and not clear-on-
    read
*******************************************************************************/
MTD_STATUS mtdPRBSCounterReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,    
    IN MTD_U16  HorXunit
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdPRBSCounterReset: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_CTR_RESET_BIT,1,(MTD_U16)1));
        
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdSetPRBSWaitForLock
    Makes the receiver start counting right away, even before it's locked 
    on the pattern
*******************************************************************************/
MTD_STATUS mtdSetPRBSWaitForLock
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,    
    IN MTD_U16  HorXunit,
    IN MTD_U16  disableWaitforLock
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetPRBSWaitForLock: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_WAIT_LOCK_BIT,1,disableWaitforLock));
        
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetPRBSWaitForLock
    Gets the value of the control bit that sets whether the counters start
    right away, or wait until the receiver is locked.
*******************************************************************************/
MTD_STATUS mtdGetPRBSWaitForLock
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,    
    IN MTD_U16  HorXunit,
    OUT MTD_U16 *disableWaitforLock
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetPRBSWaitForLock: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_WAIT_LOCK_BIT,1,disableWaitforLock));
        
    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdSetPRBSClearOnRead
    Sets the bit that controls whether the counters clear when read or
    clear manually by setting a bit.
*******************************************************************************/
MTD_STATUS mtdSetPRBSClearOnRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,    
    IN MTD_U16  HorXunit,
    IN MTD_U16  enableReadClear
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSetPRBSClearOnRead: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_READ_CLEAR_BIT,1,enableReadClear));
        
    return MTD_OK;
}


/*******************************************************************************
 MTD_STATUS mtdGetPRBSClearOnRead
    Reads the bit setting that controls whether the counters clear when read or
    clear manually by setting a bit.
*******************************************************************************/
MTD_STATUS mtdGetPRBSClearOnRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16   port,    
    IN MTD_U16   HorXunit,
    OUT MTD_U16 *enableReadClear
)
{
    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetPRBSClearOnRead: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_READ_CLEAR_BIT,1,enableReadClear));
        
    return MTD_OK;
}


/*******************************************************************************
 MTD_STATUS mtdGetPRBSLocked
    Reads the status bit that indicates if the receiver is locked onto the
    PRBS pattern or not.
*******************************************************************************/
MTD_STATUS mtdGetPRBSLocked
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16    port,
    IN MTD_U16    HorXunit,
    OUT MTD_BOOL *prbsLocked
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetPRBSLocked: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_PRBS_CONTROL,PRBS_LOCKED_BIT,1,&temp));\
    *prbsLocked = MTD_GET_BIT_AS_BOOL(temp);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetPRBSCounts
    Reads the counters. Will clear the counters if they've been setup to
    clear-on-read.
*******************************************************************************/
MTD_STATUS mtdGetPRBSCounts
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16   port,    
    IN MTD_U16   HorXunit,
    OUT MTD_U64 *txBitCount,
    OUT MTD_U64 *rxBitCount,
    OUT MTD_U64 *rxBitErrorCount
)
{
    MTD_U16 lowTxWord, lowRxWord, lowErrWord,
            midWord, hiWord;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGetPRBSCounts: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    /* initialize values, in case of error return 0 */
    *txBitCount = *rxBitCount = *rxBitErrorCount = 0;

    /* must read low words first to latch the upper two words */

    /* read tx count lower/mid/upper */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_TXCTR1,&lowTxWord));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_TXCTR2,&midWord));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_TXCTR3,&hiWord));
    *txBitCount = lowTxWord + (((MTD_U64)(midWord))<<16) + (((MTD_U64)(hiWord))<<32);

    /* read rx count lower/mid/upper */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_RXCTR1,&lowRxWord));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_RXCTR2,&midWord));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_RXCTR3,&hiWord));
    *rxBitCount = lowRxWord + (((MTD_U64)(midWord))<<16) + (((MTD_U64)(hiWord))<<32);

    /* read err count lower/mid/upper */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_ERRCTR1,&lowErrWord));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_ERRCTR2,&midWord));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_PRBS_SYM_ERRCTR3,&hiWord));
    *rxBitErrorCount = lowErrWord + (((MTD_U64)(midWord))<<16) + (((MTD_U64)(hiWord))<<32);

    return MTD_OK;
}

/******************************************************************************
 Functions shared between H unit and X unit for 1000BX/SGMII
******************************************************************************/

MTD_STATUS mtdSet1000BXSGMIIControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL loopback,
    IN MTD_U16 speed,
    IN MTD_BOOL an_enable,
    IN MTD_BOOL power_down,
    IN MTD_BOOL restart_an,
    IN MTD_BOOL sw_reset
)
{       
    MTD_U16 bit6, bit13, temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSet1000BXSGMIIControl: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    if (speed > MTD_SGMII_SPEED_ASIS)
    {
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,&temp));
  
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(loopback),14,1,&temp));

    if (speed != MTD_SGMII_SPEED_ASIS)
    {
        switch (speed)
        {
            case MTD_SGMII_SPEED_10M:
                bit6 = bit13 = 0;
                break;

            case MTD_SGMII_SPEED_100M:
                bit6 = 0;
                bit13 = 1;                
                break;

            case MTD_SGMII_SPEED_1G:
                bit6 = 1;
                bit13 = 0;
                break;

            default:
                return MTD_FAIL;
                break;
        }

        MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,bit6,6,1,&temp));
        MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,bit13,13,1,&temp));
        
    }

      
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(an_enable),12,1,&temp));  
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(power_down),11,1,&temp));
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(restart_an),9,1,&temp)); 
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(sw_reset),15,1,&temp));

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,temp));
    
    return MTD_OK;    
}

MTD_STATUS mtdGet1000BXSGMIIControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *loopback,
    OUT MTD_U16 *speed,
    OUT MTD_BOOL *an_enable,
    OUT MTD_BOOL *power_down
)
{
    MTD_U16 temp,bit6,bit13; 

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet1000BXSGMIIControl: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,14,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp,*loopback);
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,6,1,&bit6));   
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,13,1,&bit13));
    *speed = (bit6<<1) | bit13;

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,12,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp,*an_enable);
    
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_1000X_SGMII_CONTROL,11,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp,*power_down);
    
    return MTD_OK;
}

MTD_STATUS mtdGet1000BXSGMIIStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *an_complete,
    OUT MTD_BOOL *remote_fault,
    OUT MTD_BOOL *link_status_latched,
    OUT MTD_BOOL *link_status_current
)
{
    MTD_U16 temp,temp2;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet1000BXSGMIIStatus: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    /* read once to get all bits including latched low link status (may clear LL link status) */
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_1000X_SGMII_STATUS,&temp));

    mtdHwGetRegFieldFromWord(temp,5,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2,*an_complete);

    mtdHwGetRegFieldFromWord(temp,4,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2,*remote_fault);

    mtdHwGetRegFieldFromWord(temp,2,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2,*link_status_latched);

    /* read it again to get current link status */
    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr,port,HorXunit,MTD_1000X_SGMII_STATUS,2,1,&temp));
    MTD_CONVERT_UINT_TO_BOOL(temp,*link_status_current);
    
    return MTD_OK;
}

/******************************************************************************
 Functions shared between H unit and X unit for 10GBASE-R PCS
 XFI
 SFI
 USXGMII
 All use H unit or X unit 10GBASE-R PCS 
******************************************************************************/

#define MTD_USXGMII_AN_CNTL MTD_T_UNIT_PMA_PMD, 0xC049 /* disable/enable AN for USXGMII Mac Type*/
MTD_STATUS mtdUSXGMIIAutoNeg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableAutoNeg,
    IN MTD_BOOL do10GBRSwReset
)
{
    MTD_BOOL loopback;
    MTD_BOOL power_down;
    MTD_BOOL sw_reset = MTD_TRUE;

    if (MTD_IS_X33X0_BASE(devPtr->deviceId) || MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        /* Control register logic: 1 for disable, 0 for enable */
        if (enableAutoNeg == MTD_DISABLE)
        {
            /* Set bit to disable the AutoNeg */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_USXGMII_AN_CNTL,0,1,0x1));
        }
        else if (enableAutoNeg == MTD_ENABLE)
        {
            /* Clear bit to enable the AutoNeg */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_USXGMII_AN_CNTL,0,1,0x0));
        }
        else
        {
            MTD_DBG_ERROR("mtdUSXGMIIAutoNeg: Incorrect Auto-Neg Option\n");
        }

        if (do10GBRSwReset == MTD_TRUE)
        {
            MTD_ATTEMPT(mtdGetUSXGMIIControl(devPtr,port,MTD_H_UNIT,&loopback,&power_down));
            MTD_ATTEMPT(mtdSetUSXGMIIControl(devPtr,port,MTD_H_UNIT,loopback,power_down,sw_reset));
        }
    }
    else
    {
        if (enableAutoNeg == MTD_DISABLE)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF0A0,2,2,0x2));
        }
        else if (enableAutoNeg == MTD_ENABLE)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF0A0,2,2,0x3));
        }
        else
        {
            MTD_DBG_ERROR("mtdUSXGMIIAutoNeg: Incorrect Auto-Neg Option\n");
        }
    }
    
    return MTD_OK;
}

/* can also call with mtdSetUSXGMIIControl() */
MTD_STATUS mtdSet10GBRControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    IN MTD_BOOL loopback,
    IN MTD_BOOL power_down,
    IN MTD_BOOL sw_reset
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdSet10GBRControl/mtdSetUSXGMIIControl: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_CONTROL,&temp));    

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(loopback),14,1,&temp)); 
    
    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(power_down),11,1,&temp)); 

    MTD_ATTEMPT(mtdHwSetRegFieldToWord(temp,MTD_GET_BOOL_AS_BIT(sw_reset),15,1,&temp)); 

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr,port,HorXunit,MTD_10GBR_PCS_CONTROL,temp));

    if (MTD_IS_E21X0_BASE(devPtr->deviceId))
    {
        /* Needs workaround for loopback to work in E21X0 devices */
        if (loopback)
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF07E,13,3,5)); /* force signal lock */ 
        }
        else
        {
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,4,0xF07E,13,3,1)); /* clear overwrite bit */
        }
    }
    
    return MTD_OK;
}

/* can also call with mtdGetUSXGMIIControl() */
MTD_STATUS mtdGet10GBRControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *loopback,
    OUT MTD_BOOL *power_down
)
{
    MTD_U16 lb,pd,temp;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet10GBRControl/mtdGetUSXGMIIControl: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_CONTROL,&temp));
    mtdHwGetRegFieldFromWord(temp,14,1,&lb);
    mtdHwGetRegFieldFromWord(temp,11,1,&pd);
    MTD_CONVERT_UINT_TO_BOOL(lb, *loopback);
    MTD_CONVERT_UINT_TO_BOOL(pd, *power_down);
        
    return MTD_OK;
}

/* can also call with mtdGetUSXGMIIStatus1() */
MTD_STATUS mtdGet10GBRStatus1
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,
    OUT MTD_BOOL *tx_lpi_latch,
    OUT MTD_BOOL *rx_lpi_latch,
    OUT MTD_BOOL *tx_lpi_current,
    OUT MTD_BOOL *rx_lpi_current,
    OUT MTD_BOOL *fault,
    OUT MTD_BOOL *link_status_latch,
    OUT MTD_BOOL *link_status_current    
)
{
    MTD_U16 temp1,temp2,temp3;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet10GBRStatus1/mtdGetUSXGMIIStatus1: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_STATUS1,&temp1)); /* latched value */

    mtdHwGetRegFieldFromWord(temp1,11,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *tx_lpi_latch);
    mtdHwGetRegFieldFromWord(temp1,10,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *rx_lpi_latch);
    mtdHwGetRegFieldFromWord(temp1,2,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *link_status_latch);
            
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_STATUS1,&temp2)); /* current value */

    mtdHwGetRegFieldFromWord(temp2,9,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *tx_lpi_current);
    mtdHwGetRegFieldFromWord(temp2,8,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *rx_lpi_current);
    mtdHwGetRegFieldFromWord(temp2,7,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *fault);
    mtdHwGetRegFieldFromWord(temp2,2,1,&temp3);
    MTD_CONVERT_UINT_TO_BOOL(temp3, *link_status_current);
        
    return MTD_OK;
}

/* can also call with mtdGetUSXGMIIFault() */
MTD_STATUS mtdGet10GBRFault
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit,    
    OUT MTD_BOOL *tx_fault_latch,
    OUT MTD_BOOL *rx_fault_latch,
    OUT MTD_BOOL *tx_fault_current,
    OUT MTD_BOOL *rx_fault_current
)
{
    MTD_U16 temp1, fault;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet10GBRFault/mtdGetUSXGMIIFault: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_FAULT,&temp1)); /* latched value */
    mtdHwGetRegFieldFromWord(temp1,11,1,&fault);
    MTD_CONVERT_UINT_TO_BOOL(fault, *tx_fault_latch);
    mtdHwGetRegFieldFromWord(temp1,10,1,&fault);
    MTD_CONVERT_UINT_TO_BOOL(fault, *rx_fault_latch);

    
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_FAULT,&temp1)); /* current value */
    mtdHwGetRegFieldFromWord(temp1,11,1,&fault);
    MTD_CONVERT_UINT_TO_BOOL(fault, *tx_fault_current);
    mtdHwGetRegFieldFromWord(temp1,10,1,&fault);
    MTD_CONVERT_UINT_TO_BOOL(fault, *rx_fault_current);

    return MTD_OK;
}

/* can also call with mtdGetUSXGMIIReceiveStatus() */
MTD_STATUS mtdGet10GBRReceiveStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit, 
    OUT MTD_BOOL *link_up,
    OUT MTD_BOOL *high_ber,
    OUT MTD_BOOL *block_lock    
)
{
    MTD_U16 temp1, temp2;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet10GBRReceiveStatus/mtdGetUSXGMIIReceiveStatus: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_RXSTATUS,&temp1)); /* current value */
    mtdHwGetRegFieldFromWord(temp1,12,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2, *link_up);
    mtdHwGetRegFieldFromWord(temp1,1,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2, *high_ber);
    mtdHwGetRegFieldFromWord(temp1,0,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2, *block_lock);
    
    
    return MTD_OK;
}

/* can also call with mtdGetUSXGMIIStatus2() */
MTD_STATUS mtdGet10GBRStatus2
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXunit, 
    OUT MTD_BOOL *has_block_lock,
    OUT MTD_BOOL *reported_high_ber,
    OUT MTD_U8 *ber_counter,
    OUT MTD_U8 *errored_blocks_counter    
)
{
    MTD_U16 temp1, temp2;

    if (HorXunit == MTD_X_UNIT && !MTD_HAS_X_UNIT(devPtr))
    {
        MTD_DBG_ERROR("mtdGet10GBRStatus2/mtdGetUSXGMIIStatus2: This device doesn't have X Unit(Media/Fiber Interface).\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr,port,HorXunit,MTD_10GBR_PCS_STATUS2,&temp1)); /* latch value, clears latch/counters */
    mtdHwGetRegFieldFromWord(temp1,15,1,&temp2);

    MTD_CONVERT_UINT_TO_BOOL(temp2, *has_block_lock);

    mtdHwGetRegFieldFromWord(temp1,14,1,&temp2);
    MTD_CONVERT_UINT_TO_BOOL(temp2, *reported_high_ber);

    mtdHwGetRegFieldFromWord(temp1,8,6,&temp2);
    *ber_counter = (MTD_U8)temp2;
    
    mtdHwGetRegFieldFromWord(temp1,0,8,&temp2);
    *errored_blocks_counter = (MTD_U8)temp2;
    
    return MTD_OK;
}

MTD_STATUS mtdSetHXunitMacLoopback
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,
    IN MTD_U16  HorXunit,
    IN MTD_BOOL passThrough,
    IN MTD_BOOL enable
)
{
    if (HorXunit == MTD_H_UNIT)
    {
        /* Shallow MAC Loopback */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL1,12,1,(MTD_U16)enable));
    }
    else /* X-unit */
    {
        if (MTD_HAS_X_UNIT(devPtr))
        {
            /* Deep MAC Loopback */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_X_UNIT,MTD_10GBR_PCS_CONTROL,14,1,(MTD_U16)enable));
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_X_UNIT,MTD_SERDES_CONTROL1,6,1,(MTD_U16)(!passThrough)));
        } 
        else
        {
            MTD_DBG_ERROR("mtdSetHXunitMacLoopback: This device doesn't have X Unit(Media/Fiber Interface).\n");
            return MTD_FAIL;
        }
    }

    return MTD_OK;
}

MTD_STATUS mtdSetHXunitLineLoopback
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,
    IN MTD_U16  HorXunit,
    IN MTD_BOOL passThrough,
    IN MTD_BOOL enable
)
{
    MTD_U16 temp;

    if (HorXunit == MTD_H_UNIT)
    {
        /* Deep Line Loopback */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_10GBR_PCS_CONTROL,14,1,(MTD_U16)enable));
        if (MTD_IS_X35X0_BASE(devPtr->deviceId))
        {
            temp = (enable == MTD_TRUE) ? 0x3 : 0;
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_RS_FEC_CONTROL1,0,2,temp));
        }

        /* Needs workaround for loopback to work in E21X0 devices */
        if (MTD_IS_E21X0_BASE(devPtr->deviceId))
        {
            if (enable)
            {
                MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,0xF07E,13,3,5)); /* force signal lock */ 
            }
            else
            {
                MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,0xF07E,13,3,1)); /* clear overwrite bit */
            }
        }

        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL1,6,1,(MTD_U16)(!passThrough)));
    }
    else  /* X-unit */
    {
        if (MTD_HAS_X_UNIT(devPtr))
        {
            /* Shallow Line Loopback */
            MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_X_UNIT,MTD_SERDES_CONTROL1,12,1,(MTD_U16)enable));
        }
        else
        {
            MTD_DBG_ERROR("mtdSetHXunitLineLoopback: This device doesn't have X Unit(Media/Fiber Interface).\n");
            return MTD_FAIL;
        }
    }
  
    return MTD_OK;
}

MTD_STATUS mtdSetShallowMacLoopbackLinkDown
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16  port,
    IN MTD_U16  linkDownSpeed,
    IN MTD_BOOL enable
)
{
    MTD_U16 macType;
    MTD_BOOL macIfPowerDown;
    MTD_U16 macIfSnoopSel;
    MTD_U16 macIfActiveLaneSelect;
    MTD_U16 macLinkDownSpeed;
    MTD_U16 macMaxIfSpeed;
    MTD_U16 speedsAvailable;

    /* find out what speeds this device supports */
    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr,port,&speedsAvailable));

    /* Get Host MAC interface control parameters */
    MTD_ATTEMPT(mtdGetMacInterfaceControl(devPtr,port,&macType, &macIfPowerDown, &macIfSnoopSel, \
            &macIfActiveLaneSelect, &macLinkDownSpeed, &macMaxIfSpeed));

    /* Set host link down speed */
    switch (linkDownSpeed)
    {
        case MTD_SPEED_10GIG_FD:
            if (!(speedsAvailable & MTD_SPEED_10GIG_FD))
            {
                MTD_DBG_ERROR("mtdSetShallowMacLoopbackLinkDown: Invalid link down speed!\n");
                return MTD_FAIL;
            }

            macLinkDownSpeed = MTD_MAC_SPEED_10_GBPS;
            macMaxIfSpeed = MTD_MAX_MAC_SPEED_10G;
            break;

        case MTD_SPEED_5GIG_FD:
            if (!(speedsAvailable & MTD_SPEED_5GIG_FD))
            {
                MTD_DBG_ERROR("mtdSetShallowMacLoopbackLinkDown: Invalid link down speed!\n");
                return MTD_FAIL;
            }

            macLinkDownSpeed = MTD_MAC_SPEED_10_GBPS;
            macMaxIfSpeed = MTD_MAX_MAC_SPEED_5G;
            break;

        case MTD_SPEED_2P5GIG_FD:
            if (!(speedsAvailable & MTD_SPEED_2P5GIG_FD))
            {
                MTD_DBG_ERROR("mtdSetShallowMacLoopbackLinkDown: Invalid link down speed!\n");
                return MTD_FAIL;
            }

            macLinkDownSpeed = MTD_MAC_SPEED_10_GBPS;
            macMaxIfSpeed = MTD_MAX_MAC_SPEED_2P5G;
            break;

        case MTD_SPEED_1GIG_FD:
        case MTD_SPEED_1GIG_HD:            
            macLinkDownSpeed = MTD_MAC_SPEED_1000_MBPS;
            macMaxIfSpeed = MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED;
            break;

        case MTD_SPEED_100M_FD:
        case MTD_SPEED_100M_HD:
            macLinkDownSpeed = MTD_MAC_SPEED_100_MBPS;
            macMaxIfSpeed = MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED;
            break;

        case MTD_SPEED_10M_FD:
        case MTD_SPEED_10M_HD:
            macLinkDownSpeed = MTD_MAC_SPEED_10_MBPS;
            macMaxIfSpeed = MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED;
            break;

        default:
            MTD_DBG_ERROR("mtdSetShallowMacLoopbackLinkDown: Invalid link down speed!\n");
            return MTD_FAIL;
            break;
    }

    /* overwrite for PHYs */
    if (MTD_IS_X32X0_BASE(devPtr->deviceId))
    {
        macMaxIfSpeed = MTD_MAX_MAC_SPEED_NOT_APPLICABLE;
    }
    else if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        macMaxIfSpeed = MTD_MAX_MAC_SPEED_NOT_APPLICABLE;
        macLinkDownSpeed = MTD_MAX_MAC_SPEED_NOT_APPLICABLE;
    }

    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr,port,macType, macIfPowerDown, macIfSnoopSel, \
            macIfActiveLaneSelect, macLinkDownSpeed, macMaxIfSpeed,MTD_TRUE,MTD_TRUE));

    /* Shallow MAC Loopback */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL1,12,1,(MTD_U16)enable));
  
    return MTD_OK;
}
