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
* @file cpssDxChTcamUT.c
*
* @brief Unit tests for cpssDxChTcam.
*
* @version   10
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
/*#include <cpss/dxCh/dxChxGen/tti/cpssDxChPcl.h>*/


#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* defines */

/* valid indexes for testing */
/* for SIP5 and SIP6 devices 10-byte rules can begin from any index, for SIP6_10 - from odd index only */
#define TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX        CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS + CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS +1
#define TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX        CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS + CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS +2
#define TCAM_CORRECT_RULE_SIZE_50_RULE_INDEX        CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS + CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS +2
#define TCAM_CORRECT_RULE_SIZE_80_RULE_INDEX        CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS + CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS +8

/* invalid indexes for testing */
#define TCAM_INVALID_POS_RULE_INDEX                CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS*CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS*CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS

/* valid values for tcamPattern and tcamMaskPtr */
#define TCAM_PATTERN    0xFEAB
#define TCAM_MASK       0xFCEB

/* out of range TCAM start rule index range - (APPLICABLE VALUES: 0,1.5K,3K,..34.5K)*/
#define TCAM_OUT_OF_RANGE_START_RULE_INDEX                CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_HALF_FLOOR_CNS+20

/* invalid TCAM start rule index range - (APPLICABLE VALUES: 0,1.5K,3K,..34.5K)*/
#define TCAM_INVALID_START_RULE_INDEX                        ((CPSS_DXCH_TCAM_MAX_NUM_RULES_CNS - CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS)*2)+5

/*out of range num of rule entries*/
#define TCAM_OUT_OF_RANGE_NUM_OF_RULE_ENTRIES                        CPSS_DXCH_TCAM_MAX_NUM_RULES_CNS+5

/*valid num of rule entries*/
#define TCAM_VALID_NUM_OF_RULE_ENTRIES                         CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_HALF_FLOOR_CNS

/*invalid num of rule entries*/
#define TCAM_INVALID_NUM_OF_RULE_ENTRIES                         (CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_HALF_FLOOR_CNS*2)+5

/*out of range TCAM power up start rule index (APPLICABLE VALUES: 0,256,512,..(36K-256)) */
#define TCAM_OUT_OF_RANGE_POWER_UP_START_RULE_INDEX                (CPSS_DXCH_TCAM_MAX_NUM_RULES_CNS-CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS)+5

/* invalid TCAM power up start rule index range - (APPLICABLE VALUES: 0,256,512,..(36K-256))*/
#define TCAM_INVALID_POWER_UP_START_RULE_INDEX                        (CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS*2)+5

/*valid power up num of rule entries (APPLICABLE VALUES: 256,512,768,..36K)*/
#define TCAM_VALID_POWER_UP_NUM_OF_RULE_ENTRIES                        CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS

/*out of range power up num of rule entries (APPLICABLE VALUES: 256,512,768,..36K)*/
#define TCAM_OUT_OF_RANGE_POWER_UP_NUM_OF_RULE_ENTRIES                        CPSS_DXCH_TCAM_MAX_NUM_RULES_CNS+5

/*invalid power up num of rule entries (APPLICABLE VALUES: 256,512,768,..36K)*/
#define TCAM_INVALID_POWER_UP_NUM_OF_RULE_ENTRIES                         (CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS*2)+5



/* defines */

/* fill bmp of ports */
#define PORTS_BMP_FILL_MAC(field)       \
        field = PRV_CPSS_PP_MAC(currentTesteddev)->existingPorts;        \
        if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(currentTesteddev))   \
        {                                                                   \
            /* add CPU port */                                              \
            CPSS_PORTS_BMP_PORT_SET_MAC(&field,CPSS_CPU_PORT_NUM_CNS);      \
        }                                                                   \
        if(fullFill == GT_FALSE)                                            \
        {                                                                   \
            CPSS_PORTS_BMP_BITWISE_AND_MAC(&field,                          \
                                           &field,                          \
                                           &testedPortListBmpMask);         \
        }

/* Invalid enum */
#define PCL_INVALID_ENUM_CNS    0x5AAAAAA5

/* get port belogs to port group */
#define UTF_GET_PORT_FOR_PORT_GROUP_MAC(_portGroup) ((_portGroup) << 4)

/* there are 3K entries in floor
   NOTE: currently this number is not 'per device'
*/
#define NUM_RULES_IN_FLOOR_MAC(_devNum)     CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS

/* macro to roundup a number that is divided so :
    for example :
    3 / 2 is round up to 2 and not round down to 1
*/
#define ROUNDUP_DIV_MAC(_number , _divider)             \
    (((_number) + ((_divider)-1)) / (_divider))

#define CPSS_DXCH_TCAM_ACTIVE_FLOOR_NUM_MAC(_devNum)                          \
    ((PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policyTcamRaws)/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS)

/* get the number of floors needed for the number of rules in the tcam */
#define CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(_devNum)                          \
    ROUNDUP_DIV_MAC(                                                        \
        PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm,     \
        NUM_RULES_IN_FLOOR_MAC(_devNum))

/* maximal size of rule in words - 80 Byte / 4 */
#define UTF_TCAM_MAX_RULE_SIZE_CNS 20

/**
* @internal prvCpssDxChTcamInvalidateRulesPerClient function
* @endinternal
*
*/

extern GT_STATUS prvCpssDxChTcamInvalidateRulesPerClient
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT   tcamClient,
    IN  GT_BOOL                     clearAll
);

