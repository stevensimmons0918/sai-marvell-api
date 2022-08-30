/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains common types and defines across the Marvell
T-unit (copper) API driver (MTD) for the Marvell 88X32X0, 88X33X0, 
88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#ifndef MTD_TYPES_H
#define MTD_TYPES_H
#include <mtdFeatures.h>
#ifndef MTD_FEATURES_H
#error "mtdFeatures.h must be included before mtdApiTypes.h"
#endif

#ifdef MTD_MRVL_DEMO_BOARD    /* for demo board environment */
#define DB_TEST_ENVIRONMENT 1 /* set to 0 for host environment */
                              /* set to 1 to build sample code */
                              /* to interface to Marvell DB via SMI board */
#define MTD_INCLUDE_SLAVE_IMAGE  1 /* flash slave hdr file is compiled and linked with API */
                                   /* define 1 and include mtdFwImage.h and mtdFwImage.c from fwimages */
#else
#define DB_TEST_ENVIRONMENT 0
#define MTD_INCLUDE_SLAVE_IMAGE  1 /* flash slave hdr file must be read by host from file system */
#endif

#define MTD_CLAUSE_22_MDIO  0 /* set to 1 to use Clause 22 MDIO support instead of Clause 45 */
                              /* NOTE: X32X0 family DOES NOT support Clause 22 MDIO */

#include <stdint.h>
#if DB_TEST_ENVIRONMENT
#include "stdio.h"
#define  MTD_DEBUG /* comment out to remove all print code/data */
#define  MTD_USE_MDIO_BLOCK_WRITE 1 /* download will do a block transfer instead of one at-a-time XMDIO write if this is 1 */
#else
#define  MTD_USE_MDIO_BLOCK_WRITE 0 /* API code always does each XMDIO write */
#endif
                                   
#define C_LINKAGE 1 /* set to 1 if C compile/linkage on C files is desired with C++ */

#if MTD_PKG_CE_SERDES
#include <serdes/mcesd/mcesdApiTypes.h>
#endif  /* MTD_PKG_CE_SERDES */

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/* general */

#undef IN
#define IN
#undef OUT
#define OUT
#undef INOUT
#define INOUT

#define MTD_STATIC static

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif  /* __cplusplus */
#endif

typedef void        MTD_VOID;
typedef int8_t      MTD_8;
typedef int16_t     MTD_16; 
typedef int32_t     MTD_32;
typedef int64_t     MTD_64;

typedef uint8_t         MTD_U8;
typedef int8_t          MTD_S8;
typedef uint16_t        MTD_U16;
typedef uint32_t        MTD_U32;
typedef unsigned int    MTD_UINT; 
typedef int             MTD_INT;
typedef int16_t         MTD_S16;
typedef void*           MTD_PVOID;

typedef unsigned long long  MTD_U64;

typedef enum {
    MTD_FALSE = 0,
    MTD_TRUE  = 1
} MTD_BOOL;

#define MTD_CONVERT_BOOL_TO_UINT(boolVar,uintVar) {(boolVar) ? (uintVar=1) : (uintVar=0);}
#define MTD_CONVERT_UINT_TO_BOOL(uintVar,boolVar) {(uintVar) ? (boolVar=MTD_TRUE) : (boolVar=MTD_FALSE);}
#define MTD_GET_BOOL_AS_BIT(boolVar) ((boolVar) ? 1 : 0)
#define MTD_GET_BIT_AS_BOOL(uintVar) ((uintVar) ? MTD_TRUE : MTD_FALSE)

typedef void     (*MTD_VOIDFUNCPTR) (void); /* ptr to function returning void */
typedef MTD_U32  (*MTD_INTFUNCPTR)  (void); /* ptr to function returning int  */

typedef MTD_U32 MTD_STATUS;

/* Defines for mtdLoadDriver() mtdUnloadDriver() and all API functions which need MTD_DEV */
typedef struct _MTD_DEV MTD_DEV;
typedef MTD_DEV *MTD_DEV_PTR;

