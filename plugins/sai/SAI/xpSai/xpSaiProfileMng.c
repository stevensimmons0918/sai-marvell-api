// xpSaiProfileMng.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/*******************************************************************************
* @file xpSaiProfileMng.c
*
* @brief Private API implementation for CPSS ACL feature which can be used in
*        XPS layer.
*
* @version   01
*******************************************************************************/

#include "cpssDxChPortTx.h"
#include "openXpsTypes.h"
#include "saitypes.h"
#include "xpSaiLog.h"
#include "cpssHalQos.h"
#include "xpSaiProfileMng.h"
#include "xpSaiWred.h"

XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

static GT_U32 gProfileCapacity[CPSS_PORT_TX_DROP_PROFILE_16_E+1];

typedef struct
{
    GT_U32    enumVal;
    const GT_CHAR * strVal;
} xpSaiProfileMngEnumToStr;

const GT_CHAR * xpSaiProfileMngAlfaToStr
(
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alfa
)
{
    sai_uint32_t i;
    static xpSaiProfileMngEnumToStr alfaToStr [] =
    {
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E),
        _STR_VAL(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E)
    };

    for (i=0; i<sizeof(alfaToStr)/sizeof(alfaToStr[0]); i++)
    {
        if (alfaToStr[i].enumVal == alfa)
        {
            return alfaToStr[i].strVal;
        }
    }

    return "Unknown enum";

}

static void xpSaiProfileMngRequiredAttributesDump
(
    IN xpSaiProfileMngPortAttr_t         *pCurrentParams
)
{
    GT_U32 i;
    /*Get profile params for all the queues*/

    XP_SAI_LOG_DBG("Try to update HW with below :\n");

    for (i=0; i<PROFILE_MNG_MAX_TC; i++)
    {
        XP_SAI_LOG_DBG("  [tc %d dp0 alfa %s GRNT %d Wred_size 0x%x probability %d ecn %d]\n",
                       i,
                       xpSaiProfileMngAlfaToStr(pCurrentParams->queueTdProfileParams[i].dp0QueueAlpha),
                       pCurrentParams->queueWredProfileParams[i].dp0WredAttributes.guaranteedLimit,
                       pCurrentParams->queueWredProfileParams[i].dp0WredAttributes.wredSize,
                       pCurrentParams->queueWredProfileParams[i].dp0WredAttributes.probability,
                       pCurrentParams->queueEcnProfileParams[i].dp0EcnAttributes.ecnEnable);

        XP_SAI_LOG_DBG("  [tc %d dp1 alfa %s GRNT %d Wred_size 0x%x probability %d ecn %d]\n",
                       i,
                       xpSaiProfileMngAlfaToStr(pCurrentParams->queueTdProfileParams[i].dp1QueueAlpha),
                       pCurrentParams->queueWredProfileParams[i].dp1WredAttributes.guaranteedLimit,
                       pCurrentParams->queueWredProfileParams[i].dp1WredAttributes.wredSize,
                       pCurrentParams->queueWredProfileParams[i].dp1WredAttributes.probability,
                       pCurrentParams->queueEcnProfileParams[i].dp1EcnAttributes.ecnEnable);

        XP_SAI_LOG_DBG("  [tc %d dp2 alfa %s GRNT %d Wred_size 0x%x probability %d ecn %d]\n",
                       i,
                       xpSaiProfileMngAlfaToStr(pCurrentParams->queueTdProfileParams[i].dp2QueueAlpha),
                       pCurrentParams->queueWredProfileParams[i].dp2WredAttributes.guaranteedLimit,
                       pCurrentParams->queueWredProfileParams[i].dp2WredAttributes.wredSize,
                       pCurrentParams->queueWredProfileParams[i].dp2WredAttributes.probability,
                       pCurrentParams->queueEcnProfileParams[i].dp2EcnAttributes.ecnEnable);
    }

    XP_SAI_LOG_DBG("  [port alfa %s]\n",
                   xpSaiProfileMngAlfaToStr(pCurrentParams->portAlpha));
}

GT_VOID xpSaiProfileMngUpdateCapacity
(
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profile,
    GT_BOOL                           add
)
{
    if (add)
    {
        gProfileCapacity[profile]++;
    }
    else
    {
        gProfileCapacity[profile]--;
    }

    XP_SAI_LOG_DBG(" Profile %d has %d entries\n", profile,
                   gProfileCapacity[profile]);
}

GT_VOID xpSaiProfileMngAddToProfile
(
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profile
)
{
    xpSaiProfileMngUpdateCapacity(profile, GT_TRUE);
}

