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
* @file cpssDxChStreamUT.c
*
* @brief Unit tests for cpssDxChStream.h
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/stream/cpssDxChStream.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*
GT_STATUS cpssDxChStreamSngEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSngEnableSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_BOOL                                     enable;
    GT_BOOL                                     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(enable = GT_FALSE ; enable <= GT_TRUE ; enable ++)
        {
            st = cpssDxChStreamSngEnableSet(dev, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            st = cpssDxChStreamSngEnableGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable,enableGet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSngEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSngEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSngEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSngEnableGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_BOOL                                     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChStreamSngEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSngEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSngEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSngEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChStreamIrfEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamIrfEnableSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_BOOL                                     enable;
    GT_BOOL                                     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(enable = GT_FALSE ; enable <= GT_TRUE ; enable ++)
        {
            st = cpssDxChStreamIrfEnableSet(dev, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            st = cpssDxChStreamIrfEnableGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable,enableGet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamIrfEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamIrfEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamIrfEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamIrfEnableGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_BOOL                                     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChStreamIrfEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamIrfEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamIrfEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamIrfEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

#define NUM_SNG_ENTRIES_CNS _2K
#define NUM_SNG_STEP_CNS    ((NUM_SNG_ENTRIES_CNS/61) | 1)
#define NUM_SRF_ENTRIES_CNS _1K
#define NUM_SRF_STEP_CNS    ((NUM_SRF_ENTRIES_CNS/61) | 1)
#define NUM_SRF_HIST_ENTRIES_CNS _1K
#define NUM_SRF_HIST_STEP_CNS    ((NUM_SRF_HIST_ENTRIES_CNS/61) | 1)
/*
GT_STATUS cpssDxChStreamEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    IN  CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          sngIrfInfo;
    CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          sngIrfInfoGet;
    CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      srfMappingInfo;
    CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      srfMappingInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&sngIrfInfo,0,sizeof(sngIrfInfo));
        cpssOsMemSet(&srfMappingInfo,0,sizeof(srfMappingInfo));

        for(index = 0 ; index < NUM_SNG_ENTRIES_CNS ; index += NUM_SNG_STEP_CNS)
        {
            sngIrfInfo.seqNumAssignmentEnable = (GT_BOOL)((index) % 2);
            sngIrfInfo.resetSeqNum           = (GT_BOOL)((index >> 1) % 2);
            sngIrfInfo.currentSeqNum       = GT_NA;/* read only field */

            st = cpssDxChStreamEntrySet(dev, index , &sngIrfInfo,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            srfMappingInfo.srfEnable                = (GT_BOOL)((index+1) % 2);
            srfMappingInfo.isMulticast              = (GT_BOOL)(((index >> 1)+1) % 2);
            srfMappingInfo.srfIndex                 = (index + 100) % NUM_SRF_ENTRIES_CNS;
            srfMappingInfo.egressPortsBmp.ports[0]  = 0xFFFFFFFF - index;
            srfMappingInfo.egressPortsBmp.ports[1]  = index;

            st = cpssDxChStreamEntrySet(dev, index , NULL, &srfMappingInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChStreamEntrySet(dev, index , &sngIrfInfo, &srfMappingInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(index = 0 ; index < NUM_SNG_ENTRIES_CNS ; index += NUM_SNG_STEP_CNS)
        {
            sngIrfInfo.seqNumAssignmentEnable = (GT_BOOL)((index) % 2);
            sngIrfInfo.resetSeqNum           = (GT_BOOL)((index >> 1) % 2);
            sngIrfInfo.currentSeqNum       = GT_NA;/* read only field */

            st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,NULL,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(sngIrfInfo.seqNumAssignmentEnable ,sngIrfInfoGet.seqNumAssignmentEnable);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE/*auto reset*/            ,sngIrfInfoGet.resetSeqNum           );
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(sngIrfInfo.currentSeqNum      ,sngIrfInfoGet.currentSeqNum         );

            srfMappingInfo.srfEnable                = (GT_BOOL)((index+1) % 2);
            srfMappingInfo.isMulticast              = (GT_BOOL)(((index >> 1)+1) % 2);
            srfMappingInfo.srfIndex                 = (index + 100) % NUM_SRF_ENTRIES_CNS;
            srfMappingInfo.egressPortsBmp.ports[0]  = 0xFFFFFFFF - index;
            srfMappingInfo.egressPortsBmp.ports[1]  = index;

            st = cpssDxChStreamEntryGet(dev, index , NULL,NULL,&srfMappingInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(srfMappingInfo.srfEnable               ,srfMappingInfoGet.srfEnable              );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfMappingInfo.isMulticast             ,srfMappingInfoGet.isMulticast            );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfMappingInfo.srfIndex                ,srfMappingInfoGet.srfIndex               );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfMappingInfo.egressPortsBmp.ports[0] ,srfMappingInfoGet.egressPortsBmp.ports[0]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfMappingInfo.egressPortsBmp.ports[1] ,srfMappingInfoGet.egressPortsBmp.ports[1]);
        }

        cpssOsMemSet(&sngIrfInfo,0,sizeof(sngIrfInfo));
        cpssOsMemSet(&srfMappingInfo,0,sizeof(srfMappingInfo));

        index = 0;

        /* support NULL pointer on one param */
        st = cpssDxChStreamEntrySet(dev, index , &sngIrfInfo,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* support NULL pointer on one param */
        st = cpssDxChStreamEntrySet(dev, index , NULL,&srfMappingInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check bad params */
        srfMappingInfo.srfIndex = NUM_SRF_ENTRIES_CNS;
        st = cpssDxChStreamEntrySet(dev, index , NULL,&srfMappingInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfMappingInfo.srfIndex = 0;  /*restore value to 'good one' */

        index = NUM_SNG_ENTRIES_CNS;
        st = cpssDxChStreamEntrySet(dev, index , &sngIrfInfo,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        st = cpssDxChStreamEntrySet(dev, index , NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    index = 0;
    cpssOsMemSet(&sngIrfInfo,0,sizeof(sngIrfInfo));
    cpssOsMemSet(&srfMappingInfo,0,sizeof(srfMappingInfo));

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamEntrySet(dev, index , &sngIrfInfo,&srfMappingInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEntrySet(dev, index , &sngIrfInfo,&srfMappingInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChStreamSngEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_STREAM_SNG_ENTRY_STC       *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          sngIrfInfoGet;
    CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC     irfCountersGet;
    CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      srfMappingInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < NUM_SNG_ENTRIES_CNS ; index += NUM_SNG_STEP_CNS)
        {
            st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,&irfCountersGet,&srfMappingInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* support NULL pointer on 2 params */
            st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,NULL,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* support NULL pointer on 2 params */
            st = cpssDxChStreamEntryGet(dev, index , NULL,&irfCountersGet,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* support NULL pointer on 2 params */
            st = cpssDxChStreamEntryGet(dev, index , NULL,NULL,&srfMappingInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* support NULL pointer on 1 param */
            st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,&irfCountersGet,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* support NULL pointer on 1 param */
            st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,NULL,&srfMappingInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* support NULL pointer on 1 param */
            st = cpssDxChStreamEntryGet(dev, index , NULL,&irfCountersGet,&srfMappingInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChStreamEntryGet(dev, index , NULL,NULL,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        index = NUM_SNG_ENTRIES_CNS;
        st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    index = 0;
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev , (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,&irfCountersGet,&srfMappingInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEntryGet(dev, index , &sngIrfInfoGet,&irfCountersGet,&srfMappingInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT     type,
    IN CPSS_PACKET_CMD_ENT                      command
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamExceptionCommandSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii,jj;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        type;
    CPSS_PACKET_CMD_ENT                         command;
    CPSS_PACKET_CMD_ENT                         commandGet;
    CPSS_PACKET_CMD_ENT                         commandGet_lastOk = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E
    };
    CPSS_PACKET_CMD_ENT                         ok_commandArr[5] =
    {
         CPSS_PACKET_CMD_FORWARD_E
        ,CPSS_PACKET_CMD_MIRROR_TO_CPU_E
        ,CPSS_PACKET_CMD_TRAP_TO_CPU_E
        ,CPSS_PACKET_CMD_DROP_HARD_E
        ,CPSS_PACKET_CMD_DROP_SOFT_E
    };
    CPSS_PACKET_CMD_ENT                         not_ok_commandArr[7] =
    {
         CPSS_PACKET_CMD_ROUTE_E
        ,CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E
        ,CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E
        ,CPSS_PACKET_CMD_BRIDGE_E
        ,CPSS_PACKET_CMD_NONE_E
        ,CPSS_PACKET_CMD_LOOPBACK_E
        ,CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 4 ; jj++)
        {
            type = typesArr[jj];

            /* test GT_OK cases */
            for(ii = 0 ; ii < 5 ; ii ++)
            {
                command = ok_commandArr[ii];
                st = cpssDxChStreamExceptionCommandSet(dev, type ,command);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChStreamExceptionCommandGet(dev, type ,&commandGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(command,commandGet);

                commandGet_lastOk = commandGet;
            }

            /* test GT_BAD_PARAM cases */
            for(ii = 0 ; ii < 7 ; ii ++)
            {
                command = not_ok_commandArr[ii];
                st = cpssDxChStreamExceptionCommandSet(dev, type ,command);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

                st = cpssDxChStreamExceptionCommandGet(dev, type ,&commandGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(commandGet_lastOk,commandGet);
            }
        }

        type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
        command = CPSS_PACKET_CMD_FORWARD_E;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamExceptionCommandSet
                            (dev, type ,command),
                            type);

        type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
        command = CPSS_PACKET_CMD_FORWARD_E;

        /* test GT_BAD_PARAM cases on for param : command */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamExceptionCommandSet
                            (dev, type ,command),
                            command);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
    command = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamExceptionCommandSet(dev, type ,command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamExceptionCommandSet(dev, type ,command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamExceptionCommandGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        type;
    CPSS_PACKET_CMD_ENT                         commandGet;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 4 ; jj++)
        {
            type = typesArr[jj];

            st = cpssDxChStreamExceptionCommandGet(dev, type ,&commandGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChStreamExceptionCommandGet(dev, type , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamExceptionCommandGet
                            (dev, type ,&commandGet),
                            type);
    }

    type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamExceptionCommandGet(dev, type ,&commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamExceptionCommandGet(dev, type ,&commandGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT     type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamExceptionCpuCodeSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        type;
    CPSS_NET_RX_CPU_CODE_ENT                    cpuCode;
    CPSS_NET_RX_CPU_CODE_ENT                    cpuCodeGet;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 4 ; jj++)
        {
            type = typesArr[jj];

            /* test maybe GT_OK , maybe 'bad_param' cases */
            for(cpuCode = 0 ;
                cpuCode < CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
                cpuCode ++)
            {
                st = cpssDxChStreamExceptionCpuCodeSet(dev, type ,cpuCode);
                if(st == GT_OK)
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    st = cpssDxChStreamExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(cpuCode,cpuCodeGet);
                }
            }

            /* test GT_OK cases */
            for(cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
                cpuCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
                cpuCode ++)
            {
                st = cpssDxChStreamExceptionCpuCodeSet(dev, type ,cpuCode);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChStreamExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                if(cpuCodeGet != cpuCode &&
                   cpuCodeGet >= CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E)
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(cpuCode,cpuCodeGet);
                }
            }
        }

        type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
        cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamExceptionCpuCodeSet
                            (dev, type ,cpuCode),
                            type);

        type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
        cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

        /* test GT_BAD_PARAM cases on enum for param : cpuCode */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamExceptionCpuCodeSet
                            (dev, type ,cpuCode),
                            cpuCode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
    cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamExceptionCpuCodeSet(dev, type ,cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamExceptionCpuCodeSet(dev, type ,cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamExceptionCpuCodeGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        type;
    CPSS_NET_RX_CPU_CODE_ENT                    cpuCodeGet;
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E
        ,CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 4 ; jj++)
        {
            type = typesArr[jj];

            st = cpssDxChStreamExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChStreamExceptionCpuCodeGet(dev, type , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;
        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamExceptionCpuCodeGet
                            (dev, type ,&cpuCodeGet),
                            type);
    }

    type    = CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamCountingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       irfMode,
    IN  GT_BOOL                                 srfTaglessEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamCountingModeSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii,jj;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT           irfMode;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT           irfModeGet;
    GT_BOOL                                     srfTaglessEnable;
    GT_BOOL                                     srfTaglessEnableGet;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT           irfModeArr[2] =
    {
         CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E
        ,CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(ii = 0 ; ii < 2 ; ii++)
        {
            srfTaglessEnable =  ii ? GT_TRUE : GT_FALSE;

            for(jj = 0 ; jj < 2 ; jj++)
            {
                irfMode = irfModeArr[jj];
                srfTaglessEnable = GT_TRUE;

                st = cpssDxChStreamCountingModeSet(dev, irfMode ,srfTaglessEnable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChStreamCountingModeGet(dev, &irfModeGet ,&srfTaglessEnableGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(irfMode,irfModeGet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(srfTaglessEnable,srfTaglessEnableGet);
            }
        }

        irfMode    = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E;
        srfTaglessEnable = GT_FALSE;
        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChStreamCountingModeSet
                            (dev, irfMode ,srfTaglessEnable),
                            irfMode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    irfMode    = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E;
    srfTaglessEnable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamCountingModeSet(dev, irfMode ,srfTaglessEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamCountingModeSet(dev, irfMode ,srfTaglessEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamCountingModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       *irfModePtr,
    OUT GT_BOOL                                 *srfTaglessEnablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamCountingModeGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT           irfModeGet;
    GT_BOOL                                     srfTaglessEnableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChStreamCountingModeGet(dev, &irfModeGet ,&srfTaglessEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamCountingModeGet(dev, &irfModeGet , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamCountingModeGet(dev, NULL , &srfTaglessEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamCountingModeGet(dev, NULL , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamCountingModeGet(dev, &irfModeGet ,&srfTaglessEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamCountingModeGet(dev, &irfModeGet ,&srfTaglessEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChStreamSrfDaemonInfoSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfDaemonInfoSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC        info;
    CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC        infoGet;
    GT_BOOL                                     bool_1,bool_2,bool_3,bool_4;
    GT_U32                                      time_leDetectionArr[4];/*InMicroSec*/
    GT_U32                                      time_restartArr[4];/*InMicroSec*/
    GT_U32                                      ii,jj;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    cpssOsMemSet(&info,0,sizeof(info));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        time_restartArr[0] = (BIT_23-1) / PRV_CPSS_PP_MAC(dev)->coreClock;/* max allowed */
        time_restartArr[1] = time_restartArr[0] / 2;
        time_restartArr[2] = time_restartArr[0] / 4;
        time_restartArr[3] = GT_NA;

        time_leDetectionArr[0] = (BIT_26-1) / PRV_CPSS_PP_MAC(dev)->coreClock ;/* max allowed */
        time_leDetectionArr[1] = time_leDetectionArr[0] / 2;
        time_leDetectionArr[2] = time_leDetectionArr[0] / 4;
        time_leDetectionArr[3] = GT_NA;

        for(bool_1 = GT_FALSE ; bool_1 <= GT_TRUE ; bool_1 ++)
        {
            info.restartDaemonEnable = bool_1;
            for(bool_2 = GT_FALSE ; bool_2 <= GT_TRUE ; bool_2 ++)
            {
                info.latentErrorDaemonEnable = bool_2;
                for(bool_3 = GT_FALSE ; bool_3 <= GT_TRUE ; bool_3 ++)
                {
                    info.latentErrorDetectionAlgorithmEnable = bool_3;

                    for(bool_4 = GT_FALSE ; bool_4 <= GT_TRUE ; bool_4 ++)
                    {
                        info.latentErrorDetectionResetEnable = bool_4;

                        for(ii = 0 ; time_restartArr[ii] != GT_NA ; ii++)
                        {
                            info.restartPeriodTimeInMicroSec = time_restartArr[ii];

                            for(jj = 0 ; time_leDetectionArr[jj] != GT_NA ; jj++)
                            {
                                info.latentErrorDetectionPeriodTimeInMicroSec = time_leDetectionArr[jj];

                                st = cpssDxChStreamSrfDaemonInfoSet(dev, &info);
                                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                                st = cpssDxChStreamSrfDaemonInfoGet(dev, &infoGet);
                                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                                UTF_VERIFY_EQUAL0_PARAM_MAC(info.restartDaemonEnable                     ,infoGet.restartDaemonEnable                     );
                                UTF_VERIFY_EQUAL0_PARAM_MAC(info.latentErrorDaemonEnable                 ,infoGet.latentErrorDaemonEnable                 );
                                UTF_VERIFY_EQUAL0_PARAM_MAC(info.restartPeriodTimeInMicroSec             ,infoGet.restartPeriodTimeInMicroSec             );
                                UTF_VERIFY_EQUAL0_PARAM_MAC(info.latentErrorDetectionPeriodTimeInMicroSec,infoGet.latentErrorDetectionPeriodTimeInMicroSec);
                                UTF_VERIFY_EQUAL0_PARAM_MAC(info.latentErrorDetectionAlgorithmEnable     ,infoGet.latentErrorDetectionAlgorithmEnable     );
                                UTF_VERIFY_EQUAL0_PARAM_MAC(info.latentErrorDetectionResetEnable         ,infoGet.latentErrorDetectionResetEnable         );
                            }/*jj*/
                        }/*ii*/
                    }/*bool_4*/
                }/*bool_3*/
            }/*bool_2*/
        }/*bool_1*/


        cpssOsMemSet(&info,0,sizeof(info));

        info.latentErrorDetectionPeriodTimeInMicroSec = time_leDetectionArr[0] + 1;
        st = cpssDxChStreamSrfDaemonInfoSet(dev, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        info.latentErrorDetectionPeriodTimeInMicroSec = time_leDetectionArr[0];

        info.restartPeriodTimeInMicroSec              = time_restartArr[0] + 1;
        st = cpssDxChStreamSrfDaemonInfoSet(dev, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        info.restartPeriodTimeInMicroSec              = time_restartArr[0];

        st = cpssDxChStreamSrfDaemonInfoSet(dev, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfDaemonInfoSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    cpssOsMemSet(&info,0,sizeof(info));

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfDaemonInfoSet(dev, &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfDaemonInfoSet(dev, &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChStreamSrfDaemonInfoGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfDaemonInfoGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC        infoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChStreamSrfDaemonInfoGet(dev, &infoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfDaemonInfoGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfDaemonInfoGet(dev, &infoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfDaemonInfoGet(dev, &infoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamAgingGroupSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          agingGroupBmp
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamAgingGroupSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      startIndex;
    GT_U32                                      agingGroupBmp;
    GT_U32                                      agingGroupBmpGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(startIndex = 0 ; startIndex < NUM_SNG_ENTRIES_CNS ; startIndex +=32)
        {
            agingGroupBmp = 0xFFFFFFFF - startIndex;

            st = cpssDxChStreamAgingGroupSet(dev, startIndex ,agingGroupBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }


        for(startIndex = 0 ; startIndex < NUM_SNG_ENTRIES_CNS ; startIndex +=32)
        {
            agingGroupBmp = 0xFFFFFFFF - startIndex;

            st = cpssDxChStreamAgingGroupGet(dev, startIndex ,&agingGroupBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(agingGroupBmp,agingGroupBmpGet);
        }

        agingGroupBmp = 0;
        for(startIndex = 0 ; startIndex < NUM_SNG_ENTRIES_CNS ; startIndex += NUM_SNG_STEP_CNS)
        {
            if(0 == (startIndex & 0x1f))
            {
                continue;
            }
            st = cpssDxChStreamAgingGroupSet(dev, startIndex ,agingGroupBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        st = cpssDxChStreamAgingGroupSet(dev, NUM_SNG_ENTRIES_CNS ,agingGroupBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    startIndex    = 0;
    agingGroupBmp = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamAgingGroupSet(dev, startIndex ,agingGroupBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamAgingGroupSet(dev, startIndex ,agingGroupBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamAgingGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *agingGroupBmpPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamAgingGroupGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      startIndex;
    GT_U32                                      agingGroupBmpGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(startIndex = 0 ; startIndex < NUM_SNG_ENTRIES_CNS ; startIndex +=32)
        {
            st = cpssDxChStreamAgingGroupGet(dev, startIndex , &agingGroupBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(startIndex = 0 ; startIndex < NUM_SNG_ENTRIES_CNS ; startIndex += NUM_SNG_STEP_CNS)
        {
            if(0 == (startIndex & 0x1f))
            {
                continue;
            }
            st = cpssDxChStreamAgingGroupGet(dev, startIndex ,&agingGroupBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        st = cpssDxChStreamAgingGroupGet(dev, NUM_SNG_ENTRIES_CNS ,&agingGroupBmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        startIndex = 0;
        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamAgingGroupGet(dev, startIndex , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    startIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamAgingGroupGet(dev, startIndex ,&agingGroupBmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamAgingGroupGet(dev, startIndex ,&agingGroupBmpGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       srfConfigInfo;
    CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       srfConfigInfoGet;
    CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC     srfCountersGet;
    CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC       srfStatusInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < NUM_SRF_ENTRIES_CNS ; index += NUM_SRF_STEP_CNS)
        {
            srfConfigInfo.useVectorRecoveryAlgorithm  = (GT_BOOL)((index) % 2);
            srfConfigInfo.vectorSequenceHistoryLength = (NUM_SRF_ENTRIES_CNS - index) * 2;
            srfConfigInfo.takeNoSeqNum                = (GT_BOOL)((index >> 1) % 2);
            srfConfigInfo.takeAnySeqNum               = (GT_BOOL)((index >> 2) % 2);
            srfConfigInfo.firstHistoryBuffer          = (index + 100) % _1K;
            srfConfigInfo.numOfBuffers                = 1 + (index % 16);
            srfConfigInfo.restartThreshold            = BIT_16 - 1 - index;

            srfConfigInfo.leEnable                    = (GT_BOOL)((index >> 3) % 2);
            srfConfigInfo.leResetTimeEnabled          = (GT_BOOL)((index >> 4) % 2);
            srfConfigInfo.leResetTimeInSeconds        = (index >> 4) % 32;

            st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(index = 0 ; index < NUM_SRF_ENTRIES_CNS ; index += NUM_SRF_STEP_CNS)
        {
            srfConfigInfo.useVectorRecoveryAlgorithm  = (GT_BOOL)((index) % 2);
            srfConfigInfo.vectorSequenceHistoryLength = (NUM_SRF_ENTRIES_CNS - index) * 2;
            srfConfigInfo.takeNoSeqNum                = (GT_BOOL)((index >> 1) % 2);
            srfConfigInfo.takeAnySeqNum               = (GT_BOOL)((index >> 2) % 2);
            srfConfigInfo.firstHistoryBuffer          = (index + 100) % _1K;
            srfConfigInfo.numOfBuffers                = 1 + (index % 16);
            srfConfigInfo.restartThreshold            = BIT_16 - 1 - index;
            srfConfigInfo.leEnable                    = (GT_BOOL)((index >> 3) % 2);
            srfConfigInfo.leResetTimeEnabled          = (GT_BOOL)((index >> 4) % 2);
            srfConfigInfo.leResetTimeInSeconds        = (index >> 4) % 32;

            st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,&srfCountersGet,&srfStatusInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.useVectorRecoveryAlgorithm   ,srfConfigInfoGet.useVectorRecoveryAlgorithm  );
            if(srfConfigInfo.useVectorRecoveryAlgorithm == GT_TRUE)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.vectorSequenceHistoryLength  ,srfConfigInfoGet.vectorSequenceHistoryLength );
            }
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.takeNoSeqNum                 ,srfConfigInfoGet.takeNoSeqNum                );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.takeAnySeqNum                ,srfConfigInfoGet.takeAnySeqNum               );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.firstHistoryBuffer           ,srfConfigInfoGet.firstHistoryBuffer          );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.numOfBuffers                 ,srfConfigInfoGet.numOfBuffers                );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.restartThreshold             ,srfConfigInfoGet.restartThreshold);
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.leEnable                     ,srfConfigInfoGet.leEnable                    );
            UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.leResetTimeEnabled           ,srfConfigInfoGet.leResetTimeEnabled          );
            if(srfConfigInfo.leResetTimeEnabled == GT_TRUE)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(srfConfigInfo.leResetTimeInSeconds         ,srfConfigInfoGet.leResetTimeInSeconds        );
            }
        }

        cpssOsMemSet(&srfConfigInfo,0,sizeof(srfConfigInfo));
        index = 0;
        srfConfigInfo.numOfBuffers  = 1;/*can't be 0*/
        srfConfigInfo.leResetTimeInSeconds = 1;/*can't be 0*/

        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check bad params */
        srfConfigInfo.useVectorRecoveryAlgorithm  = GT_TRUE;
        srfConfigInfo.vectorSequenceHistoryLength = _2K+1;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        srfConfigInfo.useVectorRecoveryAlgorithm  = GT_FALSE;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        srfConfigInfo.useVectorRecoveryAlgorithm  = GT_TRUE;
        srfConfigInfo.vectorSequenceHistoryLength = 2;
        srfConfigInfo.firstHistoryBuffer = _1K;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfConfigInfo.firstHistoryBuffer = 0;

        srfConfigInfo.numOfBuffers = 0;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfConfigInfo.numOfBuffers = 1;

        srfConfigInfo.numOfBuffers = 17;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfConfigInfo.numOfBuffers = 1;

        srfConfigInfo.restartThreshold = BIT_16;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfConfigInfo.restartThreshold = 0;

        srfConfigInfo.leResetTimeEnabled = GT_TRUE;
        srfConfigInfo.leResetTimeInSeconds = 0;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfConfigInfo.leResetTimeInSeconds = 1;

        srfConfigInfo.leResetTimeInSeconds = 32;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        srfConfigInfo.leResetTimeInSeconds = 1;

        srfConfigInfo.leResetTimeEnabled = GT_FALSE;
        srfConfigInfo.leResetTimeInSeconds = 0;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        srfConfigInfo.leResetTimeInSeconds = 1;

        srfConfigInfo.leResetTimeInSeconds = 32;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        srfConfigInfo.leResetTimeInSeconds = 1;

        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check bad index */
        index = NUM_SRF_ENTRIES_CNS;
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* check NULL pointer */
        index = 0;
        st = cpssDxChStreamSrfEntrySet(dev, index , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    index = 0;
    cpssOsMemSet(&srfConfigInfo,0,sizeof(srfConfigInfo));

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfEntrySet(dev, index , &srfConfigInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr,
    OUT CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC     *srfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC       *srfStatusInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       srfConfigInfoGet;
    CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC     srfCountersGet;
    CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC       srfStatusInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < NUM_SRF_ENTRIES_CNS ; index += NUM_SRF_STEP_CNS)
        {
            st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,&srfCountersGet,&srfStatusInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        index = 0;

        /* support NULL pointer on 2 params */
        st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* support NULL pointer on 2 params */
        st = cpssDxChStreamSrfEntryGet(dev, index , NULL,&srfCountersGet,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* support NULL pointer on 2 params */
        st = cpssDxChStreamSrfEntryGet(dev, index , NULL,NULL,&srfStatusInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* support NULL pointer on 1 param */
        st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,&srfCountersGet,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* support NULL pointer on 1 param */
        st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,NULL,&srfStatusInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* support NULL pointer on 1 param */
        st = cpssDxChStreamSrfEntryGet(dev, index , NULL,&srfCountersGet,&srfStatusInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfEntryGet(dev, index , NULL,NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        index = NUM_SRF_ENTRIES_CNS;
        st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    }

    index = 0;
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev , (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,&srfCountersGet,&srfStatusInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfEntryGet(dev, index , &srfConfigInfoGet,&srfCountersGet,&srfStatusInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfLatentErrorDetectedGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *errorGroupBmpPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfLatentErrorDetectedGroupGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      startIndex;
    GT_U32                                      errorGroupBmpGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(startIndex = 0 ; startIndex < NUM_SRF_ENTRIES_CNS ; startIndex +=32)
        {
            st = cpssDxChStreamSrfLatentErrorDetectedGroupGet(dev, startIndex , &errorGroupBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(startIndex = 0 ; startIndex < NUM_SRF_ENTRIES_CNS ; startIndex += NUM_SRF_STEP_CNS)
        {
            if(0 == (startIndex & 0x1f))
            {
                continue;
            }
            st = cpssDxChStreamSrfLatentErrorDetectedGroupGet(dev, startIndex ,&errorGroupBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        st = cpssDxChStreamSrfLatentErrorDetectedGroupGet(dev, NUM_SRF_ENTRIES_CNS ,&errorGroupBmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        startIndex = 0;
        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfLatentErrorDetectedGroupGet(dev, startIndex , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    startIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfLatentErrorDetectedGroupGet(dev, startIndex ,&errorGroupBmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfLatentErrorDetectedGroupGet(dev, startIndex ,&errorGroupBmpGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfHistoryBufferEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfHistoryBufferEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC histBuff;
    CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC histBuffGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < NUM_SRF_HIST_ENTRIES_CNS ; index += NUM_SRF_HIST_STEP_CNS)
        {
            histBuff.historyBuffer[0] = index << 15 | index;
            histBuff.historyBuffer[1] = 0xFFFFFFFF - histBuff.historyBuffer[0];
            histBuff.historyBuffer[2] = 0xAA55AA55 - histBuff.historyBuffer[0];
            histBuff.historyBuffer[3] = 0xAA55AA55 + histBuff.historyBuffer[0];

            st = cpssDxChStreamSrfHistoryBufferEntrySet(dev, index ,&histBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(index = 0 ; index < NUM_SRF_HIST_ENTRIES_CNS ; index += NUM_SRF_HIST_STEP_CNS)
        {
            histBuff.historyBuffer[0] = index << 15 | index;
            histBuff.historyBuffer[1] = 0xFFFFFFFF - histBuff.historyBuffer[0];
            histBuff.historyBuffer[2] = 0xAA55AA55 - histBuff.historyBuffer[0];
            histBuff.historyBuffer[3] = 0xAA55AA55 + histBuff.historyBuffer[0];

            st = cpssDxChStreamSrfHistoryBufferEntryGet(dev, index ,&histBuffGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(histBuff.historyBuffer[0] ,histBuffGet.historyBuffer[0]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(histBuff.historyBuffer[1] ,histBuffGet.historyBuffer[1]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(histBuff.historyBuffer[2] ,histBuffGet.historyBuffer[2]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(histBuff.historyBuffer[3] ,histBuffGet.historyBuffer[3]);
        }

        st = cpssDxChStreamSrfHistoryBufferEntrySet(dev, NUM_SRF_HIST_ENTRIES_CNS ,&histBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfHistoryBufferEntrySet(dev, index , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfHistoryBufferEntrySet(dev, index ,&histBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfHistoryBufferEntrySet(dev, index ,&histBuff);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfHistoryBufferEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfHistoryBufferEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC histBuffGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < NUM_SRF_HIST_ENTRIES_CNS ; index += NUM_SRF_HIST_STEP_CNS)
        {
            st = cpssDxChStreamSrfHistoryBufferEntryGet(dev, index ,&histBuffGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = cpssDxChStreamSrfHistoryBufferEntryGet(dev, NUM_SRF_HIST_ENTRIES_CNS ,&histBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfHistoryBufferEntryGet(dev, index , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfHistoryBufferEntryGet(dev, index ,&histBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfHistoryBufferEntryGet(dev, index ,&histBuffGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfZeroBitVectorGroupEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          vector
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfZeroBitVectorGroupEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      startIndex;
    GT_U32                                      vector;
    GT_U32                                      vectorGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(startIndex = 0 ; startIndex < NUM_SRF_HIST_ENTRIES_CNS ; startIndex +=32)
        {
            vector = 0xFFFFFFFF - startIndex;

            st = cpssDxChStreamSrfZeroBitVectorGroupEntrySet(dev, startIndex ,vector);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }


        for(startIndex = 0 ; startIndex < NUM_SRF_HIST_ENTRIES_CNS ; startIndex +=32)
        {
            vector = 0xFFFFFFFF - startIndex;

            st = cpssDxChStreamAgingGroupGet(dev, startIndex ,&vectorGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(vector,vectorGet);
        }

        vector = 0;
        for(startIndex = 0 ; startIndex < NUM_SRF_HIST_ENTRIES_CNS ; startIndex += NUM_SRF_HIST_STEP_CNS)
        {
            if(0 == (startIndex & 0x1f))
            {
                continue;
            }
            st = cpssDxChStreamSrfZeroBitVectorGroupEntrySet(dev, startIndex ,vector);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        st = cpssDxChStreamSrfZeroBitVectorGroupEntrySet(dev, NUM_SRF_HIST_ENTRIES_CNS ,vector);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    startIndex    = 0;
    vector = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfZeroBitVectorGroupEntrySet(dev, startIndex ,vector);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfZeroBitVectorGroupEntrySet(dev, startIndex ,vector);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfZeroBitVectorGroupEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *vectorPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfZeroBitVectorGroupEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      startIndex;
    GT_U32                                      vectorGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(startIndex = 0 ; startIndex < NUM_SRF_HIST_ENTRIES_CNS ; startIndex +=32)
        {
            st = cpssDxChStreamSrfZeroBitVectorGroupEntryGet(dev, startIndex , &vectorGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(startIndex = 0 ; startIndex < NUM_SRF_HIST_ENTRIES_CNS ; startIndex += NUM_SRF_HIST_STEP_CNS)
        {
            if(0 == (startIndex & 0x1f))
            {
                continue;
            }
            st = cpssDxChStreamSrfZeroBitVectorGroupEntryGet(dev, startIndex ,&vectorGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        st = cpssDxChStreamSrfZeroBitVectorGroupEntryGet(dev, NUM_SRF_HIST_ENTRIES_CNS ,&vectorGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        startIndex = 0;
        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSrfZeroBitVectorGroupEntryGet(dev, startIndex , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    startIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfZeroBitVectorGroupEntryGet(dev, startIndex ,&vectorGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfZeroBitVectorGroupEntryGet(dev, startIndex ,&vectorGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfTrgPortMappedMulticastIndexSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          index
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfTrgPortMappedMulticastIndexSet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    GT_U32                                      index;
    GT_U32                                      indexGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);
        st = prvUtfNextGenericPortReset(&portNum, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            index = portNum & (BIT_6-1);
            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, index);
        }

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            index = portNum & (BIT_6-1);
            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, index);
            UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                               "get another index than was set: %d", dev);
        }

        portNum = 0;
        index = (BIT_6-1);
        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, index);

        index = BIT_6;
        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, portNum, index);

        index = (BIT_6-1);

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        index = (BIT_6-1);

        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        index = (BIT_6-1);

        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    index = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(dev, portNum, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSrfTrgPortMappedMulticastIndexGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *indexPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSrfTrgPortMappedMulticastIndexGet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum  = 0;
    GT_U32                                      indexGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E);
        st = prvUtfNextGenericPortReset(&portNum, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable.
                Expected: GT_OK.
            */

            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, indexGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(dev, portNum, &indexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChStream suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChStream)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSngEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSngEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamIrfEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamIrfEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamExceptionCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamExceptionCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamExceptionCpuCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamCountingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamCountingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfDaemonInfoSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfDaemonInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamAgingGroupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamAgingGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfLatentErrorDetectedGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfHistoryBufferEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfHistoryBufferEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfZeroBitVectorGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfZeroBitVectorGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfTrgPortMappedMulticastIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSrfTrgPortMappedMulticastIndexGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChStream)

/**
* @internal cpssDxChStreamSngDump function
* @endinternal
*
* @brief   debug tool --- print Stream Sng 'active' instances
*                       (entries with currentSeqNum != 0x0)
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChStreamSngDump(
    IN GT_U8 devNum
)
{
    GT_STATUS                    rc;
    GT_U32                       index;
    CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC    info;
    GT_U32                       activeNum = 0;
    GT_BOOL                                enable;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    rc = cpssDxChStreamSngEnableGet(devNum,&enable);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("Global Stream Sng is [%s] in the device \n",
        enable == GT_TRUE ? "Enabled" : "Disabled");

    cpssOsPrintf("Stream SNG entry format : \n"
    "A_E    = seq Num Assignment Enable : 'E' (enable) , '-' (disabled) \n"
    "R      = reset Seq Num : 'R' ask for reset , '-' (disabled) \n"
    );

    cpssOsPrintf("Print only 'active' entries (counter != 0x0) \n\n");

/*
index |SeqNum |A_E |  R
      | HEX   |    |
========================
 0000 |  04d2 |  E |  R
 0001 |  2564 |  E |  -
*/
    cpssOsPrintf(
"index |SeqNum |A_E |  R \n"
"      | HEX   |    |    \n"
"========================\n"
    );

    for(index = 0 ; index < NUM_SNG_ENTRIES_CNS ; index++)
    {
        rc = cpssDxChStreamEntryGet(devNum,index,
            &info,NULL,NULL);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(info.currentSeqNum == 0x0)
        {
            continue;
        }

        activeNum++;

        cpssOsPrintf(" %4.4d | ",index);
        cpssOsPrintf(" %4.4x | ",
            info.currentSeqNum);
        cpssOsPrintf(" %s | ",
            info.seqNumAssignmentEnable == GT_TRUE ? "E" : "-");
        cpssOsPrintf(" %s ",
            info.resetSeqNum == GT_TRUE ? "R" : "-");

        cpssOsPrintf("\n");
    }

    if(activeNum == 0)
    {
        cpssOsPrintf("Finished Stream Sng table scan of [%d K] entries , there are no active entries \n",(index/_1K));
    }
    else
    {
        cpssOsPrintf("Finished Stream Sng table of [%d K] entries. there are [%d] active entries \n",(index/_1K), activeNum);
    }
    return GT_OK;
}