typedef MTD_STATUS (*FMTD_WAIT_FUNC)(
                        MTD_DEV_PTR dev,
                        MTD_UINT milliSeconds);

#if (MTD_CLAUSE_22_MDIO == 0)

/* normal case is Clause 45 MDIO */
typedef MTD_STATUS (*FMTD_READ_MDIO)(
                        MTD_DEV*   dev,
                        MTD_U16 port, 
                        MTD_U16 mmd, 
                        MTD_U16 reg, 
                        MTD_U16* value);
typedef MTD_STATUS (*FMTD_WRITE_MDIO)(
                        MTD_DEV*   dev,
                        MTD_U16 port, 
                        MTD_U16 mmd, 
                        MTD_U16 reg, 
                        MTD_U16 value);
#else

/* Clause 22 MDIO has a different format */
typedef MTD_STATUS (*FMTD_READ_MDIO)(
                        MTD_DEV*   dev,
                        MTD_U16 port, 
                        MTD_U16 reg, 
                        MTD_U16* value);
typedef MTD_STATUS (*FMTD_WRITE_MDIO)(
                        MTD_DEV*   dev,
                        MTD_U16 port, 
                        MTD_U16 reg, 
                        MTD_U16 value);

#endif


/* MTD_DEVICE_ID format:  */
/* Bits 15:14 reserved */
/* Bit 13: 1-> E21X1 device with max speed of 2.5G */
/* Bit 12: 1-> E20X0 device with max speed of 5G and no fiber interface */
/* Bit 11: 1-> Macsec Capable (Macsec/PTP module included */
/* Bit  10: 1-> Copper Capable (T unit interface included) */
/* Bits 9:4 0x18 -> X32X0 base, 0x1A X33X0 base, 0x1B E21X0 base, 0x3C X35X0 base */
/* Bits 3:0 revision/number of ports indication, see list */
/* Following defines are for building MTD_DEVICE_ID */
#define MTD_E21X1_DEVICE (1<<13)   /* whether this is an E21X1 device group */
#define MTD_E2XX0_DEVICE (1<<12)   /* whether this is an E20X0 or E25X0 device group */
#define MTD_MACSEC_CAPABLE (1<<11) /* whether the device has a Macsec/PTP module */
#define MTD_COPPER_CAPABLE (1<<10) /* whether the device has a copper (T unit) module */
#define MTD_X32X0_BASE (0x18<<4)   /* whether the device uses X32X0 firmware base */
#define MTD_X33X0_BASE (0x1A<<4)   /* whether the device uses X33X0 firmware base */
#define MTD_E21X0_BASE (0x1B<<4)   /* whether the device uses E21X0 firmware base, this is a max 5G part, copper only, no Macsec, basic PTP */
#define MTD_X35X0_BASE (0x3C<<4)   /* whether the device uses X35X0 firmware base, this is a max 10G part, */

#define MTD_DEVID_GETBASE(mTdrevID) (mTdrevID & (0x3F<<4)) /* to get the device base identifier */

/* Following macros are to test MTD_DEVICE_ID for various features */
#define MTD_IS_MACSEC_CAPABLE(mTdrevId) ((MTD_BOOL)(mTdrevId & MTD_MACSEC_CAPABLE))
#define MTD_IS_COPPER_CAPABLE(mTdrevId) ((MTD_BOOL)(mTdrevId & MTD_COPPER_CAPABLE))
#define MTD_IS_X32X0_BASE(mTdrevId) ((MTD_BOOL)(MTD_DEVID_GETBASE(mTdrevId) == MTD_X32X0_BASE))
#define MTD_IS_X33X0_BASE(mTdrevId) ((MTD_BOOL)(MTD_DEVID_GETBASE(mTdrevId) == MTD_X33X0_BASE))
#define MTD_IS_E21X0_BASE(mTdrevId) ((MTD_BOOL)(MTD_DEVID_GETBASE(mTdrevId) == MTD_E21X0_BASE))
#define MTD_IS_X35X0_BASE(mTdrevId) ((MTD_BOOL)(MTD_DEVID_GETBASE(mTdrevId) == MTD_X35X0_BASE))
#define MTD_HAS_X_UNIT(mTddevPtr)   !(MTD_IS_E21X0_BASE(mTddevPtr->deviceId)   || \
                                      MTD_IS_E2XX0_DEVICE(mTddevPtr->deviceId) || \
                                      MTD_IS_X35X0_BASE(mTddevPtr->deviceId))