/*******************************************************************************
GT_STATUS cpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupRuleWrite)
{
/*
    ITERATE_DEVICES
    1.1. Check standard rule for all active devices with legal parameters.
    Call with portGroupsBmp

*/

    GT_STATUS                           st = GT_OK;
    GT_U32                              portGroupId = 0;
    GT_U8                               dev;
    GT_PORT_GROUPS_BMP                  portGroupsBmp = 1;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize    = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
    GT_U32                              ruleIndex = 0;
    GT_U32                              mask[UTF_TCAM_MAX_RULE_SIZE_CNS] = {TCAM_MASK, 0};
    GT_U32                              pattern[UTF_TCAM_MAX_RULE_SIZE_CNS] = {TCAM_PATTERN, 0};
    GT_BOOL                             valid       = GT_FALSE;

    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 Go over all active port groups. */

        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            /*
                1.1.1 Call with  ruleSize [CPSS_DXCH_TCAM_RULE_SIZE_10_B_E],
                                ruleIndex [TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */
            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);


             /*
                1.1.2 Call with  ruleSize [CPSS_DXCH_TCAM_RULE_SIZE_30_B_E],
                                ruleIndex [TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */
            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

             /*
                1.1.3 Call with  ruleSize [CPSS_DXCH_TCAM_RULE_SIZE_50_B_E],
                                ruleIndex [TCAM_CORRECT_RULE_SIZE_50_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */
            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_50_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

             /*
                1.1.4 Call with  ruleSize [CPSS_DXCH_TCAM_RULE_SIZE_80_B_E],
                                ruleIndex [TCAM_CORRECT_RULE_SIZE_80_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */
            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_80_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

                         /*
                1.1.5 check out of range ruleIndex.
                Call with ruleIndex [wrong value], other parameters same as in 1.2.
                Expected: NON GT_OK.
            */

            ruleIndex = TCAM_INVALID_POS_RULE_INDEX;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);



                        /*
                1.1.6 check out of range ruleIndex.
                Call with ruleIndex [out of range value], other parameters same as in 1.2.
                Expected: NON GT_OK.
            */

            ruleIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelTerm;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);

            /*
                1.1.7 Call with  ruleSize [CPSS_DXCH_TCAM_RULE_SIZE_50_B_E],
                                ruleIndex [TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Try to set status of created rule with wrong ruleIndex, rule cannot start in a given index.
                Expected: NOT GT_OK.
            */
            /* call with ruleSize = CPSS_PCL_RULE_SIZE_EXT_E */
            ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ruleIndex = %d", dev, ruleIndex);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ruleIndex = %d", dev, ruleIndex);
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)


        /* 1.2  For not-active port groups check that function returns GT_BAD_PARAM. */

        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

                ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
        ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                        ,valid,ruleSize, pattern, mask);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
    ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
    valid     = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

        /* go over all non active devices */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
                st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                    ,valid,ruleSize, pattern, mask);

                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

    /* 3. Call with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                        ,valid,ruleSize, pattern, mask);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*******************************************************************************
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupRuleValidStatusSet)
{
/*
    ITERATE_DEVICES
    1.1. Check standard rule for all active devices with legal parameters.
    Call with portGroupsBmp

*/
    GT_STATUS                           st = GT_OK;
    GT_U32                              portGroupId = 0;
    GT_U8                               dev;
    GT_PORT_GROUPS_BMP                  portGroupsBmp = 1;
    GT_U32                              ruleIndex = 0;
    GT_BOOL                             valid       = GT_FALSE;
    GT_U32                              mask[UTF_TCAM_MAX_RULE_SIZE_CNS] = {TCAM_MASK, 0};
    GT_U32                              pattern[UTF_TCAM_MAX_RULE_SIZE_CNS] = {TCAM_PATTERN, 0};
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);



    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 Go over all active port groups. */

        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            /*
                1.1.1 Call with ruleIndex [TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */
            ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                            ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  ruleIndex, valid);

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

             /*
                1.1.2 Call with ruleIndex [TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */

            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                            ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

             /*
                1.1.3 Call with ruleIndex [TCAM_CORRECT_RULE_SIZE_50_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */

            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_50_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                            ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

                        /*
                1.1.4 Call with ruleIndex [TCAM_CORRECT_RULE_SIZE_80_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */

            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_80_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                            ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);

            /* call with valid = GT_FALSE */
            valid = GT_FALSE;

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,ruleIndex, valid);

                        /*
                1.1.5 check out of range ruleIndex.
                Call with ruleIndex [wrong value], other parameters same as in 1.2.
                Expected: NON GT_OK.
            */

            ruleIndex = TCAM_INVALID_POS_RULE_INDEX;

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);


                        /*
                1.1.6 check out of range ruleIndex.
                Call with ruleIndex [out of range value], other parameters same as in 1.2.
                Expected: NON GT_OK.
            */

            ruleIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelTerm;;

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);



        }
                PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2  For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */

                ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
                ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
                valid     = GT_TRUE;

        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                        ,valid);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamPortGroupRuleRead
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamPatternPtr,
    OUT GT_U32                              *tcamMaskPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupRuleRead)
{

    GT_STATUS st = GT_OK;

    GT_U8                        dev;
    GT_PORT_GROUPS_BMP           portGroupsBmp   = 1;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize;
    GT_U32                       ruleIndex       = 0;
    GT_U32                       mask[UTF_TCAM_MAX_RULE_SIZE_CNS] = {TCAM_MASK, 0};
    GT_U32                       pattern[UTF_TCAM_MAX_RULE_SIZE_CNS] = {TCAM_PATTERN, 0};
    GT_BOOL                      valid;
    GT_U32                       portGroupId;
    GT_U32                       notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            /*
                1.1.1 Check with legal parameters. Call with ruleIndex [TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX].
                Expected: GT_OK
            */

            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
            ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
            valid     = GT_TRUE;

            st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                            ,valid,ruleSize, pattern, mask);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "device: %d, %d", dev, ruleIndex);

            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize,
                                              pattern, mask);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "device: %d, %d", dev, ruleIndex);

            /*
                1.1.2 check out of range ruleIndex.
                Call with ruleIndex [wrong value], other parameters same as in 1.1.1
                Expected: NON GT_OK.
            */

            ruleIndex = TCAM_INVALID_POS_RULE_INDEX;

            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize,
                                              pattern, mask);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);


            /*
                1.1.3 check out of range ruleIndex.
                Call with ruleIndex [out of range value], other parameters same as in 1.1.1
                Expected: NON GT_OK.
            */

            ruleIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelTerm;;

            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize,
                                              pattern, mask);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);


            /*
                1.1.4 Call with valid [NULL], other parameters same as in 1.2.
                Expected: GT_BAD_PTR.
            */

            ruleIndex = TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX;

            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, NULL, &ruleSize,
                                  pattern, mask);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, valid = NULL", dev);

            /*
                1.1.5 Call with ruleSize [NULL], other parameters same as in 1.2.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , NULL,
                                              pattern, mask);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ruleSize = NULL", dev);

            /*
                1.1.6 Call with pattern [NULL], other parameters same as in 1.2.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , &ruleSize,
                                              NULL, mask);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pattern = NULL", dev);

                         /*
                1.1.7 Call with mask [NULL], other parameters same as in 1.2.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , &ruleSize,
                                              pattern, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mask = NULL", dev);


        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , &ruleSize,
                                              pattern, mask);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */

                ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
        ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;
        valid     = GT_TRUE;

        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , &ruleSize,
                                      pattern, mask);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    ruleIndex = TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , &ruleSize,
                                              pattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &valid , &ruleSize,
                                              pattern, mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupRuleValidStatusGet)
{
    GT_STATUS st = GT_OK;

    GT_U8                                           dev;
    GT_PORT_GROUPS_BMP                              portGroupsBmp = 1;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize;
    GT_U32                                          ruleIndex = 0;
        GT_BOOL                             valid;
    GT_U32                                                            portGroupId;

    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            /*
                1.1.1 Check with legal parameters. Call with ruleSize [CPSS_PCL_RULE_SIZE_STD_E],
                ruleIndex [PCL_TESTED_RULE_INDEX], non-NULL mask, pattern and action.
                Expected: GT_OK
            */

            ruleIndex = TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX;

            st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "device: %d, %d", dev, ruleIndex);

            /*
                1.1.2 check out of range ruleIndex.
                Call with ruleIndex [wrong value], other parameters same as in 1.2.
                Expected: NON GT_OK.
            */

            ruleIndex = TCAM_INVALID_POS_RULE_INDEX;

            st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);

            /*
                1.1.3 check out of range ruleIndex.
                Call with ruleIndex [out of range value], other parameters same as in 1.2.
                Expected: NON GT_OK.
            */

            ruleIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelTerm;;

            st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ruleIndex);

            /*
                1.1.4 Call with valid [NULL], other parameters same as in 1.2.
                Expected: GT_BAD_PTR.
            */

            ruleIndex = TCAM_CORRECT_RULE_SIZE_30_RULE_INDEX;

                        st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, NULL, &ruleSize);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, valid = NULL", dev);

            /*
                1.1.5 Call with ruleSize [NULL], other parameters same as in 1.2.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, NULL);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ruleSize = NULL", dev);

        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

                        st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    ruleIndex = 0;


    /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamPortGroupRuleValidStatusGet(dev, portGroupsBmp, ruleIndex, &valid, &ruleSize);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamPortGroupClientGroupSet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    IN  GT_U32                              tcamGroup,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupClientGroupSet)
{
    GT_STATUS                    st = GT_OK;
    GT_U8                        dev;
    GT_PORT_GROUPS_BMP           portGroupsBmp = 1;
    CPSS_DXCH_TCAM_CLIENT_ENT    tcamClient=CPSS_DXCH_TCAM_IPCL_0_E;
    GT_U32                       tcamGroup=0;
    GT_BOOL                      enable=GT_TRUE;
    GT_U32                       numOfClientGroups;
    GT_U32                       portGroupId;
    GT_U32                       notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    notAppFamilyBmp |= UTF_BOBCAT3_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            numOfClientGroups = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.tcam.sip6maxTcamGroupId;
        }
        else
        {
            numOfClientGroups = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.tcam.maxClientGroups;
        }

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

                        /*
                1.1.1 Check with legal parameters. Call with tcamClient [CPSS_DXCH_TCAM_IPCL_0_E],
                tcamGroup [1], enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK
            */

            tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
            tcamGroup = 0;
            enable = GT_TRUE;

            st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "device: %d, tcamClient: %d, tcamGroup: %d", dev, tcamClient, tcamGroup);

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;
            st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "device: %d, tcamClient: %d, tcamGroup: %d", dev, tcamClient, tcamGroup);

            st = cpssDxChTcamPortGroupClientGroupSet(
                dev, portGroupsBmp, tcamClient, numOfClientGroups/*tcamGroup*/, GT_TRUE /*enable*/);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "device: %d, tcamClient: %d, tcamGroup: %d", dev, tcamClient, tcamGroup);

            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                st = cpssDxChTcamPortGroupClientGroupSet(
                    dev, portGroupsBmp, CPSS_DXCH_TCAM_IPCL_0_E, 3/*tcamGroup*/, GT_TRUE /*enable*/);
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
                    GT_OK, st, "AC5P IPCL tcamGroup restiction range 0-2 not checked");
            }



                         /*
                1.1.2 Check for wrong enum values tcamClient and other params same as in 1.1.1
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTcamPortGroupClientGroupSet
                                (dev, portGroupsBmp, tcamClient, tcamGroup, enable) ,tcamClient);

                        /*
                1.1.3 check out of range tcamGroup.
                Call with tcamGroup [wrong value - 5], other parameters same as in 1.1.1
                Expected: NON GT_OK.
            */

                        enable = GT_TRUE;
            tcamGroup = 5;

            st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcamGroup);

            /*
                1.1.4 check out of range tcamGroup.
                Call with tcamGroup [wrong value - out of range value], other parameters same as in 1.1.1
                Expected: NON GT_OK.
            */

            enable = GT_TRUE;
            tcamGroup = 0xFF;

            st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcamGroup);
                }

                PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId);

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

                        tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
                        tcamGroup = 1;
                        enable = GT_TRUE;

                        st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */

        tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
        tcamGroup = 0;
        enable = GT_TRUE;

        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

   /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamPortGroupClientGroupGet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    OUT GT_U32                              *tcamGroupPtr,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupClientGroupGet)
{

    GT_STATUS st = GT_OK;

    GT_U8                                           dev;
    GT_PORT_GROUPS_BMP                              portGroupsBmp = 1;
    CPSS_DXCH_TCAM_CLIENT_ENT                tcamClient= CPSS_DXCH_TCAM_IPCL_0_E;
    GT_U32                                          tcamGroup = 0;
        GT_BOOL                             enable= GT_TRUE;
        GT_U32                              portGroupId;


    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

                        /*
                1.1.1 Check with legal parameters. Call with tcamClient [CPSS_DXCH_TCAM_IPCL_0_E].
                Expected: GT_OK
            */

            tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
                        tcamGroup = 0;
                        enable = GT_TRUE;

            if(GT_FALSE == PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev))
            {
                st = cpssDxChTcamPortGroupClientGroupSet (dev, portGroupsBmp, tcamClient, tcamGroup, enable);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "device: %d, tcamClient: %d, tcamGroup: %d", dev, tcamClient, tcamGroup);
            }

            st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, &tcamGroup, &enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "device: %d, tcamClient: %d", dev, tcamClient);


                         /*
                1.1.2 Check for wrong enum values tcamClient and other params same as in 1.1.1
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTcamPortGroupClientGroupGet
                                (dev, portGroupsBmp, tcamClient, &tcamGroup, &enable) ,tcamClient);

                        /*
                1.1.3 Call with tcamGroup [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

                        st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, NULL, &enable);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcamGroup = NULL", dev);

                        /*
                1.1.4 Call with enable [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

                        st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, &tcamGroup, NULL);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enable = NULL", dev);
                }

                PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId);

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

            tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;

            st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, &tcamGroup, &enable);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */

                tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, &tcamGroup, &enable);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

   /*2. Go over all non active devices. */


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, &tcamGroup, &enable);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamPortGroupClientGroupGet (dev, portGroupsBmp, tcamClient, &tcamGroup, &enable);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U8            dev,
    IN GT_U32           startRuleIndex,
    IN GT_U32           numOfRuleEntries,
    IN GT_U32           group,
    IN GT_U32           hitNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamIndexRangeHitNumAndGroupSet)
{
    GT_STATUS st = GT_OK;

    GT_U8            dev;
    GT_U32           floorIndex =12;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC           floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    CPSS_DXCH_TCAM_BLOCK_INFO_STC           floorInfoArrGet[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32      notAppFamilyBmp        = 0;
    GT_U32      ii;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /*
                        1.1 Check with legal parameters. Call with floorIndex [0],
                        floorInfoArr[0].group[0], floorInfoArr[0].hitNum[0],floorInfoArr[1].group[0],floorInfoArr[1].hitNum[0].
                        Expected: GT_OK
                */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group = 0;
            floorInfoArr[ii].hitNum = 0;
        }
        floorIndex = 0;

                st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "device: %d, startRuleIndex: %d", dev, floorIndex);


                /*
                        1.2 check out of range floorIndex.
                        Call with floorIndex [wrong value], other parameters same as in 1.1
                        Expected: NON GT_OK.
                */

                floorIndex = CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS;

                st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);

                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, floorIndex);


                /*
                        1.3 check invalid group.
                        Call with group [wrong value], other parameters same as in 1.1
                        Expected: NON GT_OK.
                */

        floorIndex=0;
                floorInfoArr[0].group=6;

                st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);

                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, floorInfoArr[0].group);

                /*
                        1.4 check invalid hitNum.
                        Call with hitNum [wrong value], other parameters same as in 1.1
                        Expected: NON GT_OK.
                */

        floorInfoArr[0].group=0;
        floorInfoArr[0].hitNum=4;

                st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, floorInfoArr[0].hitNum);

        floorInfoArr[0].hitNum=0;

    }

    /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 4. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /*
                4.1 Set valid parameter and check we got the same values using get
            */
            for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
            {
                floorInfoArr[ii].group = ii/2;
                floorInfoArr[ii].hitNum = ii/2;
            }
            floorIndex = 0;

            st = cpssDxChTcamIndexRangeHitNumAndGroupSet (dev, floorIndex, floorInfoArr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "device: %d, startRuleIndex: %d", dev, floorIndex);

            st = cpssDxChTcamIndexRangeHitNumAndGroupGet (dev, floorIndex, floorInfoArrGet);
            if (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.tcam.numBanksForHitNumGranularity == 6)
            {
                for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(floorInfoArr[ii].group, floorInfoArrGet[ii].group, dev);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(floorInfoArr[ii].hitNum, floorInfoArrGet[ii].hitNum, dev);
                }
            }
            else
            {
                for (ii = 0; ii < 2; ii++)
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(floorInfoArr[ii].group, floorInfoArrGet[ii].group, dev);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(floorInfoArr[ii].hitNum, floorInfoArrGet[ii].hitNum, dev);
                }
            }
    }
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN  GT_U8            dev,
    IN  GT_U32           startRuleIndex,
    IN  GT_U32           numOfRuleEntries,
    OUT GT_U32           *groupPtr,
    OUT GT_U32           *hitNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamIndexRangeHitNumAndGroupGet)
{
        GT_STATUS st = GT_OK;

    GT_U8            dev;
        GT_U32           floorIndex =12;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC           floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];


    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /*
                        1.1 Check with legal parameters. Call with floorIndex [0].
                        Expected: GT_OK
                */

                floorIndex =0;

                st = cpssDxChTcamIndexRangeHitNumAndGroupGet (dev, floorIndex,floorInfoArr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "device: %d, startRuleIndex: %d", dev, floorIndex);


                /*
                        1.2 check out of range floorIndex.
                        Call with startRuleIndex [wrong value], other parameters same as in 1.1
                        Expected: NON GT_OK.
                */

                floorIndex = CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS;

                st = cpssDxChTcamIndexRangeHitNumAndGroupGet (dev, floorIndex,floorInfoArr);

                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, floorIndex);


                /*
                        1.3 Call with floorInfoArr [NULL], other parameters same as in 1.1
                        Expected: GT_BAD_PTR.
                */

                floorIndex = 0;
                st = cpssDxChTcamIndexRangeHitNumAndGroupGet (dev, floorIndex,NULL);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, floorInfoArr = NULL", dev);

    }

    /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamIndexRangeHitNumAndGroupGet (dev, floorIndex,floorInfoArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamIndexRangeHitNumAndGroupGet (dev, floorIndex,floorInfoArr);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamActiveFloorsSet
(
    IN  GT_U8           dev,
    IN  GT_U32          numOfActiveFloors
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamActiveFloorsSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      numOfActiveFloors = 0;

    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
                /*
                        1.1 Check with numOfActiveFloors [0].
                        Expected: GT_OK
                */

                numOfActiveFloors = 0;


                st = cpssDxChTcamActiveFloorsSet (dev, numOfActiveFloors);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "device: %d, numOfActiveFloors: %d", dev, numOfActiveFloors);
                /*
                        1.2 Check with numOfActiveFloors [11].
                        Expected: GT_OK
                */

                numOfActiveFloors = CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(dev);


                st = cpssDxChTcamActiveFloorsSet (dev, numOfActiveFloors);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "device: %d, numOfActiveFloors: %d", dev, numOfActiveFloors);

                /*
                        1.3 check out of range numOfActiveFloors.
                        Call with numOfActiveFloors [13]
                        Expected: NOT GT_OK.
                */

                numOfActiveFloors = CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(dev)+1;

                st = cpssDxChTcamActiveFloorsSet (dev, numOfActiveFloors);

                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, numOfActiveFloors);

                numOfActiveFloors = 0;

    }

    /*2. Go over all non active devices. */
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
       st = cpssDxChTcamActiveFloorsSet (dev, numOfActiveFloors);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamActiveFloorsSet (dev, numOfActiveFloors);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamActiveFloorsGet
(
    IN  GT_U8           dev,
    OUT GT_U32          numOfActiveFloorsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTcamActiveFloorsGet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      numOfActiveFloors;
    GT_U32      expNumOfActiveFloors;
    GT_U32      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1 Check with legal parameters.
                Expected: GT_OK
         */

          st = cpssDxChTcamActiveFloorsGet (dev, &numOfActiveFloors);
          UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "device: %d, numOfActiveFloors: %d", dev, numOfActiveFloors);

          /* Expected number of active floor should be zero in case of vTCAM Manager */
          expNumOfActiveFloors = CPSS_DXCH_TCAM_ACTIVE_FLOOR_NUM_MAC(dev);
          /*
             1.1 Check with numOfActiveFloors at Init stage Without vTCAM manager.
             Expected: GT_OK
          */
          UTF_VERIFY_EQUAL1_STRING_MAC(expNumOfActiveFloors, numOfActiveFloors,
                  "No of active floors at init is not Zero, numOfActiveFloors: %d", numOfActiveFloors);

          /*
              1.2 Call with numOfActiveFloors [NULL], other parameters same as in 1.1
                  Expected: GT_BAD_PTR.
          */
          st = cpssDxChTcamActiveFloorsGet (dev, NULL);
          UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bankBmp = NULL", dev);

    }


    /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamActiveFloorsGet (dev, &numOfActiveFloors);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamActiveFloorsGet (dev, &numOfActiveFloors);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                             *tcamKeyPtr,
    IN  GT_U32                              subKeyProfile
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupCpuLookupTriggerSet)
{
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_PORT_GROUPS_BMP                  portGroupsBmp = 1;
    GT_U32                              ii, kk;
    GT_U32                              group;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        size;
    GT_U32                              subKeyProfile = 0;
    GT_U32                              subKeyProfileGet;
    GT_U32                              keyArr[] = {3, 5, 8, 10, 13, 15, 20};
    GT_U32                              tcamKeyGetArr[20];
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        sizeArr[] = {CPSS_DXCH_TCAM_RULE_SIZE_10_B_E, CPSS_DXCH_TCAM_RULE_SIZE_20_B_E,
                                                     CPSS_DXCH_TCAM_RULE_SIZE_30_B_E, CPSS_DXCH_TCAM_RULE_SIZE_40_B_E,
                                                     CPSS_DXCH_TCAM_RULE_SIZE_50_B_E, CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                                                     CPSS_DXCH_TCAM_RULE_SIZE_80_B_E};
    GT_U32                              tcamKeyArr[] = {10, 21, 23, 43, 19, 27, 90, 22, 45, 64, 87, 12, 31, 34, 28, 23, 84, 20, 50, 11};/* support max key of 80 UDBs */
    GT_U32                              portGroupId;
    GT_U32                              notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)tcamKeyGetArr, sizeof(tcamKeyGetArr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }
            /*
                1.1.1 Check with legal parameters. Call with group [0],
                                                   size [CPSS_DXCH_TCAM_RULE_SIZE_10_B_E],
                                                        [CPSS_DXCH_TCAM_RULE_SIZE_20_B_E],
                                                        [CPSS_DXCH_TCAM_RULE_SIZE_30_B_E],
                                                        [CPSS_DXCH_TCAM_RULE_SIZE_40_B_E],
                                                        [CPSS_DXCH_TCAM_RULE_SIZE_50_B_E],
                                                        [CPSS_DXCH_TCAM_RULE_SIZE_60_B_E],
                                                        [CPSS_DXCH_TCAM_RULE_SIZE_80_B_E]
                Expected: GT_OK
            */

            for (ii = 0; ii <sizeof(sizeArr)/sizeof(sizeArr[0]); ii++)
            {
                size = 0;
                group = 0;
                subKeyProfile = PRV_CPSS_SIP_6_10_CHECK_MAC(dev) ? 10 : 0;

                st = cpssDxChTcamPortGroupCpuLookupTriggerSet(dev, portGroupsBmp, group, sizeArr[ii], tcamKeyArr, subKeyProfile);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"device: %d, tcamClient: %d", dev, group);

                st = cpssDxChTcamPortGroupCpuLookupTriggerGet(dev, portGroupsBmp, group, &size, tcamKeyGetArr, &subKeyProfileGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"device: %d, tcamClient: %d", dev, group);

                /* validate values */
                UTF_VERIFY_EQUAL1_STRING_MAC(sizeArr[ii], size, "got another size then was set: %d", sizeArr[ii]);
                if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(subKeyProfile, subKeyProfileGet, "got another subKeyProfile then was set: %d", subKeyProfile);
                }

                for( kk = 0; kk < keyArr[ii]; kk++)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(tcamKeyArr[kk], tcamKeyGetArr[kk], "got another key then was set: %d %d", tcamKeyGetArr[kk],size);

                }
                for( kk = keyArr[ii]; kk <= 20 - keyArr[ii]; kk++)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(tcamKeyGetArr[kk], 0, "got another key then was set: %d", tcamKeyGetArr[kk]);
                }
                cpssOsBzero((GT_VOID*)tcamKeyGetArr, sizeof(tcamKeyGetArr));
            }

            /*
                1.1.2 Check for wrong enum values tcamClient and other params same as in 1.1.1
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTcamPortGroupCpuLookupTriggerSet
                                (dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile) ,size);

            /*
                1.1.2 Check for wrong enum values subKeyProfile and other params same as in 1.1.1
                Expected: GT_BAD_PARAM.
            */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
            {
                subKeyProfile = 64;

                st = cpssDxChTcamPortGroupCpuLookupTriggerSet(dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"device: %d", dev);
                subKeyProfile = PRV_CPSS_SIP_6_10_CHECK_MAC(dev) ? 10 : 0;
            }

            /*
                1.1.3 Call with tcamKeyArr [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, NULL, subKeyProfile);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcamKeyArr = NULL", dev);

            /*
                1.1.4 check invalid group.
                      Call with group [wrong value], other parameters same as in 1.1.1
                Expected: NON GT_OK.
            */

            group = 5;
            st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);
        }

        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId);

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);
            group= 0;
            st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        group= 0;
        st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;
    /* 2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyArr, subKeyProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT       *sizePtr,
    OUT GT_U32                             *tcamKeyPtr,
    OUT GT_U32                             *subKeyProfilePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamPortGroupCpuLookupTriggerGet)
{
        GT_STATUS st = GT_OK;

    GT_U8                        dev;
    GT_PORT_GROUPS_BMP           portGroupsBmp   = 1;
    GT_U32                       group           = 0;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT size            = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
    GT_U32                       tcamKeyPtr[20];
    GT_U32                       portGroupId;
    GT_U32                       notAppFamilyBmp = 0;
    GT_U32                       subKeyProfile   = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&tcamKeyPtr, sizeof(tcamKeyPtr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

             /*
                1.1.1 Check with legal parameters. Call with group [0].
                Expected: GT_OK
            */

            group =0;
            size = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
            subKeyProfile = 0;

            st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, tcamKeyPtr, &subKeyProfile);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "device: %d, group: %d", dev, group);


             /*
                1.1.2 Call with size [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, NULL, tcamKeyPtr, &subKeyProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, size = NULL", dev);


             /*
                1.1.3 Call with tcamKeyPtr [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, NULL, &subKeyProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcamKeyPtr = NULL", dev);

             /*
                1.1.3 Call with subkeyProfile [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
            {
                st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, tcamKeyPtr, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, subKeyProfile = NULL", dev);
            }

            /*
                    1.1.4 check invalid group.
                    Call with group [wrong value], other parameters same as in 1.1.1
                    Expected: NON GT_OK.
            */

            group = 5;

            st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, tcamKeyPtr, &subKeyProfile);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);

            /*
                    1.1.5 check invalid numOfRuleEntries.
                    Call with group [out of range value], other parameters same as in 1.1.1
                    Expected: NON GT_OK.
            */

            group = 0xFF;

            st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, tcamKeyPtr, &subKeyProfile);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);


        }

        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId);

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);
            group= 0;
            st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, tcamKeyPtr, &subKeyProfile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        group= 0;
        st = cpssDxChTcamPortGroupCpuLookupTriggerGet (dev, portGroupsBmp, group, &size, tcamKeyPtr, &subKeyProfile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

   /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
       st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyPtr, subKeyProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamPortGroupCpuLookupTriggerSet (dev, portGroupsBmp, group, size, tcamKeyPtr, subKeyProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTcamCpuLookupResultsGet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamCpuLookupResultsGet)
{
        GT_STATUS st = GT_OK;

    GT_U8                                           dev;
    GT_PORT_GROUPS_BMP                              portGroupsBmp = 1;
    GT_U32                              group =0;
        GT_U32                              hitNum=0;
    GT_BOOL                             isValid;
        GT_BOOL                             isHitPtr;
        GT_U32                              hitIndexPtr;
        GT_U32                                                                portGroupId=0;

    GT_U32      notAppFamilyBmp        = 0;

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 Go over all active port groups. */

        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            /*
                1.1.1 Check with legal parameters. Call with group [0], hitNum [0].
                Expected: GT_OK
            */

            group =0;
            hitNum = 0;

            st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "device: %d, tcamClient: %d", dev, group);


            /*
                1.1.2 Call with isValid [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, NULL, &isHitPtr, &hitIndexPtr);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, isValid = NULL", dev);


            /*
                1.1.3 Call with isHitPtr [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, NULL, &hitIndexPtr);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, isHitPtr = NULL", dev);

            /*
                1.1.4 Call with hitIndexPtr [NULL], other parameters same as in 1.1.1
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, NULL);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, hitIndexPtr = NULL", dev);

            /*
                    1.1.5 check invalid group.
                    Call with group [wrong value], other parameters same as in 1.1.1
                    Expected: NON GT_OK.
            */

            group = 5;

            st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);

            /*
                    1.1.5 check invalid numOfRuleEntries.
                    Call with group [out of range value], other parameters same as in 1.1.1
                    Expected: NON GT_OK.
            */

            group = 0xFF;

            st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);

            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, group);

        }

        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId);

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            portGroupsBmp = (1 << portGroupId);

                        group= 0;

                        st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        group= 0;


                st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

   /*2. Go over all non active devices. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                 */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamCpuLookupResultsGet (dev, portGroupsBmp, group, hitNum, &isValid, &isHitPtr, &hitIndexPtr);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*#ifndef ASIC_SIMULATION*/
