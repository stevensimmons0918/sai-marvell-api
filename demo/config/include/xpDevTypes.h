// xpDevTypes.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpDevTypes_h_
#define _xpDevTypes_h_

#include <string.h>

//#include "xpSalInterface.h"
#include "xpsXpImports.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    XP_SDK_DEV_TYPE_WM,           /* White model simulator */
    XP_SDK_DEV_TYPE_SHADOW,      /* Not really related to platform, but needed for profile */
    XP_SDK_DEV_TYPE_SVB,
    XP_SDK_DEV_TYPE_CRB1_REV1,
    XP_SDK_DEV_TYPE_CRB1_REV2,
    XP_SDK_DEV_TYPE_CRB2,
    XP_SDK_DEV_TYPE_CRB2_REV2,
    XP_SDK_DEV_TYPE_CRB2_REV3,
    XP_SDK_DEV_TYPE_CRB3_REV1,
    XP_SDK_DEV_TYPE_CRB3_REV2,
    XP_SDK_DEV_TYPE_CRB1_XP70_REV1,
    XP_SDK_DEV_TYPE_CRB1_XP70_REV2,
    XP_SDK_DEV_TYPE_ET6448M,
    XP_SDK_DEV_TYPE_ALDRIN_DB,
    XP_SDK_DEV_TYPE_CYGNUS,
    XP_SDK_DEV_TYPE_ALDRIN2_XL,
    XP_SDK_DEV_TYPE_TG48M_P,
    XP_SDK_DEV_TYPE_FALCON_32,
    XP_SDK_DEV_TYPE_FALCON_32X400G,
    XP_SDK_DEV_TYPE_FALCON_32X100G_8X400G,
    XP_SDK_DEV_TYPE_FALCON_24X25G_6X100G_3_2,
    XP_SDK_DEV_TYPE_FALCON_24X25G_8X100G,
    XP_SDK_DEV_TYPE_FALCON_24X25G_8X100G_6_4,
    XP_SDK_DEV_TYPE_FALCON_24X25G_4X200G,
    XP_SDK_DEV_TYPE_FALCON_24X25G_4X200G_6_4,
    XP_SDK_DEV_TYPE_FALCON_24X100G_4X400G,
    XP_SDK_DEV_TYPE_FALCON_16X100G_8X400G,
    XP_SDK_DEV_TYPE_FALCON_48X100G_8X400G,
    XP_SDK_DEV_TYPE_FALCON_128X10G,
    XP_SDK_DEV_TYPE_FALCON_128X25G,
    XP_SDK_DEV_TYPE_FALCON_128X50G,
    XP_SDK_DEV_TYPE_FALCON_128X100G,
    XP_SDK_DEV_TYPE_FALCON_128,
    XP_SDK_DEV_TYPE_FALCON_32X25G,
    XP_SDK_DEV_TYPE_FALCON_32X25G_6_4,
    XP_SDK_DEV_TYPE_FALCON_64X100G,
    XP_SDK_DEV_TYPE_FALCON_64X100G_R4,
    XP_SDK_DEV_TYPE_FALCON_64X25G,
    XP_SDK_DEV_TYPE_FALCON_16X400G,
    XP_SDK_DEV_TYPE_FALCON_16X25G,
    XP_SDK_DEV_TYPE_FALCON_32X25G_IXIA,
    XP_SDK_DEV_TYPE_FALCON_64X100G_R4_IXIA,
    XP_SDK_DEV_TYPE_FALCON_32X100G_R4_IXIA_6_4,
    XP_SDK_DEV_TYPE_FALCON_32X100G_16X400G,
    XP_SDK_DEV_TYPE_FALCON_96X100G_8X400G,
    XP_SDK_DEV_TYPE_FALCON_16X25G_IXIA,
    XP_SDK_DEV_TYPE_FALCON_48x10G_8x100G,
    XP_SDK_DEV_TYPE_FALCON_48x25G_8x100G,
    XP_SDK_DEV_TYPE_FALCON_64x25G_64x10G,
    XP_SDK_DEV_TYPE_FALCON_32x25G_8x100G,
    XP_SDK_DEV_TYPE_FALCON_F2T80x25G,
    XP_SDK_DEV_TYPE_FALCON_F2T80x25GIXIA,
    XP_SDK_DEV_TYPE_FALCON_F2T48x25G8x100G,
    XP_SDK_DEV_TYPE_FALCON_F2T48x10G8x100G,
    XP_SDK_DEV_TYPE_AC5XRD,
    XP_SDK_DEV_TYPE_AC5XRD_48x1G6x10G,
    XP_SDK_DEV_TYPE_ALDRIN2_XL_FL,
    XP_SDK_DEV_TYPE_AC3X_FS,
    XP_SDK_DEV_TYPE_UNKNOWN
} xpSdkDevType_t;

