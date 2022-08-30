--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* falcon_50G_port_create_port_delete_under_traffic.lua
--*
--* DESCRIPTION:
--*       The test port create , port delete under traffic :
--*        tested speed : 50G
--*
--*     the test generate WF traffic using 'traffic-generator emulate' command on 2 ports.
--*     the test iterate on 8 other ports with operations of 'speed 50G' and 'no speed' ,
--*     that is done under traffic
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--
--
local devNum  = devEnv.dev

-- skip test for unsupported devices
SUPPORTED_FEATURE_DECLARE(devNum, "WIRESPEED_EMULATION")
-- limited to Falcon (using 50G speeds)
SUPPORTED_SIP_DECLARE(devNum,"SIP_6")

local devFamily = wrlCpssDeviceFamilyGet(devNum)


-- 8 ports of 50G / 1 port of 400G
local generate_traffic_GOP_400G
local first_port_in_generate_traffic_GOP_400G
local do_show_mib_on_part1_ports
local other_GOPs_400G
local first_ports_in_other_GOPs_400G
local traffic_port_1
local traffic_port_2
local traffic_port_3
local traffic_port_4
local do_show_mib_on_all_ports
local tested_8_50G_ports
local all_ports

local _400G_speed_on_port = "400000 mode KR8" --400G
local _50G_speed_on_port  = "50000 mode KR"--50G

local other_GOPs_200G
local first_ports_in_other_GOPs_200G

if(is_sip_6_30(devNum))then
    tested_8_50G_ports = "0,2,4,6,8,10,12,14"
    generate_traffic_GOP_400G = "16,18,20,22,24,26,28,30"
    first_port_in_generate_traffic_GOP_400G = "16"
    do_show_mib_on_part1_ports = tested_8_50G_ports .. "," .. first_port_in_generate_traffic_GOP_400G
    other_GOPs_400G = nil
    other_GOPs_200G = "32,34,36,38"
    first_ports_in_other_GOPs_400G = nil
    first_ports_in_other_GOPs_200G = "32"
    traffic_port_1 = "32"
    traffic_port_2 = traffic_port_1
    traffic_port_3 = traffic_port_1
    traffic_port_4 = traffic_port_1
    do_show_mib_on_all_ports = do_show_mib_on_part1_ports .. "," .. first_ports_in_other_GOPs_200G
    all_ports = tested_8_50G_ports .. "," .. generate_traffic_GOP_400G .. "," .. other_GOPs_200G