#ifndef ASIC_SIMULATION
static GT_U32 tcamWaIterations = 1000000;
#else
static GT_U32 tcamWaIterations = 5;/*WM*/
#endif
GT_U32 debug_utTcamWaIterationsSet(GT_U32 newIteration)
{
    GT_U32 oldIterations = tcamWaIterations;
    tcamWaIterations =  newIteration;
    return oldIterations;
}

UTF_TEST_CASE_MAC(cpssDxChTcamWriteParityDaemonTest)
{
/*
    ITERATE_DEVICES
    1.1. check that TCAM Parity interrupt is not erased during TCAM write
    Call with portGroupsBmp

*/

    GT_STATUS                           st = GT_OK;
    GT_U32                              portGroupId = 0;
    GT_U8                               dev;
    GT_PORT_GROUPS_BMP                  portGroupsBmp = 1;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize    = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
    GT_U32                              ruleIndex = 0;
    GT_U32                              maskArr[3];
    GT_U32                              patternArr[3];
    GT_BOOL                             valid       = GT_FALSE;
    GT_U32                              ii;
    GT_U32                              notAppFamilyBmp        = 0;
    GT_U32                              numOfEvents = 0; /*number of events expected (interrupts) */
    CPSS_UNI_EV_CAUSE_ENT               uniEvent;/* unified event */
    GT_U32                              tcamSize = _12K;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    uniEvent = CPSS_PP_DATA_INTEGRITY_ERROR_E;

    if(cpssDeviceRunCheck_onEmulator() &&
        tcamWaIterations == 1000000)/*was not predefined*/
    {
        /* on Phoenix emulator this equal 60 seconds running time of the test (reasonable time) */
        tcamWaIterations = 4000;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PCL_TCAM_E, &tcamSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 Go over all active port groups. */

        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            cpssOsMemSet(maskArr, 0, sizeof(maskArr));
            cpssOsMemSet(patternArr, 0, sizeof(patternArr));

            if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))/*since ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_10_B_E */
            {
                /* the actions are in 20B rule resolution */
                /* the rules in the odd entries that maybe from the previous tests , not allow this test to run */
                /* so first we must remove those old entries from the tcam  */
                ruleIndex = 1;
                valid     = GT_FALSE;
                for (ii = 0 ; ii < tcamWaIterations; ii++)
                {
                    st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex,valid);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ruleIndex, valid);
                    ruleIndex += 2;
                    if(ruleIndex >= tcamSize)
                    {
                        break;
                    }
                }

                valid     = GT_TRUE;
            }

            /*
                1.1.1 Call with  ruleSize [CPSS_DXCH_TCAM_RULE_SIZE_10_B_E],
                                ruleIndex [TCAM_CORRECT_RULE_SIZE_10_RULE_INDEX],
                                valid [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with valid = GT_TRUE */
            ruleSize  = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
            ruleIndex = 0;
            valid     = GT_TRUE;

            /* give time for event task to handle interrupts */
            cpssOsTimerWkAfter(100);

            /* clear interrupts (from previous time) */
            st = utfGenEventCounterGet(dev, uniEvent, GT_TRUE, &numOfEvents);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "utfGenEventCounterGet");

            ruleIndex = 0;
            for (ii = 0 ; ii < tcamWaIterations; ii++)
            {
                if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev)) /*since ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_10_B_E */
                {
                    ruleIndex |= 1; /* make rule index odd */
                }
                /* Set mask and pattern */
                maskArr[0] = 1;
                patternArr[0] = 0;
                st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                            ,valid,ruleSize, patternArr, maskArr);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

                /* change one bit in pattern to change parity */
                maskArr[0] = 1;
                patternArr[0] = 1;

                st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                            ,valid,ruleSize, patternArr, maskArr);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

                ruleIndex++;
                if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))/*since ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_10_B_E */
                {
                    /* the actions are in 20B rule resolution */
                    ruleIndex++;
                }
                ruleIndex = ruleIndex % tcamSize;
            }

            /* give time for event task to handle interrupts */
            cpssOsTimerWkAfter(100);

            /* check data integrity interrupts */
            st = utfGenEventCounterGet(dev, uniEvent, GT_TRUE, &numOfEvents);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "utfGenEventCounterGet");

            UTF_VERIFY_EQUAL0_STRING_MAC(0, numOfEvents, "TCAM Parity Error");

            for (ii = 0 ; ii < tcamWaIterations; ii++)
            {
                if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev)) /*since ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_10_B_E */
                {
                    ruleIndex |= 1; /* make rule index odd */
                }
                /* Set mask and pattern */
                maskArr[0] = 1;
                patternArr[0] = 0;
                st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                            ,valid,ruleSize, patternArr, maskArr);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ruleSize, ruleIndex, valid);

                /* Invalid rule. This operation changes one bit and parity also. */
                st = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp,ruleIndex
                                                            ,GT_FALSE);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  ruleIndex, valid);

                ruleIndex++;
                if(PRV_CPSS_SIP_6_10_CHECK_MAC(dev))/*since ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_10_B_E */
                {
                    /* the actions are in 20B rule resolution */
                    ruleIndex++;
                }
                ruleIndex = ruleIndex % tcamSize;
            }

            /* give time for event task to handle interrupts */
            cpssOsTimerWkAfter(100);

            /* check data integrity interrupts */
            st = utfGenEventCounterGet(dev, uniEvent, GT_TRUE, &numOfEvents);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "utfGenEventCounterGet");

            UTF_VERIFY_EQUAL0_STRING_MAC(0, numOfEvents, "TCAM Parity Error");
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
    }
}

