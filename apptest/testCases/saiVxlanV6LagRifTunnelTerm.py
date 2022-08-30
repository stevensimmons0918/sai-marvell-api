import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Vxlan Ipv6 Packet',
    'tcName' : 'saiVxlanV6LagRifTunnelTerm',
    'description' : 'Verify Vxlan tunnel Termination on Lag RIF',
    'ingressPort' : ['30'],
    'egressPort' : ['29'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
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
sai_set_port_attribute 281474976710687 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
lag
sai_create_lag 9288674231451648 1 > lag0
sai_create_lag_member 9288674231451648 281474976710684 $lag0 > mem28
sai_create_lag_member 9288674231451648 281474976710686 $lag0 > mem30
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT $lag0 $vr_id > rif_i_prt_01
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 1 10:01:0:0:0:0:0:0:0:0:0:0:0:0:0:1 >nh0
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 1 10:01:0:0:0:0:0:0:0:0:0:0:0:0:0:1 00:04:04:04:04:49
back
route
sai_create_route_entry 9288674231451648 $vr_id 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] [ff:ff:0:0:0:0:0:0:0:0:0:0:0:0:0:0]
sai_set_route_entry_attribute 9288674231451648 $vr_id 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] [ff:ff:0:0:0:0:0:0:0:0:0:0:0:0:0:0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nh0

back
tunnel
sai_create_tunnel_map 9288674231451648 SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI > tunnel_map1
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI 0 $tunnel_map1 2000 0 100 0 0 > tunmapEntry1
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI 0 $tunnel_map1 3000 0 300 0 0 > tunmapEntry3
sai_create_tunnel_map 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID > tunnel_map2
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID 0 $tunnel_map2 0 100 0 2000 0 > tunmapEntry2
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID 0 $tunnel_map2 0 300 0 3000 0 > tunmapEntry4
back
vlan
sai_create_vlan 9288674231451648 100 > vlan100
sai_create_vlan 9288674231451648 300 > vlan300
back
tunnel
sai_create_tunnel 9288674231451648 $rif_i_prt_01 $tunnel_map2 1 [4f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] 0 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] SAI_TUNNEL_TYPE_VXLAN $tunnel_map1 > tunnelId
sai_create_tunnel 9288674231451648 $rif_i_prt_01 $tunnel_map2 1 [5f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] 0 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] SAI_TUNNEL_TYPE_VXLAN $tunnel_map1 > tunnelId2
sai_create_tunnel 9288674231451648 $rif_i_prt_01 $tunnel_map2 1 [6f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] 0 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] SAI_TUNNEL_TYPE_VXLAN $tunnel_map1 > tunnelId3

sai_create_tunnel_term_table_entry 9288674231451648 1 [4f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] 1 [ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff] $tunnelId SAI_TUNNEL_TYPE_VXLAN 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P $vr_id 1 [ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff] > tuntermId
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg
back
bridge
sai_create_bridge_port 9288674231451648 0 0 $brg SAI_BRIDGE_PORT_TYPE_TUNNEL $tunnelId 0 > brgport
sai_create_bridge_port 9288674231451648 0 0 $brg SAI_BRIDGE_PORT_TYPE_TUNNEL $tunnelId2 0 > brgport2
sai_create_bridge_port 9288674231451648 0 0 $brg SAI_BRIDGE_PORT_TYPE_TUNNEL $tunnelId3 0 > brgport3
back
vlan
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218077 1 > vmem29
sai_create_vlan_member 9288674231451648 $vlan300 16325548649218077 1 > vmem29_2

sai_create_vlan_member 9288674231451648 $vlan100 $brgport 1 > vmem1
sai_create_vlan_member 9288674231451648 $vlan100 $brgport2 1 > vmem2
sai_create_vlan_member 9288674231451648 $vlan100 $brgport3 1 > vmem3

sai_create_vlan_member 9288674231451648 $vlan300 $brgport 1 > vmem11
sai_create_vlan_member 9288674231451648 $vlan300 $brgport2 1 > vmem21
sai_create_vlan_member 9288674231451648 $vlan300 $brgport3 1 > vmem31
alter_create_mode 0

back
lag
sai_create_lag_member 9288674231451648 281474976710687 $lag0 > mem31
sai_remove_lag_member $mem30
sai_remove_lag_member $mem28
sai_create_lag_member 9288674231451648 281474976710686 $lag0 > mem30
sai_create_lag_member 9288674231451648 281474976710684 $lag0 > mem28
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem31
sai_remove_vlan_member $vmem21
sai_remove_vlan_member $vmem11
sai_remove_vlan_member $vmem3
sai_remove_vlan_member $vmem2
sai_remove_vlan_member $vmem1
back
bridge
sai_remove_bridge_port $brgport3
sai_remove_bridge_port $brgport2
sai_remove_bridge_port $brgport
back
tunnel
sai_remove_tunnel_term_table_entry $tuntermId
sai_remove_tunnel $tunnelId3
sai_remove_tunnel $tunnelId2
sai_remove_tunnel $tunnelId

sai_remove_tunnel_map_entry $tunmapEntry4
sai_remove_tunnel_map_entry $tunmapEntry3
sai_remove_tunnel_map_entry $tunmapEntry2
sai_remove_tunnel_map_entry $tunmapEntry1
sai_remove_tunnel_map $tunnel_map2
sai_remove_tunnel_map $tunnel_map1
back
route
sai_remove_route_entry 9288674231451648 $vr_id 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] [ff:ff:0:0:0:0:0:0:0:0:0:0:0:0:0:0]
back
nexthop
sai_remove_next_hop $nh0
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 1 10:01:0:0:0:0:0:0:0:0:0:0:0:0:0:1
back
routerinterface
sai_remove_router_interface $rif_i_prt_01
back
vlan
sai_remove_vlan $vlan100
sai_remove_vlan $vlan300
back
lag
sai_remove_lag_member $mem30
sai_remove_lag_member $mem28
sai_remove_lag_member $mem31
sai_remove_lag $lag0
back
virtualrouter
sai_remove_virtual_router $vr_id

'''
packet_info = scapy_t.Ether(src="00:04:04:04:04:49",dst="00:00:01:11:22:02")/scapy_t.IPv6(dst="3ffe:0501:4819:0:0:0:0:0", src="4ffe:0501:4819:0:0:0:0:0",hlim=29,tc=18)/scapy_t.UDP(sport=10, dport=4789)/scapy_t.VXLAN(vni=2000)/scapy_t.Ether(src="00:00:11:00:bb:cc",dst="00:aa:ff:ff:ff:00")/scapy_t.Dot1Q(vlan=100)/scapy_t.IP(dst="1.1.1.1", src="2.2.2.2",ttl=50)/scapy_t.UDP()

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
       'tap1':[<Ether  dst=00:aa:ff:ff:ff:00 src=00:00:11:00:bb:cc type=IPv4 |<IP  version=4 ihl=5 tos=0x10 len=28 id=1 flags= frag=0 ttl=29 proto=udp chksum=0x97bb src=2.2.2.2 dst=1.1.1.1 |<UDP  sport=domain dport=domain len=0 chksum=0x0 |
        '''
}
