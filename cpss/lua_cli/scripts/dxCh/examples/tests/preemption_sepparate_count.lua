--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* preemption.lua
--*
--* DESCRIPTION:PMAC and EMAC separate counting
--*       Test set ports that can work in preemptive mode to preemptive mode.
--*       Then sanity test that transmit packet from port in preemptive mode is executed.
--*       purpose of example:
--*       1. Check that HW can be configured to preemptive mode
--*       2. Check that control path navigate packets in preemptive channel to correct port.
--*
--*    
--*    
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

SUPPORTED_FEATURE_DECLARE(devNum, "PREEMPTION_MAC_802_3BR")

require("dxCh/examples/common/preemption_utils")
--[[
    THE TEST START
]]--
-- disable automatic counter check.We will check them manually.
luaTgfTransmitEngineNeedToCheckCounters(false)
-- run the test with PMAC and EMAC separate counting
preemptionTest(true)

-- restore configurations
luaTgfTransmitEngineNeedToCheckCounters(true)



