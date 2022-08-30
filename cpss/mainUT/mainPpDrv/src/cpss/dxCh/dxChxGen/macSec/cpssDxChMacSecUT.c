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
* @file cpssDxChMacSecUT.c
*
* @brief Unit tests for cpssDxChMacSec, that provides
* Function implementation for MAC Sec configuration.
*
* @version   44
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSec.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/*************************** Data definitions for the tests ***************************************/

/* EIP-163 devices IDs */
#define UTF_CPSS_CFYE_EGRESS_DP0_DEVICE_ID_CNS   0
#define UTF_CPSS_CFYE_INGRESS_DP0_DEVICE_ID_CNS  1
#define UTF_CPSS_CFYE_EGRESS_DP1_DEVICE_ID_CNS   2
#define UTF_CPSS_CFYE_INGRESS_DP1_DEVICE_ID_CNS  3

/* EIP-164 devices IDs */
#define UTF_CPSS_SECY_EGRESS_DP0_DEVICE_ID_CNS   0
#define UTF_CPSS_SECY_INGRESS_DP0_DEVICE_ID_CNS  1
#define UTF_CPSS_SECY_EGRESS_DP1_DEVICE_ID_CNS   2
#define UTF_CPSS_SECY_INGRESS_DP1_DEVICE_ID_CNS  3


/* MACsec key */
static GT_U8 K1[] = {
    0xad, 0x7a, 0x2b, 0xd0, 0x3e, 0xac, 0x83, 0x5a,
    0x6f, 0x62, 0x0f, 0xdc, 0xb5, 0x06, 0xb3, 0x45,
};

