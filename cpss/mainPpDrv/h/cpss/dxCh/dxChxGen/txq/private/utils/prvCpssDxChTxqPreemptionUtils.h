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
* @file prvCpssDxChTxqPreemptionUtils.h
*
* @brief CPSS SIP6 TXQ preemtion feature implementation layer
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqPreemptionUtils
#define __prvCpssDxChTxqPreemptionUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define  PRV_CPSS_DXCH_CURRENT_PORT_SPEED_IN_MB_MAC(_devNum,_physicalPortNum,_speedInMb) \
    do \
    {\
      GT_U32 portMacNum;\
      PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_devNum, _physicalPortNum, portMacNum);\
      _speedInMb = prvCpssCommonPortSpeedEnumToMbPerSecConvert(PRV_CPSS_DXCH_PORT_SPEED_MAC(_devNum,portMacNum));\
    }\
    while(0);


#define  PRV_CPSS_SIP_6_10_DEFAULT_PREEMPTION_ALLOWED_MAC GT_TRUE

/*macro define preemptive pairs for AC5P*/
#define TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC 0
#define TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC 8
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_0_NUM_MAC 0, 5
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_1_NUM_MAC 1, 9
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_2_NUM_MAC 2, 3
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_3_NUM_MAC 6, 7
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_4_NUM_MAC 10, 11
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_5_NUM_MAC 14, 15
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_6_NUM_MAC 18, 19
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_7_NUM_MAC 22, 23

/*macro define preemptive pairs for Harrier*/
#define TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC
#define TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC 8
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_0_NUM_MAC 0, 1
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_1_NUM_MAC 2, 3
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_2_NUM_MAC 4, 5
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_3_NUM_MAC 6, 7
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_4_NUM_MAC 8, 9
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_5_NUM_MAC 10, 11
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_6_NUM_MAC 12, 13
#define TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_7_NUM_MAC 14, 15

/*macro define preemptive pairs for IronmanL */
#define TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC \
    (TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC + TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC)
#define TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC 28
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_00_NUM_MAC  0, 24
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_01_NUM_MAC  1, 25
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_02_NUM_MAC  2, 26
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_03_NUM_MAC  3, 27
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_04_NUM_MAC  4, 28
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_05_NUM_MAC  5, 29
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_06_NUM_MAC  6, 30
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_07_NUM_MAC  7, 31
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_08_NUM_MAC  8, 32
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_09_NUM_MAC  9, 33
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_10_NUM_MAC 10, 34
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_11_NUM_MAC 11, 35
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_12_NUM_MAC 12, 36
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_13_NUM_MAC 13, 37
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_14_NUM_MAC 14, 38
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_15_NUM_MAC 15, 39
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_16_NUM_MAC 16, 40
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_17_NUM_MAC 17, 41
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_18_NUM_MAC 18, 42
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_19_NUM_MAC 19, 43
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_20_NUM_MAC 20, 44
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_21_NUM_MAC 21, 45
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_22_NUM_MAC 22, 46
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_23_NUM_MAC 23, 47
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_24_NUM_MAC 48, 52
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_25_NUM_MAC 49, 53
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_26_NUM_MAC 50, 54
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_27_NUM_MAC 51, 55

#define TXQ_AC5P_DP_PREEMPTIVE_COUPLES_MAC TXQ_AC5P_DP_PREEMPTIVE_COUPLE_0_NUM_MAC, \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_1_NUM_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_2_NUM_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_3_NUM_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_4_NUM_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_5_NUM_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_6_NUM_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_7_NUM_MAC



#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLES_MAC TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_0_NUM_MAC, \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_1_NUM_MAC,                    \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_2_NUM_MAC,                    \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_3_NUM_MAC,                    \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_4_NUM_MAC,                    \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_5_NUM_MAC,                    \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_6_NUM_MAC,                    \
    TXQ_HARRRIER_DP_PREEMPTIVE_COUPLE_7_NUM_MAC

#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLES_MAC                      \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_00_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_01_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_02_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_03_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_04_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_05_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_06_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_07_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_08_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_09_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_10_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_11_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_12_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_13_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_14_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_15_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_16_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_17_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_18_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_19_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_20_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_21_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_22_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_23_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_24_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_25_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_26_NUM_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_27_NUM_MAC

/*max speed in Mbyte*/
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_0_MAX_SPEED_MAC 110000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_1_MAX_SPEED_MAC 25000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_2_MAX_SPEED_MAC 50000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_3_MAX_SPEED_MAC 25000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_4_MAX_SPEED_MAC 110000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_5_MAX_SPEED_MAC 25000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_6_MAX_SPEED_MAC 50000
#define TXQ_AC5P_DP_PREEMPTIVE_COUPLE_7_MAX_SPEED_MAC 25000

