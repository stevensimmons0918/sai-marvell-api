/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalTcam.c
*
* @brief Private API implementation for CPSS TCAM feature which can be used in
*        XPS layer.
*
* @version   01
*******************************************************************************/

#include "cpssHalDevice.h"
#include "cpssHalTcam.h"

/* NOTE: Below mapping is based on FS recommendation. We have deviated from recommendation on
   clients using group-id. When 2 clients share same group (sau IPCL0,IPCL1), We observe that 2 look-up passes
   one of IPCL0 and IPCL1 for same ACL rule and counter increments twice. Looks like for such scenarios,
   PCL-id has to be different in look-up stages. Since PCL-id is not qualified at present,
   use existing mapping for AC3X,ALD adn BOBCAT2 devices. */
uint32_t cpssHalTcamClientGroupMap[CPSS_TCAM_DEVICE_ARRAY_SIZE][5]=
{
    /*IPCL0*/             /*IPCL1*/                 /*IPCL2*/               /*EPCL*/          /*TTI*/
    {CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP2}, //CPSS_TCAM_AC3X
    {CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP2}, //CPSS_TCAM_ALD
    {CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP2, CPSS_TCAM_CLIENT_GROUP3, CPSS_TCAM_CLIENT_GROUP4, CPSS_TCAM_CLIENT_GROUP0}, //CPSS_TCAM_ALD2XL
    {CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP2}, //CPSS_TCAM_BOBCAT2
    {CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP2, CPSS_TCAM_CLIENT_GROUP3, CPSS_TCAM_CLIENT_GROUP0}, //CPSS_TCAM_FALCON
    {CPSS_TCAM_CLIENT_GROUP0, CPSS_TCAM_CLIENT_GROUP1, CPSS_TCAM_CLIENT_GROUP2, CPSS_TCAM_CLIENT_GROUP2, CPSS_TCAM_CLIENT_GROUP0} //CPSS_TCAM_AC5X
};