elseif(devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E") then
    tested_8_50G_ports = "0,1,2,3,4,5,6,7"
    generate_traffic_GOP_400G = "12,13,14,15,16,17,18,19"
    first_port_in_generate_traffic_GOP_400G = "12"
    do_show_mib_on_part1_ports = tested_8_50G_ports .. "," .. first_port_in_generate_traffic_GOP_400G
    other_GOPs_400G = nil
    other_GOPs_200G = "8,9,10,11"
    first_ports_in_other_GOPs_400G = nil
    first_ports_in_other_GOPs_200G = "8"
    traffic_port_1 = "8"
    traffic_port_2 = traffic_port_1
    traffic_port_3 = traffic_port_1
    traffic_port_4 = traffic_port_1
    do_show_mib_on_all_ports = do_show_mib_on_part1_ports .. "," .. first_ports_in_other_GOPs_200G
    all_ports = tested_8_50G_ports .. "," .. generate_traffic_GOP_400G .. "," .. other_GOPs_200G
elseif(devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E") then
    _400G_speed_on_port = "25000 mode KR" --25G
    _50G_speed_on_port  = "25000 mode KR" --25G
    tested_8_50G_ports = "0,8,16,24,32,40,48,49"
    generate_traffic_GOP_400G = "50-53"
    first_port_in_generate_traffic_GOP_400G = "50"
    do_show_mib_on_part1_ports = tested_8_50G_ports .. "," .. first_port_in_generate_traffic_GOP_400G
    other_GOPs_400G = "50-53" -- reuse first group
    first_ports_in_other_GOPs_400G = "50"
    traffic_port_1 = "50"
    traffic_port_2 = "51"
    traffic_port_3 = traffic_port_2
    traffic_port_4 = traffic_port_2
    do_show_mib_on_all_ports = do_show_mib_on_part1_ports
    all_ports = tested_8_50G_ports .. "," .. generate_traffic_GOP_400G
elseif(is_sip_6_10(devNum))then
    tested_8_50G_ports = "0,1,2,6,10,14,18,22"
    generate_traffic_GOP_400G = "26,27,28,32,36,40,44,48"
    first_port_in_generate_traffic_GOP_400G = "26"
    do_show_mib_on_part1_ports = tested_8_50G_ports .. "," .. first_port_in_generate_traffic_GOP_400G
    other_GOPs_400G = "52-59,64-71"
    first_ports_in_other_GOPs_400G = "52,64"
    traffic_port_1 = "52"
    traffic_port_2 = "64"
    traffic_port_3 = traffic_port_2
    traffic_port_4 = traffic_port_2
    do_show_mib_on_all_ports = do_show_mib_on_part1_ports .. "," .. first_ports_in_other_GOPs_400G
    all_ports = tested_8_50G_ports .. "," .. generate_traffic_GOP_400G .. "," .. other_GOPs_400G
else
    tested_8_50G_ports = "0-7"
    generate_traffic_GOP_400G = "8-15"
    first_port_in_generate_traffic_GOP_400G = "8"
    do_show_mib_on_part1_ports = tested_8_50G_ports .. "," .. first_port_in_generate_traffic_GOP_400G
    other_GOPs_400G = "16-47"
    first_ports_in_other_GOPs_400G = "16,24,32,40"
    traffic_port_1 = "16"
    traffic_port_2 = "24"
    traffic_port_3 = "32"
    traffic_port_4 = "40"
    do_show_mib_on_all_ports = do_show_mib_on_part1_ports .. "," .. first_ports_in_other_GOPs_400G
    all_ports = "0-47"
end

local init_config =
[[
//disable the appDemo from processing NA
do shell-execute prvWrAppAllowProcessingOfAuqMessages 0
end

end
config
// the mac SA (to prevent SA re-learning)
mac address-table static 00:02:22:33:34:44 device ${dev} vlan 1 ethernet ${dev}/50

]]


-- set port 8 as FWS generating traffic
local init_generate_FWS =
[[
end
config
interface range ethernet ${dev}/]] .. generate_traffic_GOP_400G .. [[

no speed
exit
interface range ethernet ${dev}/]] .. first_port_in_generate_traffic_GOP_400G .. [[

speed ]] .. _400G_speed_on_port .. [[

force link up
no shutdown
exit

end
traffic
traffic-generator emulate port ${dev}/]] .. first_port_in_generate_traffic_GOP_400G .. [[ data 000111222333000222333444 count continuous-wire-speed

do shell-execute osTimerWkAfter 1000
do show interfaces mac counters ethernet ${dev}/]] .. do_show_mib_on_part1_ports.. [[

]]

local other_ports_400g_speed = _400G_speed_on_port
local other_ports_400g_list  = other_GOPs_400G
local other_ports_400g_list_first_port  = first_ports_in_other_GOPs_400G

if not other_GOPs_400G then
	other_ports_400g_list   = other_GOPs_200G
	other_ports_400g_speed	= "200000 mode KR4"
	other_ports_400g_list_first_port = first_ports_in_other_GOPs_200G
end

local init_generate_FWS_enhanced =
[[

end
config
interface range ethernet ${dev}/]] ..other_ports_400g_list .. [[

no force link up
no speed
exit
interface range ethernet ${dev}/]] .. other_ports_400g_list_first_port .. [[

no speed
speed ]] .. other_ports_400g_speed .. [[

force link up
no shutdown
exit

end
traffic
traffic-generator emulate port ${dev}/]] .. traffic_port_1 .. [[ data 000111222333000222333444 count continuous-wire-speed

traffic-generator emulate port ${dev}/]] .. traffic_port_2 .. [[ data 000111222333000222333444 count continuous-wire-speed
traffic-generator emulate port ${dev}/]] .. traffic_port_3 .. [[ data 000111222333000222333444 count continuous-wire-speed
traffic-generator emulate port ${dev}/]] .. traffic_port_4 .. [[ data 000111222333000222333444 count continuous-wire-speed
do shell-execute osTimerWkAfter 1000
do show interfaces mac counters ethernet ${dev}/]] .. do_show_mib_on_all_ports .. [[

]]


if not wrlCpssIsAsicSimulation() then
    -- more stress if not WM
    init_generate_FWS = init_generate_FWS .. init_generate_FWS_enhanced
end

local restore_config =
[[
end

traffic
no traffic-generator emulate port ${dev}/]] .. first_port_in_generate_traffic_GOP_400G .. [[

no traffic-generator emulate port ${dev}/]] .. traffic_port_1 .. [[

no traffic-generator emulate port ${dev}/]] .. traffic_port_2 .. [[

no traffic-generator emulate port ${dev}/]] .. traffic_port_3 .. [[

no traffic-generator emulate port ${dev}/]] .. traffic_port_4 .. [[

do shell-execute osTimerWkAfter 1000
//do consecutive 2 'read' counters to see that traffic is alive on the ports that should show traffic
do show interfaces mac counters ethernet ${dev}/]] .. do_show_mib_on_part1_ports .. [[

do show interfaces mac counters ethernet ${dev}/]] .. do_show_mib_on_part1_ports .. [[

end
config
interface range ethernet ${dev}/]] .. all_ports .. [[

no speed
speed ]] .. _50G_speed_on_port .. [[

no force link up
no shutdown
exit

do shell-execute prvWrAppAllowProcessingOfAuqMessages 1
end
config
// the mac SA (to prevent SA re-learning)
no mac address-table static 00:02:22:33:34:44 device ${dev} vlan 1
end
]]

local getIntoInterfaceMode =
[[
end
config
//get into ports 0-7
interface range ethernet ${dev}/]] .. tested_8_50G_ports .. [[

]]
--
-- under_traffic_config is called in context of : interface range ethernet ${dev}/0-7
--
local under_traffic_config =
[[
no speed
do shell-execute osTimerWkAfter 300
speed ]] .. _50G_speed_on_port .. [[

force link up
no shutdown
//do consecutive 2 'read' counters to see that traffic is alive on the ports that should show traffic
do show interfaces mac counters ethernet ${dev}/]] .. do_show_mib_on_part1_ports .. [[


do shell-execute osTimerWkAfter 1000
do show interfaces mac counters ethernet ${dev}/]] .. do_show_mib_on_part1_ports .. [[


]]

executeStringCliCommands(init_config)
executeStringCliCommands(init_generate_FWS)
executeStringCliCommands(getIntoInterfaceMode)
for ii = 0,5 do
    executeStringCliCommands(under_traffic_config)
end
executeStringCliCommands(restore_config)

