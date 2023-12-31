/*******************************************************************************
*              (c), Copyright 2011, Marvell International Ltd.                 *
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
* @file wraplCpssExtras.h
*
* @brief Internal header which defines API for generic wrappers functions.
*
* @version   9
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>


/***** macroses ********/

/* Lua environment API return codes array   */
#define LUA_ENVIRONMENT_API_RETURN_CODE_ARRAY           "returnCodes"

/* macro to get a pointer on the ExMx device (exMx format)
    devNum - the device id of the exMx device
*/
#define WRL_PRV_CPSS_PP_MAC PRV_CPSS_PP_MAC

/* Port Type speed  10/20 Gbps      */
#define WRL_PRV_CPSS_PORT_XG_E  PRV_CPSS_PORT_XG_E

/* get the port type */
#define WRL_PRV_CPSS_DXCH_PORT_TYPE_MAC PRV_CPSS_DXCH_PORT_TYPE_MAC
#define WRL_PRV_CPSS_EXMXPM_PORT_TYPE_MAC PRV_CPSS_EXMXPM_PORT_TYPE_MAC

/* access to the device's trunk info of the device */
#define WRL_PRV_CPSS_DEV_TRUNK_INFO_MAC PRV_CPSS_DEV_TRUNK_INFO_MAC

/* macro CPSS_PORTS_BMP_PORT_SET_MAC
    to set a port in the bmp ports mask

  portsBmpPtr - of type CPSS_PORTS_BMP_STC*
                pointer to the ports bmp
  portNum - the port num to set in the ports bmp
*/
#define CPSS_PORTS_BMP_MASK_PORT_SET_MAC(portsBmpPtr, portNum)          \
    (portsBmpPtr)->ports[(portNum)>>5] |= 0

/* check that the port number is valid physical port (CPU port is invalid)
    return GT_BAD_PARAM on error
*/
#define WRL_PRV_CPSS_PHY_PORT_CHECK_MAC(devNum, portNum)                \
      ((portNum < WRL_PRV_CPSS_PP_MAC(devNum)->numOfPorts) &&           \
        (1 == (CPSS_PORTS_BMP_IS_PORT_SET_MAC(                          \
            &(WRL_PRV_CPSS_PP_MAC(devNum)->existingPorts) , portNum))))

/* check that the port number is valid physical port (CPU port is valid)
    return GT_BAD_PARAM on error
*/
#define WRL_PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum)     \
      ((portNum) != CPSS_CPU_PORT_NUM_CNS) &&                           \
        WRL_PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,portNum)


/***** macroses ********/

/* Buffer string for temporary storage of lua-wrappers warning messages */
extern GT_CHAR wraplWarningMessageString[];


