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
* @file cpssDxChHsrPrpUT.c
*
* @brief Unit tests for cpssDxChHsrPrp.h
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/hsrPrp/cpssDxChHsrPrp.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*
GT_STATUS cpssDxChHsrPrpExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    IN CPSS_PACKET_CMD_ENT                      command
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpExceptionCommandSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii,jj;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        type;
    CPSS_PACKET_CMD_ENT                         command;
    CPSS_PACKET_CMD_ENT                         commandGet;
    CPSS_PACKET_CMD_ENT                         commandGet_lastOk = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E
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
                st = cpssDxChHsrPrpExceptionCommandSet(dev, type ,command);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChHsrPrpExceptionCommandGet(dev, type ,&commandGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(command,commandGet);

                commandGet_lastOk = commandGet;
            }

            /* test GT_BAD_PARAM cases */
            for(ii = 0 ; ii < 7 ; ii ++)
            {
                command = not_ok_commandArr[ii];
                st = cpssDxChHsrPrpExceptionCommandSet(dev, type ,command);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

                st = cpssDxChHsrPrpExceptionCommandGet(dev, type ,&commandGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(commandGet_lastOk,commandGet);
            }
        }

        type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        command = CPSS_PACKET_CMD_FORWARD_E;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCommandSet
                            (dev, type ,command),
                            type);

        type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        command = CPSS_PACKET_CMD_FORWARD_E;

        /* test GT_BAD_PARAM cases on for param : command */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCommandSet
                            (dev, type ,command),
                            command);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
    command = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpExceptionCommandSet(dev, type ,command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpExceptionCommandSet(dev, type ,command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpExceptionCommandGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        type;
    CPSS_PACKET_CMD_ENT                         commandGet;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 4 ; jj++)
        {
            type = typesArr[jj];

            st = cpssDxChHsrPrpExceptionCommandGet(dev, type ,&commandGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChHsrPrpExceptionCommandGet(dev, type , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCommandGet
                            (dev, type ,&commandGet),
                            type);
    }

    type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpExceptionCommandGet(dev, type ,&commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpExceptionCommandGet(dev, type ,&commandGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpExceptionCpuCodeSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        type;
    CPSS_NET_RX_CPU_CODE_ENT                    cpuCode;
    CPSS_NET_RX_CPU_CODE_ENT                    cpuCodeGet;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E
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
                st = cpssDxChHsrPrpExceptionCpuCodeSet(dev, type ,cpuCode);
                if(st == GT_OK)
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    st = cpssDxChHsrPrpExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(cpuCode,cpuCodeGet);
                }
            }

            /* test GT_OK cases */
            for(cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
                cpuCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
                cpuCode ++)
            {
                st = cpssDxChHsrPrpExceptionCpuCodeSet(dev, type ,cpuCode);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChHsrPrpExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                if(cpuCodeGet != cpuCode &&
                   cpuCodeGet >= CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E)
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(cpuCode,cpuCodeGet);
                }
            }
        }

        type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCpuCodeSet
                            (dev, type ,cpuCode),
                            type);

        type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

        /* test GT_BAD_PARAM cases on enum for param : cpuCode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCpuCodeSet
                            (dev, type ,cpuCode),
                            cpuCode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
    cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpExceptionCpuCodeSet(dev, type ,cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpExceptionCpuCodeSet(dev, type ,cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpExceptionCpuCodeGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        type;
    CPSS_NET_RX_CPU_CODE_ENT                    cpuCodeGet;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT        typesArr[4] =
    {
         CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E
        ,CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 4 ; jj++)
        {
            type = typesArr[jj];

            st = cpssDxChHsrPrpExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChHsrPrpExceptionCpuCodeGet(dev, type , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCpuCodeGet
                            (dev, type ,&cpuCodeGet),
                            type);
    }

    type    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpExceptionCpuCodeGet(dev, type ,&cpuCodeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortPrpEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpEnableSet)
{
    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         portNum  = 0;
    GT_BOOL             enable = GT_TRUE;
    GT_BOOL             enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E);
        st = prvUtfNextGenericPortReset(&portNum, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);

            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "get another enable than was set: %d", dev);

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);
            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "get another enable than was set: %d", dev);

        }

        enable = GT_TRUE;

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHsrPrpPortPrpEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpEnableGet)
{
    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         portNum  = 0;
    GT_BOOL             enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E);
        st = prvUtfNextGenericPortReset(&portNum, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable.
                Expected: GT_OK.
            */

            st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enableGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpEnableGet(dev, portNum, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHsrPrpPortPrpLanIdSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    IN  GT_U32                   lanId
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpLanIdSet)
{
    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         portNum  = 0;
    CPSS_DIRECTION_ENT  direction;
    GT_U32              lanId;
    GT_U32              lanIdGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(direction = CPSS_DIRECTION_INGRESS_E ; direction <= CPSS_DIRECTION_BOTH_E; direction++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   direction != CPSS_DIRECTION_EGRESS_E ?/*ingress ot both*/
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
            {
                lanId = (portNum + 5) % 16;/*0..15*/

                st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lanId);

                /*
                    1.1.2. Call cpssDxChBrgVlanPortVidGet.
                    Expected: GT_OK and the same vlanId.
                */
                st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
                if(direction == CPSS_DIRECTION_BOTH_E)
                {
                    /* the 'Get' not supports 'both' , so use both directions to check the value */
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                             "cpssDxChBrgVlanPortVidGet: %d, %d", dev, portNum);
                    st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum,
                            CPSS_DIRECTION_EGRESS_E,&lanIdGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChBrgVlanPortVidGet: %d, %d", dev, portNum);

                    UTF_VERIFY_EQUAL2_STRING_MAC(lanId, lanIdGet,
                                             "get another lanId than was set: %d, %d", dev, portNum);

                    st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum,
                            CPSS_DIRECTION_INGRESS_E,&lanIdGet);
                }

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChBrgVlanPortVidGet: %d, %d", dev, portNum);

                UTF_VERIFY_EQUAL2_STRING_MAC(lanId, lanIdGet,
                                         "get another lanId than was set: %d, %d", dev, portNum);

                /*
                    1.1.3. Call with out of range lanId [0..15].
                    Expected: GT_OUT_OF_RANGE.
                */
                lanId   = 16;
                st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, portNum, lanId);
            }

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
            {
                lanId = (portNum + 5) % 16;/*0..15*/
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */

            lanId = 0;
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                            */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    lanId = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpLanIdSet(dev, portNum, direction,lanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortPrpLanIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    OUT GT_U32                   *lanIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpLanIdGet)
{
    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         portNum  = 0;
    CPSS_DIRECTION_ENT  direction;
    GT_U32              lanIdGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(direction = CPSS_DIRECTION_INGRESS_E ; direction <= CPSS_DIRECTION_BOTH_E; direction++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   direction != CPSS_DIRECTION_EGRESS_E ?/*ingress ot both*/
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
            {
                /*
                    1.1.2. Call cpssDxChBrgVlanPortVidGet.
                    Expected: GT_OK and the same vlanId.
                */
                st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
                if(direction == CPSS_DIRECTION_BOTH_E)
                {
                /* the 'Get' not supports 'both' */
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                             "cpssDxChBrgVlanPortVidGet: %d, %d", dev, portNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChBrgVlanPortVidGet: %d, %d", dev, portNum);
                }

                /*
                    1.1.2. Call with lanIdPtr [NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChBrgVlanPortVidGet(dev, portNum, direction,NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, lanIdPtr = NULL", dev, portNum);
            }

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

                /*
                    1.1.2. Call with lanIdPtr [NULL] -- ON non-valid portNum.
                    Expected: GT_BAD_PARAM / GT_BAD_PTR.
                */
                st = cpssDxChBrgVlanPortVidGet(dev, portNum, direction,NULL);
                if(st != GT_BAD_PARAM)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, lanIdPtr = NULL", dev, portNum);
                }
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */

            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                            */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
            if(direction == CPSS_DIRECTION_BOTH_E)
            {
                /* the 'Get' not supports 'both' */
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                         "cpssDxChBrgVlanPortVidGet: %d, %d", dev, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
            }
        }
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpLanIdGet(dev, portNum, direction,&lanIdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet)
{
    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         portNum  = 0;
    GT_BOOL             enable = GT_TRUE;
    GT_BOOL             enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E);
        st = prvUtfNextGenericPortReset(&portNum, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);

            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "get another enable than was set: %d", dev);

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);
            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "get another enable than was set: %d", dev);

        }

        enable = GT_TRUE;

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet)
{
    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         portNum  = 0;
    GT_BOOL             enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E);
        st = prvUtfNextGenericPortReset(&portNum, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable.
                Expected: GT_OK.
            */

            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enableGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(dev, portNum, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortsBmpSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    IN  CPSS_PORTS_SHORT_BMP_STC            bmp
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortsBmpSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii,jj;
    CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT          bmpType;
    CPSS_PORTS_SHORT_BMP_STC                    bmp;
    CPSS_PORTS_SHORT_BMP_STC                    bmpGet;
    CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT        typesArr[6] =
    {
         CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_2_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_3_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_DDL_DECISION_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_SOURCE_DUPLICATION_DISCARD_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(ii = 0 ; ii < 7 ; ii++)
        {
            for(jj = 0 ; jj < 6 ; jj++)
            {
                bmp = (0xABCD + (ii * 11) + jj) & (BIT_10-1);
                bmpType = typesArr[jj];

                st = cpssDxChHsrPrpPortsBmpSet(dev, bmpType ,bmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            for(jj = 0 ; jj < 6 ; jj++)
            {
                bmp = (0xABCD + (ii * 11) + jj) & (BIT_10-1);
                bmpType = typesArr[jj];

                st = cpssDxChHsrPrpPortsBmpGet(dev, bmpType ,&bmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(bmp,bmpGet);
            }
        }

        bmpType = CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E;
        bmp = BIT_10-1;
        st = cpssDxChHsrPrpPortsBmpSet(dev, bmpType ,bmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        bmpType = CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E;
        bmp = BIT_10;
        st = cpssDxChHsrPrpPortsBmpSet(dev, bmpType ,bmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        bmpType = CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E;
        bmp = 0x1;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpPortsBmpSet
                            (dev, bmpType ,bmp),
                            bmpType);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    bmpType    = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
    bmp = 0x1;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortsBmpSet(dev, bmpType ,bmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortsBmpSet(dev, bmpType ,bmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortsBmpGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    OUT CPSS_PORTS_SHORT_BMP_STC            *bmpPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortsBmpGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      jj;
    CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT          bmpType;
    CPSS_PORTS_SHORT_BMP_STC                    bmpGet;
    CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT        typesArr[6] =
    {
         CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_2_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_3_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_DDL_DECISION_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_SOURCE_DUPLICATION_DISCARD_E
        ,CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E
    };

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(jj = 0 ; jj < 6 ; jj++)
        {
            bmpType = typesArr[jj];

            st = cpssDxChHsrPrpPortsBmpGet(dev, bmpType ,&bmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChHsrPrpPortsBmpGet(dev, bmpType , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        bmpType    = CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E;
        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpPortsBmpGet
                            (dev, bmpType ,&bmpGet),
                            bmpType);
    }

    bmpType    = CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortsBmpGet(dev, bmpType ,&bmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortsBmpGet(dev, bmpType ,&bmpGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChHsrPrpHPortSrcEPortBaseSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         baseEport
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpHPortSrcEPortBaseSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_PORT_NUM                                 portNum;
    GT_PORT_NUM                                 portNumGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChHsrPrpHPortSrcEPortBaseSet(dev, portNum);
            if(portNum & 0xF)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                st = cpssDxChHsrPrpHPortSrcEPortBaseGet(dev, &portNumGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(portNum,portNumGet);
            }
        }

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);
        st = cpssDxChHsrPrpHPortSrcEPortBaseSet(dev, portNum);
        if(portNum < BIT_16)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        portNum = BIT_16;
        st = cpssDxChHsrPrpHPortSrcEPortBaseSet(dev, portNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, portNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpHPortSrcEPortBaseSet(dev, portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpHPortSrcEPortBaseSet(dev, portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpHPortSrcEPortBaseGet
(
    IN  GT_U8               devNum,
    OUT GT_PORT_NUM         *baseEportPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpHPortSrcEPortBaseGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_PORT_NUM                                 portNumGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChHsrPrpHPortSrcEPortBaseGet(dev, &portNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChHsrPrpHPortSrcEPortBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpHPortSrcEPortBaseGet(dev, &portNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpHPortSrcEPortBaseGet(dev, &portNumGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpCounterGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT  counterType,
    OUT GT_U64                              *counterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpCounterGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT          counterType;
    GT_U64                                      counterGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(counterType = CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E ;
           counterType <= CPSS_DXCH_HSR_PRP_COUNTER_OVERRIDEN_NON_EXPIRED_ENTRIES_E ;
           counterType ++)
        {
            st = cpssDxChHsrPrpCounterGet(dev, counterType , &counterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChHsrPrpCounterGet(dev, counterType , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        /* test GT_BAD_PARAM cases on enum for param : counterType */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpCounterGet
                            (dev, counterType , &counterGet),
                            counterType);
    }

    counterType = CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpCounterGet(dev, counterType , &counterGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpCounterGet(dev, counterType , &counterGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              threshold
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      threshold;
    GT_U32                                      thresholdGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(threshold = 0 ; threshold < BIT_10 ; threshold += 11)
        {
            st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet(dev, threshold);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(dev, &thresholdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(threshold,thresholdGet);
        }

        threshold = BIT_10-1;
        st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet(dev, threshold);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, threshold);

        threshold = BIT_10;
        st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet(dev, threshold);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, threshold);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    threshold = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet(dev, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet(dev, threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet
(
    IN  GT_U8               devNum,
    OUT GT_U32              *thresholdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      thresholdGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(dev, &thresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(dev, &thresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(dev, &thresholdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpHPortEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    IN  CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpHPortEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii;
    GT_U32                                      interation;
    GT_U32                                      hPort;
    CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC           info;
    CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC           infoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(ii = 0 ; ii < 100 ; ii ++)
        {
            for(hPort = 0 ; hPort < CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS ; hPort++)
            {
                interation = ii * CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS + hPort;

                info.pntLookupMode         = (CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT)(interation % 4);
                info.fdbDdlMode            = (CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT)((interation >> 2) % 3);
                info.pntLearnTaggedPackets = (GT_BOOL)((interation >> 4) % 2);
                info.acceptSamePort        = (GT_BOOL)((interation >> 5) % 2);
                info.lreRingPortType       = (CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT) ((interation >> 6) % 2);
                info.lreInstance           = ((interation >> 7) % 4);
                info.defaultForwardingBmp  = (BIT_10 - interation)& (BIT_10-1);
                info.srcIsMeBmp            = (BIT_9 + interation) & (BIT_10-1);
                info.srcDuplicateDiscardBmp= (BIT_8 + interation) & (BIT_10-1);
                info.destToMeForwardingBmp = (BIT_7 + interation) & (BIT_10-1);

                st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            for(hPort = 0 ; hPort < CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS ; hPort++)
            {
                interation = ii * CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS + hPort;

                info.pntLookupMode         = (CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT)(interation % 4);
                info.fdbDdlMode            = (CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT)((interation >> 2) % 3);
                info.pntLearnTaggedPackets = (GT_BOOL)((interation >> 4) % 2);
                info.acceptSamePort        = (GT_BOOL)((interation >> 5) % 2);
                info.lreRingPortType       = (CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT) ((interation >> 6) % 2);
                info.lreInstance           = ((interation >> 7) % 4);
                info.defaultForwardingBmp  = (BIT_10 - interation)& (BIT_10-1);
                info.srcIsMeBmp            = (BIT_9 + interation) & (BIT_10-1);
                info.srcDuplicateDiscardBmp= (BIT_8 + interation) & (BIT_10-1);
                info.destToMeForwardingBmp = (BIT_7 + interation) & (BIT_10-1);

                st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , NULL , NULL , NULL);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(info.pntLookupMode         ,infoGet.pntLookupMode         );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.fdbDdlMode            ,infoGet.fdbDdlMode            );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.pntLearnTaggedPackets ,infoGet.pntLearnTaggedPackets );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.acceptSamePort        ,infoGet.acceptSamePort        );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.lreRingPortType       ,infoGet.lreRingPortType       );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.lreInstance           ,infoGet.lreInstance           );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.defaultForwardingBmp  ,infoGet.defaultForwardingBmp  );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.srcIsMeBmp            ,infoGet.srcIsMeBmp            );
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.srcDuplicateDiscardBmp,infoGet.srcDuplicateDiscardBmp);
                UTF_VERIFY_EQUAL0_PARAM_MAC(info.destToMeForwardingBmp ,infoGet.destToMeForwardingBmp );
            }
        }

        hPort = 0;
        cpssOsMemSet(&info,0,sizeof(info));

        /* test GT_BAD_PARAM cases on enum for param : pntLookupMode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpHPortEntrySet
                            (dev, hPort , &info),
                            info.pntLookupMode);
        info.pntLookupMode = 0;
        /* test GT_BAD_PARAM cases on enum for param : fdbDdlMode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpHPortEntrySet
                            (dev, hPort , &info),
                            info.fdbDdlMode);
        info.fdbDdlMode = 0;
        /* test GT_BAD_PARAM cases on enum for param : lreRingPortType */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpHPortEntrySet
                            (dev, hPort , &info),
                            info.lreRingPortType);
        info.lreRingPortType = 0;

        info.lreInstance = 4;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.lreInstance);
        info.lreInstance = 0;

        info.defaultForwardingBmp = BIT_10;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.defaultForwardingBmp);
        info.defaultForwardingBmp = 0;

        info.srcIsMeBmp = BIT_10;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.srcIsMeBmp);
        info.srcIsMeBmp = 0;

        info.srcDuplicateDiscardBmp = BIT_10;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.srcDuplicateDiscardBmp);
        info.srcDuplicateDiscardBmp = 0;

        info.destToMeForwardingBmp = BIT_10;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.destToMeForwardingBmp);
        info.destToMeForwardingBmp = 0;

        /* make sure that indeed we not forgot one of the parameters at 'GT_OUT_OF_RANGE' */
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, info.destToMeForwardingBmp);

        hPort = CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        hPort = 0;
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    hPort = 0;
    cpssOsMemSet(&info,0,sizeof(info));

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpHPortEntrySet(dev, hPort , &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpHPortEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    OUT CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr,
    OUT GT_U64                          *firstCounterPtr,
    OUT GT_U64                          *secondCounterPtr,
    OUT GT_U64                          *thirdCounterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpHPortEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      hPort;
    CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC           infoGet;
    GT_U64                                      firstCounterGet;
    GT_U64                                      secondCounterGet;
    GT_U64                                      thirdCounterGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(hPort = 0 ; hPort < CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS ; hPort++)
        {
            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , &secondCounterGet , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , &firstCounterGet , &secondCounterGet , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , NULL , &secondCounterGet , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , &firstCounterGet , NULL , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , NULL ,NULL , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , &firstCounterGet , &secondCounterGet , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , NULL , &secondCounterGet , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL , &firstCounterGet , NULL , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , NULL , &secondCounterGet , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , NULL , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , NULL ,NULL , &thirdCounterGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , &secondCounterGet , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , NULL , &secondCounterGet , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , NULL , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , NULL,NULL,NULL,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        hPort = CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS;
        st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , &secondCounterGet , &thirdCounterGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    hPort = 0;
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev , (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , &secondCounterGet , &thirdCounterGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpHPortEntryGet(dev, hPort , &infoGet , &firstCounterGet , &secondCounterGet , &thirdCounterGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPntEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_HSR_PRP_PNT_OPER_ENT       operation,
    IN  CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPntEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii;
    GT_U32                                      interation;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_PNT_OPER_ENT              operation;
    CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC             info;
    CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC             infoGet;
    CPSS_DXCH_HSR_PRP_PNT_STATE_ENT             stateGet;
    CPSS_DXCH_HSR_PRP_PNT_STATE_ENT             stateGet_expected;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(ii = 0 ; ii < 100 ; ii ++)
        {
            for(index = 0 ; index < 128 ; index+=17)
            {
                interation = ii * 128 + index;

                operation                  = (CPSS_DXCH_HSR_PRP_PNT_OPER_ENT)(interation % 3);
                stateGet_expected          = operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E ?
                                                    CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E :
                                             operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E ?
                                                    CPSS_DXCH_HSR_PRP_PNT_STATE_VALID_E :
                                                    CPSS_DXCH_HSR_PRP_PNT_STATE_NOT_VALID_E;

                if(operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E ||
                   operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E )
                {
                    info.streamId              = (BIT_16-1 - interation);
                }
                else
                {
                    /* not checked ! */
                    info.streamId              = GT_NA;
                }

                if(operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E )
                {
                    info.macAddr.arEther[0]    = 0x88;
                    info.macAddr.arEther[1]    = 0x99;
                    info.macAddr.arEther[2]    = 0x77;
                    info.macAddr.arEther[3]    = 0xaa;
                    info.macAddr.arEther[4]    = (GT_U8)(interation >> 8);
                    info.macAddr.arEther[5]    = (GT_U8)interation;
                    info.ageBit                = (GT_BOOL)((interation >> 4) % 2);
                    info.untagged              = (GT_BOOL)((interation >> 5) % 2);
                    info.lreInstance           = (interation >> 2) % 4;
                    info.destBmp               = (BIT_10   - interation)& (BIT_10-1);
                }
                else
                {
                    /* not checked ! */
                    info.macAddr.arEther[0]    = 0xFF;
                    info.macAddr.arEther[1]    = 0xFF;
                    info.macAddr.arEther[2]    = 0xFF;
                    info.macAddr.arEther[3]    = 0xFF;
                    info.macAddr.arEther[4]    = 0xFF;
                    info.macAddr.arEther[5]    = 0xFF;
                    info.ageBit                = GT_NA;
                    info.untagged              = GT_NA;
                    info.lreInstance           = GT_NA;
                    info.destBmp               = GT_NA;
                }

                st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            for(index = 0 ; index < 128 ; index+=17)
            {
                interation = ii * 128 + index;

                operation                  = (CPSS_DXCH_HSR_PRP_PNT_OPER_ENT)(interation % 3);
                stateGet_expected          = operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E ?
                                                    CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E :
                                             operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E ?
                                                    CPSS_DXCH_HSR_PRP_PNT_STATE_VALID_E :
                                                    CPSS_DXCH_HSR_PRP_PNT_STATE_NOT_VALID_E;

                if(operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E ||
                   operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E )
                {
                    info.streamId              = (BIT_16-1 - interation);
                }
                else
                {
                    /* not checked ! */
                    info.streamId              = GT_NA;
                }

                if(operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E )
                {
                    info.macAddr.arEther[0]    = 0x88;
                    info.macAddr.arEther[1]    = 0x99;
                    info.macAddr.arEther[2]    = 0x77;
                    info.macAddr.arEther[3]    = 0xaa;
                    info.macAddr.arEther[4]    = (GT_U8)(interation >> 8);
                    info.macAddr.arEther[5]    = (GT_U8)interation;
                    info.ageBit                = (GT_BOOL)((interation >> 4) % 2);
                    info.untagged              = (GT_BOOL)((interation >> 5) % 2);
                    info.lreInstance           = (interation >> 2) % 4;
                    info.destBmp               = (BIT_10   - interation)& (BIT_10-1);
                }
                else
                {
                    /* not checked ! */
                    info.macAddr.arEther[0]    = 0xFF;
                    info.macAddr.arEther[1]    = 0xFF;
                    info.macAddr.arEther[2]    = 0xFF;
                    info.macAddr.arEther[3]    = 0xFF;
                    info.macAddr.arEther[4]    = 0xFF;
                    info.macAddr.arEther[5]    = 0xFF;
                    info.ageBit                = GT_NA;
                    info.untagged              = GT_NA;
                    info.lreInstance           = GT_NA;
                    info.destBmp               = GT_NA;
                }

                st = cpssDxChHsrPrpPntEntryGet(dev, index , &stateGet , &infoGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL0_PARAM_MAC(stateGet_expected , stateGet);

                if(operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E ||
                   operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E )
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.streamId         ,infoGet.streamId   );
                }

                if(operation == CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E )
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.macAddr.arEther[0],infoGet.macAddr.arEther[0] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.macAddr.arEther[1],infoGet.macAddr.arEther[1] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.macAddr.arEther[2],infoGet.macAddr.arEther[2] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.macAddr.arEther[3],infoGet.macAddr.arEther[3] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.macAddr.arEther[4],infoGet.macAddr.arEther[4] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.macAddr.arEther[5],infoGet.macAddr.arEther[5] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ageBit           ,infoGet.ageBit     );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.untagged         ,infoGet.untagged   );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.lreInstance      ,infoGet.lreInstance);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.destBmp          ,infoGet.destBmp    );
                }

                /* invalidate the entry so we can run the test again after this one finishes */
                operation = CPSS_DXCH_HSR_PRP_PNT_OPER_SET_INVALID_E;
                st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        index = 0;
        operation                  = CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E;
        cpssOsMemSet(&info,0,sizeof(info));

        info.streamId = BIT_16;
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.streamId);
        info.streamId = 0;

        info.lreInstance = 4;
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.lreInstance);
        info.lreInstance = 0;

        info.destBmp = BIT_10;
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.destBmp);
        info.destBmp = 0;

        /* make sure that indeed we not forgot one of the parameters at 'GT_OUT_OF_RANGE' */
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 128;
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        operation                  = CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E;
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* test GT_BAD_PARAM cases on enum for param : operation */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpPntEntrySet
                            (dev, index , operation , &info),
                            operation);

        /* invalidate the entry so we can run the test again after this one finishes */
        operation = CPSS_DXCH_HSR_PRP_PNT_OPER_SET_INVALID_E;
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    index = 0;
    operation                  = CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E;
    cpssOsMemSet(&info,0,sizeof(info));

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPntEntrySet(dev, index , operation , &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPntEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_HSR_PRP_PNT_STATE_ENT      *statePtr,
    OUT CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPntEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC             infoGet;
    CPSS_DXCH_HSR_PRP_PNT_STATE_ENT             stateGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 128 ; index+=13)
        {

            st = cpssDxChHsrPrpPntEntryGet(dev, index , &stateGet , &infoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
             * 1.2. Call function with param as NULL.
             * Expected: GT_BAD_PTR.
             */
            st = cpssDxChHsrPrpPntEntryGet(dev, index , &stateGet , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChHsrPrpPntEntryGet(dev, index , NULL , &infoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        index = 128;
        st = cpssDxChHsrPrpPntEntryGet(dev, index , &stateGet , &infoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPntEntryGet(dev, index , &stateGet , &infoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPntEntryGet(dev, index , &stateGet , &infoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPntAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPntAgingApply)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    GT_U32                                      numOfEntries;
    GT_U32                                      numOfEntriesDeleted;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 128 ; index+=13)
        {
            numOfEntries = 128 - index;
            st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* allow NULL */
            st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* allow 'FULL table' */
            numOfEntries = 0xFFFFFFFF;
            st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        index = 128;
        numOfEntries = 1;
        st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        numOfEntries = 0;
        st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        numOfEntries = 128+1;
        st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    }

    index = 0;
    numOfEntries = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPntAgingApply(dev, index , numOfEntries , &numOfEntriesDeleted);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpLreInstanceEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpLreInstanceEntrySet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii,index;
    CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC    info;
    CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC    infoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(ii = 0 ; ii < 100 ; ii ++)
        {
            info.numOfPorts  = ii % 11;
            info.lowestHPort = ii % 10;
            info.eVidxMappingBase = BIT_13 -
                (1 << info.numOfPorts) - 1;

            index = ii % 4;

            st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
            if((info.lowestHPort + info.numOfPorts) > 10)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

                info.numOfPorts = 10 - info.lowestHPort;
                info.eVidxMappingBase = BIT_13 -
                    (1 << info.numOfPorts) - 1;

                st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
            }
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* check the last 4 writes to index 0..3 */
        for(ii = 96 ; ii < 100 ; ii ++)
        {
            info.numOfPorts  = ii % 11;
            info.lowestHPort = ii % 10;
            if((info.lowestHPort + info.numOfPorts) > 10)
            {
                info.numOfPorts = 10 - info.lowestHPort;
            }
            info.eVidxMappingBase = BIT_13 -
                (1 << info.numOfPorts) - 1;

            index = ii % 4;

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet,NULL,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(info.numOfPorts       , infoGet.numOfPorts);
            UTF_VERIFY_EQUAL0_PARAM_MAC(info.lowestHPort      , infoGet.lowestHPort);
            UTF_VERIFY_EQUAL0_PARAM_MAC(info.eVidxMappingBase , infoGet.eVidxMappingBase);

        }

        index = 0;
        cpssOsMemSet(&info,0,sizeof(info));
        info.numOfPorts = 1;

        info.numOfPorts = 11;
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.numOfPorts);
        info.numOfPorts = 1;

        info.lowestHPort = 10;
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.lowestHPort);
        info.lowestHPort = 0;

        /* 7+4 > 10*/
        info.numOfPorts  = 7;
        info.lowestHPort = 4;
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.lowestHPort,info.numOfPorts);
        info.numOfPorts  = 0;
        info.lowestHPort = 0;

        info.numOfPorts       = 5;
        info.eVidxMappingBase = BIT_13 -
            (1 << info.numOfPorts);
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, info.eVidxMappingBase,info.numOfPorts);
        info.eVidxMappingBase = 0;
        info.numOfPorts       = 0;

        /* make sure that indeed we not forgot one of the parameters at 'GT_OUT_OF_RANGE' */
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 4;
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        index = 0;

        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    index = 0;
    cpssOsMemSet(&info,0,sizeof(info));

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpLreInstanceEntrySet(dev, index , &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChHsrPrpLreInstanceEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr,
    OUT GT_U64                              *lreADiscardCounterPtr,
    OUT GT_U64                              *lreBDiscardCounterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpLreInstanceEntryGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC    infoGet;
    GT_U64                                      lreADiscardCounter;
    GT_U64                                      lreBDiscardCounter;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , &lreADiscardCounter , &lreBDiscardCounter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , NULL , &lreBDiscardCounter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , &lreADiscardCounter , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , NULL , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , NULL , &lreADiscardCounter , &lreBDiscardCounter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , NULL , NULL , &lreBDiscardCounter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , NULL , &lreADiscardCounter , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , NULL , NULL , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        index = 4;
        st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , &lreADiscardCounter , &lreBDiscardCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        index = 0;
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    index = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , &lreADiscardCounter , &lreBDiscardCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpLreInstanceEntryGet(dev, index , &infoGet , &lreADiscardCounter , &lreBDiscardCounter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpFdbPartitionModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT mode
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpFdbPartitionModeSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    modeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            st = cpssDxChHsrPrpFdbPartitionModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(mode,modeGet);
        }

        mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
        /* test GT_BAD_PARAM cases on enum for param : mode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpFdbPartitionModeSet
                            (dev, mode),
                            mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpFdbPartitionModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT *modePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpFdbPartitionModeGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    modeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChHsrPrpFdbPartitionModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChHsrPrpFdbPartitionModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpFdbPartitionModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpFdbPartitionModeGet(dev, &modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChHsrPrpDdeTimerGranularityModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT mode
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeTimerGranularityModeSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT    mode;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT    modeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E ;
            mode <= CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            st = cpssDxChHsrPrpDdeTimerGranularityModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(mode,modeGet);
        }

        mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E;
        /* test GT_BAD_PARAM cases on enum for param : mode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpDdeTimerGranularityModeSet
                            (dev, mode),
                            mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChHsrPrpDdeTimerGranularityModeGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT *modePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeTimerGranularityModeGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT    modeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChHsrPrpDdeTimerGranularityModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
         * 1.2. Call function with param as NULL.
         * Expected: GT_BAD_PTR.
         */
        st = cpssDxChHsrPrpDdeTimerGranularityModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeTimerGranularityModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeTimerGranularityModeGet(dev, &modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpDdeTimeFieldSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    IN  GT_U32                          timeInMicroSec
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeTimeFieldSet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT    mode;
    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT        field;
    GT_U32                                      timeInMicroSec;
    GT_U32                                      timeInMicroSecGet;
    GT_U32                                      maxMicroDiffArr[]={5,10,20,40};

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E ;
            mode <= CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            for(field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;
                field <= CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E;
                field++)
            {
                for(timeInMicroSec = 0 ;
                    timeInMicroSec < (BIT_22*maxMicroDiffArr[mode]*2+ maxMicroDiffArr[mode] ) ;
                    timeInMicroSec += 11111)
                {
                    st = cpssDxChHsrPrpDdeTimeFieldSet(dev, field,timeInMicroSec);
                    if(field == CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E)
                    {
                        /* the field can not be 'set' as it is 'read only' running timer */
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
                        break;
                    }

                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    st = cpssDxChHsrPrpDdeTimeFieldGet(dev, field , &timeInMicroSecGet);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    if(timeInMicroSec >= timeInMicroSecGet)
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(((maxMicroDiffArr[mode]>=(timeInMicroSec-timeInMicroSecGet))?1:0),1);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(((maxMicroDiffArr[mode]>=(timeInMicroSecGet-timeInMicroSec))?1:0),1);
                    }
                }
            }
        }

        for(mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E ;
            mode <= CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            for(field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;
                field <= CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E;
                field++)
            {
                /* this is the lowest GT_OUT_OF_RANGE */
                timeInMicroSec = (BIT_22-1) * maxMicroDiffArr[mode]*2 + maxMicroDiffArr[mode];
                st = cpssDxChHsrPrpDdeTimeFieldSet(dev, field,timeInMicroSec);
                if(field == CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E)
                {
                    /* the field can not be 'set' as it is 'read only' running timer */
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
                    break;
                }
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);


                timeInMicroSec --;/*one less from the lowest 'GT_OUT_OF_RANGE'*/
                st = cpssDxChHsrPrpDdeTimeFieldSet(dev, field,timeInMicroSec);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;
        timeInMicroSec = 100;
        /* test GT_BAD_PARAM cases on enum for param : mode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpDdeTimeFieldSet
                            (dev, field , timeInMicroSec),
                            field);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;
    timeInMicroSec = 100;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeTimeFieldSet(dev, field,timeInMicroSec);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeTimeFieldSet(dev, field,timeInMicroSec);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpDdeTimeFieldGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    OUT GT_U32                          *timeInMicroSecPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeTimeFieldGet)
{
    GT_STATUS                                   st           = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT    mode;
    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT        field;
    GT_U32                                      timeInMicroSecGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E ;
            mode <= CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            for(field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;
                field <= CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E;
                field++)
            {
                st = cpssDxChHsrPrpDdeTimeFieldGet(dev, field , &timeInMicroSecGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChHsrPrpDdeTimeFieldGet(dev, field , NULL);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
            }
        }

        field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;

        /* test GT_BAD_PARAM cases on enum for param : mode */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpDdeTimeFieldGet
                            (dev, field , &timeInMicroSecGet),
                            field);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    field = CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeTimeFieldGet(dev, field,&timeInMicroSecGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeTimeFieldGet(dev, field,&timeInMicroSecGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*
GT_STATUS cpssDxChHsrPrpPortPrpTrailerActionSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT action
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpTrailerActionSet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT    action;
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT    actionGet;

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
            for(action = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E ;
                action <= CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_REMOVE_PRP_E;
                action++)
            {

                st = cpssDxChHsrPrpPortPrpTrailerActionSet(dev, portNum, action);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, action);

                /*
                    1.1.2. Call get func.
                    Expected: GT_OK and the same enable.
                */

                st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, action);
                UTF_VERIFY_EQUAL1_STRING_MAC(action, action,
                                   "get another action than was set: %d", dev);
            }
        }

        portNum = 0;
        action = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCommandSet
                            (dev, portNum ,action),
                            action);

        action = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E;

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpTrailerActionSet(dev, portNum, action);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpTrailerActionSet(dev, portNum, action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpTrailerActionSet(dev, portNum, action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    action = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpTrailerActionSet(dev, portNum, action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpTrailerActionSet(dev, portNum, action);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHsrPrpPortPrpTrailerActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT *actionPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpTrailerActionGet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum  = 0;
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT    actionGet;

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

            st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, actionGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpTrailerActionGet(dev, portNum, &actionGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortPrpPaddingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   mode
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpPaddingModeSet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT      mode;
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT      modeGet;

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
            for(mode = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E ;
                mode <= CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_64_E;
                mode++)
            {

                st = cpssDxChHsrPrpPortPrpPaddingModeSet(dev, portNum, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, mode);

                /*
                    1.1.2. Call get func.
                    Expected: GT_OK and the same enable.
                */

                st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, mode);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, mode,
                                   "get another mode than was set: %d", dev);
            }
        }

        portNum = 0;
        mode = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E;

        /* test GT_BAD_PARAM cases on enum for param : type */
        UTF_ENUMS_CHECK_MAC(cpssDxChHsrPrpExceptionCommandSet
                            (dev, portNum ,mode),
                            mode);

        mode = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E;

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpPaddingModeSet(dev, portNum, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpPaddingModeSet(dev, portNum, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpPaddingModeSet(dev, portNum, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    mode = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpPaddingModeSet(dev, portNum, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpPaddingModeSet(dev, portNum, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHsrPrpPortPrpPaddingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   *modePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortPrpPaddingModeGet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum  = 0;
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT      modeGet;

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

            st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, modeGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortPrpPaddingModeGet(dev, portNum, &modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpDdeEntryInvalidate
(
    IN  GT_U8         devNum,
    IN  GT_U32        index
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeEntryInvalidate)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    GT_U32                                      maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E+1];
    GT_U32                                      numEntries;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev,CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E,&numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E] = 0;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E ] = numEntries/2;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E ] = numEntries - numEntries/4;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E ] = numEntries - numEntries/8;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ] = numEntries;

        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(maxDdeNumArr[mode] != 0)
            {
                for(index = 0; index < maxDdeNumArr[mode] ; index += 111)
                {
                    st = cpssDxChHsrPrpDdeEntryInvalidate(dev, index);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
                }

                /* 1.3. For active device check that function returns GT_BAD_PARAM */
                /* for out of bound value for port number.                         */

                index = maxDdeNumArr[mode];
            }
            else
            {
                index = 0;
            }
            st = cpssDxChHsrPrpDdeEntryInvalidate(dev, index);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }

        /* restore */
        mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
        st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    index = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeEntryInvalidate(dev, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpDdeEntryWrite
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    IN  CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC   *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeEntryWrite)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    GT_U32                                      ii,iterator;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    GT_U32                                      maxMicroDiffArr[]={5,10,20,40};
    GT_U32                                      maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E+1];
    GT_U32                                      numEntries;
    CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC             info;
    CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC             infoGet;
    GT_BOOL                                     isValid;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT    GRANULARITY_mode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev,CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E,&numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E] = 0;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E ] = numEntries/2;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E ] = numEntries - numEntries/4;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E ] = numEntries - numEntries/8;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ] = numEntries;

        GRANULARITY_mode = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E;
        st = cpssDxChHsrPrpDdeTimerGranularityModeSet(dev, GRANULARITY_mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(maxDdeNumArr[mode] != 0)
            {
                for(iterator = 0; iterator < maxDdeNumArr[mode] * 11 ; iterator += 111)
                {
                    info.ddeKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
                    info.ddeKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
                    info.ddeKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
                    info.ddeKey.macAddr.arEther[3] = (GT_U8) iterator;
                    info.ddeKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
                    info.ddeKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
                    info.ddeKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
                    info.ddeKey.lreInstance        = iterator & 0x3;
                    info.destBmp = iterator & 0x3ff;
                    info.srcHPort = iterator % 10;
                    info.timeInMicroSec = (iterator & 0x3FFFFF)*10;
                    for(ii = 0 ; ii < 10 ; ii++)
                    {
                        info.destHportCounters[ii] = (iterator+ii) & 3;
                    }
                    info.ageBit = iterator & 1;
                    index = iterator % maxDdeNumArr[mode];

                    st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
                }

                for(iterator = 0; iterator < maxDdeNumArr[mode]  * 11; iterator += 111)
                {
                    info.ddeKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
                    info.ddeKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
                    info.ddeKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
                    info.ddeKey.macAddr.arEther[3] = (GT_U8) iterator;
                    info.ddeKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
                    info.ddeKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
                    info.ddeKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
                    info.ddeKey.lreInstance        = iterator & 0x3;
                    info.destBmp = iterator & 0x3ff;
                    info.srcHPort = iterator % 10;
                    info.timeInMicroSec = (iterator & 0x3FFFFF)*10;
                    for(ii = 0 ; ii < 10 ; ii++)
                    {
                        info.destHportCounters[ii] = (iterator+ii) & 3;
                    }
                    info.ageBit = iterator & 1;
                    index = iterator % maxDdeNumArr[mode];

                    st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , &isValid);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, isValid);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.macAddr.arEther[0] , infoGet.ddeKey.macAddr.arEther[0] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.macAddr.arEther[1] , infoGet.ddeKey.macAddr.arEther[1] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.macAddr.arEther[2] , infoGet.ddeKey.macAddr.arEther[2] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.macAddr.arEther[3] , infoGet.ddeKey.macAddr.arEther[3] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.macAddr.arEther[4] , infoGet.ddeKey.macAddr.arEther[4] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.macAddr.arEther[5] , infoGet.ddeKey.macAddr.arEther[5] );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.seqNum             , infoGet.ddeKey.seqNum             );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ddeKey.lreInstance        , infoGet.ddeKey.lreInstance        );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.destBmp                   , infoGet.destBmp                   );
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.srcHPort                  , infoGet.srcHPort                  );

                    if(info.timeInMicroSec >= infoGet.timeInMicroSec)
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(((maxMicroDiffArr[GRANULARITY_mode]>=(info.timeInMicroSec-infoGet.timeInMicroSec))?1:0),1);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(((maxMicroDiffArr[GRANULARITY_mode]>=(infoGet.timeInMicroSec-info.timeInMicroSec))?1:0),1);
                    }

                    for(ii = 0 ; ii < 9 ; ii++)
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(info.destHportCounters[ii] , infoGet.destHportCounters[ii]     );
                    }
                    UTF_VERIFY_EQUAL0_PARAM_MAC(info.ageBit                    , infoGet.ageBit                    );

                    /* restore */
                    st = cpssDxChHsrPrpDdeEntryInvalidate(dev, index);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                    st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , &isValid);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, isValid);
                }

                /* 1.3. For active device check that function returns GT_BAD_PARAM */
                /* for out of bound value for port number.                         */

                index = maxDdeNumArr[mode];
            }
            else
            {
                index = 0;
            }
            st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

            index = 0;
            st = cpssDxChHsrPrpDdeEntryWrite(dev, index , NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);

        }

        index = 0;
        cpssOsMemSet(&info,0,sizeof(info));
        /* check out of range parameters */
        info.ddeKey.seqNum             = BIT_16;/*16 bits*/
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index);
        info.ddeKey.seqNum             = 0;

        info.ddeKey.lreInstance             = BIT_2;/*2 bits*/
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index);
        info.ddeKey.lreInstance             = 0;

        info.destBmp             = BIT_10;/*10 bits*/
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index);
        info.destBmp             = 0;

        info.srcHPort             = 10;/*max value 9*/
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index);
        info.srcHPort             = 0;

        info.timeInMicroSec             = (BIT_22-1) * maxMicroDiffArr[GRANULARITY_mode]*2 + maxMicroDiffArr[GRANULARITY_mode];
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index);
        info.timeInMicroSec             = 0;

        for(ii = 0 ; ii < 10 ; ii++)
        {
            info.destHportCounters[ii] = BIT_2;/*2 bits*/
            st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index);
            info.destHportCounters[ii] = 0;
        }

        /* make sure that all above parameters 'could' be OK and not failed on
           something else */
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore */
        st = cpssDxChHsrPrpDdeEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore */
        mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
        st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    index = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChHsrPrpDdeEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    OUT CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC  *entryPtr,
    OUT GT_BOOL                     *isValidPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeEntryRead)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    GT_U32                                      maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E+1];
    GT_U32                                      numEntries;
    CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC             infoGet;
    GT_BOOL                                     isValid;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev,CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E,&numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E] = 0;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E ] = numEntries/2;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E ] = numEntries - numEntries/4;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E ] = numEntries - numEntries/8;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ] = numEntries;

        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(maxDdeNumArr[mode] != 0)
            {
                for(index = 0; index < maxDdeNumArr[mode] ; index += 1111)
                {
                    st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , &isValid);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                    st = cpssDxChHsrPrpDdeEntryRead(dev, index , NULL , &isValid);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);

                    st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , NULL);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
                }

                /* 1.3. For active device check that function returns GT_BAD_PARAM */
                /* for out of bound value for port number.                         */

                index = maxDdeNumArr[mode];
            }
            else
            {
                index = 0;
            }
            st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , &isValid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }

        /* restore */
        mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
        st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    index = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , &isValid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeEntryRead(dev, index , &infoGet , &isValid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssDxChHsrPrpDdeAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeAgingApply)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      ii;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    GT_U32                                      maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E+1];
    GT_U32                                      numEntries;
    CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC             info;
    GT_U32                                      age_numOfEntriesDeleted;
    GT_U32                                      age_numOfEntries;
    GT_U32                                      age_0;
    GT_U32                                      age_1;
    GT_U32                                      iterator;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev,CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E,&numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E] = 0;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E ] = numEntries/2;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E ] = numEntries - numEntries/4;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E ] = numEntries - numEntries/8;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ] = numEntries;

        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(maxDdeNumArr[mode] == 0)
            {
                continue;
            }
            age_0 = 0;
            age_1 = 0;

            for(index = maxDdeNumArr[mode] / 2;
                index < (maxDdeNumArr[mode] * 2) / 3 ;
                index ++)
            {
                iterator = index;

                info.ddeKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
                info.ddeKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
                info.ddeKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
                info.ddeKey.macAddr.arEther[3] = (GT_U8) iterator;
                info.ddeKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
                info.ddeKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
                info.ddeKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
                info.ddeKey.lreInstance        = iterator & 0x3;
                info.destBmp = iterator & 0x3ff;
                info.srcHPort = iterator % 10;
                info.timeInMicroSec = (iterator & 0x3FFFFF)*10;
                for(ii = 0 ; ii < 10 ; ii++)
                {
                    info.destHportCounters[ii] = (iterator+ii) & 3;
                }
                info.ageBit = iterator & 1;
                if(iterator & 1)
                {
                    /* number of entries with age = 1 */
                    age_1++;
                }
                else
                {
                    /* number of entries with age = 0 */
                    age_0++;
                }

                st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }

            age_numOfEntries =  age_1 + age_0;

            st = cpssDxChHsrPrpDdeAgingApply(dev,
                maxDdeNumArr[mode] / 2,/* index*/
                age_numOfEntries,/*numOfEntries*/
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* those that were in age 0 should be deleted */
            UTF_VERIFY_EQUAL0_PARAM_MAC(age_0, age_numOfEntriesDeleted);

            st = cpssDxChHsrPrpDdeAgingApply(dev,
                maxDdeNumArr[mode] / 2,/* index*/
                age_numOfEntries,/*numOfEntries*/
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* those that were in age 1 (at start) should be deleted */
            UTF_VERIFY_EQUAL0_PARAM_MAC(age_1, age_numOfEntriesDeleted);


            index = 7;
            cpssOsMemSet(&info,0,sizeof(info));
            info.ageBit = GT_TRUE;

            st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

            age_numOfEntries = 1;
            /* NULL is ignored */
            st = cpssDxChHsrPrpDdeAgingApply(dev, index , age_numOfEntries , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* NULL is ignored */
            st = cpssDxChHsrPrpDdeAgingApply(dev, index , age_numOfEntries , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



            age_0 = 0;
            age_1 = 0;

            for(index = 0; index < maxDdeNumArr[mode]; index ++)
            {
                iterator = index;

                info.ddeKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
                info.ddeKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
                info.ddeKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
                info.ddeKey.macAddr.arEther[3] = (GT_U8) iterator;
                info.ddeKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
                info.ddeKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
                info.ddeKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
                info.ddeKey.lreInstance        = iterator & 0x3;
                info.destBmp = iterator & 0x3ff;
                info.srcHPort = iterator % 10;
                info.timeInMicroSec = (iterator & 0x3FFFFF)*10;
                for(ii = 0 ; ii < 10 ; ii++)
                {
                    info.destHportCounters[ii] = (iterator+ii) & 3;
                }
                info.ageBit = iterator & 1;
                if(iterator & 1)
                {
                    /* number of entries with age = 1 */
                    age_1++;
                }
                else
                {
                    /* number of entries with age = 0 */
                    age_0++;
                }

                st = cpssDxChHsrPrpDdeEntryWrite(dev, index , &info);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }

            age_numOfEntries =  0xFFFFFFFF;   /*FULL table*/
            index = 7;
            st = cpssDxChHsrPrpDdeAgingApply(dev,
                index,/* index*/
                age_numOfEntries,/* numOfEntries - FULL table */
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* those that were in age 0 should be deleted */
            UTF_VERIFY_EQUAL0_PARAM_MAC(age_0, age_numOfEntriesDeleted);

            st = cpssDxChHsrPrpDdeAgingApply(dev,
                index,/* index*/
                age_numOfEntries,/* numOfEntries - FULL table */
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* those that were in age 1 (at start) should be deleted */
            UTF_VERIFY_EQUAL0_PARAM_MAC(age_1, age_numOfEntriesDeleted);

            index = maxDdeNumArr[mode];
            age_numOfEntries = 1;
            st = cpssDxChHsrPrpDdeAgingApply(dev,
                index,/* index*/
                age_numOfEntries,/* numOfEntries */
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            index = maxDdeNumArr[mode];
            age_numOfEntries =  0xFFFFFFFF;   /*FULL table*/
            st = cpssDxChHsrPrpDdeAgingApply(dev,
                index,/* index*/
                age_numOfEntries,/* numOfEntries */
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            index = maxDdeNumArr[mode] - 1;
            age_numOfEntries = 0;
            st = cpssDxChHsrPrpDdeAgingApply(dev,
                index,/* index*/
                age_numOfEntries,/* numOfEntries */
                &age_numOfEntriesDeleted);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /* restore */
        mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
        st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    index = 0;

    index = 0;
    age_numOfEntries = 1;
    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeAgingApply(dev, index , age_numOfEntries , &age_numOfEntriesDeleted);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeAgingApply(dev, index , age_numOfEntries , &age_numOfEntriesDeleted);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpDdeHashCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC   *entryKeyPtr,
    IN  GT_U32                          startBank,
    IN  GT_U32                          numOfBanks,
    OUT GT_U32                          hashArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpDdeHashCalc)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      iterator;
    GT_U32                                      ii;
    GT_U32                                      value;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    GT_U32                                      maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E+1];
    GT_U32                                      numEntries;
    CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC          entryKey;
    GT_U32                                      *bmpHitArray = NULL;/* dynamic allocated table */
    GT_U32                                      noSpace;/* number of keys without place in the table */
    GT_U32                                      hashArr[16];

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev,CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E,&numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E] = 0;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E ] = numEntries/2;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E ] = numEntries - numEntries/4;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E ] = numEntries - numEntries/8;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ] = numEntries;

        FREE_PTR_MAC(bmpHitArray);
        bmpHitArray = cpssOsMalloc(numEntries*sizeof(GT_U32) / 32);

        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(maxDdeNumArr[mode] != 0)
            {
                /* check how good is the hash */
                cpssOsMemSet(bmpHitArray,0,numEntries*sizeof(GT_U32) / 32);
                noSpace = 0;

                for(iterator = 0; iterator < maxDdeNumArr[mode] ; iterator ++)
                {
                    entryKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
                    entryKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
                    entryKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
                    entryKey.macAddr.arEther[3] = (GT_U8) iterator;
                    entryKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
                    entryKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
                    entryKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
                    entryKey.lreInstance        = iterator & 0x3;

                    st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    for(ii = 0 ; ii < 16 ; ii++)
                    {
                        st = prvCpssFieldValueGet(bmpHitArray,hashArr[ii],1,&value);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                        if(value)
                        {
                            /* already set */
                            continue;
                        }
                        /*set the bit for the first time*/
                        st = prvCpssFieldValueSet(bmpHitArray,hashArr[ii],1,1);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                        break;
                    }

                    if(ii == 16)
                    {
                        /* the key found no empty place */
                        noSpace ++;
                    }
                }

                if(noSpace > iterator/12)/* more than 8.5% */
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(iterator/12, noSpace);
                }
            }
            else
            {
                iterator = 0x12345678;

                entryKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
                entryKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
                entryKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
                entryKey.macAddr.arEther[3] = (GT_U8) iterator;
                entryKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
                entryKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
                entryKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
                entryKey.lreInstance        = iterator & 0x3;

                st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

                continue;
            }

            iterator = 0x12345678;

            entryKey.macAddr.arEther[0] = (GT_U8)(iterator >> 24);
            entryKey.macAddr.arEther[1] = (GT_U8)(iterator >> 16);
            entryKey.macAddr.arEther[2] = (GT_U8)(iterator >>  8);
            entryKey.macAddr.arEther[3] = (GT_U8) iterator;
            entryKey.macAddr.arEther[4] = (GT_U8)(iterator >> 13);
            entryKey.macAddr.arEther[5] = (GT_U8)(iterator >>  3);
            entryKey.seqNum             = iterator & 0xFFFF;/*16 bits*/
            entryKey.lreInstance        = iterator & 0x3;

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* start checking 'bad param' */

            entryKey.seqNum = BIT_16;
            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            entryKey.seqNum             = iterator & 0xFFFF;/*16 bits*/

            entryKey.lreInstance = BIT_2;
            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            entryKey.lreInstance        = iterator & 0x3;

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, NULL, 0 ,16 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 1 ,16 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,17 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 5 ,12 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 4 ,12 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 11 ,1 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,0 , &hashArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* restore */
        mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
        st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    FREE_PTR_MAC(bmpHitArray);

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpDdeHashCalc(dev, &entryKey, 0 ,16 , &hashArr[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    GT_U32                                      lsduOffset;
    GT_U32                                      lsduOffsetGet;

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
            lsduOffset = portNum & (BIT_6-1);
            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffset);
        }

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            lsduOffset = portNum & (BIT_6-1);
            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffset);
            UTF_VERIFY_EQUAL1_STRING_MAC(lsduOffset, lsduOffsetGet,
                               "get another lsduOffset than was set: %d", dev);
        }

        portNum = 0;
        lsduOffset = (BIT_6-1);
        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffset);

        lsduOffset = BIT_6;
        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, portNum, lsduOffset);

        lsduOffset = (BIT_6-1);

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        lsduOffset = (BIT_6-1);

        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        lsduOffset = (BIT_6-1);

        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    lsduOffset = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(dev, portNum, lsduOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum  = 0;
    GT_U32                                      lsduOffsetGet;

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

            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffsetGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    GT_U32                                      lsduOffset;
    GT_U32                                      lsduOffsetGet;

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
            lsduOffset = portNum & (BIT_6-1);
            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffset);
        }

        /* 1.1. For all active devices go over all available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
        {
            lsduOffset = portNum & (BIT_6-1);
            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffset);
            UTF_VERIFY_EQUAL1_STRING_MAC(lsduOffset, lsduOffset,
                               "get another lsduOffset than was set: %d", dev);
        }

        portNum = 0;
        lsduOffset = (BIT_6-1);
        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffset);

        lsduOffset = BIT_6;
        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, portNum, lsduOffset);

        lsduOffset = (BIT_6-1);

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        lsduOffset = (BIT_6-1);

        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        lsduOffset = (BIT_6-1);

        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    lsduOffset = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(dev, portNum, lsduOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum  = 0;
    GT_U32                                      lsduOffsetGet;

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

            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, lsduOffsetGet);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextGenericPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available eports. */
        while(GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /*
                1.2.2. Call with enablePtr [NULL] -- ON non-valid portNum.
                Expected: GT_BAD_PARAM / GT_BAD_PTR.
            */
            st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
            if(st != GT_BAD_PARAM)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, portNum);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(dev, portNum, &lsduOffsetGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

extern void prvCpssDxChTestFdbHashUT(IN GT_U32    fdbSize,
    IN CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT fdbPartitionMode);
UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_with_DDE_partitions)
{
    GT_STATUS                                   st    = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      fdbSize;
    GT_U32                                      supported = 0;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT    mode;
    GT_U32                                      maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E+1];
    GT_U32                                      numEntries;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChCfgTableNumEntriesGet(dev,CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E,&numEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E] = 0;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E ] = numEntries/2;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E ] = numEntries - numEntries/4;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E ] = numEntries - numEntries/8;
        maxDdeNumArr[CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ] = numEntries;

        fdbSize = numEntries;

        for(mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E ;
            mode <= CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E ;
            mode ++)
        {
            /* the prvCpssDxChTestFdbHashUT will set
                cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
               after setting the FDB table size
            */

            if(maxDdeNumArr[mode] == 0)
            {
                continue;
            }
            prvCpssDxChTestFdbHashUT(fdbSize,mode);
        }

        /* the restore for mode = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
            st = cpssDxChHsrPrpFdbPartitionModeSet(dev, mode);
           done inside prvCpssDxChTestFdbHashUT();
        */
        supported++;
        break;
    }

    if(!supported)
    {
        /* state that test was skipped */
        SKIP_TEST_MAC;
    }

}

