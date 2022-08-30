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
* @file tgfCommonExactMatchManager.c
*
* @brief Enhanced UTs for CPSS Exact Match Manager
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfPclGen.h>
#include <common/tgfExactMatchManagerGen.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionTti.h>
#include <exactMatchManager/prvTgfExactMatchManagerBasicDefaultActionPcl.h>
#include <exactMatchManager/prvTgfExactMatchManagerBasicDefaultActionTti.h>
#include <exactMatch/prvTgfExactMatchBasicExpandedActionPcl.h>
#include <exactMatch/prvTgfExactMatchBasicExpandedActionTti.h>
#include <exactMatchManager/prvTgfExactMatchManagerBasicExpandedActionPcl.h>
#include <exactMatchManager/prvTgfExactMatchManagerBasicExpandedActionTti.h>
#include <exactMatch/prvTgfExactMatchUdbPcl.h>
#include <exactMatch/prvTgfExactMatchDefaultActionUdbPcl.h>
#include <exactMatchManager/prvTgfExactMatchManagerDefaultActionUdbPcl.h>
#include <exactMatch/prvTgfExactMatchExpandedActionUdbPcl.h>
#include <exactMatchManager/prvTgfExactMatchManagerExpandedActionUdbPcl.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>
#include <exactMatchManager/prvTgfExactMatchManagerTtiPclFullPath.h>
#include <exactMatch/prvTgfExactMatchReducedActionTti.h>
#include <exactMatchManager/prvTgfExactMatchManagerReducedActionTti.h>
#include <exactMatchManager/prvTgfExactMatchManagerHaFullCapacity.h>
#include <exactMatchManager/prvTgfExactMatchManagerScanning.h>

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchManagerBasicDefaultActionPcl

    configure VLAN, FDB entries
    configure PCL rules - to trap
    send traffic
    verify traffic is trapped

    Invalidate PCL Rule
    Set Exact Match Default Action configuration
    verify traffic is drop
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerBasicDefaultActionPcl)
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

    GT_U32           exactMatchManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerBasicDefaultActionPclManagerCreate(exactMatchManagerId);

    /* Set PCl configuration */
    prvTgfExactMatchBasicDefaultActionPclGenericConfig();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchBasicDefaultActionPclTrafficGenerate(GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchBasicDefaultActionPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Default Action */
    prvTgfExactMatchBasicDefaultActionPclTrafficGenerate(GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchBasicDefaultActionPclGenericRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerBasicDefaultActionPclManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchManagerBasicExpandedActionPcl

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
UTF_TEST_CASE_MAC(tgfExactMatchManagerBasicExpandedActionPcl)
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

    GT_U32           exactMatchManagerId = 21;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerBasicExpandedActionPclManagerCreate(exactMatchManagerId);

    /* Set PCl configuration */
    prvTgfExactMatchBasicExpandedActionPclGenericConfig();

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerBasicExpandedActionPclConfigSet(exactMatchManagerId);

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

    /* Delete Exact Match Entry from Manager */
    prvTgfExactMatcManagerhBasicExpandedActionPclDeleteEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchBasicExpandedActionPclTrafficGenerate(GT_FALSE,GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchBasicExpandedActionPclGenericRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerBasicExpandedActionPclManagerDelete(exactMatchManagerId);

}

/*----------------------------------------------------------------------------*/
/*
    tgfExactMatchManagerDefaultActionUdb40EthOthreL2Pcl

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
UTF_TEST_CASE_MAC(tgfExactMatchManagerDefaultActionUdb40EthOthreL2Pcl)
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

    GT_U32           exactMatchManagerId = 22;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    /* Create manager with device */
    prvTgfExactMatchDefaultActionUdbPclManagerCreate(exactMatchManagerId);

    /* Set PCl UDB configuration
       Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet */
    prvTgfExactMatchUdb40EthOthreL2PclGenericConfig();

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE,GT_FALSE);

    /* invalidate PCL Rule */
    prvTgfExactMatchDefaultActionUdbPclInvalidateRule();

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Default Action */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchUdb40EthOthreL2PclGenericRestore();

    /* Delete manager and device */
    prvTgfExactMatchDefaultActionUdbPclManagerDelete(exactMatchManagerId);
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
UTF_TEST_CASE_MAC(tgfExactMatchManagerExpandedActionUdb40EthOthreL2Pcl)
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

    GT_U32           exactMatchManagerId = 23;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfExactMatchCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerExpandedActionUdbPclManagerCreate(exactMatchManagerId);

    /* Set PCl UDB configuration
       Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet */
    prvTgfExactMatchUdb40EthOthreL2PclGenericConfig();

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerExpandedActionUdbPclConfigSet(exactMatchManagerId);

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

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerExpandedActionUdbPclInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore PCL generic configuration */
    prvTgfExactMatchUdb40EthOthreL2PclGenericRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerExpandedActionUdbPclManagerDelete(exactMatchManagerId);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchManagerTtiPclFullPath:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry

*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPath)
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

    GT_U32           exactMatchManagerId = 24;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_TRUE,GT_FALSE);

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchManagerTtiPclFullPathRedirectToPort:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPort)
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

    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchManagerTtiPclFullPathRedirectToPortReduced:
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReduced)
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

    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathReducedManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedNonZeroKeyStart::
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   Key Start parameter different from 0
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedNonZeroKeyStart)
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

    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathReducedNonZeroKeyStartManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedNonFullKeyMask::
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
   Key Mask parameter different from 0xFF
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedNonFullKeyMask)
{
    /*
    1. Set Exact Match configuration
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

    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathReducedNonFullKeyMaskManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedUdb47::
   Test functionality of Exact Match Action
   Configure TTI and PCL to get match in Exact Match Entry
   Expect traffic to be redirect to port from Exact Match,
   port is taken from reduced action
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedUdb47)
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

    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration with keySize=47B */
    prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE,GT_TRUE);

    /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Restore TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore();

    /* Restore PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerTtiPclFullPathManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/*
   tgfExactMatchManagerBasicDefaultActionTti :
   Test functionality of Exact Match default action ,
   base on tgfTunnelTermLlcNonSnapType
   Exact Match default action gets active incase no hit in Tcam or Exact Match lookup
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerBasicDefaultActionTti)
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

    GT_U32           exactMatchManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
    prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerBasicDefaultActionTtiManagerCreate(exactMatchManagerId,GT_FALSE);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet();

    /* Generate traffic - expect to be forwarded */
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* disable TTI rule */
    prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet(GT_FALSE);

    /* Generate traffic -- check no traffic and no cpuCode */
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_FALSE,GT_FALSE);

    /* change Default Action for Trap
       delete the manager and create it again with trap parameters*/
    prvTgfExactMatchManagerBasicDefaultActionTtiManagerDelete(exactMatchManagerId);
    prvTgfExactMatchManagerBasicDefaultActionTtiManagerCreate(exactMatchManagerId,GT_TRUE);

    /* Generate traffic -- check no traffic and cpuCode 502*/
    prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerBasicDefaultActionTtiManagerDelete(exactMatchManagerId);
}


