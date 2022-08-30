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
* @file tgfCommonExactMatch.c
*
* @brief Enhanced UTs for CPSS Exact Match
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfPclGen.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>
#include <exactMatch/prvTgfExactMatchBasicExpandedActionPcl.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionTti.h>
#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <exactMatch/prvTgfExactMatchDefaultActionUdbPcl.h>
#include <exactMatch/prvTgfExactMatchExpandedActionUdbPcl.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>
#include <exactMatch/prvTgfExactMatchBasicExpandedActionTti.h>
#include <exactMatch/prvTgfExactMatchReducedActionTti.h>
#include <exactMatch/prvTgfExactMatchReducedActionTtiFixedKey.h>
#include <exactMatch/prvTgfExactMatchAutoLearnTti.h>

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 2

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchBasicDefaultActionPcl

    configure VLAN, FDB entries
    configure PCL rules - to trap
    send traffic
    verify traffic is trapped

    Invalidate PCL Rule
    Set Exact Match Default Action configuration
    verify traffic is drop
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicDefaultActionPcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL configuration
        3. Generate traffic - expect match in TCAM - traffic trapped
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match Default entry hit - traffic drop
        6. Restore PCL configuration
        7. Restore Exact Match configuration
    */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl configuration */
    prvTgfExactMatchBasicDefaultActionPclGenericConfig();

    /* Set Exact Match Default Action PCL configuration */
    prvTgfExactMatchBasicDefaultActionPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicDefaultActionPclTrafficGenerate(GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchBasicDefaultActionPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Default Action */
    prvTgfExactMatchBasicDefaultActionPclTrafficGenerate(GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchBasicDefaultActionPclGenericRestore();

    /* Restore Exact Match Default Action PCL configuration */
    prvTgfExactMatchBasicDefaultActionPclConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchBasicExpandedActionPcl

    configure VLAN, FDB entries
    configure PCL rules - to trap
    send traffic
    verify traffic is trapped

    Invalidate PCL Rule - to prevent a hit in TCAM
    Set Exact Match Profile key parameters
    Set Exact Match Expanded Action configuration
    Set Exact Match Entry configuration
    verify traffic is drop

    Invalidate the exact match entry
    verify traffic is forwarded (FDB)
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicExpandedActionPcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL configuration
        3. Generate traffic - expect match in TCAM - traffic trapped
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic drop
        6. Invalidate Exact Match Entry
        7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        8. Restore PCL configuration
        9. Restore Exact Match configuration
    */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl configuration */
    prvTgfExactMatchBasicExpandedActionPclGenericConfig();

    /* Set Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchBasicExpandedActionPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchBasicExpandedActionPclExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* set back TCAM priotity over Exact Match */
    prvTgfExactMatchBasicExpandedActionPclExactMatchPriorityConfig(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchBasicExpandedActionPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchBasicExpandedActionPclInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchBasicExpandedActionPclGenericRestore();

    /* Restore Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchBasicExpandedActionPclConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchDefaultActionUdb40EthOthreL2Pcl

    configure VLAN, FDB entries;
    configure PCL rule and different UDBs - forward to specific port;

    Test on Ingress PCL UDB40 L2 offset type on Ethernet Other packet.
    Fields: 40 first bytes from L2 header beginning.
    UDB mapping 10-49 => L2+0 - L2+39

    send traffic to match PCL rules and UDBs;
    verify traffic is forwarded with trigger.

    Invalidate PCL Rule
    Set Exact Match Default Action configuration
    verify traffic is drop
*/
UTF_TEST_CASE_MAC(tgfExactMatchDefaultActionUdb40EthOthreL2Pcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL UDB configuration
        3. Generate traffic - expect match in TCAM - traffic forwarded with trigger
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match Default entry hit - traffic drop
        6. Restore PCL configuration
        7. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl UDB configuration
       Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet */
    prvTgfExactMatchUdb40EthOthreL2PclGenericConfig();

    /* Set Exact Match Default Action PCL configuration */
    prvTgfExactMatchDefaultActionUdbPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchDefaultActionUdbPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Default Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchUdb40EthOthreL2PclGenericRestore();

    /* Restore Exact Match Default Action PCL configuration */
    prvTgfExactMatchDefaultActionUdbPclConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchExpandedActionUdb40EthOthreL2Pcl

    configure VLAN, FDB entries
    configure PCL rules - forward to specific port

    Test on Ingress PCL UDB40 L2 offset type on Ethernet Other packet.
    Fields: 40 first bytes from L2 header beginning.
    UDB mapping 10-49 => L2+0 - L2+39

    send traffic
    verify traffic is forwarded with trigger.

    Invalidate PCL Rule - to prevent a hit in TCAM
    Set Exact Match Profile key parameters
    Set Exact Match Expanded Action configuration
    Set Exact Match Entry configuration
    verify traffic is drop

    Invalidate the exact match entry
    verify traffic is forwarded (FDB) - with specific VLAN

*/
UTF_TEST_CASE_MAC(tgfExactMatchExpandedActionUdb40EthOthreL2Pcl)
{
      /*
        1. Set Exact Match configuration
        2. Set PCL UDB configuration
        3. Generate traffic - expect match in TCAM - traffic forward
        4. Invalidate PCL Rule - to prevent a hit in TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic drop
        6. Invalidate Exact Match Entry
        7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        8. Restore PCL configuration
        9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Set PCl UDB configuration
       Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet */
    prvTgfExactMatchUdb40EthOthreL2PclGenericConfig();

    /* Set Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchExpandedActionUdbPclConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* set back TCAM priotity over Exact Match */
    prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE, GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchExpandedActionUdbPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE, GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchExpandedActionUdbPclInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchUdb40EthOthreL2PclGenericRestore();

    /* Restore Exact Match Expanded Action PCL configuration */
    prvTgfExactMatchExpandedActionUdbPclConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPath:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry

*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPath)
{
    /*
    1. Set Exact Match Non Default Action configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic drop
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigSet(CPSS_PACKET_CMD_TRAP_TO_CPU_E);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_TRUE,GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPort:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPort)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 3 from Expanded action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1 (from FDB)
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigSet(CPSS_PACKET_CMD_FORWARD_E);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReduced:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReduced)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedNonZeroKeyStart:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   Key Start parameter different from 0
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonZeroKeyStart)
{
    /*
    1. Set Exact Match configuration - Key Start parameter different from 0
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams.keySize to be != 0
       UDB0 should be removed from TCAM key -
       and we should get a hit on the Exact Match*/
    prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet();

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedNonFullKeyMask:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   Key Start parameter different from 0
   Key Mask parameter different from 0xFF
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonFullKeyMask)
{
    /*
    1. Set Exact Match configuration -
       Key Start parameter different from 0
       Key Mask parameter different from 0xFF
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams.keySize to be != 0
       UDB0 should be removed from TCAM key -
       and we should get a hit on the Exact Match*/
    prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet();

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathUdb40ConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams
       keyParams.keySize = 47 bytes
       keyParams.keyStart = 1
       keyParams.mask != 0xff
       UDB0 should be removed from TCAM key and we should get a hit on the Exact Match */
    prvTgfExactMatchTtiPclFullPathUdb47ConfigSet();

   /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47CheckActivity:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   check activity bit update
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47CheckActivity)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. check Activity bit = 0
    5. Generate traffic - expect Exact Match hit - traffic forwarded to port 2 from reduced action
    6. check Activity bit = 1
    7  clear Activity
    8. Invalidate Exact Match Entry
    9. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    10. check Activity bit = 0
    11. Restore TTI configuration
    12. Restore PCL configuration
    13. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathUdb47ConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();

    /* Set Exact Match keyParams
       keyParams.keySize = 47 bytes
       keyParams.keyStart = 1
       keyParams.mask != 0xff
       UDB0 should be removed from TCAM key and we should get a hit on the Exact Match */
    prvTgfExactMatchTtiPclFullPathUdb47ConfigSet();

    /* no traffic hit - expect activity =0 */
    prvTgfExactMatchTtiPclFullPathActivitySet(GT_FALSE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* traffic hit - expect activity = 1 */
    prvTgfExactMatchTtiPclFullPathActivitySet(GT_TRUE);

    /* disable global activity bit */
    prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_FALSE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* traffic hit - expect activity = 0, since it was globaly disabled */
    prvTgfExactMatchTtiPclFullPathActivitySet(GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchTtiPclFullPathReducedUdbAllCheckActivity:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect MacSA to be changed from Exact Match
   Expect traffic to be forward to port from FDB
   check activity bit update
*/
UTF_TEST_CASE_MAC(tgfExactMatchTtiPclFullPathReducedUdbAllCheckActivity)
{
    PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT key;

    /*
    1.  Set Exact Match configuration
    2.  Set TTI UDB configuration
    3.  Set PCL UDB configuration
    4.  For all keys (5B,19B,33B,47B):
        - enable global activity bit
        - check Activity bit = 0
        - Generate traffic - expect Exact Match hit for key - traffic forwarded to port 2 from FDB, macSA changed from reduced action.
        - check Activity bit = 1
        - disbale global activity bit
        - check Activity bit = 0
        - Generate traffic - expect Exact Match hit for key - traffic forwarded to port 2 from FDB, macSA changed from reduced action.
    5. Invalidate Exact Match Entry
    6. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 2 from FDB, macSA is not changed.
    7. Restore TTI configuration
    8. Restore PCL configuration
    9. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match; with Sub Profile ID 1 */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(1);

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathReducedConfigMacSaSet(CPSS_PACKET_CMD_FORWARD_E);

    /* invalidate Exact Match Entry configured in prvTgfExactMatchTtiPclFullPathReducedConfigMacSaSet
       leaving only the entry that will be configured in prvTgfExactMatchTtiPclFullPathUdbAllConfigSet
       should be called before configuration on new Exact Match entry and key configuration */
    prvTgfExactMatchTtiPclFullPathInvalidateEmEntry();


    for (key=PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E; key<PRV_TGF_EXACT_MATCH_KEY_SIZE_LAST_E; key++)
    {
        /* enable global activity bit */
        prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_TRUE);

        /* Set Exact Match keyParams
           keyParams.keySize = according to key
           keyParams.keyStart = 2
           keyParams.mask != 0xff
           UDB0,1 should be removed from TCAM key and we should get a hit on the Exact Match */
        prvTgfExactMatchTtiPclFullPathUdbAllConfigSet(key);

        /* no traffic hit - expect activity =0 */
        prvTgfExactMatchTtiPclFullPathActivitySet(GT_FALSE);

        /* Generate traffic - check no traffic, no match in TCAM,
           get Exact Match Expanded Action */
        prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

        /* traffic hit - expect activity = 1 */
        prvTgfExactMatchTtiPclFullPathActivitySet(GT_TRUE);

        /* disbale global activity bit */
        prvTgfExactMatchActivityBitEnableSet(prvTgfDevNum,GT_FALSE);

        /* Generate traffic - check no traffic, no match in TCAM,
           get Exact Match Expanded Action */
        prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

        /* traffic hit - expect activity = 0, since it was globaly disabled */
        prvTgfExactMatchTtiPclFullPathActivitySet(GT_FALSE);

        /* invalidate Exact Match Entry */
        prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry();

    }

    /* Restore Exact Match Test related configuration */
    prvTgfExactMatchTtiPclFullPathUdbAllConfigRestore();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded with no change to macSA */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchTtiPclFullPathConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   tgfExactMatchBasicDefaultActionTti :
   Test functionality of Exact Match default action ,
   base on tgfTunnelTermLlcNonSnapType
   Exact Match default action gets active incase no hit in Tcam or Exact Match lookup
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicDefaultActionTti)
{
    /*
    1. Set Exact match profileId and Default Action
    2. Set TTI Ether Type Key Base configuration
    3. Generate traffic -- check traffic pass
    4. disable TTI rule
    5. Generate traffic -- check traffic block
    6. change Default Action for Trap
    7. Generate traffic -- check traffic block and cpuCode
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet();

    /* Set Exact Match Default Action TTI basic configuration */
    prvTgfExactMatchBasicDefaultActionTtiConfigSet();

    /* Generate traffic - expect to be forwarded */
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* disable TTI rule */
    prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet(GT_FALSE);

    /* Generate traffic -- check no traffic and no cpuCode */
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_FALSE,GT_FALSE);

    /* change Default Action for Trap */
    prvTgfExactMatchBasicDefaultActionTtiCommandTrap();

    /* Generate traffic -- check no traffic and cpuCode 502*/
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore();

    /* Restore Exact match Default Action TTI Configuration */
    prvTgfExactMatchBasicDefaultActionTtiConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchBasicExpandedActionTti :
   Test functionality of Exact Match Expanded Action ,

   Exact Match Expanded action gets active incase no hit in Tcam or
   proity bit in TTI action is set

   Test on PRV_TGF_TTI_RULE_UDB_30_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
   first 4 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchBasicExpandedActionTti)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration -Trap
        3. Generate traffic - expect match in TCAM - traffic forwarded
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap
        6. set Exact Match priority over TCAM back to FALSE
        7. Generate traffic - expect match in TCAM - traffic forwarded
        8. Invalidate TTI Rule - to prevent a hit in TCAM
        9. Generate traffic - expect Exact Match entry hit - traffic Trap
        10. Invalidate Exact Match Entry
        11. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        12. Restore TTI configuration
        13. Restore Exact Match configuration
    */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiConfigSet();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchBasicExpandedActionTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_TRUE, GT_FALSE);

    /* set back TCAM priotity over Exact Match */
    prvTgfExactMatchBasicExpandedActionTtiExactMatchPriorityConfig(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE, GT_TRUE);

    /* invalidate TTI Rule */
    prvTgfExactMatchBasicExpandedActionTtiRuleValidStatusSet(GT_FALSE);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_TRUE, GT_FALSE);

    /* invalidate Exact Match Entry */
    prvTgfExactMatchBasicExpandedActionTtiInvalidateEmEntry();

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchReducedActionTti :
   Test functionality of Exact Match Reduced Action ,

    Test on PRV_TGF_TTI_RULE_UDB_10_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
    first 7 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchReducedActionTti)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration -Trap with CPU code 503
        3. Generate traffic - expect match in TCAM - traffic forwarded
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap code 503
        6. change Reduced Action to Trap with CPUcode 505 and Expanded Action to forward
        8. Generate traffic - expect Exact Match entry hit - traffic Trap with CPU code 505
        9. Restore TTI configuration
        10. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigSet(GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_TRUE,GT_FALSE,0,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 503
    */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE,503,GT_FALSE);

    /* Invalidate EM entry then add it again with:
       changed Reduced Action to Trap with CPUcode 505 and Expanded Action to forward
    */
    prvTgfExactMatchReducedActionTtiReducedActionSet(GT_FALSE);

    /* Generate traffic - check no traffic - CPUcode 505
    */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE, GT_TRUE,505,GT_FALSE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigRestore();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchReducedActionTtiFixedKey :
   Test functionality of Exact Match Reduced Action
   Sending an IPV4 over IPV4 packet and using the translation APIs
   to set UDB pattern for IPV4 legacy key

   Test on PRV_TGF_TTI_RULE_UDB_30_E and not PRV_TGF_TTI_RULE_UDB_10_E as in tgfExactMatchReducedActionTti
   PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E and not PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet as in tgfExactMatchReducedActionTti
*/
UTF_TEST_CASE_MAC(tgfExactMatchReducedActionTtiFixedKey)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration
        3. Generate traffic - expect match in TCAM - Trap with CPU code 500
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap code 503
        6. change Reduced Action to Trap with CPUcode 505 and Expanded Action to forward
        8. Generate traffic - expect Exact Match entry hit - traffic Trap with CPU code 505
        9. Restore TTI configuration
        10. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigSetFixedKey();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigSetFixedKey();

    /* Generate traffic - check match in TCAM - check no traffic - CPUcode 500 */
    prvTgfExactMatchReducedActionTtiTrafficGenerateFixedKey(GT_FALSE,GT_TRUE,500);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchReducedActionTtiExactMatchPriorityConfigFixedKey(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 503*/
    prvTgfExactMatchReducedActionTtiTrafficGenerateFixedKey(GT_FALSE,GT_TRUE,503);

    /* Invalidate EM entry then add it again with:
       changed Reduced Action to Trap with CPUcode 505 and Expanded Action to forward*/
    prvTgfExactMatchReducedActionTtiReducedActionSetFixedKey();

    /* Generate traffic - check no traffic - CPUcode 505*/
    prvTgfExactMatchReducedActionTtiTrafficGenerateFixedKey(GT_FALSE, GT_TRUE,505);

    /* Restore TTI generic configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigRestoreFixedKey();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigRestoreFixedKey();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchReducedActionTtiFlowIdMaskCheck :
   Test functionality of Exact Match Reduced Action with flowId field with
   mask different then full. part of the flowId will be taken from reduced
   and part from the extended

    Test on PRV_TGF_TTI_RULE_UDB_10_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
    first 7 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchReducedActionTtiFlowIdMaskCheck)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration -Trap with CPU code 503
        3. Generate traffic - expect match in TCAM - traffic forwarded
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap code 503
        6. change Reduced Action to Trap with CPUcode 505 and Expanded Action to forward
        8. Generate traffic - expect Exact Match entry hit - traffic Trap with CPU code 505
        9. Restore TTI configuration
        10. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigSet(GT_TRUE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_TRUE,GT_FALSE,0,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 503*/
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE,503,GT_TRUE);

    /* Invalidate EM entry then add it again with:
       changed Reduced Action to Trap with CPUcode 505 and Expanded Action to forward*/
    prvTgfExactMatchReducedActionTtiReducedActionSet(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 505*/
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE, GT_TRUE,505,GT_TRUE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigRestore();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchReducedActionTtiConfigRestore();
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchAutoLearnTti :
   Test functionality of Exact Match AutoLearn,

    Test on PRV_TGF_TTI_RULE_UDB_10_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
    first 7 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTti)
{
      /*
        1. Set TTI configuration
        2. Set Exact Match Expanded Action TTI configuration -Trap with CPU code 503
        3. Generate traffic - expect match in TCAM - traffic forwarded
        4. set Exact Match priority over TCAM
        5. Generate traffic - expect Exact Match entry hit - traffic Trap code 503
        6. Restore TTI configuration
        7. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI configuration */
    prvTgfExactMatchAutoLearnTtiGenericConfigSet();

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(4);

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchAutoLearnTtiConfigSet();

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Generate traffic - check match in TCAM - entry was learned */
    prvTgfExactMatchAutoLearnTtiTrafficGenerate(GT_TRUE,GT_FALSE,0);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchAutoLearnTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode=(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS + 3) */
    prvTgfExactMatchAutoLearnTtiTrafficGenerate(GT_FALSE,GT_TRUE,(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS + 3));

    /* Restore TTI generic configuration */
    prvTgfExactMatchAutoLearnTtiGenericConfigRestore();

    /* Restore Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchAutoLearnTtiConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(1);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTTtiPclFullPathRedirectToPortReduced:
   Test functionality of Exact Match Action
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReduced)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Generate traffic - expect Exact Match hit on an autoLearn Entry - traffic forwarded to port 2 from reduced action
    6. Invalidate Exact Match Entry
    7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    8. Restore TTI configuration
    9. Restore PCL configuration
    10. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(4);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

   /* Generate traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from reduced action */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend(1);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(1);

    /* Generate traffic - expect no hit in TCAM or Exact Match
       - traffic forwarded to port 1*/
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend(0);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTestForEmulator:
   Test functionality of Exact Match Action
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTestForEmulator)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Generate traffic - expect Exact Match hit on an autoLearn Entry - traffic forwarded to port 2 from reduced action
    6. Invalidate Exact Match Entry
    7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    8. Restore TTI configuration
    9. Restore PCL configuration
    10. Restore Exact Match configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(4);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

   /* Generate traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from reduced action */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend(1);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(1);

    /* Generate traffic - expect no hit in TCAM or Exact Match
       - traffic forwarded to port 1*/
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend(0);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearning:
   Test functionality of Exact Match Action with multiple learning
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearning)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Generate traffic - 4 packets - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action
    6. Invalidate 4 Exact Match Entry
    7. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    8. Generate traffic - 4 packets - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action
    9. Invalidate 2 Exact Match Entry
    10.Generate 2 packets traffic - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled
    11.Invalidate 4 Exact Match Entry
    12.Restore TTI configuration
    13.Restore PCL configuration
    14.Restore IPFIX configuration
    15.Restore Exact Match and Auto Learn configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(4);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

    /* Generate 4 packets traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled*/
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,4);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(4);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend(0);

    /* Generate 4 packets traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,4);

    /* invalidate 2 Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(2);

    /* Generate 2 packets traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,2);

    /* invalidate 4 Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(4);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers:
   Test functionality of Exact Match Action with multiple learning
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Send traffic + invalidate EM entries and check EM pointers.
    6.Restore TTI configuration
    7.Restore PCL configuration
    8.Restore IPFIX configuration
    9.Restore Exact Match and Auto Learn configuration
    */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(10);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios(10);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers:
   Test functionality of Exact Match Action with multiple learning
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers2)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Send traffic + invalidate EM entries and check EM pointers.
    6.Restore TTI configuration
    7.Restore PCL configuration
    8.Restore IPFIX configuration
    9.Restore Exact Match and Auto Learn configuration
    */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(20);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios2(20);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers:
   Test functionality of Exact Match Action with multiple learning
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers3)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Send traffic + invalidate EM entries and check EM pointers.
    6.Restore TTI configuration
    7.Restore PCL configuration
    8.Restore IPFIX configuration
    9.Restore Exact Match and Auto Learn configuration
    */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(10);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios3(10);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningMaxFlow:
   Test functionality of Exact Match Action with multiple learning
   Test that we cannot learn more than the maximum defined
   and we get a counter update if we try to learn more then allowed
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningMaxFlow)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Generate traffic - 5 packets - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action
    6. check we cannot learn more than the maximum defined
       and we get a counter update on the last packet send
    7. Invalidate 4 Exact Match Entry
    8. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    9. Generate traffic - 5 packets - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action
    10.check we cannot learn more than the maximum defined
       and we get a counter update on the last packet send
    11.Invalidate 4 Exact Match Entry
    12.Restore TTI configuration
    13.Restore PCL configuration
    14.Restore IPFIX configuration
    15.Restore Exact Match and Auto Learn configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigSet(4);

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_19B_E);

    /* clean interrupts */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt(GT_TRUE,0,0,0,0,0);

    /* Generate 5 packets traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled*/
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,5);

    /* check fail counter updated */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter(1,0);

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(4);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend(0);

    /* Generate 4 packets traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled */
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,5);

    /* check another fail counter updated */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter(1,0);

    /* invalidate 4 Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(4);

    /* read interrupts - expect to get events for:
       Flow ID Threshold Crossed, flowId range full, Flow ID Allocation Failed */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt(GT_FALSE,2,2,4,0,0);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCollision:
   Test functionality of Exact Match Action with multiple learning
   Test that we cannot learn more than the maximum EM entries defined
   and we get a counter update if we try to learn more then we can
   Configure Auto Learn
   Configure TTI and PCL to get match in Exact Match Auto Learned Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCollision)
{
    /*
    1. Set Exact Match configuration
    2. Set TTI UDB configuration
    3. Set PCL UDB configuration
    4. Set auto learn configuration
    5. Generate traffic - 5 packets - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action
    6. check we cannot learn more than the maximum defined
       and we get a counter update on the last packet send
    7. Invalidate 4 Exact Match Entry
    8. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded to port 1
    9. Generate traffic - 5 packets - expect Exact Match hit on an autoLearn Entry
       traffic forwarded to port 2 from auto learn reduced action
    10.check we cannot learn more than the maximum defined
       and we get a counter update on the last packet send
    11.Invalidate 4 Exact Match Entry
    12.Restore TTI configuration
    13.Restore PCL configuration
    14.Restore IPFIX configuration
    15.Restore Exact Match and Auto Learn configuration
    */

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

   /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Auto Learn configuration */
    prvTgfExactMatchAutoLearnMaxConfigSet();

    /* set ipfix configuration */
    prvTgfExactMatchAutoLearnIpfixtInit();

    /* Set Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet(CPSS_PACKET_CMD_FORWARD_E,PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E);

    /* clean interrupts */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt(GT_TRUE,0,0,0,0,0);

    /* Generate 5 packets traffic - expect Exact Match hit on an autoLearn Entry
      traffic forwarded to port 2 from auto learn reduced action - auto learn is enabled*/
    prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend(1,0xFFFFFFFF);

    /* check fail counter updated - flowID fail counter is 0 indexFailCounter is also 0
       since the fail index will be checked right after the traffic send so when getting
       to this call the counter was already cleared */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter(0,0);

    /* check collision parametrs*/
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckCollision();

    /* invalidate the Auto Learn Exact Match Entry - and disable auto learn */
    prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry(0xFFFFFFFF);

    /* read interrupts - expect to get events for:
       Auto Learn Collosion, Flow ID Allocation Failed */
    prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt(GT_FALSE,0,0,0XFFFFFFFF,0,1);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Restore Exact Match Expanded Action configuration */
    prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore();

    /* Restore IPFIX configuration */
    prvTgfExactMatchAutoLearnIpfixtRestore(0);

    /* Restore Auto Learn configuration */
    prvTgfExactMatchAutoLearnConfigRestore();

}

/*
 * Configuration of tgfExactMatch suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfExactMatch)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicDefaultActionPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicExpandedActionPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchDefaultActionUdb40EthOthreL2Pcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchExpandedActionUdb40EthOthreL2Pcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPath)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReduced)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonZeroKeyStart)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedNonFullKeyMask)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathRedirectToPortReducedUdb47CheckActivity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchTtiPclFullPathReducedUdbAllCheckActivity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicDefaultActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchBasicExpandedActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchReducedActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchReducedActionTtiFixedKey)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchReducedActionTtiFlowIdMaskCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTestForEmulator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReduced)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearning)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningMaxFlow)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers2)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointers3)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCollision)/* need to be the last in Auto Learn tests */

UTF_SUIT_END_TESTS_MAC(tgfExactMatch)
