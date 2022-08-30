--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cnc_ingress_vlan.lua
--*
--* DESCRIPTION:
--*       The test for testing CNC ingress vlan
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5")

local payloads
local ret, status
local vlanID = 0x14
local numberOfPackets, numberOfBytes

--read packet data
do
  ret, payloads = pcall(dofile, "dxCh/examples/packets/cnc_ingress_vlan.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

-- next lines will set 'speed' on the ports (because the pre-test may removed speed)
resetPortCounters(devEnv.dev,devEnv.port[1])
resetPortCounters(devEnv.dev,devEnv.port[2])

executeLocalConfig("dxCh/examples/configurations/cnc_ingress_vlan.txt")

local devNum1 = devEnv.dev
local portNum1 = devEnv.port[1]


local transmitInfo1 =
{
  devNum = devNum1,
  portNum = portNum1, pktInfo = {fullPacket = payloads["cnc_ingress_vlan_packet"]}
}

--sending packet
status = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo1, nil);

local params = {}
params["devID"] = devNum1
params["vlanIdOrAll"] = vlanID

local ret, showVals = showCountersIngressVlan(params)

executeLocalConfig("dxCh/examples/configurations/cnc_ingress_vlan_deconfig.txt")

if (ret == false) then
    setFailState()
else
    numberOfPackets = showVals["pass"][1]["Packet-Counter"]
    numberOfBytes = showVals["pass"][1]["Byte-count counter"]

    if not (numberOfPackets == 1 and numberOfBytes >= 64) then
        setFailState()
    end
end

