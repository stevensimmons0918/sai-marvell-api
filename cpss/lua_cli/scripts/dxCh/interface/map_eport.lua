--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* map_eport.lua
--*
--* DESCRIPTION:
--*     commands for egress mapping eports to physical ports or trunks
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- ************************************************************************
---
--  map_eport
--        @description  Map ePort to target physical port or trunk, or restore default mapping
--
--        @param params             - params["flagNo"]: no command property
--                   --
--        @return       true on success, otherwise false and error message
--
local function map_eport(params)
    local result, values, ret
    local dstTable
    local physicalInfoPtr = {}
    local all_ports, dev_ports, devnum
    local enable=1

    all_ports = getGlobal("ifRange")--get table of ports

    if true == params.flagNo then
        -- "no" form of the command: remember this fact
        enable=0
    else

        -- Get the parameters in the way CLI has set them up
        dstTable=params[params["etOrPrtChnl"]]

        if "ethernet"==params["etOrPrtChnl"] then

            -- Mapping to physical port: Convert device number to HW device number
            physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
            physicalInfoPtr.devPort = {}
            physicalInfoPtr.devPort.devNum = dstTable.devId
            physicalInfoPtr.devPort.portNum = dstTable.portNum
            result, values =
                device_to_hardware_format_convert(physicalInfoPtr.devPort.devNum)
            if 0 == result then
                physicalInfoPtr.devPort.devNum = values
            else
                print("Failed to convert device number to HW: port: "..physicalInfoPtr.devPort.devNum.."/"..physicalInfoPtr.devPort.portNum)
            end

        elseif "port-channel"==params["etOrPrtChnl"] then

            -- Mapping to trunk: Just copy the deviceless value
            physicalInfoPtr.type = "CPSS_INTERFACE_TRUNK_E"
            physicalInfoPtr.trunkId = dstTable
        end
    end



    for devNum, dev_ports in pairs(all_ports) do

        for k, port in pairs(dev_ports) do

            if enable == 0 then

                -- "no" form of the command: restore default mapping
                -- (Not a trivial default, it involves some calculation)

                local maxPortNum, maxPhysicalPortNum
                if is_sip_6_30(devNum) then
                    maxPhysicalPortNum = 64    --  6 bits
                elseif is_sip_6_10(devNum) then
                    maxPhysicalPortNum = 128    --  7 bits
                elseif is_sip_6(devNum) then
                    maxPhysicalPortNum = 1024   -- 10 bits
                elseif is_sip_5_20(devNum) then
                    maxPhysicalPortNum = 512    -- 9 bits
                else
                    maxPhysicalPortNum = 256    -- 8 bits
                end
                result, maxPortNum = wrLogWrapper("wrlCpssDevicePortNumberGet","(devId)",devNum)
                if 0 ~= result then
                    return false, "Port nax number reading error at device " ..
                                  tostring(devNum)
                end
                physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
                physicalInfoPtr.devPort = {}
                if port < maxPortNum then
                    -- If the ePort number is in the physical port numbers range, then the default HW device number is our HW device number
                    result, values =
                        device_to_hardware_format_convert(devNum)
                    if 0 == result then
                        physicalInfoPtr.devPort.devNum = values
                    else
                        print("Failed to convert device number to HW: port: "..physicalInfoPtr.devPort.devNum.."/"..physicalInfoPtr.devPort.portNum)
                        physicalInfoPtr.devPort.devNum = 0
                    end
                else
                    -- If the ePort number is out of the physical port numbers range, then the default HW device number is 0
                    physicalInfoPtr.devPort.devNum = 0
                end
                -- Default physical port number is the ePort number truncated to a device dependant number of bits
                physicalInfoPtr.devPort.portNum = port % maxPhysicalPortNum
            end

            -- Call API to set the mapping
            ret = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_PORT_NUM", "portNum", port },
                { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
            })
            if 0~=ret then
                if enable == 0 then
                    print("Failed to restore map eport: ".. devNum.."/"..port.." to default: "..physicalInfoPtr.devPort.devNum.."/"..physicalInfoPtr.devPort.portNum)
                else
                    if physicalInfoPtr.type == "CPSS_INTERFACE_PORT_E" then
                        print("Failed to map eport: ".. devNum.."/"..port.." to port: "..dstTable.devId.."/"..dstTable.portNum)
                    else
                        print("Failed to map eport: ".. devNum.."/"..port.." to trunk: "..dstTable)
                    end
                end
            end
        end
    end

end

--------------------------------------------------------------------------------
-- command registration: Set / Restore_default ePort to physical port or trunk target mapping
--------------------------------------------------------------------------------
CLI_addCommand("interface_eport", "map", {
    func = map_eport,
    help = "Map ePort to physical port or trunk",
   params = {
        { type = "named",
            "#etOrPrtChnl",
          mandatory = { "etOrPrtChnl" }
        }
    }
})

CLI_addCommand("interface_eport", "no map", {
    func = function(params)
        params.flagNo=true
        return map_eport(params)
    end,
    help = "Disable Map ePort to physical port or trunk - Restore default map"
})
--print("\nePort map Commands added!\n\n")