GT_STATUS cpssHalVtcamMgrCreate
(
    IN  GT_U32  devId,
    IN  GT_U32  vTcamMgrId
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevId = 0;
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC vTcamMgrConfig = {GT_TRUE, CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_FAILED_RULES_ADD_E};

    if ((rc = cpssDxChVirtualTcamManagerCreate(vTcamMgrId, &vTcamMgrConfig)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChVirtualTcamManagerCreate failed with err : %d on device :%d\n ", rc,
              devId);
        return rc;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Add device to vTCAM manager */
        if ((rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMgrId, &cpssDevId,
                                                       1)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChVirtualTcamManagerDevListAdd failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalVtcamMgrDelete
(
    IN  GT_U32  devId,
    IN  GT_U32  vTcamMgrId
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Remove device from vTCAM manager */
        if ((rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMgrId, &cpssDevId,
                                                          1)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChVirtualTcamManagerDevListRemove failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
    }

    if ((rc = cpssDxChVirtualTcamManagerDelete(vTcamMgrId)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChVirtualTcamManagerDelete  failed with err : %d on device :%d\n ", rc,
              devId);
        return rc;
    }

    return GT_OK;
}

GT_STATUS cpssHalVtcamCreate
(
    IN  GT_U32                           vTcamMgrId,
    IN  GT_U32                           vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC *vTcamInfo
)
{
    GT_STATUS rc = GT_OK;

    if ((rc = cpssDxChVirtualTcamCreate(vTcamMgrId, vTcamId, vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChVirtualTcamCreate failed with err : %d  ", rc);
        return rc;
    }

    return rc;
}

GT_STATUS cpssHalVtcamRemove
(
    IN  GT_U32 vTcamMgrId,
    IN  GT_U32 vTcamId
)
{
    GT_STATUS rc = GT_OK;

    if ((rc = cpssDxChVirtualTcamRemove(vTcamMgrId, vTcamId)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChVirtualTcamRemove failed with err : %d on device :%d\n ", rc);
        return rc;
    }

    return rc;
}

GT_STATUS cpssHalTcamAclClientGroupSet
(
    IN GT_U32 devId
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;
    GT_U32    ipcl0ClientGrpId = 0xffffffff;
    GT_U32    ipcl1ClientGrpId = 0xffffffff;
    GT_U32    epclClientGrpId = 0xffffffff;
    GT_U32    portGroupsBmp = 0xffffffff;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devId);

    switch (devType)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            {
                ipcl0ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_FALCON][CPSS_DXCH_TCAM_IPCL_0_E];
                ipcl1ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_FALCON][CPSS_DXCH_TCAM_IPCL_1_E];
                epclClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_FALCON][CPSS_DXCH_TCAM_EPCL_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            {
                ipcl0ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD2XL][CPSS_DXCH_TCAM_IPCL_0_E];
                ipcl1ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD2XL][CPSS_DXCH_TCAM_IPCL_1_E];
                epclClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD2XL][CPSS_DXCH_TCAM_EPCL_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            {
                ipcl0ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD][CPSS_DXCH_TCAM_IPCL_0_E];
                ipcl1ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD][CPSS_DXCH_TCAM_IPCL_1_E];
                epclClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD][CPSS_DXCH_TCAM_EPCL_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            {
                ipcl0ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_BOBCAT2][CPSS_DXCH_TCAM_IPCL_0_E];
                ipcl1ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_BOBCAT2][CPSS_DXCH_TCAM_IPCL_1_E];
                epclClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_BOBCAT2][CPSS_DXCH_TCAM_EPCL_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            {
                ipcl0ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC3X][CPSS_DXCH_TCAM_IPCL_0_E];
                ipcl1ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC3X][CPSS_DXCH_TCAM_IPCL_1_E];
                epclClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC3X][CPSS_DXCH_TCAM_EPCL_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            {
                ipcl0ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC5X][CPSS_DXCH_TCAM_IPCL_0_E];
                ipcl1ClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC5X][CPSS_DXCH_TCAM_IPCL_1_E];
                epclClientGrpId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC5X][CPSS_DXCH_TCAM_EPCL_E];
            }
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc= cpssDxChTcamPortGroupClientGroupSet(cpssDevId, portGroupsBmp,
                                                     CPSS_DXCH_TCAM_IPCL_0_E,
                                                     ipcl0ClientGrpId, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChTcamPortGroupClientGroupSet failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
        if ((rc= cpssDxChTcamPortGroupClientGroupSet(cpssDevId, portGroupsBmp,
                                                     CPSS_DXCH_TCAM_EPCL_E,
                                                     epclClientGrpId, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChTcamPortGroupClientGroupSet failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
        if ((rc= cpssDxChTcamPortGroupClientGroupSet(cpssDevId, portGroupsBmp,
                                                     CPSS_DXCH_TCAM_IPCL_1_E,
                                                     ipcl1ClientGrpId, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChTcamPortGroupClientGroupSet failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
    }

    return rc;
}

GT_STATUS cpssHalTcamAclClientGroupIdGet
(
    IN GT_U32 devId,
    IN CPSS_DXCH_TCAM_CLIENT_ENT client,
    OUT GT_U32 *groupId
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devId);

    if (client > CPSS_DXCH_TCAM_TTI_E)
    {
        return GT_FAIL;
    }

    switch (devType)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            {
                *groupId = cpssHalTcamClientGroupMap[CPSS_TCAM_FALCON][client];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            {
                *groupId = cpssHalTcamClientGroupMap[CPSS_TCAM_ALD2XL][client];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            {
                *groupId = cpssHalTcamClientGroupMap[CPSS_TCAM_ALD][client];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            {
                *groupId = cpssHalTcamClientGroupMap[CPSS_TCAM_BOBCAT2][client];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            {
                *groupId = cpssHalTcamClientGroupMap[CPSS_TCAM_AC3X][client];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            {
                *groupId = cpssHalTcamClientGroupMap[CPSS_TCAM_AC5X][client];
            }
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    return rc;
}

GT_STATUS cpssHalTcamTtiClientGroupSet
(
    IN GT_U32 devId
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;
    GT_U32    ttiClientGroupId = 0xffffffff;
    GT_U32    portGroupsBmp = 0xffffffff;

    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devId);

    switch (devType)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            {
                ttiClientGroupId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_FALCON][CPSS_DXCH_TCAM_TTI_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            {
                ttiClientGroupId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD2XL][CPSS_DXCH_TCAM_TTI_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            {
                ttiClientGroupId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_ALD][CPSS_DXCH_TCAM_TTI_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            {
                ttiClientGroupId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_BOBCAT2][CPSS_DXCH_TCAM_TTI_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            {
                ttiClientGroupId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC3X][CPSS_DXCH_TCAM_TTI_E];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            {
                ttiClientGroupId =
                    cpssHalTcamClientGroupMap[CPSS_TCAM_AC5X][CPSS_DXCH_TCAM_TTI_E];
            }
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc= cpssDxChTcamPortGroupClientGroupSet(cpssDevId, portGroupsBmp,
                                                     CPSS_DXCH_TCAM_TTI_E, ttiClientGroupId, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChTcamPortGroupClientGroupSet failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
    }

    return rc;
}

GT_STATUS cpssHalVirtualTcamResize
(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules
)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChVirtualTcamResize(vTcamMngId, vTcamId, rulePlace, toInsert,
                                       sizeInRules);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Resize TCAM Table Entry, "
              "status:%d, MgrId:%d TableId:%d\n", status, vTcamMngId, vTcamId);
        return status;
    }

    return status;
}