/* Following macros are to test whether passed-in device structure is a specific device */
#define MTD_IS_E2XX0_DEVICE(mTdrevId) ((MTD_BOOL)(mTdrevId & MTD_E2XX0_DEVICE)) /* E20X0 or E25X0 device, i.e. no 10G copper and no X unit */
#define MTD_IS_E20X0_DEVICE(mTdrevId) ((MTD_IS_X33X0_BASE(mTdrevId)) && ((MTD_BOOL)(mTdrevId & MTD_E2XX0_DEVICE)))
#define MTD_IS_E25X0_DEVICE(mTdrevId) ((MTD_IS_X35X0_BASE(mTdrevId)) && ((MTD_BOOL)(mTdrevId & MTD_E2XX0_DEVICE)))
#define MTD_IS_X3540_E2540_DEVICE(mTddevPtr) (MTD_IS_X35X0_BASE(mTddevPtr->deviceId) && mTddevPtr->numPortsPerDevice == 4)
#define MTD_IS_X3580_DEVICE(mTddevPtr) (MTD_IS_X35X0_BASE(mTddevPtr->deviceId) && mTddevPtr->numPortsPerDevice == 8) /* Will also be true for E2580 */
#define MTD_IS_X35X0_E2540_DEVICE(mTddevPtr) (MTD_IS_X3580_DEVICE(mTddevPtr) || MTD_IS_X3540_E2540_DEVICE(mTddevPtr)) /* Will also be true for E2580 */
#define MTD_IS_E21X1_DEVICE(mTdrevId) ((MTD_BOOL)(mTdrevId & MTD_E21X1_DEVICE)) /* E2111 or E2181 device, 2.5G and below device */

#define MTD_X33X0BASE_SINGLE_PORTA0 0xA
#define MTD_X33X0BASE_DUAL_PORTA0   0x6
#define MTD_X33X0BASE_QUAD_PORTA0   0x2
#define MTD_X33X0BASE_SINGLE_PORTA1 0xB
#define MTD_X33X0BASE_DUAL_PORTA1   0x7
#define MTD_X33X0BASE_QUAD_PORTA1   0x3
#define MTD_X33X0BASE_SINGLE_PORTB0 0xE
#define MTD_X33X0BASE_DUAL_PORTB0   0x4
#define MTD_X33X0BASE_QUAD_PORTB0   0xC
#define MTD_X33X0BASE_SINGLE_PORTB1 0xF
#define MTD_X33X0BASE_DUAL_PORTB1   0x5
#define MTD_X33X0BASE_QUAD_PORTB1   0xD

#define MTD_E21X0BASE_SINGLE_PORTZ1 0x8 /* Base for single port */
#define MTD_E21X0BASE_SINGLE_PORTA0 0x9 
#define MTD_E21X0BASE_QUAD_PORTA0   0x4 /* Base for quad, currently not available */
#define MTD_E21X0BASE_OCTAL_PORTA0  0x0 /* base for octal */
#define MTD_E21X0BASE_OCTAL_PORTA1  0x1

#define MTD_X35X0BASE_QUAD_PORTA0   0x0
#define MTD_X35X0BASE_QUAD_PORTB0   0x1

#define MTD_X35X0BASE_OCTAL_PORTA0  0x3 /* X3580 */


