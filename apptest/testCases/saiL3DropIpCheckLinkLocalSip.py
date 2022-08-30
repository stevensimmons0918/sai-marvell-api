import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3DropIpCheckLinkLocalSip',
    'description' : 'TC to verify if RIF Drop stats works with basic forwarding',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'pktAction' : 'DROP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'rif_counter'    : ['ing_rif_id'],
    'count' : 0,             # expected data count
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

home
sai

vlan
sai_create_vlan 9288674231451648 100 >vlan_id_100
sai_create_vlan 9288674231451648 200 >vlan_id_200
sai_create_vlan_member 281474976710685 $vlan_id_100 16325548649218077 >vlan_mbr_29
sai_create_vlan_member 281474976710685 $vlan_id_200 16325548649218078 >vlan_mbr_30
back

virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back

routerinterface
sai_create_router_interface 9288674231451648 $vlan_id_100 SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id >ing_rif_id
sai_create_router_interface 9288674231451648 $vlan_id_200 SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id >egr_rif_id

back
neighbor
sai_create_neighbor_entry 9288674231451648 $egr_rif_id 0 10.1.1.20 00:02:03:04:05:06
back

nexthop
sai_create_next_hop 9288674231451648 0 $egr_rif_id SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1
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
sai_remove_neighbor_entry 9288674231451648 $egr_rif_id 0 10.1.1.20
back
routerinterface
sai_remove_router_interface $egr_rif_id
sai_remove_router_interface $ing_rif_id
back
vlan
sai_remove_vlan $vlan_id_100
sai_remove_vlan $vlan_id_200
back
'''
packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.Dot1Q(vlan =100)/scapy_t.IP(dst="192.168.2.10", src="169.254.10.125")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'',
}
