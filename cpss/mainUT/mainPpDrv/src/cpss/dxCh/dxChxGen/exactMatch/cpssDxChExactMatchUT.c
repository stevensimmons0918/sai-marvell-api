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
* @file cpssDxChExactMatchUT.c
*
* @brief Unit tests for cpssDxChExactMatch, that provides
* Exact Match CPSS Falcon implementation.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Defines */

/* Default valid value for port/trunk id */
#define TTI_VALID_PORT_TRUNK_CNS        20

/* Default valid value for vlan id */
#define TTI_VALID_VLAN_ID_CNS           100


extern GT_U32 utfFamilyTypeGet(IN GT_U8 devNum);
/* add additional macro to treat devices like Ironman that not hold dedicated
   family in UTF_PP_FAMILY_BIT_ENT , and uses the UTF_AC5X_E */
#define PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC                             \
    if(0 == PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.parametericTables.numEntriesEm) \
    {                                                                    \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,utfFamilyTypeGet(prvTgfDevNum));         \
    }


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiProfileIdModePacketTypeSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E /
                                     CPSS_DXCH_TTI_KEY_UDB_IPV6_E /
                                     CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           ttiLookupNum [first / second ]
                           and profileId [1,6,15]
    Expected: GT_OK.
    1.2. Call cpssDxChExactMatchTtiProfileIdModePacketTypeGet.
    Expected: GT_OK and the same profileId.
    1.3. Call with wrong ttiLookupNum [last]
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong profileId [16]
    Expected: GT_OUT_OF_RANGE.
    1.5. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6 Call cpssDxChExactMatchTtiProfileIdModePacketTypeSet with legacy keyType vlaue.
    Expected: GT_OK.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_BOOL                             enableExactMatchLookup;
    GT_BOOL                             enableExactMatchLookupGet;
    GT_U32                              profileId;
    GT_U32                              profileIdGet;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);


        /*
           1.1.1 Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E]
                                   ttiLookupNum [first] and profileId [1]
           Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 1;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.1 Call cpssDxChExactMatchTtiProfileIdModePacketTypeGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchTtiProfileIdModePacketTypeGet: %d, %d , %d", devNum, keyType,ttiLookupNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,
            "got other enableExactMatchLookupGet then was set: %d", devNum);
        if (enableExactMatchLookup==enableExactMatchLookupGet)
        {
            if(enableExactMatchLookupGet==GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
        }

        /*
           1.1.2 Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV6_E]
                                   ttiLookupNum [second] and profileId [6]
           Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 6;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup,profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup,profileId);
        /*
            1.2.2 Call cpssDxChExactMatchTtiProfileIdModePacketTypeGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchTtiProfileIdModePacketTypeGet: %d, %d , %d", devNum, keyType,ttiLookupNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
            "got other profileId then was set: %d", devNum);
        /*
           1.1.3 Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_UDE6_E]
                                   ttiLookupNum [first] and profileId [15]
           Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.3 Call cpssDxChExactMatchTtiProfileIdModePacketTypeGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchTtiProfileIdModePacketTypeGet: %d, %d , %d", devNum, keyType,ttiLookupNum);
       if (enableExactMatchLookup==enableExactMatchLookupGet)
        {
            if(enableExactMatchLookupGet==GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
        }
        /*
            1.3. Call with wrong ttiLookupNum [last]
            Expected: GT_BAD_PARAM.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        profileId   = 1;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyType, ttiLookupNum ,profileId);
        /*
            1.4. Call with wrong profileId [16]
            Expected: GT_OUT_OF_RANGE.
        */
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 16;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, keyType, ttiLookupNum ,profileId);

        /* Restore valid params. */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 2;
        enableExactMatchLookup = GT_TRUE;
        /*
            1.5. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, NULL);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, profileId = NULL",
                                     devNum, keyType,ttiLookupNum);
        /*
            1.6 Call cpssDxChExactMatchTtiProfileIdModePacketTypeSet with legacy keyType vlaue.
            Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum , &enableExactMatchLookupGet, profileIdGet);
    }


    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 1;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookupGet, profileIdGet);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchTtiProfileIdModePacketTypeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    OUT GT_U32                             *profileIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiProfileIdModePacketTypeGet)
{
/*
    ITERATE_DEVICES (Falcon)
    ITERATE_DEVICES (Falcon)
    1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E /
                                     CPSS_DXCH_TTI_KEY_UDB_IPV6_E /
                                     CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           ttiLookupNum [first / second ]
    Expected: GT_OK.
    1.2. Call cpssDxChExactMatchTtiProfileIdModePacketTypeGet.
    1.2. Call with out of range keyType and other valid params.
    Expected: NOT GT_OK.
    1.3. Call with out of range ttiLookupNum and other valid params.
    Expected: NOT GT_OK.
    1.4. Call with NULL profileIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL enableExactMatchLookup and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_U32                              profileId;
    GT_BOOL                             enableExactMatchLookup;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);

        /*
            1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E /
                                     CPSS_DXCH_TTI_KEY_UDB_IPV6_E /
                                     CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           ttiLookupNum [first / second ]
            Expected: GT_OK.
        */
        /* iterate with entryIndex = 0 */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
             1.2. Call with out of range keyType and other valid params.
            Expected: NOT GT_OK.
        */

        keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)20;

        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

         /*
           1.3. Call with out of range ttiLookupNum and other valid params.
            Expected: NOT GT_OK.
         */

        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;

        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.4. Call with NULL profileIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, profileIdPtr = NULL",
                                     devNum, keyType);

        /*
            1.5. Call with NULL enableExactMatchLookup and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,NULL, &profileId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enableExactMatchLookup = NULL",
                                     devNum, keyType);

     }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    keyType = CPSS_DXCH_TTI_KEY_UDB_UDE_E;
    ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiProfileIdModePacketTypeGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiPortProfileIdModeSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT   profileIdMode
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiPortProfileIdModeSet)
{
/*
    ITERATE_DEVICES PHY_PORTS (Falcon)
    1.1.1.  Call function with profileIdMode  [CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E /
                                               CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PORT_E /
                                               CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PORT_PACKET_TYPE_E ]

    Expected: GT_OK.
    1.1.2. Call cpssDxChExactMatchTtiPortProfileIdModeGet.
    Expected: GT_OK and the same profileIdMode.
    1.1.3. Call with wrong profileIdMode [last] and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT   profileIdMode;
    CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT   profileIdModeGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {

            for (profileIdMode=CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E;
                 profileIdMode<CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_LAST_E; profileIdMode++)
            {
                 /*
                    1.1.1. Call with profileIdMode [CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E,
                                                    CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PORT_E,
                                                    CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PORT_PACKET_TYPE_E]
                    Expected: GT_OK.
                */
                st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, profileIdMode);

                /*
                    1.1.2. Call cpssDxChExactMatchTtiPortProfileIdModeGet.
                    Expected: GT_OK and the same profileIdMode.
                */
                st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchTtiPortProfileIdModeGet: %d, %d", devNum, portNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(profileIdMode, profileIdModeGet,
                    "got another profileIdMode then was set: %d, %d", devNum, portNum);
            }

            /*
                1.1.3. Call with wrong profileIdMode [last]
                Expected: GT_BAD_PARAM.
            */
            profileIdMode=CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_LAST_E;
            st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, profileIdMode);

        }

        profileIdMode = CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PORT_TRUNK_CNS;
    profileIdMode = CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiPortProfileIdModeSet(devNum, portNum, profileIdMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiPortProfileIdModeGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT   *profileIdModePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiPortProfileIdModeGet)
{
/*
    ITERATE_DEVICES PHY_PORTS (Falcon)
    1.1.1.  Call function with valid parameters
    Expected: GT_OK.
    1.1.2. Call with NULL profileIdModePtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT   profileIdModePtr;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
                /*
                    1.1.1. Call function with valid parameters
                    Expected: GT_OK.
                */
                st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModePtr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, profileIdModePtr);


            /*
                1.1.2. Call with NULL profileIdModePtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, profileIdModePtr = NULL",devNum, portNum);

        }

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        profileIdModePtr = CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E;

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModePtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModePtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModePtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PORT_TRUNK_CNS;
    profileIdModePtr = CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiPortProfileIdModeGet(devNum, portNum, &profileIdModePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiPortProfileIdModePortSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            ttiLookupNum,
    IN  GT_BOOL                                     enableExactMatchLookup,
    IN  GT_U32                                      profileId
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiPortProfileIdModePortSet)
{
/*
    ITERATE_DEVICES PHY_PORTS (Falcon)
    1.1.1.  Call function with ttiLookupNum  [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                              CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                              and profileId [1]

    Expected: GT_OK.
    1.1.2. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
    Expected: GT_OK and the same profileId.
    1.1.3.  Call function with ttiLookupNum  [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                              CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                              and profileId [15]

    Expected: GT_OK.
    1.1.4. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
    Expected: GT_OK and the same profileId.
    1.1.5. Call with wrong ttiLookupNum [last] and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with wrong profileId [16] and other valid params.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_BOOL                             enableExactMatchLookup;
    GT_BOOL                             enableExactMatchLookupGet;
    GT_U32                              profileId;
    GT_U32                              profileIdGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {

            for (ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
                 ttiLookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E; ttiLookupNum++)
            {
                 /*
                    1.1.1. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                     and profileId [1]
                    Expected: GT_OK.
                */
                profileId   = 1;
                enableExactMatchLookup = GT_TRUE;
                st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);

                /*
                    1.1.2. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
                    Expected: GT_OK and the same profileId.
                */
                st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum, portNum, ttiLookupNum, &enableExactMatchLookupGet, &profileIdGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchTtiPortProfileIdModePortGet: %d, %d, %d", devNum, portNum, ttiLookupNum);
                UTF_VERIFY_EQUAL3_STRING_MAC(profileId, profileIdGet,
                    "got another profileIdMode then was set: %d, %d, %d", devNum, portNum, ttiLookupNum);
                if (enableExactMatchLookup==enableExactMatchLookupGet)
                {
                    if(enableExactMatchLookupGet==GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(profileId, profileIdGet,
                                                     "got other profileId then was set:  %d, %d, %d", devNum, portNum, ttiLookupNum);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(0, profileIdGet,
                                                     "got other profileId then was set:  %d, %d, %d", devNum, portNum, ttiLookupNum);
                    }
                }

                /*
                   1.1.3. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                  CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                    and profileId [15]
                   Expected: GT_OK.
               */
               profileId  = 15;
               enableExactMatchLookup = GT_TRUE;
               st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
               UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);

               /*
                   1.1.4. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
                   Expected: GT_OK and the same profileId.
               */
               st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum, portNum, ttiLookupNum, &enableExactMatchLookupGet, &profileIdGet);
               UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChExactMatchTtiPortProfileIdModePortGet: %d, %d, %d", devNum, portNum, ttiLookupNum);
               UTF_VERIFY_EQUAL3_STRING_MAC(profileId, profileIdGet,
                   "got another profileIdMode then was set: %d, %d, %d", devNum, portNum, ttiLookupNum);
               if (enableExactMatchLookup==enableExactMatchLookupGet)
               {
                   if(enableExactMatchLookupGet==GT_TRUE)
                   {
                       UTF_VERIFY_EQUAL3_STRING_MAC(profileId, profileIdGet,
                                                    "got other profileId then was set:  %d, %d, %d", devNum, portNum, ttiLookupNum);
                   }
                   else
                   {
                       UTF_VERIFY_EQUAL3_STRING_MAC(0, profileIdGet,
                                                    "got other profileId then was set:  %d, %d, %d", devNum, portNum, ttiLookupNum);
                   }
               }

            }

            /*
                1.1.5. Call with wrong ttiLookupNum [last]
                Expected: GT_BAD_PARAM.
            */
            ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
            profileId = 15;
            enableExactMatchLookup = GT_TRUE;
            st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);

            /*
                1.1.6. Call with wrong profileId [16]
                Expected: GT_OUT_OF_RANGE.
            */
            ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
            profileId  = 16;
            enableExactMatchLookup = GT_TRUE;
            st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);

        }

        /* Restore valid params. */
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId  = 15;
        enableExactMatchLookup = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PORT_TRUNK_CNS;
    ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
    profileId  = 15;
    enableExactMatchLookup = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiPortProfileIdModePortSet(devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum, enableExactMatchLookup, profileId);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiPortProfileIdModePortGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            ttiLookupNum,
    OUT GT_BOOL                                     *enableExactMatchLookupPtr,
    OUT GT_U32                                      *profileIdPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiPortProfileIdModePortGet)
{
/*
    ITERATE_DEVICES PHY_PORTS (Falcon)
    1.1.1. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
    Expected: GT_OK.
    1.1.2. Call with wrong ttiLookupNum [last] and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with NULL enableExactMatchLookupPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.4. Call with NULL profileIdPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_BOOL                             enableExactMatchLookupPtr;
    GT_U32                              profileIdPtr;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            for (ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
                 ttiLookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E; ttiLookupNum++)
            {
                 /*
                    1.1.1. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                    Expected: GT_OK.
                */
                st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, ttiLookupNum);
            }

            /*
                1.1.2. Call with wrong ttiLookupNum [last] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
            st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum);

            /*
                1.1.3. Call with NULL enableExactMatchLookupPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
            st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,NULL,&profileIdPtr);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enableExactMatchLookupPtr = NULL",devNum, portNum, ttiLookupNum);

            /*
                1.1.4. Call with NULL profileIdPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
            st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,NULL);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, profileIdPtr = NULL",devNum, portNum, ttiLookupNum);

        }

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, ttiLookupNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PORT_TRUNK_CNS;
    ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, portNum, ttiLookupNum);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiPortProfileIdModePortGet(devNum,portNum,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, ttiLookupNum);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT                  keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            ttiLookupNum,
    IN  GT_BOOL                                     enableExactMatchLookup,
    IN  GT_U32                                      profileId
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet)
{
/*
    ITERATE_DEVICES PHY_PORTS (Falcon)
    1.1.1.  Call function with ttiLookupNum  [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                              CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                               keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E]
                               and profileId [1]

    Expected: GT_OK.
    1.1.2. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
    Expected: GT_OK and the same profileId.
    1.1.3.  Call function with ttiLookupNum  [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                              CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                               keyType [CPSS_DXCH_TTI_KEY_UDB_UDE6_E]
                               and profileId [15]

    Expected: GT_OK.
    1.1.4. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
    Expected: GT_OK and the same profileId.
    1.1.5. Call with wrong ttiLookupNum [last] and other valid params
    Expected: GT_BAD_PARAM.
    1.1.6. Call with wrong profileId [16] and other valid params
    Expected: GT_OUT_OF_RANGE.
    1.1.7. Call with out of range keyType and other valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    GT_BOOL                             enableExactMatchLookup;
    GT_BOOL                             enableExactMatchLookupGet;
    GT_U32                              profileId;
    GT_U32                              profileIdGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {

            for (ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
                 ttiLookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E; ttiLookupNum++)
            {
                 /*
                    1.1.1. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                     keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E]
                                     and profileId [1]
                    Expected: GT_OK.
                */
                profileId   = 1;
                enableExactMatchLookup = GT_TRUE;
                keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
                st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
                UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);

                /*
                    1.1.2. Call cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet.
                    Expected: GT_OK and the same profileId.
                */
                st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum, portNum, keyType, ttiLookupNum, &enableExactMatchLookupGet, &profileIdGet);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
                UTF_VERIFY_EQUAL4_STRING_MAC(profileId, profileIdGet,
                    "got another profileIdMode then was set: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
                if (enableExactMatchLookup==enableExactMatchLookupGet)
                {
                    if(enableExactMatchLookupGet==GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(profileId, profileIdGet,
                                                     "got other profileId then was set: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(0, profileIdGet,
                                                     "got other profileId then was set: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
                    }
                }

                /*
                   1.1.3. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                  CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                    keyType [CPSS_DXCH_TTI_KEY_UDB_UDE6_E]
                                    and profileId [15]
                   Expected: GT_OK.
               */
               profileId  = 15;
               keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
               enableExactMatchLookup = GT_TRUE;
               st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
               UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);

               /*
                   1.1.4. Call cpssDxChExactMatchTtiPortProfileIdModePortGet.
                   Expected: GT_OK and the same profileId.
               */
               st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum, portNum, keyType, ttiLookupNum, &enableExactMatchLookupGet, &profileIdGet);
               UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                   "cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
               UTF_VERIFY_EQUAL4_STRING_MAC(profileId, profileIdGet,
                   "got another profileIdMode then was set: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
               if (enableExactMatchLookup==enableExactMatchLookupGet)
               {
                   if(enableExactMatchLookupGet==GT_TRUE)
                   {
                       UTF_VERIFY_EQUAL4_STRING_MAC(profileId, profileIdGet,
                                                    "got other profileId then was set: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
                   }
                   else
                   {
                       UTF_VERIFY_EQUAL4_STRING_MAC(0, profileIdGet,
                                                    "got other profileId then was set: %d, %d, %d, %d", devNum, portNum, keyType, ttiLookupNum);
                   }
               }

            }

            /*
                1.1.5. Call with wrong ttiLookupNum [last]
                Expected: GT_BAD_PARAM.
            */
            ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
            profileId = 15;
            keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
            enableExactMatchLookup = GT_TRUE;
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);

            /*
                1.1.6. Call with wrong profileId [16]
                Expected: GT_OUT_OF_RANGE.
            */
            ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
            profileId  = 16;
            keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
            enableExactMatchLookup = GT_TRUE;
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);

            /*
                1.1.7. Call with out of range keyType.
                Expected: NOT GT_OK.
            */
            ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
            profileId = 15;
            keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)20;
            enableExactMatchLookup = GT_TRUE;
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_NOT_EQUAL6_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
        }

        /* Restore valid params. */
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId  = 15;
        enableExactMatchLookup = GT_TRUE;
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
            UTF_VERIFY_EQUAL6_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PORT_TRUNK_CNS;
    ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
    profileId  = 15;
    enableExactMatchLookup = GT_TRUE;
    keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet(devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
    UTF_VERIFY_EQUAL6_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum, enableExactMatchLookup, profileId);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT                  keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            ttiLookupNum,
    OUT GT_BOOL                                     *enableExactMatchLookupPtr,
    OUT GT_U32                                      *profileIdPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet)
{
/*
    ITERATE_DEVICES PHY_PORTS (Falcon)
    1.1.1. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                     and keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E]
    Expected: GT_OK.
    1.1.2. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                     and keyType [CPSS_DXCH_TTI_KEY_UDB_UDE6_E]
    Expected: GT_OK.
    1.1.3. Call with out of range keyType and other valid params.
    Expected: NOT GT_OK.
    1.1.4. Call with wrong ttiLookupNum [last] and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with NULL enableExactMatchLookupPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.6. Call with NULL profileIdPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_BOOL                             enableExactMatchLookupPtr;
    GT_U32                              profileIdPtr;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_TRUE))
        {
            for (ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
                 ttiLookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E; ttiLookupNum++)
            {
                 /*
                    1.1.1. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                     and keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E]
                    Expected: GT_OK.
                */
                keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
                st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum);

                /*
                    1.1.2. Call with ttiLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,
                                                   CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]
                                     and keyType [CPSS_DXCH_TTI_KEY_UDB_UDE6_E]
                    Expected: GT_OK.
                */
                keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
                st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum);
            }

            /*
                1.1.3. Call with out of range keyType and other valid params.
                Expected: NOT GT_OK.
            */

            keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)20;
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum);

            /*
                1.1.4. Call with wrong ttiLookupNum [last] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
            keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum);

            /*
                1.1.5. Call with NULL enableExactMatchLookupPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,NULL,&profileIdPtr);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, %d, enableExactMatchLookupPtr = NULL",devNum, portNum, keyType, ttiLookupNum);

            /*
                1.1.6. Call with NULL profileIdPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,NULL);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, %d, profileIdPtr = NULL",devNum, portNum, keyType, ttiLookupNum);

        }

        st = prvUtfNextDefaultEPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextDefaultEPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call for each non-active port */
            st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, keyType, ttiLookupNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = TTI_VALID_PORT_TRUNK_CNS;
    ttiLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, portNum, keyType, ttiLookupNum);
    }

    /* 3. Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet(devNum,portNum,keyType,ttiLookupNum,&enableExactMatchLookupPtr,&profileIdPtr);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, keyType, ttiLookupNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   IN  GT_U32                              profileId
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPclProfileIdSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with valid parameters
         Expected: GT_OK.
    1.2  Call cpssDxChExactMatchPclProfileIdGet with non-NULL profileIdPtr.
         Expected: GT_OK and the same profileId .
    2.1  Call function with direction [CPSS_PCL_DIRECTION_EGRESS_E]
         packetType [CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E ,CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E]
         pclLookupNum  [first,second] and same subProfileid
         Expected: GT_OK.
    2.2  Call cpssDxChTtiExactMatchProfileIdGet.
         Expected: GT_OK and the same profileIds.
    3.1.  Call with wrong pclLookupNum [last]
         Expected: GT_BAD_PARAM.
    3.2. Call with wrong profileId [17]
         Expected: GT_OUT_OF_RANGE.
    3.3. Call with wrong subProfileId [8]
         Expected: GT_BAD_PARAM.
*/
   GT_STATUS                           st;
   GT_U8                               devNum;
   CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
   CPSS_PCL_DIRECTION_ENT              direction;
   GT_U32                              subProfileId;
   CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum;
   GT_BOOL                             enableExactMatchLookup;
   GT_BOOL                             enableExactMatchLookupGet;
   GT_U32                              profileId;
   GT_U32                              profileIdGet;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,pclLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, pclLookupNum);
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(devNum,pclLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, pclLookupNum);

        /*
           1.1.1 Call function with valid parameters
           Expected: GT_OK.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.1 Call cpssDxChTtiExactMatchProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,
            "got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
            "got other profileId then was set: %d", devNum);
        }

        /*
           1.1.2 Call function with direction [CPSS_PCL_DIRECTION_EGRESS_E] and same parameters
           Expected: GT_OK.
        */
        direction = CPSS_PCL_DIRECTION_EGRESS_E ;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchPclProfileIdSet(devNum,direction , packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum,direction , packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.2 Call cpssDxChTtiExactMatchProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction, packetType, subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,
            "got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,"got other profileId then was set: %d", devNum);
        }

         /*
           2.1 Call function with direction [CPSS_PCL_DIRECTION_EGRESS_E]
               packetType [CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E ,CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E]
               pclLookupNum  [first,second]
               and same subProfileid
           Expected: GT_OK.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
        direction = CPSS_PCL_DIRECTION_EGRESS_E ;
        subProfileId = 7;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 9;

        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);

        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 12;

        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            2.2 Call cpssDxChTtiExactMatchProfileIdGet.
            Expected: GT_OK and the same profileIds.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
        subProfileId = 7;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;/* hit 1 client is PCL by default */
        profileId   = 9;

        st = cpssDxChExactMatchPclProfileIdGet(devNum,  direction ,packetType,subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,"got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,"got other profileId then was set: %d", devNum);
        }


        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 12;

        st = cpssDxChExactMatchPclProfileIdGet(devNum,  direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,"got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,"got other profileId then was set: %d", devNum);
        }

        /*
            3.1 Call with wrong pclLookupNum [last]
            Expected: GT_BAD_PARAM.
        */
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            3.2 Call with wrong profileId [17]
            Expected: GT_OUT_OF_RANGE.
        */
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId = 17;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            3.3. Call with wrong subProfileId [8]
            Expected: GT_BAD_PARAM.
        */
        profileId = 2;
        subProfileId=8;
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            subProfileId=16;
        }
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }

    /* 4. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* valid parameters*/
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }
}
/*
GT_STATUS cpssDxChExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   OUT GT_U32                              *profileIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPclProfileIdGet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with valid parameters
         Expected: GT_OK.
    1.2  Call function with NULL profileIdPtr.
         Expected: GT_BAD_PTR.
    1.3  Call function with NULL enableExactMatchLookupPtr
         Expected: GT_BAD_PTR.
    1.4  Call with wrong pclLookupNum [last]
         Expected: GT_BAD_PARAM.
    1.5  Call with wrong subProfileId [8]
         Expected: GT_BAD_PARAM.
    2.   Call for for not-active devices or devices from non-applicable family
         Expected: GT_NOT_APPLICABLE_DEVICE.
*/
   GT_STATUS                           st;
   GT_U8                               devNum;
   CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
   CPSS_PCL_DIRECTION_ENT              direction;
   GT_U32                              subProfileId;
   CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum;
   GT_BOOL                             enableExactMatchLookup;
   GT_U32                              profileId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with valid parameters
                 Expected: GT_OK.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        st = cpssDxChExactMatchPclProfileIdGet(devNum,direction , packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        /*
            1.2  Call function with NULL profileIdPtr.
                 Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileIdPtr = NULL", devNum);

       /*  1.3  Call function with NULL enableExactMatchLookupPtr
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,NULL, &profileId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enableExactMatchLookupPtr = NULL", devNum);

        /*
            1.4 Call with wrong pclLookupNum [second]
                Expected: GT_BAD_PARAM.
        */
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.5 Call with wrong subProfileId [8]
                Expected: GT_BAD_PARAM.
        */
        subProfileId=8;
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            subProfileId=16;
        }
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }
    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* valid parameters*/
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchClientLookupSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchClientLookupSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchLookupNum [first, second]
    Expected: GT_OK.
    1.2. Call with out of range clientType [CPSS_DXCH_EM_CLIENT_LAST_E].
    Expected: NON GT_OK.
    1.3. Call with out of range exactMatchLookupNum [last].
    Expected: NON GT_OK.