/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchManagerBasicExpandedActionTti :
   Test functionality of Exact Match Expanded Action ,

   Exact Match Expanded action gets active incase no hit in Tcam or
   proity bit in TTI action is set

   Test on PRV_TGF_TTI_RULE_UDB_30_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
   first 4 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerBasicExpandedActionTti)
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

    GT_U32           exactMatchManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerBasicExpandedActionTtiManagerCreate(exactMatchManagerId);

    /* Set TTI configuration */
    prvTgfExactMatchBasicExpandedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchManagerBasicExpandedActionTtiConfigSet(exactMatchManagerId);

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

   /* Delete Exact Match Entry from Manager */
    prvTgfExactMatcManagerhBasicExpandedActionTtiDeleteEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerBasicExpandedActionTtiManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/*
   AUTODOC: tgfExactMatchManagerReducedActionTti :
   Test functionality of Exact Match Reduced Action ,

    Test on PRV_TGF_TTI_RULE_UDB_10_E
            PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E packet.
    first 7 bytes UDB from PRV_TGF_TTI_OFFSET_L3_MINUS_2_E
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerReducedActionTti)
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
    GT_U32           exactMatchManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Create manager with device */
    prvTgfExactMatchManagerReducedActionTtiManagerCreate(exactMatchManagerId,GT_FALSE);

    /* Set TTI configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigSet();

    /* Set Exact Match Expanded Action TTI configuration */
    prvTgfExactMatchManagerReducedActionTtiConfigSet(exactMatchManagerId,GT_FALSE);

    /* Generate traffic - check match in TCAM */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_TRUE,GT_FALSE,0,GT_FALSE);

    /* set Exact Match priority over TCAM*/
    prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig(GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 503
    */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE,GT_TRUE,503,GT_FALSE);

    /*change Reduced Action to Trap with CPUcode 505
      and Expanded Action to forward
    */
    prvTgfExactMatchManagerBasicExpandedActionTtiManagerDelete(exactMatchManagerId);
    prvTgfExactMatchManagerReducedActionTtiManagerCreate(exactMatchManagerId,GT_TRUE);
    prvTgfExactMatchManagerReducedActionTtiConfigSet(exactMatchManagerId,GT_TRUE);

    /* Generate traffic - check no traffic - CPUcode 505
    */
    prvTgfExactMatchReducedActionTtiTrafficGenerate(GT_FALSE, GT_TRUE,505,GT_FALSE);

    /* Restore TTI generic configuration */
    prvTgfExactMatchReducedActionTtiGenericConfigRestore();

    /* Delete manager and device */
    prvTgfExactMatchManagerBasicExpandedActionTtiManagerDelete(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Exact Match Manager simple HA case - single entry: based on test
    tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedUdb47
    set generic configuration and create Exact Match Manager.
    send traffic and verify expected packet and counters
    Set HA flag
    Delete all entries and Manager
    Configure again the generic configuration and the manager create
    call prvCpssDxChExactMatchSyncSwHwForHa()
    Run validity
    send traffic and verify expected packet and counters;
    Set HA completed flag
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerTtiPclFullPathHaCheckSingleEntry)
{
    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry(exactMatchManagerId,GT_TRUE);
    prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry(exactMatchManagerId);
    prvTgfExactMatchManagerTtiPclFullPathHaSystemRecoveryStateSet();
    prvTgfExactMatchManagerTtiPclFullPathHaDelBasicConfigSingleEntry(exactMatchManagerId);
    prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry(exactMatchManagerId,GT_FALSE);
    prvTgfExactMatchManagerTtiPclFullPathHaSyncSwHwSingleEntry();
    prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry(exactMatchManagerId);
    prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Exact Match Manager full capacity HA case
    set generic configuration and create Exact Match Manager.
    Add max number of exact natch entries with rehashing=true
    send traffic and verify expected packet and counters
    Set HA flag
    Delete all entries and Manager
    Configure again the generic configuration and the manager create
    replay all exact match entries
    call prvCpssDxChExactMatchSyncSwHwForHa()
    Run validity
    send traffic and verify expected packet and counters;
    Set HA completed flag
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerHaFullCapacity)
{
    GT_U32           exactMatchManagerId = 25;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfExactMatchManagerHaFullCapacityBasicConfigAdd(exactMatchManagerId,GT_TRUE);
    prvTgfExactMatchManagerHaFullCapacityValidityCheckAndTrafficSend(exactMatchManagerId);
    prvTgfExactMatchManagerHaFullCapacitySystemRecoveryStateSet();
    prvTgfExactMatchManagerHaFullCapacityDelBasicConfig(exactMatchManagerId);
    prvTgfExactMatchManagerHaFullCapacityBasicConfigAdd(exactMatchManagerId,GT_FALSE);
    prvTgfExactMatchManagerHaFullCapacitySyncSwHw();
    prvTgfExactMatchManagerHaFullCapacityValidityCheckAndTrafficSend(exactMatchManagerId);
    prvTgfExactMatchManagerHaFullCapacityBasicConfigRestore(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Exact Match Manager full capacity scanDelete case,
    the part of the entry adding is based on HA full capacity test
    set generic configuration and create Exact Match Manager.
    Add max number of exact match entries with rehashing=true
    send traffic and verify expected packet and counters
    Delete only entries that meet the creteria of scanDelete
    Run validity
    send traffic and verify expected packet and counters - no traffic match
    delete all entries added in the beginning - expect same number of added entries
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerDeleteScanFullCapacity)
{
    GT_U32           exactMatchManagerId = 31;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfExactMatchManagerScanningBasicConfigAdd(exactMatchManagerId,GT_TRUE,GT_TRUE);
    prvTgfExactMatchManagerScanningValidityCheckAndTrafficSend(exactMatchManagerId);
    prvTgfExactMatchManagerScanningDeleteFilterConfig(exactMatchManagerId);
    prvTgfExactMatchManagerScanningValidityCheckAndTrafficSend(exactMatchManagerId);
    prvTgfExactMatchManagerScanningDelBasicConfig(exactMatchManagerId);
    prvTgfExactMatchManagerScanningBasicConfigRestore(exactMatchManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Exact Match Manager full capacity scanAge and scanAgeout case,
    the part of the entry adding is based on HA full capacity test
    set generic configuration and create Exact Match Manager.
    Add max number of exact match entries with rehashing=true
    send traffic and verify expected packet and counters
    Age/AgeDelete only entries that meet the creteria of scanAging
    Run validity
    send traffic and verify expected packet and counters - no traffic match
    delete all entries added in the beginning - expect same number of added entries
*/
UTF_TEST_CASE_MAC(tgfExactMatchManagerAgingScanFullCapacity)
{
    GT_U32           exactMatchManagerId = 31;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfExactMatchManagerScanningBasicConfigAdd(exactMatchManagerId,GT_TRUE,GT_FALSE);
    prvTgfExactMatchManagerScanningValidityCheckAndTrafficSend(exactMatchManagerId);
    prvTgfExactMatchManagerScanningAgeoutFilterConfig(exactMatchManagerId);
    prvTgfExactMatchManagerScanningValidityCheckAndTrafficSend(exactMatchManagerId);
    prvTgfExactMatchManagerScanningBasicConfigRestore(exactMatchManagerId);
}
/*
 * Configuration of tgfExactMatchManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfExactMatchManager)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerBasicDefaultActionPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerBasicExpandedActionPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerDefaultActionUdb40EthOthreL2Pcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerExpandedActionUdb40EthOthreL2Pcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPath)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReduced)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedNonZeroKeyStart)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedNonFullKeyMask)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPathRedirectToPortReducedUdb47)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerBasicDefaultActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerBasicExpandedActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerReducedActionTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerTtiPclFullPathHaCheckSingleEntry)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerHaFullCapacity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerDeleteScanFullCapacity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfExactMatchManagerAgingScanFullCapacity)
UTF_SUIT_END_TESTS_MAC(tgfExactMatchManager)



