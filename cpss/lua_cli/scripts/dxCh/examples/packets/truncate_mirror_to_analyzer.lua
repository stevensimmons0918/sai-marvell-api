-- Incoming and egressed packets used in truncate mirror to analyzer test.
local rxEthernetHeader = ""
   .."000000000022"             -- MAC DA
   .."000000000011"             -- MAC SA
   

local payload_truncated = ""
   .."fffefdfcfbfa000000000000" 
   .."000000000000000000000000000000000000000000000000000000000000000000000000"
   .."000000000000000000000000bc00000000000000a2000000000000000000000000000000"
   .."0000000000000000000000000000000000000000000000bb00cc00ff"

 local payload = payload_truncated .. --[[additional bytes that will be truncated]]
	 "00000000" 
   .."000000000000000000000000000000000000000000000000000000000000000000000000"
   .."000000000000000000000000000000000000000000000000000000000000000000000000"   
  
-------------------- PACKETS ---------------------------------------------------
-- incoming packet
local rxPacket = rxEthernetHeader..payload

-- the packet sent by device should be egressed to port 2
local txPacket = rxEthernetHeader..payload

-- the packet sent by device should be egressed to port 2
local txPacket_truncated = rxEthernetHeader..payload_truncated

return { 
   rx1 = rxPacket,
   tx2 = txPacket,
   tx3 = txPacket_truncated
}