#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_0_MAX_SPEED_MAC 110000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_1_MAX_SPEED_MAC 25000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_2_MAX_SPEED_MAC 50000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_3_MAX_SPEED_MAC 25000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_4_MAX_SPEED_MAC 110000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_5_MAX_SPEED_MAC 25000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_6_MAX_SPEED_MAC 50000
#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_7_MAX_SPEED_MAC 25000

#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_00_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_01_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_02_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_03_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_04_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_05_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_06_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_07_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_08_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_09_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_10_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_11_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_12_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_13_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_14_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_15_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_16_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_17_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_18_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_19_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_20_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_21_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_22_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_23_MAX_SPEED_MAC  2500
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_24_MAX_SPEED_MAC 10000
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_25_MAX_SPEED_MAC 10000
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_26_MAX_SPEED_MAC 10000
#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_27_MAX_SPEED_MAC 10000


#define TXQ_AC5P_DP_PREEMPTIVE_COUPLES_MAX_SPEED_MAC TXQ_AC5P_DP_PREEMPTIVE_COUPLE_0_MAX_SPEED_MAC, \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_1_MAX_SPEED_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_2_MAX_SPEED_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_3_MAX_SPEED_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_4_MAX_SPEED_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_5_MAX_SPEED_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_6_MAX_SPEED_MAC,                    \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLE_7_MAX_SPEED_MAC

#define TXQ_HARRIER_DP_PREEMPTIVE_COUPLES_MAX_SPEED_MAC TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_0_MAX_SPEED_MAC, \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_1_MAX_SPEED_MAC,                    \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_2_MAX_SPEED_MAC,                    \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_3_MAX_SPEED_MAC,                    \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_4_MAX_SPEED_MAC,                    \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_5_MAX_SPEED_MAC,                    \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_6_MAX_SPEED_MAC,                    \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLE_7_MAX_SPEED_MAC

#define TXQ_IRONMAN_DP_PREEMPTIVE_COUPLES_MAX_SPEED_MAC                  \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_00_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_01_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_02_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_03_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_04_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_05_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_06_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_07_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_08_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_09_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_10_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_11_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_12_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_13_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_14_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_15_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_16_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_17_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_18_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_19_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_20_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_21_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_22_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_23_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_24_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_25_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_26_MAX_SPEED_MAC,                   \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLE_27_MAX_SPEED_MAC


#define TXQ_ALL_DP_PREEMPTIVE_COUPLES_MAC      \
    TXQ_AC5P_DP_PREEMPTIVE_COUPLES_MAC,        \
    TXQ_HARRIER_DP_PREEMPTIVE_COUPLES_MAC,     \
    TXQ_IRONMAN_DP_PREEMPTIVE_COUPLES_MAC



typedef enum {
    /** @brief Local port can be used as express channel
     */
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,
    /** @brief Local port can be used as preemptive channel
     */
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,
    /** @brief Local port cannot  be used as express or preemptive channel
     */
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E
} PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT;