GT_VOID xpSaiProfileMngSubstructFromProfile
(
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profile
)
{
    xpSaiProfileMngUpdateCapacity(profile, GT_FALSE);
}

XP_STATUS xpSaiProfileMngSyncPortToProfile
(
    xpsDevice_t  xpsDevId,
    sai_uint32_t portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileInd;

    rc = cpssHalPortTxBindPortToDpGet(xpsDevId, portNum, &profileInd);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", portNum,
                       rc);
        return XP_ERR_FAILED;
    }

    xpSaiProfileMngAddToProfile(profileInd);

    XP_SAI_LOG_DBG("Sync port %d to profile %d\n", portNum, profileInd);

    return XP_NO_ERR;
}

XP_STATUS xpSaiProfileMngFindFreeProfile
(
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT * freeProfilePtr
)
{
    GT_U32 i;

    XP_SAI_LOG_DBG("Start looking for  free profile\n");

    for (i = CPSS_PORT_TX_DROP_PROFILE_1_E;
         i<=(GT_U32)CPSS_PORT_TX_DROP_PROFILE_16_E; i++)
    {
        if (0==gProfileCapacity[i])
        {
            *freeProfilePtr=(CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i;
            XP_SAI_LOG_DBG("Found free profile %d\n", *freeProfilePtr);
            return XP_NO_ERR;
        }
    }

    return XP_ERR_OUT_OF_IDS;
}

XP_STATUS xpSaiProfileMngFindCorrespondingProfile
(
    xpsDevice_t  xpsDevId,
    /*in*/xpSaiProfileMngPortAttr_t * requiredAttributesPtr,
    /*out*/CPSS_PORT_TX_DROP_PROFILE_SET_ENT               *foundProfilePtr
)
{
    GT_U32 i, j;
    GT_STATUS rc;
    CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC tmpWred;
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC         tmpTd;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT            tmpPortAlfa;
    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC
    tmpPortTailDropWredProfileParams;
    CPSS_PORT_QUEUE_ECN_PARAMS_STC                  tmpEcn;

    for (i=CPSS_PORT_TX_DROP_PROFILE_1_E; i<=(GT_U32)CPSS_PORT_TX_DROP_PROFILE_16_E;
         i++)
    {
        for (j=0; j<PROFILE_MNG_MAX_TC; j++)
        {
            /*1.Get guaranteed limit*/
            rc = cpssHalPortTx4TcTailDropWredProfileGet(xpsDevId,
                                                        (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i, j, &tmpWred);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Failed to get  profile %d tc %d parameters [rc %d])\n", i, j,
                               rc);
                return XP_ERR_FAILED;
            }

            /*2.Get alfa*/
            rc = cpssHalPortTx4TcTailDropProfileGet(xpsDevId,
                                                    (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i, j, &tmpTd);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Failed to get  profile %d tc %d parameters [rc %d])\n", i, j,
                               rc);
                return XP_ERR_FAILED;
            }

            /*Get ECN*/
            rc = cpssHalPortTx4TcTailDropEcnMarkingProfileGet(xpsDevId,
                                                              (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i, j, &tmpEcn);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Failed to get  profile %d tc %d parameters [rc %d])\n", i, j,
                               rc);
                return XP_ERR_FAILED;
            }

            /*3.Compare*/

            if (0 != memcmp(&(requiredAttributesPtr->queueWredProfileParams[j]),
                            &tmpWred, sizeof(CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC)))
            {
                /*This is not the one*/
                j=0;
                break;
            }

            if (requiredAttributesPtr->queueTdProfileParams[j].dp0QueueAlpha !=
                tmpTd.dp0QueueAlpha)
            {
                /*This is not the one*/
                j=0;
                break;
            }

            if (requiredAttributesPtr->queueEcnProfileParams[j].dp0EcnAttributes.ecnEnable
                != tmpEcn.dp0EcnAttributes.ecnEnable
                ||
                requiredAttributesPtr->queueEcnProfileParams[j].dp1EcnAttributes.ecnEnable!=
                tmpEcn.dp1EcnAttributes.ecnEnable
                ||
                requiredAttributesPtr->queueEcnProfileParams[j].dp2EcnAttributes.ecnEnable!=
                tmpEcn.dp2EcnAttributes.ecnEnable)
            {
                /*This is not the one*/
                j=0;
                break;
            }


        }

        if (PROFILE_MNG_MAX_TC==j)
        {
            /*This is  the one from queue point of view*/
            /*Now check the port*/

            /*1.Get guaranteed limit*/
            rc = cpssHalPortTxTailDropWredProfileGet(xpsDevId,
                                                     (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i, &tmpPortTailDropWredProfileParams);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Failed to get  wred drop profile of port [rc %d])\n", rc);
                return XP_ERR_FAILED;
            }
            /*2.Get alfa*/
            rc = cpssHalPortTxTailDropProfileGet(xpsDevId,
                                                 (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i, &tmpPortAlfa);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", rc);
                return XP_ERR_FAILED;
            }
            /*3.Compare*/

            if (tmpPortAlfa==requiredAttributesPtr->portAlpha)
            {
                /*That is the one*/
                if (tmpPortTailDropWredProfileParams.guaranteedLimit ==
                    requiredAttributesPtr->portWredProfileParams.guaranteedLimit)
                {
                    *foundProfilePtr = (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)i;
                    break;
                }
            }

        }
    }

    if (i>CPSS_PORT_TX_DROP_PROFILE_16_E)
    {
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

static XP_STATUS xpSaiProfileMngUpdateHwProfile
(IN  xpsDevice_t                       devId,
 OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT newProfileInd,
 OUT xpSaiProfileMngPortAttr_t         *pNewParams
)
{
    GT_STATUS rc;
    GT_U32 i;

    xpSaiProfileMngRequiredAttributesDump(pNewParams);

    /*update HW with new profile*/
    for (i=0; i<PROFILE_MNG_MAX_TC; i++)
    {
        /*TBD -cpss hal*/
        rc = cpssDxChPortTx4TcTailDropWredProfileSet(devId, newProfileInd, i,
                                                     &(pNewParams->queueWredProfileParams[i]));
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to set  tail drop profile [%d])\n", rc);
            return XP_ERR_FAILED;
        }

        rc = cpssHalPortTx4TcTailDropProfileSet(devId, newProfileInd, i,
                                                &(pNewParams->queueTdProfileParams[i]));
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to set  tail drop profile  [rc %d])\n", rc);
            return XP_ERR_FAILED;
        }

        rc = cpssHalPortTx4TcTailDropEcnMarkingProfileSet(devId, newProfileInd, i,
                                                          &(pNewParams->queueEcnProfileParams[i]));
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to set  tail drop profile of port [rc %d])\n", rc);
            return XP_ERR_FAILED;
        }
    }

    /*update port level*/
    rc = cpssHalPortTxTailDropProfileSet(devId, newProfileInd,
                                         pNewParams->portAlpha);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to set  tail drop profile of port [rc %d])\n", rc);
        return XP_ERR_FAILED;
    }

    rc = cpssHalPortTxTailDropWredProfileSet(devId, newProfileInd,
                                             &(pNewParams->portWredProfileParams));
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to set  wred drop profile of port [rc %d])\n", rc);
        return XP_ERR_FAILED;
    }

    return XP_NO_ERR;
}