/**
* @internal ddeEntryDump function
* @endinternal
*
* @brief   debug tool --- print FDB HSR/PRP single line (DDE format)
*
* @param[in] index                 - the index of the entry
* @param[in] entryPtr              - the entry
* @param[in] printHeader           - indication if this is first entry that
*                                    requires 'header'
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
static GT_VOID ddeEntryDump(
    IN GT_U32  index,
    IN CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC * entryPtr,
    IN GT_BOOL printHeader
)
{
    GT_U32  ii;
    /*  Example:

    index | mac addr        |seqNum|Ins|dHP Bmp |sHP|time    |A|  dHP counters
            (Hex)           |(Hex) |   |  (Hex) |   |(Hex)   | |
    *******************************************************************************
    000000|00:00:00:00:00:00| 0000 | 0 |  000   | 00|00000000|-|0,1,2,3,0,1,2,3,0,1
    000019|00:02:00:13:10:02| 0013 | 3 |  013   | 01|001400be|A|3,0,1,2,3,0,1,2,3,0
    000021|00:00:c0:15:06:02| c015 | 1 |  015   | 03|000780d2|A|1,2,3,0,1,2,3,0,1,2
    000042|00:01:80:2a:0c:05| 802a | 2 |  02a   | 06|000f01a4|-|2,3,0,1,2,3,0,1,2,3
    000044|00:00:40:2c:02:05| 402c | 0 |  02c   | 08|000281b8|-|0,1,2,3,0,1,2,3,0,1
    000063|00:02:40:3f:12:07| 403f | 3 |  03f   | 09|00168276|A|3,0,1,2,3,0,1,2,3,0
    */
    if (printHeader == GT_TRUE)
    {
        static GT_BOOL isFirst = GT_TRUE;
        if(isFirst == GT_TRUE)
        {
            cpssOsPrintf("DDE entry format \n"
            "Ins  = lreInstance \n"
            "sHP  = srcHPort \n"
            "A    = Aging \n"
            "dHP  = destHport \n"
            );
            isFirst = GT_FALSE;
        }

        cpssOsPrintf(
        "index | mac addr        |seqNum|Ins|dHP Bmp |sHP|time    |A|  dHP counters \n"
        "         (Hex)          |(Hex) |   |  (Hex) |   |(Hex)   | |               \n"
        "*******************************************************************************\n"
            );
    }

    cpssOsPrintf("%6.6d|",index);
    cpssOsPrintf("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
         entryPtr->ddeKey.macAddr.arEther[0],
         entryPtr->ddeKey.macAddr.arEther[1],
         entryPtr->ddeKey.macAddr.arEther[2],
         entryPtr->ddeKey.macAddr.arEther[3],
         entryPtr->ddeKey.macAddr.arEther[4],
         entryPtr->ddeKey.macAddr.arEther[5]);
    cpssOsPrintf("|");
    cpssOsPrintf(" %4.4x ", entryPtr->ddeKey.seqNum);
    cpssOsPrintf("|");
    cpssOsPrintf(" %1.1d ", entryPtr->ddeKey.lreInstance);
    cpssOsPrintf("|");
    cpssOsPrintf("  %3.3x   ", entryPtr->destBmp);
    cpssOsPrintf("|");
    cpssOsPrintf(" %2.2d", entryPtr->srcHPort);
    cpssOsPrintf("|");
    cpssOsPrintf("%8.8x", entryPtr->timeInMicroSec);
    cpssOsPrintf("|");
    cpssOsPrintf("%s", entryPtr->ageBit == GT_TRUE ? "A" : "-");
    cpssOsPrintf("|");
    for(ii = 0 ; ii < 9 ; ii++)
    {
        cpssOsPrintf("%1.1d,",entryPtr->destHportCounters[ii]);
    }
    /* last one without ',' */
    cpssOsPrintf("%1.1d",entryPtr->destHportCounters[ii]);

    cpssOsPrintf("\n");
}