/**
 * @internal prvCpssFalconTxqUtilsPortPreemptionAllowedGet function
 * @endinternal
 *
 * @brief  Check if physical port resources are taken in order to act as preemptive channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] portNum                  -physical port number
 * @param[out] actAsPreemptiveChannelPtr            (pointer to )GT_TRUE if physical port resources are taken in order to act
*                                                                                        as preemptive channel,GT_FALSE otherwise
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PortActAsPreemptiveChannelGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    OUT GT_BOOL              *actAsPreemptiveChannelPtr,
    OUT GT_U32              *expPortNumPtr
);
/**
 * @internal prvCpssDxChTxqSip6GoQIndexGet function
 * @endinternal
 *
 * @brief  Get group of queue number for physical port
 *
 * @note   APPLICABLE DEVICES:      AC5P;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                     - PP's device number.
 * @param[in] physicalPortNum            - physical port
 * @param[in] readFromHw                 - if equal GT_TRUE then thenumber is fetched from EGF table,
 *  otherwise the number is fetched from txQ software data base
 * @param[out] goQPtr                     -(pointer to)group of queues number
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6GoQIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                physicalPortNum,
    IN  GT_BOOL                             readFromHw,
    OUT GT_U32                              *goQPtr
);
/**
 * @internal prvCpssFalconTxqUtilsPortPreemptionAllowedGet function
 * @endinternal
 *
 * @brief  Check if preemption can be enabled on specific port
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] portNum                  -physical port number
 * @param[out] preemptionAllowedPtr            (pointer to )preemption can be activated boolean
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 */
GT_STATUS prvCpssFalconTxqUtilsPortPreemptionAllowedGet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    OUT GT_BOOL                            *preemptionAllowedPtr
);
/**
 * @internal prvCpssDxChTxqSip6_10PreChannelGet function
 * @endinternal
 *
 * @brief  Get preemptive channel for express channel.
 *
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 *  @param[in] speedInMb                            desired speed (for validation purpose ,0 if validation is not required)
 * @param[in] expPort                            express local port in data path
 * @param[out] prePortPtr                        (pointer to)preemption channel number in data path
 *
 * @retval GT_OK                                  - on success.
 * @retval GT_BAD_PARAM                           - wrong pdx number.
 * @retval GT_HW_ERROR                            -on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE               - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreChannelGet
(
    IN GT_U8                devNum,
    IN GT_U32               speedInMb,
    IN GT_U32               expPort,
    OUT GT_U32              *prePortPtr
);

/**
 * @internal prvCpssDxChTxqSip6_10ExpChannelGet function
 * @endinternal
 *
 * @brief  Get express channel for preemptive channel.
 *
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
*  @param[in] devNum                   -PP's device number.
 * @param[in] speedInMb                            desired speed (for validation purpose ,0 if validation is not required)
 * @param[in] expPort                            express local port in data path
 * @param[out] prePortPtr                        (pointer to)preemption channel number in data path
 *
 * @retval GT_OK                                    -   on success.
 * @retval GT_BAD_PARAM                             -  wrong pdx number.
 * @retval GT_HW_ERROR                              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10ExpChannelGet
(
    IN GT_U8                devNum,
    IN GT_U32               speedInMb,
    IN GT_U32               prePort,
    OUT GT_U32              *expPortPtr
);

/**
 * @internal prvCpssDxChTxqSip6_10PreemptionEnableGet function
 * @endinternal
 *
 * @brief  Get enable/disable preemption at PDX glue for specific physical port
 *  Function convert physical port to group of queues and then querry PDX table
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
*  @param[in] speedInMb                            desired speed (in Mb)
 * @param[out] enablePtr                   (pointer to)if equal GT_TRUE then preemption is enabled in PDX,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreemptionEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_U32               speedInMb,
    OUT GT_BOOL             *enablePtr
);
/**
 * @internal prvCpssDxChTxqSip6PreemptionFeatureAllowedGet function
 * @endinternal
 *
 * @brief  Check if preemption feature is enabled for this device/tile
 *
 * @note   APPLICABLE DEVICES:      AC5P;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] enablePtr                   if equal GT_TRUE then preemption feature is enabled per tile ,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PreemptionFeatureAllowedGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL   *enablePtr
);
/**
 * @internal prvCpssDxChTxqSip6_10RestoreChannel function
 * @endinternal
 *
 * @brief  Restore preemption channel configuration that were configured before preemption channel was used .
 *
 * @note   APPLICABLE DEVICES:      AC5P;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expPhyPortNum                       physical port that was used as express channel
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10RestoreChannel
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM expPhyPortNum
);
/**
 * @internal prvCpssDxChTxqSip6_10CopyChannel function
 * @endinternal
 *
 * @brief  Copy configuration of express channel to preemptive channel .
 *
 * @note   APPLICABLE DEVICES:      AC5P;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expPhyPortNum                       physical port that  used as express channel
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10CopyChannel
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM expPhyPortNum
);

/**
 * @internal prvCpssDxChTxqSip6_10PreemptionEnableSet function
 * @endinternal
 *
 * @brief  Set enable/disable preemption at PDX glue for specific physical port.
 *  Function convert physical port to group of queues and then update PDX table
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
 * @param[in] enablePtr                   if equal GT_TRUE then preemption is enabled in PDX,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreemptionEnableSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_BOOL enable
);

GT_STATUS  prvCpssDxChTxqSip6PreeptivePartnerMappedGet
(
    IN GT_U8 devNum,
    IN GT_U32 expPort,
    IN GT_U32 dp,
    OUT GT_BOOL                             *prePortMappedPtr,
    OUT GT_U32                              *prePortPtr
);

/**
 * @internal prvCpssDxChTxqSip6ExpressPartnerGoQGet function
 * @endinternal
 *
 * @brief  Get express channel  group of queues for preemptive channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] prePort                  - express local port in data path
 * @param[in] dp                       - data path index
 * @param[out] expPortGoQPtr            (pointer togroup of queues index of the express channel
 *
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 */
GT_STATUS  prvCpssDxChTxqSip6ExpressPartnerGoQGet
(
    IN GT_U8 devNum,
    IN GT_U32 prePort,
    IN GT_U32 dp,
    OUT GT_U32                              *expPortGoQPtr
);