*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientTypeGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exactMatchLookupNum [first]
            Expected: GT_OK.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchLookupNum, clientType);

        st = cpssDxChExactMatchClientLookupGet(devNum,  exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchClientLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(clientType, clientTypeGet,
                       "get another clientTypeGet than was set: %d", devNum);

        /*
            1.1. Call with exactMatchLookupNum [second]
            Expected: GT_OK.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;

        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchLookupNum, clientType);

        st = cpssDxChExactMatchClientLookupGet(devNum,  exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchClientLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(clientType, clientTypeGet,
                       "get another clientTypeGet than was set: %d", devNum);

        /*
           1.2. Call with out of range clientType [CPSS_DXCH_EM_CLIENT_LAST_E].
            Expected: NON GT_OK.
        */
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E;

        st = cpssDxChExactMatchClientLookupSet(devNum,  exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, devNum, exactMatchLookupNum, clientType);

       st = cpssDxChExactMatchClientLookupGet(devNum,  exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchClientLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E, clientTypeGet,
                       "get another emFirstLookupClientGet than was set: %d", devNum);


        /* 1.3. Call with out of range exactMatchLookupNum [last].
            Expected: NON GT_OK.
         */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

        st = cpssDxChExactMatchClientLookupSet(devNum,  exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchLookupNum, clientType);

        /* return to default values */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchLookupNum, clientType);

    }

    exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchClientLookupGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            *clientTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchClientLookupGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null clientTypeGet
    Expected: GT_OK.
    1.2. Call with out of range exactMatchLookupNum [last].
    Expected: GT_BAD_PTR.
    1.3. Call with clientTypeGet [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientTypeGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null EM LookupClientsGet
            Expected: GT_OK.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with out of range exactMatchLookupNum [last].
            Expected: GT_BAD_PARAM.
        */

        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with clientTypeGet [NULL].
            Expected: GT_BAD_PTR.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupGet(devNum,exactMatchLookupNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, clientTypeGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL exactMatchActivityBit
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchActivityBitEnableSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchActivityBit[GT_TRUE]
    Expected: GT_OK.
    1.2. Call with exactMatchActivityBit[GT_FALSE]
    Expected: GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_BOOL     exactMatchActivityBitEnable;
    GT_BOOL     exactMatchActivityBitEnableGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1. Call with exactMatchActivityBitEnable[GT_TRUE]
               Expected: GT_OK.
        */
        exactMatchActivityBitEnable=GT_TRUE;

        st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableSet: %d", devNum);

        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(exactMatchActivityBitEnable, exactMatchActivityBitEnableGet,
                       "get another exactMatchActivityBitEnable than was set: %d", devNum);

        /*
            1.2. Call with exactMatchActivityBitEnable[GT_FALSE]
                Expected: GT_OK.
        */

        exactMatchActivityBitEnable=GT_FALSE;

        st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableSet: %d", devNum);

        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(exactMatchActivityBitEnable, exactMatchActivityBitEnableGet,
                       "get another exactMatchActivityBitEnable than was set: %d", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchActivityBitEnable=GT_TRUE;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *exactMatchActivityBitEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchActivityBitEnableGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null exactMatchActivityBitEnablePtr
    Expected: GT_OK.
    1.2. Call with exactMatchActivityBitEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_BOOL     exactMatchActivityBitEnableGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null exactMatchActivityBitEnablePtr
            Expected: GT_OK.
        */
        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableGet: %d", devNum);

        /*
            1.2. Call with exactMatchActivityBitEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchActivityBitEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;


    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/* check port group return: GT_TRUE - not valid group; GT_FALSE - valid */
static GT_BOOL  prvUtfExactMatchActivePortGroupCheck(GT_U8 devNum, GT_U32 portGroupId)
{
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        switch (portGroupId)
        {
            case 0:
            case 2:
            case 4:
            case 6:
                /* valid groups*/
                break;
            default:
                return GT_TRUE; /* skip other */
        }
    }
    /* all SIP_5 device ignores port group for PCL APIs.
       all port groups are valid for Lion2 devices */

    return GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchActivityStatusGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchEntryIndex[0] exactMatchClearActivity[GT_FALSE]
    Expected: GT_BAD_VALUE, Exact Match entry is invalid.
    1.2. Call with exactMatchEntryIndex[1] exactMatchClearActivity[GT_TRUE]
    Expected: GT_BAD_VALUE, Exact Match entry is invalid.
    1.3. Call with exactMatchActivityStatusPtr[NULL]
    Expected: GT_OK.
*/

    GT_STATUS               st  = GT_OK;
    GT_U8                   devNum = 0;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;
    GT_U32                  exactMatchEntryIndex;
    GT_BOOL                 exactMatchClearActivity;
    GT_BOOL                 exactMatchActivityStatus;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /* check and skip not valid port groups */
            if (prvUtfExactMatchActivePortGroupCheck(devNum,portGroupId) == GT_TRUE)
            {
                continue;
            }

           /*
               1.1. Call with exactMatchEntryIndex[0] exactMatchClearActivity[GT_FALSE]
                   Expected: GT_NOT_FOUND, Exact Match entry is invalid.
            */

            exactMatchEntryIndex=0;
            exactMatchClearActivity=GT_FALSE;

            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     &exactMatchActivityStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);

            /*
                1.2. Call with exactMatchEntryIndex[1] exactMatchClearActivity[GT_TRUE]
                    Expected: GT_NOT_FOUND, Exact Match entry is invalid.
            */
            exactMatchEntryIndex=1;
            exactMatchClearActivity=GT_TRUE;

            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     &exactMatchActivityStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);

            /* 1.3. Call with exactMatchActivityStatusPtr[NULL]
               Expected: GT_BAD_PTR.*/
            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchEntryIndex=1;
    exactMatchClearActivity=GT_TRUE;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     &exactMatchActivityStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    }

    /* Initialize port group. */
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                             exactMatchEntryIndex,
                                             exactMatchClearActivity,
                                             &exactMatchActivityStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileKeyParamsSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     *keyParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileKeyParamsSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchProfileIndex, keyParamsPtr
    Expected: GT_OK.
    1.2. Call with out of range exactMatchProfileIndex [16]
    Expected: NON GT_OK.
    1.3. Call with keyParamsPtr [NULL]
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_U8       i=0;

    GT_U32                                          exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParamsGet;


    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exactMatchProfileIndex, keyParamsPtr
                Expected: GT_OK.
        */

       exactMatchProfileIndex = 3;
       keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
       keyParams.keyStart=3;
       for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
       {
           keyParams.mask[i]=i+1;
       }

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, exactMatchProfileIndex, keyParams.keySize, keyParams.keyStart);


        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchProfileKeyParamsGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keySize, keyParamsGet.keySize,
                       "get another keyParams.keySize than was set: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keyStart, keyParamsGet.keyStart,
                       "get another keyParams.keyStart than was set: %d", devNum);

        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.mask[i], keyParamsGet.mask[i],
                       "get another keyParams.mask than was set: %d", devNum);
        }

        /*
           1.2. Call with out of range exactMatchProfileIndex [16]
            Expected: NON GT_OK.

        */
        exactMatchProfileIndex = 16;

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchProfileIndex);


        /*
           1.3. Call with keyParamsPtr [NULL]
            Expected: NON GT_OK.

        */
        exactMatchProfileIndex = 5;

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, exactMatchProfileIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchProfileIndex = 7;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          exactMatchProfileIndex,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileKeyParamsGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null exactMatchProfileIndex, keyParamsPtr
    Expected: GT_OK.
    1.2. Call with out of range exactMatchProfileIndex [16]
    Expected: NON GT_OK.
    1.3. Call with keyParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    GT_U32 exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC keyParamsGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null EM LookupClientsGet
            Expected: GT_OK.
        */
        exactMatchProfileIndex=2;

        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with out of range exactMatchProfileIndex [16]
                Expected: NON GT_OK.
        */
        exactMatchProfileIndex=16;
        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, EM LookupClientsGet = NULL", devNum);

         /*
            1.3. Call with keyParamsPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=1;
        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, EM LookupClientsGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchProfileIndex=2;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
* @internal prvSetExactMatchActionDefaultValues function
* @endinternal
*
* @brief   This routine set default values to an exact match
*          Action
*
* @param[in] actionPtr           - (pointer to) action
* @param[in] type                - action type (TTI/PCL/EPCL)
*/
GT_VOID prvSetExactMatchActionDefaultValues
(
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType
)
{
    cpssOsBzero((GT_VOID*) actionPtr, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        actionPtr->ttiAction.tunnelTerminate                    = GT_FALSE;
        actionPtr->ttiAction.ttPassengerPacketType              = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        actionPtr->ttiAction.tsPassengerPacketType              = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;

        actionPtr->ttiAction.ttHeaderLength                     = 0;
        actionPtr->ttiAction.continueToNextTtiLookup            = GT_FALSE;

        actionPtr->ttiAction.copyTtlExpFromTunnelHeader         = GT_FALSE;

        actionPtr->ttiAction.mplsCommand                        = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
        actionPtr->ttiAction.mplsTtl                            = 0;

        actionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode = CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E;
        actionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;

        actionPtr->ttiAction.enableDecrementTtl                 = GT_FALSE;

        actionPtr->ttiAction.command                            = CPSS_PACKET_CMD_FORWARD_E;

        actionPtr->ttiAction.redirectCommand                    = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionPtr->ttiAction.egressInterface.type               = CPSS_INTERFACE_PORT_E;
        actionPtr->ttiAction.egressInterface.devPort.hwDevNum   = 0;
        actionPtr->ttiAction.egressInterface.devPort.portNum    = TTI_VALID_PORT_TRUNK_CNS;
        actionPtr->ttiAction.egressInterface.trunkId            = TTI_VALID_PORT_TRUNK_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->ttiAction.egressInterface.trunkId);
        actionPtr->ttiAction.egressInterface.vidx               = 0;
        actionPtr->ttiAction.egressInterface.vlanId             = 0;
        actionPtr->ttiAction.egressInterface.hwDevNum             = 0;
        actionPtr->ttiAction.egressInterface.fabricVidx         = 0;
        actionPtr->ttiAction.egressInterface.index              = 0;
        actionPtr->ttiAction.arpPtr                             = 0;
        actionPtr->ttiAction.tunnelStart                        = GT_TRUE;
        actionPtr->ttiAction.tunnelStartPtr                     = 0;
        actionPtr->ttiAction.routerLttPtr                       = 0;
        actionPtr->ttiAction.vrfId                              = 0;

        actionPtr->ttiAction.sourceIdSetEnable                  = GT_FALSE;
        actionPtr->ttiAction.sourceId = 0;

        actionPtr->ttiAction.tag0VlanCmd                        = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
        actionPtr->ttiAction.tag0VlanId                         = TTI_VALID_VLAN_ID_CNS;
        actionPtr->ttiAction.tag1VlanCmd                        = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        actionPtr->ttiAction.tag1VlanId                         = TTI_VALID_VLAN_ID_CNS;
        actionPtr->ttiAction.tag0VlanPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        actionPtr->ttiAction.nestedVlanEnable                   = GT_FALSE;

        actionPtr->ttiAction.bindToPolicerMeter                 = GT_FALSE;
        actionPtr->ttiAction.bindToPolicer                      = GT_FALSE;
        actionPtr->ttiAction.policerIndex                       = 0;

        actionPtr->ttiAction.qosPrecedence                      = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        actionPtr->ttiAction.keepPreviousQoS                    = GT_FALSE;
        actionPtr->ttiAction.trustUp                            = GT_FALSE;
        actionPtr->ttiAction.trustDscp                          = GT_FALSE;
        actionPtr->ttiAction.trustExp                           = GT_FALSE;
        actionPtr->ttiAction.qosProfile                         = 0;
        actionPtr->ttiAction.modifyTag0Up                       = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
        actionPtr->ttiAction.tag1UpCommand                      = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
        actionPtr->ttiAction.modifyDscp                         = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
        actionPtr->ttiAction.tag0Up                             = 0;
        actionPtr->ttiAction.tag1Up                             = 0;
        actionPtr->ttiAction.remapDSCP                          = GT_FALSE;

        actionPtr->ttiAction.qosUseUpAsIndexEnable              = GT_FALSE;
        actionPtr->ttiAction.qosMappingTableIndex               = 0;
        actionPtr->ttiAction.mplsLLspQoSProfileEnable           = GT_FALSE;


        actionPtr->ttiAction.pcl0OverrideConfigIndex            = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        actionPtr->ttiAction.pcl0_1OverrideConfigIndex          = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        actionPtr->ttiAction.pcl1OverrideConfigIndex            = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        actionPtr->ttiAction.iPclConfigIndex                    = 0;

        actionPtr->ttiAction.iPclUdbConfigTableEnable           = GT_FALSE;
        actionPtr->ttiAction.iPclUdbConfigTableIndex            = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;

        actionPtr->ttiAction.mirrorToIngressAnalyzerEnable      = GT_FALSE;
        actionPtr->ttiAction.mirrorToIngressAnalyzerIndex       = 0;
        actionPtr->ttiAction.userDefinedCpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        actionPtr->ttiAction.bindToCentralCounter               = GT_FALSE;
        actionPtr->ttiAction.centralCounterIndex                = 0;
        actionPtr->ttiAction.vntl2Echo                          = GT_FALSE;
        actionPtr->ttiAction.bridgeBypass                       = GT_FALSE;
        actionPtr->ttiAction.ingressPipeBypass                  = GT_FALSE;
        actionPtr->ttiAction.actionStop                         = GT_FALSE;
        actionPtr->ttiAction.hashMaskIndex                      = 0;
        actionPtr->ttiAction.modifyMacSa                        = GT_FALSE;
        actionPtr->ttiAction.modifyMacDa                        = GT_FALSE;
        actionPtr->ttiAction.ResetSrcPortGroupId                = GT_FALSE;
        actionPtr->ttiAction.multiPortGroupTtiEnable            = GT_FALSE;

        actionPtr->ttiAction.sourceEPortAssignmentEnable        = GT_FALSE;
        actionPtr->ttiAction.sourceEPort                        = TTI_VALID_PORT_TRUNK_CNS;
        actionPtr->ttiAction.ipfixEn                            = GT_FALSE;
        actionPtr->ttiAction.flowId                             = 0;
        actionPtr->ttiAction.setMacToMe                         = GT_FALSE;
        actionPtr->ttiAction.rxProtectionSwitchEnable           = GT_FALSE;
        actionPtr->ttiAction.rxIsProtectionPath                 = GT_FALSE;
        actionPtr->ttiAction.pwTagMode                          = CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E;

        actionPtr->ttiAction.oamTimeStampEnable                 = GT_FALSE;
        actionPtr->ttiAction.oamOffsetIndex                     = 0;
        actionPtr->ttiAction.oamProcessEnable                   = GT_FALSE;
        actionPtr->ttiAction.oamProfile                         = 0;
        actionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable= GT_FALSE;
        actionPtr->ttiAction.isPtpPacket                        = GT_FALSE;
        actionPtr->ttiAction.ptpTriggerType                     = CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E;
        actionPtr->ttiAction.ptpOffset                          = 0;

        actionPtr->ttiAction.cwBasedPw                          = GT_FALSE;
        actionPtr->ttiAction.ttlExpiryVccvEnable                = GT_FALSE;
        actionPtr->ttiAction.pwe3FlowLabelExist                 = GT_FALSE;
        actionPtr->ttiAction.pwCwBasedETreeEnable               = GT_FALSE;
        actionPtr->ttiAction.applyNonDataCwCommand              = GT_FALSE;

        actionPtr->ttiAction.unknownSaCommandEnable             = GT_FALSE;
        actionPtr->ttiAction.unknownSaCommand                   = CPSS_PACKET_CMD_FORWARD_E;
        actionPtr->ttiAction.sourceMeshIdSetEnable              = GT_FALSE;
        actionPtr->ttiAction.sourceMeshId                       = 0;
        actionPtr->ttiAction.copyReservedAssignmentEnable       = GT_FALSE;
        actionPtr->ttiAction.copyReserved                       = 0;
        actionPtr->ttiAction.triggerHashCncClient               = GT_FALSE;

        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:

    /*  actionPtr [ pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
        modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
        qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
        vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].
    */
        actionPtr->pclAction.pktCmd       = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        actionPtr->pclAction.actionStop   = GT_TRUE;
        if(actionType==CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E)
        {
            actionPtr->pclAction.egressPolicy = GT_TRUE;
        }
        else
        {
            actionPtr->pclAction.egressPolicy = GT_FALSE;
        }

        actionPtr->pclAction.mirror.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;
        actionPtr->pclAction.mirror.mirrorToRxAnalyzerPort = GT_FALSE;

        actionPtr->pclAction.matchCounter.enableMatchCount = GT_FALSE;
        actionPtr->pclAction.matchCounter.matchCounterIndex = 0;

        actionPtr->pclAction.qos.ingress.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        actionPtr->pclAction.qos.ingress.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        actionPtr->pclAction.qos.ingress.profileIndex = 0;
        actionPtr->pclAction.qos.ingress.profileAssignIndex = GT_FALSE;
        actionPtr->pclAction.qos.ingress.profilePrecedence =
                                    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

        actionPtr->pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

        actionPtr->pclAction.redirect.data.outIf.outInterface.type   = CPSS_INTERFACE_VID_E;
        actionPtr->pclAction.redirect.data.outIf.outInterface.vlanId = 100;

        actionPtr->pclAction.redirect.data.outIf.vntL2Echo     = GT_FALSE;
        actionPtr->pclAction.redirect.data.outIf.tunnelStart   = GT_FALSE;
        actionPtr->pclAction.redirect.data.outIf.tunnelType    =
                                CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;

        /* logical Source Port applicable only for xCat C0; xCat3 device */
        actionPtr->pclAction.redirect.data.logicalSourceInterface.logicalInterface.type = CPSS_INTERFACE_PORT_E;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum = 0;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum = 0;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.sourceMeshIdSetEnable = GT_FALSE;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.sourceMeshId = 0;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.userTagAcEnable = GT_FALSE;

        actionPtr->pclAction.policer.policerEnable = GT_FALSE;
        actionPtr->pclAction.policer.policerId     = 0;

        actionPtr->pclAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
        actionPtr->pclAction.vlan.ingress.nestedVlan = GT_FALSE;
        actionPtr->pclAction.vlan.ingress.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        actionPtr->pclAction.vlan.ingress.vlanId     = 100;

        actionPtr->pclAction.ipUcRoute.doIpUcRoute       = GT_FALSE;
        actionPtr->pclAction.ipUcRoute.arpDaIndex        = 0;
        actionPtr->pclAction.ipUcRoute.decrementTTL      = GT_FALSE;
        actionPtr->pclAction.ipUcRoute.bypassTTLCheck    = GT_FALSE;
        actionPtr->pclAction.ipUcRoute.icmpRedirectCheck = GT_FALSE;

        actionPtr->pclAction.sourceId.assignSourceId = GT_TRUE;
        actionPtr->pclAction.sourceId.sourceIdValue  = 0;

        /* oam field applicable only for Bobcat2; Caelum; Bobcat3 device */
        actionPtr->pclAction.oam.timeStampEnable = GT_FALSE;
        actionPtr->pclAction.oam.offsetIndex = 0;
        actionPtr->pclAction.oam.oamProcessEnable = GT_FALSE;
        actionPtr->pclAction.oam.oamProfile = 0;

        /* flowId field applicable only for Bobcat2; Caelum; Bobcat3 device */
        actionPtr->pclAction.flowId = 0;

        /* Assigns the unknown-SA-command applicable only for xCat C0; xCat3 device */
        actionPtr->pclAction.unknownSaCommandEnable = GT_FALSE;
        actionPtr->pclAction.unknownSaCommand = CPSS_PACKET_CMD_FORWARD_E;
        actionPtr->pclAction.copyReserved.assignEnable       = GT_FALSE;
        actionPtr->pclAction.copyReserved.copyReserved       = 0;
        actionPtr->pclAction.triggerHashCncClient            = GT_FALSE;

        break;
    default:
        break;
    }
}