static XP_STATUS xpSaiProfileMngPortAttributesGet
(IN  xpsDevice_t                       devId,
 IN  xpsPort_t                         devPort,
 OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT *pCurrentProfileInd,
 OUT xpSaiProfileMngPortAttr_t         *pCurrentParams
)
{
    GT_STATUS rc;
    GT_U32 i;

    /*Get the profile*/
    rc = cpssHalPortTxBindPortToDpGet(devId, devPort, pCurrentProfileInd);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", devPort,
                       rc);
        return XP_ERR_FAILED;
    }

    XP_SAI_LOG_DBG(" Port %d belong to profile %d \n", devPort,
                   *pCurrentProfileInd);


    /*Get profile params for all the queues*/

    for (i=0; i<PROFILE_MNG_MAX_TC; i++)
    {
        rc = cpssHalPortTx4TcTailDropWredProfileGet(devId, *pCurrentProfileInd, i,
                                                    &(pCurrentParams->queueWredProfileParams[i]));
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get  wred profile of port [rc %d])\n", devPort, rc);
            return XP_ERR_FAILED;
        }

        rc = cpssHalPortTx4TcTailDropProfileGet(devId, *pCurrentProfileInd, i,
                                                &(pCurrentParams->queueTdProfileParams[i]));
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", devPort,
                           rc);
            return XP_ERR_FAILED;
        }

        rc = cpssHalPortTx4TcTailDropEcnMarkingProfileGet(devId, *pCurrentProfileInd, i,
                                                          &(pCurrentParams->queueEcnProfileParams[i]));
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", devPort,
                           rc);
            return XP_ERR_FAILED;
        }

        XP_SAI_LOG_DBG("  [tc %d dp0 alfa %s ecn %d]\n", i,
                       xpSaiProfileMngAlfaToStr(pCurrentParams->queueTdProfileParams[i].dp0QueueAlpha),
                       pCurrentParams->queueEcnProfileParams[i].dp0EcnAttributes.ecnEnable);
        XP_SAI_LOG_DBG("  [tc %d dp1 alfa %s ecn %d]\n", i,
                       xpSaiProfileMngAlfaToStr(pCurrentParams->queueTdProfileParams[i].dp1QueueAlpha),
                       pCurrentParams->queueEcnProfileParams[i].dp1EcnAttributes.ecnEnable);
        XP_SAI_LOG_DBG("  [tc %d dp2 alfa %s ecn %d]\n", i,
                       xpSaiProfileMngAlfaToStr(pCurrentParams->queueTdProfileParams[i].dp2QueueAlpha),
                       pCurrentParams->queueEcnProfileParams[i].dp2EcnAttributes.ecnEnable);
    }

    /*Get profile params for port level*/
    rc = cpssHalPortTxTailDropWredProfileGet(devId, *pCurrentProfileInd,
                                             &(pCurrentParams->portWredProfileParams));
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to get  wred drop profile of port [rc %d])\n", devPort,
                       rc);
        return XP_ERR_FAILED;
    }
    rc = cpssHalPortTxTailDropProfileGet(devId, *pCurrentProfileInd,
                                         &(pCurrentParams->portAlpha));
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", devPort,
                       rc);
        return XP_ERR_FAILED;
    }


    XP_SAI_LOG_DBG("  [port alfa %s]\n",
                   xpSaiProfileMngAlfaToStr(pCurrentParams->portAlpha));

    return XP_NO_ERR;
}