#if 0
UTF_TEST_CASE_MAC(cpssDxChTcamClearPerClient)
{
    GT_STATUS st;
    GT_PORT_GROUPS_BMP                  portGroupsBmp = CPSS_CHIPLETS_UNAWARE_MODE_CNS;
    CPSS_DXCH_TCAM_CLIENT_ENT           tcamClientArr[] = {CPSS_DXCH_TCAM_TTI_E, CPSS_DXCH_TCAM_IPCL_0_E, CPSS_DXCH_TCAM_EPCL_E};
    GT_U32                              tcamGroup; /* tcam group */
    GT_U32                              tempTcamGroup; /* temporary tcam group */
    GT_BOOL                             valid = GT_TRUE; /* rule's valid status*/
    GT_U8                               dev; /* device number */
    GT_U32                              ruleIndex; /* rule index */
    GT_U32                              jj = 0; /* iteratore */
    GT_U32                              numOfActiveFloors; /* number of active tcam floors */
    CPSS_DXCH_TCAM_BLOCK_INFO_STC       floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]; /* tcam floor info */
    GT_U32                              numOfBlocksInFloor; /* amount of blocks in one tcam floor */
    GT_U32                              numOfBanksInBlock;  /* amount of banks in one tcam block */
    GT_U32                              numOfIndexesInBank = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS; /* amount of data raws in one TCAM bank */
    GT_U32                              notAppFamilyBmp        = 0;

    /* tti rule variables */
    CPSS_DXCH_TTI_RULE_TYPE_ENT         ttiRuleType = CPSS_DXCH_TTI_RULE_UDB_20_E;
    CPSS_DXCH_TTI_RULE_UNT              ttiPattern;
    CPSS_DXCH_TTI_RULE_UNT              ttiMask;   /* this feature is on eArch devices */
    CPSS_DXCH_TTI_ACTION_STC            ttiAction;

    /* pcl rule variables */
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  pclRuleFormat;
    CPSS_DXCH_PCL_RULE_OPTION_ENT       ruleOptionsBmp = CPSS_DXCH_PCL_RULE_OPTION_WRITE_DEFAULT_E;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       pclPattern;
    CPSS_DXCH_PCL_ACTION_STC            pclAction;
    CPSS_PCL_RULE_SIZE_ENT              pclRuleSize;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    /* some previous test result in failure on BC3 WM  */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT3_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfBlocksInFloor = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.tcam.numBanksForHitNumGranularity;
        numOfBanksInBlock = CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS / numOfBlocksInFloor;
        numOfActiveFloors = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS;

        st = cpssDxChTcamActiveFloorsSet (dev, numOfActiveFloors);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "device: %d, numOfActiveFloors: %d", dev, numOfActiveFloors);

        /* enable all clients and bind the blocks to different TCAM group */
        st = cpssDxChPclIngressPolicyEnable(dev, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPclIngressPolicyEnable");
        st = cpssDxCh2PclEgressPolicyEnable(dev, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxCh2PclEgressPolicyEnable");

        /* bind each of the test's client to TCAM group by increased order */
        for (tcamGroup = 0 ; tcamGroup <sizeof(tcamClientArr)/sizeof(tcamClientArr[0]) ; tcamGroup++)
        {
            tempTcamGroup = tcamGroup;

            if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.tcam.sip6maxTcamGroupId == 1)
            {
                /* use the single group supported by the device */
                tcamGroup = 0;
            }

            /* Bobcat3 clients bound to specifiec groups */
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
            {
                switch (tcamClientArr[tcamGroup])
                {
                    case CPSS_DXCH_TCAM_TTI_E:
                        tcamGroup = 0;
                        break;
                    case CPSS_DXCH_TCAM_IPCL_0_E:
                        tcamGroup = 1;
                        break;
                    case CPSS_DXCH_TCAM_IPCL_1_E:
                        tcamGroup = 2;
                        break;
                    case CPSS_DXCH_TCAM_IPCL_2_E:
                        tcamGroup = 3;
                        break;
                    case CPSS_DXCH_TCAM_EPCL_E:
                        tcamGroup = 4;
                        break;
                    default: /* does not supposed to reach this part */
                        tcamGroup = 0;
                        break;
                }
            }
            else
            {
                st = cpssDxChTcamPortGroupClientGroupSet(dev, portGroupsBmp, tcamClientArr[tcamGroup], tcamGroup /*group ID*/, GT_TRUE);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"cpssDxChTcamPortGroupClientGroupSet - device: %d, tcamClient: %d, tcamGroup: %d",
                                             dev, tcamClientArr[tcamGroup], tcamGroup);
            }

            /* if each floor bind all blocks to the relevant group */
            for (jj = 0 ; jj < numOfBlocksInFloor; jj++)
            {
                floorInfoArr[jj].group = tcamGroup;
                floorInfoArr[jj].hitNum = 0;
            }

            /* set for each TCAM index which TCAM group and lookup number is served by TCAM */
            st = cpssDxChTcamIndexRangeHitNumAndGroupSet(dev, tcamGroup /*floorIndex*/, floorInfoArr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTcamIndexRangeHitNumAndGroupSet - device: %d, floorIndex: %d", dev, tcamGroup);

            ruleIndex = tcamGroup /*floor index */* numOfBlocksInFloor * numOfBanksInBlock * numOfIndexesInBank; /* first raw in the floor */

            if (tcamClientArr[tcamGroup] == CPSS_DXCH_TCAM_TTI_E)
            {
                /* set tti rule */
                cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
                cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
                cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));
                ttiAction.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
                ttiAction.bridgeBypass = GT_TRUE;
                ttiAction.ingressPipeBypass = GT_FALSE;
                st = cpssDxChTtiRuleSet(dev, ruleIndex, ttiRuleType, &ttiPattern, &ttiMask, &ttiAction);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTtiRuleSet");

                /* delete the rule using the lua command */
                prvCpssDxChTcamInvalidateRulesPerClient(dev, CPSS_DXCH_TCAM_TTI_E, GT_FALSE);

                /* get valid status of the rule */
                st = cpssDxChTtiRuleValidStatusGet(dev, ruleIndex, &valid);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTtiRuleValidStatusGet");

            }
            if (tcamClientArr[tcamGroup] == CPSS_DXCH_TCAM_IPCL_0_E || tcamClientArr[tcamGroup] == CPSS_DXCH_TCAM_EPCL_E )
            {
                /* set pcl rule */
                pclRuleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                cpssOsMemSet(&pclPattern,0,sizeof(pclPattern));
                cpssOsMemSet(&pclMask   ,0,sizeof(pclMask));
                cpssOsMemSet(&pclAction ,0,sizeof(pclAction));

                pclAction.mirror.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;
                st = cpssDxChPclRuleSet(dev, 0/*tcamIndex*/, pclRuleFormat, ruleIndex, ruleOptionsBmp, &pclMask, &pclPattern, &pclAction);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPclRuleSet");

                /* delete the rule using the lua command */
                prvCpssDxChTcamInvalidateRulesPerClient(dev, tcamClientArr[tcamGroup], GT_FALSE);

                /* get valid status of the rule */
                st = cpssDxChPclRuleStateGet(dev, 0/*tcamIndex*/, ruleIndex, &valid, &pclRuleSize);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChPclRuleStateGet");
            }
            /* check that the rule is invalid */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, valid, "Test Failed");

            tcamGroup = tempTcamGroup;
        }
    }
}
#endif