typedef enum
{
    XP_SDK_PROFILE_OPTIC,
    XP_SDK_PROFILE_5M_DAC,
    XP_SDK_PROFILE_3M_DAC,
    XP_SDK_PROFILE_1M_DAC,
    XP_SDK_PROFILE_MAX,
} xpSdkProfile_t;

/*
 * currentTargetPlatform or device type strings
 * this array is tightly coupled with xpSdkDevType_t enum
 */
/* Compilation fix for "error: lambda expressions only available with -std=c++0x or -std=gnu++0x "
 * Destined initialization support is available in g++ from 4.7 above but it doesn't work in 4.5 g++ version
 * Hence removing the destined init, and make sure enum index co-relates with name
 */
static const char *xpSdkDevTypeName[XP_SDK_DEV_TYPE_UNKNOWN + 1] =
{
    /* [XP_SDK_DEV_TYPE_WM]        =*/ "wm",
    /* [XP_SDK_DEV_TYPE_SHADOW]    =*/ "shadow",
    /* [XP_SDK_DEV_TYPE_SVB]       =*/ "svb",
    /* [XP_SDK_DEV_TYPE_CRB1_REV1] =*/ "crb1-rev1",
    /* [XP_SDK_DEV_TYPE_CRB1_REV2] =*/ "crb1-rev2",
    /* [XP_SDK_DEV_TYPE_CRB2]      =*/ "crb2",
    /* [XP_SDK_DEV_TYPE_CRB2_REV2] =*/ "crb2-rev2",
    /* [XP_SDK_DEV_TYPE_CRB2_REV3] =*/ "crb2-rev3",
    /* [XP_SDK_DEV_TYPE_CRB3_REV1] =*/ "crb3-rev1",
    /* [XP_SDK_DEV_TYPE_CRB3_REV2] =*/ "crb3-rev2",
    /* [XP_SDK_DEV_TYPE_CRB1_XP70_REV1] =*/ "crb1-xp70-rev1",
    /* [XP_SDK_DEV_TYPE_CRB1_XP70_REV2] =*/ "crb1-xp70-rev2",
    /* [XP_SDK_DEV_TYPE_ET6448M] =*/ "et6448m",
    /* [XP_SDK_DEV_TYPE_ALDRIN_DB] =*/ "ald-db",
    /* [XP_SDK_DEV_TYPE_ALDRIN_DB] =*/ "CYGNUS",
    /* [XP_SDK_DEV_TYPE_ALDRIN2_XL] =*/ "ALDRIN2XL",
    /* [XP_SDK_DEV_TYPE_TG48M_P] =*/ "TG48MP",
    /* [XP_SDK_DEV_TYPE_FALCON32] =*/ "FALCON32",
    /* [XP_SDK_DEV_TYPE_FALCON_32X400G] =*/ "FALCON32x400G",
    /* [XP_SDK_DEV_TYPE_FALCON_32X100G_8X400G] =*/ "FC32x100G8x400G",
    /* [XP_SDK_DEV_TYPE_FALCON_24X25G_6X100G_3_2] =*/ "FALCONEBOF",
    /* [XP_SDK_DEV_TYPE_FALCON_24X25G_8X100G] =*/ "FC24x25G8x100G",
    /* [XP_SDK_DEV_TYPE_FALCON_24X25G_8X100G_6_4] =*/ "FC24x258x100G64",
    /* [XP_SDK_DEV_TYPE_FALCON_24X25G_4X200G] =*/ "FC24x25G8x200G",
    /* [XP_SDK_DEV_TYPE_FALCON_24X25G_4X200G_6_4] =*/ "FC24x254x200G64",
    /* [XP_SDK_DEV_TYPE_FALCON24X100G_4X400G] =*/ "FC24x100G4x400G",
    /* [XP_SDK_DEV_TYPE_FALCON16x100G_8x400G] =*/ "FC16x100G8x400G",
    /* [XP_SDK_DEV_TYPE_FALCON48X100G_8X400G] =*/ "FC48x100G8x400G",
    /* [XP_SDK_DEV_TYPE_FALCON_128X10G] = */ "FALCON128x10G",
    /* [XP_SDK_DEV_TYPE_FALCON_128X25G] = */ "FALCON128x25G",
    /* [XP_SDK_DEV_TYPE_FALCON128X50G] =*/ "FALCON128x50G",
    /* [XP_SDK_DEV_TYPE_FALCON128X100G] =*/ "FALCON128x100G",
    /* [XP_SDK_DEV_TYPE_FALCON128] =*/ "FALCON128",
    /* [XP_SDK_DEV_TYPE_FALCON32X25G] =*/ "FALCON32X25G",
    /* [XP_SDK_DEV_TYPE_FALCON32X25G_6_4] =*/ "FALCON32x25G64",
    /* [XP_SDK_DEV_TYPE_FALCON_64X100G] =*/ "FALCON64x100G",
    /* [XP_SDK_DEV_TYPE_FALCON_64X100G_R4] =*/ "FALCON64x100GR4",
    /* [XP_SDK_DEV_TYPE_FALCON_64X25G] =*/ "FALCON64x25G",
    /* [XP_SDK_DEV_TYPE_FALCON16X25G] =*/ "FALCON16x400G",
    /* [XP_SDK_DEV_TYPE_FALCON16X25G] =*/ "FALCON16X25G",
    /* [XP_SDK_DEV_TYPE_FALCON_32X25G_IXIA] =*/ "FC32x25GIXIA",
    /* [XP_SDK_DEV_TYPE_FALCON_64X100G_R4_IXIA] =*/ "FC64x100GR4IXIA",
    /* [XP_SDK_DEV_TYPE_FALCON_32X100G_R4_IXIA_6_4] =*/ "FC32x100GR4IXIA",
    /* [XP_SDK_DEV_TYPE_FALCON32X100G_16X400G] =*/ "FC32x10016x400G",
    /* [XP_SDK_DEV_TYPE_FALCON96X100G_8X400G] =*/ "FC96x100G8x400G",
    /* [XP_SDK_DEV_TYPE_FALCON_16X25G_IXIA] =*/ "FC16x25GIXIA",
    /* [XP_SDK_DEV_TYPE_FALCON_48x10G_8x100G] =*/ "FC48x10G8x100G",
    /* [XP_SDK_DEV_TYPE_FALCON_48x25G_8x100G] =*/ "FC48x25G8x100G",
    /* [XP_SDK_DEV_TYPE_FALCON_64x25G_64x10G] =*/ "FC64x25G64x10G",
    /* [XP_SDK_DEV_TYPE_FALCON_32x25G_8x100G] =*/ "FC32x25G8x100G",
    /* [XP_SDK_DEV_TYPE_FALCON_F2T80x25G] =*/ "F2T80x25G",
    /* [XP_SDK_DEV_TYPE_FALCON_F2T80x25GIXIA] =*/ "F2T80x25GIXIA",
    /* [XP_SDK_DEV_TYPE_FALCON_F2T48x25G8x100G] =*/ "F2T48x25G8x100G",
    /* [XP_SDK_DEV_TYPE_FALCON_F2T48x10G8x100G] =*/ "F2T48x10G8x100G",
    /* [XP_SDK_DEV_TYPE_AC5XRD] =*/ "AC5XRD",
    /* [XP_SDK_DEV_TYPE_AC5XRD_48x1G6x10G] =*/ "AC5X48x1G6x10G",
    /* [XP_SDK_DEV_TYPE_ALDRIN2_XL_FL] =*/ "ALDRIN2XLFL",
    /* [XP_SDK_DEV_TYPE_AC3X_FS] =*/ "AC3XFS",
    /* [XP_SDK_DEV_TYPE_UNKNOWN]   =*/ "\0"
};

static inline xpSdkDevType_t xpPlatformGetBoardType(const char *targetPlatform)
{
    int idx;
    XP_STATUS status = XP_NO_ERR;

    for (idx=XP_SDK_DEV_TYPE_WM; idx<XP_SDK_DEV_TYPE_UNKNOWN; idx++)
    {
        if (!strcmp(targetPlatform, xpSdkDevTypeName[idx]))
        {
            return (xpSdkDevType_t) idx;
        }
    }
    /* QUIRKS */
    /* 1. Backward compatibility for crb1
     *    crb1 is same as crb1-rev1
     */
#define CRB1REV1_ALT_DEV_NAME   "crb1"
    if (!strcmp(targetPlatform, CRB1REV1_ALT_DEV_NAME))
    {
        return XP_SDK_DEV_TYPE_CRB1_REV1;
    }

#define AS7512_ALT_DEV_NAME     "as7512"
    if (!strcmp(targetPlatform, AS7512_ALT_DEV_NAME))
    {
        XP_DEV_TYPE_T devType = XP80;
        if ((status = xpSalGetDeviceType(0u, &devType)) != XP_NO_ERR)
        {
            return XP_SDK_DEV_TYPE_UNKNOWN;
        }
        if (devType == XP80)
        {
            return XP_SDK_DEV_TYPE_CRB1_REV1;
        }
        else
        {
            return XP_SDK_DEV_TYPE_CRB1_REV2;
        }
    }

    return XP_SDK_DEV_TYPE_UNKNOWN;
}

#ifdef __cplusplus
}
#endif

#endif /* _xpDevTypes_h_ */