XP_STATUS xpSaiProfileMngConfigureGuaranteedLimitAndAlfa
(xpsDevice_t devId,
 xpsPort_t devPort,
 uint32_t queueNum,
 uint32_t *lengthMaxThresholdPtr,
 CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT *alphaPtr
)
{
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileInd, newProfileInd;
    GT_STATUS rc;
    XP_STATUS st;
    XP_STATUS newProfStatus;
    static xpSaiProfileMngPortAttr_t requiredTdProfile;


    st = xpSaiProfileMngPortAttributesGet(devId, devPort, &profileInd,
                                          &requiredTdProfile);
    if (st!=XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiProfileMngPortAttributesGet failed |retVal: %d\n", st);
        return st;
    }


    /*Now update required queue*/
    if (lengthMaxThresholdPtr)
    {
        if (queueNum!=PROFILE_MNG_PER_PORT_CFG)
        {
            requiredTdProfile.queueWredProfileParams[queueNum].dp0WredAttributes.guaranteedLimit
                =*lengthMaxThresholdPtr;
            requiredTdProfile.queueWredProfileParams[queueNum].dp1WredAttributes.guaranteedLimit
                =*lengthMaxThresholdPtr;
            requiredTdProfile.queueWredProfileParams[queueNum].dp2WredAttributes.guaranteedLimit
                =*lengthMaxThresholdPtr;
            XP_SAI_LOG_DBG("Try to change Q %d garanteed to %d \n", queueNum,
                           *lengthMaxThresholdPtr);
        }
        else
        {
            requiredTdProfile.portWredProfileParams.guaranteedLimit =
                *lengthMaxThresholdPtr;
            XP_SAI_LOG_DBG("Try to change Port garanteed to %d \n", *lengthMaxThresholdPtr);
        }
    }

    if (alphaPtr)
    {
        if (queueNum!=PROFILE_MNG_PER_PORT_CFG)
        {
            requiredTdProfile.queueTdProfileParams[queueNum].dp0QueueAlpha = *alphaPtr;
            requiredTdProfile.queueTdProfileParams[queueNum].dp1QueueAlpha = *alphaPtr;
            requiredTdProfile.queueTdProfileParams[queueNum].dp2QueueAlpha = *alphaPtr;
            XP_SAI_LOG_DBG("Try to change Q %d alfa %s \n", queueNum,
                           xpSaiProfileMngAlfaToStr(*alphaPtr));
        }
        else
        {
            requiredTdProfile.portAlpha= *alphaPtr;
            XP_SAI_LOG_DBG("Try to change Port alfa to %s \n",
                           xpSaiProfileMngAlfaToStr(*alphaPtr));
        }
    }

    newProfStatus = xpSaiProfileMngFindCorrespondingProfile(devId,
                                                            &requiredTdProfile, &newProfileInd);

    if (newProfStatus == XP_ERR_NOT_FOUND)
    {
        XP_SAI_LOG_DBG("Corresponding profile is not found.Try to create new one \n");

        xpSaiProfileMngSubstructFromProfile(profileInd);
        st = xpSaiProfileMngFindFreeProfile(&newProfileInd);
        if (st==XP_NO_ERR)
        {
            xpSaiProfileMngAddToProfile(newProfileInd);

            /*update HW with new profile*/
            XP_SAI_LOG_DBG("Update HW TD profile %d\n", newProfileInd);
            st = xpSaiProfileMngUpdateHwProfile(devId, newProfileInd, &requiredTdProfile);
            if (st!=XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpSaiProfileMngPortAttributesGet failed |retVal: %d\n", st);
                return st;
            }
        }
        else
        {
            XP_SAI_LOG_ERR("no free profile is found \n");
            return XP_ERR_FAILED;
        }
    }
    else if (newProfStatus == XP_ERR_FAILED)
    {
        XP_SAI_LOG_ERR("Error in finding free profile \n");
        return XP_ERR_FAILED;
    }

    if (newProfileInd!=profileInd)
    {
        XP_SAI_LOG_DBG("Profile updated: P %d , old td profile %d -> new td profile %d \n",
                       devPort, profileInd, newProfileInd);

        if (newProfStatus != XP_ERR_NOT_FOUND)
        {
            xpSaiProfileMngSubstructFromProfile(profileInd);
            xpSaiProfileMngAddToProfile(newProfileInd);
        }

        rc = cpssHalPortTxBindPortToDpSet(devId, devPort, newProfileInd);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", devPort,
                           rc);
            return XP_ERR_FAILED;
        }
    }
    else
    {
        XP_SAI_LOG_DBG("Found  profile %d that fit \n", newProfileInd);
    }


    return XP_NO_ERR;
}



