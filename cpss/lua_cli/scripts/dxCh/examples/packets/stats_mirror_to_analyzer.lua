-- Incoming and egressed packets used in stats mirror to analyzer test.
local rxEthernetHeader = ""
   .."000000000033"             -- MAC DA
   .."000000000011"             -- MAC SA
   
local payload = ""
   .."fffefdfcfbfa000000000000" -- 48 bytes
   .."000000000000000000000000000000000000000000000000000000000000000000000000"

   
-------------------- PACKETS ---------------------------------------------------
-- incoming packet
local rxPacket = rxEthernetHeader..payload

-- the packet sent by device should be egressed to port 2
local txPacket = rxEthernetHeader..payload

return { 
   rx1 = rxPacket,
   tx2 = txPacket
}
