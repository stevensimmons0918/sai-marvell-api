/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tcamUtils.c
*
* DESCRIPTION:
*  utility functions for working with TCAM
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "tcamUtils.h"

#include <gtOs/gtOsMem.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*****************************************************/
/**************** Static Varaibles       ********************/
/*****************************************************/

static GT_U32 ttiCncFirstCounterIndx = 10240;


/**
* @internal csRefInfraFindTCAMFirstRule function
* @endinternal
*
* @brief   find first rules for TTI, PCL, EPCL  -used for setting TCAM rule at correct index.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           -                    device number
* @param[out] groupFirstRuleIndexIPCL      -first IPCL rule index (1-st parallel lookup).
* @param[out] groupFirstRuleIndexEPCL      -first EPCL rule index
* @param[out] groupFirstRuleIndexTTI        -first TTI rule index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS csRefInfraFindTCAMFirstRule
(
    IN GT_U8    devNum,
    OUT GT_U32 *groupFirstRuleIndexIPCL,
    OUT GT_U32 *groupFirstRuleIndexEPCL,
    OUT GT_U32 *groupFirstRuleIndexTTI
)
{
    GT_U32                              floorIndex, numOfFloors, blocksPerFloor, rulesPerBlock;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC       floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32                              i, tcamGroupIPCL, blockIndexIPCL, floorIndexIPCL;
    GT_U32                              tcamGroupEPCL, blockIndexEPCL, floorIndexEPCL;
    GT_U32                              tcamGroupTTI, blockIndexTTI, floorIndexTTI;
    GT_BOOL                             enable, firstRuleFound;
    GT_STATUS rc;

/*************************************** Stage 1: Get TCAM info, so that we know what index assign to rule ********************/
    numOfFloors    = CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(0);
    blocksPerFloor = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;
    rulesPerBlock  = (12/blocksPerFloor)*256;


    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        rc = cpssDxChTcamPortGroupClientGroupGet(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_TCAM_TTI_E, &tcamGroupTTI, &enable);
        if( GT_OK != rc)
           return rc;
        if(enable == GT_TRUE)
        {
            cpssOsPrintf("%d floors, %d blocks per floor\nwork with group %d for TTI\n", numOfFloors, blocksPerFloor, tcamGroupTTI);
        }
        else
        {
            cpssOsPrintf("need to assign clientGroup to TTI\n");
            return GT_NOT_FOUND;
        }

        rc = cpssDxChTcamPortGroupClientGroupGet(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_TCAM_IPCL_0_E, &tcamGroupIPCL, &enable);
        if( GT_OK != rc)
           return rc;
        if(enable == GT_TRUE)
        {
            cpssOsPrintf("work with group %d for IPCL\n", tcamGroupIPCL);
        }
        else
        {
            cpssOsPrintf("need to assign clientGroup to PCL0\n");
            return GT_NOT_FOUND;
        }

        rc = cpssDxChTcamPortGroupClientGroupGet(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_TCAM_EPCL_E, &tcamGroupEPCL, &enable);
        if( GT_OK != rc)
           return rc;
        if(enable == GT_TRUE)
        {
            cpssOsPrintf("work with group %d for EPCL\n", tcamGroupEPCL);
        }
        else
        {
            cpssOsPrintf("need to assign clientGroup to EPCL\n");
            return GT_NOT_FOUND;
        }
    }
    else
    {
        tcamGroupEPCL = 4;
        tcamGroupIPCL = 1;
        tcamGroupTTI  = 0;
    }

    /* Find the first TTI rule index we can use */
    firstRuleFound = GT_FALSE;

    for(floorIndex = 0; floorIndex < numOfFloors; floorIndex++)
    {
        rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);
        if (rc != GT_OK)
            return rc;
        for(i=0;i<blocksPerFloor;i++)
        {
            if ((floorInfoArr[i].group  == tcamGroupTTI) && (floorInfoArr[i].hitNum == 0))
            {
                floorIndexTTI = floorIndex;
                blockIndexTTI = i;
                firstRuleFound = GT_TRUE;
                *groupFirstRuleIndexTTI = (floorIndexTTI*blocksPerFloor + blockIndexTTI)*rulesPerBlock;
                break;
            }
        }
        if (firstRuleFound == GT_TRUE)
            break;
    }
    if (firstRuleFound == GT_TRUE)
    {
        cpssOsPrintf("first TTI rule is %d\n", *groupFirstRuleIndexTTI);
    }
    else
    {
        cpssOsPrintf("Could not find an index for the first TTI rule\n");
        return GT_NOT_FOUND;
    }

