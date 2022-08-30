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
* @file cpssDxChPortPipUT.c
*
* @brief Unit tests for cpssDxChPortPip, that provides
* CPSS implementation for Pre-Ingress Prioritization (PIP) .
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* PIP : prepare iterator for go over all active devices */
#define UT_PIP_RESET_DEV(_devPtr)  \
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC((_devPtr),     \
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | \
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |      \
        UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E)

/* PIP : sip6 : prepare iterator for go over all active devices */
#define UT_PIP_SIP6_RESET_DEV(_devPtr)  \
    PRV_TGF_APPLIC_DEV_RESET_MAC((_devPtr),     \
        UTF_CPSS_PP_ALL_SIP6_CNS)


extern GT_U32 utfFamilyTypeGet(IN GT_U8 devNum);
/* add additional macro to treat devices like Ironman that not hold dedicated
   family in UTF_PP_FAMILY_BIT_ENT , and uses the UTF_AC5X_E */
#define PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC                             \
    if(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pipInfo.isIaUnitNotSupported == GT_TRUE) \
    {                                                                    \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,utfFamilyTypeGet(prvTgfDevNum));         \
    }



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet  = GT_FALSE;

    /* there is no RXDMA/IA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(enable = GT_FALSE ; enable <= GT_TRUE ; enable++)
        {
            st = cpssDxChPortPipGlobalEnableSet(dev, enable);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enable);
            }

            st = cpssDxChPortPipGlobalEnableGet(dev, &enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEnableGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with stack [IPv4, IPv6, IPv46].
           Expected: GT_OK.
        */

        st = cpssDxChPortPipGlobalEnableGet(dev,  &enable);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        st = cpssDxChPortPipGlobalEnableGet(dev, NULL);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, enable);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enable);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_U32       maxIndexArr[CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E+1] = {
/*CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E */ 4,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E     */ 2,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E     */ 1,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E     */ 1,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E      */ 4,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E*/4

};
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEtherTypeProtocolSet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol;
    GT_U32       index;
    GT_U16       etherType;
    GT_U16       etherTypeGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        etherType = 0x5555;
        for(protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E ;
            protocol <= CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E ;
            protocol ++)
        {
            if(protocol >= CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E &&
               !PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* invalid protocol */
                index     = 0;
                etherType = 0;
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index,etherType);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
                continue;
            }

            /* check valid protocol */
            for(index = 0 ; index < maxIndexArr[protocol] ; index ++)
            {
                etherType++;

                /* check valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, protocol,index,etherType);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
            }

            if(maxIndexArr[protocol] > 1)
            {
                /* check NOT valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index,etherType);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
            }
            else
            {
                /* index is ignored anyway */
                index = BIT_31;
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,(etherType+0x7777));
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, protocol,index,(etherType+0x7777));
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,(etherType+0x7777));
                }
            }

        }

        /* check that all values are 'saved' */
        etherType = 0x5555;
        for(protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E ;
            protocol <= CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E ;
            protocol ++)
        {
            if(protocol >= CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E &&
               !PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                continue;
            }

            /* check valid protocol */
            for(index = 0 ; index < maxIndexArr[protocol] ; index ++)
            {
                etherType++;

                /* check valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, protocol,index,etherType);
                    /* compare results */
                    if(maxIndexArr[protocol] > 1)
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(etherType, etherTypeGet, dev, protocol,index,etherType);
                    }
                    else
                    {
                        /* the 0x7777 came due to override when index == BIT_31*/
                        UTF_VERIFY_EQUAL4_PARAM_MAC((etherType+0x7777), etherTypeGet, dev, protocol,index,etherType);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
            }
        }

        etherType = 0x8888;
        index = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalEtherTypeProtocolSet
                            (dev, protocol,index,etherType),
                            protocol);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E;
    index = 0;
    etherType = 0x6666;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index,etherType);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEtherTypeProtocolGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol;
    GT_U32       index;

    GT_U16       etherTypeGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E ;
            protocol <= CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E ;
            protocol ++)
        {
            if(protocol >= CPSS_DXCH_PORT_PIP_PROTOCOL_LATENCY_SENSITIVE_E &&
               !PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* invalid protocol */
                index     = 0;
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
                continue;
            }

            /* check valid protocol */
            for(index = 0 ; index < maxIndexArr[protocol] ; index ++)
            {
                /* check valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }

                /* check NULL pointer */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,NULL);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
            }

            if(maxIndexArr[protocol] > 1)
            {
                /* check NOT valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
            }
            else
            {
                /* index is ignored anyway */
                index = BIT_31;
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
            }
        }


        index = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalEtherTypeProtocolGet
                            (dev, protocol,index,&etherTypeGet),
                            protocol);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E;
    index = 0;


    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipTrustEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     trust;
    GT_BOOL     trustGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for(trust = GT_FALSE ; trust <= GT_TRUE ; trust ++)
            {
                st = cpssDxChPortPipTrustEnableSet(dev,port,trust);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,trust);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,trust);
                }

                st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                    /* compare results */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(trust, trustGet, dev,port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
                }

            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        trust = GT_TRUE;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipTrustEnableSet(dev,port,trust);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,trust);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,trust);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    trust = GT_TRUE;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipTrustEnableSet(dev, port,trust);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipTrustEnableSet(dev, port,trust);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipTrustEnableGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     trustGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipTrustEnableGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipTrustEnableGet(dev, port,&trustGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipTrustEnableGet(dev, port,&trustGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipProfileSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     pipProfile;
    GT_U32     pipProfileGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for(pipProfile = 0 ; pipProfile < 4 ; pipProfile ++)
            {
                st = cpssDxChPortPipProfileSet(dev,port,pipProfile);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,pipProfile);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,pipProfile);
                }

                st = cpssDxChPortPipProfileGet(dev,port,&pipProfileGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                    /* compare results */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(pipProfile, pipProfileGet, dev,port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
                }

            }

            /* check not valid profile */
            st = cpssDxChPortPipProfileSet(dev,port,pipProfile);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,port,pipProfile);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,pipProfile);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        pipProfile = 0;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipProfileSet(dev,port,pipProfile);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,pipProfile);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,pipProfile);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    pipProfile = 0;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipProfileSet(dev, port,pipProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipProfileSet(dev, port,pipProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipProfileGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     pipProfileGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipProfileGet(dev,port,&pipProfileGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* check NULL */
            st = cpssDxChPortPipProfileGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipProfileGet(dev,port,&pipProfileGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipProfileGet(dev, port,&pipProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipProfileGet(dev, port,&pipProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipVidClassificationEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for(enable = GT_FALSE ; enable <= GT_TRUE ; enable ++)
            {
                st = cpssDxChPortPipVidClassificationEnableSet(dev,port,enable);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,enable);
                }

                st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                    /* compare results */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet, dev,port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
                }

            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        enable = GT_TRUE;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipVidClassificationEnableSet(dev,port,enable);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,enable);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipVidClassificationEnableSet(dev, port,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipVidClassificationEnableSet(dev, port,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipVidClassificationEnableGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = 0;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipVidClassificationEnableGet(dev, port,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipVidClassificationEnableGet(dev, port,&enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipPrioritySet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     priority;
    GT_U32     priorityGet;
    GT_U32     maxPriority;
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
        }
        else
        {
            maxPriority = 3;
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E ;
                type <= CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E ;
                type ++)
            {
                /* check valid priority */
                for(priority = 0 ; priority < maxPriority ; priority++)
                {
                    st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,type);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                    }

                    st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,type);
                        /* compare results */
                        UTF_VERIFY_EQUAL3_PARAM_MAC(priority, priorityGet, dev,port,type);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                    }
                }

                /* check invalid priority */
                st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,port,type);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                }

            }


            /* check not valid type */
            priority = 0;
            st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,type);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
            }

            /*
                1.3. Call with wrong enum values type.
                Expected: NOT GT_OK.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPipPrioritySet
                                (dev,port,type,priority),
                                type);

        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;
        priority = 0;

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,priority);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,priority);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    priority = 0;
    port = 0;
    type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipPriorityGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     priorityGet;
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E ;
                type <= CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E ;
                type ++)
            {
                st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,type);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                }

                /* check NULL pointer */
                st = cpssDxChPortPipPriorityGet(dev,port,type,NULL);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev,port,type);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                }

            }

            /* check not valid type */
            st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,type);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
            }

            /*
                1.3. Call with wrong enum values type.
                Expected: NOT GT_OK.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPipPriorityGet
                                (dev,port,type,&priorityGet),
                                type);

        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;
    type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_U32       maxFieldIndexArr[CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E+1] = {
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E*/ 128,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E*/            8,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E*/       8,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E*/          8,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E*/        256,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E*/         256,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E*/         4,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E */     4
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalProfilePrioritySet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32                  pipProfile;
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type;
    GT_U32                  fieldIndex;
    GT_U32                  priority;
    GT_U32                  priorityGet;
    GT_U32                  maxPriority;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
        }
        else
        {
            maxPriority = 3;
        }

        /* check valid profile */
        for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                type++)
            {
                /* check valid fieldIndex */
                for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                {
                    if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                        fieldIndex >= 64 && /* instead of 256 */
                        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                    {
                        break;
                    }

                    /* check valid priority */
                    for(priority = 0 ; priority < maxPriority ; priority++)
                    {
                        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                    }

                    /* check not valid priority */
                    st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, pipProfile,type,fieldIndex,priority);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                    }

                }/*fieldIndex*/

                priority = 0;
                /* check not valid fieldIndex */
                st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex,priority);
                }
                else
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                }
            }/*type*/

            priority = 0;
            fieldIndex = 0;
            /* check not valid type */
            st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex,priority);
            }
            else
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
            }

        }/*pipProfile*/

        priority = 0;
        fieldIndex = 0;
        type = 0;
        /* check not valid pipProfile */
        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex,priority);
        }
        else
        {
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
        }


        /* check that values are saved */
        /* set different profile to different fields ... and check at the end that
           the 'get' values match */
        {
            priority = 0;
            for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
            {
                for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                    type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                    type++)
                {
                    for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                    {
                        if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                            fieldIndex >= 64 && /* instead of 256 */
                            GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                                PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                        {
                            break;
                        }
                        priority++;
                        priority %= maxPriority;
                        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                    }/*fieldIndex*/
                }/*type*/
            }/*pipProfile*/

            priority = 0;
            for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
            {
                for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                    type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                    type++)
                {
                    for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                    {
                        if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                            fieldIndex >= 64 && /* instead of 256 */
                            GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                                PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                        {
                            break;
                        }
                        priority++;
                        priority %= maxPriority;
                        st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
                        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex);
                            /* compare the results */
                            UTF_VERIFY_EQUAL4_PARAM_MAC(priority, priorityGet, dev, pipProfile,type,fieldIndex);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
                        }
                    }/*fieldIndex*/
                }/*type*/
            }/*pipProfile*/
        }

        pipProfile = 0;
        fieldIndex = 0;
        priority = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalProfilePrioritySet
                            (dev ,pipProfile,type,fieldIndex,priority),
                            type);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    pipProfile = 0;
    fieldIndex = 0;
    priority = 0;
    type = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalProfilePriorityGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32                  pipProfile;
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type;
    GT_U32                  fieldIndex;
    GT_U32                  priorityGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check valid profile */
        for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                type++)
            {
                /* check valid fieldIndex */
                for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                {
                    if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                        fieldIndex >= 64 && /* instead of 256 */
                        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                    {
                        break;
                    }

                    st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
                    }
                }/*fieldIndex*/

                /* check not valid fieldIndex */
                st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
                }
            }/*type*/

            fieldIndex = 0;
            /* check not valid type */
            st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
            }

        }/*pipProfile*/

        fieldIndex = 0;
        type = 0;
        /* check not valid pipProfile */
        st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
        }

        pipProfile = 0;
        fieldIndex = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalProfilePriorityGet
                            (dev ,pipProfile,type,fieldIndex,&priorityGet),
                            type);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    pipProfile = 0;
    fieldIndex = 0;
    type = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalVidClassificationSet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    GT_U32       vidIndex;
    GT_U16       vidArr[3] = {_2K/*ok*/,_4K-1/*ok*/,_4K/*out of range*/};
    GT_U16       vidGet;


    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            for(vidIndex = 0 ; vidIndex < 3; vidIndex++)
            {
                st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    if(vidIndex != 2)
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index,vidArr[vidIndex]);
                    }
                    else  /*vidIndex == 2*/
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index,vidArr[vidIndex]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index,vidArr[vidIndex]);
                }

                st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    if(vidIndex != 2)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                        UTF_VERIFY_EQUAL2_PARAM_MAC(vidArr[vidIndex], vidGet, dev, index);
                    }
                    else  /*vidIndex == 2*/
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                        UTF_VERIFY_EQUAL2_PARAM_MAC(vidArr[vidIndex - 1], vidGet, dev, index);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
                }
            }
        }

        vidIndex = 0;

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index,vidArr[vidIndex]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index,vidArr[vidIndex]);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    vidIndex = 0;
    index = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index,vidArr[vidIndex]);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index,vidArr[vidIndex]);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalVidClassificationGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    GT_U16       vidGet;


    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }

            /* check NULL pointer*/
            st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }
        }

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalMacDaClassificationEntrySet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entry;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entryGet;
    GT_U32      byteIndex;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        entry.macAddrValue.arEther[0] = 0x11;
        entry.macAddrValue.arEther[1] = 0x22;
        entry.macAddrValue.arEther[2] = 0x33;
        entry.macAddrValue.arEther[3] = 0x44;
        entry.macAddrValue.arEther[4] = 0x55;
        entry.macAddrValue.arEther[5] = 0x66;

        for(index = 0 ; index < 4 ; index ++)
        {
            for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
            {
                /* calc byte mask */
                entry.macAddrMask.arEther[byteIndex] = (GT_U8)
                    (0xff - entry.macAddrValue.arEther[byteIndex]);

                /* update byte value */
                entry.macAddrValue.arEther[byteIndex] ++;
            }

            st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }

            /* check NULL pointer*/
            st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }
        }

        /* check the save values */
        {
            entry.macAddrValue.arEther[0] = 0x11;
            entry.macAddrValue.arEther[1] = 0x22;
            entry.macAddrValue.arEther[2] = 0x33;
            entry.macAddrValue.arEther[3] = 0x44;
            entry.macAddrValue.arEther[4] = 0x55;
            entry.macAddrValue.arEther[5] = 0x66;

            for(index = 0 ; index < 4 ; index ++)
            {
                for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
                {
                    /* calc byte mask */
                    entry.macAddrMask.arEther[byteIndex] = (GT_U8)
                        (0xff - entry.macAddrValue.arEther[byteIndex]);

                    /* update byte value */
                    entry.macAddrValue.arEther[byteIndex] ++;
                }

                st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
                    /* compare values */
                    for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(entry.macAddrValue.arEther[byteIndex], entryGet.macAddrValue.arEther[byteIndex], dev, index);
                        UTF_VERIFY_EQUAL2_PARAM_MAC(entry.macAddrMask.arEther[byteIndex] , entryGet.macAddrMask.arEther[byteIndex] , dev, index);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
                }
            }
        }

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    entry.macAddrValue.arEther[0] = 0x11;
    entry.macAddrValue.arEther[1] = 0x22;
    entry.macAddrValue.arEther[2] = 0x33;
    entry.macAddrValue.arEther[3] = 0x44;
    entry.macAddrValue.arEther[4] = 0x55;
    entry.macAddrValue.arEther[5] = 0x66;
    for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
    {
        /* calc byte mask */
        entry.macAddrMask.arEther[byteIndex] = (GT_U8)
            (0xff - entry.macAddrValue.arEther[byteIndex]);

        /* update byte value */
        entry.macAddrValue.arEther[byteIndex] ++;
    }
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalMacDaClassificationEntryGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entryGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }

            /* check NULL pointer*/
            st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }
        }

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8        devNum,
    IN GT_U32       priority,
    IN GT_U32       threshold
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsSet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       priority;
    GT_U32       thresholdArr[3] = {512/*ok*/,_1K-1/*ok*/,_1K/*out of range*/};
    GT_U32       thresholdGet;
    GT_U32       thresholdIndex;
    GT_U32       thresholdSet;
    GT_U32       dpSelector[MAX_DP_CNS] = {0,};
    GT_U32       thresholdBack;
    GT_U32       maxDp;
    GT_U32       dpIter;
    GT_U32       maxPriority;

    /* there is no RXDMA/IA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
            thresholdArr[2] = _64K;
        }
        else
        {
            maxPriority = 3;
            thresholdArr[2] = _1K;
        }

        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.supportMultiDataPath)
        {
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                maxDp = PRV_CPSS_PP_MAC(dev)->multiPipe.numOfPipes;
                for(dpIter = 0; dpIter < maxDp; dpIter++)
                {
                    dpSelector[dpIter] = (1 << dpIter);
                }
            }
            else
            {
                maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
                for(dpIter = 0; dpIter < maxDp; dpIter++)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.
                        info[dpIter].dataPathNumOfPorts)
                    {
                        dpSelector[dpIter] = (1 << dpIter);
                    }
                }
            }
        }
        for(priority = 0; priority < maxPriority; priority ++)
        {
            for(thresholdIndex = 0 ; thresholdIndex < 3 ; thresholdIndex ++)
            {
                st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
                    CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
                    thresholdArr[thresholdIndex]);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    if(thresholdIndex == 2)
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, priority,thresholdArr[thresholdIndex]);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, priority,thresholdArr[thresholdIndex]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority,thresholdArr[thresholdIndex]);
                }

                st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                    CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, &thresholdGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, priority , thresholdArr[thresholdIndex]);
                    if(thresholdIndex != 2)
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(thresholdArr[thresholdIndex], thresholdGet, dev, priority , thresholdArr[thresholdIndex]);
                    }
                    else /* thresholdIndex == 2 */
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(thresholdArr[thresholdIndex - 1], thresholdGet, dev, priority , thresholdArr[thresholdIndex - 1]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority , thresholdArr[thresholdIndex]);
                }

                /* Set two different thresholds and read them then compare */
                if(dpSelector[0])
                {

                    thresholdSet = 200;

                    /* Backup value */
                    st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
                        &thresholdBack);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                    /* Set tresholds */
                    for(dpIter = 0; dpIter < MAX_DP_CNS; dpIter++)
                    {
                        if(dpSelector[dpIter] == 0)
                        {
                            break;
                        }
                        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
                            dpSelector[dpIter], priority, thresholdSet++);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                    }

                    /* Read back theresholds */
                    for(; dpIter; dpIter--)
                    {
                        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                            dpSelector[dpIter - 1], priority, &thresholdGet);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(--thresholdSet,
                            thresholdGet);
                    }

                    /* Set back value */
                    st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
                        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
                        thresholdBack);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                }
            }
        }

        /* check NOT valid dataPathBmp */
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.
                    supportMultiDataPath)
        {
            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                0xFFFFFFFE, priority, &thresholdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, 0xFFFFFFFE);
        }

        thresholdIndex = 0;

        /* check NOT valid priority */
        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++,
            thresholdArr[thresholdIndex]);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, priority,thresholdArr[thresholdIndex]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority,thresholdArr[thresholdIndex]);
        }
        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++,
            thresholdArr[thresholdIndex]);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, priority,thresholdArr[thresholdIndex]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority,thresholdArr[thresholdIndex]);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    priority = 0;
    thresholdIndex = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev, priority,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, thresholdArr[thresholdIndex]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
        thresholdArr[thresholdIndex]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);

}