/*****************************************************************************/
/*  The code below is tool for read register performance measurement.        */
/*  The test checks read register lasting in PP core clocks.                 */
/*****************************************************************************/

/* bits of address passes as is throw PCI window */
#define NOT_ADDRESS_COMPLETION_BITS_NUM_CNS 19
/* bits of address extracted from address completion registers */
#define ADDRESS_COMPLETION_BITS_MASK_CNS    (0xFFFFFFFF << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

#define LAST_READ_TS_MAC() \
*((volatile GT_U32 *)(0x00000040 + baseAddr))

int debug_pex_perfcheck()
{
    GT_UINTPTR baseAddr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[0]->hwInfo[0].resource.switching.start;
    GT_UINTPTR address;
    GT_U32 pciAddr, regAddr, compIdx, leData, temp, addrRegion, lastTs, lastTs2;
    GT_U32 completReg = 0x0000013C;
    cpssOsPrintf("base addr 0x%X\n", baseAddr);
    cpssOsTimerWkAfter(500);

    regAddr = 0x05501000;
    compIdx = 7;

    pciAddr =
        ((compIdx << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)
        | (regAddr & (~ ADDRESS_COMPLETION_BITS_MASK_CNS)));

    addrRegion = (regAddr >> NOT_ADDRESS_COMPLETION_BITS_NUM_CNS);

    /*  write to completion register */
    address = completReg + baseAddr;
    leData = addrRegion;

    *((volatile GT_U32*)address) = leData;
    /* need to put GT_SYNC to avoid from having the CPU doing
    * write combine. Some CPUs like advanced new PPC can do write combining
    * of two writes to continuous addresses. So, to avoid that we use the eioio
    * in PPC architecture */
    GT_SYNC; /* to avoid from write combining */

    /* read regiter */
    address = pciAddr + baseAddr;
    temp = *((volatile GT_U32 *)address);
    lastTs = LAST_READ_TS_MAC();
    temp = *((volatile GT_U32 *)address);
    lastTs2 = LAST_READ_TS_MAC();

    cpssOsPrintf("reg addr 0x%X data 0x%X \n", regAddr, temp);
    cpssOsPrintf(" - One reads TS1 0x%X TS2 0x%X diff %d\n", lastTs, lastTs2, lastTs2 - lastTs);
    cpssOsTimerWkAfter(1000);

    temp = *((volatile GT_U32 *)address);
    lastTs = LAST_READ_TS_MAC();
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    temp = *((volatile GT_U32 *)address);
    lastTs2 = LAST_READ_TS_MAC();

    cpssOsPrintf("reg addr 0x%X data 0x%X \n", regAddr, temp);
    cpssOsPrintf(" - 10 reads TS1 0x%X TS2 0x%X diff %d\n", lastTs, lastTs2, lastTs2 - lastTs);
    cpssOsTimerWkAfter(1000);

    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, 0x00000040, &lastTs);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, 0x00000040, &lastTs2);

    cpssOsPrintf("reg addr 0x%X data 0x%X \n", regAddr, temp);
    cpssOsPrintf(" - One API reads TS1 0x%X TS2 0x%X diff %d\n", lastTs, lastTs2, lastTs2 - lastTs);
    cpssOsTimerWkAfter(1000);

    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, 0x00000040, &lastTs);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, regAddr, &temp);
    prvCpssDrvHwPpPortGroupReadRegister(0, 0, 0x00000040, &lastTs2);

    cpssOsPrintf("reg addr 0x%X data 0x%X \n", regAddr, temp);
    cpssOsPrintf(" - 10 API reads TS1 0x%X TS2 0x%X diff %d\n", lastTs, lastTs2, lastTs2 - lastTs);

    return 0;
}
/*#endif*/

