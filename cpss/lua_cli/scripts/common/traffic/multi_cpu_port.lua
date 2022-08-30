--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* multi_cpu_port.lua
--*
--* DESCRIPTION:
--*       function for multi cpu SDMA ports.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
cmdLuaCLI_registerCfunction("prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet")

--includes

--constants

local curr_cpuTxQueue = 7--[[default]]
-- wrapper to call 'prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet'
-- also save the value that was used !
function luaTgfTrafficGeneratorDefaultTxSdmaQueueSet(cpuTxQueue)
    curr_cpuTxQueue = cpuTxQueue
    return prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet(cpuTxQueue)
end

-- function to get the curr_cpuTxQueue
function luaTgfTrafficGeneratorDefaultTxSdmaQueueGet()
    return curr_cpuTxQueue
end