/*
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8        devNum,
    IN GT_U32       priority,
    OUT GT_U32       *thresholdPtr
);
*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      priority;
    GT_U32      thresholdGet;
    GT_U32      maxPriority;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
        }
        else
        {
            maxPriority = 3;
        }

        for(priority = 0; priority < maxPriority; priority ++)
        {
            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, &thresholdGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, priority);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority );
            }

            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, priority);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority );
            }

        }

        /* check NOT valid priority */
        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++, &thresholdGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
        }
        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++, &thresholdGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
        }

        /* check NOT valid dataPathBmp */
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.
                    supportMultiDataPath)
        {
            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev, 0xFFFFFFFE,
                priority, &thresholdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, 0xFFFFFFFE);
        }
    }

    /* 3. For not active devices check that function returns non GT_OK.*/
    priority = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,&thresholdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
    }

    /* 4.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, &thresholdGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalDropCounterGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U64      counterArr[4];

    /* there is no RXDMA/IA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortPipGlobalDropCounterGet(dev,counterArr);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev );
        }

        st = cpssDxChPortPipGlobalDropCounterGet(dev,NULL);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev );
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalDropCounterGet(dev,counterArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalDropCounterGet(dev,counterArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipDropCounterGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       port;
    GT_U64       counter;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL2_PARAM_MAC(0 , counter.l[1], dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port );
            }

            st = cpssDxChPortPipDropCounterGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port );
            }
        }

        /* check NOT valid port */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet
(
    IN GT_U8                devNum,
    IN GT_U32               numLsb
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      numLsb,numLsbGet;
    GT_U32      maxValue = 15;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (numLsb = 0 ; numLsb <= maxValue; numLsb++)
        {
            st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(dev,numLsb);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, numLsb);

            st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,&numLsbGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* check that 'get' value is the same as the 'set' value */
            UTF_VERIFY_EQUAL1_PARAM_MAC(numLsb, numLsbGet, dev);
        }

        /* out of range */
        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(dev,numLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, numLsb);

        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,&numLsbGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* check that 'get' value not changed */
        UTF_VERIFY_EQUAL1_PARAM_MAC(maxValue, numLsbGet, dev);

        numLsb = 0xFFFFFFFF;/* -1 */
        /* out of range */
        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(dev,numLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, numLsb);

        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,&numLsbGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* check that 'get' value not changed */
        UTF_VERIFY_EQUAL1_PARAM_MAC(maxValue, numLsbGet, dev);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    numLsb = 1;

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(dev,numLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, numLsb);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(dev,numLsb);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, numLsb);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet
(
    IN GT_U8                devNum,
    OUT GT_U32               *numLsbPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      numLsb;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,&numLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* NULL pointer */
        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, numLsb);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,&numLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(dev,&numLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*

GT_STATUS cpssDxChPortPipGlobalThresholdSet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType,
    IN GT_U32           threshold
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalThresholdSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP   portGroupsBmp;
    CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType;
    GT_U32           threshold;
    GT_PORT_GROUPS_BMP   portGroupsBmpArr[CPSS_MAX_PORT_GROUPS_CNS+1] = {0};
    GT_U32          ii;
    GT_U32          thresholdArr[]       = {0      ,  _32K , _64K-1 ,              _64K , GT_NA};
    GT_STATUS       thresholdArrResult[] = {GT_OK  , GT_OK ,  GT_OK ,   GT_OUT_OF_RANGE , GT_NA};
    GT_U32          thresholdId;
    GT_U32          thresholdArr_perType[CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E+1] =
                    {100,200,300,400};
    GT_U32          expected_threshold;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(portGroupsBmpArr,0,sizeof(portGroupsBmpArr));
        /* first index hold 'unaware mode' */
        portGroupsBmpArr[0] = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        /* next indexes hold 'specific port group' */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,ii)
        {
            portGroupsBmpArr[ii+1] = 1 << ii;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,ii)

        for(ii = 0; ii < (CPSS_MAX_PORT_GROUPS_CNS + 1); ii++)
        {
            if(portGroupsBmpArr[ii] == 0)
            {
                continue;
            }

            portGroupsBmp = portGroupsBmpArr[ii];

            for(thresholdType = 0 ;
                thresholdType <=  CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E ;
                thresholdType ++)
            {
                for(thresholdId = 0 ; thresholdArr[thresholdId] != GT_NA ; thresholdId++)
                {
                    threshold = thresholdArr[thresholdId];

                    st = cpssDxChPortPipGlobalThresholdSet(dev,portGroupsBmp,thresholdType,threshold);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(thresholdArrResult[thresholdId], st, dev, portGroupsBmp,thresholdType,threshold);
                }

                /* put value that we can compare per thresholdType and port group */
                threshold = thresholdArr_perType[thresholdType];
                if(portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
                {
                    threshold += 1000 * ii;/* ii is 1..max */
                }

                st = cpssDxChPortPipGlobalThresholdSet(dev,portGroupsBmp,thresholdType,threshold);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portGroupsBmp,thresholdType,threshold);

            }
        }

        /* compare 'get' values with the expected values */
        for(ii = 0; ii < (CPSS_MAX_PORT_GROUPS_CNS + 1); ii++)
        {
            if(portGroupsBmpArr[ii] == 0)
            {
                continue;
            }

            portGroupsBmp = portGroupsBmpArr[ii];

            for(thresholdType = 0 ;
                thresholdType <=  CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPE_PFC_COUNTER_TRAFFIC_E ;
                thresholdType ++)
            {
                expected_threshold = thresholdArr_perType[thresholdType];
                if(portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
                {
                    expected_threshold += 1000 * ii;/* ii is 1..max */
                }
                else
                {
                    /* the 'unaware' value is expected to be overwritten by 'first port group' */
                    expected_threshold += 1000;
                }

                st = cpssDxChPortPipGlobalThresholdGet(dev,portGroupsBmp,thresholdType,&threshold);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp,thresholdType);

                /* compare value with expected value */
                UTF_VERIFY_EQUAL3_PARAM_MAC(expected_threshold, threshold, dev, portGroupsBmp,thresholdType);

            }
        }

        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        thresholdType = 0;
        threshold = 1;

        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalThresholdSet
                            (dev,portGroupsBmp,thresholdType,threshold),
                            thresholdType);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    thresholdType = 0;
    threshold = 1;

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalThresholdSet(dev,portGroupsBmp,thresholdType,threshold);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portGroupsBmp,thresholdType,threshold);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalThresholdSet(dev,portGroupsBmp,thresholdType,threshold);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp,thresholdType,threshold);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalThresholdGet
(
    IN GT_U8            devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType,
    OUT GT_U32           *thresholdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalThresholdGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP   portGroupsBmp;
    CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType;
    GT_U32           threshold;

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    thresholdType = 0;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortPipGlobalThresholdGet(dev,portGroupsBmp,thresholdType,&threshold);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp,thresholdType);

        /* NULL pointer */
        st = cpssDxChPortPipGlobalThresholdGet(dev,portGroupsBmp,thresholdType,NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, portGroupsBmp,thresholdType);

        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalThresholdGet
                            (dev,portGroupsBmp,thresholdType,&threshold),
                            thresholdType);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalThresholdGet(dev,portGroupsBmp,thresholdType,&threshold);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portGroupsBmp,thresholdType);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalThresholdGet(dev,portGroupsBmp,thresholdType,&threshold);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp,thresholdType);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalPfcTcVectorSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType,
    IN GT_U32               tcVector
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalPfcTcVectorSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType;
    GT_U32 tcVector;
    GT_U32 expected_tcVector;
    GT_U32 tcVectorArr[] =        {  100 ,  200 ,  255 ,            256 , GT_NA};
    GT_U32 tcVectorArr_status[] = { GT_OK, GT_OK, GT_OK,GT_OUT_OF_RANGE , GT_NA};
    GT_U32 ii;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(vectorType = 0 ;
            vectorType <= CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E ;
            vectorType++)
        {

            for(ii = 0 ; tcVectorArr[ii] != GT_NA ; ii++)
            {
                tcVector = tcVectorArr[ii];
                st = cpssDxChPortPipGlobalPfcTcVectorSet(dev,vectorType,tcVector);
                UTF_VERIFY_EQUAL3_PARAM_MAC(tcVectorArr_status[ii], st, dev, vectorType,tcVector);
            }

            /* set unique value per type ... to allow compare with 'get' values */
            tcVector = vectorType + 1;

            st = cpssDxChPortPipGlobalPfcTcVectorSet(dev,vectorType,tcVector);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vectorType,tcVector);
        }

        for(vectorType = 0 ;
            vectorType <= CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPE_HEAVY_CONGESTION_E ;
            vectorType++)
        {
            expected_tcVector = vectorType + 1;

            st = cpssDxChPortPipGlobalPfcTcVectorGet(dev,vectorType,&tcVector);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vectorType);

            /* compare 'expected' with 'get' value */
            UTF_VERIFY_EQUAL2_PARAM_MAC(expected_tcVector, tcVector, dev, vectorType);
        }

        vectorType = 0;
        tcVector = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalPfcTcVectorSet
                            (dev,vectorType,tcVector),
                            vectorType);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    vectorType = 0;
    tcVector = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalPfcTcVectorSet(dev,vectorType,tcVector);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, vectorType,tcVector);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalPfcTcVectorSet(dev,vectorType,tcVector);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vectorType,tcVector);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalPfcTcVectorGet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType,
    OUT GT_U32               *tcVectorPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalPfcTcVectorGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType;
    GT_U32               tcVector;

    vectorType = 0;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortPipGlobalPfcTcVectorGet(dev,vectorType,&tcVector);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vectorType);

        /* NULL pointer */
        st = cpssDxChPortPipGlobalPfcTcVectorGet(dev,vectorType,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, vectorType);

        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalPfcTcVectorGet
                            (dev,vectorType,&tcVector),
                            vectorType);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);
    PRV_TGF_NON_PIP_IA_APPLIC_DEV_RESET_MAC;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalPfcTcVectorGet(dev,vectorType,&tcVector);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, vectorType);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalPfcTcVectorGet(dev,vectorType,&tcVector);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vectorType);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxDmaGlobalDropCounterModeSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  *counterModeInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxDmaGlobalDropCounterModeSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC counterModeInfo;
    CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC counterModeInfoGet;
    GT_U32  ii,jj;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        cpssOsMemSet(&counterModeInfo,0,sizeof(counterModeInfo));
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        counterModeInfo.modesBmp = CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PIP_E;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        counterModeInfo.modesBmp = CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PACKET_BUFFER_FILL_LEVEL_E;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        counterModeInfo.modesBmp = BIT_2;/* not defined as value value */
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        counterModeInfo.modesBmp = CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PIP_E |
                                   CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_PACKET_BUFFER_FILL_LEVEL_E;
        counterModeInfo.portNumPattern = 0;
        counterModeInfo.portNumMask    = 0xFFFFFFFF;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for(ii = 0 ; ii < 4 ;ii++)
        {
            for(jj = 0 ; jj < 4 ;jj++)
            {
                counterModeInfo.priorityPattern = ii;
                counterModeInfo.priorityMask    = jj;

                st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            counterModeInfo.priorityPattern = ii;
            counterModeInfo.priorityMask    = jj;
            st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* pattern ignored with mask is 0 */
        counterModeInfo.priorityPattern = 0xaabbccdd;
        counterModeInfo.priorityMask    =  0;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        counterModeInfo.priorityPattern = 0xaabbccdd;
        counterModeInfo.priorityMask    = 0x1;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        counterModeInfo.priorityPattern = 2;
        counterModeInfo.priorityMask    = 3;

        counterModeInfo.portNumMask    = 0x0000000F;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        counterModeInfo.portNumMask    = 0x00000000;
        counterModeInfo.portNumPattern = 0xFFFFFFFF;
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortRxDmaGlobalDropCounterModeGet(dev,&counterModeInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(counterModeInfoGet.modesBmp       , counterModeInfo.modesBmp        , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(counterModeInfoGet.portNumPattern , counterModeInfo.portNumPattern  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(counterModeInfoGet.portNumMask    , counterModeInfo.portNumMask     , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(counterModeInfoGet.priorityPattern, counterModeInfo.priorityPattern , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(counterModeInfoGet.priorityMask   , counterModeInfo.priorityMask    , dev);
    }

    cpssOsMemSet(&counterModeInfo,0,sizeof(counterModeInfo));
    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortRxDmaGlobalDropCounterModeSet(dev,&counterModeInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxDmaGlobalDropCounterModeGet
(
    IN GT_U8            devNum,
    OUT CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  *counterModeInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxDmaGlobalDropCounterModeGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC counterModeInfo;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortRxDmaGlobalDropCounterModeGet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* NULL pointer */
        st = cpssDxChPortRxDmaGlobalDropCounterModeGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxDmaGlobalDropCounterModeGet(dev,&counterModeInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortRxDmaGlobalDropCounterModeGet(dev,&counterModeInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortRxDmaGlobalDropCounterGet
(
    IN  GT_U8            devNum,
    IN  GT_DATA_PATH_BMP dataPathBmp,
    OUT GT_U64           *counterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortRxDmaGlobalDropCounterGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8             dev;
    GT_U64            counter;
    GT_DATA_PATH_BMP  dataPathBmp;
    GT_STATUS         rc = GT_OK;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        dataPathBmp = 0x3;
        st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, dataPathBmp, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* NULL pointer */
        st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 0x0 bitmap */
        st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, 0x0, &counter);
        rc = (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp == 1) ? GT_OK : GT_BAD_PARAM;
        UTF_VERIFY_EQUAL1_PARAM_MAC(rc, st, dev);

        /* Out of ranges bitmap */
        /* Falcon has 8 DPs per tile */
        if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E))
        {
            dataPathBmp = (PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles == 1) ? 0x1FF/* 1 tile */ : 0x1FFFF /* 2 tile */;
            if(PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles < 4)
            {
                st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, dataPathBmp, &counter);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
        }/* AC5P has 4 DPs */
        else if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E))
        {
            st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, 0x1F, &counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }/* Harrier has 3 DPs */
        else if((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, 0xF, &counter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortRxDmaGlobalDropCounterGet(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortParserGlobalTpidSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_U16       etherType,
    IN CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortParserGlobalTpidSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    GT_U16       etherType,etherTypeGet,extected_etherType;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize,tpidSizeGet,extected_tpidSize;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        etherType = 0x6666;
        for (index = 0; index  < 4 ; index ++)
        {
            for(tpidSize = 0 ;
                tpidSize <= CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E ;
                tpidSize ++ , etherType++)
            {
                st = cpssDxChPortParserGlobalTpidSet(dev,index,etherType,tpidSize);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChPortParserGlobalTpidGet(dev,index,&etherTypeGet,&tpidSizeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL1_PARAM_MAC(etherType, etherTypeGet, dev);
                UTF_VERIFY_EQUAL1_PARAM_MAC(tpidSize , tpidSizeGet , dev);
            }

            /* tpidSize - out of range */
            st = cpssDxChPortParserGlobalTpidSet(dev,index,etherType,tpidSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            tpidSize  = index % (CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E + 1);
            etherType = (GT_U16)(index + 0x8888);
            st = cpssDxChPortParserGlobalTpidSet(dev,index,etherType,tpidSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* check the 'get' values*/
        for (index = 0; index  < 4 ; index ++)
        {
            st = cpssDxChPortParserGlobalTpidGet(dev,index,&etherTypeGet,&tpidSizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            extected_tpidSize  = index % (CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E + 1);
            extected_etherType = (GT_U16)(index + 0x8888);

            UTF_VERIFY_EQUAL1_PARAM_MAC(extected_etherType, etherTypeGet, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(extected_tpidSize , tpidSizeGet , dev);
        }

        index = 0;
        tpidSize = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortParserGlobalTpidSet
                            (dev,index,etherType,tpidSize),
                            tpidSize);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    etherType = 0x6666;
    tpidSize  = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortParserGlobalTpidSet(dev,index,etherType,tpidSize);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,etherType,tpidSize);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortParserGlobalTpidSet(dev,index,etherType,tpidSize);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,etherType,tpidSize);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortParserGlobalTpidGet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr,
    OUT CPSS_BRG_TPID_SIZE_TYPE_ENT  *tpidSizePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortParserGlobalTpidGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      index = 0;
    GT_U16      etherType;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize;

    /* prepare device iterator */
    UT_PIP_SIP6_RESET_DEV(&dev);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortParserGlobalTpidGet(dev,index,&etherType,&tpidSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* NULL pointer */
        st = cpssDxChPortParserGlobalTpidGet(dev,index,NULL,&tpidSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPortParserGlobalTpidGet(dev,index,&etherType,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_SIP6_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortParserGlobalTpidGet(dev,index,&etherType,&tpidSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortParserGlobalTpidGet(dev,index,&etherType,&tpidSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortPip suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortPip)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEtherTypeProtocolSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEtherTypeProtocolGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipTrustEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipTrustEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipVidClassificationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipVidClassificationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipPrioritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipPriorityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalProfilePrioritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalProfilePriorityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalVidClassificationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalVidClassificationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalMacDaClassificationEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalMacDaClassificationEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalDropCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipDropCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalPfcTcVectorSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalPfcTcVectorGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxDmaGlobalDropCounterModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxDmaGlobalDropCounterModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRxDmaGlobalDropCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortParserGlobalTpidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortParserGlobalTpidGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortPip)


