--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pcl.lua
--*
--* DESCRIPTION:
--*       PCL UDB initialization, types and validators
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

-- ************************************************************************
--  check_param_addressMask
--
--  @description  check if the mask is correct according to the data type
--
--  @param param - The mask to check (can either be a full address mask or of the form /num)
--  @param data - The type of address (mac,ipv6,ipv4)
--
--  @return    boolean check status
--  @return    true:parsed address or false:error_string if failed
--
-- ************************************************************************


function CLI_check_param_macMask(param, data)
    local s,e,ret,maskLength,i

    ret={}
    s,e,ret[1],ret[2]= string.find(param,"(/)(%d?%d?%d)")

    if (s~=nil) and (e==string.len(param)) then  --a mask of the form "/num", returns a number
        maskLength=tonumber(ret[2])

        if maskLength<=48 and maskLength>=0 then
            param=""
            for i=1,6 do
                if (maskLength-i*8>=0) then
                    param=param.."ff"
                elseif (maskLength-i*8<=-8) then
                    param=param.."00"
                else
                    param=param..string.format("%x",tonumber( string.rep("1",maskLength%8)..string.rep("0",8-maskLength%8),2))
                end
                if  (i~=6) then param=param..":" end
            end
        else
            return false,"max length of mask is 48"
        end
    end

    return   CLI_type_dict["mac-address"]["checker"](param,data)
end

local function CLI_check_param_ipv4Mask(param, data)
    local s,e,ret,maskLength,i

    ret={}
    s,e,ret[1],ret[2]= string.find(param,"(/)(%d?%d?%d)")

    if (s~=nil) and (e==string.len(param)) then  --a mask of the form "/num", returns a number
        maskLength=tonumber(ret[2])

        if maskLength<=32 and maskLength>=0 then
            param=""
            for i=1,4 do
                if (maskLength-i*8>=0) then
                    param=param.."255"
                elseif (maskLength-i*8<=-8) then
                    param=param.."0"
                else
                    param=param..tonumber( string.rep("1",maskLength%8)..string.rep("0",8-maskLength%8),2)
                end
                if (i~=4) then param=param.."." end
            end
        else
            return false,"max length of mask is 32"
        end
    end

    return   CLI_type_dict["ipv4"]["checker"](param,data)

end

local function CLI_check_param_ipv6Mask(param, data)
    local s,e,ret,maskLength,i

    ret={}
    s,e,ret[1],ret[2]= string.find(param,"(/)(%d?%d?%d)")

    if (s~=nil) and (e==string.len(param)) then  --a mask of the form "/num", returns a number
        maskLength=tonumber(ret[2])

        if maskLength<=128 and maskLength>=0 then
            param=""
            for i=1,8 do
                if (maskLength-i*16>=0) then
                    param=param.."ffff"
                elseif (maskLength-i*16<=-16) then
                    param=param.."0000"
                else
                    param=param..string.format("%x",tonumber( string.rep("1",maskLength%16)..string.rep("0",16-maskLength%16),2))
                end
                if (i~=8) then param=param..":" end
            end
        else
            return false,"max length of mask is 128"
        end
    end


    return   CLI_type_dict["ipv6"]["checker"](param,data)
end


function CLI_check_param_ruleid(param, data)

    -- global variable calculated at Init System
    local numEntries = NumberOfEntriesIn_PCL_TCAM_table;
    local parmVal = tonumber(param);

    if (parmVal==nil) then return false,"Rule ID must be a number" end

    if(is_sip_5()) then
        -- assuming EXT rules : are '60B' rules
        -- to fit next check
        numEntries = numEntries * 2
    end

    if ((numEntries  / 2) <= parmVal) or (parmVal < 0) then
        return false,"Rule id is out of range, the range is: 0-"..((numEntries/2) -1);
    end

    return true, parmVal;
end

function CLI_check_param_ruleid_or_all(param,data)
    if param == "all" then
        return true, param
    end
    return CLI_check_param_ruleid(param,name)
end

