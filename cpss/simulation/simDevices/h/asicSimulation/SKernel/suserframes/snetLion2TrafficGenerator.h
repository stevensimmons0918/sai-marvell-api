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
* @file snetLion2TrafficGenerator.h
*
* @brief This is a external API definition for Lion2 traffic generator module.
*
* @version   2
********************************************************************************
*/
#ifndef __snetLion2TrafficGeneratorh
#define __snetLion2TrafficGeneratorh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/skernel.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* enumeration that hold the types of TG modes */
typedef enum
{
    SNET_LION2_TG_MODE_INCREMENT_MAC_DA_E,
    SNET_LION2_TG_MODE_RANDOM_DATA_E,
    SNET_LION2_TG_MODE_RANDOM_LENGTH_E,
} SNET_LION2_TG_MODE_ENT;

/**
* @internal snetLion2TgPacketGenerator function
* @endinternal
*
* @brief   Packet generator main process.
*
* @param[in] tgDataPtr                - pointer to traffic generator data.
*/
GT_VOID snetLion2TgPacketGenerator
(
    IN SKERNEL_TRAFFIC_GEN_STC * tgDataPtr
);

/**
* @internal snetLion3SdmaTaskPerQueue function
* @endinternal
*
* @brief   SDMA Message Transmission task.
*
* @param[in] sdmaTxDataPtr            - Pointer to Tx SDMA data
*/
GT_VOID snetLion3SdmaTaskPerQueue
(
    SKERNEL_SDMA_TRANSMIT_DATA_STC  *sdmaTxDataPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetLion2TrafficGeneratorh */



