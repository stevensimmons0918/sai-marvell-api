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
* @file mainPpDrv/h/cpss/generic/cpssHwInit/cpssHwInit.h
*
* @brief Includes CPSS basic Hw initialization functions, and data structures.
*
* @version   38
********************************************************************************
*/
#ifndef __cpssHwInith
#define __cpssHwInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/extMac/cpssExtMacDrv.h>

/**
* @enum CPSS_MAC_SA_LSB_MODE_ENT
 *
 * @brief Enumeration of MAC SA lsb modes ,
 * this relate to "pause packets" and to "routed packets".
*/
typedef enum{

    /** @brief Sets the MAC SA least significant byte
     *  according to the per port setting.
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2.)
     *  (NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    CPSS_SA_LSB_PER_PORT_E,

    /** @brief Sets the MAC SA least significant 12-bits
     *  according to the packet's VLAN ID.
     *  (NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_SA_LSB_PER_PKT_VID_E,

    /** @brief Sets the MAC SA least significant byte
     *  according to the per vlan setting.
     */
    CPSS_SA_LSB_PER_VLAN_E,

    /** @brief Global MAC SA: Sets full 48-bit MAC SA
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    CPSS_SA_LSB_FULL_48_BIT_GLOBAL

} CPSS_MAC_SA_LSB_MODE_ENT;

/**
* @enum CPSS_SRAM_FREQUENCY_ENT
 *
 * @brief Defines the different Sram frequencies that an sram can have.
*/
typedef enum{

    /** 125 MHz sram frequency. */
    CPSS_SRAM_FREQ_125_MHZ_E = 125,

    /** 166 MHz sram frequency. */
    CPSS_SRAM_FREQ_166_MHZ_E = 166,

    /** 200 MHz sram frequency. */
    CPSS_SRAM_FREQ_200_MHZ_E = 200,

    /** 250 MHz sram frequency. */
    CPSS_SRAM_FREQ_250_MHZ_E = 250,

    /** 300 MHz sram frequency. */
    CPSS_SRAM_FREQ_300_MHZ_E = 300

} CPSS_SRAM_FREQUENCY_ENT;



/**
* @struct CPSS_BUFF_MGMT_CFG_STC
 *
 * @brief Includes common buffer management configuration values, to be
 * set to Pp's registers at initialization.
*/
typedef struct{

    /** @brief when required, indicates to overwrite the PP's default
     *  values.
     *  GT_TRUE - overwrite,
     *  GT_FALSE - ignore the rest of this struct's data.
     */
    GT_BOOL overwriteDef;

    /** @brief The number of buffers allocated in the PP Buffer
     *  DRAM. This value must not exceed the maximal allowed
     *  number of buffers (depending on the device type).
     */
    GT_U16 numPpBuffers;

    /** @brief The total number of buffers used by the uplink
     *  device
     *  Comments:
     *  - Refer to the data-sheet for complete description of these parameters.
     *  - Setting an undefined values, like reserved values, may result in
     *  unpredictable results.
     */
    GT_U16 numUplinkBuffers;

} CPSS_BUFF_MGMT_CFG_STC;


/* Delimiter for registers list regs */
#define CPSS_REG_LIST_DELIMITER_CNS         0xFFFFFFFF

/* Delimiter for phy addr regs */
#define CPSS_REG_PHY_ADDR_TYPE_CNS          0xEEEEEEEE

/**
* @struct CPSS_REG_VALUE_INFO_STC
 *
 * @brief Holds a specific register value to be used for configuring a
 * device's register in system.
 * This structure is a part of a values' list to be
 * written into the device in the initialization sequence.
*/
typedef struct{

    /** Holds the register address to write to. */
    GT_U32 addrType;

    /** @brief Holds the valid bits out of regVal to be written to the
     *  register, value of 0xFFFFFFFF will write to the whole
     *  register.
     */
    GT_U32 regMask;

    /** @brief Holds the register value to write (after a bit wise & with
     *  regMask).
     */
    GT_U32 regVal;

    /** @brief Number of repeatitions this register writes. (Useful
     *  for DFCDL's values configuration).
     */
    GT_U32 repeatCount;

} CPSS_REG_VALUE_INFO_STC;


/**
* @enum CPSS_AU_MESSAGE_LENGTH_ENT
 *
 * @brief packet buffer sizes
*/
typedef enum{

    /** AU message is 16 byte length */
    CPSS_AU_MESSAGE_LENGTH_4_WORDS_E  = 0,

    /** AU message is 32 byte length */
    CPSS_AU_MESSAGE_LENGTH_8_WORDS_E

} CPSS_AU_MESSAGE_LENGTH_ENT;

/**
* @enum CPSS_BUF_MODE_ENT
 *
 * @brief packet buffer sizes
*/
typedef enum{

    /** 1536 byte buffers */
    CPSS_BUF_1536_E  = 0,

    /** 2K byte buffers */
    CPSS_BUF_2K_E,

    /** 10K byte buffers */
    CPSS_BUF_10K_E

} CPSS_BUF_MODE_ENT;

/**
* @enum CPSS_BANK_MODE_ENT
 *
 * @brief packet bank modes
*/
typedef enum{

    /** no smart bank */
    CPSS_NO_SMART_BANK_E = 0,

    /** partial smart bank */
    CPSS_PARTIAL_SMART_BANK_E,

    /** regular smart bank */
    CPSS_REGULAR_SMART_BANK_E

} CPSS_BANK_MODE_ENT;


/**
* @enum CPSS_DRAM_SIZE_ENT
 *
 * @brief Packet Processor DRAM sizes
*/
typedef enum{

    /** External DRAM is not connected. */
    CPSS_DRAM_NOT_USED_E   = 0x0,

    /** DRAM size is 64KByte. */
    CPSS_DRAM_64KB_E       = _64KB,

    /** DRAM size is 128KByte. */
    CPSS_DRAM_128KB_E      = _128KB,

    /** DRAM size is 8 MByte. */
    CPSS_DRAM_8MB_E        = _8MB,

    /** DRAM size is 16 MByte. */
    CPSS_DRAM_16MB_E       = _16MB,

    /** DRAM size is 32 MByte. */
    CPSS_DRAM_32MB_E       = _32MB,

    /** DRAM size is 64 MByte. */
    CPSS_DRAM_64MB_E       = _64MB,

    /** DRAM size is 128 MByte. */
    CPSS_DRAM_128MB_E      = _128MB,

    /** DRAM size is 256 MByte. */
    CPSS_DRAM_256MB_E      = _256MB,

    /** DRAM size is 512 MByte. */
    CPSS_DRAM_512MB_E      = _512MB,

    /** @brief DRAM size is 1 GByte.
     *  Notes: Some Prestera devices do not have external memory interfaces
     */
    CPSS_DRAM_1GB_E        = _1GB

} CPSS_DRAM_SIZE_ENT;



/**
* @enum CPSS_DRAM_FREQUENCY_ENT
 *
 * @brief Defines the different DRAM frequencies that a DRAM can have.
*/
typedef enum{

    /** 100 MHz dram frequency. */
    CPSS_DRAM_FREQ_100_MHZ_E = 100,

    /** 125 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_125_MHZ_E = 125,

    /** 166 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_166_MHZ_E = 166,

    /** 200 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_200_MHZ_E = 200,

    /** 250 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_250_MHZ_E = 250,

    /** 300 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_300_MHZ_E = 300,

    /** 667 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_667_MHZ_E = 667,

    /** 800 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_800_MHZ_E = 800,

    /** 933 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_933_MHZ_E = 933,

    /** 1066 MHz dram frequency. */
    CPSS_DRAM_FREQ_1066_MHZ_E = 1066

} CPSS_DRAM_FREQUENCY_ENT;

/**
* @enum CPSS_DRAM_BUS_WIDTH_ENT
 *
 * @brief Defines the DRAM interface bus width in bits.
*/
typedef enum{

    /** 4 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_4_E,

    /** 8 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_8_E,

    /** 16 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_16_E,

    /** 32 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_32_E

} CPSS_DRAM_BUS_WIDTH_ENT;

/**
* @enum CPSS_DRAM_TEMPERATURE_ENT
 *
 * @brief Defines the DRAM operation temperature
 * (refresh interval related).
*/
typedef enum{

    /** Up to 85C operating temperature. */
    CPSS_DRAM_TEMPERATURE_NORMAL_E,

    /** Operating temperature can exceed 85C. */
    CPSS_DRAM_TEMPERATURE_HIGH_E

} CPSS_DRAM_TEMPERATURE_ENT;

/**
* @enum CPSS_DRAM_SPEED_BIN_ENT
 *
 * @brief Defines DRAM speed bining (JEDEC standard name).
*/
typedef enum{

    /** DDR3-800D JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_800D_E,

    /** DDR3-800E JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_800E_E,

    /** DDR3-1066E JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1066E_E,

    /** DDR3-1066F JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1066F_E,

    /** DDR3-1066G JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1066G_E,

    /** DDR3-1333F JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333F_E,

    /** DDR3-1333G JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333G_E,

    /** DDR3-1333H JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333H_E,

    /** DDR3-1333J JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333J_E,

    /** DDR3-1600G JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600G_E,

    /** DDR3-1600H JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600H_E,

    /** DDR3-1600J JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600J_E,

    /** DDR3-1600K JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600K_E,

    /** DDR3-1866J JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866J_E,

    /** DDR3-1866K JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866K_E,

    /** DDR3-1866L JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866L_E,

    /** DDR3-1866M JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866M_E,

    /** DDR3-2133K JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133K_E,

    /** DDR3-2133L JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133L_E,

    /** DDR3-2133M JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133M_E,

    /** DDR3-2133N JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133N_E

} CPSS_DRAM_SPEED_BIN_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* cpssHwInit */


