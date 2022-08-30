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
--includes

--constants

local genericCpuIndex

--called from other test
function genericCpuIndexSet(cpuPortIndex)
    genericCpuIndex = cpuPortIndex
end

-- support call from the config file : ${@@physicalCpuSdmaPortGet(3)}
-- return physical port number for this SDMA CPU port
function physicalCpuSdmaPortGet(cpuPortIndex)
    local devNum  = devEnv.dev

    if(not cpuPortIndex) then
        -- indication for 'generic' index that the test need
        cpuPortIndex = genericCpuIndex 
        if(not cpuPortIndex) then
            return 0xFFFFFFFF
        end
    end
    
    if(cpuPortIndex < 1) then
        return 0xFFFFFFFF
    end

    return luaNetIfSdmaQueueToPhysicalPortGet(devNum,(cpuPortIndex-1)*8)
end

-- call to cpssDxChNetIfSdmaQueueToPhysicalPortGet to get the physical number of the CPU
function luaNetIfSdmaQueueToPhysicalPortGet(devNum,queueIdx)
    local command_data = Command_Data()
    local apiName = "cpssDxChNetIfSdmaQueueToPhysicalPortGet"

    local isError , result, values =     
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum},
        { "IN",     "GT_U8",    "queueIdx",     queueIdx},
        { "OUT",    "GT_PHYSICAL_PORT_NUM",    "OUT_portNum"}
    })

    if isError then
        return devEnv.portCPU -- default CPU port
    end

    return values["OUT_portNum"]
end