/* Find the first PCL rule index we can use */
    firstRuleFound = GT_FALSE;

    for(floorIndex = 0; floorIndex < numOfFloors; floorIndex++)
    {
        rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);
        if (rc != GT_OK)
            return rc;
        for(i=0;i<blocksPerFloor;i++)
        {
            if ((floorInfoArr[i].group  == tcamGroupIPCL) && (floorInfoArr[i].hitNum == 0))
            {
                floorIndexIPCL = floorIndex;
                blockIndexIPCL = i;
                firstRuleFound = GT_TRUE;
                *groupFirstRuleIndexIPCL = (floorIndexIPCL*blocksPerFloor + blockIndexIPCL)*rulesPerBlock;
                break;
            }
        }
        if (firstRuleFound == GT_TRUE)
            break;
    }
    if (firstRuleFound == GT_TRUE)
    {
        cpssOsPrintf("first IPCL rule is %d\n", *groupFirstRuleIndexIPCL);
    }
    else
    {
        cpssOsPrintf("Could not find an index for first IPCL rule\n");
        return GT_NOT_FOUND;
    }

    firstRuleFound = GT_FALSE;
    for(floorIndex = 0; floorIndex < numOfFloors; floorIndex++)
    {
        rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(devNum, floorIndex, floorInfoArr);
        if (rc != GT_OK)
            return rc;
        for(i=0;i<blocksPerFloor;i++)
        {
            if ((floorInfoArr[i].group  == tcamGroupEPCL) && (floorInfoArr[i].hitNum == 0))
            {
                floorIndexEPCL = floorIndex;
                blockIndexEPCL = i;
                firstRuleFound = GT_TRUE;
                *groupFirstRuleIndexEPCL = (floorIndexEPCL*blocksPerFloor + blockIndexEPCL)*rulesPerBlock;
                if(*groupFirstRuleIndexEPCL == *groupFirstRuleIndexIPCL)
                    *groupFirstRuleIndexEPCL += 1;
                break;
            }
        }
        if (firstRuleFound == GT_TRUE)
            break;

    }

    if (firstRuleFound == GT_TRUE)
    {
        cpssOsPrintf("first EPCL rule is %d\n", *groupFirstRuleIndexEPCL);
    }
    else
    {
        cpssOsPrintf("Could not find an index for the first EPCL rule\n");
        return GT_NOT_FOUND; 
    }

    return GT_OK;
}