/* WARNING: If you add/modify this list, you must also modify mtdIsPhyRevisionValid() */
typedef enum
{
    MTD_REV_UNKNOWN = 0,
    MTD_REV_3240P_Z2 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x1),
    MTD_REV_3240P_A0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x2),
    MTD_REV_3240P_A1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x3),
    MTD_REV_3220P_Z2 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x4),
    MTD_REV_3220P_A0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x5),
    MTD_REV_3240_Z2 = (MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x1),
    MTD_REV_3240_A0 = (MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x2),
    MTD_REV_3240_A1 = (MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x3),
    MTD_REV_3220_Z2 = (MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x4),
    MTD_REV_3220_A0 = (MTD_COPPER_CAPABLE | MTD_X32X0_BASE | 0x5),

    MTD_REV_3310P_A0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA0),
    MTD_REV_3320P_A0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA0),
    MTD_REV_3340P_A0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA0),
    MTD_REV_3310_A0 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA0),
    MTD_REV_3320_A0 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA0),
    MTD_REV_3340_A0 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA0),

    MTD_REV_E2010P_A0 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA0),
    MTD_REV_E2020P_A0 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA0),
    MTD_REV_E2040P_A0 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA0),
    MTD_REV_E2010_A0 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA0),
    MTD_REV_E2020_A0 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA0),
    MTD_REV_E2040_A0 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA0),

    MTD_REV_2340P_A0 = (MTD_MACSEC_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA0),
    MTD_REV_2340_A0 = (MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA0), 

    MTD_REV_3310P_A1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA1),
    MTD_REV_3320P_A1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA1),
    MTD_REV_3340P_A1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA1),
    MTD_REV_3310_A1 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA1),
    MTD_REV_3320_A1 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA1),
    MTD_REV_3340_A1 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA1),

    MTD_REV_E2010P_A1 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA1),
    MTD_REV_E2020P_A1 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA1),
    MTD_REV_E2040P_A1 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA1),
    MTD_REV_E2010_A1 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTA1),
    MTD_REV_E2020_A1 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTA1),
    MTD_REV_E2040_A1 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA1),

    MTD_REV_2340P_A1 = (MTD_MACSEC_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA1),
    MTD_REV_2340_A1 = (MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTA1),     

    MTD_REV_2340P_A1_NA = (MTD_MACSEC_CAPABLE | MTD_X32X0_BASE | 0x3),  /* 2340 is based on 33X0 device, there is no such device as this one */
    MTD_REV_2320P_A0 = (MTD_MACSEC_CAPABLE | MTD_X32X0_BASE | 0x5),     /* 2320 is based on 32X0 device */
    MTD_REV_2340_A1_NA = (MTD_X32X0_BASE | 0x3),                        /* 2340 is based on 33X0 device, there is no such device as this one */
    MTD_REV_2320_A0 = (MTD_X32X0_BASE | 0x5),                           /* 2320 is based on 32X0 device */

    MTD_REV_3310P_B0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB0),
    MTD_REV_3320P_B0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB0),
    MTD_REV_3340P_B0 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB0),
    MTD_REV_3310_B0 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB0),
    MTD_REV_3320_B0 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB0),
    MTD_REV_3340_B0 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB0),

    MTD_REV_E2010P_B0 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB0),
    MTD_REV_E2020P_B0 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB0),
    MTD_REV_E2040P_B0 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB0),
    MTD_REV_E2010_B0 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB0),
    MTD_REV_E2020_B0 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB0),
    MTD_REV_E2040_B0 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB0),

    MTD_REV_2340P_B0 = (MTD_MACSEC_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB0),
    MTD_REV_2340_B0 = (MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB0),

    MTD_REV_3310P_B1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB1),
    MTD_REV_3320P_B1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB1),
    MTD_REV_3340P_B1 = (MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB1),
    MTD_REV_3310_B1 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB1),
    MTD_REV_3320_B1 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB1),
    MTD_REV_3340_B1 = (MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB1),

    MTD_REV_E2010P_B1 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB1),
    MTD_REV_E2020P_B1 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB1),
    MTD_REV_E2040P_B1 = (MTD_E2XX0_DEVICE | MTD_MACSEC_CAPABLE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB1),
    MTD_REV_E2010_B1 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_SINGLE_PORTB1),
    MTD_REV_E2020_B1 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_DUAL_PORTB1),
    MTD_REV_E2040_B1 = (MTD_E2XX0_DEVICE | MTD_COPPER_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB1),

    MTD_REV_2340P_B1 = (MTD_MACSEC_CAPABLE | MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB1),
    MTD_REV_2340_B1 = (MTD_X33X0_BASE | MTD_X33X0BASE_QUAD_PORTB1),

    MTD_REV_E2110_Z1 = (MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_SINGLE_PORTZ1),
    MTD_REV_E2110_A0 = (MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_SINGLE_PORTA0),
    MTD_REV_E2111_A0 = (MTD_E21X1_DEVICE | MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_SINGLE_PORTA0),
    
    MTD_REV_E2140_A0 = (MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_QUAD_PORTA0), /* part not currently available */
    
    MTD_REV_E2180_A0 = (MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_OCTAL_PORTA0),
    MTD_REV_E2180_A1 = (MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_OCTAL_PORTA1),
    MTD_REV_E2181_A1 = (MTD_E21X1_DEVICE | MTD_E21X0_BASE | MTD_COPPER_CAPABLE | MTD_E21X0BASE_OCTAL_PORTA1),

    MTD_REV_X3540_A0 = (MTD_X35X0_BASE | MTD_COPPER_CAPABLE | MTD_X35X0BASE_QUAD_PORTA0),
    MTD_REV_X3540_B0 = (MTD_X35X0_BASE | MTD_COPPER_CAPABLE | MTD_X35X0BASE_QUAD_PORTB0),
    MTD_REV_E2540_A0 = (MTD_E2XX0_DEVICE | MTD_X35X0_BASE | MTD_COPPER_CAPABLE | MTD_X35X0BASE_QUAD_PORTA0),
    MTD_REV_E2540_B0 = (MTD_E2XX0_DEVICE | MTD_X35X0_BASE | MTD_COPPER_CAPABLE | MTD_X35X0BASE_QUAD_PORTB0),

    MTD_REV_X3580_A0 = (MTD_X35X0_BASE | MTD_COPPER_CAPABLE | MTD_X35X0BASE_OCTAL_PORTA0),
    MTD_REV_E2580_A0 = (MTD_E2XX0_DEVICE | MTD_X35X0_BASE | MTD_COPPER_CAPABLE | MTD_X35X0BASE_OCTAL_PORTA0)

} MTD_DEVICE_ID;