GT_STATUS  debugTcam(GT_U32 ruleIndex)
{
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev = 0;
    GT_PORT_GROUPS_BMP                  portGroupsBmp = 1;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize    = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSizeGet;
    GT_U32                              mask[UTF_TCAM_MAX_RULE_SIZE_CNS] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF, 0};
    GT_U32                              pattern[UTF_TCAM_MAX_RULE_SIZE_CNS] = {0x4411,0x5522,0x6633, 0};
    GT_U32                              maskGet[UTF_TCAM_MAX_RULE_SIZE_CNS] = {0};
    GT_U32                              patternGet[UTF_TCAM_MAX_RULE_SIZE_CNS] = {0};
    GT_BOOL                             valid;
    GT_BOOL                             validGet;

    valid     = GT_TRUE;

    st = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp,ruleIndex
                                                ,valid,ruleSize, pattern, mask);
    if (st != GT_OK)
    {
        cpssOsPrintf("Write Failed\n");
        return st;
    }

    st = cpssDxChTcamPortGroupRuleRead(dev, portGroupsBmp, ruleIndex, &validGet, &ruleSizeGet,
                                      patternGet, maskGet);
    if (st != GT_OK)
    {
        cpssOsPrintf("Read Failed\n");
        return st;
    }

    cpssOsPrintf("Valid %d, Size %d\n", validGet, ruleSizeGet);
    cpssOsPrintf("Mask 0x%08X 0x%08X 0x%08X\n", maskGet[0],maskGet[1],maskGet[2]);
    cpssOsPrintf("Pattern 0x%08X 0x%08X 0x%08X\n", patternGet[0],patternGet[1],patternGet[2]);

    return GT_OK;
}