/**
* @internal internal_cpssDxChHsrPrpFdbDump function
* @endinternal
*
* @brief   debug tool --- print FDB HSR/PRP entries
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
static GT_STATUS internal_cpssDxChHsrPrpFdbDump(
    IN GT_U8 devNum,
    IN GT_BOOL countOnly
)
{
    GT_STATUS   rc;
    GT_U32  index = 0;
    GT_BOOL  isValid;
    CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC  entry;
    GT_U32  validNum = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

#ifdef _WIN32
    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        cpssOsPrintf("Sorry , but the GM on WINDOWs is so slow that we can't iterate on it's FDB entries (and stuck in this debug function) \n");
        return GT_OK; /* allow the LUA test to call this function without fail and without stuck in it */
    }
#endif /*_WIN32*/

    cpssOsPrintf("\nStart FDB HSR/PRP partition scan\n");

    do
    {
        rc = cpssDxChHsrPrpDdeEntryRead(devNum,index,&entry,&isValid);
        if (rc != GT_OK)
        {
            /* break on invalid index */
            break;
        }

        if(isValid == GT_FALSE)
        {
            index++;
            continue;
        }

        if(countOnly == GT_FALSE)
        {
            ddeEntryDump(index, &entry,
                validNum == 0 ? GT_TRUE : GT_FALSE/* printHeader*/);
        }

        index++;
        validNum++;
    }while(1);

    if(validNum == 0)
    {
        cpssOsPrintf("Finished FDB HSR/PRP scan of [%d K] entries , there are no valid entries \n",(index/_1K));
    }
    else
    {
        cpssOsPrintf("Finished FDB HSR/PRP scan of [%d K] entries. there are [%d] valid entries \n",(index/_1K), validNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpFdbDump function
* @endinternal
*
* @brief   debug tool --- print valid FDB HSR/PRP entries (and print total count)
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpFdbDump(
    IN GT_U8 devNum
)
{
    return internal_cpssDxChHsrPrpFdbDump(devNum,GT_FALSE/* to dump , not only count */);
}
/**
* @internal cpssDxChHsrPrpFdbCount function
* @endinternal
*
* @brief   debug tool --- pritn the count of valid FDB HSR/PRP entries.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpFdbCount(
    IN GT_U8 devNum
)
{
    return internal_cpssDxChHsrPrpFdbDump(devNum,GT_TRUE/* to only count */);
}

