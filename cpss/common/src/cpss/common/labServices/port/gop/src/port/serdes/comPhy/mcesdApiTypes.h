/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains common types and defines across the Marvell CE
SERDES API driver (MCESD) for COMPHY_28G_PIPE4_X2, ...
********************************************************************/
#ifndef MCESD_TYPES_H
#define MCESD_TYPES_H

#include <stdint.h>

#define C_LINKAGE 1 /* set to 1 if C compile/linkage on C files is desired with C++ */

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

#define STATIC static

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif  /* __cplusplus */
#endif

typedef void      MCESD_VOID;
typedef int8_t    MCESD_8;
typedef int16_t   MCESD_16;
typedef int32_t   MCESD_32;
typedef int64_t   MCESD_64;

typedef uint8_t       MCESD_U8;
typedef int8_t        MCESD_S8;
typedef uint16_t      MCESD_U16;
typedef uint32_t      MCESD_U32;
typedef uint64_t      MCESD_U64;
typedef unsigned int  MCESD_UINT;
typedef int           MCESD_INT;
typedef int16_t       MCESD_S16;
typedef void*         MCESD_PVOID;

typedef enum {
    MCESD_FALSE = 0,
    MCESD_TRUE  = 1
} MCESD_BOOL;

typedef MCESD_U32 MCESD_STATUS;

#define MCESD_OK        0    /* Operation succeeded */
#define MCESD_FAIL      1    /* Operation failed    */
#define MCESD_PENDING   2    /* Pending  */

#define MCESD_ENABLE    1    /* feature enable */
#define MCESD_DISABLE   0    /* feature disable */

#define MAKEU32FROMU8(msbH, msbL, lsbH, lsbL) (msbH << 24) | (msbL << 16) | (lsbH << 8) | lsbL
#define MAKEU16FROMU8(msb, lsb) (msb << 8) | lsb
#define MAKEU32FROMU16(msb, lsb) (msb << 16 ) | lsb
#define MAKEU64FROMU32(msb, lsb) (msb << 32) | lsb

#ifdef MCESD_DEBUG

#ifdef _WINDOWS
#include <Windows.h>
#include <debugapi.h>
#endif
#include <stdio.h>

typedef enum
{
    MCESD_DBG_OFF_LVL,
    MCESD_DBG_ERR_LVL,
    MCESD_DBG_INF_LVL,
    MCESD_DBG_ALL_LVL
} MCESD_DBG_LEVEL;

void mcesdDbgPrint(FILE *stream, MCESD_DBG_LEVEL debug_level, char* format, ...);

#define MCESD_DBG_ERROR(...)       mcesdDbgPrint(stderr, MCESD_DBG_ERR_LVL, __VA_ARGS__) /* macro for error messages */
#define MCESD_DBG_INFO(...)        mcesdDbgPrint(stdout, MCESD_DBG_ALL_LVL, __VA_ARGS__) /* macro for informational messages */
#define MCESD_DBG_CRITIC_INFO(...) mcesdDbgPrint(stdout, MCESD_DBG_INF_LVL, __VA_ARGS__) /* macro for informational messages */

#else /* MCESD_DEBUG not defined */

#define MCESD_DBG_ERROR(...)
#define MCESD_DBG_INFO(...)
#define MCESD_DBG_CRITIC_INFO(...)

#endif

/* This macro is handy for calling a function when you want to test the
   return value and return MCESD_FAIL, if the function returned MCESD_FAIL,
   otherwise continue */
#define MCESD_ATTEMPT(xFuncToTry) {if(xFuncToTry != MCESD_OK) \
                            {MCESD_DBG_ERROR("MCESD_ATTEMPT failed at %s:%u\n",__FUNCTION__,__LINE__); \
                            return MCESD_FAIL;}}

#define FIELD_DEFINE(reg, hiBit, loBit) { \
    reg, \
    hiBit, \
    loBit, \
    (hiBit-loBit) + 1, \
    (MCESD_U32) ((((MCESD_U64) 1 << ((hiBit-loBit) + 1)) - 1) << loBit), \
    (MCESD_U32) ~((((MCESD_U64) 1 << ((hiBit-loBit) + 1)) - 1) << loBit)}

