--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_probe_packet_drop_code.lua
--*
--* DESCRIPTION:
--*       Probe Packet Drop Code show commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function showProbePacketDropCode(params)
    local result, value, devNum, devices,d
    local command_data = Command_Data()

    if not is_sip_6(devNum) then
        return
    end
    devNum = params["devID"]

    header = "\n" .. "devID        Direction       DropCode    \n" ..
             "---------------------------------------------------------------"
    footer = "\n"

    command_data:setEmergencyPrintingHeaderAndFooter(header, footer)

    command_data:enablePausedPrinting()

    command_data:clearResultArray()

    if (devNum == "all") then
        devices = wrlDevList()
    else
        devices = { devNum }
    end

    for d = 1, #devices do
        result, value = myGenWrapper("cpssDxChCfgProbePacketDropCodeGet",{
            { "IN",  "GT_U8",                "devNum",   devices[d]                 },
            { "IN",  "CPSS_DIRECTION_ENT",   "portType", "CPSS_DIRECTION_INGRESS_E" },
            { "OUT", "CPSS_NET_RX_CPU_CODE_ENT", "dropCodePtr"   },
        })
        if 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error calling cpssDxChCfgProbePacketDropCodeGet "..returnCodes[result])
        end

        command_data:addToResultArray(
                string.format(" %-7d| %-18s| %-16s",
                devices[d],
                "Ingress",
                value.dropCodePtr))

        result, value = myGenWrapper("cpssDxChCfgProbePacketDropCodeGet",{
            { "IN",  "GT_U8",                "devNum",   devices[d]                },
            { "IN",  "CPSS_DIRECTION_ENT",   "portType", "CPSS_DIRECTION_EGRESS_E" },
            { "OUT", "CPSS_NET_RX_CPU_CODE_ENT", "dropCodePtr"   },
        })

        if 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error calling cpssDxChCfgProbePacketDropCodeGet "..returnCodes[result])
        end

        command_data:addToResultArray(
                string.format(" %-7d| %-18s| %-16s",
                devices[d],
                "Egress",
                value.dropCodePtr))
    end
        command_data:analyzeCommandExecution()
        command_data:printAndFreeResultArray()

        return command_data:getCommandExecutionResults()

end

--------------------------------------------------------------------------------
-- command registration: show probe packet drop-code
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show probe-packet drop-code", {
    func   = showProbePacketDropCode,
    help   = "Show Last dropped probe packet drop-code in ingress & egress engine",
    params = {{type = "named",
                {format="device %devID_all",  name="devID", help = "Device number" },
                mandatory = {"devID"}
             }}
})
