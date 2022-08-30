import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3VerifyVrfVlanForwarding',
    'description' : 'TC to verify if basic VRF forwarding works through route and nexthop via router ingress vlan interface',
    'ingressPort' : ['28'],
    'egressPort' : ['30'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai

vlan
sai_create_vlan 9288674231451648 100 >vlan_id
sai_create_vlan_member 281474976710685 $vlan_id 16325548649218077 >vlan_mbr_29
sai_create_vlan_member 281474976710685 $vlan_id 16325548649218076 >vlan_mbr_28
back

virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id2
sai_set_virtual_router_attribute $vr_id2 SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:aa
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id2 >out_rif
sai_create_router_interface 9288674231451648 $vlan_id SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id >in_rif
sai_set_router_interface_attribute $in_rif SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:33

back
neighbor
sai_create_neighbor_entry 9288674231451648 $out_rif 0 10.1.1.20 00:02:03:04:05:06
back

nexthop
sai_create_next_hop 9288674231451648 0 $out_rif SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1
back

route
sai_create_route 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $next_hop_id_out1

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
back
nexthop
sai_remove_next_hop $next_hop_id_out1
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $out_rif 0 10.1.1.20
back
routerinterface
sai_remove_router_interface $in_rif
sai_remove_router_interface $out_rif
back
vlan
sai_remove_vlan $vlan_id
back
virtualrouter
sai_remove_virtual_router $vr_id2
sai_remove_virtual_router $vr_id
back

'''
packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:33")/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap2': [<Ether  dst=00:02:03:04:05:06 src=00:00:01:11:22:aa type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=63 proto=hopopt chksum=0xf784 src=192.168.1.10 dst=192.168.2.10 |
'''
}