typedef enum
{
    MTD_MSEC_REV_Z0A,
    MTD_MSEC_REV_Y0A,
    MTD_MSEC_REV_A0B,
    MTD_MSEC_REV_FPGA,
    MTD_MSEC_REV_UNKNOWN  = -1
} MTD_MSEC_REV;

/* compatible for USB test */
typedef struct  _MTD_MSEC_CTRL {
  MTD_32 dev_num;      /* indicates the device number (0 if only one) when multiple devices are present on SVB.*/
  MTD_32 port_num;     /* Indicates which port (0 to 4) is requesting CPU */
  MTD_U16 prev_addr;   /* < Prev write address */
  MTD_U16 prev_dataL;  /* < Prev dataL value */
  MTD_MSEC_REV msec_rev;  /* revision */
} MTD_MSEC_CTRL;

#if MTD_PKG_CE_SERDES

typedef struct _MTD_SERDES_CTRL
{
    MCESD_DEV        serdesDev;     /* serdes device struct to control serdes */
    MTD_U16          serdesPort;    /* the mapped port for serdes operation, */
                                    /* MUST be set by calling mtdSetSerdesPortGetDevPtr() or mtdSetSerdesPort() */
                                    /* before any COMPHY serdes API calls */
} MTD_SERDES_CTRL, *PMTD_SERDES_CTRL;

typedef union _MTD_SERDES_CTRL_TYPE {
    MTD_U8  serdesCtrlBuf[64];  /* reserved 64 bytes for the serdes control union */
    MTD_SERDES_CTRL c28gX2;
} MTD_SERDES_CTRL_TYPE;

#endif  /* MTD_PKG_CE_SERDES */