/* MACsec Hash key */
static GT_U8 hashKey[] = {
    0x45, 0x77, 0x24, 0x01, 0x84, 0x51, 0x32, 0x88,
    0x46, 0x78, 0x25, 0x02, 0x85, 0x52, 0x33, 0x89,
};



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecInit
(
    IN  GT_U8               devNum,
    IN  GT_MACSEC_UNIT_BMP  unitBmp
);
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecInit)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChMacSecInit with valid values.
    Expected: GT_OK.

    1.2. Call cpssDxChMacSecInit with valid values but after it was already called and initialized and therefore should fail
    Expected: GT_ALREADY_EXIST.

    1.3 Call cpssDxChMacSecInit with invalid unitBmp.
    unitBmp [6]                                                .
    Expected: GT_BAD_PARAM.
    NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .

    2.1. Call cpssDxChMacSecInit for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecInit with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS           st = GT_OK;
    GT_MACSEC_UNIT_BMP  unitBmp;
    GT_U8               dev;
    GT_U32              macSecDpId;
    GT_U32              macSecMaxNumDp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChMacSecInit with valid parameters.
            Expected: GT_OK.
        */

        /* Get maximum number of Data path units connected to MACSec */
        macSecMaxNumDp = ((PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.macSecInfo.macSecDpBmp) >> 1) + 1;

        /* Loop over the maximum number of Data paths. For Hawk:2 for Phoenix:1  */
        for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
        {
            unitBmp = BIT_0 << macSecDpId;
            st = cpssDxChMacSecInit(dev, unitBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call cpssDxChMacSecInit with valid parameters but after it was already called and initialized and therefore should fail
            Expected: GT_ALREADY_EXIST.
        */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_ALREADY_EXIST, st, dev);

        /*
           1.3 Call cpssDxChMacSecInit with invalid unitBmp.
           unitBmp [6]                                                .
           Expected: GT_BAD_PARAM                                     .
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecInit(dev, 6);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /*
        2.1. Call cpssDxChMacSecInit for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    unitBmp = 1;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecInit(dev, unitBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecInit with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecInit(dev, unitBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortClassifyConfigGet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecPortClassifyConfigSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecPortClassifyConfigSet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecPortClassifyConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_PHYSICAL_PORT_NUM                    port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  classifyPortCfg;
    CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  classifyPortCfgGet;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&classifyPortCfg, 0, sizeof(classifyPortCfg));
                st = cpssDxChMacSecPortClassifyConfigSet(dev,
                                                         port,
                                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                         &classifyPortCfg);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                 initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyConfigSet with valid values.
                    Expected: GT_OK.
            */

            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            cpssOsMemSet(&classifyPortCfg, 0, sizeof(classifyPortCfg));
            classifyPortCfg.bypassMacsecDevice = GT_TRUE;
            classifyPortCfg.exceptionCfgEnable = GT_TRUE;
            classifyPortCfg.forceDrop          = GT_TRUE;
            classifyPortCfg.defaultVPortValid  = GT_TRUE;
            st = cpssDxChMacSecPortClassifyConfigSet(dev,
                                                     port,
                                                     direction,
                                                     &classifyPortCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyConfigGet with the same parameters.
               Expected: GT_OK and the same values that were set.
            */
            st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                                     port,
                                                     direction,
                                                     &classifyPortCfgGet);
            /* Verify set and get parameters are the same */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(classifyPortCfg.bypassMacsecDevice , classifyPortCfgGet.bypassMacsecDevice , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(classifyPortCfg.forceDrop          , classifyPortCfgGet.forceDrop          , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(classifyPortCfg.defaultVPortValid  , classifyPortCfgGet.defaultVPortValid  , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(classifyPortCfg.defaultVPort       , classifyPortCfgGet.defaultVPort       , dev);

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyConfigSet(dev, port, direction, &classifyPortCfg), direction);

            /*
               1.1.5 Call api with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortClassifyConfigSet(dev,
                                                     port,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyConfigSet(dev,
                                                     port,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     &classifyPortCfg);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyConfigSet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 &classifyPortCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyConfigSet(dev,
                                             port,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             &classifyPortCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyConfigGet
(
    IN   GT_U8                                   devNum,
    IN   GT_PHYSICAL_PORT_NUM                    portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyConfigGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.1 Call cpssDxChMacSecPortClassifyConfigGet with valid values.
          Expected: GT_OK.

    1.1.2 Call cpssDxChMacSecPortClassifyConfigGet with portCfgPtr [NULL]
          Expected: GT_BAD_PARAM

    1.1.3 Call cpssDxChMacSecPortClassifyConfigGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecPortClassifyConfigGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyConfigGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_PHYSICAL_PORT_NUM                    port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  portCfgGet;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&portCfgGet, 0, sizeof(portCfgGet));
                st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                                         port,
                                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                         &portCfgGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyConfigGet with valid values.
                     Expected: GT_OK.
            */
            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E ;
            cpssOsMemSet(&portCfgGet, 0, sizeof(portCfgGet));
            st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                                     port,
                                                     direction,
                                                     &portCfgGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyConfigGet with portCfgPtr [NULL]
                     Expected: GT_BAD_PTR
            */
            st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                                     port,
                                                     direction,
                                                     NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyConfigGet(dev, port, direction, &portCfgGet), direction);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                                     port,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     &portCfgGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /*
        2.1. Call cpssDxChMacSecPortClassifyConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 &portCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyConfigGet(dev,
                                             port,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             &portCfgGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportAdd)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportAdd before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportAdd with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyVportAdd and try to add more vPorts than allowed                                                                        .
           Expected: GT_FAIL.

    1.1.4 Call cpssDxChMacSecClassifyVportAdd with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyVportAdd with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyVportAdd with vPortParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call cpssDxChMacSecClassifyVportAdd with vPortHandlePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.8 Call cpssDxChMacSecClassifyVportAdd with invalid packet expansion in ingress direction
          vPortParams.pktExpansion != 0                                                              .
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyVportAdd for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportAdd with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandlesTable[257];
    GT_U32                                  i;
    GT_U32                                  maxClassifyVports;
    GT_MACSEC_UNIT_BMP                      unitBmp;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyVportAdd before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportAdd with valid values.
                 Expected: GT_OK.
        */

        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             NULL,
                                             &maxClassifyVports,
                                             NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortHandle = 0;
        unitBmp = BIT_0;

        /* Add the exact maximum allowed number of vPorts */
        for (i=0;i<maxClassifyVports;i++)
        {
            st = cpssDxChMacSecClassifyVportAdd(dev,
                                                unitBmp,
                                                direction,
                                                &vPortParams,
                                                &vPortHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Save vPort handle */
            vPortHandlesTable[i] = vPortHandle;
        }


        /*
        1.1.3 Call cpssDxChMacSecClassifyVportAdd to try and add more vPorts than allowed                                                                        .
              Expected: GT_FAIL.
        */

        /* Try to add one more vPort than allowed  (maxClassifyVports) */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);


        /* Now remove all vPorts for proper exit */
        for (i=0;i<maxClassifyVports;i++)
        {
            st = cpssDxChMacSecClassifyVportRemove(dev,
                                                   unitBmp,
                                                   direction,
                                                   vPortHandlesTable[i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyVportAdd with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyVportAdd(dev,
                                                6,
                                                direction,
                                                &vPortParams,
                                                &vPortHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyVportAdd(dev, unitBmp, direction, &vPortParams, &vPortHandle), direction);

        /*
           1.1.6 Call api with vPortParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            NULL,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with vPortHandlePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.8 Call cpssDxChMacSecClassifyVportAdd with invalid packet expansion in ingress direction
                 vPortParams.pktExpansion != 0                                                              .
           Expected: GT_BAD_PARAM.
        */
        /* Set it first with packet expansion of 24 bytes */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Now set it with packet expansion of 32 bytes */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportAdd for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportAdd with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyVportAdd(dev,
                                        BIT_0,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        &vPortParams,
                                        &vPortHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportRemove
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportRemove)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportRemove before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle.
          Call cpssDxChMacSecClassifyVportRemove with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyVportRemove with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyVportRemove with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecClassifyVportRemove with invalid vPortHandle.
          vPortHandle [0]
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyVportRemove for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportRemove with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyVportRemove before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        vPortHandle = 1;
        /* Make sure it is not zero */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportRemove with valid values.
                 Expected: GT_OK.
        */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now remove this vPort */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyVportRemove with invalid unitBmp.
                 unitBmp [6]                                                                 .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyVportRemove(dev,
                                                   6,
                                                   direction,
                                                   vPortHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyVportRemove(dev, unitBmp, direction, vPortHandle), direction);

        /*
           1.1.5 Call api with invalid vPortHandle.
                 vPortHandle [0]                                       .
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportRemove for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    vPortHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportRemove with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyVportRemove(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportIndexGet
(
    IN   GT_U32                                  devNum,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  GT_U32                                  *vPortIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportIndexGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle.
          Call cpssDxChMacSecClassifyVportIndexGet with valid values.
          Expected: GT_OK.

    1.1.2 Call cpssDxChMacSecClassifyVportIndexGet with invalid vPortHandle.
          vPortHandle [0]
          Expected: GT_BAD_PARAM.

    1.1.3 Call cpssDxChMacSecClassifyVportIndexGet with vPortIndexPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyVportIndexGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportIndexGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.1 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle.
                 Call cpssDxChMacSecClassifyVportIndexGet with valid values.
                 Expected: GT_OK.
        */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportIndexGet with invalid vPortHandle.
                 vPortHandle [0]                                                                       .
           Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 0,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyVportIndexGet with vPortIndexPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportIndexGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    vPortHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportIndexGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                             vPortHandle,
                                             &vPortIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleAdd)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleAdd before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyRuleAdd with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleAdd and try to add more rules than allowed                                                                        .
          Expected: GT_FAIL.

    1.1.4 Call cpssDxChMacSecClassifyRuleAdd with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyRuleAdd with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyRuleAdd with invalid vPortHandle.
          vPortHandle [0]
          Expected: GT_BAD_PARAM.

    1.1.7 Call cpssDxChMacSecClassifyRuleAdd with ruleParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.8 Call cpssDxChMacSecClassifyRuleAdd with ruleHandlePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyRuleAdd for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleAdd with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams, ruleParamsGet;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle, vPortHandleGet;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandlesTable[513];
    GT_U32                                  i;
    GT_U32                                  maxClassifyRules;
    GT_BOOL                                 ruleEnableGet;
    GT_MACSEC_UNIT_BMP                      unitBmp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleAdd before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Make sure it is not zero */
        vPortHandle = 1;
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
                 Call cpssDxChMacSecClassifyRuleAdd with valid values.
                 Expected: GT_OK.
        */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Get maximum number of rules allowed in the device */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             NULL,
                                             NULL,
                                             &maxClassifyRules);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ruleParams.key.packetType  = 0x3;
        ruleParams.mask.packetType = 0x3;
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                unitBmp,
                direction,
                vPortHandle,
                &ruleParams,
                &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        ruleHandlesTable[0] = ruleHandle;

        st = cpssDxChMacSecClassifyRuleGet(dev,
                unitBmp,
                direction,
                ruleHandle,
                &vPortHandleGet,
                &ruleParamsGet,
                &ruleEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssOsMemCmp(&ruleParams, &ruleParamsGet, sizeof(ruleParamsGet))?GT_OK : GT_FALSE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add the exact maximum allowed number of rules */
        for (i=1;i<maxClassifyRules;i++)
        {
            st = cpssDxChMacSecClassifyRuleAdd(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle,
                                               &ruleParams,
                                               &ruleHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Save rule handle */
            ruleHandlesTable[i] = ruleHandle;
        }


        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleAdd and try to add more rules than allowed                                                                        .
           Expected: GT_FAIL.
        */

        /* Try to add one more rule than allowed  (maxClassifyRules) */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);


        /* Now remove all rules for proper exit */
        for (i=0;i<maxClassifyRules;i++)
        {
            st = cpssDxChMacSecClassifyRuleRemove(dev,
                                                  BIT_0,
                                                  direction,
                                                  ruleHandlesTable[i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }


        /* Remove this vPort for proper exit */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecClassifyRuleAdd with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleAdd(dev,
                                               6,
                                               direction,
                                               vPortHandle,
                                               &ruleParams,
                                               &ruleHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleAdd(dev, unitBmp, direction, vPortHandle, &ruleParams ,&ruleHandle), direction);

        /*
           1.1.6 Call cpssDxChMacSecClassifyRuleAdd with invalid vPortHandle.
                 vPortHandle [0]                                                                 .
           Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           0,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.1.7 Call API with ruleParams as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle,
                                           NULL,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.8 Call API with ruleHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle,
                                           &ruleParams,
                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleAdd for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    vPortHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleAdd with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyRuleAdd(dev,
                                       BIT_0,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       vPortHandle,
                                       &ruleParams,
                                       &ruleHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleRemove
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE    ruleHandle
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleRemove)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleRemove before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
          Call cpssDxChMacSecClassifyRuleAdd to get valid rule handle
          Call cpssDxChMacSecClassifyRuleRemove with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleRemove with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyRuleRemove with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecClassifyRuleRemove with invalid ruleHandle.
          ruleHandle [0]
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyRuleRemove for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleRemove with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleRemove before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Set it to not zero */
        ruleHandle = 0x1;
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
                 Call cpssDxChMacSecClassifyRuleAdd to get valid rule handle
                 Call cpssDxChMacSecClassifyRuleRemove with valid values.
                 Expected: GT_OK.
        */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ruleHandle = 0;
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Add rule and get valid rule handle */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove that rule */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleRemove with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleRemove(dev,
                                                  6,
                                                  direction,
                                                  ruleHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyRuleRemove with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleRemove(dev, unitBmp, direction, ruleHandle), direction);

        /*
           1.1.5 Call cpssDxChMacSecClassifyRuleRemove with invalid ruleHandle.
                 ruleHandle [0]                                                                 .
           Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleRemove for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    ruleHandle = 0;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleRemove with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyRuleRemove(dev,
                                          BIT_0,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          ruleHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleEnable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  GT_BOOL                                 enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleEnable)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleEnable before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
          Call cpssDxChMacSecClassifyRuleAdd with valid values
          Call cpssDxChMacSecClassifyRuleEnable with valid values
          enable [GT_TRUE/GT_FALSE]
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleEnable with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyRuleEnable with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecClassifyRuleEnable with invalid ruleHandle.
          ruleHandle [0]
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyRuleEnable for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleEnable with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_BOOL                                 enable;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleEnable before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Set it to not zero */
        ruleHandle = 0x1;
        st = cpssDxChMacSecClassifyRuleEnable(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              ruleHandle,
                                              GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
                 Call cpssDxChMacSecClassifyRuleAdd with valid values.
                 Call cpssDxChMacSecClassifyRuleEnable with valid values.
                 enable [GT_TRUE/GT_FALSE]
                 Expected: GT_OK.
        */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ruleHandle = 0;
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Enable rule */
        enable = GT_TRUE;
        st = cpssDxChMacSecClassifyRuleEnable(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle,
                                              enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Disable rule */
        enable = GT_FALSE;
        st = cpssDxChMacSecClassifyRuleEnable(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle,
                                              enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleEnable with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleEnable(dev,
                                                  6,
                                                  direction,
                                                  ruleHandle,
                                                  GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyRuleEnable with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleEnable(dev, unitBmp, direction, ruleHandle, enable), direction);

        /*
           1.1.5 Call cpssDxChMacSecClassifyRuleEnable with invalid ruleHandle.
                 ruleHandle [0]                                                                 .
           Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMacSecClassifyRuleEnable(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              0,
                                              GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleEnable for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    ruleHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleEnable(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              ruleHandle,
                                              GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleEnable with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyRuleEnable(dev,
                                          BIT_0,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          ruleHandle,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleIndexGet
(
    IN   GT_U32                                 devNum,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE  ruleHandle,
    OUT  GT_U32                                 *ruleIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleIndexGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleIndexGet with valid values.
          Expected: GT_OK.

    1.1.2 Call cpssDxChMacSecClassifyRuleIndexGet with invalid ruleHandle.
          vPortHandle [0]
          Expected: GT_BAD_PARAM.

    1.1.3 Call cpssDxChMacSecClassifyRuleIndexGet with ruleIndexPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyRuleIndexGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleIndexGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    GT_U32                                  ruleIndex;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.1 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
                 Call cpssDxChMacSecClassifyRuleAdd with valid values.
                 Call cpssDxChMacSecClassifyRuleIndexGet with valid values.
                 enable [GT_TRUE/GT_FALSE]
                 Expected: GT_OK.
        */
        direction  = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        ruleHandle = 0;
        unitBmp    = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ruleHandle = 0;
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated rule index */
        st = cpssDxChMacSecClassifyRuleIndexGet(dev,
                                                ruleHandle,
                                                 &ruleIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Remove this rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyRuleIndexGet with invalid ruleHandle.
                 ruleHandle [0]                                                                       .
           Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMacSecClassifyRuleIndexGet(dev,
                                                0,
                                                &ruleIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleIndexGet with ruleIndexPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleIndexGet(dev,
                                                ruleHandle,
                                                NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleIndexGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    ruleHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleIndexGet(dev,
                                                ruleHandle,
                                                &ruleIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleIndexGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyRuleIndexGet(dev,
                                            ruleHandle,
                                            &ruleIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaAdd
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  GT_U32                              vPortId,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC        *saParamsPtr,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *trRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *saHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaAdd)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaAdd before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaAdd with mismatch configuration between SA and vPort.
          Set SA with action type of egress which is opposed to vPort parameter that was previously set to no expansion                                                              .
          Expected: GT_BAD_PARAM                               .

    1.1.3 Call cpssDxChMacSecSecySaAdd with mismatch configuration between SA and vPort.
          Set SA to include sci which is opposed to vPort parameter that was previously set to packet expansion of 24 bytes                                                              .
          Expected: GT_BAD_PARAM

    1.1.4 Call cpssDxChMacSecSecySaAdd with SA valid parameters in regard to vPort no expansion setting
          First set SA action to bypass
          Second set SA action to drop
          Expected: GT_OK in both                               .

    1.1.5 Call cpssDxChMacSecSecySaAdd with valid values
          Expected: GT_OK.

    1.1.6 Call cpssDxChMacSecSecySaAdd and try to add one more SA than allowed                                                                        .
           Expected: GT_FAIL.

    1.1.7 Call cpssDxChMacSecSecySaAdd with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.8 Call cpssDxChMacSecSecySaAdd with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.9 Call cpssDxChMacSecSecySaAdd with saParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.10 Call cpssDxChMacSecSecySaAdd with trRecParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.11 Call cpssDxChMacSecSecySaAdd with saHandlePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.12 Call api with invalid vPortIndex.
           vPortIndex [maximum number of vPorts + 1]                                       .
           Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecySaAdd for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaAdd with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandlesTable[513];
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandleNoExp;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandleNoExp;
    GT_U32                                  vPortIndex;
    GT_U32                                  vPortIndexNoExp;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  i;
    GT_U32                                  maxSecySas;
    GT_U32                                  maxSecyVports;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaAdd before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Set values to zero */
        saHandle = 0;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        vPortIndex = 0;

        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Prepare parameters */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        saHandle  = 0;
        unitBmp   = BIT_0;

        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E;

        /* Add first vPort while set to no expansion */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandleNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandleNoExp,
                                                 &vPortIndexNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Set second vPort this time with expansion of 24 bytes (no SCI) */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set Hash key */
        trRecParams.customHkeyEnable = GT_TRUE;

        for (i=0;i<sizeof(hashKey);i++)
        {
            trRecParams.customHkeyArr[i] = hashKey[i];
        }

        /* Set SA parameters */
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;

        /*
           1.1.2 Call cpssDxChMacSecSecySaAdd with mismatch configuration between SA and vPort.
                 Set SA with action type of egress which is opposed to vPort parameter that was previously set to no expansion                                                              .
                 Expected: GT_BAD_PARAM                               .
        */

        /* Call SA add api with action type of egress which is opposed to vPort parameter that was previously set to no expansion
           therefore expect the api to fail */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndexNoExp,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecySaAdd with mismatch configuration between SA and vPort.
                 Set SA to include sci which is opposed to vPort parameter that was previously set to packet expansion of 24 bytes                                                              .
                 Expected: GT_BAD_PARAM                               .
        */

        /* Set it to include sci in order to cause mismatch compared to vport parameter
           Expect the API to fail */
        saParams.params.egress.includeSci = GT_TRUE;

        /* Call cpssDxChMacSecSecySaAdd with valid values but opposed to vPort setting */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecSecySaAdd with SA valid parameters in regard to vPort no expansion setting
                 First set SA action to bypass
                 Second set SA action to drop
                 Expected: GT_OK in both                               .
        */

        /* Now set it with SA bypass action */
        saParams.params.egress.includeSci = GT_FALSE;
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E;

        /* Call cpssDxChMacSecSecySaAdd with valid values */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandleNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove SA */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandleNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Now set it with SA drop action */
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E;

        /* Call cpssDxChMacSecSecySaAdd with valid values */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandleNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove SA */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandleNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.5 Call cpssDxChMacSecSecySaAdd with valid values and match to vPort settings.
                 Expected: GT_OK.
        */

        /* Now set it to not include sci as it was set in add vport api configurations */
        saParams.params.egress.includeSci = GT_FALSE;

        /* Get maximum number of rules allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         NULL,
                                         &maxSecySas,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add the exact maximum allowed number of SAs */
        for (i=0;i<maxSecySas;i++)
        {
            st = cpssDxChMacSecSecySaAdd(dev,
                                         BIT_0,
                                         direction,
                                         vPortIndex,
                                         &saParams,
                                         &trRecParams,
                                         &saHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Save SA handle */
            saHandlesTable[i] = saHandle;
        }

        /*
           1.1.6 Call cpssDxChMacSecSecySaAdd and try to add one more SA than allowed                                                                        .
           Expected: GT_FAIL.
        */

        /* Try to add one more SA than allowed  (maxClassifySas) */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);


        /* Now remove all SAs for proper exit */
        for (i=0;i<maxSecySas;i++)
        {
            st = cpssDxChMacSecSecySaRemove(dev,
                                            unitBmp,
                                            direction,
                                            saHandlesTable[i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }


        /* Remove these vPorts for proper exit */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandleNoExp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.7 Call cpssDxChMacSecSecySaAdd with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaAdd(dev,
                                         6,
                                         direction,
                                         0,
                                         &saParams,
                                         &trRecParams,
                                         &saHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /*
           1.1.8 Call cpssDxChMacSecSecySaAdd with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaAdd(dev, unitBmp, direction, vPortIndex, &saParams, &trRecParams, &saHandle), direction);

        /*
           1.1.9 Call api with saParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     unitBmp,
                                     direction,
                                     0,
                                     0,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.10 Call api with trRecParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        direction = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        st = cpssDxChMacSecSecySaAdd(dev,
                                     unitBmp,
                                     direction,
                                     0,
                                     &saParams,
                                     0,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.11 Call api with saHandlePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     unitBmp,
                                     direction,
                                     0,
                                     &saParams,
                                     &trRecParams,
                                     NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.12 Call api with invalid vPortIndex.
           vPortIndex [maximum number of vPorts + 1]                                       .
           Expected: GT_BAD_PARAM.
        */
        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         &maxSecyVports,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call API with vPortIndex set to maximum number of vPorts that for index value it is 1 above valid range */
        vPortIndex = maxSecyVports;
        st = cpssDxChMacSecSecySaAdd(dev,
                                     unitBmp,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaAdd for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     0,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaAdd with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecySaAdd(dev,
                                 BIT_0,
                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                 0,
                                 &saParams,
                                 &trRecParams,
                                 &saHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaRemove
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaRemove)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaRemove before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaRemove with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaRemove with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecySaRemove with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecySaRemove with invalid saHandle.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecInit for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecInit with out of bound device id.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  i;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaRemove before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        saHandle = 1;
        unitBmp   = BIT_0;
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));

        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSecySaRemove with valid values
                 Expected: GT_OK.
        */

        /* Clear parameters */
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort with expansion of 24 bytes (no SCI) */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set few of SA parameters */
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;

        /* Set it to not include sci as it was set in add vport api configurations */
        saParams.params.egress.includeSci = GT_FALSE;

        /* Call cpssDxChMacSecSecySaAdd */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Remove the SA */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecySaAdd with invalid unitBmp.
                 unitBmp [6]                                                             .
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaRemove(dev,
                                            6,
                                            direction,
                                            saHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecSecySaRemove with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaRemove(dev, unitBmp, direction, saHandle), direction);

        /*
           1.1.5 Call api with invalid saHandle
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaRemove for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    saHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaRemove(dev,
                                        BIT_0,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaRemove with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecySaRemove(dev,
                                    BIT_0,
                                    CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                    saHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaChainSet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     activeSaHandle,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *newTrRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *newSaHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaChainSet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaChainSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaChainSet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaChainSet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecySaChainSet with newTrRecParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.5 Call cpssDxChMacSecSecySaChainSet with newSaHandlePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSecySaChainSet with invalid activeSaHandle.
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecInit for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecInit with out of bound device id.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     newTrRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         activeSaHandle;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         newSaHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  i;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaChainSet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Set values to zero */
        newSaHandle = 0;
        cpssOsMemSet(&newTrRecParams, 0, sizeof(trRecParams));
        activeSaHandle = 0;

        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &newTrRecParams,
                                          &newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Prepare parameters */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        activeSaHandle  = 0;
        unitBmp   = BIT_0;

        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.params.egress.includeSci           = GT_FALSE;
        saParams.actionType                         = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                           = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.protectFrames        = GT_TRUE;
        saParams.params.egress.confProtect          = GT_TRUE;
        saParams.params.egress.controlledPortEnable = GT_TRUE;
        saParams.params.egress.preSecTagAuthLength  = 12;  /* MAC DA,SA */

        /* Call cpssDxChMacSecSecySaAdd with valid values */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSecySaChainSet with valid values.
                 Expected: GT_OK.
        */

        cpssOsMemSet(&newTrRecParams, 0, sizeof(newTrRecParams));
        newSaHandle  = 0;

        /* Set new transform record parameters */
        newTrRecParams.keyByteCount    = sizeof(K1);
        newTrRecParams.seqNumLo        = 0x6;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            newTrRecParams.keyArr[i] = K1[i]+1;
        }

        /* Set Hash key */
        newTrRecParams.customHkeyEnable = GT_TRUE;

        for (i=0;i<sizeof(hashKey);i++)
        {
            newTrRecParams.customHkeyArr[i] = hashKey[i];
        }

        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &newTrRecParams,
                                          &newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove first SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove new SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecySaChainSet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaChainSet(dev,
                                              6,
                                              activeSaHandle,
                                              &newTrRecParams,
                                              &newSaHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /*
           1.1.4 Call api with trRecParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          0,
                                          &newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /*
           1.1.5 Call api with newSaHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &newTrRecParams,
                                          0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /*
           1.1.6 Call api with invalid activeSaHandle.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          0,
                                          &newTrRecParams,
                                          &newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);



        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaChainSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    activeSaHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &newTrRecParams,
                                          &newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaChainSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecySaChainSet(dev,
                                      BIT_0,
                                      activeSaHandle,
                                      &newTrRecParams,
                                      &newSaHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaChainGet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     currentSaHandle,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *nextSaHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaChainGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaChainGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaChainGet with valid values
          Expected: GT_OK.

    1.1.3 Compare outputs from cpssDxChMacSecSecySaChainSet to cpssDxChMacSecSecySaChainGet
          Hnadles output should be the same.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecySaChainGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecySaChainGet with nextSaHandlePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSecySaChainGet with invalid activeSaHandle.
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecInit for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecInit with out of bound device id.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     newTrRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         activeSaHandle;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         newSaHandle;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         nextSaHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  i;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaChainGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Set values to zero */
        nextSaHandle = 0;
        cpssOsMemSet(&newTrRecParams, 0, sizeof(trRecParams));
        activeSaHandle = 0;

        st = cpssDxChMacSecSecySaChainGet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &nextSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Prepare parameters */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        activeSaHandle  = 0;
        unitBmp   = BIT_0;

        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.params.egress.includeSci           = GT_FALSE;
        saParams.actionType                         = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                           = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.protectFrames        = GT_TRUE;
        saParams.params.egress.confProtect          = GT_TRUE;
        saParams.params.egress.controlledPortEnable = GT_TRUE;
        saParams.params.egress.preSecTagAuthLength  = 12;  /* MAC DA,SA */

        /* Call cpssDxChMacSecSecySaAdd with valid values */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSecySaChainGet with valid values.
                 Expected: GT_OK.
        */

        cpssOsMemSet(&newTrRecParams, 0, sizeof(newTrRecParams));
        newSaHandle  = 0;

        /* Set new transform record parameters */
        newTrRecParams.keyByteCount    = sizeof(K1);
        newTrRecParams.seqNumLo        = 0x6;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            newTrRecParams.keyArr[i] = K1[i]+1;
        }

        st = cpssDxChMacSecSecySaChainSet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &newTrRecParams,
                                          &newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssDxChMacSecSecySaChainGet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &nextSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Compare outputs from cpssDxChMacSecSecySaChainSet to cpssDxChMacSecSecySaChainGet
                 Hnadles output should be the same.
                 Expected: GT_OK.
        */
        if (prvCpssMacSecSecySaHandlesCompare(newSaHandle,nextSaHandle))
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove first SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove new SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        newSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecSecySaChainGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaChainGet(dev,
                                              6,
                                              activeSaHandle,
                                              &nextSaHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /*
           1.1.5 Call api with nextSaHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaChainGet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /*
           1.1.6 Call api with invalid activeSaHandle.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaChainGet(dev,
                                          BIT_0,
                                          0,
                                          &nextSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);



        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaChainGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    activeSaHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaChainGet(dev,
                                          BIT_0,
                                          activeSaHandle,
                                          &nextSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    /*
        3.1. Call cpssDxChMacSecSecySaChainGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecySaChainGet(dev,
                                      BIT_0,
                                      activeSaHandle,
                                      &nextSaHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortSecyConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  *portCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortSecyConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecPortSecyConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortSecyConfigSet with Egress valid values.
          Call cpssDxChMacSecPortSecyConfigGet to check Egress parameters are the same ones that were set                                                                   .
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortSecyConfigSet with Ingress valid values.
          Call cpssDxChMacSecPortSecyConfigGet to check Ingress parameters are the same ones that were set                                                                   .
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecPortSecyConfigSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecPortSecyConfigSet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecPortSecyConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortSecyConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_PHYSICAL_PORT_NUM                port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT      direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  secyPortCfg;
    CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  secyPortCfgGet;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortSecyConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&secyPortCfg, 0, sizeof(secyPortCfg));
                st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                     port,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     &secyPortCfg);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortSecyConfigSet with Egress valid values.
                     Call cpssDxChMacSecPortSecyConfigGet to check Egress parameters are the same ones that were set                                                                   .
                     Expected: GT_OK.
            */
            cpssOsMemSet(&secyPortCfg, 0, sizeof(secyPortCfg));

            /* Common parameters (Egress & Ingress) */
            secyPortCfg.bypassMacsecDevice               = GT_FALSE;

            /* Egress parameters only */
            secyPortCfg.pktNumThrStrictCompareModeEnable = GT_TRUE;
            secyPortCfg.statCtrl.seqNrThreshold          = 0x12345678;
            secyPortCfg.statCtrl.seqNrThreshold64.l[0]   = 0x22222222;
            secyPortCfg.statCtrl.seqNrThreshold64.l[1]   = 0x33333333;

            /* Egress port setting */
            st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 &secyPortCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get port setting of Egress unit */
            st = cpssDxChMacSecPortSecyConfigGet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 &secyPortCfgGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Compare values of set and get */
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.bypassMacsecDevice               , secyPortCfgGet.bypassMacsecDevice               , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.pktNumThrStrictCompareModeEnable , secyPortCfgGet.pktNumThrStrictCompareModeEnable , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.statCtrl.seqNrThreshold          , secyPortCfgGet.statCtrl.seqNrThreshold          , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.statCtrl.seqNrThreshold64.l[0]   , secyPortCfgGet.statCtrl.seqNrThreshold64.l[0]   , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.statCtrl.seqNrThreshold64.l[1]   , secyPortCfgGet.statCtrl.seqNrThreshold64.l[1]   , dev);


            /*
               1.1.3 Call cpssDxChMacSecPortSecyConfigSet with Ingress valid values.
                     Call cpssDxChMacSecPortSecyConfigGet to check Ingress parameters are the same ones that were set                                                                   .
                     Expected: GT_OK.
            */
            cpssOsMemSet(&secyPortCfg, 0, sizeof(secyPortCfg));

            /* Common parameters (Egress & Ingress) */
            secyPortCfg.bypassMacsecDevice   = GT_FALSE;

            /* Ingress parameters only */
            secyPortCfg.ruleSecTag.compEtype = GT_TRUE;
            secyPortCfg.ruleSecTag.checkV    = GT_TRUE;
            secyPortCfg.ruleSecTag.checkKay  = GT_TRUE;
            secyPortCfg.ruleSecTag.checkCe   = GT_TRUE;
            secyPortCfg.ruleSecTag.checkSc   = GT_TRUE;
            secyPortCfg.ruleSecTag.checkSl   = GT_TRUE;
            secyPortCfg.ruleSecTag.checkPn   = GT_TRUE;

            /* Ingress port setting */
            st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                                 &secyPortCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get port setting of Ingress unit */
            st = cpssDxChMacSecPortSecyConfigGet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                                 &secyPortCfgGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Compare values of set and get */
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.bypassMacsecDevice   , secyPortCfgGet.bypassMacsecDevice   , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.compEtype , secyPortCfgGet.ruleSecTag.compEtype , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.checkV    , secyPortCfgGet.ruleSecTag.checkV    , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.checkKay  , secyPortCfgGet.ruleSecTag.checkKay  , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.checkCe   , secyPortCfgGet.ruleSecTag.checkCe   , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.checkSc   , secyPortCfgGet.ruleSecTag.checkSc   , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.checkSl   , secyPortCfgGet.ruleSecTag.checkSl   , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secyPortCfg.ruleSecTag.checkPn   , secyPortCfgGet.ruleSecTag.checkPn   , dev);


            /*
               1.1.4 Call cpssDxChMacSecPortSecyConfigSet with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortSecyConfigSet(dev, port, direction, &secyPortCfg), direction);

            /*
               1.1.5 Call cpssDxChMacSecPortSecyConfigSet with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 &secyPortCfg);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortSecyConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortSecyConfigSet(dev,
                                             port,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             &secyPortCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortSecyConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortSecyConfigSet(dev,
                                         port,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortSecyConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    OUT CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC   *portCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortSecyConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecPortSecyConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortSecyConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortSecyConfigGet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecPortSecyConfigSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecPortSecyConfigSet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecInit for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecInit with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_PHYSICAL_PORT_NUM                    port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  secyPortCfgGet;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortSecyConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&secyPortCfgGet, 0, sizeof(secyPortCfgGet));
                st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                     port,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     &secyPortCfgGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortSecyConfigGet with valid values.
                    Expected: GT_OK.
            */

            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            cpssOsMemSet(&secyPortCfgGet, 0, sizeof(secyPortCfgGet));
            /* Call cpssDxChMacSecPortSecyConfigGet to get port parameters */
            st = cpssDxChMacSecPortSecyConfigGet(dev,
                                                 port,
                                                 direction,
                                                 &secyPortCfgGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.4 Call cpssDxChMacSecPortSecyConfigGet with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortSecyConfigGet(dev, port, direction, &secyPortCfgGet), direction);

            /*
               1.1.5 Call cpssDxChMacSecPortSecyConfigGet with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortSecyConfigSet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortSecyConfigGet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 &secyPortCfgGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortSecyConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortSecyConfigGet(dev,
                                             port,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             &secyPortCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortSecyConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortSecyConfigGet(dev,
                                         port,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         &secyPortCfgGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyStatisticsTcamHitsGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           ruleId,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statTcamHitsCounterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyStatisticsTcamHitsGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with maximum valid ruleId value.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with maximum +1 ruleId value.
          Expected: GT_FAIL.

    1.1.5 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecSecySaChainGet with invalid unitBmp.
          unitBmp [6]                                                             .
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.7 Call api with statTcamHitsCounter as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecInit for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecInit with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                         st = GT_OK;
    GT_U8                             dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT    direction;
    GT_MACSEC_UNIT_BMP                unitBmp;
    GT_U32                            ruleId;
    GT_BOOL                           syncEnable;
    GT_U64                            statTcamHitsCounter;
    GT_U32                            maxClassifyRules;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Set values to zero */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;
        ruleId = 0;
        syncEnable = GT_FALSE;
        statTcamHitsCounter.l[0] = 0;
        statTcamHitsCounter.l[1] = 0;

        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         unitBmp,
                                                         direction,
                                                         ruleId,
                                                         syncEnable,
                                                         &statTcamHitsCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         unitBmp,
                                                         direction,
                                                         ruleId,
                                                         syncEnable,
                                                         &statTcamHitsCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Now call api with syncEnable = GT_TRUE */
        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         unitBmp,
                                                         direction,
                                                         ruleId,
                                                         GT_TRUE,
                                                         &statTcamHitsCounter);

        /*
           1.1.3 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with maximum valid ruleId value.
                 Expected: GT_OK.
        */

        /* Get maximum number of rules allowed in the device */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             NULL,
                                             NULL,
                                             &maxClassifyRules);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ruleId  = maxClassifyRules -1 ;
        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         unitBmp,
                                                         direction,
                                                         ruleId,
                                                         syncEnable,
                                                         &statTcamHitsCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with maximum +1 invalid ruleId value.
                 Expected: GT_FAIL.
        */

        ruleId  = maxClassifyRules ;
        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         unitBmp,
                                                         direction,
                                                         ruleId,
                                                         syncEnable,
                                                         &statTcamHitsCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);


        /*
           1.1.5 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev, unitBmp, direction, ruleId, syncEnable, &statTcamHitsCounter), direction);

        /*
           1.1.6 Call cpssDxChMacSecSecySaChainGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                             6,
                                                             direction,
                                                             ruleId,
                                                             syncEnable,
                                                             &statTcamHitsCounter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /*
           1.1.7 Call api with statTcamHitsCounter as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         unitBmp,
                                                         direction,
                                                         ruleId,
                                                         syncEnable,
                                                         0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyStatisticsTcamHitsGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                         BIT_0,
                                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                         0,
                                                         GT_FALSE,
                                                         &statTcamHitsCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyStatisticsTcamHitsGet(dev,
                                                     BIT_0,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     0,
                                                     GT_FALSE,
                                                     &statTcamHitsCounter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyStatisticsGet
(
    IN   GT_U8                                          devNum,
    IN   GT_PHYSICAL_PORT_NUM                           portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                 direction,
    IN   GT_BOOL                                        syncEnable,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC  *portStatPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyStatisticsGet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyStatisticsGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyStatisticsGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortClassifyStatisticsGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecPortClassifyStatisticsGet with NULL pointer.
          Expected: GT_BAD_PTR.

    1.2.1. Call cpssDxChMacSecPortClassifyStatisticsGet for each non-active port
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecPortClassifyStatisticsGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecPortClassifyStatisticsGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                        st = GT_OK;
    GT_U8                                            dev;
    GT_PHYSICAL_PORT_NUM                             port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT                   direction;
    CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC    portStatistics;
    GT_BOOL                                          syncEnable;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            syncEnable = GT_FALSE;

            /*
               1.1.1 Call cpssDxChMacSecPortClassifyStatisticsGet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                             port,
                                                             direction,
                                                             syncEnable,
                                                             &portStatistics);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyStatisticsGet with valid values.
                    Expected: GT_OK.
            */

            /* Call cpssDxChMacSecPortSecyConfigGet with syncEnable = GT_FALSE */
            st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                         port,
                                                         direction,
                                                         syncEnable,
                                                         &portStatistics);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /* Call cpssDxChMacSecPortSecyConfigGet with syncEnable = GT_TRUE */
            st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                         port,
                                                         direction,
                                                         GT_TRUE,
                                                         &portStatistics);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyStatisticsGet with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyStatisticsGet(dev, port, direction, syncEnable, &portStatistics), direction);

            /*
               1.1.4 Call cpssDxChMacSecPortClassifyStatisticsGet with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                         port,
                                                         direction,
                                                         syncEnable,
                                                         NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                         port,
                                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                         GT_FALSE,
                                                         &portStatistics);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyStatisticsGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                     port,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     GT_FALSE,
                                                     &portStatistics);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyStatisticsGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyStatisticsGet(dev,
                                                 port,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 GT_FALSE,
                                                 &portStatistics);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsSaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    IN   GT_BOOL                            syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT  *statSaPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsSaGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsSaGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsSaGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatisticsSaGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecSecyStatisticsSaGet with invalid unitBmp.
          unitBmp [6]                                                             .
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecyStatisticsSaGet with statTcamHitsCounter as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSecyStatisticsSaGet with invalid saHandle.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecSecyStatisticsSaGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSecyStatisticsSaGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_BOOL                                 syncEnable;
    CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT       statisticsSa;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex;
    GT_U32                                  i;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsSaGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;
        syncEnable = GT_FALSE;
        saHandle = 0;

        st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                               unitBmp,
                                               direction,
                                               saHandle,
                                               syncEnable,
                                               &statisticsSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsSaGet with valid values.
                 Expected: GT_OK.
        */

        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.params.egress.includeSci           = GT_FALSE;
        saParams.actionType                         = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                           = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.protectFrames        = GT_TRUE;
        saParams.params.egress.confProtect          = GT_TRUE;
        saParams.params.egress.controlledPortEnable = GT_TRUE;
        saParams.params.egress.preSecTagAuthLength  = 12;  /* MAC DA,SA */

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Call cpssDxChMacSecSecyStatisticsSaGet with valid values
           Set syncEnable as GT_TRUE */
        st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                               unitBmp,
                                               direction,
                                               saHandle,
                                               GT_TRUE,
                                               &statisticsSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Call cpssDxChMacSecSecyStatisticsSaGet with valid values
           Set syncEnable as GT_FALSE */
        st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                               unitBmp,
                                               direction,
                                               saHandle,
                                               GT_FALSE,
                                               &statisticsSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove first SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecyStatisticsSaGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyStatisticsSaGet(dev, unitBmp, direction, saHandle, syncEnable, &statisticsSa), direction);

        /*
           1.1.4 Call cpssDxChMacSecSecyStatisticsSaGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                                   6,
                                                   direction,
                                                   saHandle,
                                                   syncEnable,
                                                   &statisticsSa);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecSecyStatisticsSaGet with statisticsSa as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                               unitBmp,
                                               direction,
                                               saHandle,
                                               syncEnable,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecSecyStatisticsSaGet with invalid saHandle.
           Expected: GT_BAD_PARAM.
        */
        st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                               unitBmp,
                                               direction,
                                               0,
                                               syncEnable,
                                               &statisticsSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsSaGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    saHandle = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               saHandle,
                                               GT_TRUE,
                                               &statisticsSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsSaGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsSaGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           saHandle,
                                           GT_TRUE,
                                           &statisticsSa);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsSecyGet
(
    IN   GT_U8                                devNum,
    IN   GT_MACSEC_UNIT_BMP                   unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    IN   GT_U32                               vPort,
    IN   GT_BOOL                              syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT  *statSecyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsSecyGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsSecyGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsSecyGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with maximum valid vPortIndex value.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecyStatisticsSecyGet with maximum +1 vPortIndex value.
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecyStatisticsSecyGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecSecyStatisticsSecyGet with invalid unitBmp.
          unitBmp [6]                                                             .
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.7 Call cpssDxChMacSecSecyStatisticsSecyGet with statisticsSecyPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1   Call cpssDxChMacSecSecyStatisticsSecyGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSecyStatisticsSecyGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_BOOL                                 syncEnable;
    CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT     statisticsSecy;
    GT_U32                                  vPortIndex;
    GT_U32                                  maxSecyVports;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsSecyGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;
        syncEnable = GT_FALSE;
        vPortIndex = 0;

        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 vPortIndex,
                                                 syncEnable,
                                                 &statisticsSecy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsSecyGet with valid values.
                 Expected: GT_OK.
        */

        /* Call cpssDxChMacSecSecyStatisticsSecyGet with valid values
           Set syncEnable as GT_TRUE */
        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 vPortIndex,
                                                 GT_TRUE,
                                                 &statisticsSecy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Call cpssDxChMacSecSecyStatisticsSecyGet with valid values
           Set syncEnable as GT_FALSE */
        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 vPortIndex,
                                                 GT_FALSE,
                                                 &statisticsSecy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecClassifyStatisticsTcamHitsGet with maximum valid vPortIndex value.
                 Expected: GT_OK.
        */

        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         &maxSecyVports,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vPortIndex  = maxSecyVports -1 ;
        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 vPortIndex,
                                                 syncEnable,
                                                 &statisticsSecy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecSecyStatisticsSecyGet with maximum +1 invalid vPortIndex value.
                 Expected: GT_BAD_PARAM.
        */

        vPortIndex  = maxSecyVports ;
        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 vPortIndex,
                                                 syncEnable,
                                                 &statisticsSecy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
           1.1.5 Call cpssDxChMacSecSecyStatisticsSecyGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyStatisticsSecyGet(dev, unitBmp, direction, vPortIndex, syncEnable, &statisticsSecy), direction);

        /*
           1.1.6 Call cpssDxChMacSecSecyStatisticsSecyGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                     6,
                                                     direction,
                                                     vPortIndex,
                                                     syncEnable,
                                                     &statisticsSecy);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /*
           1.1.7 Call cpssDxChMacSecSecyStatisticsSecyGet with statisticsSecy as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 vPortIndex,
                                                 syncEnable,
                                                 NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsSecyGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                                 BIT_0,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 0,
                                                 GT_TRUE,
                                                 &statisticsSecy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsSecyGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsSecyGet(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             0,
                                             GT_TRUE,
                                             &statisticsSecy);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsIfcGet
(
    IN   GT_U8                                devNum,
    IN   GT_MACSEC_UNIT_BMP                   unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    IN   GT_U32                               vPort,
    IN   GT_BOOL                              syncEnable,
    OUT  CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT   *statIfcPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsIfcGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsIfcGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsIfcGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatisticsIfcGet with maximum valid vPortIndex value.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecyStatisticsIfcGet with maximum +1 vPortIndex value.
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecyStatisticsIfcGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecSecyStatisticsIfcGet with invalid unitBmp.
          unitBmp [6]                                                             .
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.7 Call cpssDxChMacSecSecyStatisticsSecyGet with statisticsSecyPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1   Call cpssDxChMacSecSecyStatisticsIfcGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSecyStatisticsIfcGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_BOOL                                 syncEnable;
    CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT      statisticsIfc;
    GT_U32                                  vPortIndex;
    GT_U32                                  maxSecyVports;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsIfcGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;
        syncEnable = GT_FALSE;
        vPortIndex = 0;

        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                unitBmp,
                                                direction,
                                                vPortIndex,
                                                syncEnable,
                                                &statisticsIfc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsIfcGet with valid values.
                 Expected: GT_OK.
        */

        /* Call cpssDxChMacSecSecyStatisticsIfcGet with valid values
           Set syncEnable as GT_TRUE */
        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                unitBmp,
                                                direction,
                                                vPortIndex,
                                                GT_TRUE,
                                                &statisticsIfc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call cpssDxChMacSecSecyStatisticsIfcGet with valid values
           Set syncEnable as GT_FALSE */
        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                unitBmp,
                                                direction,
                                                vPortIndex,
                                                GT_FALSE,
                                                &statisticsIfc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecyStatisticsIfcGet with maximum valid vPortIndex value.
                 Expected: GT_OK.
        */

        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         &maxSecyVports,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vPortIndex  = maxSecyVports -1 ;

        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                unitBmp,
                                                direction,
                                                vPortIndex,
                                                syncEnable,
                                                &statisticsIfc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecSecyStatisticsIfcGet with maximum +1 invalid vPortIndex value.
                 Expected: GT_BAD_PARAM.
        */

        vPortIndex  = maxSecyVports ;
        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                unitBmp,
                                                direction,
                                                vPortIndex,
                                                syncEnable,
                                                &statisticsIfc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
           1.1.5 Call cpssDxChMacSecSecyStatisticsIfcGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyStatisticsIfcGet(dev, unitBmp, direction, vPortIndex, syncEnable, &statisticsIfc), direction);

        /*
           1.1.6 Call cpssDxChMacSecSecyStatisticsIfcGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                    6,
                                                    direction,
                                                    vPortIndex,
                                                    syncEnable,
                                                    &statisticsIfc);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.7 Call cpssDxChMacSecSecyStatisticsIfcGet with statisticsIfc as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                unitBmp,
                                                direction,
                                                vPortIndex,
                                                syncEnable,
                                                NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsIfcGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                                BIT_0,
                                                CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                0,
                                                GT_TRUE,
                                                &statisticsIfc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsIfcGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsIfcGet(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            0,
                                            GT_TRUE,
                                            &statisticsIfc);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsRxCamGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   GT_U32                           scIndex,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statRxCamCounterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsRxCamGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsRxCamGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsRxCamGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatisticsRxCamGet with maximum valid scIndex value.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecyStatisticsRxCamGet with maximum +1 scIndex value.
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecyStatisticsRxCamGet with invalid unitBmp.
          unitBmp [6]                                                             .
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.6 Call cpssDxChMacSecSecyStatisticsRxCamGet with statisticsSecyPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1   Call cpssDxChMacSecSecyStatisticsRxCamGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSecyStatisticsRxCamGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_MACSEC_UNIT_BMP                  unitBmp;
    GT_BOOL                             syncEnable;
    GT_U64                              statisticsRxCam;
    GT_U32                              scIndex;
    GT_U32                              maxSecySc;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsRxCamGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        unitBmp = BIT_0;
        syncEnable = GT_FALSE;
        scIndex = 0;

        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  unitBmp,
                                                  scIndex,
                                                  syncEnable,
                                                  &statisticsRxCam);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsRxCamGet with valid values.
                 Expected: GT_OK.
        */

        /* Call cpssDxChMacSecSecyStatisticsRxCamGet with valid values
           Set syncEnable as GT_TRUE */
        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  unitBmp,
                                                  scIndex,
                                                  GT_TRUE,
                                                  &statisticsRxCam);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call cpssDxChMacSecSecyStatisticsRxCamGet with valid values
           Set syncEnable as GT_FALSE */
        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  unitBmp,
                                                  scIndex,
                                                  GT_FALSE,
                                                  &statisticsRxCam);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecyStatisticsRxCamGet with maximum valid scIndex value.
                 Expected: GT_OK.
        */

        /* Get maximum number of SC allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &maxSecySc);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        scIndex  = maxSecySc -1 ;

        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  unitBmp,
                                                  scIndex,
                                                  syncEnable,
                                                  &statisticsRxCam);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecSecyStatisticsRxCamGet with maximum +1 scIndex value.
                 Expected: GT_BAD_PARAM.
        */

        scIndex  = maxSecySc ;
        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  unitBmp,
                                                  scIndex,
                                                  syncEnable,
                                                  &statisticsRxCam);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
           1.1.5 Call cpssDxChMacSecSecyStatisticsRxCamGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM                                                        .
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                      6,
                                                      scIndex,
                                                      syncEnable,
                                                      &statisticsRxCam);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.6 Call cpssDxChMacSecSecyStatisticsRxCamGet with statisticsRxCam as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  unitBmp,
                                                  scIndex,
                                                  syncEnable,
                                                  NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);



        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsRxCamGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                                  BIT_0,
                                                  0,
                                                  GT_TRUE,
                                                  &statisticsRxCam);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsRxCamGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsRxCamGet(dev,
                                              BIT_0,
                                              0,
                                              GT_TRUE,
                                              &statisticsRxCam);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsVportClear
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPort
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsVportClear)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsVportClear before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsVportClear with valid parameters including maximum valid vPortIndex value.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatisticsVportClear with maximum +1 vPortIndex value.
          Expected: GT_FAIL.

    1.1.4 Call cpssDxChMacSecSecyStatisticsVportClear with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecyStatisticsVportClear with invalid unitBmp.
          unitBmp [6]                                                             .
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    2.1   Call cpssDxChMacSecSecyStatisticsVportClear and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSecyStatisticsVportClear with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  vPortIndex;
    GT_U32                                  maxSecyVports;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsVportClear before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;
        vPortIndex = 0;

        st = cpssDxChMacSecSecyStatisticsVportClear(dev,
                                                    unitBmp,
                                                    direction,
                                                    vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsVportClear with valid values and with maximum valid vPortIndex value.
                 Expected: GT_OK.
        */

        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         &maxSecyVports,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vPortIndex  = maxSecyVports -1 ;

        /* Call cpssDxChMacSecSecyStatisticsVportClear with valid values */
        st = cpssDxChMacSecSecyStatisticsVportClear(dev,
                                                    unitBmp,
                                                    direction,
                                                    vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecyStatisticsVportClear with maximum +1 vPortIndex value.
                 Expected: GT_FAIL.
        */
        vPortIndex  = maxSecyVports ;
        st = cpssDxChMacSecSecyStatisticsVportClear(dev,
                                                    unitBmp,
                                                    direction,
                                                    vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);


        /*
           1.1.4 Call cpssDxChMacSecSecyStatisticsVportClear with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyStatisticsVportClear(dev, unitBmp, direction, vPortIndex), direction);

        /*
           1.1.5 Call cpssDxChMacSecSecyStatisticsVportClear with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsVportClear(dev,
                                                        6,
                                                        direction,
                                                        vPortIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsVportClear for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsVportClear(dev,
                                                    BIT_0,
                                                    CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                    0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsVportClear with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsVportClear(dev,
                                                BIT_0,
                                                CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet
(
    IN  GT_U8                                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                    st = GT_OK;
    GT_U8                                        dev;
    GT_PHYSICAL_PORT_NUM                         port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT               direction;
    CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  secTagParser;
    CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  secTagParserGet;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&secTagParser, 0, sizeof(secTagParser));
                st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev,
                                                                        port,
                                                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                        &secTagParser);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet with valid values.
                    Expected: GT_OK.
            */

            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            cpssOsMemSet(&secTagParser, 0, sizeof(secTagParser));
            secTagParser.checkKay       = GT_TRUE;
            secTagParser.checkVersion   = GT_TRUE;
            secTagParser.compType       = GT_TRUE;
            secTagParser.lookupUseSci   = GT_FALSE;
            secTagParser.macSecTagValue = 0x88e5;
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev,
                                                                    port,
                                                                    direction,
                                                                    &secTagParser);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with the same parameters.
               Expected: GT_OK and the same values that were set.
            */
            cpssOsMemSet(&secTagParserGet, 0, sizeof(secTagParserGet));
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                                    port,
                                                                    direction,
                                                                    &secTagParserGet);
            /* Verify set and get parameters are the same */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secTagParser.checkKay       , secTagParserGet.checkKay , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secTagParser.checkVersion   , secTagParserGet.checkVersion , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secTagParser.compType       , secTagParserGet.compType , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secTagParser.lookupUseSci   , secTagParserGet.lookupUseSci , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(secTagParser.macSecTagValue , secTagParserGet.macSecTagValue , dev);

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev, port, direction, &secTagParser), direction);

            /*
               1.1.5 Call api with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev,
                                                                    port,
                                                                    CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                    NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev,
                                                                    port,
                                                                    CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                    &secTagParser);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev,
                                                                port,
                                                                CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                &secTagParser);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(dev,
                                                            port,
                                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                            &secTagParser);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet
(
    IN   GT_U8                                        devNum,
    IN   GT_PHYSICAL_PORT_NUM                         portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.1 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with valid values.
          Expected: GT_OK.

    1.1.2 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with NULL pointer
          Expected: GT_BAD_PARAM

    1.1.3 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                    st = GT_OK;
    GT_U8                                        dev;
    GT_PHYSICAL_PORT_NUM                         port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT               direction;
    CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  secTagParser;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&secTagParser, 0, sizeof(secTagParser));
                st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                                        port,
                                                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                        &secTagParser);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with valid values.
                     Expected: GT_OK.
            */
            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E ;
            cpssOsMemSet(&secTagParser, 0, sizeof(secTagParser));
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                                    port,
                                                                    direction,
                                                                    &secTagParser);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with secTagParser [NULL]
                     Expected: GT_BAD_PTR
            */
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                                    port,
                                                                    direction,
                                                                    NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev, port, direction, &secTagParser), direction);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                                    port,
                                                                    CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                    &secTagParser);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /*
        2.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                                port,
                                                                CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                &secTagParser);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(dev,
                                                            port,
                                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                            &secTagParser);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyHdrParserVlanConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyHdrParserVlanConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                  st = GT_OK;
    GT_U8                                      dev;
    GT_PHYSICAL_PORT_NUM                       port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT             direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  vlanParser;
    CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  vlanParserGet;
    GT_U32                                     i;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&vlanParser, 0, sizeof(vlanParser));
                st = cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev,
                                                                      port,
                                                                      CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                      &vlanParser);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with valid values.
                    Expected: GT_OK.
            */

            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            cpssOsMemSet(&vlanParser, 0, sizeof(vlanParser));
            vlanParser.cp.parseQTag = GT_TRUE;
            vlanParser.defaultUp    = 2;
            vlanParser.qTag         = 0x1234;
            vlanParser.qTagUpEnable = GT_TRUE;
            for (i=0;i<8;i++)
            {
                vlanParser.upTable1[i] = i;
            }
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev,
                                                                  port,
                                                                  direction,
                                                                  &vlanParser);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with the same parameters.
               Expected: GT_OK and the same values that were set.
            */
            cpssOsMemSet(&vlanParserGet, 0, sizeof(vlanParserGet));
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                                  port,
                                                                  direction,
                                                                  &vlanParserGet);
            /* Verify set and get parameters are the same */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(vlanParser.cp.parseQTag , vlanParserGet.cp.parseQTag , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(vlanParser.defaultUp    , vlanParserGet.defaultUp    , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(vlanParser.qTag         , vlanParserGet.qTag         , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(vlanParser.qTagUpEnable , vlanParserGet.qTagUpEnable , dev);
            for (i=0;i<8;i++)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(vlanParser.upTable1[i] , vlanParserGet.upTable1[i] , dev);
            }

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev, port, direction, &vlanParser), direction);

            /*
               1.1.5 Call api with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev,
                                                                  port,
                                                                  CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                  NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev,
                                                                  port,
                                                                  CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                  &vlanParser);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev,
                                                              port,
                                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                              &vlanParser);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(dev,
                                                          port,
                                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                          &vlanParser);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyHdrParserVlanConfigGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PHYSICAL_PORT_NUM                       portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyHdrParserVlanConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.1 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with valid values.
          Expected: GT_OK.

    1.1.2 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with NULL pointer
          Expected: GT_BAD_PARAM

    1.1.3 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                  st = GT_OK;
    GT_U8                                      dev;
    GT_PHYSICAL_PORT_NUM                       port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT             direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  vlanParser;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&vlanParser, 0, sizeof(vlanParser));
                st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                                      port,
                                                                      CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                      &vlanParser);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with valid values.
                     Expected: GT_OK.
            */
            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E ;
            cpssOsMemSet(&vlanParser, 0, sizeof(vlanParser));
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                                  port,
                                                                  direction,
                                                                  &vlanParser);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with vlanParser [NULL]
                     Expected: GT_BAD_PTR
            */
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                                  port,
                                                                  direction,
                                                                  NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev, port, direction, &vlanParser), direction);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                                  port,
                                                                  CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                  &vlanParser);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /*
        2.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                              port,
                                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                              &vlanParser);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyHdrParserVlanConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(dev,
                                                          port,
                                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                          &vlanParser);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyStatisticsConfigSet
(
    IN  GT_U8                                             devNum,
    IN  GT_MACSEC_UNIT_BMP                                unitBmp,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyStatisticsConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyStatisticsConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyStatisticsConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyStatisticsConfigGet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecClassifyStatisticsConfigSet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyStatisticsConfigSet with statisticsCfgPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyStatisticsConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyStatisticsConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                         st = GT_OK;
    GT_U8                                             dev;
    GT_MACSEC_UNIT_BMP                                unitBmp;
    CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  statisticsCfg;
    CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  statisticsCfgGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyStatisticsConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        cpssOsMemSet(&statisticsCfg, 0, sizeof(statisticsCfg));
        st = cpssDxChMacSecClassifyStatisticsConfigSet(dev,
                                                       BIT_0,
                                                       &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyStatisticsConfigSet with valid values.
                 Expected: GT_OK.
        */
        unitBmp = BIT_0;
        statisticsCfg.autoStatCntrsReset     = GT_TRUE;
        statisticsCfg.countFrameThr.l[0]     = 0x200;
        statisticsCfg.countFrameThr.l[1]     = 0x300;
        statisticsCfg.countIncDisCtrl        = CPSS_DXCH_MACSEC_CLASSIFY_TCAM_COUNT_INC_DIS_CNS;
        statisticsCfg.portCountFrameThr.l[0] = 0x101;
        statisticsCfg.portCountFrameThr.l[1] = 0x404;

        st = cpssDxChMacSecClassifyStatisticsConfigSet(dev,
                                                       unitBmp,
                                                       &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecClassifyStatisticsConfigGet with the same parameters.
           Expected: GT_OK and the same values that were set.
        */
        cpssOsMemSet(&statisticsCfgGet, 0, sizeof(statisticsCfgGet));
        st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                       unitBmp,
                                                       &statisticsCfgGet);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.autoStatCntrsReset     , statisticsCfgGet.autoStatCntrsReset     , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.countFrameThr.l[0]     , statisticsCfgGet.countFrameThr.l[0]     , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.countFrameThr.l[1]     , statisticsCfgGet.countFrameThr.l[1]     , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.countIncDisCtrl        , statisticsCfgGet.countIncDisCtrl        , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.portCountFrameThr.l[0] , statisticsCfgGet.portCountFrameThr.l[0] , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.portCountFrameThr.l[1] , statisticsCfgGet.portCountFrameThr.l[1] , dev);

        /*
           1.1.4 Call cpssDxChMacSecClassifyStatisticsConfigSet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyStatisticsConfigSet(dev,
                                                           6,
                                                           &statisticsCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyStatisticsConfigSet(dev,
                                                       unitBmp,
                                                       NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyStatisticsConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyStatisticsConfigSet(dev,
                                                       BIT_0,
                                                       &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyStatisticsConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyStatisticsConfigSet(dev,
                                                   BIT_0,
                                                   &statisticsCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyStatisticsConfigGet
(
    IN  GT_U8                                             devNum,
    IN  GT_MACSEC_UNIT_BMP                                unitBmp,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyStatisticsConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyStatisticsConfigGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyStatisticsConfigGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyStatisticsConfigGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyStatisticsConfigGet with statisticsCfgPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyStatisticsConfigGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyStatisticsConfigGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                         st = GT_OK;
    GT_U8                                             dev;
    GT_MACSEC_UNIT_BMP                                unitBmp;
    CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  statisticsCfg;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyStatisticsConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        cpssOsMemSet(&statisticsCfg, 0, sizeof(statisticsCfg));
        st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                       BIT_0,
                                                       &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyStatisticsConfigGet with valid parameters.
           Expected: GT_OK
        */
        unitBmp = BIT_0;
        st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                       unitBmp,
                                                       &statisticsCfg);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyStatisticsConfigGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                           6,
                                                           &statisticsCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call api with NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                       unitBmp,
                                                       NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyStatisticsConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                       BIT_0,
                                                       &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyStatisticsConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyStatisticsConfigGet(dev,
                                                   BIT_0,
                                                   &statisticsCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyControlPktDetectConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyControlPktDetectConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                  st = GT_OK;
    GT_U8                                      dev;
    GT_PHYSICAL_PORT_NUM                       port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT             direction;
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  ctrlPktDetect;
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  ctrlPktDetectGet;
    GT_U32                                     i;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&ctrlPktDetect, 0, sizeof(ctrlPktDetect));
                st = cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev,
                                                                         port,
                                                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                         &ctrlPktDetect);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with valid values.
                    Expected: GT_OK.
            */
            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            cpssOsMemSet(&ctrlPktDetect, 0, sizeof(ctrlPktDetect));

            ctrlPktDetect.macDaEtRules[0].etherType                           = 0x0800;
            ctrlPktDetect.macDaEtRules[0].etherTypeMask.primaryMask           = CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ENABLE_COMPARE_E;
            ctrlPktDetect.macDaEtRules[0].etherTypeMatchMode.primaryMatchMode = CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_OUTER_E;
            for (i=0;i<6;i++)
            {
                ctrlPktDetect.macDaEtRules[0].macDa.arEther[i] = 0x11 + i;
            }
            ctrlPktDetect.macDaEtRules[0].macDaMask.primaryMask = CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ENABLE_COMPARE_E;

            st = cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev,
                                                                     port,
                                                                     direction,
                                                                     &ctrlPktDetect);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with the same parameters.
               Expected: GT_OK and the same values that were set.
            */
            cpssOsMemSet(&ctrlPktDetectGet, 0, sizeof(ctrlPktDetectGet));
            st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                                     port,
                                                                     direction,
                                                                     &ctrlPktDetectGet);
            /* Verify set and get parameters are the same */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(ctrlPktDetect.macDaEtRules[0].etherType                            , ctrlPktDetectGet.macDaEtRules[0].etherType                            , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(ctrlPktDetect.macDaEtRules[0].etherTypeMask.primaryMask            , ctrlPktDetectGet.macDaEtRules[0].etherTypeMask.primaryMask            , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(ctrlPktDetect.macDaEtRules[0].etherTypeMatchMode.primaryMatchMode  , ctrlPktDetectGet.macDaEtRules[0].etherTypeMatchMode.primaryMatchMode  , dev);
            for (i=0;i<6;i++)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(ctrlPktDetect.macDaEtRules[0].macDa.arEther[i] , ctrlPktDetectGet.macDaEtRules[0].macDa.arEther[i] , dev);
            }

            /*
               1.1.4 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev, port, direction, &ctrlPktDetect), direction);

            /*
               1.1.5 Call api with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev,
                                                                     port,
                                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                     NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev,
                                                                     port,
                                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                     &ctrlPktDetect);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev,
                                                                 port,
                                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                 &ctrlPktDetect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyControlPktDetectConfigSet(dev,
                                                             port,
                                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                             &ctrlPktDetect);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyControlPktDetectConfigGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyControlPktDetectConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                  st = GT_OK;
    GT_U8                                      dev;
    GT_PHYSICAL_PORT_NUM                       port = 0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT             direction;
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  ctrlPktDetect;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                cpssOsMemSet(&ctrlPktDetect, 0, sizeof(ctrlPktDetect));
                st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                                         port,
                                                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                         &ctrlPktDetect);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with valid values.
                    Expected: GT_OK.
            */
            direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
            cpssOsMemSet(&ctrlPktDetect, 0, sizeof(ctrlPktDetect));

            st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                                     port,
                                                                     direction,
                                                                     &ctrlPktDetect);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.3 Call api with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev, port, direction, &ctrlPktDetect), direction);

            /*
               1.1.4 Call api with NULL pointer.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                                     port,
                                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                     NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                                     port,
                                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                     &ctrlPktDetect);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                                 port,
                                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                                 &ctrlPktDetect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyControlPktDetectConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyControlPktDetectConfigGet(dev,
                                                             port,
                                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                             &ctrlPktDetect);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsConfigSet
(
    IN  GT_U8                                         devNum,
    IN  GT_MACSEC_UNIT_BMP                            unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsConfigSet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsConfigSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsConfigSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatisticsConfigGet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecSecyStatisticsConfigSet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecyStatisticsConfigSet with statisticsCfgPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecSecyStatisticsConfigSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecyStatisticsConfigSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_MACSEC_UNIT_BMP                            unitBmp;
    CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  statisticsCfg;
    CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  statisticsCfgGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsConfigSet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        cpssOsMemSet(&statisticsCfg, 0, sizeof(statisticsCfg));
        st = cpssDxChMacSecSecyStatisticsConfigSet(dev,
                                                   BIT_0,
                                                   &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsConfigSet with valid values.
                 Expected: GT_OK.
        */
        unitBmp = BIT_0;
        statisticsCfg.autoStatCntrsReset      = GT_TRUE;
        statisticsCfg.countIncDisCtrl         = (CPSS_DXCH_MACSEC_SECY_SA_COUNT_INC_DIS_CNS | CPSS_DXCH_MACSEC_SECY_IFC_COUNT_INC_DIS_CNS);
        statisticsCfg.ifc1CountFrameThr.l[0]  = 0x11111111;
        statisticsCfg.ifc1CountFrameThr.l[1]  = 0x1;
        statisticsCfg.ifc1CountOctetThr.l[0]  = 0x22222222;
        statisticsCfg.ifc1CountOctetThr.l[1]  = 0x2;
        statisticsCfg.ifcCountFrameThr.l[0]   = 0x33333333;
        statisticsCfg.ifcCountFrameThr.l[1]   = 0x3;
        statisticsCfg.ifcCountOctetThr.l[0]   = 0x44444444;
        statisticsCfg.ifcCountOctetThr.l[1]   = 0x4;
        statisticsCfg.rxCamCountFrameThr.l[0] = 0x55555555;
        statisticsCfg.rxCamCountFrameThr.l[1] = 0x5;
        statisticsCfg.saCountFrameThr.l[0]    = 0x66666666;
        statisticsCfg.saCountFrameThr.l[1]    = 0x6;
        statisticsCfg.saCountOctetThr.l[0]    = 0x77777777;
        statisticsCfg.saCountOctetThr.l[1]    = 0x7;
        statisticsCfg.secyCountFrameThr.l[0]  = 0x88888888;
        statisticsCfg.secyCountFrameThr.l[1]  = 0x8;
        statisticsCfg.seqNrThreshold          = 0x99999999;
        statisticsCfg.seqNrThreshold64.l[0]   = 0xaaaaaaaa;
        statisticsCfg.seqNrThreshold64.l[1]   = 0xa;

        st = cpssDxChMacSecSecyStatisticsConfigSet(dev,
                                                   unitBmp,
                                                   &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecyStatisticsConfigGet with the same parameters.
           Expected: GT_OK and the same values that were set.
        */
        cpssOsMemSet(&statisticsCfgGet, 0, sizeof(statisticsCfgGet));
        st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                                   unitBmp,
                                                   &statisticsCfgGet);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.autoStatCntrsReset      , statisticsCfgGet.autoStatCntrsReset      , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.countIncDisCtrl         , statisticsCfgGet.countIncDisCtrl         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifc1CountFrameThr.l[0]  , statisticsCfgGet.ifc1CountFrameThr.l[0]  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifc1CountFrameThr.l[1]  , statisticsCfgGet.ifc1CountFrameThr.l[1]  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifc1CountOctetThr.l[0]  , statisticsCfgGet.ifc1CountOctetThr.l[0]  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifc1CountOctetThr.l[1]  , statisticsCfgGet.ifc1CountOctetThr.l[1]  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifcCountFrameThr.l[0]   , statisticsCfgGet.ifcCountFrameThr.l[0]   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifcCountFrameThr.l[1]   , statisticsCfgGet.ifcCountFrameThr.l[1]   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifcCountOctetThr.l[0]   , statisticsCfgGet.ifcCountOctetThr.l[0]   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.ifcCountOctetThr.l[1]   , statisticsCfgGet.ifcCountOctetThr.l[1]   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.rxCamCountFrameThr.l[0] , statisticsCfgGet.rxCamCountFrameThr.l[0] , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.rxCamCountFrameThr.l[1] , statisticsCfgGet.rxCamCountFrameThr.l[1] , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.saCountFrameThr.l[0]    , statisticsCfgGet.saCountFrameThr.l[0]    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.saCountFrameThr.l[1]    , statisticsCfgGet.saCountFrameThr.l[1]    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.saCountOctetThr.l[0]    , statisticsCfgGet.saCountOctetThr.l[0]    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.saCountOctetThr.l[1]    , statisticsCfgGet.saCountOctetThr.l[1]    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.secyCountFrameThr.l[0]  , statisticsCfgGet.secyCountFrameThr.l[0]  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.secyCountFrameThr.l[1]  , statisticsCfgGet.secyCountFrameThr.l[1]  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.seqNrThreshold          , statisticsCfgGet.seqNrThreshold          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.seqNrThreshold64.l[0]   , statisticsCfgGet.seqNrThreshold64.l[0]   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(statisticsCfg.seqNrThreshold64.l[1]   , statisticsCfgGet.seqNrThreshold64.l[1]   , dev);


        /*
           1.1.4 Call cpssDxChMacSecSecyStatisticsConfigSet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsConfigSet(dev,
                                                       6,
                                                       &statisticsCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatisticsConfigSet(dev,
                                                   unitBmp,
                                                   NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsConfigSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsConfigSet(dev,
                                                   BIT_0,
                                                   &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsConfigSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsConfigSet(dev,
                                               BIT_0,
                                               &statisticsCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatisticsConfigGet
(
    IN  GT_U8                                         devNum,
    IN  GT_MACSEC_UNIT_BMP                            unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatisticsConfigGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatisticsConfigGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatisticsConfigGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatisticsConfigGet with the same parameters.
          Expected: GT_OK and the same values that were set.

    1.1.4 Call cpssDxChMacSecSecyStatisticsConfigGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecyStatisticsConfigGet with statisticsCfgPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecSecyStatisticsConfigGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecyStatisticsConfigGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_MACSEC_UNIT_BMP                            unitBmp;
    CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  statisticsCfg;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatisticsConfigGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        cpssOsMemSet(&statisticsCfg, 0, sizeof(statisticsCfg));
        st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                                   BIT_0,
                                                   &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyStatisticsConfigGet with valid values.
                 Expected: GT_OK.
        */
        unitBmp = BIT_0;
        st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                                   unitBmp,
                                                   &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecyStatisticsConfigGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                                       6,
                                                       &statisticsCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                                   unitBmp,
                                                   NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatisticsConfigGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                                   BIT_0,
                                                   &statisticsCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatisticsConfigGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatisticsConfigGet(dev,
                                               BIT_0,
                                               &statisticsCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportSet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyVportGet and verify the values are the same as they were set                                                                        .
           Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyVportSet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyVportSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyVportSet with vPortParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyVportSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParamsGet;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyVportSet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        vPortHandle = 1;
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        st = cpssDxChMacSecClassifyVportSet(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            vPortHandle,
                                            &vPortParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportSet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now set this vPort and update soem partameters */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E ;
        vPortParams.secTagOffset = 12;

        /* Set vPort parameters */
        st = cpssDxChMacSecClassifyVportSet(dev,
                                            unitBmp,
                                            direction,
                                            vPortHandle,
                                            &vPortParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecClassifyVportGet and verified values are the same as it was set.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecClassifyVportGet(dev,
                                            unitBmp,
                                            direction,
                                            vPortHandle,
                                            &vPortParamsGet);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(vPortParams.pktExpansion , vPortParamsGet.pktExpansion , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(vPortParams.secTagOffset , vPortParamsGet.secTagOffset , dev);

         /*
           1.1.4 Call cpssDxChMacSecClassifyVportSet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyVportSet(dev,
                                                6,
                                                direction,
                                                vPortHandle,
                                                &vPortParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyVportSet(dev, unitBmp, direction, vPortHandle, &vPortParams), direction);

        /*
           1.1.6 Call api with vPortParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportSet(dev,
                                            unitBmp,
                                            direction,
                                            vPortHandle,
                                            NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    vPortHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportSet(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            vPortHandle,
                                            &vPortParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    vPortHandle = 1;

    st = cpssDxChMacSecClassifyVportSet(dev,
                                        BIT_0,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        vPortHandle,
                                        &vPortParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportGet with valid values.
          Expected: GT_OK.

    1.1.3 Verify values are the same as they were set in add vPort API.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyVportGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyVportGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyVportGet with vPortParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyVportGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParamsGet;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyVportGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        vPortHandle = 1;
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        st = cpssDxChMacSecClassifyVportGet(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            vPortHandle,
                                            &vPortParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Now set this vPort and update some partameters */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E ;
        vPortParams.secTagOffset = 16;
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChMacSecClassifyVportGet(dev,
                                            unitBmp,
                                            direction,
                                            vPortHandle,
                                            &vPortParamsGet);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Verify values are the same as they wer set in add vPort API.
                 Expected: GT_OK.
        */

        UTF_VERIFY_EQUAL1_PARAM_MAC(vPortParams.pktExpansion , vPortParamsGet.pktExpansion , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(vPortParams.secTagOffset , vPortParamsGet.secTagOffset , dev);

         /*
           1.1.4 Call cpssDxChMacSecClassifyVportGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyVportGet(dev,
                                                6,
                                                direction,
                                                vPortHandle,
                                                &vPortParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyVportGet(dev, unitBmp, direction, vPortHandle, &vPortParams), direction);

        /*
           1.1.6 Call api with vPortParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportGet(dev,
                                            unitBmp,
                                            direction,
                                            vPortHandle,
                                            NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    vPortHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportGet(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            vPortHandle,
                                            &vPortParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    vPortHandle = 1;

    st = cpssDxChMacSecClassifyVportGet(dev,
                                        BIT_0,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        vPortHandle,
                                        &vPortParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  vPortIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportHandleGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportHandleGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportHandleGet with valid values.
          Expected: GT_OK.

    1.1.3 Verify values are the same as they were set in add vPort API.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyVportHandleGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyVportHandleGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyVportHandleGet with vPortParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call api with invalid vPortId.
          vPortId [maximum number of vPorts + 1]                                   .
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyVportHandleGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportHandleGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandleGet;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  vPortId;
    GT_U32                                  maxClassifyVports;



    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyVportHandleGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        vPortId = 0;
        st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                                  BIT_0,
                                                  CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                  vPortId,
                                                  &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportHandleGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Now set this vPort and update some partameters */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E ;
        vPortParams.secTagOffset = 12;
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now get the vPort handle associated with vPortId */
        st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                                  unitBmp,
                                                  direction,
                                                  vPortId,
                                                  &vPortHandleGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Verify vPort handle is the same one as from add vPort
                 Expected: GT_OK.
        */

        if (prvCpssMacSecClassifyVportHandlesCompare(vPortHandleGet, vPortHandle))
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


         /*
           1.1.4 Call cpssDxChMacSecClassifyVportHandleGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                                      6,
                                                      direction,
                                                      vPortId,
                                                      &vPortHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyVportHandleGet(dev, unitBmp, direction, vPortId, &vPortHandle), direction);

        /*
           1.1.6 Call api with vPortHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                                  unitBmp,
                                                  direction,
                                                  vPortId,
                                                  NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with invalid vPortId.
           vPortId [maximum number of values + 1]                                   .
           Expected: GT_BAD_PARAM.
        */
        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             NULL,
                                             &maxClassifyVports,
                                             NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call API with vPortId set to maximum number of vPorts that for index value it is 1 above valid range */
        vPortId = maxClassifyVports;
        st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                                  unitBmp,
                                                  direction,
                                                  vPortId,
                                                  &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportHandleGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    vPortHandle = 1;
    vPortId = 0;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                                  BIT_0,
                                                  CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                  vPortId,
                                                  &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportHandleGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    vPortHandle = 1;
    vPortId = 0;

    st = cpssDxChMacSecClassifyVportHandleGet(dev,
                                              BIT_0,
                                              CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                              vPortId,
                                              &vPortHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaSet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC      *saParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaSet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaSet and verify the values are the same as they were set                                                                        .
           Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecySaSet with sa parameter that is not compatible to vPort parameter
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecySaSet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.6 Call cpssDxChMacSecSecySaSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.7 Call cpssDxChMacSecSecySaSet with vPortParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecSecySaSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParamsGet;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U8                                   sci[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS];

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaSet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        saHandle = 1;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        st = cpssDxChMacSecSecySaSet(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     saHandle,
                                     &saParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaSet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.params.egress.includeSci = GT_FALSE;
        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now set this SA and update some partameters */
        saParams.actionType                          = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                            = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.confidentialityOffset = 24;
        saParams.params.egress.confProtect           = GT_TRUE;
        saParams.params.egress.controlledPortEnable  = GT_TRUE;
        saParams.params.egress.includeSci            = GT_FALSE;
        saParams.params.egress.preSecTagAuthLength   = 12;
        saParams.params.egress.preSecTagAuthStart    = 12;
        saParams.params.egress.protectFrames         = GT_TRUE;
        saParams.params.egress.saInUse               = GT_FALSE;
        saParams.params.egress.useEs                 = GT_TRUE;
        saParams.params.egress.useScb                = GT_TRUE;

        /* Set SA parameters */
        st = cpssDxChMacSecSecySaSet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     &saParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSecySaGet and verified values are the same as it was set.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecSecySaGet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     &saParamsGet,
                                     &vPortIndex,
                                     sci);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.actionType                          , saParamsGet.actionType                          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.destPort                            , saParamsGet.destPort                            , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.confidentialityOffset , saParamsGet.params.egress.confidentialityOffset , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.confProtect           , saParamsGet.params.egress.confProtect           , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.controlledPortEnable  , saParamsGet.params.egress.controlledPortEnable  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.includeSci            , saParamsGet.params.egress.includeSci            , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.preSecTagAuthLength   , saParamsGet.params.egress.preSecTagAuthLength   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.preSecTagAuthStart    , saParamsGet.params.egress.preSecTagAuthStart    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.protectFrames         , saParamsGet.params.egress.protectFrames         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.saInUse               , saParamsGet.params.egress.saInUse               , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.useEs                 , saParamsGet.params.egress.useEs                 , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.egress.useScb                , saParamsGet.params.egress.useScb                , dev);

        /*
           1.1.4 Call cpssDxChMacSecSecySaSet with sa parameter that is not compatible to vPort parameter
                 Expected: GT_BAD_PARAM.
        */

        /* Set SA parameter includeSci to true as opposed to vPort parameter that was previously set to not include SCI  */
        saParamsGet.params.egress.includeSci = GT_TRUE;
        st = cpssDxChMacSecSecySaSet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     &saParamsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

         /*
           1.1.5 Call cpssDxChMacSecSecySaSet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaSet(dev,
                                         6,
                                         direction,
                                         saHandle,
                                         &saParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.6 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaSet(dev, unitBmp, direction, saHandle, &saParams), direction);

        /*
           1.1.7 Call api with saParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaSet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    saHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaSet(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     saHandle,
                                     &saParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    saHandle = 1;

    st = cpssDxChMacSecSecySaSet(dev,
                                 BIT_0,
                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                 saHandle,
                                 &saParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_STC       *saParamsPtr,
    OUT  GT_U32                             *vPortIdPtr,
    OUT  GT_U8                              sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaGet and verify the values are the same as they were set                                                                        .
           Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecySaGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecySaGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecSecySaGet with saParams as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call cpssDxChMacSecSecySaGet with vPortIndex as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.8 Call cpssDxChMacSecSecySaGet with invalid sci.
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecySaGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParamsGet;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    GT_U32                                  vPortIndex;
    GT_U32                                  vPortIndexGet;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U8                                   sciGet[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS];

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        saHandle = 1;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        st = cpssDxChMacSecSecySaGet(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     saHandle,
                                     &saParams,
                                     &vPortIndex,
                                     sciGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E;

        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SCI */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            trRecParams.sciArr[i] = i;
        }

        /* Set SA parameters */
        saParams.actionType                          = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        saParams.destPort                            = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.ingress.allowTagged           = GT_TRUE;
        saParams.params.ingress.allowUntagged         = GT_FALSE;
        saParams.params.ingress.an                    = 2;
        saParams.params.ingress.confidentialityOffset = 12;
        saParams.params.ingress.preSecTagAuthLength   = 12;
        saParams.params.ingress.preSecTagAuthStart    = 0;
        saParams.params.ingress.replayProtect         = GT_TRUE;
        saParams.params.ingress.retainIcv             = GT_FALSE;
        saParams.params.ingress.retainSecTag          = GT_FALSE;
        saParams.params.ingress.validateFramesTagged  = CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E;
        saParams.params.ingress.validateUntagged      = GT_FALSE;
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            saParams.params.ingress.sciArr[i] = i;
        }

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecySaGet and verified values are the same as it was set in add SA.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecSecySaGet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     &saParamsGet,
                                     &vPortIndexGet,
                                     sciGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify set and get parameters are the same */

        /* Check SA parameters */
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.actionType                           , saParamsGet.actionType                           , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.destPort                             , saParamsGet.destPort                             , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.allowTagged           , saParamsGet.params.ingress.allowTagged           , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.allowUntagged         , saParamsGet.params.ingress.allowUntagged         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.an                    , saParamsGet.params.ingress.an                    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.confidentialityOffset , saParamsGet.params.ingress.confidentialityOffset , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.preSecTagAuthLength   , saParamsGet.params.ingress.preSecTagAuthLength   , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.preSecTagAuthStart    , saParamsGet.params.ingress.preSecTagAuthStart    , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.replayProtect         , saParamsGet.params.ingress.replayProtect         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.retainIcv             , saParamsGet.params.ingress.retainIcv             , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.retainSecTag          , saParamsGet.params.ingress.retainSecTag          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.validateFramesTagged  , saParamsGet.params.ingress.validateFramesTagged  , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.validateUntagged      , saParamsGet.params.ingress.validateUntagged      , dev);
        /* Check vPort ID */
        UTF_VERIFY_EQUAL1_PARAM_MAC(vPortIndex                                   , vPortIndexGet                                   , dev);
        /* Check SCI */
        for (i=0; i< CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS; i++)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(saParams.params.ingress.sciArr[i] , sciGet[i] , dev);
        }

         /*
           1.1.4 Call cpssDxChMacSecSecySaGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaGet(dev,
                                         6,
                                         direction,
                                         saHandle,
                                         &saParams,
                                         &vPortIndex,
                                         sciGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaGet(dev, unitBmp, direction, saHandle, &saParams, &vPortIndex, sciGet), direction);

        /*
           1.1.6 Call api with saParams as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaGet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     NULL,
                                     &vPortIndex,
                                     sciGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with vPortIndex as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaGet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     &saParams,
                                     NULL,
                                     sciGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.8 Call api with vPortIndex as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaGet(dev,
                                     unitBmp,
                                     direction,
                                     saHandle,
                                     &saParams,
                                     &vPortIndex,
                                     NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    saHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaGet(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     saHandle,
                                     &saParams,
                                     &vPortIndex,
                                     sciGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    saHandle = 1;

    st = cpssDxChMacSecSecySaGet(dev,
                                 BIT_0,
                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                 saHandle,
                                 &saParams,
                                 &vPortIndex,
                                 sciGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaIndexGet
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  GT_U32                             *saIndexPtr,
    OUT  GT_U32                             *scIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaIndexGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaIndexGet with invalid SA SA handle
          Expected: GT_BAD_PARAM

    1.1.2 Call cpssDxChMacSecSecySaIndexGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaIndexGet with saIndex as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.4 Call cpssDxChMacSecSecySaIndexGet with scIndex as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecSecySaIndexGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaIndexGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U32                                  saIndex;
    GT_U32                                  scIndex;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaIndexGet with invalid SA handle
                 Expected: GT_BAD_PARAM.
        */
        saHandle = 0;
        st = cpssDxChMacSecSecySaIndexGet(dev,
                                          saHandle,
                                          &saIndex,
                                          &scIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaIndexGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SCI */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            trRecParams.sciArr[i] = i;
        }

        /* Set SA parameters */
        saParams.actionType                           = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        saParams.destPort                             = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.ingress.allowTagged           = GT_TRUE;
        saParams.params.ingress.allowUntagged         = GT_FALSE;
        saParams.params.ingress.an                    = 2;
        saParams.params.ingress.confidentialityOffset = 12;
        saParams.params.ingress.preSecTagAuthLength   = 12;
        saParams.params.ingress.preSecTagAuthStart    = 0;
        saParams.params.ingress.replayProtect         = GT_TRUE;
        saParams.params.ingress.retainIcv             = GT_FALSE;
        saParams.params.ingress.retainSecTag          = GT_FALSE;
        saParams.params.ingress.validateFramesTagged  = CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E;
        saParams.params.ingress.validateUntagged      = GT_FALSE;
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            saParams.params.ingress.sciArr[i] = i;
        }

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecySaIndexGet with valid parameters.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecSecySaIndexGet(dev,
                                          saHandle,
                                          &saIndex,
                                          &scIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.4 Call api with saIndex as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaIndexGet(dev,
                                          saHandle,
                                          NULL,
                                          &scIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.5 Call api with scIndex as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaIndexGet(dev,
                                          saHandle,
                                          &saIndex,
                                          NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaIndexGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    saHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaIndexGet(dev,
                                          saHandle,
                                          &saIndex,
                                          &scIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaIndexGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    saHandle = 1;

    st = cpssDxChMacSecSecySaIndexGet(dev,
                                      saHandle,
                                      &saIndex,
                                      &scIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaHandleGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   GT_U32                             saIndex,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_HANDLE    *saHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaHandleGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaHandleGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaHandleGet with valid values.
          Expected: GT_OK.

    1.1.3 Verify SA handle output is the same one that was allocated in add SA API                                                                        .
           Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecySaHandleGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecySaHandleGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecSecySaHandleGet with saHandle as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call api with invalid saIndex.
          saIndex [maximum number of SAs +1]                                   .
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecySaHandleGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaHandleGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandleGet;
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U32                                  saIndex;
    GT_U32                                  scIndex;
    GT_U32                                  maxSecySas;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaHandleGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        saHandle = 1;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        st = cpssDxChMacSecSecySaHandleGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           saIndex,
                                           &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaHandleGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SCI */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            trRecParams.sciArr[i] = i;
        }

        /* Set SA parameters */
        saParams.actionType                          = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        saParams.destPort                            = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.ingress.allowTagged           = GT_TRUE;
        saParams.params.ingress.allowUntagged         = GT_FALSE;
        saParams.params.ingress.an                    = 2;
        saParams.params.ingress.confidentialityOffset = 12;
        saParams.params.ingress.preSecTagAuthLength   = 12;
        saParams.params.ingress.preSecTagAuthStart    = 0;
        saParams.params.ingress.replayProtect         = GT_TRUE;
        saParams.params.ingress.retainIcv             = GT_FALSE;
        saParams.params.ingress.retainSecTag          = GT_FALSE;
        saParams.params.ingress.validateFramesTagged  = CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E;
        saParams.params.ingress.validateUntagged      = GT_FALSE;
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            saParams.params.ingress.sciArr[i] = i;
        }

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get SA index associated with this SA handle */
        st = cpssDxChMacSecSecySaIndexGet(dev,
                                          saHandle,
                                          &saIndex,
                                          &scIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecySaHandleGet with valid parameters
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecSecySaHandleGet(dev,
                                           unitBmp,
                                           direction,
                                           saIndex,
                                           &saHandleGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.4 Verify SA handle output is the same SA handle from add SA API
                 Expected: GT_OK.
        */

        if (prvCpssMacSecSecySaHandlesCompare(saHandleGet,saHandle))
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
           1.1.4 Call cpssDxChMacSecSecySaHandleGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaHandleGet(dev,
                                               6,
                                               direction,
                                               saIndex,
                                               &saHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaHandleGet(dev, unitBmp, direction, saIndex, &saHandle), direction);

        /*
           1.1.6 Call api with saHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaHandleGet(dev,
                                           unitBmp,
                                           direction,
                                           saIndex,
                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with invalid saIndex.
           saIndex [maximum number of SAs + 1]                                   .
           Expected: GT_BAD_PARAM.
        */
        /* Get maximum number of SAs allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         NULL,
                                         &maxSecySas,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call API with saIndex set to maximum number of SAs that for index value it is 1 above valid range */
        saIndex = maxSecySas;
        st = cpssDxChMacSecSecySaHandleGet(dev,
                                           unitBmp,
                                           direction,
                                           saIndex,
                                           &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaHandleGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    saHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaHandleGet(dev,
                                           unitBmp,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           saIndex,
                                           &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaHandleGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    saHandle = 1;

    st = cpssDxChMacSecSecySaHandleGet(dev,
                                       unitBmp,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       saIndex,
                                       &saHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaActiveGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  GT_U32                            vPortId,
    IN  GT_U8                             sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS],
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE   activeSaHandleArr[CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaActiveGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaActiveGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaActiveGet with valid values.
          Expected: GT_OK.

    1.1.3 Verify active SA handle is the same one we got when we first added the SA
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecSecySaActiveGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecySaActiveGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecSecySaActiveGet with sa Handle pointer as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call api with invalid vPortIndex.
          vPortIndex [= maximum number of vPorts]                                      .
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecySaActiveGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaActiveGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         activeSaHandle;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandles[CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS];
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U8                                   sci[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS];
    GT_U32                                  maxSecyVports;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaActiveGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        activeSaHandle = 1;
        vPortIndex = 0;
        /* Set SCI buffer */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            sci[i] = i;
        }
        st = cpssDxChMacSecSecySaActiveGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortIndex,
                                           sci,
                                           &activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount    = sizeof(K1);
        trRecParams.seqNumLo        = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SCI */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            trRecParams.sciArr[i] = sci[i];
        }

        /* Set SA parameters */
        saParams.actionType                          = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        saParams.destPort                            = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.ingress.allowTagged           = GT_TRUE;
        saParams.params.ingress.allowUntagged         = GT_FALSE;
        saParams.params.ingress.an                    = 2;
        saParams.params.ingress.confidentialityOffset = 12;
        saParams.params.ingress.preSecTagAuthLength   = 12;
        saParams.params.ingress.preSecTagAuthStart    = 0;
        saParams.params.ingress.replayProtect         = GT_TRUE;
        saParams.params.ingress.retainIcv             = GT_FALSE;
        saParams.params.ingress.retainSecTag          = GT_FALSE;
        saParams.params.ingress.validateFramesTagged  = CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E;
        saParams.params.ingress.validateUntagged      = GT_FALSE;
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            saParams.params.ingress.sciArr[i] = sci[i];
        }

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     unitBmp,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaActiveGet with valid parameters
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecSecySaActiveGet(dev,
                                           unitBmp,
                                           direction,
                                           vPortIndex,
                                           sci,
                                           &saHandles[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Verify active SA handle is the same one we got when we first add SA
                 Expected: GT_OK.
        */
        /* Compare SA handle in entry AN vs SA handle that was first allocated */
        if (prvCpssMacSecSecySaHandlesCompare(saHandles[saParams.params.ingress.an],activeSaHandle))
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
           1.1.4 Call cpssDxChMacSecSecySaActiveGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaActiveGet(dev,
                                               6,
                                               direction,
                                               vPortIndex,
                                               sci,
                                               &activeSaHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaActiveGet(dev, unitBmp, direction, vPortIndex, sci, &activeSaHandle), direction);

        /*
           1.1.6 Call api with activeSaHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaActiveGet(dev,
                                           unitBmp,
                                           direction,
                                           vPortIndex,
                                           sci,
                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with invalid vPortIndex.
           vPortIndex [= maximum number of vPorts]                                      .
           Expected: GT_BAD_PARAM.
        */
        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         &maxSecyVports,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call API with vPortIndex set to maximum number of vPorts */
        vPortIndex = maxSecyVports;
        st = cpssDxChMacSecSecySaActiveGet(dev,
                                           unitBmp,
                                           direction,
                                           vPortIndex,
                                           sci,
                                           &activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaActiveGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    vPortIndex = 0;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaActiveGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortIndex,
                                           sci,
                                           &activeSaHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaActiveGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    vPortIndex = 0;

    st = cpssDxChMacSecSecySaActiveGet(dev,
                                       BIT_0,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       vPortIndex,
                                       sci,
                                       &activeSaHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaNextPnUpdate
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  GT_U32                            nextPnLo,
    IN  GT_U32                            nextPnHi,
    OUT GT_BOOL                           *nextPnWrittenPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaNextPnUpdate)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaNextPnUpdate before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaNextPnUpdate with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaNextPnUpdate with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecySaNextPnUpdate with nextPnWrittenPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecSecySaNextPnUpdate for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaNextPnUpdate with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U32                                  nextPnLo;
    GT_U32                                  nextPnHi;
    GT_BOOL                                 nextPnWritten;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaNextPnUpdate before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        saHandle = 1;
        nextPnLo = 0x9;
        nextPnHi = 0xa;
        st = cpssDxChMacSecSecySaNextPnUpdate(dev,
                                              BIT_0,
                                              saHandle,
                                              nextPnLo,
                                              nextPnHi,
                                              &nextPnWritten);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaNextPnUpdate with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set sequence number in transform record */
        trRecParams.seqTypeExtended = GT_TRUE;
        trRecParams.seqNumLo        = 0x5;
        trRecParams.seqNumHi        = 0x8;

        /* Set key in trnsform record */
        trRecParams.keyByteCount = sizeof(K1);
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call cpssDxChMacSecSecySaNextPnUpdate API to update new nextPN  */
        st = cpssDxChMacSecSecySaNextPnUpdate(dev,
                                              BIT_0,
                                              saHandle,
                                              nextPnLo,
                                              nextPnHi,
                                              &nextPnWritten);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
           1.1.3 Call cpssDxChMacSecSecySaNextPnUpdate with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaNextPnUpdate(dev,
                                                  6,
                                                  saHandle,
                                                  nextPnLo,
                                                  nextPnHi,
                                                  &nextPnWritten);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call api with nextPnWrittenPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaNextPnUpdate(dev,
                                              BIT_0,
                                              saHandle,
                                              nextPnLo,
                                              nextPnHi,
                                              NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaNextPnUpdate for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    saHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaNextPnUpdate(dev,
                                              BIT_0,
                                              saHandle,
                                              nextPnLo,
                                              nextPnHi,
                                              &nextPnWritten);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaNextPnUpdate with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    saHandle = 1;

    st = cpssDxChMacSecSecySaNextPnUpdate(dev,
                                          BIT_0,
                                          saHandle,
                                          nextPnLo,
                                          nextPnHi,
                                          &nextPnWritten);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaNextPnGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    OUT GT_U32                            *nextPnLoPtr,
    OUT GT_U32                            *nextPnHiPtr,
    OUT GT_BOOL                           *extendedPnPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaNextPnGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaNextPnGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecySaNextPnGet with valid values.
          Expected: GT_OK.

    1.1.3 Verify nextPN is as was set in transform record

    1.1.4 Call cpssDxChMacSecSecySaNextPnGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecSecySaNextPnGet with nextPnLoPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSecySaNextPnGet with nextPnHiPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call cpssDxChMacSecSecySaNextPnGet with extendedPnPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.8 Call cpssDxChMacSecSecySaNextPnGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecySaNextPnGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaNextPnGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    GT_U32                                  i;
    GT_U32                                  nextPnLo;
    GT_U32                                  nextPnHi;
    GT_BOOL                                 extendedPn;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaNextPnGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        saHandle = 1;
        direction = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
        st = cpssDxChMacSecSecySaNextPnGet(dev,
                                           BIT_0,
                                           direction,
                                           saHandle,
                                           &nextPnLo,
                                           &nextPnHi,
                                           &extendedPn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecySaNextPnGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add SA */

        /* Set sequence number in transform record */
        trRecParams.seqTypeExtended = GT_TRUE;
        trRecParams.seqNumLo        = 0x5;
        trRecParams.seqNumHi        = 0x8;

        /* Set key in trnsform record */
        trRecParams.keyByteCount = sizeof(K1);
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        saParams.actionType = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call cpssDxChMacSecSecySaNextPnGet API to update new nextPN  */
        st = cpssDxChMacSecSecySaNextPnGet(dev,
                                           BIT_0,
                                           direction,
                                           saHandle,
                                           &nextPnLo,
                                           &nextPnHi,
                                           &extendedPn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
          1.1.3 Verify nextPn is as was set in transform record                                                              .
       */
        UTF_VERIFY_EQUAL1_PARAM_MAC(trRecParams.seqNumLo, nextPnLo, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(trRecParams.seqNumHi, nextPnHi, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(trRecParams.seqTypeExtended, extendedPn, dev);

         /*
           1.1.4 Call cpssDxChMacSecSecySaNextPnGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaNextPnGet(dev,
                                               6,
                                               direction,
                                               saHandle,
                                               &nextPnLo,
                                               &nextPnHi,
                                               &extendedPn);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with nextPnLo as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaNextPnGet(dev,
                                           BIT_0,
                                           direction,
                                           saHandle,
                                           NULL,
                                           &nextPnHi,
                                           &extendedPn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call api with nextPnHi as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaNextPnGet(dev,
                                           BIT_0,
                                           direction,
                                           saHandle,
                                           &nextPnLo,
                                           NULL,
                                           &extendedPn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with extendedPn as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecySaNextPnGet(dev,
                                           BIT_0,
                                           direction,
                                           saHandle,
                                           &nextPnLo,
                                           &nextPnHi,
                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.8 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaNextPnGet(dev, unitBmp, direction, saHandle, &nextPnLo, &nextPnHi, &extendedPn), direction);


        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this SA to return to previous state for proper exit behavior */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaNextPnGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    saHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaNextPnGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                           saHandle,
                                           &nextPnLo,
                                           &nextPnHi,
                                           &extendedPn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaNextPnGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    saHandle = 1;

    st = cpssDxChMacSecSecySaNextPnGet(dev,
                                       BIT_0,
                                       CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                       saHandle,
                                       &nextPnLo,
                                       &nextPnHi,
                                       &extendedPn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleSet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleSet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyRuleSet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleGet and verify the values are the same as they were set previously                                                                        .
           Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyRuleSet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyRuleSet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyRuleSet with ruleParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyRuleSet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleSet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParamsGet;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    GT_BOOL                                 ruleEnable;
    GT_BOOL                                 isAldrin3MDevice; /* is it Aldrin3M PN of Harrier */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleSet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        isAldrin3MDevice = ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devType & 0xFF000000) == 0x90000000) ? GT_TRUE : GT_FALSE;
        vPortHandle = 1;
        ruleHandle = 1;
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        st = cpssDxChMacSecClassifyRuleSet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle,
                                           ruleHandle,
                                           &ruleParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyRuleSet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        /* vPort partameters */
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E ;
        vPortParams.secTagOffset = 12;
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add rule */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Set mask. Exact match on all these fields. */
        ruleParams.mask.portNum = 0x1f;
        ruleParams.mask.numTags = 0x7f;
        /* Set data */
        ruleParams.key.portNum  = isAldrin3MDevice ? 12 : 0;
        ruleParams.key.numTags  = 0x01; /* Bit[0] No VLAN tags */
        /* Set mask. Match on Destination address. */
        ruleParams.dataMask[0] = 0xffffffff;
        ruleParams.dataMask[1] = 0xffff0000;
        /* Set MAC DA */
        ruleParams.data[0] = (0x66<<24) | (0x55<<16) | (0x44<<8) | (0x33);
        ruleParams.data[1] = (0x22<<24) | (0x11<<16);
        /* Set policy */
        ruleParams.policy.controlPacket = GT_FALSE;
        ruleParams.policy.drop          = GT_FALSE;
        ruleParams.policy.rulePriority  = 2;
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Now set this rule and change some partameters.
           Use Port from DP #0 */
        ruleParams.key.portNum  = (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ? 40 :
                                  isAldrin3MDevice ? 16: 2;

        /* Set MAC DA */
        ruleParams.data[0] = (0xff<<24) | (0xee<<16) | (0xdd<<8) | (0xcc);
        ruleParams.data[1] = (0xbb<<24) | (0xaa<<16);

        /* Rule policy */
        ruleParams.policy.rulePriority  = 3;

        /* Set rule parameters */
        st = cpssDxChMacSecClassifyRuleSet(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           ruleHandle,
                                           &ruleParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleGet and verified values are the same as they were set.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           unitBmp,
                                           direction,
                                           ruleHandle,
                                           &vPortHandle,
                                           &ruleParamsGet,
                                           &ruleEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.mask.portNum         , ruleParamsGet.mask.portNum         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.mask.numTags         , ruleParamsGet.mask.numTags         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.key.portNum          , ruleParamsGet.key.portNum          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.key.numTags          , ruleParamsGet.key.numTags          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.dataMask[0]          , ruleParamsGet.dataMask[0]          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.dataMask[1]          , ruleParamsGet.dataMask[1]          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.data[0]              , ruleParamsGet.data[0]              , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.data[1]              , ruleParamsGet.data[1]              , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.policy.controlPacket , ruleParamsGet.policy.controlPacket , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.policy.drop          , ruleParamsGet.policy.drop          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.policy.rulePriority  , ruleParamsGet.policy.rulePriority  , dev);

         /*
           1.1.4 Call cpssDxChMacSecClassifyRuleSet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleSet(dev,
                                               6,
                                               direction,
                                               vPortHandle,
                                               ruleHandle,
                                               &ruleParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleSet(dev, unitBmp, direction, vPortHandle, ruleHandle, &ruleParams), direction);

        /*
           1.1.6 Call api with ruleParamsPtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleSet(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           ruleHandle,
                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* First remove this rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* then remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleSet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    vPortHandle = 1;
    ruleHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleSet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           vPortHandle,
                                           ruleHandle,
                                           &ruleParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleSet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    vPortHandle = 1;
    ruleHandle = 1;

    st = cpssDxChMacSecClassifyRuleSet(dev,
                                       BIT_0,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       vPortHandle,
                                       ruleHandle,
                                       &ruleParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleDataPtr,
    OUT  GT_BOOL                                 *ruleEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyRuleGet with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleGet and verify the values are the same as they were set when rule was added                                                                        .
           Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyRuleGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyRuleGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyRuleGet with vPortHandle as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call cpssDxChMacSecClassifyRuleGet with ruleParams as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.8 Call cpssDxChMacSecClassifyRuleGet with ruleEnable as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1. Call cpssDxChMacSecClassifyRuleGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParamsGet;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    GT_BOOL                                 ruleEnable;
    GT_U32                                  portFullMask;
    GT_BOOL                                 isAldrin3MDevice; /* is it Aldrin3M PN of Harrier */


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portFullMask = (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ? 0x3F : 0x1F;
        isAldrin3MDevice = ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devType & 0xFF000000) == 0x90000000) ? GT_TRUE : GT_FALSE;

        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */

        vPortHandle = 1;
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           ruleHandle,
                                           &vPortHandle,
                                           &ruleParams,
                                           &ruleEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyRuleGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        /* vPort partameters */
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E ;
        vPortParams.secTagOffset = 12;
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add rule */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Set mask. Exact match on all these fields. */
        ruleParams.mask.portNum = portFullMask;
        ruleParams.mask.numTags = 0x7f;
        /* Set data */
        ruleParams.key.portNum  = (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ? 26 :
                                  isAldrin3MDevice ? 12 : 0;
        ruleParams.key.numTags  = 0x01; /* Bit[0] No VLAN tags */
        /* Set mask. Match on Destination address. */
        ruleParams.dataMask[0] = 0xffffffff;
        ruleParams.dataMask[1] = 0xffff0000;
        /* Set MAC DA */
        ruleParams.data[0] = (0x66<<24) | (0x55<<16) | (0x44<<8) | (0x33);
        ruleParams.data[1] = (0x22<<24) | (0x11<<16);
        /* Set policy */
        ruleParams.policy.controlPacket = GT_FALSE;
        ruleParams.policy.drop          = GT_FALSE;
        ruleParams.policy.rulePriority  = 2;
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleGet and verified values are the same as they were set in add rule.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           unitBmp,
                                           direction,
                                           ruleHandle,
                                           &vPortHandle,
                                           &ruleParamsGet,
                                           &ruleEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify set and get parameters are the same */
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.mask.portNum         , ruleParamsGet.mask.portNum         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.mask.numTags         , ruleParamsGet.mask.numTags         , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.key.portNum          , ruleParamsGet.key.portNum          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.key.numTags          , ruleParamsGet.key.numTags          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.dataMask[0]          , ruleParamsGet.dataMask[0]          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.dataMask[1]          , ruleParamsGet.dataMask[1]          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.data[0]              , ruleParamsGet.data[0]              , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.data[1]              , ruleParamsGet.data[1]              , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.policy.controlPacket , ruleParamsGet.policy.controlPacket , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.policy.drop          , ruleParamsGet.policy.drop          , dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(ruleParams.policy.rulePriority  , ruleParamsGet.policy.rulePriority  , dev);

         /*
           1.1.4 Call cpssDxChMacSecClassifyRuleGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleGet(dev,
                                               6,
                                               direction,
                                               ruleHandle,
                                               &vPortHandle,
                                               &ruleParamsGet,
                                               &ruleEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleGet(dev, unitBmp, direction, ruleHandle, &vPortHandle, &ruleParamsGet, &ruleEnable), direction);

        /*
           1.1.6 Call api with vPortHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           unitBmp,
                                           direction,
                                           ruleHandle,
                                           NULL,
                                           &ruleParamsGet,
                                           &ruleEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with ruleParamsGet as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           unitBmp,
                                           direction,
                                           ruleHandle,
                                           &vPortHandle,
                                           NULL,
                                           &ruleEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.8 Call api with ruleEnable as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           unitBmp,
                                           direction,
                                           ruleHandle,
                                           &vPortHandle,
                                           &ruleParamsGet,
                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* First remove this rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* then remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    vPortHandle = 1;
    ruleHandle = 1;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           ruleHandle,
                                           &vPortHandle,
                                           &ruleParamsGet,
                                           &ruleEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    vPortHandle = 1;
    ruleHandle = 1;

    st = cpssDxChMacSecClassifyRuleGet(dev,
                                       BIT_0,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       ruleHandle,
                                       &vPortHandle,
                                       &ruleParamsGet,
                                       &ruleEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  ruleIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleHandleGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleHandleGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyRuleHandleGet with valid values.
          Expected: GT_OK.

    1.1.3 Verify values are the same as they were set in add vPort API.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecClassifyRuleHandleGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.5 Call cpssDxChMacSecClassifyRuleHandleGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.6 Call cpssDxChMacSecClassifyRuleHandleGet with vPortParamsPtr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.7 Call api with invalid ruleId.
          ruleId [maximum number of rules + 1]                                 .
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyRuleHandleGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleHandleGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  ruleId;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleGet;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    GT_U32                                  maxClassifyRules;
    GT_BOOL                                 isAldrin3MDevice; /* is it Aldrin3M PN of Harrier */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleHandleGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        isAldrin3MDevice = ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devType & 0xFF000000) == 0x90000000) ? GT_TRUE : GT_FALSE;

        ruleId = 0;
        st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                                 BIT_0,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 ruleId,
                                                 &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyRuleHandleGet with valid values.
                 Expected: GT_OK.
        */

        /* First add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Now set this vPort and update some partameters */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E ;
        vPortParams.secTagOffset = 12;
        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now add rule */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Set mask. Exact match on all these fields. */
        ruleParams.mask.portNum = 0x3f;
        ruleParams.mask.numTags = 0x7f;
        /* Set data */
        ruleParams.key.portNum  = isAldrin3MDevice ? 14 : 0;
        ruleParams.key.numTags  = 0x01; /* Bit[0] No VLAN tags */
        /* Set mask. Match on Destination address. */
        ruleParams.dataMask[0] = 0xffffffff;
        ruleParams.dataMask[1] = 0xffff0000;
        /* Set MAC DA */
        ruleParams.data[0] = (0x66<<24) | (0x55<<16) | (0x44<<8) | (0x33);
        ruleParams.data[1] = (0x22<<24) | (0x11<<16);
        /* Set policy */
        ruleParams.policy.controlPacket = GT_FALSE;
        ruleParams.policy.drop          = GT_FALSE;
        ruleParams.policy.rulePriority  = 2;
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated rule index */
        st = cpssDxChMacSecClassifyRuleIndexGet(dev,
                                                ruleHandle,
                                                &ruleId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now get the rule handle associated with ruleId */
        st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 ruleId,
                                                 &ruleHandleGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Verify rule handle is the same one as from add rule
                 Expected: GT_OK.
        */

        if (prvCpssMacSecClassifyRuleHandlesCompare(ruleHandleGet, ruleHandle))
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


         /*
           1.1.4 Call cpssDxChMacSecClassifyRuleHandleGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                                     6,
                                                     direction,
                                                     ruleId,
                                                     &ruleHandle);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call api with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleHandleGet(dev, unitBmp, direction, ruleId, &ruleHandle), direction);

        /*
           1.1.6 Call api with ruleHandle as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 ruleId,
                                                 NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.7 Call api with invalid ruleId.
           ruleId [maximum number of rules + 1]                                 .
           Expected: GT_BAD_PARAM.
        */
        /* Get maximum number of rules allowed in the device */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             NULL,
                                             NULL,
                                             &maxClassifyRules);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call API with ruleId set to maximum number of rules that for index value it is 1 above valid range */
        ruleId = maxClassifyRules;
        st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                                 unitBmp,
                                                 direction,
                                                 ruleId,
                                                 &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* First remove this rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Then remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleHandleGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    ruleHandle = 1;
    ruleId = 0;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                                 BIT_0,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 ruleId,
                                                 &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleHandleGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    ruleHandle = 1;
    ruleId = 0;

    st = cpssDxChMacSecClassifyRuleHandleGet(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             ruleId,
                                             &ruleHandle);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleEnableDisable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleDisable,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleEnable,
    IN  GT_BOOL                                 enableAll,
    IN  GT_BOOL                                 disableAll
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleEnableDisable)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleEnableDisable before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
          Call first cpssDxChMacSecClassifyRuleAdd for enable rule
          Call second cpssDxChMacSecClassifyRuleAdd for disable rule
          Call cpssDxChMacSecClassifyRuleEnableDisable to enable first rule
          Call cpssDxChMacSecClassifyRuleEnableDisable to disable second rule
          Call cpssDxChMacSecClassifyRuleEnableDisable to enable and disable both rules
          Call cpssDxChMacSecClassifyRuleEnableDisable to enable all rules
          Call cpssDxChMacSecClassifyRuleEnableDisable to disable all rules
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleEnableDisable with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyRuleEnableDisable with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyRuleEnableDisable for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleEnableDisable with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle1;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle2;
    GT_MACSEC_UNIT_BMP                      unitBmp;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleEnableDisable before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     BIT_0,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     ruleHandle1,
                                                     ruleHandle2,
                                                     GT_FALSE,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyVportAdd to get valid vPort handle
                 Call first cpssDxChMacSecClassifyRuleAdd for enable rule
                 Call second cpssDxChMacSecClassifyRuleAdd for disable rule
                 Call cpssDxChMacSecClassifyRuleEnableDisable to enable first rule
                 Call cpssDxChMacSecClassifyRuleEnableDisable to disable second rule
                 Call cpssDxChMacSecClassifyRuleEnableDisable to enable and disable both rules
                 Call cpssDxChMacSecClassifyRuleEnableDisable to enable all rules
                 Call cpssDxChMacSecClassifyRuleEnableDisable to disable all rules
                 Expected: GT_OK.
        */

        /* Add vPort */
        direction   = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        vPortHandle = 0;
        unitBmp     = BIT_0;

        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;

        /* Add vPort and get valid vPort handle */
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add first rule */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add second rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Enable first rule */
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     unitBmp,
                                                     direction,
                                                     ruleHandle1,
                                                     0,
                                                     GT_FALSE,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Disable second rule */
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     unitBmp,
                                                     direction,
                                                     0,
                                                     ruleHandle2,
                                                     GT_FALSE,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Enable first rule and disable second rule */
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     unitBmp,
                                                     direction,
                                                     ruleHandle1,
                                                     ruleHandle2,
                                                     GT_FALSE,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now enable all rules */
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     unitBmp,
                                                     direction,
                                                     ruleHandle1,
                                                     ruleHandle2,
                                                     GT_TRUE,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now disable all rules */
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     unitBmp,
                                                     direction,
                                                     ruleHandle1,
                                                     ruleHandle2,
                                                     GT_FALSE,
                                                     GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
           1.1.3 Call cpssDxChMacSecClassifyRuleEnableDisable with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                         6,
                                                         direction,
                                                         ruleHandle1,
                                                         ruleHandle2,
                                                         GT_FALSE,
                                                         GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyRuleEnableDisable with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleEnableDisable(dev, unitBmp, direction, ruleHandle1, ruleHandle2, GT_FALSE, GT_FALSE), direction);


        /* Exit properly */

        /* Remove first rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove second rule to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              unitBmp,
                                              direction,
                                              ruleHandle2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleEnableDisable for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                     BIT_0,
                                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                     ruleHandle1,
                                                     ruleHandle2,
                                                     GT_FALSE,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleEnableDisable with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyRuleEnableDisable(dev,
                                                 BIT_0,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 ruleHandle1,
                                                 ruleHandle2,
                                                 GT_FALSE,
                                                 GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyLimitsGet
(
    IN  GT_U8                 devNum,
    IN  GT_MACSEC_UNIT_BMP    unitBmp,
    OUT GT_U32                *maxPortNumPtr,
    OUT GT_U32                *maxVportNumPtr,
    OUT GT_U32                *maxRuleNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyLimitsGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyLimitsGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyLimitsGet with valid values and different combinations.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyLimitsGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    2.1. Call cpssDxChMacSecClassifyLimitsGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyLimitsGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_U32                                  maxPortNum = 0;
    GT_U32                                  maxVportNum = 0;
    GT_U32                                  maxRuleNum = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyLimitsGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             &maxPortNum,
                                             &maxVportNum,
                                             &maxRuleNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyLimitsGet with valid values.
                 Expected: GT_OK.
        */

        /* Get only maxPortNum */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             &maxPortNum,
                                             NULL,
                                             NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get only maxVportNum and maxRuleNum */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             NULL,
                                             &maxVportNum,
                                             &maxRuleNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get all three parameters */
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             &maxPortNum,
                                             &maxVportNum,
                                             &maxRuleNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        cpssOsPrintf("maxPortNum:%d, maxVportNum:%d, maxRuleNum:%d \n",maxPortNum , maxVportNum, maxRuleNum);
         /*
           1.1.3 Call cpssDxChMacSecClassifyLimitsGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyLimitsGet(dev,
                                                 6,
                                                 &maxPortNum,
                                                 &maxVportNum,
                                                 &maxRuleNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyLimitsGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyLimitsGet(dev,
                                             BIT_0,
                                             &maxPortNum,
                                             &maxVportNum,
                                             &maxRuleNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyLimitsGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyLimitsGet(dev,
                                         BIT_0,
                                         &maxPortNum,
                                         &maxVportNum,
                                         &maxRuleNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_MACSEC_UNIT_BMP      unitBmp,
    OUT GT_U32                  *maxPortNumPtr,
    OUT GT_U32                  *maxVportNumPtr,
    OUT GT_U32                  *maxSaNumPtr,
    OUT GT_U32                  *maxScNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyLimitsGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyLimitsGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyLimitsGet with valid values and different combinations.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyLimitsGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    2.1. Call cpssDxChMacSecSecyLimitsGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecyLimitsGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_U32                                  maxPortNum  = 0;
    GT_U32                                  maxVportNum = 0;
    GT_U32                                  maxSaNum    = 0;
    GT_U32                                  maxScNum    = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyLimitsGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         BIT_0,
                                         &maxPortNum,
                                         &maxVportNum,
                                         &maxSaNum,
                                         &maxScNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyLimitsGet with valid values.
                 Expected: GT_OK.
        */

        /* Get only maxPortNum */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         BIT_0,
                                         &maxPortNum,
                                         NULL,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get only maxSaNum and maxScNum */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         BIT_0,
                                         NULL,
                                         NULL,
                                         &maxSaNum,
                                         &maxScNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get all four parameters */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         BIT_0,
                                         &maxPortNum,
                                         &maxVportNum,
                                         &maxSaNum,
                                         &maxScNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        cpssOsPrintf("maxPortNum:%d, maxVportNum:%d, maxSaNum:%d, maxScNum:%d \n",maxPortNum , maxVportNum, maxSaNum, maxScNum);
         /*
           1.1.3 Call cpssDxChMacSecSecyLimitsGet with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyLimitsGet(dev,
                                             6,
                                             &maxPortNum,
                                             &maxVportNum,
                                             &maxSaNum,
                                             &maxScNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyLimitsGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         BIT_0,
                                         &maxPortNum,
                                         &maxVportNum,
                                         &maxSaNum,
                                         &maxScNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyLimitsGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyLimitsGet(dev,
                                     BIT_0,
                                     &maxPortNum,
                                     &maxVportNum,
                                     &maxSaNum,
                                     &maxScNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyGlobalCfgDump)
{
/*
    1.1.1 Call cpssDxChMacSecSecyGlobalCfgDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyGlobalCfgDump with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyGlobalCfgDump with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecyGlobalCfgDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecyGlobalCfgDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecyGlobalCfgDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                        st = GT_OK;
    GT_U8                            dev;
    GT_MACSEC_UNIT_BMP               unitBmp = BIT_0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT   direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyGlobalCfgDump before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecSecyGlobalCfgDump(dev,
                                             unitBmp,
                                             direction);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyGlobalCfgDump with valid values.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecSecyGlobalCfgDump(dev,
                                             unitBmp,
                                             direction);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
           1.1.3 Call cpssDxChMacSecSecyGlobalCfgDump with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyGlobalCfgDump(dev,
                                                 6,
                                                 direction);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecSecyGlobalCfgDump with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyGlobalCfgDump(dev, unitBmp, direction), direction);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyGlobalCfgDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyGlobalCfgDump(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyGlobalCfgDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyGlobalCfgDump(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortSecyDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_BOOL                           allPorts
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortSecyDump)
{
/*
    1.1.1 Call cpssDxChMacSecPortSecyDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortSecyDump with valid values.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecPortSecyDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.2.1. Call cpssDxChMacSecPortSecyDump for each non-active port
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecPortSecyDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortSecyDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_MACSEC_UNIT_BMP                  unitBmp = BIT_0;
    GT_PHYSICAL_PORT_NUM                port = 0;
    GT_BOOL                             allPorts = GT_FALSE;
    CPSS_DXCH_MACSEC_DIRECTION_ENT      direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortSecyDump before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                st = cpssDxChMacSecPortSecyDump(dev,
                                                unitBmp,
                                                direction,
                                                port,
                                                allPorts);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortSecyDump with valid values.
                     Expected: GT_OK.
            */

            /* Get dump of specific port */
            st = cpssDxChMacSecPortSecyDump(dev,
                                            unitBmp,
                                            direction,
                                            port,
                                            allPorts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.4 Call cpssDxChMacSecPortSecyDump with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortSecyDump(dev, unitBmp, direction, port, allPorts), direction);
        }


        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortSecyDump(dev,
                                            unitBmp,
                                            direction,
                                            port,
                                            allPorts);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortSecyDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortSecyDump(dev,
                                        unitBmp,
                                        direction,
                                        port,
                                        allPorts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortSecyDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortSecyDump(dev,
                                    unitBmp,
                                    direction,
                                    port,
                                    allPorts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyVportDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPortId,
    IN   GT_BOOL                          allvPorts,
    IN   GT_BOOL                          includeSa
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyVportDump)
{
/*
    1.1.1 Call cpssDxChMacSecSecyVportDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Add vPort and get vPort ID
          Add SA
          Call cpssDxChMacSecSecyVportDump API with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyVportDump with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecyVportDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecyVportDump with invalid vPortIndex.
          vPortIndex [= maximum number of vPorts]                                                              .
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecyVportDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecyVportDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex = 0;
    GT_MACSEC_UNIT_BMP                      unitBmp = BIT_0;
    GT_U32                                  i;
    GT_BOOL                                 allVports = GT_FALSE;
    GT_BOOL                                 includeSa = GT_FALSE;
    GT_U32                                  maxSecyVports;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyVportDump before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecSecyVportDump(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         vPortIndex,
                                         allVports,
                                         includeSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Add vPort and get vPort ID
                 Add SA
                 Call cpssDxChMacSecSecyVportDump API with valid values
                 Expected: GT_OK.
        */

        /* Prepare parameters */
        saHandle  = 0;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        vPortParams.secTagOffset = 12;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount = sizeof(K1);
        trRecParams.seqNumLo     = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.actionType                  = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                    = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.confProtect   = GT_TRUE;
        saParams.params.egress.protectFrames = GT_TRUE;
        saParams.params.egress.includeSci    = GT_FALSE;

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get information dump related to specific vPort and the associated SA (includeSa:true)  */
        st = cpssDxChMacSecSecyVportDump(dev,
                                         unitBmp,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         vPortIndex,
                                         allVports,
                                         GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecyVportDump with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyVportDump(dev,
                                             6,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             vPortIndex,
                                             allVports,
                                             includeSa);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecSecyVportDump with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyVportDump(dev, unitBmp, direction, vPortIndex, allVports, includeSa), direction);

        /*
           1.1.5 Call cpssDxChMacSecSecyVportDump with invalid vPortIndex.
           vPortIndex [maximum number of vPorts + 1]                                                              .
           Expected: GT_BAD_PARAM.
        */
        /* Get maximum number of vPorts allowed in the device */
        st = cpssDxChMacSecSecyLimitsGet(dev,
                                         unitBmp,
                                         NULL,
                                         &maxSecyVports,
                                         NULL,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call API with vPortIndex set to maximum number of vPorts that is 1 above valid range */
        vPortIndex = maxSecyVports;
        st = cpssDxChMacSecSecyVportDump(dev,
                                         unitBmp,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         vPortIndex,
                                         allVports,
                                         GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);

        /* Exit properly */

        /* Remove these vPorts for proper exit */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now remove all SAs for proper exit */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyVportDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyVportDump(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         0,
                                         allVports,
                                         includeSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyVportDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyVportDump(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     0,
                                     allVports,
                                     includeSa);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecySaDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN   GT_BOOL                           allSas
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaDump)
{
/*
    1.1.1 Call cpssDxChMacSecSecySaDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Add vPort and get vPort ID
          Add SA
          Call cpssDxChMacSecSecySaDump API with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecySaDump with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecySaDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecySaDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecySaDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex = 0;
    GT_MACSEC_UNIT_BMP                      unitBmp = BIT_0;
    GT_U32                                  i;
    GT_BOOL                                 allSas = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecySaDump before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        saHandle = 1;
        st = cpssDxChMacSecSecySaDump(dev,
                                      BIT_0,
                                      CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                      saHandle,
                                      allSas);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Add vPort and get vPort ID
                 Add SA
                 Call cpssDxChMacSecSecySaDump API with valid values
                 Expected: GT_OK.
        */

        /* Prepare parameters */
        saHandle  = 0;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        vPortParams.secTagOffset = 12;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount = sizeof(K1);
        trRecParams.seqNumLo     = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.actionType                  = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                    = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.confProtect   = GT_TRUE;
        saParams.params.egress.protectFrames = GT_TRUE;
        saParams.params.egress.includeSci    = GT_FALSE;

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get information dump related to specific SA */
        st = cpssDxChMacSecSecySaDump(dev,
                                      BIT_0,
                                      CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                      saHandle,
                                      allSas);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecySaDump with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecySaDump(dev,
                                          6,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          saHandle,
                                          allSas);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecSecySaDump with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecySaDump(dev, unitBmp, direction, saHandle, allSas), direction);

        /* Exit properly */

        /* Remove these vPorts for proper exit */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now remove all SAs for proper exit */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecySaDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecySaDump(dev,
                                      BIT_0,
                                      CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                      saHandle,
                                      GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecySaDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecySaDump(dev,
                                  BIT_0,
                                  CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                  saHandle,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyGlobalCfgDump)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyGlobalCfgDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyGlobalCfgDump with valid values.
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyGlobalCfgDump with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyGlobalCfgDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyGlobalCfgDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyGlobalCfgDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                        st = GT_OK;
    GT_U8                            dev;
    GT_MACSEC_UNIT_BMP               unitBmp = BIT_0;
    CPSS_DXCH_MACSEC_DIRECTION_ENT   direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyGlobalCfgDump before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecClassifyGlobalCfgDump(dev,
                                                 unitBmp,
                                                 direction);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyGlobalCfgDump with valid values.
                 Expected: GT_OK.
        */
        st = cpssDxChMacSecClassifyGlobalCfgDump(dev,
                                                 unitBmp,
                                                 direction);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
           1.1.3 Call cpssDxChMacSecClassifyGlobalCfgDump with invalid unitBmp.
                 unitBmp [6]                                                              .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyGlobalCfgDump(dev,
                                                     6,
                                                     direction);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyGlobalCfgDump with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyGlobalCfgDump(dev, unitBmp, direction), direction);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyGlobalCfgDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyGlobalCfgDump(dev,
                                                 BIT_0,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyGlobalCfgDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyGlobalCfgDump(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecPortClassifyDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_BOOL                           allPorts
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecPortClassifyDump)
{
/*
    1.1.1 Call cpssDxChMacSecPortClassifyDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecPortClassifyDump with valid values.
          Expected: GT_OK.

    1.1.4 Call cpssDxChMacSecPortClassifyDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.2.1. Call cpssDxChMacSecPortClassifyDump for each non-active port
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecPortClassifyDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecPortClassifyDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_MACSEC_UNIT_BMP                  unitBmp = BIT_0;
    GT_PHYSICAL_PORT_NUM                port = 0;
    GT_BOOL                             allPorts = GT_FALSE;
    CPSS_DXCH_MACSEC_DIRECTION_ENT      direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    GT_BOOL                                 initDone;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        initDone = GT_FALSE;

        /* 1.1 For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1 Call cpssDxChMacSecPortClassifyDump before cpssDxChMacSecInit (before MACSec initialization).
                    Expected: GT_NOT_INITIALIZED.
            */

            /* Do it one time and then initialize MACSec init for next tests */
            if (!initDone)
            {
                st = cpssDxChMacSecPortClassifyDump(dev,
                                                    unitBmp,
                                                    direction,
                                                    port,
                                                    allPorts);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* Perform MACSec init */
                st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                initDone = GT_TRUE;
            }

            /*
               1.1.2 Call cpssDxChMacSecPortClassifyDump with valid values.
                     Expected: GT_OK.
            */

            /* Get dump of specific port */
            st = cpssDxChMacSecPortClassifyDump(dev,
                                                unitBmp,
                                                direction,
                                                port,
                                                allPorts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
               1.1.4 Call cpssDxChMacSecPortClassifyDump with wrong direction [wrong enum values].
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChMacSecPortClassifyDump(dev, unitBmp, direction, port, allPorts), direction);
        }

        st = prvUtfNextMACSecPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 For all active devices go over all non-active ports */
        while(GT_OK == prvUtfNextMACSecPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChMacSecPortClassifyDump(dev,
                                                unitBmp,
                                                direction,
                                                port,
                                                allPorts);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecPortClassifyDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecPortClassifyDump(dev,
                                            unitBmp,
                                            direction,
                                            port,
                                            allPorts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecPortClassifyDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecPortClassifyDump(dev,
                                        unitBmp,
                                        direction,
                                        port,
                                        allPorts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyVportDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE   vPortHandle,
    IN   GT_BOOL                                  allvPorts,
    IN   GT_BOOL                                  includeRule
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyVportDump)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyVportDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Add vPort and get vPort ID
          Add SA
          Add rule
          Call cpssDxChMacSecClassifyVportDump API with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyVportDump with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyVportDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyVportDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyVportDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex = 0;
    GT_MACSEC_UNIT_BMP                      unitBmp = BIT_0;
    GT_U32                                  i;
    GT_BOOL                                 allVports = GT_FALSE;
    GT_BOOL                                 includeRule = GT_FALSE;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    GT_BOOL                                 isAldrin3MDevice; /* is it Aldrin3M PN of Harrier */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyVportDump before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        vPortHandle = 1;
        isAldrin3MDevice = ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devType & 0xFF000000) == 0x90000000) ? GT_TRUE : GT_FALSE;
        st = cpssDxChMacSecClassifyVportDump(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             vPortHandle,
                                             allVports,
                                             includeRule);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Add vPort and get vPort ID
                 Add SA
                 Add rule
                 Call cpssDxChMacSecClassifyVportDump API with valid values
                 Expected: GT_OK.
        */

        /* Prepare parameters */
        saHandle  = 0;
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        vPortParams.secTagOffset = 12;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now prepare SA API parameters */

        /* Set few of transform record parameters */
        trRecParams.keyByteCount = sizeof(K1);
        trRecParams.seqNumLo     = 0x5;
        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i] = K1[i];
        }

        /* Set SA parameters */
        saParams.actionType                  = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
        saParams.destPort                    = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
        saParams.params.egress.confProtect   = GT_TRUE;
        saParams.params.egress.protectFrames = GT_TRUE;
        saParams.params.egress.includeSci    = GT_FALSE;

        /* Add SA */
        st = cpssDxChMacSecSecySaAdd(dev,
                                     BIT_0,
                                     direction,
                                     vPortIndex,
                                     &saParams,
                                     &trRecParams,
                                     &saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add rule */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Set mask. Exact match on all these fields. */
        ruleParams.mask.portNum = 0x3f;
        ruleParams.mask.numTags = 0x7f;
        /* Set data */
        ruleParams.key.portNum  = isAldrin3MDevice ? 14 : 0;
        ruleParams.key.numTags  = 0x01; /* Bit[0] No VLAN tags */
        /* Set mask. Match on Destination address. */
        ruleParams.dataMask[0] = 0xffffffff;
        ruleParams.dataMask[1] = 0xffff0000;
        /* Set MAC DA */
        ruleParams.data[0] = (0x66<<24) | (0x55<<16) | (0x44<<8) | (0x33);
        ruleParams.data[1] = (0x22<<24) | (0x11<<16);
        /* Set policy */
        ruleParams.policy.controlPacket = GT_FALSE;
        ruleParams.policy.drop          = GT_FALSE;
        ruleParams.policy.rulePriority  = 2;
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get information dump related to specific vPort and the associated rule (includeRule:true)  */
        st = cpssDxChMacSecClassifyVportDump(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             vPortHandle,
                                             allVports,
                                             GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyVportDump with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyVportDump(dev,
                                                 6,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 vPortHandle,
                                                 allVports,
                                                 includeRule);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyVportDump with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyVportDump(dev, unitBmp, direction, vPortHandle, allVports, includeRule), direction);

        /* Exit properly */

        /* Remove rule */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              BIT_0,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove these vPorts for proper exit */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Now remove all SAs for proper exit */
        st = cpssDxChMacSecSecySaRemove(dev,
                                        unitBmp,
                                        direction,
                                        saHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyVportDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyVportDump(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             1,
                                             GT_FALSE,
                                             GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyVportDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyVportDump(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         1,
                                         GT_FALSE,
                                         GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyRuleDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE    ruleHandle,
    IN   GT_BOOL                                  allRules
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyRuleDump)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyRuleDump before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Add vPort and get vPort ID
          Call cpssDxChMacSecClassifyRuleDump API with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyRuleDump with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyRuleDump with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyRuleDump for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyRuleDump with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex = 0;
    GT_MACSEC_UNIT_BMP                      unitBmp = BIT_0;
    GT_BOOL                                 allRules = GT_FALSE;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    GT_BOOL                                 isAldrin3MDevice; /* is it Aldrin3M PN of Harrier */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyRuleDump before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        ruleHandle = 1;
        isAldrin3MDevice = ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devType & 0xFF000000) == 0x90000000) ? GT_TRUE : GT_FALSE;
        st = cpssDxChMacSecClassifyRuleDump(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            ruleHandle,
                                            allRules);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Add vPort and get vPort ID
                 Add rule
                 Call cpssDxChMacSecClassifyRuleDump API with valid values
                 Expected: GT_OK.
        */

        /* Prepare parameters */
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E;
        vPortParams.secTagOffset = 12;
        st = cpssDxChMacSecClassifyVportAdd(dev,
                                            unitBmp,
                                            direction,
                                            &vPortParams,
                                            &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev,
                                                 vPortHandle,
                                                 &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Add rule */
        cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));
        /* Set mask. Exact match on all these fields. */
        ruleParams.mask.portNum = 0x3f;
        ruleParams.mask.numTags = 0x7f;
        /* Set data */
        ruleParams.key.portNum  = isAldrin3MDevice ? 14 : 0;
        ruleParams.key.numTags  = 0x01; /* Bit[0] No VLAN tags */
        /* Set mask. Match on Destination address. */
        ruleParams.dataMask[0] = 0xffffffff;
        ruleParams.dataMask[1] = 0xffff0000;
        /* Set MAC DA */
        ruleParams.data[0] = (0x66<<24) | (0x55<<16) | (0x44<<8) | (0x33);
        ruleParams.data[1] = (0x22<<24) | (0x11<<16);
        /* Set policy */
        ruleParams.policy.controlPacket = GT_FALSE;
        ruleParams.policy.drop          = GT_FALSE;
        ruleParams.policy.rulePriority  = 2;
        /* Add rule */
        st = cpssDxChMacSecClassifyRuleAdd(dev,
                                           unitBmp,
                                           direction,
                                           vPortHandle,
                                           &ruleParams,
                                           &ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get information dump related to specific rule */
        st = cpssDxChMacSecClassifyRuleDump(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            ruleHandle,
                                            allRules);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyRuleDump with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyRuleDump(dev,
                                                6,
                                                CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                ruleHandle,
                                                allRules);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyRuleDump with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyRuleDump(dev, unitBmp, direction, ruleHandle, allRules), direction);

        /* Exit properly */

        /* Remove rule */
        st = cpssDxChMacSecClassifyRuleRemove(dev,
                                              BIT_0,
                                              direction,
                                              ruleHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Remove these vPorts for proper exit */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyRuleDump for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyRuleDump(dev,
                                            BIT_0,
                                            CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                            1,
                                            GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyRuleDump with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyRuleDump(dev,
                                        BIT_0,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        1,
                                        GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecClassifyStatusGet
(
    IN   GT_U8                                       devNum,
    IN   GT_MACSEC_UNIT_BMP                          unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT              direction,
    IN   GT_BOOL                                     getEccStatus,
    IN   GT_BOOL                                     getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC   *unitStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecClassifyStatusGet)
{
/*
    1.1.1 Call cpssDxChMacSecClassifyStatusGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecClassifyStatusGet API with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecClassifyStatusGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecClassifyStatusGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecClassifyStatusGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecClassifyStatusGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT              direction = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
    GT_MACSEC_UNIT_BMP                          unitBmp = BIT_0;
    GT_BOOL                                     getEccStatus = GT_TRUE;
    GT_BOOL                                     getPktProcessDebug = GT_TRUE;
    CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC   unitStatus;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecClassifyStatusGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecClassifyStatusGet(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                             getEccStatus,
                                             getPktProcessDebug,
                                             &unitStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecClassifyStatusGet API with valid values
                 Expected: GT_OK.
        */

        /* Get Classifier packet processing status and ECC counters status */
        st = cpssDxChMacSecClassifyStatusGet(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             getEccStatus,
                                             getPktProcessDebug,
                                             &unitStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecClassifyStatusGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecClassifyStatusGet(dev,
                                                 6,
                                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                 getEccStatus,
                                                 getPktProcessDebug,
                                                 &unitStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecClassifyStatusGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecClassifyStatusGet(dev, unitBmp, direction, getEccStatus, getPktProcessDebug, &unitStatus), direction);


        /*
           1.1.5 Call api with unitStatus as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecClassifyStatusGet(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             getEccStatus,
                                             getPktProcessDebug,
                                             NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecClassifyStatusGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecClassifyStatusGet(dev,
                                             BIT_0,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             GT_TRUE,
                                             GT_TRUE,
                                             &unitStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecClassifyStatusGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecClassifyStatusGet(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         GT_TRUE,
                                         GT_TRUE,
                                         &unitStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecyStatusGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   GT_BOOL                                 getEccStatus,
    IN   GT_BOOL                                 getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC   *unitStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecyStatusGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecyStatusGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecyStatusGet API with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecyStatusGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.
          NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored

    1.1.4 Call cpssDxChMacSecSecyStatusGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    2.1. Call cpssDxChMacSecSecyStatusGet for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1. Call cpssDxChMacSecSecyStatusGet with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
    GT_MACSEC_UNIT_BMP                      unitBmp = BIT_0;
    GT_BOOL                                 getEccStatus = GT_TRUE;
    GT_BOOL                                 getPktProcessDebug = GT_TRUE;
    CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC   unitStatus;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecyStatusGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        st = cpssDxChMacSecSecyStatusGet(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                         getEccStatus,
                                         getPktProcessDebug,
                                         &unitStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecyStatusGet API with valid values
                 Expected: GT_OK.
        */

        /* Get Transformer packet processing status and ECC counters status */
        st = cpssDxChMacSecSecyStatusGet(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         getEccStatus,
                                         getPktProcessDebug,
                                         &unitStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecyStatusGet with invalid unitBmp.
                 unitBmp [6]                                                             .
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored                                                           .
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecyStatusGet(dev,
                                             6,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             getEccStatus,
                                             getPktProcessDebug,
                                             &unitStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecSecyStatusGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecyStatusGet(dev, unitBmp, direction, getEccStatus, getPktProcessDebug, &unitStatus), direction);


        /*
           1.1.5 Call api with unitStatus as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecyStatusGet(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         getEccStatus,
                                         getPktProcessDebug,
                                         NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecyStatusGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecyStatusGet(dev,
                                         BIT_0,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         GT_TRUE,
                                         GT_TRUE,
                                         &unitStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecyStatusGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecyStatusGet(dev,
                                     BIT_0,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     GT_TRUE,
                                     GT_TRUE,
                                     &unitStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSAExpiredSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    OUT  GT_U32                             indexesArr[],
    OUT  GT_U32                             *indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSAExpiredSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecSAExpiredSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSAExpiredSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSAExpiredSummaryGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecSAExpiredSummaryGet with invalid unitBmp.
          unitBmp [6].
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSAExpiredSummaryGet with indexesArr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSAExpiredSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecSAExpiredSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSAExpiredSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  saSummaryIndexesArr[512] = {0};
    GT_U32                                  saSummaryIndexesArrSize = 0;



    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSAExpiredSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;

        st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               saSummaryIndexesArr,
                                               &saSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.2 Call cpssDxChMacSecSAExpiredSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               saSummaryIndexesArr,
                                               &saSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecSAExpiredSummaryGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSAExpiredSummaryGet(dev, unitBmp, direction, saSummaryIndexesArr, &saSummaryIndexesArrSize), direction);

        /*
           1.1.4 Call cpssDxChMacSecSAExpiredSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                                   6,
                                                   direction,
                                                   saSummaryIndexesArr,
                                                   &saSummaryIndexesArrSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecSAExpiredSummaryGet with saSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               NULL,
                                               &saSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecSAExpiredSummaryGet with saSummaryIndexesArrSize as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               saSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSAExpiredSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               saSummaryIndexesArr,
                                               &saSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSAExpiredSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSAExpiredSummaryGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           saSummaryIndexesArr,
                                           &saSummaryIndexesArrSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSAPNThresholdSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    OUT  GT_U32                             indexesArr[],
    OUT  GT_U32                             *indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSAPNThresholdSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecSAPNThresholdSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSAPNThresholdSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSAPNThresholdSummaryGet with invalid unitBmp.
          unitBmp [6].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecSAPNThresholdSummaryGet with indexesArray as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.5 Call cpssDxChMacSecSAPNThresholdSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecSAPNThresholdSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSAPNThresholdSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  saPnThrSummaryIndexesArr[512] = {0};
    GT_U32                                  saPnThrSummaryIndexesArrSize;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSAPNThresholdSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        unitBmp = BIT_0;

        st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                               unitBmp,
                                               saPnThrSummaryIndexesArr,
                                               &saPnThrSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSAPNThresholdSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                               unitBmp,
                                               saPnThrSummaryIndexesArr,
                                               &saPnThrSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSAPNThresholdSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                                   6,
                                                   saPnThrSummaryIndexesArr,
                                                   &saPnThrSummaryIndexesArrSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecSAPNThresholdSummaryGet with saPnThrSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                               unitBmp,
                                               NULL,
                                               &saPnThrSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.5 Call cpssDxChMacSecSAPNThresholdSummaryGet with saPnThrSummaryIndexesArrSize as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                               unitBmp,
                                               saPnThrSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSAPNThresholdSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                               BIT_0,
                                               saPnThrSummaryIndexesArr,
                                               &saPnThrSummaryIndexesArrSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSAPNThresholdSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSAPNThresholdSummaryGet(dev,
                                           BIT_0,
                                           saPnThrSummaryIndexesArr,
                                           &saPnThrSummaryIndexesArrSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSACountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    OUT  GT_U32                             indexesArr[],
    OUT  GT_U32                             *indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSACountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecSACountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSACountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSACountSummaryGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecSACountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSACountSummaryGet with indexesArrindexesArr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSACountSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecSACountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSACountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  saCountSummaryIndexesArr[512] = {0};
    GT_U32                                  saCountSummaryIndexesArrSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSACountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;

        st = cpssDxChMacSecSACountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               saCountSummaryIndexesArr,
                                               &saCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSACountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecSACountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               saCountSummaryIndexesArr,
                                               &saCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSACountSummaryGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSACountSummaryGet(dev, unitBmp, direction, saCountSummaryIndexesArr, &saCountSummaryIndexesArrSizePtr), direction);

        /*
           1.1.4 Call cpssDxChMacSecSACountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSACountSummaryGet(dev,
                                                   6,
                                                   direction,
                                                   saCountSummaryIndexesArr,
                                                   &saCountSummaryIndexesArrSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecSACountSummaryGet with saCountSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSACountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               NULL,
                                               &saCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecSACountSummaryGet with saCountSummaryIndexesArrSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSACountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               saCountSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSACountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSACountSummaryGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               saCountSummaryIndexesArr,
                                               &saCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSACountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSACountSummaryGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           saCountSummaryIndexesArr,
                                           &saCountSummaryIndexesArrSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecSecYCountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    OUT  GT_U32                             indexesArr[],
    OUT  GT_U32                             *indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecSecYCountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecSecYCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecSecYCountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecSecYCountSummaryGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecSecYCountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecSecYCountSummaryGet with indexesArray as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecSecYCountSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecSecYCountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecSecYCountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  secYCountSummaryIndexesArr[256] = {0};
    GT_U32                                  secYCountSummaryIndexesArrSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecSecYCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;

        st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               secYCountSummaryIndexesArr,
                                               &secYCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecSecYCountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               secYCountSummaryIndexesArr,
                                               &secYCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecSecYCountSummaryGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecSecYCountSummaryGet(dev, unitBmp, direction, secYCountSummaryIndexesArr, &secYCountSummaryIndexesArrSizePtr), direction);

        /*
           1.1.4 Call cpssDxChMacSecSecYCountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                                   6,
                                                   direction,
                                                   secYCountSummaryIndexesArr,
                                                   &secYCountSummaryIndexesArrSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecSecYCountSummaryGet with secYCountSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               NULL,
                                               &secYCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecSecYCountSummaryGet with secYCountSummaryIndexesArrSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               secYCountSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecSecYCountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               secYCountSummaryIndexesArr,
                                               &secYCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecSecYCountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecSecYCountSummaryGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           secYCountSummaryIndexesArr,
                                           &secYCountSummaryIndexesArrSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecIfc0CountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    OUT  GT_U32                             indexesArr[]
    OUT  GT_U32                             *indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecIfc0CountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecIfc0CountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecIfc0CountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecIfc0CountSummaryGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecIfc0CountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecIfc0CountSummaryGet with indexesArray as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecIfc0CountSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecIfc0CountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecIfc0CountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  ifc0CountSummaryIndexesArr[256] = {0};
    GT_U32                                  ifc0CountSummaryIndexesArrSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecIfc0CountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;

        st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               ifc0CountSummaryIndexesArr,
                                               &ifc0CountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecIfc0CountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               ifc0CountSummaryIndexesArr,
                                               &ifc0CountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
           1.1.3 Call cpssDxChMacSecIfc0CountSummaryGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecIfc0CountSummaryGet(dev, unitBmp, direction, ifc0CountSummaryIndexesArr, &ifc0CountSummaryIndexesArrSizePtr), direction);

        /*
           1.1.4 Call cpssDxChMacSecIfc0CountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                                   6,
                                                   direction,
                                                   ifc0CountSummaryIndexesArr,
                                                   &ifc0CountSummaryIndexesArrSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecIfc0CountSummaryGet with ifc0CountSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               NULL,
                                               &ifc0CountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecIfc0CountSummaryGet with ifc0CountSummaryIndexesArrSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               ifc0CountSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecIfc0CountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               ifc0CountSummaryIndexesArr,
                                               &ifc0CountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecIfc0CountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecIfc0CountSummaryGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           ifc0CountSummaryIndexesArr,
                                           &ifc0CountSummaryIndexesArrSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecIfc1CountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    OUT  GT_U32                             indexesArr[]
    OUT  GT_U32                             *indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecIfc1CountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecIfc1CountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecIfc1CountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecIfc1CountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecIfc1CountSummaryGet with indexesArray as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.5 Call cpssDxChMacSecIfc1CountSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecIfc1CountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecIfc1CountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  ifc1CountSummaryIndexesArr[256] = {0};
    GT_U32                                  ifc1CountSummaryIndexesArriSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecIfc1CountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        unitBmp = BIT_0;

        st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                               unitBmp,
                                               ifc1CountSummaryIndexesArr,
                                               &ifc1CountSummaryIndexesArriSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecIfc1CountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                               unitBmp,
                                               ifc1CountSummaryIndexesArr,
                                               &ifc1CountSummaryIndexesArriSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecIfc1CountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                                   6,
                                                   ifc1CountSummaryIndexesArr,
                                                   &ifc1CountSummaryIndexesArriSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecIfc1CountSummaryGet with ifc1CountSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                               unitBmp,
                                               NULL,
                                               &ifc1CountSummaryIndexesArriSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.5 Call cpssDxChMacSecIfc1CountSummaryGet with ifc1CountSummaryIndexesArriSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                               unitBmp,
                                               ifc1CountSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecIfc1CountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                               BIT_0,
                                               ifc1CountSummaryIndexesArr,
                                               &ifc1CountSummaryIndexesArriSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecIfc1CountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecIfc1CountSummaryGet(dev,
                                           BIT_0,
                                           ifc1CountSummaryIndexesArr,
                                           &ifc1CountSummaryIndexesArriSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecRxCamCountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    OUT  GT_U32                             indexesArr[],
    OUT  GT_U32                             indexesArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecRxCamCountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecRxCamCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecRxCamCountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecRxCamCountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecRxCamCountSummaryGet with indexesArray as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.5 Call cpssDxChMacSecRxCamCountSummaryGet with indexesArrSizePtr as NULL pointer.
          Expected: GT_BAD_PARAM.

    2.1   Call cpssDxChMacSecRxCamCountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecRxCamCountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  rxCamCountSummaryIndexesArr[256] = {0};
    GT_U32                                  rxCamCountSummaryIndexesArrSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecRxCamCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        unitBmp = BIT_0;

        st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                               unitBmp,
                                               rxCamCountSummaryIndexesArr,
                                               &rxCamCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecRxCamCountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                               unitBmp,
                                               rxCamCountSummaryIndexesArr,
                                               &rxCamCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecRxCamCountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                                   6,
                                                   rxCamCountSummaryIndexesArr,
                                                   &rxCamCountSummaryIndexesArrSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.4 Call cpssDxChMacSecRxCamCountSummaryGet with rxCamCountSummaryIndexesArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                               unitBmp,
                                               NULL,
                                               &rxCamCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.5 Call cpssDxChMacSecRxCamCountSummaryGet with rxCamCountSummaryIndexesArrSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                               unitBmp,
                                               rxCamCountSummaryIndexesArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecRxCamCountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                               BIT_0,
                                               rxCamCountSummaryIndexesArr,
                                               &rxCamCountSummaryIndexesArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecRxCamCountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecRxCamCountSummaryGet(dev,
                                           BIT_0,
                                           rxCamCountSummaryIndexesArr,
                                           &rxCamCountSummaryIndexesArrSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecCfyePortCountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    OUT  GT_PHYSICAL_PORT_NUM               portsArr[],
    OUT  GT_U32                             *portsArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecCfyePortCountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecCfyePortCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecCfyePortCountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecCfyePortCountSummaryGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecCfyePortCountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecCfyePortCountSummaryGet with portsArray as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.6 Call cpssDxChMacSecCfyePortCountSummaryGet with portsArrSizePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1   Call cpssDxChMacSecCfyePortCountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecCfyePortCountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  portsList[64] = {0};
    GT_U32                                  portsArrSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecCfyePortCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;

        st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                                   unitBmp,
                                                   direction,
                                                   portsList,
                                                   &portsArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecCfyePortCountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                                   unitBmp,
                                                   direction,
                                                   portsList,
                                                   &portsArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecCfyePortCountSummaryGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecCfyePortCountSummaryGet(dev, unitBmp, direction, portsList, &portsArrSizePtr), direction);

        /*
           1.1.4 Call cpssDxChMacSecCfyePortCountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                                   6,
                                                   direction,
                                                   portsList,
                                                   &portsArrSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecCfyePortCountSummaryGet with portsList as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                                   unitBmp,
                                                   direction,
                                                   NULL,
                                                   &portsArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecCfyePortCountSummaryGet with portsArrSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                                   unitBmp,
                                                   direction,
                                                   portsList,
                                                   NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecCfyePortCountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                                   BIT_0,
                                                   CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                                   portsList,
                                                   &portsArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecCfyePortCountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecCfyePortCountSummaryGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               portsList,
                                               &portsArrSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChMacSecCfyeTcamCountSummaryGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    OUT  GT_U32                             indexArr[],
    OUT  GT_U32                             *indexArrSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChMacSecCfyeTcamCountSummaryGet)
{
/*
    1.1.1 Call cpssDxChMacSecCfyeTcamCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
          Expected: GT_NOT_INITIALIZED.

    1.1.2 Call cpssDxChMacSecCfyeTcamCountSummaryGet with valid values
          Expected: GT_OK.

    1.1.3 Call cpssDxChMacSecCfyeTcamCountSummaryGet with wrong direction [wrong enum values].
          Expected: GT_BAD_PARAM.

    1.1.4 Call cpssDxChMacSecCfyeTcamCountSummaryGet with invalid unitBmp.
          unitBmp [6]
          Expected: GT_BAD_PARAM.

    1.1.5 Call cpssDxChMacSecCfyeTcamCountSummaryGet with indexArr as NULL pointer.
          Expected: GT_BAD_PTR.

    1.1.5 Call cpssDxChMacSecCfyeTcamCountSummaryGet with indexArrSizePtr as NULL pointer.
          Expected: GT_BAD_PTR.

    2.1   Call cpssDxChMacSecCfyeTcamCountSummaryGet and go over all non active devices
          Expected: GT_NOT_APPLICABLE_DEVICE.

    3.1   Call cpssDxChMacSecCfyeTcamCountSummaryGet with out of bound value for device id
          Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction;
    GT_MACSEC_UNIT_BMP                      unitBmp;
    GT_U32                                  tcamCountSummaryIndexArr[512] = {0};
    GT_U32                                  tcamCountSummaryIndexArrSizePtr;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1.1 Call cpssDxChMacSecCfyeTcamCountSummaryGet before cpssDxChMacSecInit (before MACSec initialization).
                 Expected: GT_NOT_INITIALIZED.
        */
        /* Prepare values */
        direction = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
        unitBmp = BIT_0;

        st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               tcamCountSummaryIndexArr,
                                               &tcamCountSummaryIndexArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

        /* MACSec init */
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.2 Call cpssDxChMacSecCfyeTcamCountSummaryGet with valid values.
                 Expected: GT_OK.
        */

        st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               tcamCountSummaryIndexArr,
                                               &tcamCountSummaryIndexArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.1.3 Call cpssDxChMacSecCfyeTcamCountSummaryGet with wrong direction [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChMacSecCfyeTcamCountSummaryGet(dev, unitBmp, direction, tcamCountSummaryIndexArr, &tcamCountSummaryIndexArrSizePtr), direction);

        /*
           1.1.4 Call cpssDxChMacSecCfyeTcamCountSummaryGet with invalid unitBmp.
                 unitBmp [6].
           Expected: GT_BAD_PARAM.
           NOTE: Skip Phoenix since it is a single DP device and therefore unitBmp parameter is ignored.
        */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                                   6,
                                                   direction,
                                                   tcamCountSummaryIndexArr,
                                                   &tcamCountSummaryIndexArrSizePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
           1.1.5 Call cpssDxChMacSecCfyeTcamCountSummaryGet with tcamCountSummaryIndexArr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               NULL,
                                               &tcamCountSummaryIndexArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.1.6 Call cpssDxChMacSecCfyeTcamCountSummaryGet with tcamCountSummaryIndexArrSizePtr as NULL pointer.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                               unitBmp,
                                               direction,
                                               tcamCountSummaryIndexArr,
                                               NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE             */

    /*
        2.1. Call cpssDxChMacSecCfyeTcamCountSummaryGet for none active devices.
        Expected: GT_NOT_APPLICABLE_DEVICE.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                               BIT_0,
                                               CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                               tcamCountSummaryIndexArr,
                                               &tcamCountSummaryIndexArrSizePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    /*
        3.1. Call cpssDxChMacSecCfyeTcamCountSummaryGet with out of bound device id.
        Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChMacSecCfyeTcamCountSummaryGet(dev,
                                           BIT_0,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           tcamCountSummaryIndexArr,
                                           &tcamCountSummaryIndexArrSizePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChMacSecSecySaNextPnGet_test)
{
    /* cpssDxChMacSecInit
        cpssDxChMacSecClassifyVportAdd
        cpssDxChMacSecClassifyVportIndexGet
        cpssDxChMacSecSecySaAdd
        cpssDxChMacSecSecySaNextPnUpdate
        cpssDxChMacSecSecySaNextPnGet
        cpssDxChMacSecSecySaRemove
        cpssDxChMacSecSecySaAdd
        cpssDxChMacSecSecySaNextPnUpdate
        cpssDxChMacSecSecySaNextPnGet */

    GT_U8                                   dev;
#ifndef ASIC_SIMULATION
    GT_STATUS                               st = GT_OK;
    CPSS_DXCH_MACSEC_DIRECTION_ENT          direction = CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle;
    GT_U32                                  vPortIndex;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         saHandle[2];
    GT_U32                                  nextPnLo;
    GT_U32                                  nextPnHi;
    GT_U32                                  nextPnLoGet;
    GT_U32                                  nextPnHiGet;
    GT_BOOL                                 nextPnWritten;
    GT_BOOL                                 extendedPn;
    GT_U32                                  i;
    GT_MACSEC_UNIT_BMP                      unitBmp = BIT_0;
    GT_U8 sci[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS] = {0x37, 0x78, 0x36, 0x9a, 0xbd, 0xe8, 0xeb, 0xa7};
#endif

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChMacSecInit(dev, CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Prepare parameters */
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Add vPort */
        vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E;
        vPortParams.secTagOffset = 0;

        /* Prepare parameters */
        cpssOsMemSet(&saParams, 0, sizeof(saParams));
        cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));
        cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

        /* Set few of transform record parameters */
        trRecParams.keyByteCount = sizeof(K1);
        trRecParams.seqTypeExtended = GT_TRUE;
        trRecParams.seqMask      = 0x3fffffff;
        trRecParams.seqNumLo     = 0x5;
        trRecParams.seqNumHi     = 0xAABBCCDD;

        /* Set key */
        for (i=0;i<sizeof(K1);i++)
        {
            trRecParams.keyArr[i]  = K1[i];
        }
        trRecParams.ssciArr[0] = trRecParams.ssciArr[1] = trRecParams.ssciArr[2] = trRecParams.ssciArr[3] = 0x11;

        /* Set SA parameters */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
        {
            saParams.params.ingress.sciArr[i] = sci[i];
        }
        saParams.actionType                         = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
        saParams.destPort                           = CPSS_DXCH_MACSEC_SECY_PORT_CONTROLLED_E;
        saParams.params.ingress.saInUse             = GT_TRUE;
        saParams.params.ingress.replayProtect       = GT_TRUE;
        saParams.params.ingress.validateFramesTagged= CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E;
        saParams.params.ingress.allowTagged         = GT_TRUE;
        saParams.params.ingress.preSecTagAuthLength = 12;

        st = cpssDxChMacSecClassifyVportAdd(dev, unitBmp, direction, &vPortParams, &vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get the associated vPort index */
        st = cpssDxChMacSecClassifyVportIndexGet(dev, vPortHandle, &vPortIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        PRV_UTF_LOG1_MAC("vPortIndex = [%d]\n", vPortIndex);

        nextPnLo = 0x7270ea;
        nextPnHi = 0xfefefefe;
        for(i=0; i<2; i++)
        {
            /* Add SA */
            st = cpssDxChMacSecSecySaAdd(dev, unitBmp, direction, vPortIndex, &saParams, &trRecParams, &saHandle[i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            PRV_UTF_LOG1_MAC("******* saHandle = [0x%x] *********\n\n", saHandle[i]);

            st = cpssDxChMacSecSecySaNextPnUpdate(dev, unitBmp, saHandle[i], nextPnLo, nextPnHi, &nextPnWritten);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL0_STRING_MAC(nextPnWritten, GT_TRUE, "nextPnWritten is not expected\n");

            st = cpssDxChMacSecSecySaNextPnGet(dev, unitBmp, direction, saHandle[i], &nextPnLoGet, &nextPnHiGet, &extendedPn);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_STRING_MAC(nextPnLo, nextPnLoGet, "Did not match expected nextPnLo\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(nextPnHi, nextPnHiGet, "Did not match expected nextPnHi\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(extendedPn, GT_TRUE,   "Did not match extendedPn get status\n");

            nextPnLo+=1;
            nextPnHi+=1;
            st = cpssDxChMacSecSecySaNextPnUpdate(dev, unitBmp, saHandle[i], nextPnLo, nextPnHi, &nextPnWritten);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL0_STRING_MAC(nextPnWritten, GT_TRUE, "nextPnWritten is not expected\n");

            st = cpssDxChMacSecSecySaNextPnGet(dev, unitBmp, direction, saHandle[i], &nextPnLoGet, &nextPnHiGet, &extendedPn);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_STRING_MAC(nextPnLo, nextPnLoGet, "Did not match expected nextPnLo\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(nextPnHi, nextPnHiGet, "Did not match expected nextPnHi\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(extendedPn, GT_TRUE,   "Did not match extendedPn get status\n");
        }

        /* Remove this vPort to return to previous state for proper exit behavior */
        st = cpssDxChMacSecClassifyVportRemove(dev,
                                               unitBmp,
                                               direction,
                                               vPortHandle);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for(i=0; i<2; i++)
        {
            /* Remove this SA to return to previous state for proper exit behavior */
            st = cpssDxChMacSecSecySaRemove(dev,
                                            unitBmp,
                                            direction,
                                            saHandle[i]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* Un initialize MACSec units for proper exit */
        st = prvCpssMacSecUnInit(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
#endif
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChCos suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChMacSec)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaChainSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaChainGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortSecyConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortSecyConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyStatisticsTcamHitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyStatisticsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsSaGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsSecyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsIfcGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsRxCamGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsVportClear)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyHdrParserVlanConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyHdrParserVlanConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyStatisticsConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyStatisticsConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyControlPktDetectConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyControlPktDetectConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatisticsConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportHandleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaHandleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaActiveGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleHandleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleEnableDisable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyLimitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyLimitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyGlobalCfgDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortSecyDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyVportDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyGlobalCfgDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecPortClassifyDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyVportDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyRuleDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecClassifyStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecyStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSAExpiredSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSAPNThresholdSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSACountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecYCountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecIfc0CountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecIfc1CountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecRxCamCountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecCfyePortCountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecCfyeTcamCountSummaryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaNextPnUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaNextPnGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChMacSecSecySaNextPnGet_test)
UTF_SUIT_END_TESTS_MAC(cpssDxChMacSec)