/**
* @internal prvCompareExactMatchInterfaceInfoStructs function
* @endinternal
*
* @brief   This routine set compares 2 Interface info structures.
*
* @param[in] actionName               - name of tested action structs
* @param[in] interfaceInfoName        - name of interface member structs
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
static GT_STATUS prvCompareExactMatchInterfaceInfoStructs
(
    IN GT_CHAR                          *actionName,
    IN GT_CHAR                          *interfaceInfoName,
    IN CPSS_INTERFACE_INFO_STC          *firstPtr,
    IN CPSS_INTERFACE_INFO_STC          *secondPtr
)
{
    GT_STATUS                           st = GT_OK;

    if(firstPtr->type != secondPtr->type)
    {
        PRV_UTF_LOG4_MAC("get another %s.%s.type than was set, expected = %d, received = %d\n",
                         actionName, interfaceInfoName, firstPtr->type,
                         secondPtr->type);
        st = GT_FAIL;
    }

    switch(firstPtr->type)
    {
    case CPSS_INTERFACE_PORT_E:
        if(firstPtr->devPort.hwDevNum != secondPtr->devPort.hwDevNum)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.devPort.hwDevNum than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->devPort.hwDevNum, secondPtr->devPort.hwDevNum);
            st = GT_FAIL;
        }
        if(firstPtr->devPort.portNum != secondPtr->devPort.portNum)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.devPort.portNum than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->devPort.portNum, secondPtr->devPort.portNum);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_TRUNK_E:
        if(firstPtr->trunkId != secondPtr->trunkId)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.trunkId than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->trunkId, secondPtr->trunkId);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_VIDX_E:
        if(firstPtr->vidx != secondPtr->vidx)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.vidx than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->vidx, secondPtr->vidx);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_VID_E:
        if(firstPtr->vlanId != secondPtr->vlanId)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.vlanId than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->vlanId, secondPtr->vlanId);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_DEVICE_E:
        if(firstPtr->hwDevNum != secondPtr->hwDevNum)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.hwDevNum than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->hwDevNum, secondPtr->hwDevNum);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_FABRIC_VIDX_E:
        if(firstPtr->fabricVidx != secondPtr->fabricVidx)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.fabricVidx than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->fabricVidx, secondPtr->fabricVidx);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_INDEX_E:
        if(firstPtr->index != secondPtr->index)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.index than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->index, secondPtr->index);
            st = GT_FAIL;
        }
        break;

    default:
        break;
    }

    return st;
}

/**
* @internal prvCompareExactMatchActionStructs function
* @endinternal
*
* @brief   This routine set compares 2 Action structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
* @param[in] actionType               - type of the action to use
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
GT_STATUS prvCompareExactMatchActionStructs
(
    IN GT_U8                                    devNum,
    IN GT_CHAR                                  *name,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *firstActionPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *secondActionPtr
)
{
    GT_STATUS   st = GT_OK, op_st;
    GT_BOOL isEqual,failureWas;

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        if(firstActionPtr->ttiAction.tunnelTerminate != secondActionPtr->ttiAction.tunnelTerminate)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tunnelTerminate than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tunnelTerminate,
                             secondActionPtr->ttiAction.tunnelTerminate);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.ttPassengerPacketType != secondActionPtr->ttiAction.ttPassengerPacketType)
        {
            if(((CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E == firstActionPtr->ttiAction.mplsCommand) ||
                (CPSS_DXCH_TTI_MPLS_POP1_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                (CPSS_DXCH_TTI_MPLS_POP2_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                (CPSS_DXCH_TTI_MPLS_POP3_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                (CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E == firstActionPtr->ttiAction.mplsCommand)) &&
               (CPSS_DXCH_TTI_PASSENGER_MPLS_E != firstActionPtr->ttiAction.ttPassengerPacketType))
            {
                PRV_UTF_LOG1_MAC("error in test, %s.ttiAction.ttPassengerPacketType is invalid\n",
                                 name);
            }

            PRV_UTF_LOG3_MAC("get another %s.ttiAction.ttPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.ttPassengerPacketType,
                             secondActionPtr->ttiAction.ttPassengerPacketType);
            st = GT_FAIL;
        }
        if((CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == firstActionPtr->ttiAction.redirectCommand) &&
           (GT_TRUE == firstActionPtr->ttiAction.tunnelStart))
        {
            if(firstActionPtr->ttiAction.tsPassengerPacketType != secondActionPtr->ttiAction.tsPassengerPacketType)
            {
                if(((CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E == firstActionPtr->ttiAction.mplsCommand) ||
                    (CPSS_DXCH_TTI_MPLS_POP1_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                    (CPSS_DXCH_TTI_MPLS_POP2_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                    (CPSS_DXCH_TTI_MPLS_POP3_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                    (CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E == firstActionPtr->ttiAction.mplsCommand)) &&
                   (CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E != firstActionPtr->ttiAction.tsPassengerPacketType))
                {
                    PRV_UTF_LOG1_MAC("error in test, %s.ttiAction.tsPassengerPacketType is invalid\n",
                                     name);
                }

                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tsPassengerPacketType than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tsPassengerPacketType,
                                 secondActionPtr->ttiAction.tsPassengerPacketType);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.copyTtlExpFromTunnelHeader != secondActionPtr->ttiAction.copyTtlExpFromTunnelHeader)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.copyTtlExpFromTunnelHeader than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.copyTtlExpFromTunnelHeader,
                             secondActionPtr->ttiAction.copyTtlExpFromTunnelHeader);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.mplsCommand != secondActionPtr->ttiAction.mplsCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.mplsCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.mplsCommand,
                             secondActionPtr->ttiAction.mplsCommand);
            st = GT_FAIL;
        }
        if(CPSS_DXCH_TTI_MPLS_NOP_CMD_E != firstActionPtr->ttiAction.mplsCommand)
        {
            if(firstActionPtr->ttiAction.mplsTtl != secondActionPtr->ttiAction.mplsTtl)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.mplsTtl than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.mplsTtl, secondActionPtr->ttiAction.mplsTtl);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.enableDecrementTtl != secondActionPtr->ttiAction.enableDecrementTtl)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.enableDecrementTtl than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.enableDecrementTtl,
                                 secondActionPtr->ttiAction.enableDecrementTtl);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.command != secondActionPtr->ttiAction.command)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.command,
                             secondActionPtr->ttiAction.command);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.redirectCommand != secondActionPtr->ttiAction.redirectCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.redirectCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.redirectCommand,
                             secondActionPtr->ttiAction.redirectCommand);
            st = GT_FAIL;
        }
        if(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == firstActionPtr->ttiAction.redirectCommand)
        {
            op_st = prvCompareExactMatchInterfaceInfoStructs(name,
                                                   "ttiAction.egressInterface",
                                                   &firstActionPtr->ttiAction.egressInterface,
                                                   &secondActionPtr->ttiAction.egressInterface);
            st = GT_OK != op_st ? op_st: st;
            if(firstActionPtr->ttiAction.arpPtr != secondActionPtr->ttiAction.arpPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.arpPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.arpPtr,
                                 secondActionPtr->ttiAction.arpPtr);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.tunnelStart != secondActionPtr->ttiAction.tunnelStart)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tunnelStart than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tunnelStart,
                                 secondActionPtr->ttiAction.tunnelStart);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.tunnelStartPtr != secondActionPtr->ttiAction.tunnelStartPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tunnelStartPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tunnelStartPtr,
                                 secondActionPtr->ttiAction.tunnelStartPtr);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.sourceIdSetEnable != secondActionPtr->ttiAction.sourceIdSetEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceIdSetEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.sourceIdSetEnable,
                             secondActionPtr->ttiAction.sourceIdSetEnable);
            st = GT_FAIL;
        }
        if (firstActionPtr->ttiAction.sourceIdSetEnable == GT_TRUE)
        {
            if(firstActionPtr->ttiAction.sourceId != secondActionPtr->ttiAction.sourceId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceId than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.sourceId,
                                 secondActionPtr->ttiAction.sourceId);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.tag0VlanCmd != secondActionPtr->ttiAction.tag0VlanCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0VlanCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0VlanCmd,
                             secondActionPtr->ttiAction.tag0VlanCmd);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag0VlanId != secondActionPtr->ttiAction.tag0VlanId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0VlanId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0VlanId,
                             secondActionPtr->ttiAction.tag0VlanId);
            st = GT_FAIL;
        }
        if(CPSS_DXCH_TTI_MPLS_NOP_CMD_E == firstActionPtr->ttiAction.mplsCommand)
        {
            if(firstActionPtr->ttiAction.tag1VlanCmd != secondActionPtr->ttiAction.tag1VlanCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1VlanCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tag1VlanCmd,
                                 secondActionPtr->ttiAction.tag1VlanCmd);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.tag1VlanId != secondActionPtr->ttiAction.tag1VlanId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1VlanId than was set, expected = %d, received = %d\n",
                            name, firstActionPtr->ttiAction.tag1VlanId,
                            secondActionPtr->ttiAction.tag1VlanId);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.tag0VlanPrecedence != secondActionPtr->ttiAction.tag0VlanPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0VlanPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0VlanPrecedence,
                             secondActionPtr->ttiAction.tag0VlanPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.nestedVlanEnable != secondActionPtr->ttiAction.nestedVlanEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.nestedVlanEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.nestedVlanEnable,
                             secondActionPtr->ttiAction.nestedVlanEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.bindToPolicerMeter != secondActionPtr->ttiAction.bindToPolicerMeter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bindToPolicerMeter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bindToPolicerMeter,
                             secondActionPtr->ttiAction.bindToPolicerMeter);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.bindToPolicer != secondActionPtr->ttiAction.bindToPolicer)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bindToPolicer than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bindToPolicer,
                             secondActionPtr->ttiAction.bindToPolicer);
            st = GT_FAIL;
        }
        if (GT_TRUE == firstActionPtr->ttiAction.bindToPolicer)
        {
            if(firstActionPtr->ttiAction.policerIndex != secondActionPtr->ttiAction.policerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.policerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.policerIndex,
                                 secondActionPtr->ttiAction.policerIndex);
                st = GT_FAIL;
            }
        }

        if(GT_TRUE == firstActionPtr->ttiAction.ipfixEn)
        {
            if(firstActionPtr->ttiAction.flowId != secondActionPtr->ttiAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.flowId   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.flowId,
                                                 secondActionPtr->ttiAction.flowId);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.qosPrecedence != secondActionPtr->ttiAction.qosPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.qosPrecedence,
                             secondActionPtr->ttiAction.qosPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.keepPreviousQoS != secondActionPtr->ttiAction.keepPreviousQoS)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.keepPreviousQoS than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.keepPreviousQoS,
                             secondActionPtr->ttiAction.keepPreviousQoS);
            st = GT_FAIL;
        }
        if (GT_FALSE == firstActionPtr->ttiAction.keepPreviousQoS)
        {
            if(firstActionPtr->ttiAction.trustUp != secondActionPtr->ttiAction.trustUp)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.trustUp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.trustUp,
                                 secondActionPtr->ttiAction.trustUp);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.trustDscp != secondActionPtr->ttiAction.trustDscp)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.trustDscp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.trustDscp,
                                 secondActionPtr->ttiAction.trustDscp);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.trustExp != secondActionPtr->ttiAction.trustExp)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.trustExp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.trustExp,
                                 secondActionPtr->ttiAction.trustExp);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.qosProfile != secondActionPtr->ttiAction.qosProfile)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosProfile than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.qosProfile,
                                 secondActionPtr->ttiAction.qosProfile);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.modifyTag0Up != secondActionPtr->ttiAction.modifyTag0Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.modifyTag0Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.modifyTag0Up,
                             secondActionPtr->ttiAction.modifyTag0Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag1UpCommand != secondActionPtr->ttiAction.tag1UpCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1UpCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag1UpCommand,
                             secondActionPtr->ttiAction.tag1UpCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.modifyDscp != secondActionPtr->ttiAction.modifyDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.modifyDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.modifyDscp,
                             secondActionPtr->ttiAction.modifyDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag0Up != secondActionPtr->ttiAction.tag0Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0Up,
                             secondActionPtr->ttiAction.tag0Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag1Up != secondActionPtr->ttiAction.tag1Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag1Up,
                             secondActionPtr->ttiAction.tag1Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.remapDSCP != secondActionPtr->ttiAction.remapDSCP)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.remapDSCP than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.remapDSCP,
                             secondActionPtr->ttiAction.remapDSCP);
            st = GT_FAIL;
        }

        if(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E != firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.pcl0OverrideConfigIndex != secondActionPtr->ttiAction.pcl0OverrideConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.pcl0OverrideConfigIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.pcl0OverrideConfigIndex,
                                 secondActionPtr->ttiAction.pcl0OverrideConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.pcl0_1OverrideConfigIndex != secondActionPtr->ttiAction.pcl0_1OverrideConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.pcl0_1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.pcl0_1OverrideConfigIndex,
                                 secondActionPtr->ttiAction.pcl0_1OverrideConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.pcl1OverrideConfigIndex != secondActionPtr->ttiAction.pcl1OverrideConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.pcl1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.pcl1OverrideConfigIndex,
                                 secondActionPtr->ttiAction.pcl1OverrideConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.iPclConfigIndex != secondActionPtr->ttiAction.iPclConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.iPclConfigIndex than was set, expected = %d, received = %d\n",
                            name, firstActionPtr->ttiAction.iPclConfigIndex,
                            secondActionPtr->ttiAction.iPclConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.copyReservedAssignmentEnable   != secondActionPtr->ttiAction.copyReservedAssignmentEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.copyReservedAssignmentEnable than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.copyReservedAssignmentEnable,
                                                 secondActionPtr->ttiAction.copyReservedAssignmentEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.copyReserved   != secondActionPtr->ttiAction.copyReserved)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.copyReserved than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.copyReserved,
                                                 secondActionPtr->ttiAction.copyReserved);
                st = GT_FAIL;
            }
            if( (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) )
            {
                if(firstActionPtr->ttiAction.triggerHashCncClient != secondActionPtr->ttiAction.triggerHashCncClient)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.triggerHashCncClient than was set, expected = %d, received = %d\n",
                                     name, firstActionPtr->ttiAction.triggerHashCncClient,
                                     secondActionPtr->ttiAction.triggerHashCncClient);
                    st = GT_FAIL;
                }
            }
        }
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            if(firstActionPtr->ttiAction.triggerHashCncClient != secondActionPtr->ttiAction.triggerHashCncClient)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.triggerHashCncClient than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.triggerHashCncClient,
                                 secondActionPtr->ttiAction.triggerHashCncClient);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.mirrorToIngressAnalyzerEnable != secondActionPtr->ttiAction.mirrorToIngressAnalyzerEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.mirrorToIngressAnalyzerEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.mirrorToIngressAnalyzerEnable,
                             secondActionPtr->ttiAction.mirrorToIngressAnalyzerEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.mirrorToIngressAnalyzerIndex != secondActionPtr->ttiAction.mirrorToIngressAnalyzerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.mirrorToIngressAnalyzerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.mirrorToIngressAnalyzerIndex,
                             secondActionPtr->ttiAction.mirrorToIngressAnalyzerIndex);
            st = GT_FAIL;
        }

        if((CPSS_PACKET_CMD_MIRROR_TO_CPU_E == firstActionPtr->ttiAction.command) ||
           (CPSS_PACKET_CMD_TRAP_TO_CPU_E == firstActionPtr->ttiAction.command))
        {
            if(firstActionPtr->ttiAction.userDefinedCpuCode != secondActionPtr->ttiAction.userDefinedCpuCode)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.userDefinedCpuCode than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.userDefinedCpuCode,
                                 secondActionPtr->ttiAction.userDefinedCpuCode);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.bindToCentralCounter != secondActionPtr->ttiAction.bindToCentralCounter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bindToCentralCounter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bindToCentralCounter,
                             secondActionPtr->ttiAction.bindToCentralCounter);
            st = GT_FAIL;
        }
        if(GT_TRUE == firstActionPtr->ttiAction.bindToCentralCounter)
        {
            if(firstActionPtr->ttiAction.centralCounterIndex != secondActionPtr->ttiAction.centralCounterIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.centralCounterIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.centralCounterIndex,
                                 secondActionPtr->ttiAction.centralCounterIndex);
                st = GT_FAIL;
            }
        }
        if(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.vntl2Echo != secondActionPtr->ttiAction.vntl2Echo)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.vntl2Echo than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.vntl2Echo,
                                 secondActionPtr->ttiAction.vntl2Echo);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.bridgeBypass != secondActionPtr->ttiAction.bridgeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bridgeBypass than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bridgeBypass,
                             secondActionPtr->ttiAction.bridgeBypass);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.ingressPipeBypass != secondActionPtr->ttiAction.ingressPipeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.ingressPipeBypass than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.ingressPipeBypass,
                             secondActionPtr->ttiAction.ingressPipeBypass);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.actionStop != secondActionPtr->ttiAction.actionStop)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.actionStop than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.actionStop,
                             secondActionPtr->ttiAction.actionStop);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.sourceEPortAssignmentEnable != secondActionPtr->ttiAction.sourceEPortAssignmentEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceEPortAssignmentEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.sourceEPortAssignmentEnable,
                             secondActionPtr->ttiAction.sourceEPortAssignmentEnable);
            st = GT_FAIL;
        }
        if(GT_TRUE == firstActionPtr->ttiAction.sourceEPortAssignmentEnable)
        {
            if(firstActionPtr->ttiAction.sourceEPort != secondActionPtr->ttiAction.sourceEPort)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceEPort than was set, expected = %d, received = %d",
                                 name, firstActionPtr->ttiAction.sourceEPort,
                                 secondActionPtr->ttiAction.sourceEPort);
                st = GT_FAIL;
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if(firstActionPtr->ttiAction.cwBasedPw   != secondActionPtr->ttiAction.cwBasedPw)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.cwBasedPw   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.cwBasedPw,
                                                 secondActionPtr->ttiAction.cwBasedPw);
                st = GT_FAIL;
            }
        }

        if((GT_TRUE == firstActionPtr->ttiAction.tunnelTerminate) ||
          (CPSS_DXCH_TTI_PASSENGER_MPLS_E != firstActionPtr->ttiAction.ttPassengerPacketType))
        {
            if(firstActionPtr->ttiAction.ttHeaderLength != secondActionPtr->ttiAction.ttHeaderLength)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.ttHeaderLength than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.ttHeaderLength,
                                                 secondActionPtr->ttiAction.ttHeaderLength);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.continueToNextTtiLookup != secondActionPtr->ttiAction.continueToNextTtiLookup)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.continueToNextTtiLookup than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.continueToNextTtiLookup,
                                             secondActionPtr->ttiAction.continueToNextTtiLookup);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode  != secondActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode )
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.passengerParsingOfTransitMplsTunnelMode  than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode ,
                                             secondActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode );
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable   != secondActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable  )
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable  ,
                                             secondActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable  );
            st = GT_FAIL;
        }

        if(GT_FALSE != firstActionPtr->ttiAction.keepPreviousQoS)
        {
            if(firstActionPtr->ttiAction.qosUseUpAsIndexEnable != secondActionPtr->ttiAction.qosUseUpAsIndexEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosUseUpAsIndexEnable   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.qosUseUpAsIndexEnable  ,
                                                 secondActionPtr->ttiAction.qosUseUpAsIndexEnable  );
                st = GT_FAIL;
            }

            if(GT_FALSE == firstActionPtr->ttiAction.qosUseUpAsIndexEnable)
            {
                if(firstActionPtr->ttiAction.qosMappingTableIndex != secondActionPtr->ttiAction.qosMappingTableIndex)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosUseUpAsIndexEnable   than was set, expected = %d, received = %d\n",
                                                     name, firstActionPtr->ttiAction.qosUseUpAsIndexEnable  ,
                                                     secondActionPtr->ttiAction.qosUseUpAsIndexEnable);
                    st = GT_FAIL;
                }
            }
        }

        if(firstActionPtr->ttiAction.redirectCommand == CPSS_DXCH_TTI_NO_REDIRECT_E &&
            (firstActionPtr->ttiAction.iPclUdbConfigTableEnable   != secondActionPtr->ttiAction.iPclUdbConfigTableEnable  ))
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.iPclUdbConfigTableEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.iPclUdbConfigTableEnable  ,
                                             secondActionPtr->ttiAction.iPclUdbConfigTableEnable  );
            st = GT_FAIL;
        }

        if((CPSS_DXCH_TTI_NO_REDIRECT_E == firstActionPtr->ttiAction.redirectCommand) &&
            firstActionPtr->ttiAction.iPclUdbConfigTableEnable   )
        {
            if(firstActionPtr->ttiAction.iPclUdbConfigTableIndex != secondActionPtr->ttiAction.iPclUdbConfigTableIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.iPclUdbConfigTableIndex   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.iPclUdbConfigTableIndex  ,
                                                 secondActionPtr->ttiAction.iPclUdbConfigTableIndex  );
                st = GT_FAIL;
            }
        }

        if(CPSS_DXCH_TTI_NO_REDIRECT_E == firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.flowId != secondActionPtr->ttiAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.flowId   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.flowId,
                                                 secondActionPtr->ttiAction.flowId);
                st = GT_FAIL;
            }
        }

        if(CPSS_DXCH_TTI_NO_REDIRECT_E == firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.flowId != secondActionPtr->ttiAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.flowId   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.flowId,
                                                 secondActionPtr->ttiAction.flowId);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.setMacToMe   != secondActionPtr->ttiAction.setMacToMe)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.setMacToMe   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.setMacToMe,
                                             secondActionPtr->ttiAction.setMacToMe);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.rxProtectionSwitchEnable   != secondActionPtr->ttiAction.rxProtectionSwitchEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.rxProtectionSwitchEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.rxProtectionSwitchEnable,
                                             secondActionPtr->ttiAction.rxProtectionSwitchEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.rxIsProtectionPath   != secondActionPtr->ttiAction.rxIsProtectionPath)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.rxIsProtectionPath   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.rxIsProtectionPath,
                                             secondActionPtr->ttiAction.rxIsProtectionPath);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.oamProcessEnable   != secondActionPtr->ttiAction.oamProcessEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamOffsetIndex   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.oamOffsetIndex,
                                             secondActionPtr->ttiAction.oamOffsetIndex);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.oamProfile   != secondActionPtr->ttiAction.oamProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamProfile   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.oamProfile,
                                             secondActionPtr->ttiAction.oamProfile);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable   != secondActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamChannelTypeToOpcodeMappingEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable,
                                             secondActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.isPtpPacket   != secondActionPtr->ttiAction.isPtpPacket)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.isPtpPacket   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.isPtpPacket,
                                             secondActionPtr->ttiAction.isPtpPacket);
            st = GT_FAIL;
        }

        if(GT_TRUE == firstActionPtr->ttiAction.isPtpPacket)
        {
            if(firstActionPtr->ttiAction.ptpTriggerType != secondActionPtr->ttiAction.ptpTriggerType)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.ptpTriggerType   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.ptpTriggerType,
                                                 secondActionPtr->ttiAction.ptpTriggerType);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.ptpOffset != secondActionPtr->ttiAction.ptpOffset)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.ptpOffset   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.ptpOffset,
                                                 secondActionPtr->ttiAction.ptpOffset);
                st = GT_FAIL;
            }
        }
        else
        {
            if(firstActionPtr->ttiAction.oamTimeStampEnable   != secondActionPtr->ttiAction.oamTimeStampEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamTimeStampEnable   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.oamTimeStampEnable,
                                                 secondActionPtr->ttiAction.oamTimeStampEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.oamOffsetIndex   != secondActionPtr->ttiAction.oamOffsetIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamOffsetIndex   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.oamOffsetIndex,
                                                 secondActionPtr->ttiAction.oamOffsetIndex);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.ttlExpiryVccvEnable   != secondActionPtr->ttiAction.ttlExpiryVccvEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.ttlExpiryVccvEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.ttlExpiryVccvEnable,
                                             secondActionPtr->ttiAction.ttlExpiryVccvEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.pwe3FlowLabelExist   != secondActionPtr->ttiAction.pwe3FlowLabelExist)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.pwe3FlowLabelExist   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.pwe3FlowLabelExist,
                                             secondActionPtr->ttiAction.pwe3FlowLabelExist);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.pwCwBasedETreeEnable   != secondActionPtr->ttiAction.pwCwBasedETreeEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.pwCwBasedETreeEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.pwCwBasedETreeEnable,
                                             secondActionPtr->ttiAction.pwCwBasedETreeEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.applyNonDataCwCommand   != secondActionPtr->ttiAction.applyNonDataCwCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.applyNonDataCwCommand   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.applyNonDataCwCommand,
                                             secondActionPtr->ttiAction.applyNonDataCwCommand);
            st = GT_FAIL;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if(firstActionPtr->ttiAction.unknownSaCommandEnable   != secondActionPtr->ttiAction.unknownSaCommandEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.unknownSaCommandEnable   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.unknownSaCommandEnable,
                                                 secondActionPtr->ttiAction.unknownSaCommandEnable);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.unknownSaCommand   != secondActionPtr->ttiAction.unknownSaCommand)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.unknownSaCommand   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.unknownSaCommand,
                                                 secondActionPtr->ttiAction.unknownSaCommand);
                st = GT_FAIL;
            }
            if(CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E == firstActionPtr->ttiAction.redirectCommand)
            {
                if(firstActionPtr->ttiAction.sourceMeshIdSetEnable   != secondActionPtr->ttiAction.sourceMeshIdSetEnable)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceMeshIdSetEnable   than was set, expected = %d, received = %d\n",
                                                     name, firstActionPtr->ttiAction.sourceMeshIdSetEnable,
                                                     secondActionPtr->ttiAction.sourceMeshIdSetEnable);
                    st = GT_FAIL;
                }
                if(firstActionPtr->ttiAction.sourceMeshId   != secondActionPtr->ttiAction.sourceMeshId)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceMeshId   than was set, expected = %d, received = %d\n",
                                                     name, firstActionPtr->ttiAction.sourceMeshId,
                                                     secondActionPtr->ttiAction.sourceMeshId);
                    st = GT_FAIL;
                }
            }
        }

        break;

    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            /* verifying values */
            if(firstActionPtr->pclAction.pktCmd != secondActionPtr->pclAction.pktCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.pktCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.pktCmd, secondActionPtr->pclAction.pktCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.actionStop != secondActionPtr->pclAction.actionStop)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.actionStop than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.actionStop, secondActionPtr->pclAction.actionStop);
                st = GT_FAIL;
            }

            /*mirror*/
            if(firstActionPtr->pclAction.mirror.cpuCode != secondActionPtr->pclAction.mirror.cpuCode)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.cpuCode than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.cpuCode, secondActionPtr->pclAction.mirror.cpuCode);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex != secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.ingressMirrorToAnalyzerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex, secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu != secondActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu, secondActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort != secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorToRxAnalyzerPort than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort, secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn != secondActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorToTxAnalyzerPortEn than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn, secondActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex != secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.egressMirrorToAnalyzerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex, secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode != secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.egressMirrorToAnalyzerMode than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode, secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode);
                st = GT_FAIL;
            }

            /*matchCounter*/
            if(firstActionPtr->pclAction.matchCounter.enableMatchCount != secondActionPtr->pclAction.matchCounter.enableMatchCount)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.matchCounter.enableMatchCount than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.matchCounter.enableMatchCount, secondActionPtr->pclAction.matchCounter.enableMatchCount);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.matchCounter.matchCounterIndex != secondActionPtr->pclAction.matchCounter.matchCounterIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.matchCounter.matchCounterIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.matchCounter.matchCounterIndex, secondActionPtr->pclAction.matchCounter.matchCounterIndex);
                st = GT_FAIL;
            }

            /*policer*/
            if(firstActionPtr->pclAction.policer.policerEnable != secondActionPtr->pclAction.policer.policerEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.policer.policerEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.policer.policerEnable, secondActionPtr->pclAction.policer.policerEnable);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.policer.policerId != secondActionPtr->pclAction.policer.policerId)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.policer.policerId than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.policer.policerId, secondActionPtr->pclAction.policer.policerId);
                st = GT_FAIL;
            }

            /*vlan*/
            isEqual = (0 == cpssOsMemCmp(
                  (GT_VOID*) &firstActionPtr->pclAction.vlan.egress,
                  (GT_VOID*) &secondActionPtr->pclAction.vlan.egress,
                  sizeof(secondActionPtr->pclAction.vlan.egress))) ? GT_TRUE : GT_FALSE;

            if(isEqual != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.vlan.egress than was set\n");

                st = GT_FAIL;
            }
            isEqual = (0 == cpssOsMemCmp(
                  (GT_VOID*) &firstActionPtr->pclAction.vlan.ingress,
                  (GT_VOID*) &secondActionPtr->pclAction.vlan.ingress,
                  sizeof(secondActionPtr->pclAction.vlan.ingress))) ? GT_TRUE : GT_FALSE;

            if(isEqual != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.vlan.ingress than was set\n");

                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.egressPolicy != secondActionPtr->pclAction.egressPolicy)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.egressPolicy than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.egressPolicy, secondActionPtr->pclAction.egressPolicy);
                st = GT_FAIL;
            }


           /*redirect*/

            if(firstActionPtr->pclAction.redirect.redirectCmd != secondActionPtr->pclAction.redirect.redirectCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.redirectCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.redirectCmd, secondActionPtr->pclAction.redirect.redirectCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.tunnelPtr != secondActionPtr->pclAction.redirect.data.outIf.tunnelPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.tunnelPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.tunnelPtr, secondActionPtr->pclAction.redirect.data.outIf.tunnelPtr);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.tunnelStart != secondActionPtr->pclAction.redirect.data.outIf.tunnelStart)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.tunnelStart than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.tunnelStart, secondActionPtr->pclAction.redirect.data.outIf.tunnelStart);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.tunnelType != secondActionPtr->pclAction.redirect.data.outIf.tunnelType)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.tunnelType than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.tunnelType, secondActionPtr->pclAction.redirect.data.outIf.tunnelType);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.vntL2Echo != secondActionPtr->pclAction.redirect.data.outIf.vntL2Echo)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.vntL2Echo than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.vntL2Echo, secondActionPtr->pclAction.redirect.data.outIf.vntL2Echo);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort != secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorToRxAnalyzerPort than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort, secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex != secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.ingressMirrorToAnalyzerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex, secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.redirectCmd != secondActionPtr->pclAction.redirect.redirectCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.redirectCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.redirectCmd, secondActionPtr->pclAction.redirect.redirectCmd);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp(
                  (GT_VOID*) &firstActionPtr->pclAction.redirect.data.modifyMacSa.macSa,
                  (GT_VOID*) &secondActionPtr->pclAction.redirect.data.modifyMacSa.macSa,
                  sizeof(firstActionPtr->pclAction.redirect.data.modifyMacSa.macSa))) ? GT_TRUE : GT_FALSE;

            if(isEqual != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.redirect.data.modifyMacSa.macSa than was set\n");
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr != secondActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.modifyMacSa.arpPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr, secondActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.timeStampEnable != secondActionPtr->pclAction.oam.timeStampEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.timeStampEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.timeStampEnable, secondActionPtr->pclAction.oam.timeStampEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.offsetIndex != secondActionPtr->pclAction.oam.offsetIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.offsetIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.offsetIndex, secondActionPtr->pclAction.oam.offsetIndex);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.oamProcessEnable != secondActionPtr->pclAction.oam.oamProcessEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.oamProcessEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.oamProcessEnable, secondActionPtr->pclAction.oam.oamProcessEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.oamProfile != secondActionPtr->pclAction.oam.oamProfile)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.oamProfile than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.oamProfile, secondActionPtr->pclAction.oam.oamProfile);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.sourcePort.assignSourcePortEnable != secondActionPtr->pclAction.sourcePort.assignSourcePortEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.sourcePort.assignSourcePortEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.sourcePort.assignSourcePortEnable, secondActionPtr->pclAction.sourcePort.assignSourcePortEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.sourcePort.sourcePortValue != secondActionPtr->pclAction.sourcePort.sourcePortValue)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.sourcePort.sourcePortValue than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.sourcePort.sourcePortValue, secondActionPtr->pclAction.sourcePort.sourcePortValue);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.ingress.up1Cmd != secondActionPtr->pclAction.qos.ingress.up1Cmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.ingress.up1Cmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.ingress.up1Cmd, secondActionPtr->pclAction.qos.ingress.up1Cmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.ingress.up1 != secondActionPtr->pclAction.qos.ingress.up1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.pclAction.qos.ingress.up1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.ingress.up1, secondActionPtr->pclAction.qos.ingress.up1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.ingress.vlanId1Cmd != secondActionPtr->pclAction.vlan.ingress.vlanId1Cmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.ingress.vlanId1Cmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.ingress.vlanId1Cmd, secondActionPtr->pclAction.vlan.ingress.vlanId1Cmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.ingress.vlanId1 != secondActionPtr->pclAction.vlan.ingress.vlanId1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.ingress.vlanId1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.ingress.vlanId1, secondActionPtr->pclAction.vlan.ingress.vlanId1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.flowId != secondActionPtr->pclAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.flowId than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.flowId, secondActionPtr->pclAction.flowId);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.setMacToMe != secondActionPtr->pclAction.setMacToMe)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.setMacToMe than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.setMacToMe, secondActionPtr->pclAction.setMacToMe);
                st = GT_FAIL;
            }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            /* verifying values */
            /*pktCmd*/

            if(firstActionPtr->pclAction.pktCmd != secondActionPtr->pclAction.pktCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.pktCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.pktCmd, secondActionPtr->pclAction.pktCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.actionStop != secondActionPtr->pclAction.actionStop)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.actionStop than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.actionStop, secondActionPtr->pclAction.actionStop);
                st = GT_FAIL;
            }

            /*matchCounter*/
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&firstActionPtr->pclAction.matchCounter,
                                            (const GT_VOID*)&secondActionPtr->pclAction.matchCounter,
                                            sizeof(firstActionPtr->pclAction.matchCounter))) ? GT_FALSE : GT_TRUE;
            if(failureWas == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.matchCounter than was set\n");
                st = GT_FAIL;
            }

            /*policer*/
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&firstActionPtr->pclAction.policer,
                                            (const GT_VOID*)&secondActionPtr->pclAction.policer,
                                            sizeof(firstActionPtr->pclAction.policer))) ? GT_FALSE : GT_TRUE;
            if(failureWas == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.policer than was set\n");
                st = GT_FAIL;
            }

            /*vlan*/
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&firstActionPtr->pclAction.vlan,
                                            (const GT_VOID*)&secondActionPtr->pclAction.vlan,
                                            sizeof(firstActionPtr->pclAction.vlan))) ? GT_FALSE : GT_TRUE;
            if(failureWas == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.vlan than was set\n");
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.egressPolicy != secondActionPtr->pclAction.egressPolicy)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.egressPolicy than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.egressPolicy, secondActionPtr->pclAction.egressPolicy);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.modifyDscp != secondActionPtr->pclAction.qos.egress.modifyDscp)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.modifyDscp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.modifyDscp, secondActionPtr->pclAction.qos.egress.modifyDscp);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.modifyUp != secondActionPtr->pclAction.qos.egress.modifyUp)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.modifyUp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.modifyUp, secondActionPtr->pclAction.qos.egress.modifyUp);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.up1ModifyEnable != secondActionPtr->pclAction.qos.egress.up1ModifyEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.up1ModifyEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.up1ModifyEnable, secondActionPtr->pclAction.qos.egress.up1ModifyEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.up1 != secondActionPtr->pclAction.qos.egress.up1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.up1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.up1, secondActionPtr->pclAction.qos.egress.up1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.egress.vlanCmd != secondActionPtr->pclAction.vlan.egress.vlanCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.egress.vlanCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.egress.vlanCmd, secondActionPtr->pclAction.vlan.egress.vlanCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable != secondActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.egress.vlanId1ModifyEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable, secondActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.egress.vlanId1 != secondActionPtr->pclAction.vlan.egress.vlanId1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.egress.vlanId1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.egress.vlanId1, secondActionPtr->pclAction.vlan.egress.vlanId1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.channelTypeToOpcodeMapEnable != secondActionPtr->pclAction.channelTypeToOpcodeMapEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.channelTypeToOpcodeMapEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.channelTypeToOpcodeMapEnable, secondActionPtr->pclAction.channelTypeToOpcodeMapEnable);
                st = GT_FAIL;
            }

            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported)
            {

                if(firstActionPtr->pclAction.tmQueueId != secondActionPtr->pclAction.tmQueueId)
                {
                    PRV_UTF_LOG3_MAC("get another %s.pclAction.tmQueueId than was set, expected = %d, received = %d\n",
                                     name, firstActionPtr->pclAction.tmQueueId, secondActionPtr->pclAction.tmQueueId);
                    st = GT_FAIL;
                }
            }
        break;
    default:
        break;
    }

    return st;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileDefaultActionSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                  defActionEn
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileDefaultActionSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchProfileDefaultActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call with wrong values exactMatchProfileIndex, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum values actionType, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with actionPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.

    1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.6. Call function with
            actionPtr { egressPolicy [GT_FALSE]
                        mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                        redirect {
                           redirectCmd [
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                           data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                      A5:A5:A5:A5:A5:A5 /
                                                      FF:FF:FF:FF:FF:FF],
                                               arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                        oam { timeStampEnable [GT_FALSE / GT_TRUE],
                              offsetIndex [0 / 0xA / BIT_4-1],
                              oamProcessEnable [GT_FALSE / GT_TRUE],
                              oamProfile [0 / 1] },
                        sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                 sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                        qos { ingress { up1Cmd[
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                       up1[0 / 5 / BIT_3-1]} }
                        vlan { ingress { vlanId1Cmd[
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        flowId [0 / 0xA5A / BIT_12-1],
                        setMacToMe [GT_FALSE / GT_TRUE] }
            and other params from 1.1.
    Expected: GT_OK and same action as written
    1.7. Call function with
            actionPtr { egressPolicy [GT_TRUE]
                        qos { egress { modifyDscp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                      modifyUp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                      up1ModifyEnable [GT_FALSE / GT_TRUE],
                                      up1[0 / 5 / BIT_3-1]} }
                        vlan { egress { vlanCmd[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                       vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                        tmQueueId [0 / 0x2A5A / BIT_14-1] }
            and other params from 1.41.
    Expected: GT_OK and same action as written
*/
    GT_STATUS                                st = GT_OK;
    GT_U8                                    devNum = 0;
    GT_U32                                   exactMatchProfileIndex = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionDataGet;
    GT_BOOL                                  defActionEn = GT_TRUE;
    GT_BOOL                                  defActionEnGet;
    GT_ETHERADDR                             macSa1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */
        cpssOsBzero((GT_VOID*) &defaultActionData, sizeof(defaultActionData));
        cpssOsBzero((GT_VOID*) &defaultActionDataGet, sizeof(defaultActionDataGet));

        exactMatchProfileIndex = 2;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
        defActionEn = GT_TRUE;

        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                         devNum, exactMatchProfileIndex, actionType, defActionEn);

        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);

       /*
            1.2. Call with out of range exactMatchProfileIndex [16],
                           other params same as in 1.1.
            Expected: non GT_OK.
        */
        exactMatchProfileIndex = 16;

        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchProfileIndex, actionType);

        /*
            1.3. Call with wrong enum values actionType, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        exactMatchProfileIndex=3;
        UTF_ENUMS_CHECK_MAC(cpssDxChExactMatchProfileDefaultActionSet
                            (devNum, exactMatchProfileIndex, actionType, &defaultActionData, defActionEn),
                            actionType);

        /*
            1.4. Call with actionPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,NULL,defActionEn);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, defaultActionData = NULL", devNum);

        /*
           1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        exactMatchProfileIndex = 5;

        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                         devNum, exactMatchProfileIndex, actionType, defActionEn);

        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);

        /*
          1.6. Call function with
                actionPtr { egressPolicy [GT_FALSE]
                            mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                            redirect {
                               redirectCmd [
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                               data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                          A5:A5:A5:A5:A5:A5 /
                                                          FF:FF:FF:FF:FF:FF],
                                                   arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                            oam { timeStampEnable [GT_FALSE / GT_TRUE],
                                  offsetIndex [0 / 0xA / BIT_4-1],
                                  oamProcessEnable [GT_FALSE / GT_TRUE],
                                  oamProfile [0 / 1] },
                            sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                     sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                            qos { ingress { up1Cmd[
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                           up1[0 / 5 / BIT_3-1]} }
                            vlan { ingress { vlanId1Cmd[
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            flowId [0 / 0xA5A / BIT_12-1],
                            setMacToMe [GT_FALSE / GT_TRUE] }
                and other params from 1.5.
        Expected: GT_OK and same action as written
        */

            defaultActionData.pclAction.egressPolicy = GT_FALSE;
            defaultActionData.pclAction.mirror.mirrorToRxAnalyzerPort = GT_TRUE;

            defaultActionData.pclAction.mirror.ingressMirrorToAnalyzerIndex = 0;
            defaultActionData.pclAction.redirect.redirectCmd =
                                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            defaultActionData.pclAction.redirect.data.modifyMacSa.macSa = macSa1;
            defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            defaultActionData.pclAction.oam.timeStampEnable = GT_FALSE;
            defaultActionData.pclAction.oam.offsetIndex = 0;
            defaultActionData.pclAction.oam.oamProcessEnable = GT_FALSE;
            defaultActionData.pclAction.oam.oamProfile = 0;
            defaultActionData.pclAction.sourcePort.assignSourcePortEnable = GT_FALSE;
            defaultActionData.pclAction.sourcePort.sourcePortValue = 0;
            defaultActionData.pclAction.qos.ingress.up1Cmd =
                           CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.qos.ingress.up1 = 0;
            defaultActionData.pclAction.vlan.ingress.vlanId1Cmd =
                      CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.vlan.ingress.vlanId1 = 0;
            defaultActionData.pclAction.flowId = 0;
            defaultActionData.pclAction.setMacToMe = GT_FALSE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

            st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                             devNum, exactMatchProfileIndex, actionType, defActionEn);

            st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);

            /*1.7. Call function with
                actionPtr { egressPolicy [GT_TRUE]
                            qos { egress { modifyDscp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                          modifyUp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                          up1ModifyEnable [GT_FALSE / GT_TRUE],
                                          up1[0 / 5 / BIT_3-1]} }
                            vlan { egress { vlanCmd[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                           vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                            tmQueueId [0 / 0x2A5A / BIT_14-1] }
                and other params from 1.6.
            Expected: GT_OK and same action as written */

            defaultActionData.pclAction.egressPolicy = GT_TRUE;
            defaultActionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            defaultActionData.pclAction.actionStop = GT_FALSE;
            defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            defaultActionData.pclAction.sourcePort.sourcePortValue = 0;

            defaultActionData.pclAction.qos.egress.modifyDscp =
                                CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
            defaultActionData.pclAction.qos.egress.modifyUp =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.qos.egress.up1ModifyEnable = GT_FALSE;
            defaultActionData.pclAction.qos.egress.up1 = 0;
            defaultActionData.pclAction.vlan.egress.vlanCmd =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.vlan.egress.vlanId1ModifyEnable = GT_FALSE;
            defaultActionData.pclAction.vlan.egress.vlanId1 = 0;
            defaultActionData.pclAction.channelTypeToOpcodeMapEnable = GT_FALSE;
            defaultActionData.pclAction.tmQueueId = 0;


            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

            st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                             devNum, exactMatchProfileIndex, actionType, defActionEn);

            st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);




    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   exactMatchProfileIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    OUT GT_BOOL                                  *defActionEnPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileDefaultActionGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null actionPtr, defActionEnPtr
    Expected: GT_OK.
    1.2. Call with non null actionPtr, defActionEnPtr and exactMatchProfileIndex=16
    Expected: GT_BAD_PARAM.
    1.3. Call with actionPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with defActionEnPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                st = GT_OK;
    GT_U8                                    devNum = 0;
    GT_U32                                   exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionDataGet;
    GT_BOOL                                  defActionEnGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        /*
             1.1. Call with non null actionPtr, defActionEnPtr
            Expected: GT_OK.
        */
        exactMatchProfileIndex=3;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /*
             1.2. Call with non null actionPtr, defActionEnPtr and exactMatchProfileIndex=16
            Expected: GT_BAD_PARAM.
        */
        exactMatchProfileIndex=16;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with actionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=15;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,NULL,&defActionEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

        /*
             1.4. Call with defActionEnPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=15;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    exactMatchProfileIndex = 1;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
* @internal prvCompareExactMatchExpandedStructs function
* @endinternal
*
* @brief   This routine set compares 2 useExpanded structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
* @param[in] actionType               - type of the action to use
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
GT_STATUS prvCompareExactMatchExpandedStructs
(
    IN GT_CHAR                                              *name,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT     *firstActionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT     *secondActionPtr
)
{
    GT_STATUS   st = GT_OK;

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionVrfId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionFlowId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionArpPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable!= secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSourceId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionActionStop than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0 != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTrustUp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTrustExp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag0Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpfixEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpfixEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpfixEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpfixEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpfixEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReservedAssignmentEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReservedAssignmentEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCopyReservedAssignmentEnable than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReservedAssignmentEnable,
                                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReservedAssignmentEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReserved != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReserved)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCopyReserved than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReserved,
                                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyReserved);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient,
                                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient);
            st = GT_FAIL;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPktCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa!= secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMacSa than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionArpPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVrfId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionActionStop than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionLookupConfig than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMirrorMode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode);
            st = GT_FAIL;
        }
         if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMirror than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionBypassBridge than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan1 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSourceId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUp1Command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUp1 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionDscpExp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUp0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReservedEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReservedEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpendedActionCopyReservedEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReservedEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReservedEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReserved != secondActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReserved)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpendedActionCopyReserved than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReserved,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpendedActionCopyReserved);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerHashCncClient);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressCncIndexMode != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressCncIndexMode)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionEgressCncIndexMode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressCncIndexMode,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressCncIndexMode);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEnableEgressMaxSduSizeCheck != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEnableEgressMaxSduSizeCheck)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionEnableEgressMaxSduSizeCheck than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEnableEgressMaxSduSizeCheck,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEnableEgressMaxSduSizeCheck);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressMaxSduSizeProfile != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressMaxSduSizeProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionEgressMaxSduSizeProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressMaxSduSizeProfile,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressMaxSduSizeProfile);
            st = GT_FAIL;
        }
        break;
    default:
        st = GT_BAD_PARAM;
        break;
    }
    return st;
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchExpandedActionSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call with wrong values expandedActionIndex, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum values actionType, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with actionPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.

    1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.6. Call function with
            actionPtr { egressPolicy [GT_FALSE]
                        mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                        redirect {
                           redirectCmd [
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                           data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                      A5:A5:A5:A5:A5:A5 /
                                                      FF:FF:FF:FF:FF:FF],
                                               arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                        oam { timeStampEnable [GT_FALSE / GT_TRUE],
                              offsetIndex [0 / 0xA / BIT_4-1],
                              oamProcessEnable [GT_FALSE / GT_TRUE],
                              oamProfile [0 / 1] },
                        sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                 sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                        qos { ingress { up1Cmd[
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                       up1[0 / 5 / BIT_3-1]} }
                        vlan { ingress { vlanId1Cmd[
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        flowId [0 / 0xA5A / BIT_12-1],
                        setMacToMe [GT_FALSE / GT_TRUE] }
            and other params from 1.1.
    Expected: GT_OK and same action as written
    1.7. Call function with
            actionPtr { egressPolicy [GT_TRUE]
                        qos { egress { modifyDscp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                      modifyUp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                      up1ModifyEnable [GT_FALSE / GT_TRUE],
                                      up1[0 / 5 / BIT_3-1]} }
                        vlan { egress { vlanCmd[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                       vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                        tmQueueId [0 / 0x2A5A / BIT_14-1] }
            and other params from 1.41.
    Expected: GT_OK and same action as written

    1.8. Call with more than 8 reduced bytes. other params same as in 1.1.
         Expected: GT_BAD_PARAM.
    */
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    GT_U32                                          expandedActionIndex = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   expandedActionOriginDataGet;

    GT_ETHERADDR                             macSa1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */
        expandedActionIndex = 2;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

       /*
            1.2. Call with out of range expandedActionIndex [16],
                           other params same as in 1.1.
            Expected: non GT_OK.
        */
        expandedActionIndex = 16;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, expandedActionIndex, actionType);

        /*
            1.3. Call with wrong enum values actionType, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        expandedActionIndex=3;
        UTF_ENUMS_CHECK_MAC(cpssDxChExactMatchExpandedActionSet
                            (devNum, expandedActionIndex, actionType, &actionData, &expandedActionOriginDataGet),
                            actionType);

        /*
            1.4. Call with actionPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,NULL,&expandedActionOriginData);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, actionData = NULL", devNum);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, expandedActionOriginData = NULL", devNum);

        /*
           1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
          1.6. Call function with
                actionPtr { egressPolicy [GT_FALSE]
                            mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                            redirect {
                               redirectCmd [
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                               data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                          A5:A5:A5:A5:A5:A5 /
                                                          FF:FF:FF:FF:FF:FF],
                                                   arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                            oam { timeStampEnable [GT_FALSE / GT_TRUE],
                                  offsetIndex [0 / 0xA / BIT_4-1],
                                  oamProcessEnable [GT_FALSE / GT_TRUE],
                                  oamProfile [0 / 1] },
                            sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                     sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                            qos { ingress { up1Cmd[
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                           up1[0 / 5 / BIT_3-1]} }
                            vlan { ingress { vlanId1Cmd[
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            flowId [0 / 0xA5A / BIT_12-1],
                            setMacToMe [GT_FALSE / GT_TRUE] }
                and other params from 1.5.
        Expected: GT_OK and same action as written
        */

            actionData.pclAction.egressPolicy = GT_FALSE;
            actionData.pclAction.mirror.mirrorToRxAnalyzerPort = GT_TRUE;

            actionData.pclAction.mirror.ingressMirrorToAnalyzerIndex = 0;
            actionData.pclAction.redirect.redirectCmd =
                                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            actionData.pclAction.redirect.data.modifyMacSa.macSa = macSa1;
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            actionData.pclAction.oam.timeStampEnable = GT_FALSE;
            actionData.pclAction.oam.offsetIndex = 0;
            actionData.pclAction.oam.oamProcessEnable = GT_FALSE;
            actionData.pclAction.oam.oamProfile = 0;
            actionData.pclAction.sourcePort.assignSourcePortEnable = GT_FALSE;
            actionData.pclAction.sourcePort.sourcePortValue = 0;
            actionData.pclAction.qos.ingress.up1Cmd =
                           CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.qos.ingress.up1 = 0;
            actionData.pclAction.vlan.ingress.vlanId1Cmd =
                      CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.vlan.ingress.vlanId1 = 0;
            actionData.pclAction.flowId = 0;
            actionData.pclAction.setMacToMe = GT_FALSE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

            cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
            cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadata=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /*1.7. Call function with
                actionPtr { egressPolicy [GT_TRUE]
                            qos { egress { modifyDscp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                          modifyUp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                          up1ModifyEnable [GT_FALSE / GT_TRUE],
                                          up1[0 / 5 / BIT_3-1]} }
                            vlan { egress { vlanCmd[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                           vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                            tmQueueId [0 / 0x2A5A / BIT_14-1] }
                and other params from 1.6.
            Expected: GT_OK and same action as written */

            actionData.pclAction.egressPolicy = GT_TRUE;
            actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            actionData.pclAction.actionStop = GT_FALSE;
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            actionData.pclAction.sourcePort.sourcePortValue = 0;

            actionData.pclAction.qos.egress.modifyDscp =
                                CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
            actionData.pclAction.qos.egress.modifyUp =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.qos.egress.up1ModifyEnable = GT_FALSE;
            actionData.pclAction.qos.egress.up1 = 0;
            actionData.pclAction.vlan.egress.vlanCmd =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.vlan.egress.vlanId1ModifyEnable = GT_FALSE;
            actionData.pclAction.vlan.egress.vlanId1 = 0;
            actionData.pclAction.channelTypeToOpcodeMapEnable = GT_FALSE;
            actionData.pclAction.tmQueueId = 0;

            cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
            cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* 1.8. Call with more 9 reduced bytes. other params same as in 1.1.
            Expected: GT_BAD_PARAM.*/
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_TRUE;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* remove one byte from the reduced */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_FALSE;
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_FALSE;
            }

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchExpandedActionSpecificScenarioSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchExpandedActionSpecificScenarioSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written

    Call with more than 8 reduced bytes. other params same as in 1.1.
         Expected: GT_BAD_PARAM.
    Change flowIdMask and Call with more than 8 reduced bytes. other params same as in 1.1.
         Expected: GT_OK, flowId will take one byte from the reduced and one from expander.


    Repeat this method for all fields with Mask.


    1.2. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.3. Call function with
            actionPtr { egressPolicy [GT_FALSE]
                        mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                        redirect {
                           redirectCmd [
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                           data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                      A5:A5:A5:A5:A5:A5 /
                                                      FF:FF:FF:FF:FF:FF],
                                               arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                        oam { timeStampEnable [GT_FALSE / GT_TRUE],
                              offsetIndex [0 / 0xA / BIT_4-1],
                              oamProcessEnable [GT_FALSE / GT_TRUE],
                              oamProfile [0 / 1] },
                        sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                 sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                        qos { ingress { up1Cmd[
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                       up1[0 / 5 / BIT_3-1]} }
                        vlan { ingress { vlanId1Cmd[
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        flowId [0 / 0xA5A / BIT_12-1],
                        setMacToMe [GT_FALSE / GT_TRUE] }
            and other params from 1.1.
    Expected: GT_OK and same action as written
    1.4. Call function with
            actionPtr { egressPolicy [GT_TRUE]
                        qos { egress { modifyDscp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                      modifyUp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                      up1ModifyEnable [GT_FALSE / GT_TRUE],
                                      up1[0 / 5 / BIT_3-1]} }
                        vlan { egress { vlanCmd[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                       vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                        tmQueueId [0 / 0x2A5A / BIT_14-1] }
            and other params from 1.41.
    Expected: GT_OK and same action as written

    1.5. Call with more than 8 reduced bytes. other params same as in 1.1.
         Expected: GT_BAD_PARAM.

     Change flowIdMask and Call with more than 8 reduced bytes. other params same as in 1.2.
         Expected: GT_OK, flowId will take one byte from the reduced and one from expander.

    Repeat this method for all fields with Mask.

    */
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    GT_U32                                          expandedActionIndex = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   expandedActionOriginDataGet;
    GT_ETHERADDR                             macSa1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */
        expandedActionIndex = 2;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;
        actionData.ttiAction.flowId = 255;

        expandedActionOriginData.ttiExpandedActionOrigin.flowIdMask=0;

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change flowId to be !=0 - expect GT_OK - flowIdMask 127 will take only 1 byte in the reduced action */
        expandedActionOriginData.ttiExpandedActionOrigin.flowIdMask = 127;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field vrfIdMask */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionVrfId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;
        actionData.ttiAction.vrfId = 255;

        expandedActionOriginData.ttiExpandedActionOrigin.vrfIdMask=0;

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change vrfIdMask to be !=0 - expect GT_OK - vrfIdMask 127 will take only 1 byte in the reduced action */
        expandedActionOriginData.ttiExpandedActionOrigin.vrfIdMask = 127;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field egressInterfaceMask
           evidx [32:17] -Redirect_Command="Egress_Interface" and UseVIDX = "1" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_FALSE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionData.ttiAction.egressInterface.type=CPSS_INTERFACE_VIDX_E;
        actionData.ttiAction.egressInterface.vidx=0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

       /* change egressInterfaceMask to be !=0 - expect GT_OK - egressInterfaceMask 127 will take only 1 byte in the reduced action
             *  bits 17-32 eVidx
             *      bit [16] useVidx + bits [23:17] are placed in one byte
             *      bits [31:24] are placed in another byte
             *      bits [32]    is placed in another byte
             *
             *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
             *  if any bit of egressInterfaceMask[6:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
             *  if any bit of egressInterfaceMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
             *  if any bit of egressInterfaceMask[15] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
             *  any combination of the bytes must include the first byte since the useVidx is located in the first byte and will always be set
             */
        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x80;/* take only one byte - the middle one - should fail */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x8000;/* take only one byte - the last one - should fail */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x8001;/* bit 0 and 15 on - want to take byte1 and byte3 - should pass*/

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x7F;/* take only one byte - the first one - should pass */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field egressInterfaceMask
           ip_next_hop_entry_index [34:17] -  Redirect_Command="IPNextHop" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
        actionData.ttiAction.routerLttPtr=0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.routerLttPtrMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change routerLttPtrMask to be !=0 - expect GT_OK - routerLttPtrMask 127 will take only 1 byte in the reduced action
            *  routerLttPtr attribute in TTI action is located in bits [34:17]
            *  bits [23:17] are placed in one byte
            *  bits [31:24] are placed in another byte
            *  bits [34:32] are placed in another byte
            *
            *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
            *  if any bit of routerLttPtrMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of routerLttPtrMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *  if any bit of routerLttPtrMask[17:15] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        expandedActionOriginData.ttiExpandedActionOrigin.routerLttPtrMask=0x8001;/* bit 0 and 15 on - want to take byte1 and byte3 - should pass - 8 bytes*/

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.routerLttPtrMask=0x7F;/* take only one byte - the first one - should pass 7 bytes */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

          expandedActionOriginData.ttiExpandedActionOrigin.routerLttPtrMask=0x80;/* take only one byte - the middle one - should pass 7 bytes*/

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.routerLttPtrMask=0x8000;/* take only one byte - the last one - should pass 7 bytes */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field egressInterfaceMask
           trunk_id [29:18] Redirect_Command="Egress_Interface" and UseVIDX = "0" and IsTrunk="True" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_FALSE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionData.ttiAction.egressInterface.type=CPSS_INTERFACE_TRUNK_E;
        actionData.ttiAction.egressInterface.trunkId=0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change egressInterfaceMask to be !=0 - expect GT_OK - egressInterfaceMask 127 will take only 1 byte in the reduced action
        *  egress Interface attribute in IPCL action is located in:
        *  bits 18-29 trunk ID
        *  bits [23:18] are placed in one byte
        *  bits [29:24] are placed in another byte
        *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
        *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
        *  if any bit of egressInterfaceMask[11:6]  - this option is not supported - we can not set only second
        *  byte since the isTrunk bit 17 is located in the first byte and will always be set when setting trunkId
        */
        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x40;/* take only one byte - the second one - should fail */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x80;/* take only one byte - the middle one - should fail */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x3F;/* take byte1 - should pass*/

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);


        /* check field egressInterfaceMask
           trg_eport [31:18] - Redirect_Command="Egress_Interface" and UseVIDX = "0" and IsTrunk="False" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_FALSE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionData.ttiAction.egressInterface.type=CPSS_INTERFACE_PORT_E;
        actionData.ttiAction.egressInterface.devPort.hwDevNum=0x10;
        actionData.ttiAction.egressInterface.devPort.portNum=0x20;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change egressInterfaceMask to be !=0 - expect GT_OK - egressInterfaceMask 127 will take only 1 byte in the reduced action
            *  egress Interface attribute in TTI action is located in:
            *  bits 18-31 target ePort
            *  bits [23:18] are placed in one byte
            *  bits [31:24] are placed in another byte
            *  0 or a value>=BIT_14- use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of egressInterfaceMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *  we can not set only second byte since the isTrunk(bit 17) and useVidx(bit 16) are located in the first byte
            *  and will always be set when setting eport
           */
        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x40;/* take only one byte - the second one - should fail */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.egressInterfaceMask=0x3F;/* take byte1 - should pass*/

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field tunnelIndexMask
                   tunnel_pointer [60:45] - Redirect_Command="Egress_Interface" and TunnelStart="True" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionData.ttiAction.tunnelStart=GT_TRUE;
        actionData.ttiAction.tunnelStartPtr = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change tunnelIndexMask to be !=0 - expect GT_OK - tunnelIndexMask 127 will take only 1 byte in the reduced action
            *  tunnelIndex attribute in TTI action is located in bits [45:60]
            *  bits [47:45] are placed in one byte
            *  bits [55:48] are placed in another byte
            *  bits [60:56] are placed in another byte
            *
            *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
            *  if any bit of tunnelIndexMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of tunnelIndexMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *  if any bit of tunnelIndexMask[15:11] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=9;/* take all 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=0x801;/* take all 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=0x808;/* take all 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=0x7;/* take byte1 - should pass - 7 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=0x8;/* take only one byte - the second one - should pass- 7 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

         expandedActionOriginData.ttiExpandedActionOrigin.tunnelIndexMask=0x800;/* take only one byte - the third one - should pass */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field arpPtrMask
                   arp_pointer [62:45] - Redirect_Command="Egress_Interface" and TunnelStart="False" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionArpPtr=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionData.ttiAction.tunnelStart=GT_FALSE;
        actionData.ttiAction.arpPtr = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change arpPtr to be !=0 - expect GT_OK - arpPtr 127 will take only 1 byte in the reduced action
        *  arpPtr attribute in TTI action is located in bits [45:62]
        *  bits [47:45] are placed in one byte
        *  bits [55:48] are placed in another byte
        *  bits [62:56] are placed in another byte
        *
        *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
        *  if any bit of arpPtrMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
        *  if any bit of arpPtrMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
        *  if any bit of arpPtrMask[17:11] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
        */
        expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=9;/* take all 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=0x801;/* take all 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=0x808;/* take all 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=0x7;/* take byte1 - should pass - 7 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=0x8;/* take only one byte - the second one - should pass- 7 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

         expandedActionOriginData.ttiExpandedActionOrigin.arpPtrMask=0x800;/* take only one byte - the third one - should pass */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field ipclConfigIndex
           ipcl_profile_index [64:52] - Redirect_Command !="Egress_Interface" */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        /* for sip6_10 the fields are set different in the bytes need to adjust the logic to get
           more than 8 bytes in the reduced entry  */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo=GT_TRUE;
            expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;
        }
        else
        {
            expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
            expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_FALSE;
        }
        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.iPclConfigIndex = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.ipclConfigIndexMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change ipclConfigIndexMask to be !=0 - expect GT_OK
            *  ipclConfigIndex attribute in TTI action is located in bits [52:64]
            *  bits [55:52] are placed in one byte
            *  bits [63:56] are placed in another byte
            *  bits [64] are placed in another byte
            *
            *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
            *  if any bit of ipclConfigIndexMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of ipclConfigIndexMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *  if any bit of ipclConfigIndexMask[12] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        expandedActionOriginData.ttiExpandedActionOrigin.ipclConfigIndexMask=0x11;/* take 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.ipclConfigIndexMask=0x1001;/* take 2 bytes - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field centralCounterIndexMask
           counter_index [79:66] */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.centralCounterIndex = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.centralCounterIndexMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change centralCounterIndexMask to be !=0 - expect GT_OK
            *  centralCounterIndex attribute in TTI action is located in bits [66-79]
            *  bits [71:66] are placed in one byte
            *  bits [79:72] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of centralCounterIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of centralCounterIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *
            *  For AC5X, AC5P, Harrier:
            *  centralCounterIndex attribute in TTI action is located in bits [81-95]
            *  bits [81:87] are placed in one byte
            *  bits [88:95] are placed in another byte
            *
            *  0 or a value>=BIT_15 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of centralCounterIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of centralCounterIndexMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            expandedActionOriginData.ttiExpandedActionOrigin.centralCounterIndexMask=0x40;/* take first byte - should pass 8 bytes in reduced */
        }
        else
        {
            expandedActionOriginData.ttiExpandedActionOrigin.centralCounterIndexMask=0x20;/* take first byte - should pass 8 bytes in reduced */
        }

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            expandedActionOriginData.ttiExpandedActionOrigin.centralCounterIndexMask=0x80;/* take second byte - should pass 8 bytes in reduced */
        }
        else
        {
            expandedActionOriginData.ttiExpandedActionOrigin.centralCounterIndexMask=0x40;/* take second byte - should pass 8 bytes in reduced */
        }

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field policerIndexMask
           policer_ptr [95:82] */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.bindToPolicer=GT_TRUE;
        actionData.ttiAction.policerIndex = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceIdSetEnable =GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.policerIndexMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change policerIndexMask to be !=0 - expect GT_OK
            *  PolicerIndex attribute in TTI action is located in bits [82-95]
            *  bits [87:82] are placed in one byte
            *  bits [95:88] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of policerIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of policerIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
           */
        expandedActionOriginData.ttiExpandedActionOrigin.policerIndexMask=0x1;/* take first byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.policerIndexMask=0x40;/* take second byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field sourceIdMask
           sst_id [108:97] */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable=GT_FALSE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.sourceIdSetEnable = GT_TRUE;
        actionData.ttiAction.sourceId = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.sourceIdMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change sourceIdMask to be !=0 - expect GT_OK
        *  SourceId attribute in TTI action is located in bits [97-108]
        *  bits [103:97] are placed in one byte
        *  bits [108:103] are placed in another byte
        *
        *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
        *  if any bit of sourceIdMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
        *  if any bit of sourceIdMask[11:7] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
        *
        *  For AC5X, AC5P, Harrier:
        *  SourceId attribute in TTI action is located in bits [67:78]
        *  bits [67:71] are placed in one byte
        *  bits [72:78] are placed in another byte
        *
        *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
        *  if any bit of sourceIdMask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
        *  if any bit of sourceIdMask[11:5] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
        */
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            expandedActionOriginData.ttiExpandedActionOrigin.sourceIdMask=0x10;/* take first byte - should pass 8 bytes in reduced */
        }
        else
        {
            expandedActionOriginData.ttiExpandedActionOrigin.sourceIdMask=0x40;/* take first byte - should pass 8 bytes in reduced */
        }

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            expandedActionOriginData.ttiExpandedActionOrigin.sourceIdMask=0x20;/* take second byte - should pass 8 bytes in reduced */
        }
        else
        {
            expandedActionOriginData.ttiExpandedActionOrigin.sourceIdMask=0x80;/* take second byte - should pass 8 bytes in reduced */
        }

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field tag0VlanIdMask
                   evlan [129:117] */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.tag0VlanCmd=CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        actionData.ttiAction.tag0VlanId = 0xFFF;
        actionData.ttiAction.tag0VlanCmd=GT_TRUE;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.tag0VlanIdMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change tag0VlanIdMask to be !=0 - expect GT_OK
            *  tag0VlanId attribute in TTI action is located in bits [117-129]
            *  bits [119:117] are placed in one byte
            *  bits [120-127] are placed in another byte
            *  bits [129-128] are placed in another byte
            *
            *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
            *  if any bit of tag0VlanIdMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of tag0VlanIdMask[10:3] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *  if any bit of tag0VlanIdMask[12:11] is set, than 2 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        expandedActionOriginData.ttiExpandedActionOrigin.tag0VlanIdMask=0x1;/* take first byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.tag0VlanIdMask=0x8;/* take second byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.tag0VlanIdMask=0x800;/* take third byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field tag1VlanIdMask
        vid1 [142:131] */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.tag1VlanCmd=CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        actionData.ttiAction.tag1VlanId = 0xFFF;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;

        expandedActionOriginData.ttiExpandedActionOrigin.tag1VlanIdMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change tag1VlanIdMask to be !=0 - expect GT_OK
            *  tag1VlanId attribute in TTI action is located in bits [142-131]
            *  bits [135:131] are placed in one byte
            *  bits [142:136] are placed in another byte
            *
            *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of tag1VlanIdMask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of tag1VlanIdMask[11:5] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        expandedActionOriginData.ttiExpandedActionOrigin.tag1VlanIdMask=0x1;/* take first byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check field sourceEPortMask
                source_eport [207:194] */
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable=GT_FALSE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        actionData.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
        actionData.ttiAction.tag1VlanCmd=CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        actionData.ttiAction.tag1VlanId = 0xFFF;
        actionData.ttiAction.sourceEPortAssignmentEnable=GT_TRUE;
        actionData.ttiAction.sourceEPort=0xFFF;

        expandedActionOriginData.ttiExpandedActionOrigin.sourceEPortMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        /* change sourceEPortMask to be !=0 - expect GT_OK
            * sourceEPort attribute in TTI action is located in bits [194-207]
            *  bits [199:194] are placed in one byte
            *  bits [207-200] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of sourceEPortMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of sourceEPortMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
        expandedActionOriginData.ttiExpandedActionOrigin.sourceEPortMask=0x1;/* take first byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        expandedActionOriginData.ttiExpandedActionOrigin.sourceEPortMask=0x40;/* take second byte - should pass 8 bytes in reduced */

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /* check IPCL and EPCL */
        /*
           1.2. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        /* for non falcon we need to set another field inorder to get more then 8 bytes in the reduced entry */
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        actionData.pclAction.vlan.ingress.vlanId=GT_TRUE;
        actionData.pclAction.sourceId.assignSourceId =GT_TRUE;
        actionData.pclAction.sourcePort.assignSourcePortEnable=GT_TRUE;
        actionData.pclAction.flowId = 0x1E3F;

        expandedActionOriginData.pclExpandedActionOrigin.flowIdMask=0;

        /* expect GT_BAD_PARAM - to many fields to overwrite */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
          /* 208-223: 16 bits in the flow id:
           bits 208-215 take 1 byte ( flowIdMask=100, setting any bits 0-7, will set byte 1 to be taken from the reduced)
           bits 216-223 take second byte */
            expandedActionOriginData.pclExpandedActionOrigin.flowIdMask = 100;
        }
        else
        {
            /* falcon have 13 bits in the flow id:
               bits 202-207 take 1 byte ( flowIdMask=63,setting any bits 0-6, will set byte 1 to be taken from the reduced)
               bits 208-214 take second byte */
            expandedActionOriginData.pclExpandedActionOrigin.flowIdMask = 63;
        }

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
          1.3. Call function with
                actionPtr { egressPolicy [GT_FALSE]
                            mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                            redirect {
                               redirectCmd [
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                               data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                          A5:A5:A5:A5:A5:A5 /
                                                          FF:FF:FF:FF:FF:FF],
                                                   arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                            oam { timeStampEnable [GT_FALSE / GT_TRUE],
                                  offsetIndex [0 / 0xA / BIT_4-1],
                                  oamProcessEnable [GT_FALSE / GT_TRUE],
                                  oamProfile [0 / 1] },
                            sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                     sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                            qos { ingress { up1Cmd[
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                           up1[0 / 5 / BIT_3-1]} }
                            vlan { ingress { vlanId1Cmd[
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            flowId [0 / 0xA5A / BIT_12-1],
                            setMacToMe [GT_FALSE / GT_TRUE] }
                and other params from 1.5.
        Expected: GT_OK and same action as written
        */

            actionData.pclAction.egressPolicy = GT_FALSE;
            actionData.pclAction.mirror.mirrorToRxAnalyzerPort = GT_TRUE;

            actionData.pclAction.mirror.ingressMirrorToAnalyzerIndex = 0;
            actionData.pclAction.redirect.redirectCmd =
                                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            actionData.pclAction.redirect.data.modifyMacSa.macSa = macSa1;
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            actionData.pclAction.oam.timeStampEnable = GT_FALSE;
            actionData.pclAction.oam.offsetIndex = 0;
            actionData.pclAction.oam.oamProcessEnable = GT_FALSE;
            actionData.pclAction.oam.oamProfile = 0;
            actionData.pclAction.sourcePort.assignSourcePortEnable = GT_FALSE;
            actionData.pclAction.sourcePort.sourcePortValue = 0;
            actionData.pclAction.qos.ingress.up1Cmd =
                           CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.qos.ingress.up1 = 0;
            actionData.pclAction.vlan.ingress.vlanId1Cmd =
                      CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.vlan.ingress.vlanId1 = 0;
            actionData.pclAction.flowId = 0;
            actionData.pclAction.setMacToMe = GT_FALSE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

            cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
            cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

            /* we need to set another field inorder to get more then 8 bytes in the reduced entry */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort=GT_TRUE;

            actionData.pclAction.vlan.ingress.vlanId=GT_TRUE;
            actionData.pclAction.sourceId.assignSourceId =GT_TRUE;
            actionData.pclAction.sourcePort.assignSourcePortEnable=GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.flowIdMask=0;

             /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
              /* 208-223: 16 bits in the flow id:
               bits 208-215 take 1 byte
               bits 216-223 take second byte (flowIdMask=128, setting any bits 7-15, will set byte 2 to be taken from the reduced)*/
               expandedActionOriginData.pclExpandedActionOrigin.flowIdMask = 128;
            }
            else
            {
                /* falcon have 13 bits in the flow id:
                   bits 202-207 take 1 byte
                   bits 208-214 take second byte (flowIdMask=64,setting any bits 6-13, will set byte 2 to be taken from the reduced) */
                expandedActionOriginData.pclExpandedActionOrigin.flowIdMask = 64;
            }
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check bits 16-43 macSA[27:0], bits 62-81 macSA[47:28] */
            expandedActionOriginData.pclExpandedActionOrigin.flowIdMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMacSa=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E;

            actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[0] = 0x11;
            actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[1] = 0x22;
            actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[2] = 0x33;
            actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[3] = 0x44;
            actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[4] = 0x55;
            actionData.pclAction.redirect.data.modifyMacSa.macSa.arEther[5] = 0x66;

            expandedActionOriginData.pclExpandedActionOrigin.macSa_27_0_Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.macSa_47_28_Mask=0;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change macSa_27_0_Mask to be !=0 - expect GT_OK - macSa_27_0_Mask 0x80 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.macSa_27_0_Mask=0x80;
            expandedActionOriginData.pclExpandedActionOrigin.macSa_47_28_Mask=0;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change macSa_27_0_Mask to be !=0 - expect GT_OK - macSa_27_0_Mask 0x8008000 will take only second and forth byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.macSa_27_0_Mask=0x8008000;
            expandedActionOriginData.pclExpandedActionOrigin.macSa_47_28_Mask=0;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change macSa_47_28_Mask to be !=0 - expect GT_OK - macSa_47_28_Mask 0x80 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.macSa_27_0_Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.macSa_47_28_Mask=0x80;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change macSa_47_28_Mask to be !=0 - expect GT_OK - macSa_47_28_Mask 0x88000 will take only second anf third byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.macSa_27_0_Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.macSa_47_28_Mask=0x88000;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* check field vrfIdMask */
            expandedActionOriginData.pclExpandedActionOrigin.macSa_27_0_Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.macSa_47_28_Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMacSa=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVrfId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;
            actionData.pclAction.redirect.data.vrfId=255;

            expandedActionOriginData.pclExpandedActionOrigin.vrfIdMask=0;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change vrfIdMask to be !=0 - expect GT_OK
            *  vrfId attribute in IPCL action is located in bits [52:63]
            *  bits [52:55] are placed in one byte
            *  bits [56:63] are placed in another byte
            *
            *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of vrfIdMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of vrfIdMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            expandedActionOriginData.pclExpandedActionOrigin.vrfIdMask = 0x8;/* take only first byte from reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.vrfIdMask = 0x10;/* take only second byte from reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);


            /* check field egressInterfaceMask
               evidx [32:17] -Redirect_Command="Egress_Interface" and UseVIDX= "1" */
            expandedActionOriginData.pclExpandedActionOrigin.vrfIdMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVrfId=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort=GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_VIDX_E;
            actionData.pclAction.redirect.data.outIf.outInterface.vidx=0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change egressInterfaceMask to be !=0 - expect GT_OK - egressInterfaceMask 127 will take only 1 byte in the reduced action
             *  bits 17-32 eVidx
             *      bit [16] useVidx + bits [23:17] are placed in one byte
             *      bits [31:24] are placed in another byte
             *      bits [32]    is placed in another byte
             *
             *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
             *  if any bit of egressInterfaceMask[6:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
             *  if any bit of egressInterfaceMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
             *  if any bit of egressInterfaceMask[15] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
             *  any combination of the bytes must include the first byte since the useVidx is located in the first byte and will always be set
             */
            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x80;/* take only one byte - the middle one - should fail */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x8000;/* take only one byte - the last one - should fail */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x8001;/* bit 0 and 15 on - want to take byte1 and byte3 - should pass*/

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x7F;/* take only one byte - the first one - should pass */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check field egressInterfaceMask
               trunk_id [29:18] Redirect_Command="Egress_Interface" and UseVIDX = "0" and IsTrunk="True" */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_TRUNK_E;
            actionData.pclAction.redirect.data.outIf.outInterface.trunkId=0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change egressInterfaceMask to be !=0 - expect GT_OK - egressInterfaceMask 127 will take only 1 byte in the reduced action
                *  egress Interface attribute in IPCL action is located in:
                *  bits 18-29 trunk ID
                *  bits [23:18] are placed in one byte
                *  bits [29:24] are placed in another byte
                *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
                *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
                *  if any bit of egressInterfaceMask[11:6]  - this option is not supported - we can not set only second
                *  byte since the isTrunk bit 17 is located in the first byte and will always be set when setting trunkId
                */
            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x40;/* take only one byte - the second one - should fail */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x3F;/* take byte1 - should pass - also set the isTrunk bit */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

             /* check field egressInterfaceMask
               trg_eport [31:18] - Redirect_Command="Egress_Interface" and UseVIDX = "0" and IsTrunk="False" */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable =GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort=GT_FALSE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
            actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=0x10;
            actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=0x20;

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

            actionData.pclAction.vlan.ingress.vlanId=0Xff;
            actionData.pclAction.sourceId.assignSourceId =GT_TRUE;
            actionData.pclAction.sourcePort.assignSourcePortEnable=GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change egressInterfaceMask to be !=0 - expect GT_OK - egressInterfaceMask 127 will take only 1 byte in the reduced action
                *  egress Interface attribute in TTI action is located in:
                *  bits 18-31 target ePort
                *  bits [23:18] are placed in one byte
                *  bits [31:24] are placed in another byte
                *  0 or a value>=BIT_14- use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
                *  if any bit of egressInterfaceMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
                *  if any bit of egressInterfaceMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
                *  we can not set only second byte since the isTrunk(bit 17) and useVidx(bit 16) are located in the first byte
                *  and will always be set when setting eport
               */
            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x40;/* take only one byte - the second one - should fail */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0x3F;/* take byte1 - should pass*/

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check field egressInterfaceMask
               ip_next_hop_entry_index [33:50] -  Redirect_Command="IPNextHop" */
            expandedActionOriginData.pclExpandedActionOrigin.egressInterfaceMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr=GT_TRUE;
            actionData.pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E;

            actionData.pclAction.redirect.data.routerLttIndex=0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.routerLttPtrMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change routerLttPtrMask to be !=0 - expect GT_OK - routerLttPtrMask 127 will take only 1 byte in the reduced action
                *  routerLttPtr attribute in IPCL action is located in bits [33:50]
                *  bits [33:39] are placed in one byte
                *  bits [40:47] are placed in another byte
                *  bits [48:50] are placed in another byte
                *
                *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
                *  if any bit of routerLttPtrMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
                *  if any bit of routerLttPtrMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
                *  if any bit of routerLttPtrMask[17:15] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
                */
            expandedActionOriginData.pclExpandedActionOrigin.routerLttPtrMask=0x4001;/* bit 0 and 14 on - want to take byte0 and byte1 - should pass - 8 bytes*/

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.routerLttPtrMask=0x7F;/* take only one byte - the first one - should pass 7 bytes */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.routerLttPtrMask=0x80;/* take only one byte - the middle one - should pass 7 bytes*/

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.routerLttPtrMask=0x20000;/* take only one byte - the last one - should pass 7 bytes */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check field tunnelIndexMask
                   tunnel_pointer [59:44] - Redirect_Command="Egress_Interface" and TunnelStart="True" */
            expandedActionOriginData.pclExpandedActionOrigin.routerLttPtrMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            actionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
            actionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=0x10;
            actionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=0x20;

            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change tunnelIndexMask to be !=0 - expect GT_OK - tunnelIndexMask 127 will take only 1 byte in the reduced action
                *  tunnelIndex attribute in TTI action is located in bits [44:59]
                *  bits [47:44] are placed in one byte
                *  bits [55:48] are placed in another byte
                *  bits [59:56] are placed in another byte
                *
                *  0 or a value>=BIT_16 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
                *  if any bit of tunnelIndexMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
                *  if any bit of tunnelIndexMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
                *  if any bit of tunnelIndexMask[15:12] is set, than 4 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
                */
            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0x18;/* take 2 bytes (0,1) - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0x1010;/* take 2 bytes(1,2) - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0x1001;/* take 2 bytes(0,2) - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0x8;/* take byte1 - should pass - 7 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0x10;/* take only one byte - the second one - should pass- 7 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

             expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0x1000;/* take only one byte - the third one - should pass */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check field arpPtrMask
                      arp_pointer [61:44] - Redirect_Command="Egress_Interface" and TunnelStart="False" */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            actionData.pclAction.redirect.data.outIf.tunnelStart = GT_FALSE;
            actionData.pclAction.redirect.data.outIf.arpPtr = 0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change arpPtr to be !=0 - expect GT_OK - arpPtr 127 will take only 1 byte in the reduced action
              *  arpPtr attribute in IPCL action is located in bits [44:61]
              *  bits [44:47] are placed in one byte
              *  bits [48:55] are placed in another byte
              *  bits [56:61] are placed in another byte
              *
              *  0 or a value>=BIT_18 - use all the bits from the reduced entry (will occupy 3 bytes in the reduced entry)
              *  if any bit of arpPtrMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
              *  if any bit of arpPtrMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
              *  if any bit of arpPtrMask[17:12] is set, than 6 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0x11;/* take 2 (0,1) bytes - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0x1001;/* take 2 bytes(0,2) - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0x1010;/* take 2 (2,3)bytes - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0x8;/* take byte1 - should pass - 7 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0x400;/* take only one byte - the second one - should pass- 7 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

             expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0x20000;/* take only one byte - the third one - should pass */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

           /* check field ipclConfigIndex
               ipcl_profile_index [64:52] - Redirect_Command !="Egress_Interface" */
            expandedActionOriginData.pclExpandedActionOrigin.arpPtrMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionArpPtr=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            actionData.pclAction.lookupConfig.ipclConfigIndex = 0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.ipclConfigurationIndexMask=0;/* take all 3 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change ipclConfigIndexMask to be !=0 - expect GT_OK
              *  ipclConfigurationIndex attribute in IPCL action is located in bits [18:30]
              *  bits [18:23] are placed in one byte
              *  bits [24:30] are placed in another byte
              *
              *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
              *  if any bit of ipclConfigurationIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
              *  if any bit of ipclConfigurationIndexMask[12:6] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
              */
            expandedActionOriginData.pclExpandedActionOrigin.ipclConfigurationIndexMask=0x20;/* take first byte should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.ipclConfigurationIndexMask=0x40;/* take second byte - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check policer_index [79:66] */
            expandedActionOriginData.pclExpandedActionOrigin.ipclConfigurationIndexMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex=GT_FALSE;
            actionData.pclAction.lookupConfig.ipclConfigIndex = 0;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0;

            actionData.pclAction.policer.policerId=0x1FFF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
            /*
            *  in IPCL action is located in bits [66-79]
            *  bits [66:71] are placed in one byte
            *  bits [72:79] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of policerIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of policerIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */

            /* change policerIndexMask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0x20;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change policerIndexMask to be !=0 - expect GT_OK - take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0x40;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

           /* check field sourceIdMask
               sst_id [103:92] */
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command=GT_TRUE;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            actionData.pclAction.sourceId.assignSourceId = GT_TRUE;
            actionData.pclAction.sourceId.sourceIdValue = 0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.sourceIdMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /*  SourceId attribute in IPCL action is located in bits [92:103]
            *  bits [92:95] are placed in one byte
            *  bits [96:103] are placed in another byte
            *
            *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of sourceIdMask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of sourceIdMask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            expandedActionOriginData.pclExpandedActionOrigin.sourceIdMask=0x8;/* take first byte - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.sourceIdMask=0x10;/* take second byte - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check cnc_counter_index [119:106]  */
            expandedActionOriginData.pclExpandedActionOrigin.sourceIdMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourceId=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            actionData.pclAction.matchCounter.enableMatchCount = GT_TRUE;
            actionData.pclAction.matchCounter.matchCounterIndex = 0x3F;
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
            /*
            *  IPCL action is located in bits [106-119]
            *  bits [106:111] are placed in one byte
            *  bits [112:119] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of matchCounterIndexMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of matchCounterIndexMask[13:6] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *
            *  For AC5X, AC5P, Harrier:
            *  IPCL action is located in bits [105-119]
            *  bits [105:111] are placed in one byte
            *  bits [112:119] are placed in another byte
            *
            *  0 or a value>=BIT_15 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of matchCounterIndexMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of matchCounterIndexMask[14:7] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change matchCounterIndexMask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0x20;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change matchCounterIndexMask to be !=0 - expect GT_OK - take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0x80;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check tag0_vid [135:123] */
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0 =GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0;

            actionData.pclAction.vlan.ingress.vlanId=0xFFF;
            actionData.pclAction.vlan.ingress.precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* in IPCL action is located in bits [123:135]
            *  bits [123-127] are placed in one byte
            *  bits [128-135] are placed in another byte
            *
            *  0 or a value>=BIT_13 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of vlan0Mask[4:0] is set, than 5 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of vlan0Mask[12:5] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change vlan0Mask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0x10;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change vlan0Mask to be !=0 - expect GT_OK - take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0x20;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

             /* check tag1_vid [156:167] */
            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command = GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command = GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1 =GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask = 0;

            actionData.pclAction.vlan.ingress.vlanId1=0xFFF;
            actionData.pclAction.vlan.ingress.precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /*  in IPCL action is located in bits [156:167]
            *  bits [156-159] are placed in one byte
            *  bits [160-167] are placed in another byte
            *
            *  0 or a value>=BIT_12 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of vlan1Mask[3:0] is set, than 4 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of vlan1Mask[11:4] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change vlan1Mask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask = 0x8;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change vlan1Mask to be !=0 - expect GT_OK - take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask=0x10;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

             /* check qos_profile [147:138] */
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile =GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.qosProfileMask = 0;

            actionData.pclAction.qos.ingress.profileIndex=0x1FF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* qosProfileMask attribute in IPCL action is located in bits [138:147]
             *
             *  bits [138:143] are placed in one byte
             *  bits [144:147] are placed in another byte
             *
             *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
             *  if any bit of qosProfileMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
             *  if any bit of qosProfileMask[8:6] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change vlan1Mask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.qosProfileMask = 0x20;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change vlan1Mask to be !=0 - expect GT_OK - take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.qosProfileMask=0x40;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check field sourceEPortMask
                source_eport [217:230] */
            expandedActionOriginData.pclExpandedActionOrigin.qosProfileMask = 0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile = GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.tunnelIndexMask=0;/* take 3 bytes*/

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd= GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable= GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort = GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.sourceEPortMask=0;

            actionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            actionData.pclAction.sourcePort.assignSourcePortEnable = GT_TRUE;
            actionData.pclAction.sourcePort.sourcePortValue = 0xFFF;

            expandedActionOriginData.pclExpandedActionOrigin.sourceEPortMask=0;/* take all 2 bytes - should not pass more then 8 bytes in reduced */

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* sourceEPort attribute in IPCL action is located in bits [217:230]
            *  bits [217:223] are placed in one byte
            *  bits [224:230] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of sourceEPortMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of sourceEPortMask[13:7] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *
            *  For AC5X, AC5P, Harrier:
            *  bits [225:231] are placed in one byte
            *  bits [232:238] are placed in another byte
            *
            *  0 or a value>=BIT_14 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of sourceEPortMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of sourceEPortMask[13:7] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            expandedActionOriginData.pclExpandedActionOrigin.sourceEPortMask=0x40;/* take first byte - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            expandedActionOriginData.pclExpandedActionOrigin.sourceEPortMask=0x80;/* take second byte - should pass 8 bytes in reduced */

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check latency_profile [231-239] */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSourcePort = GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.sourceEPortMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0;

            actionData.pclAction.latencyMonitor.latencyProfile=0xFF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
            /* IPCL:
            *  bits [231] are placed in one byte
            *  bits [232:239] are placed in  another byte
            *
            *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of latencyMonitorMask[0] is set, than 1 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of latencyMonitorMask[8:1] is set, than 8 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *
            *  For AC5X, AC5P, Harrier:
            *  IPCL:
            *  bits [240:247] are placed in one byte
            *  bits [248] are placed in  another byte
            *
            *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of latencyMonitorMask[7:0] is set, than 8 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of latencyMonitorMask[8] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change latencyMonitorMask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x80;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x1;
            }
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change latencyMonitorMask to be !=0 - expect GT_OK - only second byte in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x100;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x2;
            }

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /*1.7. Call function with
                actionPtr { egressPolicy [GT_TRUE]
                            qos { egress { modifyDscp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                          modifyUp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                          up1ModifyEnable [GT_FALSE / GT_TRUE],
                                          up1[0 / 5 / BIT_3-1]} }
                            vlan { egress { vlanCmd[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                           vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                            tmQueueId [0 / 0x2A5A / BIT_14-1] }
                and other params from 1.6.
            Expected: GT_OK and same action as written */

            actionData.pclAction.egressPolicy = GT_TRUE;
            actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            actionData.pclAction.actionStop = GT_FALSE;
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            actionData.pclAction.sourcePort.sourcePortValue = 0;

            actionData.pclAction.qos.egress.modifyDscp =
                                CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
            actionData.pclAction.qos.egress.modifyUp =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.qos.egress.up1ModifyEnable = GT_FALSE;
            actionData.pclAction.qos.egress.up1 = 0;
            actionData.pclAction.vlan.egress.vlanCmd =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.vlan.egress.vlanId1ModifyEnable = GT_FALSE;
            actionData.pclAction.vlan.egress.vlanId1 = 0;
            actionData.pclAction.channelTypeToOpcodeMapEnable = GT_FALSE;
            actionData.pclAction.tmQueueId = 0;

            cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
            cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

            /* we need to set another field inorder to get more then 8 bytes in the reduced entry */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo=GT_TRUE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

            actionData.pclAction.vlan.egress.vlanId=GT_TRUE;
            actionData.pclAction.vlan.egress.vlanCmd=CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
            actionData.pclAction.vlan.egress.vlanId1=GT_TRUE;
            actionData.pclAction.vlan.egress.vlanId1ModifyEnable=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.flowIdMask=0;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
              /* 79-94: 16 bits in the flow id:
                 bits [79] are placed in one byte
                 bits [80-87] are placed in  another byte
                 bits [88-94] are placed in  another byte

                change flowId to be !=0 - expect GT_OK - flowIdMask 2 (bit 1 is up )will take only second byte in the reduced action */
               expandedActionOriginData.pclExpandedActionOrigin.flowIdMask = 2;
            }
            else
            {
                /* falcon have 13 bits in the flow id:
                   bits 78-79 take first byte
                   bits 80-87 take second byte (flowId=64 will set byte 1 to be take from the reduced)
                   bits 88-90 take third byte*/

                /* change flowId to be !=0 - expect GT_OK - flowIdMask 4 (bit 2 is up )will take only second byte in the reduced action */
                expandedActionOriginData.pclExpandedActionOrigin.flowIdMask = 4;
            }

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check dscp_exp [8:3] */
            expandedActionOriginData.pclExpandedActionOrigin.flowIdMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_FALSE;

            /* for sip6_10 the fields are set different in the bytes need to adjust the logic to get
               more than 8 bytes in the reduced entry  */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIpfixEnable=GT_TRUE;
            }

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionDscpExp=GT_TRUE;

            actionData.pclAction.qos.egress.dscp = 0x3F;
            expandedActionOriginData.pclExpandedActionOrigin.dscpExpMask=0;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change dscpExpMask to be !=0 - expect GT_OK - dscpExpMask 1 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.dscpExpMask=1;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change dscpExpMask to be !=0 - expect GT_OK - dscpExpMask 0x20 will take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.dscpExpMask=0x20;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check cnc_counter_index [30:17]  */
            expandedActionOriginData.pclExpandedActionOrigin.dscpExpMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionDscpExp=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIpfixEnable=GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            actionData.pclAction.matchCounter.enableMatchCount = GT_TRUE;
            actionData.pclAction.matchCounter.matchCounterIndex = 0x3F;
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change matchCounterIndexMask to be !=0 - expect GT_OK - matchCounterIndexMask 0x40 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0x40;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change matchCounterIndexMask to be !=0 - expect GT_OK - matchCounterIndexMask 0x80 will take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0x80;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check tag1_vid [44:33] */
            expandedActionOriginData.pclExpandedActionOrigin.matchCounterIndexMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1 =GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            /* for sip6 vlan1Command is located in bit 31
               for sip6_10 vlan1Command is located in bit 32
               inorder to get the amount of bytes we need we
               should disable overwriteExpandedActionVlan1Command for sip6_10 */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command=GT_FALSE;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command=GT_TRUE;
            }
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask = 0;

            actionData.pclAction.vlan.egress.vlanId1=0xFFF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change vlan1Mask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask = 0x20;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask = 0x40;
            }

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change vlan1Mask to be !=0 - expect GT_OK - vlan1Mask 0x100 will take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask=0x100;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check tag0_vid [61:50] */
            expandedActionOriginData.pclExpandedActionOrigin.vlan1Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1 =GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command=GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0 =GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0;

            actionData.pclAction.vlan.egress.vlanId=0xFFF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change vlan0Mask to be !=0 - expect GT_OK - vlan0Mask 0x8 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0x8;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change vlan0Mask to be !=0 - expect GT_OK - vlan0Mask 0x40 will take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0x40;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check policer_index [77:64] */
            expandedActionOriginData.pclExpandedActionOrigin.vlan0Mask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0 =GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0;

            actionData.pclAction.policer.policerId=0x1FFF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change policerIndexMask to be !=0 - expect GT_OK - policerIndexMask 0x8 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0x8;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change policerIndexMask to be !=0 - expect GT_OK - policerIndexMask 0x100 will take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0x100;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check timestamp_offset_index [100:94]  */
            expandedActionOriginData.pclExpandedActionOrigin.policerIndexMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = GT_FALSE;

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;


            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.oamOffsetIndexMask=0;

            actionData.pclAction.oam.offsetIndex=0x3F;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change oamOffsetIndexMask to be !=0 - expect GT_OK - oamOffsetIndexMask 2 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.oamOffsetIndexMask=2;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change oamOffsetIndexMask to be !=0 - expect GT_OK - oamOffsetIndexMask 0x40 will take only second byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.oamOffsetIndexMask=0x40;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* pha_metadata [155:124] was not checked */
            /* check pha_thread_number [164:157] */
            expandedActionOriginData.pclExpandedActionOrigin.oamOffsetIndexMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex = GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;

            /* need different bytes configuration to get 9 bytes in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_FALSE;
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIpfixEnable=GT_TRUE;
            }

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.phaThreadNumberMask=0;

            actionData.pclAction.epclPhaInfo.phaThreadId=63;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
            /*
            *  phaThreadNumber attribute in EPCL action is located in bits [157:164]
            *
            *  EPCL:
            *  bits [157:159] are placed in one byte
            *  bits [160:164] are placed in  another byte
            *
            *  0 or a value>=BIT_8 - use all the bits from the reduced entry (will occupy 5 bytes in the reduced entry)
            *  if any bit of phaThreadNumberMask[2:0] is set, than 3 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of phaThreadNumberMask[7:3] is set, than 5 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *
            *  for SIP_6_10
            *  bits [177:179] are placed in one byte
            *  bits [180:184] are placed in  another byte
            *
            *  0 or a value>=BIT_8 - use all the bits from the reduced entry (will occupy 5 bytes in the reduced entry)
            *  if any bit of phaThreadNumberMask[6:0] is set, than 7 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of phaThreadNumberMask[7] is set, than 1 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change phaThreadId to be !=0 - expect GT_OK - phaThreadId 4 will take only first byte in the reduced action */
            expandedActionOriginData.pclExpandedActionOrigin.phaThreadNumberMask=0x4;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* change phaThreadId to be !=0 - expect GT_OK - take only second byte in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.phaThreadNumberMask=0x80;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.phaThreadNumberMask=0x8;
            }
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* check latency_profile [186:178] */
            expandedActionOriginData.pclExpandedActionOrigin.phaThreadNumberMask=0;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber = GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_FALSE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0;

            /* revert configuration done in phaThreadNumberMask case */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_FALSE;
                expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionIpfixEnable=GT_FALSE;
            }

            actionData.pclAction.latencyMonitor.latencyProfile=0xFF;

            /* expect GT_BAD_PARAM - to many fields to overwrite */
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
            /*
            *  EPCL:
            *  bits [178:183] are placed in one byte
            *  bits [184:186] are placed in  another byte
            *
            *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of latencyMonitorMask[5:0] is set, than 6 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of latencyMonitorMask[8:6] is set, than 3 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            *
            *  for SIP6_10
            *  bits [198:199] are placed in one byte
            *  bits [200:206] are placed in  another byte
            *
            *  0 or a value>=BIT_9 - use all the bits from the reduced entry (will occupy 2 bytes in the reduced entry)
            *  if any bit of latencyMonitorMask[1:0] is set, than 2 lsb are taken from the reduced entry (will occupy 1 byte in the reduced  entry).
            *  if any bit of latencyMonitorMask[8:2] is set, than 7 msb are taken from the reduced entry (will occupy 1 byte in the reduced entry).
            */
            /* change latencyMonitorMask to be !=0 - expect GT_OK - take only first byte in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x1;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x20;
            }
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);


            /* change latencyMonitorMask to be !=0 - expect GT_OK - only second byte in the reduced action */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x4;
            }
            else
            {
                expandedActionOriginData.pclExpandedActionOrigin.latencyMonitorMask=0x40;
            }

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);
     }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchExpandedActionGet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchExpandedActionGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null actionPtr, expandedActionOriginPtr
    Expected: GT_BAD_VALUE - the entry was not defined yet.
    1.2. Call with non null actionPtr, expandedActionOriginPtr and expandedActionIndex=16
    Expected: GT_BAD_PARAM.
    1.3. Call with actionPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with expandedActionOriginPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType;
    GT_U32                                           expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT expandedActionOriginDataGet;

    expandedActionIndex = 2;
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
    cpssOsBzero((GT_VOID*) &expandedActionOriginDataGet, sizeof(expandedActionOriginDataGet));

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        /*
             1.1. Call with non null actionPtr, expandedActionOriginPtr
            Expected: GT_BAD_VALUE - the entry was not defined yet.
        */
        expandedActionIndex=3;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, devNum);

         /*
             1.2. Call with non null actionPtr, expandedActionOriginPtr and expandedActionIndex=16
            Expected: GT_BAD_PARAM.
        */
        expandedActionIndex=16;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with actionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        expandedActionIndex=15;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,NULL,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

        /*
             1.4. Call with expandedActionOriginPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        expandedActionIndex=15;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    expandedActionIndex = 1;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
* @internal prvCompareExactMatchEntry function
* @endinternal
*
* @brief   This routine set compares 2 Exact Match Entry
*          structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
* @param[in] actionType               - type of the action to use
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
static GT_STATUS prvCompareExactMatchEntry
(
    IN GT_CHAR                            *name,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC    *firstEntryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC    *secondEntryPtr
)
{
    GT_STATUS   st = GT_OK;
    GT_BOOL     isEqual;
    GT_U32      numOfBytes;

    if(firstEntryPtr->lookupNum != secondEntryPtr->lookupNum)
    {
        PRV_UTF_LOG3_MAC("get another %s lookupNum than was set, expected = %d, received = %d\n",
                         name, firstEntryPtr->lookupNum, secondEntryPtr->lookupNum);
        st = GT_FAIL;
    }
    if(firstEntryPtr->key.keySize != secondEntryPtr->key.keySize)
    {
        PRV_UTF_LOG3_MAC("get another %s key.keySize than was set, expected = %d, received = %d\n",
                         name, firstEntryPtr->key.keySize, secondEntryPtr->key.keySize);
        st = GT_FAIL;
    }

    switch (firstEntryPtr->key.keySize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        numOfBytes=5;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        numOfBytes=19;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        numOfBytes=33;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        numOfBytes=47;
        break;
    default:
        PRV_UTF_LOG1_MAC("illegal keySize= %d\n",firstEntryPtr->key.keySize);
        return GT_FAIL;
    }
    isEqual = (0 == cpssOsMemCmp(
          (GT_VOID*) &firstEntryPtr->key.pattern,
          (GT_VOID*) &secondEntryPtr->key.pattern,
          numOfBytes*sizeof(GT_U8))) ? GT_TRUE : GT_FALSE;
    if(isEqual != GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("get another key.pattern than was set\n");

        st = GT_FAIL;
    }
    return st;
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChExactMatchPortGroupEntrySet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    IN GT_U32                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupEntrySet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call cpssDxChExactMatchPortGroupEntrySet index=0,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        expandedActionIndexand other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.3. Call with wrong enum values actionType, other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong actionType that do not fit the action type TTI in the expanded,
        other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong value expandedActionIndex - not valid, other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong value index - that do not fit the keyType, other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong value lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.8. Call with entryPtr [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.
    1.9. Call with actionPtr [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.

    1.10. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.11. Call with wrong actionType that do not fit the action type PCL in the expanded,
        other params same as in 1.2.
        Expected: GT_BAD_PARAM.

    1.12. invalidate entries defined
        Expected: GT_TRUE.*/

    GT_STATUS                                           st = GT_OK;
    GT_U8                                               i=0;
    GT_U8                                               devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionTypeGet;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;


    GT_U32                                   index=0;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC          exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC          exactMatchEntryGet;
    GT_BOOL                                  valid;
    GT_BOOL                                  exactMatchActivityStatus;

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */

        expandedActionIndex = 2;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
        cpssOsBzero((GT_VOID*) &exactMatchEntry, sizeof(exactMatchEntry));
        cpssOsBzero((GT_VOID*) &exactMatchEntryGet, sizeof(exactMatchEntryGet));
        cpssOsBzero((GT_VOID*) &exactMatchEntry.key.pattern, sizeof(exactMatchEntry.key.pattern));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
         1.2. Call cpssDxChExactMatchPortGroupEntrySet index=0,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

         index=0;
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
         for (i=0;i<5;i++)/* 5 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
         {
             exactMatchEntry.key.pattern[i] = i;
         }
         exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                              expandedActionIndex,&exactMatchEntry,actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
         /* validation values */
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
         UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

         st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

         st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

         /* check activity status */
         st = cpssDxChExactMatchActivityStatusGet(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  index,
                                                  GT_FALSE,
                                                  &exactMatchActivityStatus);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, exactMatchActivityStatus,"error - entry is not active\n");


        /*1.3. Call with wrong enum values actionType, other params same as in 1.2.
        Expected: GT_BAD_PARAM.*/
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

         /*1.4. Call with wrong actionType that do not fit the action type TTI in the expanded,
            other params same as in 1.2.
            Expected: GT_BAD_PARAM.*/
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        /* 1.5. Call with wrong value expandedActionIndex - not valid, other params same as in 1.2.
            Expected: GT_BAD_PARAM.  */
         expandedActionIndex=4;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,actionType);

         /* 1.6. Call with wrong value index - that do not fit the keyType, other params same as in 1.2.
            Expected: GT_BAD_PARAM.*/
         expandedActionIndex=2;
         index=1;/* index must be %2==0 for SIZE_19B_E*/
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,actionType);

         /*1.7. Call with wrong value lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E other params same as in 1.2.
           Expected: GT_BAD_PARAM.*/
         expandedActionIndex=2;
         index=2;
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
         exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,actionType);

         /*  1.8. Call with entryPtr [NULL], other params same as in 1.2.
            Expected: GT_BAD_PTR.   */
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,NULL,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, exactMatchEntry = NULL", devNum);


    /* 1.9. Call with actionPtr [NULL], other params same as in 1.2.
        Expected: GT_BAD_PTR.*/
         expandedActionIndex=2;
         index=2;
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
         exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,NULL);
         UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, actionData = NULL", devNum);

         /*
           1.10. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
         1.11. Call cpssDxChExactMatchPortGroupEntrySet index=0,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        index=28;
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (i=0;i<47;i++)/* 47 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
        {
         exactMatchEntry.key.pattern[i] = i;
        }
        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");


        /*1.12. Call with wrong actionType that do not fit the action type PCL in the expanded,
            other params same as in 1.2.
            Expected: GT_BAD_PARAM.*/
        index=60;
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);


        /*  1.12. invalidate entries defined
            Expected: GT_TRUE.*/
        index = 0;
        st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, index);
        index = 28;
        st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, index);

         /*  1.12. invalidate entries that is not defined/already non valid
            Expected: GT_OK.*/
        index = 70;
        st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, index);
    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    index=6;
    expandedActionIndex=5;
    exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
    actionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                             expandedActionIndex,&exactMatchEntry,
                                             actionType,&actionData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupEntryGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupEntryGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null validPtr, actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr
    Expected: GT_OK - validPtr = GT_FALSE the entry was not defined yet.
    1.2. Call with validPtr [NULL].
    Expected: GT_BAD_PARAM.
    1.3. Call with actionTypePtr [NULL].
    Expected: GT_BAD_PARAM.
    1.4. Call with actionPtr [NULL].
    Expected: GT_BAD_PARAM.
    1.5. Call with entryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with expandedActionIndexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum = 0;
    GT_U32                                          index;
    GT_BOOL                                         valid;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           actionTypeGet;
    GT_U32                                          expandedActionIndexGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionDataGet;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC                 exactMatchEntryGet;


    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntryGet, sizeof(exactMatchEntryGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntryGet.key.pattern, sizeof(exactMatchEntryGet.key.pattern));

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1. Call with non null validPtr, actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr
           Expected: GT_OK - validPtr = GT_FALSE the entry was not defined yet.
        */
        index=4;
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, valid,"error - entry is valid, expected non valid\n");

        /*1.2. Call with validPtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,NULL,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.3. Call with actionTypePtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  NULL,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.4. Call with actionPtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,NULL,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.5. Call with entryPtr [NULL].
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,NULL,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.6. Call with expandedActionIndexPtr [NULL].
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;
    index=2;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

     st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupEntryStatusGet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN GT_U32                               index,
    OUT GT_BOOL                             *validPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupEntryStatusGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null validPtr
    Expected: GT_OK - validPtr = GT_FALSE the entry was not defined yet.
    1.2. Call with validPtr [NULL].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum = 0;
    GT_U32                                          index;
    GT_BOOL                                         valid;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1. Call with non null validPtr
           Expected: GT_OK - validPtr = GT_FALSE the entry was not defined yet.
        */
        index=4;
        st = cpssDxChExactMatchPortGroupEntryStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, valid,"error - entry is valid, expected non valid\n");

        /*1.2. Call with validPtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryStatusGet = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;
    index=2;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
         st = cpssDxChExactMatchPortGroupEntryStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

     st = cpssDxChExactMatchPortGroupEntryStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                           exactMatchCrcMultiHashArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchHashCrcMultiResultsByParamsCalc)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with entryKey.keySize[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E]
                            and entryKey.pattern[0]={0};
    Expected: GT_OK.
    1.1.2. Call function with entryKey.keySize[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E]
                            and entryKey.pattern[0]={0};
    Expected: GT_OK.

    1.2 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with fixed pattern .
    Expected: GT_OK and exactMatchCrcMultiHashArr have same indexes as expectedBankIndex .
    NOTE : The number of identical indexes depends on numberOfElemInCrcMultiHash value.

    1.3. Call with NULL entryKey and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL numberOfElemInCrcMultiHashArrPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call Call with NULL exactMatchCrcMultiHashArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with wrong keySize vlaue.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    CPSS_DXCH_EXACT_MATCH_KEY_STC       entryKey;
    GT_U32                              numberOfElemInCrcMultiHash;
    GT_U32                              exactMatchCrcMultiHashArr[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS]={0};
    GT_U32                              ii;
    GT_U32                              emTableSize;

    GT_U8 testInput[]=      {0xb0,0x3d,0xe5,0x98,0xcc,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,0,0,0} ;
    /*expectedBankIndex for CPSS_EXACT_MATCH_TABLE_SIZE_32KB and CPSS_EXACT_MATCH_MHT_4_E */
    GT_U32 CPSS_EXACT_MATCH_TABLE_SIZE_32KB__CPSS_EXACT_MATCH_MHT_4_E__expectedBankIndex[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS] =
        {0x0000295c,0x00006851,0x000027e6,0x00007ac7,
        /* test is for MHT4 .. but the device may run in MHT16 */
        GT_NA,GT_NA,GT_NA,GT_NA,
        GT_NA,GT_NA,GT_NA,GT_NA,
        GT_NA,GT_NA,GT_NA,GT_NA};

    /*expectedBankIndex for CPSS_EXACT_MATCH_TABLE_SIZE_64KB and CPSS_EXACT_MATCH_MHT_8_E */
    GT_U32 CPSS_EXACT_MATCH_TABLE_SIZE_64KB__CPSS_EXACT_MATCH_MHT_8_E__expectedBankIndex[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS] =
        {0x000052b8,0x0000d0a1,0x00004fca,0x0000f58b,
         0x000016e4,0x000020d5,0x00003836,0x0000791f,
        GT_NA,GT_NA,GT_NA,GT_NA,
        GT_NA,GT_NA,GT_NA,GT_NA};

    /*expectedBankIndex for CPSS_EXACT_MATCH_TABLE_SIZE_128KB and CPSS_EXACT_MATCH_MHT_16_E */
    GT_U32 CPSS_EXACT_MATCH_TABLE_SIZE_128KB__CPSS_EXACT_MATCH_MHT_16_E__expectedBankIndex[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS] =
        {0x0000a570,0x0001a141,0x00009f92,0x0001eb13,
         0x00002dc4,0x000041a5,0x00007066,0x0000f237,
         0x0001a9b8,0x0001c8a9,0x0001a5fa,0x00000cdb,
         0x0000afdc,0x0000118d,0x0000c9ce,0x0001d5df};

    /* Phoenix results on WM */
    GT_U32 CPSS_EXACT_MATCH_TABLE_SIZE_32KB__CPSS_EXACT_MATCH_MHT_16_E__expectedBankIndex[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS] =
        {0x00002570,0x00002141,0x00001f92,0x00006b13,
         0x00002dc4,0x000041a5,0x00007066,0x00007237,
         0x000029b8,0x000048a9,0x000025fa,0x00000cdb,
         0x00002fdc,0x0000118d,0x000049ce,0x000055df};

    /* Phoenix results on WM */
    GT_U32 CPSS_EXACT_MATCH_TABLE_SIZE_16KB__CPSS_EXACT_MATCH_MHT_16_E__expectedBankIndex[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS] =
        {0x00002570,0x00002141,0x00001f92,0x00002b13,
         0x00002dc4,0x000001a5,0x00003066,0x00003237,
         0x000029b8,0x000008a9,0x000025fa,0x00000cdb,
         0x00002fdc,0x0000118d,0x000009ce,0x000015df};

    GT_U32  *expectedBankIndex;
    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E,&emTableSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.1.1 Call function with entryKey.keySize[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E]
                                    and entryKey.pattern[0]={0};
           Expected: GT_OK.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        cpssOsMemSet(entryKey.pattern, 0, sizeof(entryKey.pattern));

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        if(numberOfElemInCrcMultiHash == 4 && emTableSize == _32K)
        {
            expectedBankIndex = &CPSS_EXACT_MATCH_TABLE_SIZE_32KB__CPSS_EXACT_MATCH_MHT_4_E__expectedBankIndex[0];
        }
        else
        if(numberOfElemInCrcMultiHash == 8 && emTableSize == _64K)
        {
            expectedBankIndex = &CPSS_EXACT_MATCH_TABLE_SIZE_64KB__CPSS_EXACT_MATCH_MHT_8_E__expectedBankIndex[0];
        }
        else
        if(numberOfElemInCrcMultiHash == 16 && emTableSize == _128K)
        {
            expectedBankIndex = &CPSS_EXACT_MATCH_TABLE_SIZE_128KB__CPSS_EXACT_MATCH_MHT_16_E__expectedBankIndex[0];
        }
        else
        if(numberOfElemInCrcMultiHash == 16 && emTableSize == _32K)
        {
            expectedBankIndex = &CPSS_EXACT_MATCH_TABLE_SIZE_32KB__CPSS_EXACT_MATCH_MHT_16_E__expectedBankIndex[0];
        }
        else
        if(numberOfElemInCrcMultiHash == 16 && emTableSize == _16K)
        {
            expectedBankIndex = &CPSS_EXACT_MATCH_TABLE_SIZE_16KB__CPSS_EXACT_MATCH_MHT_16_E__expectedBankIndex[0];
        }
        else
        {
            /* table {size , MHT} not implemented */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_NOT_IMPLEMENTED, devNum);
            continue;
        }

        /*
           1.1.2 Call function with keyType [CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E]
                                    and entryKey.pattern[0]={0};
           Expected: GT_OK.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        /*
            1.2 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with fixed pattern .
            Expected: GT_OK and exactMatchCrcMultiHashArr have same indexes as expectedBankIndex .
            NOTE :The number of identical indexes depends on numberOfElemInCrcMultiHash value.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (ii=0;ii<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;ii++)
        {
            entryKey.pattern[ii]=testInput[ii];
        }
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey,&numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        for (ii=0;ii<numberOfElemInCrcMultiHash;ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(exactMatchCrcMultiHashArr[ii], expectedBankIndex[ii],
                                         "got other index from exactMatchCrcMultiHashArr then expected: %d , index number %d ", devNum, ii);
        }

        /*
            1.3. Call with NULL entryKey and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,NULL,&numberOfElemInCrcMultiHash,&exactMatchCrcMultiHashArr[0]);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, " entryKey = NULL");

        /*
            1.4. Call with NULL numberOfElemInCrcMultiHashArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,&entryKey,NULL ,&exactMatchCrcMultiHashArr[0]);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, " numberOfElemInCrcMultiHashArr = NULL");

        /*
            1.5. Call Call with NULL exactMatchCrcMultiHashArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,&entryKey,&numberOfElemInCrcMultiHash ,NULL);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, " exactMatchCrcMultiHashArr = NULL");

        /*
            1.6 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with wrong keySize vlaue.
             Expected: GT_BAD_PARAM.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E;

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,&entryKey,&numberOfElemInCrcMultiHash,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        cpssOsMemSet(entryKey.pattern, 0, sizeof(entryKey.pattern));

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             startIndex,
    IN GT_U32                             numOfRules
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchRulesDump)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.3. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.4. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.5. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written

    1.6. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.7. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written
    1.8. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written
    1.9. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written
    1.10. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written

    1.11. Call cpssDxChExactMatchRulesDump startIndex=0,numOfRules=32K,
    Expected: GT_OK

    1.12. Call cpssDxChExactMatchRulesDump startIndex=32K,numOfRules=5,
    Expected: GT_BAD_PARAM

    1.13. invalidate entries defined
        Expected: GT_TRUE.
    */

    GT_STATUS                                           st = GT_OK;
    GT_U8                                               i=0;
    GT_U8                                               devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionTypeGet;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;


    GT_U32                                  index=0;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC         exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC         exactMatchEntryGet;
    GT_BOOL                                 valid;
    GT_BOOL                                 exactMatchActivityStatus;

    GT_U32                                  numberOfElemInCrcMultiHash;
    GT_U32                                  exactMatchCrcMultiHashArr[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS]={0};
    GT_U32                                  indexToInvalidate[8];

    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParamsGet;
    GT_U32                              emTableSize;

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */
        expandedActionIndex = 2;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
        cpssOsBzero((GT_VOID*) &exactMatchEntry, sizeof(exactMatchEntry));
        cpssOsBzero((GT_VOID*) &exactMatchEntryGet, sizeof(exactMatchEntryGet));
        cpssOsBzero((GT_VOID*) &exactMatchEntry.key.pattern, sizeof(exactMatchEntry.key.pattern));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

       /*
         1.2. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

        keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        keyParams.keyStart=1;
        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
           keyParams.mask[i]=i+1;
        }

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, expandedActionIndex, &keyParams);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, expandedActionIndex, keyParams.keySize, keyParams.keyStart);


        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, expandedActionIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchProfileKeyParamsGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keySize, keyParamsGet.keySize,
                   "get another keyParams.keySize than was set: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keyStart, keyParamsGet.keyStart,
                   "get another keyParams.keyStart than was set: %d", devNum);

        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.mask[i], keyParamsGet.mask[i],
                   "get another keyParams.mask than was set: %d", devNum);
        }

        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (i=0;i<5;i++)/* 5 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
        {
            exactMatchEntry.key.pattern[i] = i;
        }
        exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[0]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                          expandedActionIndex,&exactMatchEntry,actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                              &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /* check activity status */
        st = cpssDxChExactMatchActivityStatusGet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              index,
                                              GT_FALSE,
                                              &exactMatchActivityStatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, exactMatchActivityStatus,"error - entry is not active\n");


        /* 1.3. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
        expandedActionIndex and other params same as in 1.1.
        Expected: GT_OK and same action as written */

        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        for (i=0;i<19;i++)/* 19 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E*/
        {
         exactMatchEntry.key.pattern[i] = i;
        }
        exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[1]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                          expandedActionIndex,&exactMatchEntry,actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                              &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /* check activity status */
        st = cpssDxChExactMatchActivityStatusGet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              index,
                                              GT_FALSE,
                                              &exactMatchActivityStatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, exactMatchActivityStatus,"error - entry is not active\n");

       /* 1.4. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
         for (i=0;i<33;i++)/* 33 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E*/
         {
             exactMatchEntry.key.pattern[i] = i;
         }
         exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

         st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         index=exactMatchCrcMultiHashArr[0];
         indexToInvalidate[2]=index;

         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                              expandedActionIndex,&exactMatchEntry,actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
         /* validation values */
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
         UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

         st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

         st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

         /* check activity status */
         st = cpssDxChExactMatchActivityStatusGet(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  index,
                                                  GT_FALSE,
                                                  &exactMatchActivityStatus);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, exactMatchActivityStatus,"error - entry is not active\n");

         /* 1.5. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
         for (i=0;i<47;i++)/* 47 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E*/
         {
             exactMatchEntry.key.pattern[i] = i;
         }
         exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

         st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         index=exactMatchCrcMultiHashArr[0];
         indexToInvalidate[3]=index;

         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                              expandedActionIndex,&exactMatchEntry,actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
         /* validation values */
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
         UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

         st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

         st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

         /* check activity status */
         st = cpssDxChExactMatchActivityStatusGet(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  index,
                                                  GT_FALSE,
                                                  &exactMatchActivityStatus);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, exactMatchActivityStatus,"error - entry is not active\n");

         /*
           1.6. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
         1.7. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

        keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        keyParams.keyStart=2;
        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
           keyParams.mask[i]=i+1;
        }

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, expandedActionIndex, &keyParams);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, expandedActionIndex, keyParams.keySize, keyParams.keyStart);


        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, expandedActionIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchProfileKeyParamsGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keySize, keyParamsGet.keySize,
                   "get another keyParams.keySize than was set: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keyStart, keyParamsGet.keyStart,
                   "get another keyParams.keyStart than was set: %d", devNum);

        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.mask[i], keyParamsGet.mask[i],
                   "get another keyParams.mask than was set: %d", devNum);
        }

        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<5;i++)/* 5 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[4]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /*
         1.8. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<19;i++)/* 19 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[5]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /*
         1.9. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<33;i++)/* 33 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[6]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /*
         1.10. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<47;i++)/* 47 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[7]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        st = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E,&emTableSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*1.11. Call cpssDxChExactMatchRulesDump startIndex=0,numOfRules=32K,
        Expected: GT_OK*/
        st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(emTableSize-1));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChExactMatchRulesDump: %d\n", devNum);

        /*1.12. Call cpssDxChExactMatchRulesDump startIndex=32K,numOfRules=5,
        Expected: GT_OUT_OF_RANGE */
        st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,emTableSize,5);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st, "cpssDxChExactMatchRulesDump: %d\n", devNum);

        /*  1.13. invalidate 8 entries that were defined
            Expected: GT_OK.*/
        for (i=0;i<8;i++)
        {
            st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, indexToInvalidate[i]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, indexToInvalidate[i]);
        }
    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,5);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNum id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiRuleConvertToUdbFormat
(
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT        ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT             *patternFixedKeyFormatPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT             *maskFixedKeyFormatPtr,
    OUT GT_U8                              *patternUdbFormatArray,
    OUT GT_U8                              *maskUdbFormatArray
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiRuleConvertToUdbFormat)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1.1.  Call function with ruleType [CPSS_DXCH_TTI_RULE_IPV4_E,
                                         CPSS_DXCH_TTI_RULE_MPLS_E
                                         CPSS_DXCH_TTI_RULE_ETH_E
                                         CPSS_DXCH_TTI_RULE_MIM_E]
                               legal patternFixedKeyFormatPtr
                               legal maskFixedKeyFormatPtr

    Expected: GT_OK.
    1.1.2. Call cpssDxChExactMatchTtiRuleConvertFromUdbFormat.
    Expected: GT_OK and the same patternFixedKeyFormatPtr/maskFixedKeyFormatPtr

    1.1.3.  Call function with wrong ruleType [CPSS_DXCH_TTI_RULE_UDB_10_E,
                                               CPSS_DXCH_TTI_RULE_UDB_20_E
                                               CPSS_DXCH_TTI_RULE_UDB_30_E]

    Expected: GT_BAD_PARAM
    1.1.4. Call with NULL patternFixedKeyFormatPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.5. Call with NULL maskFixedKeyFormatPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.6. Call with NULL patternUdbFormatArray and other valid params.
    Expected: GT_BAD_PTR.
    1.1.7. Call with NULL maskUdbFormatArray and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_BOOL                             isEqual;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType;
    CPSS_DXCH_TTI_RULE_UNT              patternFixedKeyFormat;
    CPSS_DXCH_TTI_RULE_UNT              maskFixedKeyFormat;
    GT_U8                               patternUdbFormatArray[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];
    GT_U8                               maskUdbFormatArray[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    CPSS_DXCH_TTI_RULE_UNT              patternFixedKeyFormatGet;
    CPSS_DXCH_TTI_RULE_UNT              maskFixedKeyFormatGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    cpssOsBzero((GT_VOID*) &patternFixedKeyFormat, sizeof(patternFixedKeyFormat));
    cpssOsBzero((GT_VOID*) &maskFixedKeyFormat, sizeof(maskFixedKeyFormat));
    cpssOsBzero((GT_VOID*) &patternFixedKeyFormatGet, sizeof(patternFixedKeyFormatGet));
    cpssOsBzero((GT_VOID*) &maskFixedKeyFormatGet, sizeof(maskFixedKeyFormatGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        ruleType=CPSS_DXCH_TTI_RULE_IPV4_E;
        /* set default pattern */
        patternFixedKeyFormat.ipv4.common.pclId               = 0;
        patternFixedKeyFormat.ipv4.common.srcIsTrunk          = GT_FALSE;
        patternFixedKeyFormat.ipv4.common.srcPortTrunk        = 0;
        patternFixedKeyFormat.ipv4.common.mac.arEther[0]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[1]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[2]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[3]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[4]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[5]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.vid                 = 0;
        patternFixedKeyFormat.ipv4.common.isTagged            = GT_FALSE;
        patternFixedKeyFormat.ipv4.common.dsaSrcIsTrunk       = GT_FALSE;
        patternFixedKeyFormat.ipv4.common.dsaSrcPortTrunk     = 0;
        patternFixedKeyFormat.ipv4.common.dsaSrcDevice        = 0;
        patternFixedKeyFormat.ipv4.common.sourcePortGroupId   = 0;

        patternFixedKeyFormat.ipv4.tunneltype     = 0;

        patternFixedKeyFormat.ipv4.srcIp.arIP[0]  = 0xAA;
        patternFixedKeyFormat.ipv4.srcIp.arIP[1]  = 0xAA;
        patternFixedKeyFormat.ipv4.srcIp.arIP[2]  = 0xAA;
        patternFixedKeyFormat.ipv4.srcIp.arIP[3]  = 0xAA;

        patternFixedKeyFormat.ipv4.destIp.arIP[0] = 0xAA;
        patternFixedKeyFormat.ipv4.destIp.arIP[1] = 0xAA;
        patternFixedKeyFormat.ipv4.destIp.arIP[2] = 0xAA;
        patternFixedKeyFormat.ipv4.destIp.arIP[3] = 0xAA;

        patternFixedKeyFormat.ipv4.isArp          = GT_FALSE;

        /* set default mask */
        cpssOsMemSet(&maskFixedKeyFormat.ipv4.common, 0xFF, sizeof(maskFixedKeyFormat.ipv4.common));

        maskFixedKeyFormat.ipv4.common.pclId              &= BIT_10 - 1;/*10 bits*/
        maskFixedKeyFormat.ipv4.common.srcIsTrunk         &= 0x01;
        maskFixedKeyFormat.ipv4.common.srcPortTrunk       &= BIT_8 - 1;
        maskFixedKeyFormat.ipv4.common.vid                &= BIT_13 - 1;/*13 bits*/
        maskFixedKeyFormat.ipv4.common.isTagged           &= 0x01;
        maskFixedKeyFormat.ipv4.common.dsaSrcIsTrunk      &= 0x01;
        maskFixedKeyFormat.ipv4.common.dsaSrcPortTrunk    &= BIT_13 - 1;
        maskFixedKeyFormat.ipv4.common.dsaSrcDevice       &= BIT_10 - 1;
        maskFixedKeyFormat.ipv4.common.sourcePortGroupId  &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum);

        maskFixedKeyFormat.ipv4.common.sourcePortGroupId  = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum);

        cpssOsMemSet(&maskFixedKeyFormat.ipv4.common + 1, 0xFF,
                     sizeof(maskFixedKeyFormat) - sizeof(maskFixedKeyFormat.ipv4.common));
        maskFixedKeyFormat.ipv4.tunneltype        = 0x00000007;
        maskFixedKeyFormat.ipv4.isArp             = 0x01;

        /*1.1.1.  Call function with ruleType [CPSS_DXCH_TTI_RULE_IPV4_E,
                                             CPSS_DXCH_TTI_RULE_MPLS_E
                                             CPSS_DXCH_TTI_RULE_ETH_E
                                             CPSS_DXCH_TTI_RULE_MIM_E]
                                   legal patternFixedKeyFormatPtr
                                   legal maskFixedKeyFormatPtr

        Expected: GT_OK.*/
        st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         &patternFixedKeyFormat,
                                                         &maskFixedKeyFormat,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ruleType);

        /*1.1.2. Call cpssDxChExactMatchTtiRuleConvertFromUdbFormat.
        Expected: GT_OK and the same patternFixedKeyFormatPtr/maskFixedKeyFormatPtr*/
        st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray,
                                                         &patternFixedKeyFormatGet,
                                                         &maskFixedKeyFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ruleType);

        /* compare returned values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &patternFixedKeyFormat.ipv4,
                                     (GT_VOID*) &patternFixedKeyFormatGet.ipv4,
                                     sizeof(CPSS_DXCH_TTI_IPV4_RULE_STC))) ? GT_TRUE : GT_FALSE;
        if(isEqual != GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("get another patternFixedKeyFormat.ipv4 than was set\n");
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, GT_FAIL, devNum, ruleType);
        }

        /* 1.1.3.  Call function with wrong ruleType [CPSS_DXCH_TTI_RULE_UDB_10_E,
                                               CPSS_DXCH_TTI_RULE_UDB_20_E
                                               CPSS_DXCH_TTI_RULE_UDB_30_E]
           Expected: GT_BAD_PARAM*/
        ruleType = CPSS_DXCH_TTI_RULE_UDB_10_E;
        st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         &patternFixedKeyFormat,
                                                         &maskFixedKeyFormat,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ruleType);


        /*1.1.4. Call with NULL patternFixedKeyFormatPtr and other valid params.
        Expected: GT_BAD_PTR.*/
        ruleType = CPSS_DXCH_TTI_RULE_IPV4_E;
        st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         NULL,
                                                         &maskFixedKeyFormat,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
        /*1.1.5. Call with NULL maskFixedKeyFormatPtr and other valid params.
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         &patternFixedKeyFormat,
                                                         NULL,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
        /* 1.1.6. Call with NULL patternUdbFormatArray and other valid params.
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         &patternFixedKeyFormat,
                                                         &maskFixedKeyFormat,
                                                         NULL,
                                                         maskUdbFormatArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
        /* 1.1.7. Call with NULL maskUdbFormatArray and other valid params.
        Expected: GT_BAD_PTR.*/
       st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         &patternFixedKeyFormat,
                                                         &maskFixedKeyFormat,
                                                         patternUdbFormatArray,
                                                         NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
    }
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssDxChExactMatchTtiRuleConvertFromUdbFormat
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT        ruleType,
    IN  GT_U8                              *patternUdbFormatArray,
    IN  GT_U8                              *maskUdbFormatArray,
    OUT CPSS_DXCH_TTI_RULE_UNT             *patternFixedKeyFormatPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT             *maskFixedKeyFormatPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiRuleConvertFromUdbFormat)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1.1.  Call function with ruleType [CPSS_DXCH_TTI_RULE_IPV4_E,
                                         CPSS_DXCH_TTI_RULE_MPLS_E
                                         CPSS_DXCH_TTI_RULE_ETH_E
                                         CPSS_DXCH_TTI_RULE_MIM_E]
                               legal patternUdbFormatArray
                               legal maskUdbFormatArray

    Expected: GT_OK.

    1.1.2.  Call function with wrong ruleType [CPSS_DXCH_TTI_RULE_UDB_10_E,
                                               CPSS_DXCH_TTI_RULE_UDB_20_E
                                               CPSS_DXCH_TTI_RULE_UDB_30_E]

    Expected: GT_BAD_PARAM
    1.1.3. Call with NULL patternUdbFormatArray and other valid params.
    Expected: GT_BAD_PTR.
    1.1.4. Call with NULL maskUdbFormatArray and other valid params.
    Expected: GT_BAD_PTR.
    1.1.5. Call with NULL patternFixedKeyFormatPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.1.6. Call with NULL maskFixedKeyFormatPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_BOOL                             isEqual;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType;
    CPSS_DXCH_TTI_RULE_UNT              patternFixedKeyFormat;
    CPSS_DXCH_TTI_RULE_UNT              maskFixedKeyFormat;
    GT_U8                               patternUdbFormatArray[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];
    GT_U8                               maskUdbFormatArray[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    CPSS_DXCH_TTI_RULE_UNT              patternFixedKeyFormatGet;
    CPSS_DXCH_TTI_RULE_UNT              maskFixedKeyFormatGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    cpssOsBzero((GT_VOID*) &patternFixedKeyFormat, sizeof(patternFixedKeyFormat));
    cpssOsBzero((GT_VOID*) &maskFixedKeyFormat, sizeof(maskFixedKeyFormat));
    cpssOsBzero((GT_VOID*) &patternFixedKeyFormatGet, sizeof(patternFixedKeyFormatGet));
    cpssOsBzero((GT_VOID*) &maskFixedKeyFormatGet, sizeof(maskFixedKeyFormatGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        ruleType=CPSS_DXCH_TTI_RULE_IPV4_E;
        /* set default pattern */
        patternFixedKeyFormat.ipv4.common.pclId               = 0;
        patternFixedKeyFormat.ipv4.common.srcIsTrunk          = GT_FALSE;
        patternFixedKeyFormat.ipv4.common.srcPortTrunk        = 0;
        patternFixedKeyFormat.ipv4.common.mac.arEther[0]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[1]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[2]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[3]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[4]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.mac.arEther[5]      = 0xAA;
        patternFixedKeyFormat.ipv4.common.vid                 = 0;
        patternFixedKeyFormat.ipv4.common.isTagged            = GT_FALSE;
        patternFixedKeyFormat.ipv4.common.dsaSrcIsTrunk       = GT_FALSE;
        patternFixedKeyFormat.ipv4.common.dsaSrcPortTrunk     = 0;
        patternFixedKeyFormat.ipv4.common.dsaSrcDevice        = 0;
        patternFixedKeyFormat.ipv4.common.sourcePortGroupId   = 0;

        patternFixedKeyFormat.ipv4.tunneltype     = 0;

        patternFixedKeyFormat.ipv4.srcIp.arIP[0]  = 0xAA;
        patternFixedKeyFormat.ipv4.srcIp.arIP[1]  = 0xAA;
        patternFixedKeyFormat.ipv4.srcIp.arIP[2]  = 0xAA;
        patternFixedKeyFormat.ipv4.srcIp.arIP[3]  = 0xAA;

        patternFixedKeyFormat.ipv4.destIp.arIP[0] = 0xAA;
        patternFixedKeyFormat.ipv4.destIp.arIP[1] = 0xAA;
        patternFixedKeyFormat.ipv4.destIp.arIP[2] = 0xAA;
        patternFixedKeyFormat.ipv4.destIp.arIP[3] = 0xAA;

        patternFixedKeyFormat.ipv4.isArp          = GT_FALSE;

        /* set default mask */
        cpssOsMemSet(&maskFixedKeyFormat.ipv4.common, 0xFF, sizeof(maskFixedKeyFormat.ipv4.common));

        maskFixedKeyFormat.ipv4.common.pclId              &= BIT_10 - 1;/*10 bits*/
        maskFixedKeyFormat.ipv4.common.srcIsTrunk         &= 0x01;
        maskFixedKeyFormat.ipv4.common.srcPortTrunk       &= BIT_8 - 1;
        maskFixedKeyFormat.ipv4.common.vid                &= BIT_13 - 1;/*13 bits*/
        maskFixedKeyFormat.ipv4.common.isTagged           &= 0x01;
        maskFixedKeyFormat.ipv4.common.dsaSrcIsTrunk      &= 0x01;
        maskFixedKeyFormat.ipv4.common.dsaSrcPortTrunk    &= BIT_13 - 1;
        maskFixedKeyFormat.ipv4.common.dsaSrcDevice       &= BIT_10 - 1;
        maskFixedKeyFormat.ipv4.common.sourcePortGroupId  &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum);

        maskFixedKeyFormat.ipv4.common.sourcePortGroupId  = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum);

        cpssOsMemSet(&maskFixedKeyFormat.ipv4.common + 1, 0xFF,
                     sizeof(maskFixedKeyFormat) - sizeof(maskFixedKeyFormat.ipv4.common));
        maskFixedKeyFormat.ipv4.tunneltype        = 0x00000007;
        maskFixedKeyFormat.ipv4.isArp             = 0x01;

        st = cpssDxChExactMatchTtiRuleConvertToUdbFormat(ruleType,
                                                         &patternFixedKeyFormat,
                                                         &maskFixedKeyFormat,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ruleType);

        /*1.1.1.  Call function with ruleType [CPSS_DXCH_TTI_RULE_IPV4_E,
                                             CPSS_DXCH_TTI_RULE_MPLS_E
                                             CPSS_DXCH_TTI_RULE_ETH_E
                                             CPSS_DXCH_TTI_RULE_MIM_E]
                                   legal patternFixedKeyFormatPtr
                                   legal maskFixedKeyFormatPtr

        Expected: GT_OK.*/
        st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray,
                                                         &patternFixedKeyFormatGet,
                                                         &maskFixedKeyFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ruleType);

        /* compare returned values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &patternFixedKeyFormat.ipv4,
                                     (GT_VOID*) &patternFixedKeyFormatGet.ipv4,
                                     sizeof(CPSS_DXCH_TTI_IPV4_RULE_STC))) ? GT_TRUE : GT_FALSE;
        if(isEqual != GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("get another patternFixedKeyFormat.ipv4 than was set\n");
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, GT_FAIL, devNum, ruleType);
        }

        /* 1.1.3.  Call function with wrong ruleType [CPSS_DXCH_TTI_RULE_UDB_10_E,
                                               CPSS_DXCH_TTI_RULE_UDB_20_E
                                               CPSS_DXCH_TTI_RULE_UDB_30_E]
           Expected: GT_BAD_PARAM*/
        ruleType = CPSS_DXCH_TTI_RULE_UDB_10_E;
        st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray,
                                                         &patternFixedKeyFormatGet,
                                                         &maskFixedKeyFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ruleType);


        /*1.1.4. Call with NULL patternUdbFormatArray and other valid params.
        Expected: GT_BAD_PTR.*/
        ruleType = CPSS_DXCH_TTI_RULE_IPV4_E;
        st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         NULL,
                                                         maskUdbFormatArray,
                                                         &patternFixedKeyFormatGet,
                                                         &maskFixedKeyFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
        /*1.1.5. Call with NULL maskUdbFormatArray and other valid params.
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         patternUdbFormatArray,
                                                         NULL,
                                                         &patternFixedKeyFormatGet,
                                                         &maskFixedKeyFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
        /* 1.1.6. Call with NULL patternFixedKeyFormatGet and other valid params.
        Expected: GT_BAD_PTR.*/
       st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray,
                                                         NULL,
                                                         &maskFixedKeyFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
        /* 1.1.7. Call with NULL maskFixedKeyFormatGet and other valid params.
        Expected: GT_BAD_PTR.*/
       st = cpssDxChExactMatchTtiRuleConvertFromUdbFormat(ruleType,
                                                         patternUdbFormatArray,
                                                         maskUdbFormatArray,
                                                         &patternFixedKeyFormatGet,
                                                         NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ruleType);
    }
}

