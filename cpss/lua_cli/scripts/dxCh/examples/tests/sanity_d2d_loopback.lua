--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* sanity_d2d_loopback.lua
--*
--* DESCRIPTION:
--*       check traffic within the Eagle without Raven as 'Eagle-D2D' in MAC/PCS Loopback
--*       this is to bypass 'GW problems' (D2D SERDES) , till solved.
--*       the traffic from 'port2' to 'port3'
--*       the Test bypass the need for MAC ports ! (in Falcon this is without Raven)
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 
local port5   = devEnv.port[5]
local port6   = devEnv.port[6] 

local ingressPort = port2
local egressPort = port3

-- currently 'sip_6' is good enough for 'D2D exists'
SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6")
-- GM not supported
SUPPORTED_FEATURE_DECLARE(devEnv.dev, "NOT_BC2_GM")
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local macDa   = "000000000058"
local macSa   = "000000001111"

local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end
    
    return macDa .. macSa ..  partAfterMac .. packetPayload
end
-- remove first 14 bytes
-- the PCH on ingress is 8 bytes but on egress is 1 byte
-- since 2 passes in the D2D LB , we loose 7*2 bytes = 14
local function remove14Bytes(origPacket)
    return string.sub(origPacket, 1+14*2)
end

--##################################
--##################################
local ingressPacket = buildPacketUc(nil)
local egressPacket = remove14Bytes(ingressPacket)

local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable = {
    -- expected to get to other port without changes (flood in vlan)
    {portNum = egressPort, pktInfo = {fullPacket = egressPacket}}
}

--[[
GT_STATUS mvHwsD2dPcsLoopbackSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   d2dNum,
    IN MV_HWS_PORT_LB_TYPE      lbType /*DISABLE_LB = 0*/ , /*TX_2_RX_LB = 2*/
);

GT_STATUS mvHwsD2dPmaLaneTestLoopbackSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  d2dNum,
    IN MV_HWS_PORT_LB_TYPE     lbType
)

lbType = RX_2_TX_LB or DISABLE_LB

d2dNum = like in previous API (but in our case it should be in Raven) 

]]--

local DISABLE_LB = 0
local RX_2_TX_LB = 1
local TX_2_RX_LB = 2
--local TX_2_RX_DIGITAL_LB = 3

local status , numOfPortGroups , numOfPipes , tmp_numOfTiles = wrLogWrapper("wrlCpssDevicePortGroupNumberGet", "(devNum)", devEnv.dev) 

-- set ALL D2D Eagle side as loopback or not (Tx to Rx)
-- OR 
-- set ALL D2D Raven side as loopback or not (Rx to Tx)
-- it also MUST avoid Raven configurations (MAC) when D2D in LB mode - to avoid stuck the PEX
local function setD2DLoopback(devNum,side,enable)

    local lbType,d2dNum,d2dIdMax,d2dIdStart
    local apiName
    if side == "EAGLE" then
        d2dIdStart = 0
        if enable then
            lbType = TX_2_RX_LB
        else
            lbType = DISABLE_LB
        end
        apiName = "mvHwsD2dPcsLoopbackSet"
    else --"Raven"
        d2dIdStart = 1
        if enable then
            lbType = RX_2_TX_LB
        else
            lbType = DISABLE_LB
        end
        apiName = "mvHwsD2dPmaLaneTestLoopbackSet"
    end
    
    d2dIdMax = d2dIdStart + (tmp_numOfTiles*16) - 2
    --notify TGF and LUA transmit engine
    luaTgfTransmitEngineUseD2DLoopback(enable)
    
    -- set ALL D2D Eagle/Raven side
    for d2dNum=d2dIdStart,d2dIdMax,2 do
        local str = "do shell-execute "..apiName.." "..devNum..","..d2dNum..","..lbType
        executeStringCliCommands(str)
    end
   
   if enable == false then
      -- allow the D2D to restore D2D CP
      executeStringCliCommands ("do shell-execute osTimerWkAfter 1000")
   end
end


-- set tested ports in proper settings :
-- non-tested ports must not be in vlan , otherwise will do D2D LB ! and cause storming.
local function setTestedPorts(devNum)
    local defineVlan200 = 
    [[
    end
    configure
    interface vlan device ${dev} vid 200
    exit
    interface range ethernet ${dev}/]].. ingressPort .."," .. egressPort .. [[
    
    switchport allowed vlan add 200 untagged
    switchport customer vlan 200
    end
    ]]
    
    executeStringCliCommands(defineVlan200)

end

-- set tested ports in proper settings :
-- non-tested ports must not be in vlan , otherwise will do D2D LB ! and cause storming.
local function restoreTestedPorts(devNum)
    local remove_Vlan200 = 
    [[
    end
    configure
    interface range ethernet ${dev}/]].. ingressPort .."," .. egressPort .. [[
    
    switchport allowed vlan remove 200
    no switchport customer vlan
    exit
    
    no interface vlan device ${dev} vid 200
    end
    ]]
    
    executeStringCliCommands(remove_Vlan200)

    -- restore pvid (not done by 'no switchport customer vlan')
    cpssPerPortParamSet("cpssDxChBrgVlanPortVidSet", 
                        devNum, ingressPort, 1, "vlanId", 
                        "GT_U16")       
    cpssPerPortParamSet("cpssDxChBrgVlanPortVidSet", 
                        devNum, egressPort, 1, "vlanId", 
                        "GT_U16")       

end

-- function to implement the test
local function doTest(side--[["EAGLE"/"RAVEN"]])
    --set config
    local sectionName = side .. " check traffic within the Eagle without Raven as '" .. side .."-D2D' in MAC/PCS Loopback"
    
    printLog("start : " .. sectionName)
    
    setD2DLoopback(devNum,side,true)
    -- check that packet egress the needed port(s)
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    setD2DLoopback(devNum,side,false)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState() 
    end  

end

--*******************************************************--
setTestedPorts(devNum)

-- run the test
doTest("EAGLE")
-- run the test 
--[[ kill the PEX before any traffic after:
        Performing: do shell-execute tgfTrafficGeneratorUseD2DLoopbackSet 1
            luaShellExecute: tgfTrafficGeneratorUseD2DLoopbackSet 1
            return code is 0
        Performing: do shell-execute mvHwsD2dPmaLaneTestLoopbackSet 0,1,1
            luaShellExecute: mvHwsD2dPmaLaneTestLoopbackSet 0,1,1
--doTest("RAVEN")
]]--

restoreTestedPorts(devNum)
