--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* acl_redirect.lua
--*
--* DESCRIPTION:
--*       The test for ACL Redirect feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
-- the PHa supported on sip6 devices
SUPPORTED_FEATURE_DECLARE(devNum, "PHA_IOAM")

printLog("the test currently only 'config' and 'de-config' entries in the 'Pha thread-id table' (no traffic)")
--------------------------------------------------------------------------------
--load configuration 
executeLocalConfig("dxCh/examples/configurations/pha_basic.txt")
--load de-configuration 
executeLocalConfig("dxCh/examples/configurations/pha_basic_deconfig.txt")