function CLI_complete_param_ruleid_or_all(param, data, desc)
    local compl, help = CLI_complete_param_number(param,name, desc)
    if prefix_match(param, "all") then
        table.insert(compl,"all")
        help[#compl] = "Apply to all rule-ids"
    end
    return compl, help
end

function CLI_check_param_policerid(param, data)
    -- global variable initialized at Init System
    local numEntries = NumberOfEntriesIn_POLICER_METERS_table;
    local parmVal = tonumber(param);

    if (parmVal==nil) then return false,"Policer ID must be a number" end

    if (numEntries<=parmVal) or (parmVal<0) then
        return false,"Policer id is out of range, the range is: 0-".. (parmVal -1);
    end
    return true,parmVal;
end

CLI_type_dict["pcl_action_pkt_cmd"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Action performed\n",
    enum = {
        ["permit"] = { value="permit", help="Permit packet" },
        ["deny"] = { value="deny", help="Drop packet (Hard drop)" },
        ["trap-to-cpu"] = { value="trap", help="Trap to CPU" },
        ["mirror-to-cpu"] = { value="mirror-to-cpu", help="Mirror to CPU" },
        ["deny-soft"] = { value="deny-soft", help="Drop packet (Soft Drop)" }
   }
}



CLI_type_dict["pclID"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1023,
    complete = CLI_complete_param_number,
    help="PCL ID number (0-1023)"
}

CLI_type_dict["lookupNum"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1,
    complete = CLI_complete_param_number,
    help="Lookup number"
}

CLI_type_dict["pcl_direction"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "PCL Direction\n",
    enum = {
        ["ingress"] = { value="CPSS_PCL_DIRECTION_INGRESS_E"},
        ["egress"]  =  { value="CPSS_PCL_DIRECTION_EGRESS_E"}
   }
}

CLI_type_dict["pcl_lookup"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "PCL Lookup\n",
    enum = {
        ["lookup0"]    = { value="CPSS_PCL_LOOKUP_0_0_E"},
        ["lookup1"]    = { value="CPSS_PCL_LOOKUP_0_1_E"},
        ["lookup2"]    = { value="CPSS_PCL_LOOKUP_1_E"},
        ["lookup_dup"] = { value="CPSS_PCL_LOOKUP_0_E", help="lookups 0_0 and 0_1"}
   }
}

CLI_type_dict["ruleIDorAll"] = {
    checker = CLI_check_param_ruleid_or_all,
    min=0,
    max=1023,
    complete = CLI_complete_param_ruleid_or_all,
    help="Rule id number"
}

CLI_type_dict["ruleID"] = { checker = CLI_check_param_ruleid, help = "Rule id number\n"}
CLI_type_dict["policerID"] = { checker = CLI_check_param_policerid, help = "Policer id number\n"}

CLI_type_dict["ipv4Mask"] = { checker = CLI_check_param_ipv4Mask, help = "IPv4 address mask\n"}
CLI_type_dict["ipv6Mask"] = { checker = CLI_check_param_ipv6Mask, help = "IPv6 address mask\n"}
CLI_type_dict["macMask"] = { checker = CLI_check_param_macMask, help = "Mac address mask, for example : FF:FF:FF:FF:FF:FF or /48\n"}

CLI_type_dict["cheetah1_pcl_match_counter"] = {
    checker = CLI_check_param_number,
    min=1,
    max=32,
    complete = CLI_complete_param_number,
    help = "Cheetah1 ACL Rule Match Counter"
}

CLI_type_dict["prot"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "IP Protocol",
    enum = {
       ["icmp"]           = { value = 0x1   , help = "Internet Control Message Protocol(1)" },
       ["igmp"]           = { value = 0x2   , help = "Internet Group Management Protocol(2)" },
       ["ip-in-ip"]       = { value = 0x4   , help = "IP in IP encapsulation(4)" },
       ["tcp"]            = { value = 0x6   , help = "Transmission Control Protocol(6)" },
       ["udp"]            = { value = 0x11  , help = "User Datagram Protocol(17)" },
       ["rdp"]            = { value = 0x1B  , help = "Reliable Data Protocol(27)" },
       ["ipv6"]           = { value = 0x29  , help = "IPv6 Encapsulation(41)" },
       ["sdrp"]           = { value = 0x2A  , help = "Source Demand Routing Protocol(42)" },
       ["ipv6-route"]     = { value = 0x2B  , help = "Routing Header for IPv6(43)" },
       ["ipv6-frag"]      = { value = 0x2C  , help = "Fragment Header for IPv6(44)" },
       ["rsvp"]           = { value = 0x2E  , help = "Resource Reservation Protocol(46)" },
       ["gre"]            = { value = 0x2F  , help = "Generic Routing Encapsulation(47)" },
       ["ipv6-icmp"]      = { value = 0x3A  , help = "ICMP for IPv6(58)" },
       ["ipv6-nonxt"]     = { value = 0x3B  , help = "No Next Header for IPv6(59)" },
       ["ipv6-opts"]      = { value = 0x3C  , help = "Destination Options for IPv6(60)" },
       ["ospf"]           = { value = 0x59  , help = "Open Shortest Path First(89)" },
       ["etherip"]        = { value = 0x61  , help = "Ethernet-within-IP Encapsulation(97)" },
       ["vrrp"]           = { value = 0x70  , help = "Virtual Router Redundancy Protocol(112)" },
       ["isis-over-ipv4"] = { value = 0x7C  , help = "Intermediate System to Intermediate System Protocol over IPv4(124)" },
    }
}

