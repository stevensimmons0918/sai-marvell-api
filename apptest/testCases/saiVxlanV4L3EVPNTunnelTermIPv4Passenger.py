import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Vxlan Ipv4 Packet',
    'tcName' : 'saiVxlanV4L3EVPNTunnelTermIPv4Passenger',
    'description' : 'Verify Vxlan ipv4 tunnel nexthop terminate',
    'ingressPort' : ['28'],
    'egressPort' : ['29'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap1'],
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
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
vlan
sai_create_vlan 9288674231451648 100 > vlan100
sai_create_vlan 9288674231451648 200 > vlan200
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218077 1 > vmem29
sai_create_vlan_member 9288674231451648 $vlan200 16325548649218078 1 > vmem29_2
alter_create_mode 0
back

virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710684 $vr_id > rif_i_prt_01
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 192.168.2.100 > nh0
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 0 192.168.2.100 00:04:04:04:04:49
back
route
sai_create_route_entry 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nh0
back

routerinterface
sai_create_router_interface 9288674231451648 $vlan100 SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id > rif_vlan100
back

routerinterface
sai_create_router_interface 9288674231451648 $vlan200 SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id > rif_vlan200
back

nexthop
sai_create_next_hop 9288674231451648 0 $rif_vlan100 SAI_NEXT_HOP_TYPE_IP 0 10.1.1.2 > nh1
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_vlan100 0 10.1.1.2 00:05:05:05:05:05
back
route
sai_create_route_entry 9288674231451648 $vr_id 0 20.1.1.1 255.255.255.255
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 20.1.1.1 255.255.255.255 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nh1
back


tunnel
sai_create_tunnel_map 9288674231451648 SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI > tunnel_map1
sai_create_tunnel_map 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID > tunnel_map2
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI 0 $tunnel_map1 3000 0 0 0 $vr_id > tunmapEntry1
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID $vr_id $tunnel_map2 0 0 0 3000 0 > tunmapEntry2
back

tunnel
sai_create_tunnel 9288674231451648 $rif_i_prt_01 $tunnel_map2 0 192.168.3.1 0 0 192.168.2.10 SAI_TUNNEL_TYPE_VXLAN $tunnel_map1 > tunnelId
sai_create_tunnel_term_table_entry 9288674231451648 0 192.168.2.10 0 255.255.255.255 $tunnelId SAI_TUNNEL_TYPE_VXLAN 0 192.168.3.1 SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P $vr_id 0 255.255.255.255 > tuntermId
back

switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg
back
bridge
sai_create_bridge_port 9288674231451648 0 0 $brg SAI_BRIDGE_PORT_TYPE_TUNNEL $tunnelId 0 > brgport
back
vlan
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan100 $brgport 1 > vmem1_tun
alter_create_mode 0
back

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem29
sai_remove_vlan_member $vmem29_2
sai_remove_vlan_member $vmem1_tun
back

route
sai_remove_route_entry 9288674231451648 $vr_id 0 20.1.1.1 [255.255.255.255]
back

bridge
sai_remove_bridge_port $brgport
back

tunnel
sai_remove_tunnel_term_table_entry $tuntermId
sai_remove_tunnel $tunnelId
sai_remove_tunnel_map_entry $tunmapEntry1
sai_remove_tunnel_map_entry $tunmapEntry2
sai_remove_tunnel_map $tunnel_map1
sai_remove_tunnel_map $tunnel_map2
back

routerinterface
sai_remove_router_interface $rif_vlan200
back

nexthop
sai_remove_next_hop $nh1
back

neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_vlan100 0 10.1.1.2
back

routerinterface
sai_remove_router_interface $rif_vlan100
back

vlan
sai_remove_vlan $vlan100
back

route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
back

nexthop
sai_remove_next_hop $nh0
back

neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 0 192.168.2.100
back

routerinterface
sai_remove_router_interface $rif_i_prt_01
back

vlan
sai_remove_vlan $vlan200
back

virtualrouter
sai_remove_virtual_router $vr_id



'''
packet_info = scapy_t.Ether(src="00:04:04:04:04:49",dst="00:00:01:11:22:02")/scapy_t.IP(dst="192.168.3.1", src="192.168.2.10",ttl=55,tos=3)/scapy_t.UDP(sport=10, dport=4789)/scapy_t.VXLAN(vni=3000)/scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="20.1.1.1", src="2.2.2.2",ttl=30)/scapy_t.UDP()

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
       'tap1':[<Ether  dst=00:05:05:05:05:05 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=28 id=1 flags= frag=0 ttl=54 proto=udp chksum=0x6bcb src=2.2.2.2 dst=20.1.1.1 |<UDP  sport=domain dport=domain len=0 chksum=0x0 |
       '''
}