/**
* @internal csRefInfraTCAMCountersGet function
* @endinternal
*
* @brief   printout non-zero  CNC TCAM counters, csRefInfraSetTCAMCounters must be called before
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum          -     device number
* @param[in] ipclBlocks           -  number of CNC blocks assigned to IPCL (lookup 0)
* @param[in] epclBlocks          -  number of CNC blocks assigned to EPCL
* @param[in] ttiBlocks             -  number of CNC blocks assigned to TTI
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - otherwise
*
* @note
*      should be called after csRefInfraSetTCAMCounters, and with the same parameters for block numbers
*      use like csRefInfraGetTCAMCounters 0,8,2,6
*/
GT_STATUS csRefInfraTCAMCountersGet
(
    IN GT_U8 devNum,
    IN GT_U8 ipclBlocks,
    IN GT_U8 epclBlocks,
    IN GT_U8 ttiBlocks
)
{
    CPSS_DXCH_CNC_COUNTER_STC counter;
    GT_U32 totalTTICounter = 0;
    GT_U32 totalPCLCounter = 0;
    GT_U32 totalEPCLCounter = 0;
    GT_STATUS rc;
    GT_U16 i;
    GT_U16  block;
    GT_U32  numOfCncBlock,NumOfCncEntriesInBlock;

    cpssOsPrintf("==================> csRefInfraGetTCAMCounters is called <============\n");
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(0) == GT_TRUE)
    {
        for (block=0; block < ipclBlocks ; block++)
        {
            for(i=0; i< 1024;i++)
            {
                rc = cpssDxChCncCounterGet(0, block, i, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if( GT_OK != rc)
                   return rc;

                if(counter.packetCount.l[0] != 0)
                {
                    totalPCLCounter = counter.packetCount.l[0] + totalPCLCounter;
                    cpssOsPrintf("IPCL TCAM [blk:rule] [%d:%d] hit %d times\n",block, i,counter.packetCount.l[0] );
                }

            }
        }

        for (block=ipclBlocks; block < ipclBlocks + epclBlocks ; block++)
        {
            for(i=0; i< 1024;i++)
            {
                rc = cpssDxChCncCounterGet(0, block, i, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if( GT_OK != rc)
                   return rc;

                if(counter.packetCount.l[0] != 0)
                {
                    totalEPCLCounter = counter.packetCount.l[0] + totalEPCLCounter;
                    cpssOsPrintf("EPCL rule %d hit %d times\n", i + block*1024, counter.packetCount.l[0] );
                }

            }
        }

        for (block = ipclBlocks + epclBlocks; block < ipclBlocks + epclBlocks + ttiBlocks ; block++)
        {
            for(i=0; i< 1024;i++)
            {
                rc = cpssDxChCncCounterGet(0, block, i, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if( GT_OK != rc)
                   return rc;

                if(counter.packetCount.l[0] != 0)
                    {
                    totalTTICounter = counter.packetCount.l[0] + totalTTICounter;
                    cpssOsPrintf("TTI rule counter %d hit %d times\n", i + block*1024 - ttiCncFirstCounterIndx, counter.packetCount.l[0] );
                }

            }
        }

    }
    else
    {

        numOfCncBlock = PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks;
        NumOfCncEntriesInBlock = PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries;

        for (block=0; block < numOfCncBlock ; block++)
        {
            for(i=0; i< NumOfCncEntriesInBlock;i++)
            {
                rc = cpssDxChCncCounterGet(0, block, i, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if(( GT_OK != rc)& (i==0))
                    cpssOsPrintf("cpssDxChCncCounterGet failed on block %d index %d rc=%d\n",block,i,rc);

                if(counter.packetCount.l[0] != 0)
                {
                    totalEPCLCounter = counter.packetCount.l[0] + totalEPCLCounter;
                    cpssOsPrintf("EPCL rule %d hit %d times\n", i + block*2048,counter.packetCount.l[0] );
                }

            }
        }
    }
    cpssOsPrintf("%d packets passed through PCL\n" , totalPCLCounter );
    cpssOsPrintf("%d packets passed through EPCL\n", totalEPCLCounter);
    cpssOsPrintf("%d packets passed through TTI\n" , totalTTICounter );
    return GT_OK;
}


/**
* @internal csRefInfraTCAMCountersSet function
* @endinternal
*
* @brief   set CNC counters used for debugginh the configuration - TCAM rule match indication
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           -    device number
* @param[in] ipclBlocks           -  number of CNC blocks assigned to IPCL (lookup 0)
* @param[in] epclBlocks          -  number of CNC blocks assigned to EPCL
* @param[in] ttiBlocks             -  number of CNC blocks assigned to TTI
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
* counters are set for IPCL lookup 0, EPCL and TTI
*/
GT_STATUS csRefInfraTCAMCountersSet
(
    IN GT_U8 devNum,
    IN GT_U8 ipclBlocks,
    IN GT_U8 epclBlocks,
    IN GT_U8 ttiBlocks
)
{
    GT_STATUS   rc = GT_OK;
    GT_U64      indexRangesBmp;
    GT_U32      numOfCncBlock;
    GT_32       i;

    cpssOsPrintf("==================> setTCAMCounters is called <============\n");

    ttiCncFirstCounterIndx = 1024*(ipclBlocks + epclBlocks);

    rc = cpssDxChCncCountingEnableSet(devNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E, GT_TRUE);
    if( GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChCncCountingEnableSet failed for TTI rc = %d\n",rc);
        return rc;
    }
    rc = cpssDxChCncCountingEnableSet(devNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E, GT_TRUE);
    if( GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChCncCountingEnableSet failed for PCL rc = %d\n",rc);
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* each counter block contains 1024 counters; there are 32 blocks */
        for (i=0; i < ipclBlocks ; i++)
        {
            rc = cpssDxChCncBlockClientEnableSet(devNum, i, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E, GT_TRUE);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientEnableSet[0] failed i=%d rc = %d\n",i,rc);
                return rc;
            }
            indexRangesBmp.l[0] = 1 << i;
            indexRangesBmp.l[1] = 0;
            rc = cpssDxChCncBlockClientRangesSet(devNum,i,
                                                 CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,
                                                 &indexRangesBmp);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientRangesSet[0] failed i=%d rc = %d\n",i,rc);
                return rc;
            }
        }
        for (i=ipclBlocks; i < ipclBlocks+epclBlocks ; i++)
        {
            rc = cpssDxChCncBlockClientEnableSet(devNum, i, CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E, GT_TRUE);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientEnableSet[1] failed i=%d rc = %d\n",i,rc);
                return rc;
            }
            indexRangesBmp.l[0] = 1 << i;
            indexRangesBmp.l[1] = 0;
            rc = cpssDxChCncBlockClientRangesSet(devNum,i,
                                                 CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E,
                                                 &indexRangesBmp);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientRangesSet[1] failed i=%d rc = %d\n",i,rc);
                return rc;
            }
        }

        for (i=ipclBlocks+epclBlocks; i < ipclBlocks+epclBlocks+ttiBlocks ; i++)
        {
            rc = cpssDxChCncBlockClientEnableSet(devNum, i, CPSS_DXCH_CNC_CLIENT_TTI_E, GT_TRUE);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientEnableSet[%d] failed rc = %d\n",i,rc);
                return rc;
            }
            indexRangesBmp.l[0] = 1 << i;
            indexRangesBmp.l[1] = 0;
            rc = cpssDxChCncBlockClientRangesSet(devNum,i,
                                                 CPSS_DXCH_CNC_CLIENT_TTI_E,
                                                 &indexRangesBmp);
            if( GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChCncBlockClientRangesSet[%d] failed rc = %d\n",i,rc);
                return rc;
            }
        }
    }
    else
    {

        rc = cpssDxChCncClientByteCountModeSet(devNum,CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E,CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E);
        if( GT_OK != rc)
            cpssOsPrintf("cpssDxChCncClientByteCountModeSet failed rc = %d\n",rc);

        numOfCncBlock = PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks;

        for(i = 0;i < (GT_32)numOfCncBlock;i++)
        {
            rc = cpssDxChCncBlockClientEnableSet(0,
                                             i,
                                             CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E,
                                             GT_TRUE);
            if( GT_OK != rc)
                cpssOsPrintf("cpssDxChCncBlockClientEnableSet failed block %d rc = %d\n",i,rc);
            indexRangesBmp.l[0] = 1<<i;
            indexRangesBmp.l[1] = 0;
            rc = cpssDxChCncBlockClientRangesSet(0,
                                             i,
                                             CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E,
                                             &indexRangesBmp);
            if( GT_OK != rc)
                cpssOsPrintf("cpssDxChCncBlockClientRangesSet failed block %d rc = %d\n",i,rc);
            else
                cpssOsPrintf("block %d Range %d \n",i,indexRangesBmp.l[0]);

        }

    }
    return rc;
}




