--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_prefixes.lua
--*
--* DESCRIPTION:
--*       showing of the global status of IP routing and the configured IP Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show ip prefixes_func
--        @description  shows number of all 
--                      configured IP prefixes
--
--        @param params      
--                                   - params["vrfId"]: virtual router id
--
--        @return       true on success, otherwise false and error message
-- 

function show_ip_prefixes(params)
    -- Common variables declaration
	local result, values
    local vrfId, lpmDBId
	lpmDBId                           = 0
    vrfId                             = params["vrf-id"]
    result, values=myGenWrapper("cpssDxChIpLpmPrefixesNumberGet",{	
                                {"IN","GT_U32","lpmDBId",lpmDBId},
                                {"IN","GT_U32","vrId",vrfId},
                                {"OUT","GT_U32","ipv4UcPrefixNumber"},
                                {"OUT","GT_U32","ipv4McPrefixNumber"},
                                {"OUT","GT_U32","ipv6UcPrefixNumber"},
                                {"OUT","GT_U32","ipv6McPrefixNumber"}
                                })
                if        0 == result then
					print() --seperation line
					print(string.format("all existing prefixes for given virtual router %d:", vrfId)) 
					print() --seperation line
					print("number of ipv4 unicast prefixes:   "..values["ipv4UcPrefixNumber"])
					print("number of ipv6 unicast prefixes:   "..values["ipv6UcPrefixNumber"])
					print("number of ipv4 multicast prefixes: "..values["ipv4McPrefixNumber"])
					print("number of ipv6 multicast prefixes: "..values["ipv6McPrefixNumber"])

				elseif 0x12 == result then
					print(string.format("vrfId %d is not defined", vrfId))
				elseif 0xb == result then
					print(string.format("lpmDBId %d is not found", lpmDBId))
				elseif 0x1e == result then
					print(string.format("show ip prefixes: not spported for current device"))
				elseif    0 ~= result then
					print(string.format("there is an error on getting prefixes:%d", result))
                end
end


--------------------------------------------------------------------------------
-- command registration: show ip route 
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show ip prefixes", {
    func   = show_ip_prefixes,
    help   = "Show number of all added prefixes",
    params = {
        -- parameter vrf-id 
        { type="named",
            {format="vrf-id %vrf_id",
                help = "a Virtual Router Id"},
        }
    }
})  