GT_STATUS   dumpNonValidEmEntries(IN GT_U8  devNum , IN GT_U32 locateError)
{
    GT_STATUS   rc;
    GT_U32  index = 0;
    GT_U32  entryArr[8];
    GT_U32 valid = 0,emKey = 0,emAction = 0;
    while(1)
    {
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,index,&entryArr[0]);
        if(rc != GT_OK)
        {
            break;
        }
        index++;

        if(entryArr[0] & 0x1/*valid*/)
        {
            valid++;
            if(entryArr[0] & 0x2)
            {
                if(locateError)
                {
                    if(emKey > 100)
                    {
                        cpssOsPrintf("index[%d] with EM key \n",
                            (index-1));
                    }
                    else
                    {
                        /* just 'index' */
                        cpssOsPrintf("%d ",index);
                    }
                }

                /* EM key */
                emKey++;
            }
            else
            {
                /* EM Action */
                emAction++;
            }
        }
    }

    cpssOsPrintf("summary[%d] entries : valid[%d] emKey[%d] , emAction[%d] \n",
        index,valid,emKey,emAction);

    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchAutoLearnLookupSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchAutoLearnLookupNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchAutoLearnLookupSet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with exactMatchAutoLearnLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E]
    Expected: GT_OK.
    1.2. Call with out of range exactMatchAutoLearnLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E]
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                exactMatchAutoLearnLookupNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                exactMatchAutoLearnLookupNumGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exactMatchAutoLearnLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E]
                Expected: GT_OK.
        */

        exactMatchAutoLearnLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchAutoLearnLookupSet(devNum, exactMatchAutoLearnLookupNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, exactMatchAutoLearnLookupNum);

        st = cpssDxChExactMatchAutoLearnLookupGet(devNum, &exactMatchAutoLearnLookupNumGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchAutoLearnLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(exactMatchAutoLearnLookupNum, exactMatchAutoLearnLookupNumGet,
                       "get another exactMatchAutoLearnLookupNum than was set: %d", devNum);

        /*
           1.2. Call with out of range exactMatchAutoLearnLookupNum [CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E]
            Expected: NON GT_OK.

        */
        exactMatchAutoLearnLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;

        st = cpssDxChExactMatchAutoLearnLookupSet(devNum, exactMatchAutoLearnLookupNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchAutoLearnLookupNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchAutoLearnLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchAutoLearnLookupSet(devNum, exactMatchAutoLearnLookupNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchAutoLearnLookupSet(devNum, exactMatchAutoLearnLookupNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchAutoLearnLookupGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            *exactMatchAutoLearnLookupNumPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchAutoLearnLookupGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null exactMatchProfileIndex, keyParamsPtr
    Expected: GT_OK.
    1.2. Call with out of range exactMatchProfileIndex [16]
    Expected: NON GT_OK.
    1.3. Call with keyParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT exactMatchAutoLearnLookupNumGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null exactMatchAutoLearnLookupNumPtr
            Expected: GT_OK.
        */

        st = cpssDxChExactMatchAutoLearnLookupGet(devNum, &exactMatchAutoLearnLookupNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /*
            1.2. Call with exactMatchAutoLearnLookupNumPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchAutoLearnLookupGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, exactMatchAutoLearnLookupNumGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchAutoLearnLookupGet(devNum, &exactMatchAutoLearnLookupNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchAutoLearnLookupGet(devNum, &exactMatchAutoLearnLookupNumGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchAutoLearnProfileDefaultActionSet
(
    IN GT_U8                                       devNum,
    IN GT_U32                                      exactMatchProfileIndex,
    IN GT_U32                                      expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT       actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT            *actionPtr,
    IN GT_BOOL                                     actionEn
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionSet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E
    Expected: GT_OK and same action as written
    1.2. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=2,actionEn=GT_TRUE,
         expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.3. Call with out of range exactMatchProfileIndex [16],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range expandedActionIndex [16],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with not vaid expandedActionIndex [5],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values actionType,
        other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call with actionPtr [NULL],
         other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.8. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E
    Expected: GT_OK and same action as written
    1.9. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=7,actionEn=GT_TRUE,
         expandedActionIndex and other params same as in 1.8.
    Expected: GT_OK and same action as written
    1.10. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,exactMatchProfileIndex=9,actionEn=GT_FALSE,
          actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E
    Expected: GT_OK and same action as written
    1.11. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=9,actionEn=GT_TRUE,
          expandedActionIndex and other params same as in 1.10.
    Expected: GT_OK and same action as written
    1.12. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
          actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E
    Expected: GT_OK and same action as written
    1.13. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=11,actionEn=GT_TRUE,
          expandedActionIndex and other params same as in 1.12.
    Expected: GT_OK and same action as written
*/

    GT_STATUS                                st = GT_OK;
    GT_U8                                    devNum = 0;
    GT_U32                                   exactMatchProfileIndex = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionTypeGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionDataGet;
    GT_BOOL                                  actionEn = GT_TRUE;
    GT_BOOL                                  actionEnGet;
    GT_ETHERADDR                             macSa1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    GT_U32                                   expandedActionIndex=0;
    GT_U32                                   expandedActionIndexGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E
            Expected: GT_OK and same action as written

            actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */
        cpssOsBzero((GT_VOID*) &defaultActionData, sizeof(defaultActionData));
        cpssOsBzero((GT_VOID*) &defaultActionDataGet, sizeof(defaultActionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionIndex = 3;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);
        defaultActionData.ttiAction.flowId = 22;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&defaultActionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d", devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&defaultActionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);

        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
            1.2. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=2,actionEn=GT_TRUE,
                 expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written
        */

        exactMatchProfileIndex = 2;
        actionEn = GT_TRUE;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionGet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionEn, actionEnGet, "get another actionEnGet than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expandedActionIndex, expandedActionIndexGet, "get another expandedActionIndexGet than was set: %d", devNum);

       /*
            1.3. Call with out of range exactMatchProfileIndex [16],
                 other params same as in 1.1.
            Expected: non GT_OK.
        */
        exactMatchProfileIndex = 16;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        /*
            1.4. Call with out of range expandedActionIndex [16],
                 other params same as in 1.1.
            Expected: non GT_OK.
        */
        exactMatchProfileIndex = 3;
        expandedActionIndex = 16;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        /*
            1.5. Call with not vaid expandedActionIndex [5],
                 other params same as in 1.1.
            Expected: non GT_OK.
        */
        expandedActionIndex = 5;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        /*
            1.6. Call with wrong enum values actionType, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionSet
                            (devNum, exactMatchProfileIndex, expandedActionIndex, actionType, &defaultActionData, actionEn),
                            actionType);

        /*
            1.7. Call with actionPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,NULL,actionEn);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, defaultActionData = NULL", devNum);

        /*
           1.8. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E
            Expected: GT_OK and same action as written

            set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 8;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);
        defaultActionData.pclAction.flowId = 33;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&defaultActionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                     devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&defaultActionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
            1.9. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=7,actionEn=GT_TRUE,
                 expandedActionIndex and other params same as in 1.8.
            Expected: GT_OK and same action as written
        */

        exactMatchProfileIndex = 7;
        actionEn = GT_TRUE;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionGet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionEn, actionEnGet, "get another actionEnGet than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expandedActionIndex, expandedActionIndexGet, "get another expandedActionIndexGet than was set: %d", devNum);

        /*
          1.10. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
                actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E
                Expected: GT_OK and same action as written

                actionPtr { egressPolicy [GT_FALSE]
                            mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                            redirect {
                               redirectCmd [
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                               data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                          A5:A5:A5:A5:A5:A5 /
                                                          FF:FF:FF:FF:FF:FF],
                                                   arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                            oam { timeStampEnable [GT_FALSE / GT_TRUE],
                                  offsetIndex [0 / 0xA / BIT_4-1],
                                  oamProcessEnable [GT_FALSE / GT_TRUE],
                                  oamProfile [0 / 1] },
                            sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                     sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                            qos { ingress { up1Cmd[
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                           up1[0 / 5 / BIT_3-1]} }
                            vlan { ingress { vlanId1Cmd[
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            flowId [0 / 0xA5A / BIT_12-1],
                            setMacToMe [GT_FALSE / GT_TRUE] }
        */

        defaultActionData.pclAction.egressPolicy = GT_FALSE;
        defaultActionData.pclAction.mirror.mirrorToRxAnalyzerPort = GT_TRUE;

        defaultActionData.pclAction.mirror.ingressMirrorToAnalyzerIndex = 0;
        defaultActionData.pclAction.redirect.redirectCmd =
                                   CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
        defaultActionData.pclAction.redirect.data.modifyMacSa.macSa = macSa1;
        defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
        defaultActionData.pclAction.oam.timeStampEnable = GT_FALSE;
        defaultActionData.pclAction.oam.offsetIndex = 0;
        defaultActionData.pclAction.oam.oamProcessEnable = GT_FALSE;
        defaultActionData.pclAction.oam.oamProfile = 0;
        defaultActionData.pclAction.sourcePort.assignSourcePortEnable = GT_FALSE;
        defaultActionData.pclAction.sourcePort.sourcePortValue = 0;
        defaultActionData.pclAction.qos.ingress.up1Cmd =
                       CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
        defaultActionData.pclAction.qos.ingress.up1 = 0;
        defaultActionData.pclAction.vlan.ingress.vlanId1Cmd =
                  CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
        defaultActionData.pclAction.vlan.ingress.vlanId1 = 0;
        defaultActionData.pclAction.flowId = 0;
        defaultActionData.pclAction.setMacToMe = GT_FALSE;

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 9;
        actionEn = GT_FALSE;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&defaultActionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                     devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&defaultActionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
            1.11. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=9,actionEn=GT_TRUE,
                 expandedActionIndex and other params same as in 1.10.
            Expected: GT_OK and same action as written
        */

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionGet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionEn, actionEnGet, "get another actionEnGet than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expandedActionIndex, expandedActionIndexGet, "get another expandedActionIndexGet than was set: %d", devNum);

        /*1.12. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
                actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E
                Expected: GT_OK and same action as written

            actionPtr { egressPolicy [GT_TRUE]
                        qos { egress { modifyDscp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                      modifyUp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                      up1ModifyEnable [GT_FALSE / GT_TRUE],
                                      up1[0 / 5 / BIT_3-1]} }
                        vlan { egress { vlanCmd[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                       vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                        tmQueueId [0 / 0x2A5A / BIT_14-1] }
            and other params from 1.8.
        Expected: GT_OK and same action as written */

        defaultActionData.pclAction.egressPolicy = GT_TRUE;
        defaultActionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        defaultActionData.pclAction.actionStop = GT_FALSE;
        defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
        defaultActionData.pclAction.sourcePort.sourcePortValue = 0;

        defaultActionData.pclAction.qos.egress.modifyDscp =
                            CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
        defaultActionData.pclAction.qos.egress.modifyUp =
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
        defaultActionData.pclAction.qos.egress.up1ModifyEnable = GT_FALSE;
        defaultActionData.pclAction.qos.egress.up1 = 0;
        defaultActionData.pclAction.vlan.egress.vlanCmd =
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
        defaultActionData.pclAction.vlan.egress.vlanId1ModifyEnable = GT_FALSE;
        defaultActionData.pclAction.vlan.egress.vlanId1 = 0;
        defaultActionData.pclAction.channelTypeToOpcodeMapEnable = GT_FALSE;
        defaultActionData.pclAction.tmQueueId = 0;
        defaultActionData.pclAction.flowId = 55;

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;
        expandedActionIndex = 11;
        actionEn = GT_TRUE;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&defaultActionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                     devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&defaultActionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
            1.11. Call cpssDxChExactMatchAutoLearnProfileDefaultActionSet exactMatchProfileIndex=11,actionEn=GT_TRUE,
                 expandedActionIndex and other params same as in 1.12.
            Expected: GT_OK and same action as written
        */

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionGet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionEn, actionEnGet, "get another actionEnGet than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expandedActionIndex, expandedActionIndexGet, "get another expandedActionIndexGet than was set: %d", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    expandedActionIndex=3;
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchAutoLearnProfileDefaultActionGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      exactMatchProfileIndex,
    OUT GT_U32                                     *expandedActionIndexPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT      *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT           *actionPtr,
    OUT GT_BOOL                                    *actionEnPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null expandedActionIndexPtr, actionTypePtr, actionPtr, actionEnPtr
    Expected: GT_OK.
    1.2. Call with non null expandedActionIndexPtr, actionTypePtr, actionPtr, actionEnPtr and exactMatchProfileIndex=16
    Expected: GT_BAD_PARAM.
    1.3. Call with expandedActionIndexPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with actionTypePtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with actionPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with actionEnPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                st = GT_OK;
    GT_U8                                    devNum = 0;
    GT_U32                                   exactMatchProfileIndex = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionTypeGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionDataGet;
    GT_BOOL                                  actionEn = GT_TRUE;
    GT_BOOL                                  actionEnGet;
    GT_U32                                   expandedActionIndex;
    GT_U32                                   expandedActionIndexGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionIndex = 3;
        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);
        defaultActionData.ttiAction.flowId = 22;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&defaultActionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d", devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&defaultActionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);

        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        exactMatchProfileIndex = 2;
        actionEn = GT_TRUE;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum,exactMatchProfileIndex,expandedActionIndex,actionType,&defaultActionData,actionEn);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "cpssDxChExactMatchAutoLearnProfileDefaultActionSet: %d, %d, %d, %d, %d",
                                     devNum, exactMatchProfileIndex, expandedActionIndex, actionType, actionEn);

        /*
             1.1. Call with non null expandedActionIndexPtr, actionTypePtr, actionPtr, actionEnPtr
            Expected: GT_OK.
        */
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /*
             1.2. Call with non null expandedActionIndexPtr, actionTypePtr, actionPtr, actionEnPtr and exactMatchProfileIndex=16
            Expected: GT_BAD_PARAM.
        */
        exactMatchProfileIndex=16;
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with expandedActionIndexPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=15;
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,NULL,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.4. Call with actionTypePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,NULL,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.5. Call with actionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,NULL,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
             1.6. Call with actionEnPtr [NULL].
             Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=15;
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchProfileIndex = 1;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchAutoLearnProfileDefaultActionGet(devNum,exactMatchProfileIndex,&expandedActionIndexGet,&actionTypeGet,&defaultActionDataGet,&actionEnGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet
(
    IN  GT_U8                                                   devNum,
    IN  GT_PORT_GROUPS_BMP                                      portGroupsBmp,
    IN  CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC    *confPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with confPtr
    Expected: GT_OK.
    1.2. Call with out of range conf.baseFlowId [0x10000]
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range conf.maxNum [0x10000]
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range conf.threshold [0x10000]
    Expected: GT_BAD_PARAM.
    1.5. Call with confPtr [NULL]
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC conf;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC confGet;
    GT_U32 i,legalBaseFlowId,outOfRangelBaseFlowId;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* disable auto learning on all profiles */
        for (i=PRV_CPSS_DXCH_EXACT_MATCH_MIN_PROFILE_ID_VAL_CNS; i<=PRV_CPSS_DXCH_EXACT_MATCH_MAX_PROFILE_ID_VAL_CNS; i++)
        {
            st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum,i,GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, i);
        }

        /*
            1.1. Call with confPtr
                Expected: GT_OK.
        */
       legalBaseFlowId = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.plrInfo.countingEntriesNum/6 |(cpssOsRand() & 0xFF);  /* 8Bits Random */
       outOfRangelBaseFlowId=PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.plrInfo.countingEntriesNum+1;

       conf.baseFlowId = legalBaseFlowId;
       conf.maxNum = 100;
       conf.threshold = 90;

       st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&conf);
       UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, conf.baseFlowId, conf.maxNum, conf.threshold);

       st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&confGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet: %d", devNum);

       /* verifying values */
       UTF_VERIFY_EQUAL1_STRING_MAC(conf.baseFlowId, confGet.baseFlowId,
                                    "get another conf.baseFlowId than was set: %d", devNum);
       UTF_VERIFY_EQUAL1_STRING_MAC(conf.maxNum, confGet.maxNum,
                                    "get another conf.maxNum than was set: %d", devNum);
       UTF_VERIFY_EQUAL1_STRING_MAC(conf.threshold, confGet.threshold,
                                    "get another conf.threshold than was set: %d", devNum);

        /*
           1.2. Call with out of range conf.baseFlowId [0x10000]
            Expected: NON GT_OK.

        */
        conf.baseFlowId = outOfRangelBaseFlowId;

        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&conf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
           1.3. Call with out of range conf.maxNum [0x10000]
           Expected: NON GT_OK.

        */
        conf.baseFlowId = legalBaseFlowId;
        conf.maxNum = 0x10000;

        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&conf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
           1.4. Call with out of range conf.threshold [0x10000]
           Expected: NON GT_OK.

        */
        conf.maxNum = 100;
        conf.threshold = 0x10000;

        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&conf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
           1.5. Call with confPtr [NULL]
            Expected: NON GT_OK.

        */
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    conf.threshold = 90;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&conf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&conf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet
(
    IN  GT_U8                                                     devNum,
    IN  GT_PORT_GROUPS_BMP                                        portGroupsBmp,
    OUT CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC       *confPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null confPtr
    Expected: GT_OK.
    1.2. Call with confPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC confGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
       /*
            1.1. Call with non null confPtr
            Expected: GT_OK.
       */
       st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&confGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet: %d", devNum);

        /*
            1.2. Call with confPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&confGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&confGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet
(
    IN GT_U8                                       devNum,
    IN GT_U32                                      exactMatchProfileIndex,
    IN GT_BOOL                                     actionEn
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with exactMatchProfileIndex, actionEn=GT_TRUE
    Expected: GT_OK.
    1.2. Call with exactMatchProfileIndex, actionEn=GT_FALSE
    Expected: GT_OK.
    1.3. Call with out of range exactMatchProfileIndex [16]
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_U32                                          exactMatchProfileIndex;
    GT_BOOL                                         actionEn;
    GT_BOOL                                         actionEnGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exactMatchProfileIndex, actionEn=GT_TRUE
                Expected: GT_OK.
        */

       exactMatchProfileIndex = 3;
       actionEn = GT_TRUE;

       st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum,exactMatchProfileIndex,actionEn);
       UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchProfileIndex, actionEn);

       st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,&actionEnGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet: %d", devNum);

       /* verifying values */
       UTF_VERIFY_EQUAL1_STRING_MAC(actionEn, actionEnGet,
                                    "get another actionEn than was set: %d", devNum);

       /*
            1.2. Call with exactMatchProfileIndex, actionEn=GT_FALSE
                Expected: GT_OK.
        */

       exactMatchProfileIndex = 4;
       actionEn = GT_FALSE;

       st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum,exactMatchProfileIndex,actionEn);
       UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchProfileIndex, actionEn);

       st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,&actionEnGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet: %d", devNum);

       /* verifying values */
       UTF_VERIFY_EQUAL1_STRING_MAC(actionEn, actionEnGet,
                                    "get another actionEn than was set: %d", devNum);

        /*
           1.3. Call with out of range exactMatchProfileIndex [16]
            Expected: NON GT_OK.

        */
        exactMatchProfileIndex = 16;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum,exactMatchProfileIndex,actionEn);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchProfileIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchProfileIndex = 7;
    actionEn=GT_TRUE;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum,exactMatchProfileIndex,actionEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum,exactMatchProfileIndex,actionEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      exactMatchProfileIndex,
    OUT GT_BOOL                                    *actionEnPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null exactMatchProfileIndex, actionEnPtr
    Expected: GT_OK.
    1.2. Call with out of range exactMatchProfileIndex [16]
    Expected: GT_BAD_PARAM.
    1.3. Call with actionEnPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    GT_U32 exactMatchProfileIndex;
    GT_BOOL actionEnGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null exactMatchProfileIndex, actionEnPtr
            Expected: GT_OK.
        */
        exactMatchProfileIndex=2;

        st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with out of range exactMatchProfileIndex [16]
                Expected: NON GT_OK.
        */
        exactMatchProfileIndex=16;
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,&actionEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet = NULL", devNum);

         /*
            1.3. Call with keyParamsPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=1;
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    exactMatchProfileIndex=2;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,&actionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet(devNum,exactMatchProfileIndex,&actionEnGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PORT_GROUPS_BMP                                          portGroupsBmp,
    OUT CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC   *statusPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null statusPtr
    Expected: GT_OK.
    1.2. Call with statusPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC   statusGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null statusPtr
            Expected: GT_OK.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&statusGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /*
            1.2. Call with statusPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&statusGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&statusGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnEntryGetNext
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT GT_U32                                  *flowIdPtr,
    OUT GT_U32                                  *exactMatchIndexPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnEntryGetNext)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null flowIdPtr, exactMatchIndexPtr
    Expected: GT_OK.
    1.2. Call with statusPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3 Call with exactMatchIndexGet [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    GT_U32                                  flowIdGet;
    GT_U32                                  exactMatchIndexGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null flowIdPtr, exactMatchIndexPtr
            Expected: GT_OK or GT_NO_MORE.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,&exactMatchIndexGet);
        if (st != GT_OK && st != GT_NO_MORE)
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /*
            1.2. Call with flowIdGet [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL,&exactMatchIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnEntryGetNext = NULL", devNum);

        /*
            1.3. Call with exactMatchIndexGet [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnEntryGetNext = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,&exactMatchIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnEntryGetNext(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,&exactMatchIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet
(
    IN  GT_U8                                                       devNum,
    IN GT_PORT_GROUPS_BMP                                           portGroupsBmp,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC                             *keyParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null keyParamsPtr
    Expected: GT_FAIL - no collision entry to read.
    1.2. Call with keyParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC keyParamsGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
       /*
            1.1. Call with non null keyParamsPtr
            Expected: GT_FAIL; no collision entry to read..
       */
       st = cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&keyParamsGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_FAIL, st,"cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet: %d", devNum);

        /*
            1.2. Call with keyParamsPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&keyParamsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&keyParamsGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT GT_U32                                  *flowIdPtr,
    OUT GT_U32                                  *exactMatchIndexPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null flowIdPtr, exactMatchIndexPtr
    Expected: GT_FAIL - no entry to recycle.
    1.2. Call with statusPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3 Call with exactMatchIndexGet [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    GT_U32                                  flowIdGet;
    GT_U32                                  exactMatchIndexGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null flowIdPtr, exactMatchIndexPtr
            Expected: GT_EMPTY - no entry to recycle.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,&exactMatchIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_EMPTY, st, devNum);

         /*
            1.2. Call with flowIdGet [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL,&exactMatchIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet = NULL", devNum);

        /*
            1.3. Call with exactMatchIndexGet [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,&exactMatchIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&flowIdGet,&exactMatchIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null
    Expected: GT_FAIL - no entries were learned.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null
            Expected: GT_EMPTY - no entries were learned.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_EMPTY, st, devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupAutoLearnFailCountersGet
(
    IN GT_U8                                                devNum,
    IN GT_PORT_GROUPS_BMP                                   portGroupsBmp,
    OUT CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC  *failCounterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupAutoLearnFailCountersGet)
{
/*
    ITERATE_DEVICES (SIP6_10)
    1.1. Call with non null failCounterPtr
    Expected: GT_OK.
    1.2. Call with failCounterPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC failCounterGet;

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
       /*
            1.1. Call with non null failCounterPtr
            Expected: GT_OK.
       */
       st = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&failCounterGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchPortGroupAutoLearnFailCountersGet: %d", devNum);

        /*
            1.2. Call with failCounterPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupAutoLearnFailCountersGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Hawk devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_10_CNS) ;
    PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&failCounterGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchPortGroupAutoLearnFailCountersGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,&failCounterGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChExactMatch suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChExactMatch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiProfileIdModePacketTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiProfileIdModePacketTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiPortProfileIdModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiPortProfileIdModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiPortProfileIdModePortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiPortProfileIdModePortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPclProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPclProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchClientLookupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchClientLookupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchActivityBitEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchActivityBitEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchActivityStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileKeyParamsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileKeyParamsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileDefaultActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileDefaultActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchExpandedActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchExpandedActionSpecificScenarioSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchExpandedActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupEntryStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchHashCrcMultiResultsByParamsCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchRulesDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiRuleConvertToUdbFormat)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiRuleConvertFromUdbFormat)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchAutoLearnLookupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchAutoLearnLookupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnEntryGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupAutoLearnFailCountersGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChExactMatch)