XP_STATUS xpSaiProfileMngConfigureWredAndEcn
(IN xpsDevice_t devId,
 IN xpsPort_t devPort,
 IN uint32_t queueNum,
 IN xpSaiWred_t *pWredInfo
)
{
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileInd, newProfileInd;
    XP_STATUS st;
    XP_STATUS newProfStatus;
    static xpSaiProfileMngPortAttr_t requiredEcnProfile;
    GT_STATUS   rc;

    st = xpSaiProfileMngPortAttributesGet(devId, devPort, &profileInd,
                                          &requiredEcnProfile);
    if (st!=XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiProfileMngPortAttributesGet failed |retVal: %d\n", st);
        return st;
    }

    /*convert to CPSS format*/
    if ((st = xpSaiWredSai2CpssProfileGet(pWredInfo,
                                          &(requiredEcnProfile.queueWredProfileParams[queueNum]),
                                          &(requiredEcnProfile.queueEcnProfileParams[queueNum]))) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to convert  sai to cpss profile |retVal: %d\n", st);
        return st;
    }

    /*TBD  Tail Drop vs WRED minimum select*/

    newProfStatus = xpSaiProfileMngFindCorrespondingProfile(devId,
                                                            &requiredEcnProfile, &newProfileInd);

    if (newProfStatus == XP_ERR_NOT_FOUND)
    {
        XP_SAI_LOG_DBG("Corresponding profile is not found.Try to create new one \n");

        xpSaiProfileMngSubstructFromProfile(profileInd);
        st = xpSaiProfileMngFindFreeProfile(&newProfileInd);
        if (st==XP_NO_ERR)
        {
            xpSaiProfileMngAddToProfile(newProfileInd);

            /*update HW with new profile*/
            st = xpSaiProfileMngUpdateHwProfile(devId, newProfileInd, &requiredEcnProfile);
            if (st!=XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpSaiProfileMngPortAttributesGet failed |retVal: %d\n", st);
                return st;
            }
        }
        else
        {
            XP_SAI_LOG_ERR("no free profile is found \n");
            return XP_ERR_FAILED;
        }
    }
    else if (newProfStatus == XP_ERR_FAILED)
    {
        XP_SAI_LOG_ERR("Error in finding free profile \n");
        return newProfStatus;
    }

    if (newProfileInd!=profileInd)
    {
        XP_SAI_LOG_DBG("Profile updated: P %d , old td profile %d -> new td profile %d \n",
                       devPort, profileInd, newProfileInd);

        /*need to add to new profile*/
        if (newProfStatus != XP_ERR_NOT_FOUND)
        {
            xpSaiProfileMngSubstructFromProfile(profileInd);
            xpSaiProfileMngAddToProfile(newProfileInd);
        }

        rc = cpssHalPortTxBindPortToDpSet(devId, devPort, newProfileInd);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get  tail drop profile of port [rc %d])\n", devPort,
                           rc);
            return XP_ERR_FAILED;
        }
    }
    else
    {
        XP_SAI_LOG_DBG("Found  profile %d that fit \n", newProfileInd);
    }

    return XP_NO_ERR;
}
