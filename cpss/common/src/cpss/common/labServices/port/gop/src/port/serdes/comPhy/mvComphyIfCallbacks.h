/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvComphyIfCallbacks.h
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/
MCESD_STATUS mvHwsComphyRegisterWriteCallback(MCESD_DEV_PTR sDev, MCESD_U32 reg, MCESD_U32 value);
MCESD_STATUS mvHwsComphyRegisterReadCallback(MCESD_DEV_PTR sDev, MCESD_U32 reg, MCESD_U32 *value);
MCESD_STATUS mvHwsComphySetPinCallback(MCESD_DEV_PTR sDev, MCESD_U16 pin, MCESD_U16 pinValue);
MCESD_STATUS mvHwsComphyGetPinCallback(MCESD_DEV_PTR sDev, MCESD_U16 pin, MCESD_U16 *pinValue);
MCESD_STATUS mvHwsComphyWait(MCESD_DEV_PTR dev, MCESD_U32 ms);