/**
* @internal cpssDxChHsrPrpHportDump function
* @endinternal
*
* @brief   debug tool --- print HSR/PRP 10 HPorts entries
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpHportDump(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc;
    GT_U32      hPort;
    CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    info;
    GT_U64                               counters[3];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    cpssOsPrintf("HPort entry format : \n"
    "PNT lu mode    = PNT lookup mode (pntLookupMode) :\n"
    "   SKIP - skip                  \n"
    "   S_L  - src lu and  learn     \n"
    "   D_S  - dest lu and src lu    \n"
    "   S    - src lu                \n"
    "FDB/DDE mode   = SKIP / DDE / FDB lookup (fdbDdlMode)  \n"
    "PNT learn tag  = true/---- (pntLearnTaggedPackets)     \n"
    "ACC same port  = true/---- (acceptSamePort)            \n"
    "R              = A/B (lreRingPortType)     \n"
    "Ins            = LRE instance              \n"
    "def frw bmp    = (defaultForwardingBmp)    \n"
    "src is me bmp  = (srcIsMeBmp)              \n"
    "src DD bmp     = (srcDuplicateDiscardBmp)  \n"
    "dst to me bmp  = (destToMeForwardingBmp)   \n"
    "\n");

/*
hPort | PNT  | FDB  | PNT | ACC | R |Ins| def | src | src | dst |
      | lu   | DDL  |learn| same|   |   | fwr |is me| DD  |to me|
      | mode | mode | tag | port|   |   | bmp | bmp | bmp | bmp |
      |      |      |     |     |   |   |(HEX)|(HEX)|(HEX)|(HEX)|
================================================================
    0 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    1 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    2 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    3 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    4 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    5 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    6 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    7 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    8 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
    9 | SKIP | SKIP | --- | --- | A | 0 | 000 | 000 | 000 | 000 |
*/

    cpssOsPrintf(
"hPort | PNT  | FDB  | PNT | ACC | R |Ins|def|src|src|dst|  cnt   |  cnt   |  cnt   \n"
"      | lu   | DDL  |learn| same|   |   |fwr|is |DD |to |   1    |   2    |   3    \n"
"      | mode | mode | tag | port|   |   |   |me |   |me |        |        |        \n"
"      |      |      |     |     |   |   |bmp|   |bmp|bmp| bmp    | bmp    | bmp    \n"
"      |      |      |     |     |   |   |HEX|HEX|HEX|HEX| HEX    | HEX    | HEX    \n"
"===================================================================================\n"
    );


    for(hPort = 0 ; hPort < 10 ; hPort++)
    {
        rc = cpssDxChHsrPrpHPortEntryGet(devNum,hPort,
            &info,
            &counters[0],
            &counters[1],
            &counters[2]);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("    %1.1d | ",hPort);
        cpssOsPrintf("%s | ",
            info.pntLookupMode == CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SKIP_E          ? "SKIP" :
            info.pntLookupMode == CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_AND_LEARN_E ? "S_L " :
            info.pntLookupMode == CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_DST_AND_SRC_E   ? "D_S " :
            info.pntLookupMode == CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_E           ? "S   " :
            "??? ");
        cpssOsPrintf("%s | ",
            info.fdbDdlMode == CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_SKIP_E ? "SKIP" :
            info.fdbDdlMode == CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_DDL_E  ? "DDL " :
            info.fdbDdlMode == CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_FDB_E  ? "FDB " :
            "???");
        cpssOsPrintf("%s| ",
            info.pntLearnTaggedPackets == GT_TRUE ? "true" :
            "----");
        cpssOsPrintf("%s| ",
            info.acceptSamePort == GT_TRUE ? "true" :
            "----");
        cpssOsPrintf("%s | ",
            info.lreRingPortType == CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_A_E ? "A" :
            "B");
        cpssOsPrintf("%1.1d |",
            info.lreInstance);
        cpssOsPrintf("%3.3x|",
            info.defaultForwardingBmp);
        cpssOsPrintf("%3.3x|",
            info.srcIsMeBmp);
        cpssOsPrintf("%3.3x|",
            info.srcDuplicateDiscardBmp);
        cpssOsPrintf("%3.3x|",
            info.destToMeForwardingBmp);
        cpssOsPrintf("%8.8x|",
            counters[0].l[0]);
        cpssOsPrintf("%8.8x|",
            counters[1].l[0]);
        cpssOsPrintf("%8.8x",
            counters[2].l[0]);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpPntDump function
* @endinternal
*
* @brief   debug tool --- print HSR/PRP PNT valid/ready entries
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpPntDump(
    IN GT_U8 devNum
)
{
    GT_STATUS                           rc;
    GT_U32                              index;
    CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC     info;
    CPSS_DXCH_HSR_PRP_PNT_STATE_ENT     state;
    GT_U32  readyNum = 0 , validNum = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    cpssOsPrintf("HPort entry format : \n"
    "E    = entry state : V-Valid , R-Ready \n"
    "A    = Aging  : 'A'/'-'    \n"
    "T    = Tagged : 'T'/'U' (tagged/untagged) \n"
    "Ins  = LRE instance        \n"
    "dst bmp = (destBmp)        \n"
    );

/*
index | E |stream| mac address      |A|T|Ins| dst
      |   |  id  |                  | | |   | bmp
      |   |      |                  | | |   | HEX
=================================================
  000 | R | fe7f |  -- -- -- -- --  |-|-|---|---
  017 | V | fe6e | 88:99:77:aa:01:91|A|T| 0 |26f
  051 | R | fe4c |  -- -- -- -- --  |-|-|---|---
  068 | V | fe3b | 88:99:77:aa:01:c4|-|T| 1 |23c
  102 | R | fe19 |  -- -- -- -- --  |-|-|---|---
  119 | V | fe08 | 88:99:77:aa:01:f7|A|U| 1 |209
*/

    cpssOsPrintf(
"index | E |stream| mac address      |A|T|Ins| dst \n"
"      |   |  id  |                  | | |   | bmp \n"
"      |   |      |                  | | |   | HEX \n"
"================================================= \n"
    );

    for(index = 0 ; index < 128 ; index ++)
    {
        rc = cpssDxChHsrPrpPntEntryGet(devNum,index,&state,&info);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(state == CPSS_DXCH_HSR_PRP_PNT_STATE_NOT_VALID_E)
        {
            /* we print only : valid , ready entries */
            continue;
        }

        cpssOsPrintf("  %3.3d | ",index);
        cpssOsPrintf("%s | ",
            state == CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E ? "R" :
            state == CPSS_DXCH_HSR_PRP_PNT_STATE_VALID_E ? "V" :
            "?");

        cpssOsPrintf("%4.4x | ",
            info.streamId);

        if(state == CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E)
        {
            cpssOsPrintf(" -- -- -- -- --  |-|-|---|--- \n");

            readyNum++;
            continue;
        }

        validNum++;

        cpssOsPrintf("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
            info.macAddr.arEther[0],
            info.macAddr.arEther[1],
            info.macAddr.arEther[2],
            info.macAddr.arEther[3],
            info.macAddr.arEther[4],
            info.macAddr.arEther[5]);
        cpssOsPrintf("|");

        cpssOsPrintf("%s|",
            info.ageBit   == GT_TRUE ? "A" : "-");
        cpssOsPrintf("%s|",
            info.untagged == GT_TRUE ? "U" : "T");
        cpssOsPrintf(" %1.1d |",
            info.lreInstance);
        cpssOsPrintf("%3.3x",
            info.destBmp);
        cpssOsPrintf("\n");
    }

    if(validNum == 0 && readyNum == 0)
    {
        cpssOsPrintf("There are no valid/ready entries (out of [%d] entries)\n",index);
    }
    else
    {
        cpssOsPrintf("There are valid [%d] entries , ready [%d] entries (out of [%d] entries) \n",
            validNum,
            readyNum,
            index);
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpLreInstanceDump function
* @endinternal
*
* @brief   debug tool --- print HSR/PRP 4 LRE instances
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpLreInstanceDump(
    IN GT_U8 devNum
)
{
    GT_STATUS                                   rc;
    GT_U32                                      index;
    CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC    info;
    GT_U64                                      counters[2];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    cpssOsPrintf("lre instance entry format : \n"
    "n_p    = num of ports \n"
    "low hp = lowest HPort \n"
    );

/*
index |n_p |low | evidx | counter | counter
      |    |hp  | map   |    A    |   B
      |    |    | base  |  bmp    | bmp
      |    |    | HEX   |  HEX    | HEX
============================================
   0  |  0 |  0 |  0000 | 00000000|00000000
   1  |  3 |  7 |  1ff7 | 00000000|00000000
   2  |  2 |  8 |  1ffb | 00000000|00000000
   3  |  0 |  9 |  1ffe | 00000000|00000000
*/
    cpssOsPrintf(
"index |n_p |low | evidx | counter | counter \n"
"      |    |hp  | map   |    A    |   B     \n"
"      |    |    | base  |  bmp    | bmp     \n"
"      |    |    | HEX   |  HEX    | HEX     \n"
"============================================\n"
    );


    for(index = 0 ; index < 4 ; index++)
    {
        rc = cpssDxChHsrPrpLreInstanceEntryGet(devNum,index,
            &info,
            &counters[0],
            &counters[1]);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("   %1.1d  | ",index);
        cpssOsPrintf(" %1.1d | ",
            info.numOfPorts);
        cpssOsPrintf(" %1.1d | ",
            info.lowestHPort);
        cpssOsPrintf(" %4.4x | ",
            info.eVidxMappingBase);
        cpssOsPrintf("%8.8x|",
            counters[0].l[0]);
        cpssOsPrintf("%8.8x ",
            counters[1].l[0]);

        cpssOsPrintf("\n");
    }

    return GT_OK;
}


#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#ifdef CPSS_LOG_ENABLE
extern const char * prvCpssLogEnum_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT[];
extern const char * prvCpssLogEnum_CPSS_PACKET_CMD_ENT[] ;
extern const char * prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT[];
extern const char * prvCpssLogEnum_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT[];
extern const char * prvCpssLogEnum_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT[];
extern const char * prvCpssLogEnum_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT[];
extern const char * prvCpssLogEnum_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT[];
#endif /*CPSS_LOG_ENABLE*/
/**
* @internal cpssDxChHsrPrpGlobalDump function
* @endinternal
*
* @brief   debug tool --- print HSR/PRP globals info
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpGlobalDump(
    IN GT_U8 devNum
)
{
    GT_STATUS                    rc;
    char                        *str1;
    char                        *str2;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT   exception_type;
    CPSS_PACKET_CMD_ENT                    exception_command;
    CPSS_NET_RX_CPU_CODE_ENT               exception_cpuCode;
    CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT     bmpType;
    CPSS_PORTS_SHORT_BMP_STC               bmp;
    GT_PORT_NUM                            baseEport;
    CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT     counterType;
    GT_U64                                 counters[3];
    GT_U32                                 threshold;
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT fdbPartitionMode;
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT ddeTimerGranularityMode;
    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT   timeField;
    GT_U32                                 timeInMicroSec;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

/* example :
[CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E] : command [CPSS_PACKET_CMD_DROP_SOFT_E] cpu code [563] (HW CPU code) [0xff]
[CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E] : command [CPSS_PACKET_CMD_DROP_SOFT_E] cpu code [563] (HW CPU code) [0xff]
[CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E] : command [CPSS_PACKET_CMD_DROP_SOFT_E] cpu code [563] (HW CPU code) [0xff]
[CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E] : command [CPSS_PACKET_CMD_DROP_SOFT_E] cpu code [563] (HW CPU code) [0xff]
[CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E] :  bmp [0x3ff]
[CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_2_E] :  bmp [0x010]
[CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_3_E] :  bmp [0x011]
[CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_DDL_DECISION_E] :  bmp [0x012]
[CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_SOURCE_DUPLICATION_DISCARD_E] :  bmp [0x013]
[CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E] :  bmp [0x014]
baseEport :  [0x0400]
[CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E] :  counter [0x00000000]
[CPSS_DXCH_HSR_PRP_COUNTER_OVERRIDEN_NON_EXPIRED_ENTRIES_E] :  counter [0x00000000]
Num Of Ready Entries Threshold :  [0x3ff]
FDB partition mode :  [CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E]
DDE timer granularity mode :  [CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E]
[CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E] :  [0x027ffff6] (in micro seconds)
[CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_EXPIRATION_E] :  [0x027ffff6] (in micro seconds)
[CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E] :  [0x009038a0] (in micro seconds)*/


    str1 = "no name";
    str2 = "no name";

    for(exception_type = CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E;
        exception_type <= CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E;
        exception_type++)
    {
        rc = cpssDxChHsrPrpExceptionCommandGet(devNum,exception_type,&exception_command);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChHsrPrpExceptionCpuCodeGet(devNum,exception_type,&exception_cpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
#ifdef CPSS_LOG_ENABLE
        str1    = (char*)(prvCpssLogEnum_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT[exception_type]);
        str2 = (char*)(prvCpssLogEnum_CPSS_PACKET_CMD_ENT[exception_command]);
#endif /*CPSS_LOG_ENABLE*/

        rc = prvCpssDxChNetIfCpuToDsaCode(exception_cpuCode,&dsaCpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("[%s] : command [%s] cpu code [%d] (HW CPU code) [0x%2.2x]\n",
            str1,str2,exception_cpuCode,dsaCpuCode);
    }

    for(bmpType = CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E ;
        bmpType <= CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E;
        bmpType++)
    {
        rc = cpssDxChHsrPrpPortsBmpGet(devNum,bmpType,&bmp);
        if (rc != GT_OK)
        {
            return rc;
        }
#ifdef CPSS_LOG_ENABLE
        str1    = (char*)(prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT[bmpType]);
#endif /*CPSS_LOG_ENABLE*/
        cpssOsPrintf("[%s] :  bmp [0x%3.3x] \n",
            str1,bmp);
    }

    rc = cpssDxChHsrPrpHPortSrcEPortBaseGet(devNum,&baseEport);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("baseEport :  [0x%4.4x] \n",
        baseEport);

    for(counterType =  CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E ;
        counterType <= CPSS_DXCH_HSR_PRP_COUNTER_OVERRIDEN_NON_EXPIRED_ENTRIES_E ;
        counterType++)
    {
        rc = cpssDxChHsrPrpCounterGet(devNum,counterType,&counters[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
#ifdef CPSS_LOG_ENABLE
        str1 = (char*)(prvCpssLogEnum_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT[counterType]);
#endif /*CPSS_LOG_ENABLE*/
        cpssOsPrintf("[%s] :  counter [0x%8.8x] \n",
            str1,counters[0].l[0]);
    }

    rc = cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(devNum,&threshold);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("Num Of Ready Entries Threshold :  [0x%3.3x] \n",
        threshold);

    rc = cpssDxChHsrPrpFdbPartitionModeGet(devNum,&fdbPartitionMode);
    if (rc != GT_OK)
    {
        return rc;
    }
#ifdef CPSS_LOG_ENABLE
        str1 = (char*)(prvCpssLogEnum_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT[fdbPartitionMode]);
#endif /*CPSS_LOG_ENABLE*/
    cpssOsPrintf("FDB partition mode :  [%s] \n",
        str1);

    rc = cpssDxChHsrPrpDdeTimerGranularityModeGet(devNum,&ddeTimerGranularityMode);
    if (rc != GT_OK)
    {
        return rc;
    }
#ifdef CPSS_LOG_ENABLE
        str1 = (char*)(prvCpssLogEnum_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT[ddeTimerGranularityMode]);
#endif /*CPSS_LOG_ENABLE*/
    cpssOsPrintf("DDE timer granularity mode :  [%s] \n",
        str1);

    for(timeField =  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E;
        timeField <= CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E;
        timeField ++)
    {
        rc = cpssDxChHsrPrpDdeTimeFieldGet(devNum,timeField,&timeInMicroSec);
        if (rc != GT_OK)
        {
            return rc;
        }
#ifdef CPSS_LOG_ENABLE
        str1 = (char*)(prvCpssLogEnum_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT[timeField]);
#endif /*CPSS_LOG_ENABLE*/
        cpssOsPrintf("[%s] :  [0x%8.8x] (in micro seconds) \n",
            str1,timeInMicroSec);
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpPortDump function
* @endinternal
*
* @brief   debug tool --- print HSR/PRP 'per port' info
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on bad parameters,
*/
GT_STATUS cpssDxChHsrPrpPortDump(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum
)
{
    GT_STATUS   rc;
    GT_U32      maxPhysicalPort,maxDefaultEPort;
    GT_BOOL     enable;
    GT_U32      value;
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT    actionGet;
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT      paddingMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    maxPhysicalPort = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    maxDefaultEPort = PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum);

    if(portNum < maxPhysicalPort)
    {
        cpssOsPrintf("portNum[%d] : is in physical port range (and default eports , and eports) \n",
            portNum);
    }
    else
    if(portNum < maxDefaultEPort)
    {
        cpssOsPrintf("portNum[%d] : is in default eports range (and eports) , but not in physical ports range \n",
            portNum);
    }
    else
    {
        cpssOsPrintf("portNum[%d] : is in eports range , but not in physical ports range and not in default eports range \n",
            portNum);
    }

    if(portNum < maxDefaultEPort)
    {
        /*'Default eport' range*/
        rc = cpssDxChHsrPrpPortPrpEnableGet(devNum,portNum,&enable);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : Prp [%s] \n",
            portNum,
            enable == GT_TRUE ? "Enabled" : "Disabled");
    }

    /*For Ingress direction :
      APPLICABLE RANGE: 'Default eport' range.
    For Egress direction :
      APPLICABLE RANGE: eport range.
    */
    if(portNum < maxDefaultEPort)
    {
        rc = cpssDxChHsrPrpPortPrpLanIdGet(devNum,portNum,
                CPSS_DIRECTION_INGRESS_E,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : ingress PRP lanId [%d] \n",
            portNum,
            value);
    }

    rc = cpssDxChHsrPrpPortPrpLanIdGet(devNum,portNum,
            CPSS_DIRECTION_EGRESS_E,&value);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("portNum[%d] : egress PRP lanId [%d] \n",
        portNum,
        value);

    if(portNum < maxDefaultEPort)
    {
        /*'Default eport' range*/
        rc = cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(devNum,portNum,&enable);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : PRP Treat Wrong LanId As Rct Exists [%s] \n",
            portNum,
            enable == GT_TRUE ? "Enabled" : "Disabled");
    }

    if(portNum < maxPhysicalPort)
    {
        /*physical port range.*/
        rc = cpssDxChHsrPrpPortPrpTrailerActionGet(devNum,portNum,&actionGet);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : PRP Trailer Action [%s] \n",
            portNum,
            actionGet == CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E ? "None" :
            actionGet == CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ADD_PRP_E ? "Add" :
            "Remove");

        /*physical port range.*/
        rc = cpssDxChHsrPrpPortPrpPaddingModeGet(devNum,portNum,&paddingMode);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : PRP padding Mode [%s] \n",
            portNum,
            paddingMode == CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E ? "untagged 60 tagged 64" :
            paddingMode == CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_60_E ? "all 60" :
            "all 64");

        /*physical port range.*/
        rc = cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(devNum,portNum,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : target port PRP Base LSDU Offset [%d] \n",
            portNum,
            value);

        /*physical port range.*/
        rc = cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(devNum,portNum,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("portNum[%d] : source port HSR Base LSDU Offset [%d] \n",
            portNum,
            value);
    }

    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChHsrPrp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChHsrPrp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpExceptionCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpExceptionCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpExceptionCpuCodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpLanIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpLanIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortsBmpSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortsBmpGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpHPortSrcEPortBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpHPortSrcEPortBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpHPortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpHPortEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPntEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPntEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPntAgingApply)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpLreInstanceEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpLreInstanceEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpFdbPartitionModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpFdbPartitionModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeTimerGranularityModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeTimerGranularityModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeTimeFieldSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeTimeFieldGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpTrailerActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpTrailerActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpPaddingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortPrpPaddingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeEntryInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeAgingApply)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpDdeHashCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_with_DDE_partitions)
UTF_SUIT_END_TESTS_MAC(cpssDxChHsrPrp)


