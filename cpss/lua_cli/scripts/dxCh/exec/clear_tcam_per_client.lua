  --********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* clear_tcam_per_client.lua
--*
--* DESCRIPTION:
--*       Remove all TCAM entries for selected client
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--------------------------------------------
-- type registration: tcam_clients
--------------------------------------------
CLI_type_dict["tcam_clients"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "TCAM clients",
    enum = {
        ["ipcl0"] = { value="CPSS_DXCH_TCAM_IPCL_0_E", help = "TCAM client ingress policy 0 (first lookup)" },
        ["ipcl1"] = { value="CPSS_DXCH_TCAM_IPCL_1_E", help = "TCAM client ingress policy 1 (second lookup)" },
        ["ipcl2"] = { value="CPSS_DXCH_TCAM_IPCL_2_E", help = "TCAM client ingress policy 2 (third lookup)" },
        ["epcl"] = { value="CPSS_DXCH_TCAM_EPCL_E", help="TCAM client egress policy" },
        ["tti"] = { value="CPSS_DXCH_TCAM_TTI_E", help="TCAM client tunnel termination" },
        ["all"] = { value="ALL", help="clear the entire TCAM" } -- the value is not part of the CPSS enum!
    }
}


-- ************************************************************************
---
--  clear_mac_address_table_dynamic_func
--        @description  removes dynamic entries from the forwarding
--                      database
--
--        @param params         - params["devID"]: device number
--                                params["tcam_client"]: TCAM client
--
--        @return        true on success, otherwise false and error message
--
local function clear_tcam_per_client_func(params)
    -- Common variables declaration
    local result, values
    local devNum, client
    local err = 0
    local clearAll

    dev = params["devID"]
    client = params["tcam_client"]

    if is_device_eArch_enbled(dev) then
        if client == "ALL" then
            clearAll = GT_TRUE
            client   = "CPSS_DXCH_TCAM_IPCL_0_E" -- irrelevant since it iterates over all TCAM
        else
            clearAll = GT_FALSE
        end

        result, values = myGenWrapper(
            "prvCpssDxChTcamInvalidateRulesPerClient", {
                { "IN", "GT_U8"  , "dev", dev},                  -- devNum
                { "IN", "CPSS_DXCH_TCAM_CLIENT_ENT" , "tcamClient", client}, -- tcamClient
                { "IN", "GT_BOOL" , "clearAll", clearAll}, -- tcamClient
            }
            )
        if (result ~= 0) then
            err = returnCodes[result]
            print("prvCpssDxChTcamInvalidateRulesPerClient returned "..err)
        end
    else
        print("not supported device")
    end
end 


--------------------------------------------------------------------------------
-- command registration: clear mac address-table dynamic
--------------------------------------------------------------------------------
-- *exec*
--CLI_addHelp("exec", "clear tcam", "Clears the client's rules from TCAM")
CLI_addCommand("exec", "clear tcam", {
    func=clear_tcam_per_client_func,
    help = "Clears the client's rules from TCAM",
    params={
        { type="named",
        { format="device %devID",name="devID", help="The device number" } , mandatory = { "devID" },
        { format="client %tcam_clients", name="tcam_client", help = "TCAM client" }
        }
    }
})