/* Warning: If copying variables based on this type, serdesCtrl and appData may contain pointers
   that may not be valid with simple copying strategies. Also, some of the other values filled in by
   mtdLoadDriver() may not be valid if simply copied. It is not safe to make a simple copy. */
struct _MTD_DEV
{
    MTD_DEVICE_ID         deviceId;             /* type of device and capabilities */
    MTD_BOOL              devEnabled;           /* whether mtdLoadDriver() called successfully */
    MTD_U8                numPortsPerDevice;    /* number of ports per device */
    MTD_U8                thisPort;             /* relative port number on this device starting with 0 (not MDIO address) */

    FMTD_READ_MDIO        fmtdReadMdio;  
    FMTD_WRITE_MDIO       fmtdWriteMdio;
    FMTD_WAIT_FUNC        fmtdWaitFunc;

    MTD_U32               devInfo;              /* operations, features, status tracking */
    MTD_MSEC_CTRL         msec_ctrl;            /* structure use for internal verification */

#if MTD_PKG_CE_SERDES
    MTD_SERDES_CTRL_TYPE  serdesCtrl;           /* Structure for controlling Serdes API */
#endif  /* MTD_PKG_CE_SERDES */

    MTD_PVOID             appData;              /* application specific data, anything the host wants to pass to the low layer */
 
};

/*****************************************************************************
devInfo bit info used in device structure for operations, features,
status tracking, etc...
*****************************************************************************/
#define MTD_MACSEC_INDIRECT_ACCESS (1<<0)  /* indirect Macsec access for some device */
/* RESERVED (1<<1)  */
/* ...              */
/* RESERVED (1<<31) */


#define MTD_OK       0    /* Operation succeeded */
#define MTD_FAIL     1    /* Operation failed    */
#define MTD_PENDING  2    /* Pending  */

#define MTD_ENABLE   1    /* feature enable */
#define MTD_DISABLE  0    /* feature disable */


/* bit definition */
#define MTD_BIT_0       0x0001
#define MTD_BIT_1       0x0002
#define MTD_BIT_2       0x0004
#define MTD_BIT_3       0x0008
#define MTD_BIT_4       0x0010
#define MTD_BIT_5       0x0020
#define MTD_BIT_6       0x0040
#define MTD_BIT_7       0x0080
#define MTD_BIT_8       0x0100
#define MTD_BIT_9       0x0200
#define MTD_BIT_10      0x0400
#define MTD_BIT_11      0x0800
#define MTD_BIT_12      0x1000
#define MTD_BIT_13      0x2000
#define MTD_BIT_14      0x4000
#define MTD_BIT_15      0x8000



#ifdef MTD_DEBUG
typedef enum 
{
    MTD_DBG_OFF_LVL,
    MTD_DBG_ERR_LVL,
    MTD_DBG_INF_LVL,
    MTD_DBG_ALL_LVL
} MTD_DBG_LEVEL;

/*extern void mtdFuncDbgPrint(char* format, ...);*/
void mtdDbgPrint(FILE *stream, MTD_DBG_LEVEL debug_level, char* format, ...);

#define MTD_DBG_ERROR(...)       mtdDbgPrint(stderr, MTD_DBG_ERR_LVL, __VA_ARGS__) /* macro for error messages */
#define MTD_DBG_INFO(...)        mtdDbgPrint(stdout, MTD_DBG_ALL_LVL, __VA_ARGS__) /* macro for informational messages */
#define MTD_DBG_CRITIC_INFO(...) mtdDbgPrint(stdout, MTD_DBG_INF_LVL, __VA_ARGS__) /* macro for informational messages */

#else /* MTD_DEBUG not defined */

#define MTD_DBG_ERROR(...)
#define MTD_DBG_INFO(...)
#define MTD_DBG_CRITIC_INFO(...)

#endif

#ifdef MTD_SDEBUG
#include "..\mtdMatLab\tstSTypes.h"
#endif

#ifdef MTD_PY_DEBUG
#include "..\mtdPython\mtdPythonSrc\tstSTypes.h"
#endif

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* MTD_TYPES_H */
