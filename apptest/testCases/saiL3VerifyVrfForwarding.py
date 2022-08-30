import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3VerifyVrfForwarding',
    'description' : 'TC to verify if basic VRF forwarding works through route and nexthop',
    'ingressPort' : ['30'],
    'egressPort' : ['28'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap0'],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID 1 > def_vrf

home
sai
virtualrouter
sai_set_virtual_router_attribute $def_vrf SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:aa

sai_create_virtual_router 9288674231451648 >vr_id2
sai_set_virtual_router_attribute $vr_id2 SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:bb
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710684 $vr_id2 >out_rif
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $def_vrf >in_rif

back
neighbor
sai_create_neighbor_entry 9288674231451648 $out_rif 0 10.1.1.20 00:02:03:04:05:06
back

nexthop
sai_create_next_hop 9288674231451648 0 $out_rif SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1
back

route
sai_create_route 9288674231451648 $def_vrf 0 192.168.2.10 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $def_vrf 0 192.168.2.10 [255.255.255.0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $next_hop_id_out1

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
route
sai_remove_route_entry 9288674231451648 $def_vrf 0 192.168.2.10 255.255.255.0
back
nexthop
sai_remove_next_hop $next_hop_id_out1
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $out_rif SAI_IP_ADDR_FAMILY_IPV4 10.1.1.20
back
routerinterface
sai_remove_router_interface $out_rif
sai_remove_router_interface $in_rif
back
virtualrouter
sai_remove_virtual_router $vr_id2
back
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:aa")/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'''
'tap0': [<Ether  dst=00:02:03:04:05:06 src=00:00:01:11:22:bb type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=63 proto=hopopt chksum=0xf784 src=192.168.1.10 dst=192.168.2.10 |
 '''
}