/* taken from MCESD 2.8.2 */
#define EXTRACT_FIELD(xMCESD_FIELD) { \
    xMCESD_FIELD.reg, \
    xMCESD_FIELD.hiBit, \
    xMCESD_FIELD.loBit, \
    xMCESD_FIELD.totalBits, \
    xMCESD_FIELD.mask, \
    xMCESD_FIELD.retainMask}

typedef struct _MCESD_FIELD
{
    MCESD_U32 reg;
    MCESD_16 hiBit;
    MCESD_16 loBit;
    MCESD_16 totalBits;
    MCESD_U32 mask;
    MCESD_U32 retainMask;
} MCESD_FIELD, *MCESD_FIELD_PTR;

typedef enum
{
    DEVID_UNKNOWN           = 0, /* unknown SERDES IP */
    DEVID_C28GPIPE4X2R2P0   = 1  /* 12FFC COMPHY 28G PIPE4 X2 R2.0 */
} MCESD_DEVICE_ID;

/* Defines for all API functions that need MCESD_DEV */
typedef struct _MCESD_DEV MCESD_DEV;
typedef MCESD_DEV *MCESD_DEV_PTR;

typedef MCESD_STATUS (*FMCESD_READ_REG)(
                        MCESD_DEV_PTR dev,
                        MCESD_U32 reg,
                        MCESD_U32 *value);
typedef MCESD_STATUS (*FMCESD_WRITE_REG)(
                        MCESD_DEV_PTR dev,
                        MCESD_U32 reg,
                        MCESD_U32 value);
typedef MCESD_STATUS (*FMCESD_SET_PIN_CFG)(
                        MCESD_DEV_PTR dev,
                        MCESD_U16 pin,
                        MCESD_U16 pinValue);
typedef MCESD_STATUS (*FMCESD_GET_PIN_CFG)(
                        MCESD_DEV_PTR dev,
                        MCESD_U16 pin,
                        MCESD_U16 *pinValue);
typedef MCESD_STATUS (*FMCESD_WAIT_FUNC)(
                        MCESD_DEV_PTR dev,
                        MCESD_U32 ms);

typedef MCESD_STATUS (*FMCESD_FW_DOWNLOAD)(MCESD_DEV_PTR dev);

/* taken from MCESD 2.8.2 */
typedef MCESD_STATUS (*FMCESD_FW_DOWNLOAD_NEW)(
                        MCESD_DEV_PTR devPtr,
                        MCESD_U32 *fwCodePtr,
                        MCESD_U32 fwCodeSizeDW,
                        MCESD_U32 address,
                        MCESD_U16 *errCode);

struct _MCESD_DEV
{
    MCESD_U8            ipMajorRev;     /* IP revision: Major */
    MCESD_U8            ipMinorRev;     /* IP revision: Minor */

    MCESD_BOOL          devEnabled;     /* mcesdLoadDriver() sets this to TRUE when successful */
    FMCESD_READ_REG     fmcesdReadReg;
    FMCESD_WRITE_REG    fmcesdWriteReg;
    FMCESD_SET_PIN_CFG  fmcesdSetPinCfg;
    FMCESD_GET_PIN_CFG  fmcesdGetPinCfg;
    FMCESD_WAIT_FUNC    fmcesdWaitFunc;

    MCESD_PVOID         appData;        /* application specific data */
};

#define MCESD_API_ERR_SYS_CFG               0x3d<<16/*sys config is NULL. */
#define MCESD_API_ERR_DEV                   0x3c<<16/*driver struture is NULL. */

/* Can't find firmware file to download */
#define MCESD_IMAGE_FILE_DOESNT_EXIST       0xFFFC
/* Image file can't fit into buffer */
#define MCESD_IMAGE_FILE_EXCEEDS_BUFFER     0xFFFD
/* Image larger than the target device's memory */
#define MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD   0xFFFE
/* Error reading or writing register */
#define MCESD_IO_ERROR                      0xFFFF

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif
