/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalTcam.h
*
* @brief Internal header which defines API for helpers functions, which are
*        specific for XPS TCAM.
*
* @version   01
*******************************************************************************/

#ifndef _cpssHalTcam_h_
#define _cpssHalTcam_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#define CPSS_TCAM_CLIENT_GROUP0   0
#define CPSS_TCAM_CLIENT_GROUP1   1
#define CPSS_TCAM_CLIENT_GROUP2   2
#define CPSS_TCAM_CLIENT_GROUP3   3
#define CPSS_TCAM_CLIENT_GROUP4   4

#define CPSS_TCAM_AC3X    0
#define CPSS_TCAM_ALD     1
#define CPSS_TCAM_ALD2XL  2
#define CPSS_TCAM_BOBCAT2 3
#define CPSS_TCAM_FALCON  4
#define CPSS_TCAM_AC5X    5

#define CPSS_TCAM_DEVICE_ARRAY_SIZE    6

GT_STATUS cpssHalVtcamMgrCreate
(
    IN  GT_U32  devId,
    IN  GT_U32  vTcamMgrId
);

GT_STATUS cpssHalVtcamMgrDelete
(
    IN  GT_U32  devId,
    IN  GT_U32  vTcamMgrId
);

GT_STATUS cpssHalVtcamCreate
(
    IN  GT_U32                           vTcamMgrId,
    IN  GT_U32                           vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC *vTcamInfo
);

GT_STATUS cpssHalVtcamRemove
(
    IN  GT_U32 vTcamMgrId,
    IN  GT_U32 vTcamId
);

GT_STATUS cpssHalTcamAclClientGroupSet
(
    IN GT_U32 devId
);

GT_STATUS cpssHalTcamTtiClientGroupSet
(
    IN GT_U32 devId
);

GT_STATUS cpssHalVirtualTcamResize
(
    IN  GT_U32  vTcamMngId,
    IN  GT_U32  vTcamId,
    IN  GT_U32  rulePlace,
    IN  GT_BOOL toInsert,
    IN  GT_U32  sizeInRules
);

GT_STATUS cpssHalTcamAclClientGroupIdGet
(
    IN GT_U32 devId,
    IN CPSS_DXCH_TCAM_CLIENT_ENT client,
    OUT GT_U32 *groupId
);

#ifdef __cplusplus
}
#endif

#endif //_cpssHalTcam_h_
