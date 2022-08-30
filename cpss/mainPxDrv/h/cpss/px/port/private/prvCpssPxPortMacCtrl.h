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
* @file prvCpssPxPortMacCtrl.h
*
* @brief pipe mac control
*
* @version   2
********************************************************************************
*/

#ifndef __PRV_CPSS_PX_MAC_CTRL_H
#define __PRV_CPSS_PX_MAC_CTRL_H

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/*--------------------------------------------------------*/
/* GIGA MAC                                               */
/*--------------------------------------------------------*/
#define GIGA_MAC_CTRL0_PORT_TYPE_FLD_OFFS_D      1
#define GIGA_MAC_CTRL0_PORT_TYPE_FLD_LEN_D       1

#define GIGA_MAC_CTRL1_PREAMBLE_LEN_4_D          4
#define GIGA_MAC_CTRL1_PREAMBLE_LEN_8_D          8

/* Mac Control Get 1 */
#define GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_OFFS_D   15
#define GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_LEN_D    1


/* Mac Control Get 3 */
#define GIGA_MAC_CTRL3_IPG_MIN_LEN_D             0
#define GIGA_MAC_CTRL3_IPG_MAX_LEN_D             511

#define GIGA_MAC_CTRL3_IPG_LEN_FLD_OFFS_D        6
#define GIGA_MAC_CTRL3_IPG_LEN_FLD_LEN_D         9


/*--------------------------------------------------------*/
/* XLG MAC                                                */
/*--------------------------------------------------------*/
#define XLG_MAC_CTRL5_TX_IPG_MIN_VAL_D          8
#define XLG_MAC_CTRL5_TX_IPG_MAX_VAL_D          15

#define XLG_MAC_CTRL5_TX_IPG_LEN_FLD_OFFS_D       0
#define XLG_MAC_CTRL5_TX_IPG_LEN_FLD_LEN_D        4

#define XLG_MAC_CTRL5_PREAMBLE_MIN_VAL_D          1
#define XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D          8


#define XLG_MAC_CTRL5_PREAMBLE_FLD_LEN_D          3
#define XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D  4
#define XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D   XLG_MAC_CTRL5_PREAMBLE_FLD_LEN_D
#define XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D  7
#define XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D   XLG_MAC_CTRL5_PREAMBLE_FLD_LEN_D


#define XLG_MAC_CTRL5_NUM_CRC_MIN_VAL_D           1
#define XLG_MAC_CTRL5_NUM_CRC_MAX_VAL_D           4

#define XLG_MAC_CTRL5_NUM_CRC_BYTES_FLD_LEN_D     3
#define XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D 10
#define XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D  XLG_MAC_CTRL5_NUM_CRC_BYTES_FLD_LEN_D
#define XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D 13
#define XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D  XLG_MAC_CTRL5_NUM_CRC_BYTES_FLD_LEN_D


/**
* @internal prvCpssPxPortMacIPGLengthSet function
* @endinternal
*
* @brief   set XLG mac IPG length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   = ipg  in bytes
*                                       for CG MAC -->
*                                       1-8 means IPG depend on
*                                       packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacIPGLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
);

/**
* @internal prvCpssPxPortMacIPGLengthGet function
* @endinternal
*
* @brief   get XLG mac IPG length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type
*
* @param[out] lengthPtr                = pointer to length in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssPxPortMacIPGLengthGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   portType,
    IN GT_U32                  *lengthPtr
);


/**
* @internal prvCpssPxPortMacPreambleLengthSet function
* @endinternal
*
* @brief   set XLG mac Preable length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                -  (RX/TX/both)
* @param[in] length                   = ipg  in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal prvCpssPxPortMacPreambleLengthGet function
* @endinternal
*
* @brief   get XLG mac Preable length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type
* @param[in] direction                -  (RX/TX/both)
*
* @param[out] lengthPtr                - preable ipg length in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacPreambleLengthGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   portType,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *lengthPtr
);


/**
* @internal prvCpssPxPortMacCrcModeSet function
* @endinternal
*
* @brief   Set XLG mac CRC number of bytes
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      direction - direction (RX/TX/both)
* @param[in] numCrcBytes              - number of bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacCrcModeSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                            numCrcBytes
);

/**
* @internal prvCpssPxPortMacCrcModeGet function
* @endinternal
*
* @brief   Get XLG mac CRC number of bytes
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      direction - direction (RX/TX/both)
*
* @param[out] numCrcBytesPtr           - number of bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssPxPortMacCrcModeGet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                           *numCrcBytesPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