/*
GT_STATUS cpssDxChTcamLookupProfileCfgSet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tcamProfileId,
    IN   CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
);
typedef struct{
    CPSS_DXCH_TCAM_RULE_SIZE_ENT subKey0_3_Sizes[4];
    **** Mux Table Line Indexes for parallel lookups 1-3 (APPLICABLE RANGES: 0..47)
        0-value means that key will be built without muxing *****
    GT_U32 subKey1_3_MuxTableLineIndexes[3];
    ***** Mux Table Line Offsets for parallel lookups 1-3 (APPLICABLE RANGES: 0..5) in 10-byte resolution *****
    GT_U32 subKey1_3_MuxTableLineOffsets[3];

} CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC;

*/
UTF_TEST_CASE_MAC(cpssDxChTcamLookupProfileCfgSet)
{
    GT_STATUS                             st              = GT_OK;
    GT_U8                                 devNum          = 0;
    GT_U32                                tcamProfileId;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT          subKeyPattern;
    CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC tcamLookupProfileCfg;
    CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC tcamLookupProfileCfgGet;
    GT_U32                                keyMuxBanks;
    GT_U32                                index;
    GT_U32                                cfgWordSize;

    cfgWordSize = sizeof(CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC) / 4;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (tcamProfileId = 1; (tcamProfileId < 64); tcamProfileId++)
        {
            for (subKeyPattern = (CPSS_DXCH_TCAM_RULE_SIZE_ENT)0;
                  (subKeyPattern <= CPSS_DXCH_TCAM_RULE_SIZE_80_B_E);
                  subKeyPattern++)
            {
                tcamLookupProfileCfg.subKey0_3_Sizes[0] = subKeyPattern;
                tcamLookupProfileCfg.subKey0_3_Sizes[1] = ((subKeyPattern + 10) % (CPSS_DXCH_TCAM_RULE_SIZE_80_B_E + 1));
                tcamLookupProfileCfg.subKey0_3_Sizes[2] = ((subKeyPattern + 20) % (CPSS_DXCH_TCAM_RULE_SIZE_80_B_E + 1));
                tcamLookupProfileCfg.subKey0_3_Sizes[3] = ((subKeyPattern + 30) % (CPSS_DXCH_TCAM_RULE_SIZE_80_B_E + 1));
                tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[0] = ((tcamProfileId + 10) % 48);
                tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[1] = ((tcamProfileId + 20) % 48);
                tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[2] = ((tcamProfileId + 30) % 48);
                keyMuxBanks =
                    (tcamLookupProfileCfg.subKey0_3_Sizes[1] >= CPSS_DXCH_TCAM_RULE_SIZE_60_B_E)
                    ? 6 : ((GT_U32)tcamLookupProfileCfg.subKey0_3_Sizes[1] + 1);
                tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[0] = (6 - keyMuxBanks);
                keyMuxBanks =
                    (tcamLookupProfileCfg.subKey0_3_Sizes[2] >= CPSS_DXCH_TCAM_RULE_SIZE_60_B_E)
                    ? 6 : ((GT_U32)tcamLookupProfileCfg.subKey0_3_Sizes[2] + 1);
                tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[1] = (6 - keyMuxBanks);
                keyMuxBanks =
                    (tcamLookupProfileCfg.subKey0_3_Sizes[3] >= CPSS_DXCH_TCAM_RULE_SIZE_60_B_E)
                    ? 6 : ((GT_U32)tcamLookupProfileCfg.subKey0_3_Sizes[3] + 1);
                tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[2] = (6 - keyMuxBanks);
                /*
                    1.1.1. Call API.
                    Expected: GT_OK.
                */

                st = cpssDxChTcamLookupProfileCfgSet(
                    devNum, tcamProfileId, &tcamLookupProfileCfg);
                UTF_VERIFY_EQUAL3_PARAM_MAC(
                    GT_OK, st, devNum, tcamProfileId, subKeyPattern);
                if (st == GT_OK)
                {
                    st = cpssDxChTcamLookupProfileCfgGet(
                        devNum, tcamProfileId, &tcamLookupProfileCfgGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(
                        GT_OK, st, devNum, tcamProfileId, subKeyPattern);
                    for (index = 0; (index < cfgWordSize); index++)
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            ((GT_U32*)&tcamLookupProfileCfg)[index],
                            ((GT_U32*)&tcamLookupProfileCfgGet)[index],
                            devNum, tcamProfileId, subKeyPattern, index);
                    }
               }
            }
        }

        tcamProfileId = 1;
        tcamLookupProfileCfg.subKey0_3_Sizes[0] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
        tcamLookupProfileCfg.subKey0_3_Sizes[1] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
        tcamLookupProfileCfg.subKey0_3_Sizes[2] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
        tcamLookupProfileCfg.subKey0_3_Sizes[3] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[0] = 0;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[1] = 0;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[2] = 0;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[0] = 0;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[1] = 0;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[2] = 0;

        st = cpssDxChTcamLookupProfileCfgSet(
            devNum, tcamProfileId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        tcamProfileId = 64;
        st = cpssDxChTcamLookupProfileCfgSet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        tcamProfileId = 1;
        tcamLookupProfileCfg.subKey0_3_Sizes[0] = (CPSS_DXCH_TCAM_RULE_SIZE_ENT)100;
        tcamProfileId = 64;
        st = cpssDxChTcamLookupProfileCfgSet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        tcamLookupProfileCfg.subKey0_3_Sizes[0] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[0] = 48;
        st = cpssDxChTcamLookupProfileCfgSet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[0] = 0;
        tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[0] = 6;
        st = cpssDxChTcamLookupProfileCfgSet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[0] = 0;
    }

    tcamProfileId = 1;
    tcamLookupProfileCfg.subKey0_3_Sizes[0] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
    tcamLookupProfileCfg.subKey0_3_Sizes[1] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
    tcamLookupProfileCfg.subKey0_3_Sizes[2] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
    tcamLookupProfileCfg.subKey0_3_Sizes[3] = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
    tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[0] = 0;
    tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[1] = 0;
    tcamLookupProfileCfg.subKey1_3_MuxTableLineIndexes[2] = 0;
    tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[0] = 0;
    tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[1] = 0;
    tcamLookupProfileCfg.subKey1_3_MuxTableLineOffsets[2] = 0;

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTcamLookupProfileCfgSet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamLookupProfileCfgSet(
        devNum, tcamProfileId, &tcamLookupProfileCfg);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/*
GT_STATUS cpssDxChTcamLookupProfileCfgGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tcamProfileId,
    OUT  CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamLookupProfileCfgGet)
{
    GT_STATUS                             st              = GT_OK;
    GT_U8                                 devNum          = 0;
    GT_U32                                tcamProfileId;
    CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC tcamLookupProfileCfg;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (tcamProfileId = 1; (tcamProfileId < 64); tcamProfileId++)
        {
            st = cpssDxChTcamLookupProfileCfgGet(
                devNum, tcamProfileId, &tcamLookupProfileCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, tcamProfileId);
        }

        tcamProfileId = 1;
        st = cpssDxChTcamLookupProfileCfgGet(
            devNum, tcamProfileId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        tcamProfileId = 64;
        st = cpssDxChTcamLookupProfileCfgGet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
    }

    tcamProfileId = 1;

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTcamLookupProfileCfgGet(
            devNum, tcamProfileId, &tcamLookupProfileCfg);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamLookupProfileCfgGet(
        devNum, tcamProfileId, &tcamLookupProfileCfg);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/*
GT_STATUS cpssDxChTcamLookupMuxTableLineSet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        entryIndex,
    IN   CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
);
typedef struct{

    ** subKey 2-byte-units offsets to copy from super-key
     *  subKey first 60 bytes (or less if size less from 60 bytes)
     *  built by copying up to 30 2-byte units from super key.
     *  The offsets of the copyed units also specified in 2-byte resolution.
     *  (APPLICABLE RANGES: 0..29) *
    GT_U8 superKeyTwoByteUnitsOffsets[30];

} CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC;
*/
UTF_TEST_CASE_MAC(cpssDxChTcamLookupMuxTableLineSet)
{
    GT_STATUS                                    st              = GT_OK;
    GT_U8                                        devNum          = 0;
    GT_U32                                       entryIndex;
    GT_U32                                       index;
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC tcamLookupMuxTableLineCfg;
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC tcamLookupMuxTableLineCfgGet;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (entryIndex = 1; (entryIndex < 48); entryIndex++)
        {
            for (index = 0; (index < 30); index++)
            {
                tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[index] = ((entryIndex + index) % 30);
            }
            st = cpssDxChTcamLookupMuxTableLineSet(
                devNum, entryIndex, &tcamLookupMuxTableLineCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
            st = cpssDxChTcamLookupMuxTableLineGet(
                devNum, entryIndex, &tcamLookupMuxTableLineCfgGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
            for (index = 0; (index < 30); index++)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(
                    tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[index],
                    tcamLookupMuxTableLineCfgGet.superKeyTwoByteUnitsOffsets[index],
                    devNum, entryIndex, index);
            }
        }

        entryIndex = 1;
        for (index = 0; (index < 30); index++)
        {
            tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[index] = 0;
        }
        st = cpssDxChTcamLookupMuxTableLineSet(
            devNum, entryIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, entryIndex);

        entryIndex = 48;
        st = cpssDxChTcamLookupMuxTableLineSet(
            devNum, entryIndex, &tcamLookupMuxTableLineCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, entryIndex);
    }

    entryIndex = 1;
    for (index = 0; (index < 30); index++)
    {
        tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[index] = 0;
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTcamLookupMuxTableLineSet(
            devNum, entryIndex, &tcamLookupMuxTableLineCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, entryIndex);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamLookupMuxTableLineSet(
        devNum, entryIndex, &tcamLookupMuxTableLineCfg);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, entryIndex);
}

/*
GT_STATUS cpssDxChTcamLookupMuxTableLineGet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        entryIndex,
    OUT  CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTcamLookupMuxTableLineGet)
{
    GT_STATUS                                    st              = GT_OK;
    GT_U8                                        devNum          = 0;
    GT_U32                                       entryIndex;
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC tcamLookupMuxTableLineCfg;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (entryIndex = 1; (entryIndex < 48); entryIndex++)
        {
            st = cpssDxChTcamLookupMuxTableLineGet(
                devNum, entryIndex, &tcamLookupMuxTableLineCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, entryIndex);
        }

        entryIndex = 1;
        st = cpssDxChTcamLookupMuxTableLineGet(
            devNum, entryIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, entryIndex);

        entryIndex = 48;
        st = cpssDxChTcamLookupMuxTableLineGet(
            devNum, entryIndex, &tcamLookupMuxTableLineCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, entryIndex);
    }

    entryIndex = 1;

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChTcamLookupMuxTableLineGet(
            devNum, entryIndex, &tcamLookupMuxTableLineCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, entryIndex);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTcamLookupMuxTableLineGet(
        devNum, entryIndex, &tcamLookupMuxTableLineCfg);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, entryIndex);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTcam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTcam)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupRuleWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupRuleRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupRuleValidStatusSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupRuleValidStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupClientGroupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupClientGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamIndexRangeHitNumAndGroupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamIndexRangeHitNumAndGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamActiveFloorsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamActiveFloorsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupCpuLookupTriggerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamPortGroupCpuLookupTriggerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamCpuLookupResultsGet)
#if 0
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamClearPerClient)
#endif
/*#ifndef ASIC_SIMULATION*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamWriteParityDaemonTest)
/*#endif*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamLookupProfileCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamLookupProfileCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamLookupMuxTableLineSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTcamLookupMuxTableLineGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTcam)

