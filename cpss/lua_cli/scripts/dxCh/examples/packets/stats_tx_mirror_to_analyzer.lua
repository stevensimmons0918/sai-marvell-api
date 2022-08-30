-- Incoming and egressed packets used in Tx stats mirror to analyzer test.
local txEthernetHeader = ""
   .."000000000033"             -- MAC DA
   .."000000000011"             -- MAC SA
   
local payload = ""
   .."fffefdfcfbfa000000000000" -- 48 bytes
   .."000000000000000000000000000000000000000000000000000000000000000000000000"

   
-------------------- PACKETS ---------------------------------------------------
-- the packet sent by device should be mirroed at port 3
local packet = txEthernetHeader..payload

return { 
   rx1 = packet,
   tx1 = packet
}