/**
 * @internal prvCpssDxChTxqSip6LocalPortFreeGoQGet function
 * @endinternal
 *
 * @brief  Get free group of queues index for current local port.
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] prePort                  - express local port in data path
 * @param[in] dp                       -      data path index
 * @param[out] expPortGoQPtr            (pointer to group of queues index of the express channel
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6LocalPortFreeGoQGet
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE      *aNodePtr,
    IN GT_U32 dp,
    IN GT_U32 localPort,
    INOUT GT_U32                              *qroupOfQIteratorPtr
);

/**
 * @internal prvCpssDxChTxqSip6PreeptivePartnerAnodeGet function
 * @endinternal
 *
 * @brief  Get A node that act as preemptive channel for A node that act as epress channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expANodePtr                  -(pointer to )A node that act as epress channel
 * @param[out] preANodePtr                  (pointer to )pointer to A node that act as preemptive channel
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PreeptivePartnerAnodeGet
(
    IN GT_U8                            devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE   *expANodePtr,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  **preANodePtr
);
/**
 * @internal prvCpssSip6TxqUtilsPreemptionStatusGet function
 * @endinternal
 *
 * @brief  Get preemption configuration status
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
*
 *@param[out] preemptionEnabledPtr                            (pointer to )preemption enabled for port
* @param[out] actAsPreemptiveChannelPtr                  (pointer to )port act as preemptive channel
* @param[out] preemptivePhysicalPortNumPtr            (pointer to )physical port number used as preemptive channel
*
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS prvCpssSip6TxqUtilsPreemptionStatusGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    physicalPortNum,
    OUT GT_BOOL                *preemptionEnabledPtr,
    OUT GT_BOOL                *actAsPreemptiveChannelPtr,
    OUT GT_PHYSICAL_PORT_NUM   *preemptivePhysicalPortNumPtr,
    OUT GT_U32                 *preemptiveLocalPortNumPtr
);

GT_STATUS prvCpssSip6TxqUtilsPortParametersGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    OUT GT_BOOL             *portMappedPtr,
    OUT GT_BOOL             *containMacPtr
);


/**
 * @internal prvCpssDxChTxqSip6_10LocalPortTypeGet function
 * @endinternal
 *
 * @brief  Get info regarding local port preemption ability
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 *  @param[in] devNum                   -PP's device number.
 * @param[in] speedInMb                            desired speed (for validation purpose ,0 if validation is not required)
 * @param[in] localPort                  -  local port in data path
 * @param[out] portTypePtr            (pointer to)port preemption tyoe
 *
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10LocalPortTypeGet
(
    IN GT_U8                devNum,
    IN GT_U32               speedInMb,
    IN GT_U32 localPort,
    OUT PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT *portTypePtr
);


/**
 * @internal prvCpssDxChTxqSip6_10LocalPortActualTypeGet function
 * @endinternal
 *
 * @brief  Get info regarding local port preemption current configuration
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in]  devNum                - PP's device number.
 * @param[in]  dpIndex               - data path index
 * @param[in]  localDmaNum           - local port in data path
 * @param[in]  speedInMb             - port speed in megabits/sec
 * @param[out] portTypePtr           - (pointer to)port preemption type
 *
 * @retval GT_OK                      on success.
 * @retval GT_BAD_PARAM               wrong PARAMETER VALUE.
 * @retval GT_HW_ERROR                on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE   on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10LocalPortActualTypeGet
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    dpIndex,
    IN  GT_U32                                    localDmaNum,
    IN  GT_U32                                    speedInMb,
    OUT PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT *portTypePtr
);

/**
 * @internal prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet function
 * @endinternal
 *
 * @brief  Check thatport is not serving as preemptive channel and can be configured
 *
 * @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman; AC5P
 * @note   NOT APPLICABLE DEVICES: None
 *
 * @param[in] devNum                   -PP's device number.
 *  @param[in] physicalPortNum              portNum
*
 * @retval GT_OK                                          -port can be configured
 * @retval GT_BAD_STATE                           port can not be configured
*
 *
 */
GT_STATUS prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum
);

/**
* @internal prvCpssTxqPreemptionUtilsPortStatCounterGet function
* @endinternal
*
* @brief   Gets port preemption statistic counter.
*
* @note   APPLICABLE DEVICES:      AC5P; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] counter                    - statistic counter tye
*
* @param[out] valuePtr                  - (pointer to)counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssTxqPreemptionUtilsPortStatCounterGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MAC_MTI_BR_STAT_COUNTER_ENT counter,
    OUT GT_U32  *valuePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqPreemptionUtils */

