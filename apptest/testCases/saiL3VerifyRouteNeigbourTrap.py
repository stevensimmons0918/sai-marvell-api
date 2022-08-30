import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3VerifyRouteNeigbourTrap.py',
    'description' : 'TC to verify if the packet is getting trapped when the neighbor packet action is set to SAI_PACKET_ACTION_TRAP',
    'ingressPort' : ['30'],
    'egressPort' : [],
    'pktAction' : 'TRAP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : '',
    'count' : 0,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id >rif_id_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id >rif_id_2

back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_id_1 0 10.1.1.20 00:02:03:04:05:06
sai_set_neighbor_entry_attribute 9288674231451648 $rif_id_1 SAI_IP_ADDR_FAMILY_IPV4 10.1.1.20 SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION SAI_PACKET_ACTION_TRAP
back

nexthop
sai_create_next_hop 9288674231451648 0 $rif_id_1 SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1
back

route
sai_create_route 9288674231451648 $vr_id 0 192.168.2.0 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.0 [255.255.255.0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $next_hop_id_out1

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.0 255.255.255.0
back
nexthop
sai_remove_next_hop $next_hop_id_out1
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_id_1 SAI_IP_ADDR_FAMILY_IPV4 10.1.1.20
back
routerinterface
sai_remove_router_interface $rif_id_1
back
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10")
#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       '',
}
