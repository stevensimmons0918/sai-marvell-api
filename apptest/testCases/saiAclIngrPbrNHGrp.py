import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiAclIngrPbrNHGrp',
    'description'    : 'TC to verify IACL PBR NH Group Entry',
    'ingressPort'    : ['29'],
    'egressPort'     : ['31'],
    'pktAction'      : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap3'],
    'count'          : 1 ,             # expected data count
    'acl_counter'    : ['acl_counter_id0'] #only for acl cases
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
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710684 $vr_id >rif_id_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id >rif_id_2
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710687 $vr_id >rif_id_3
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id >ing_rif

back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_id_1 0 10.1.1.20 00:10:03:04:05:06
sai_create_neighbor_entry 9288674231451648 $rif_id_2 0 20.1.1.20 00:20:03:04:05:06
sai_create_neighbor_entry 9288674231451648 $rif_id_3 0 30.1.1.20 00:30:03:04:05:06
back

nexthop
sai_create_next_hop 9288674231451648 0 $rif_id_1 SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1
sai_create_next_hop 9288674231451648 0 $rif_id_2 SAI_NEXT_HOP_TYPE_IP 0 20.1.1.20 >next_hop_id_out2
sai_create_next_hop 9288674231451648 0 $rif_id_3 SAI_NEXT_HOP_TYPE_IP 0 30.1.1.20 >next_hop_id_out3
back

nexthopgroup
sai_create_next_hop_group 9288674231451648 SAI_NEXT_HOP_GROUP_TYPE_ECMP 0 >nhop_group_id1
sai_create_next_hop_group_member 9288674231451648 $next_hop_id_out1 $nhop_group_id1 0 >nh_group_member_id_1
sai_create_next_hop_group_member 9288674231451648 $next_hop_id_out2 $nhop_group_id1 0 >nh_group_member_id_2
sai_create_next_hop_group 9288674231451648 SAI_NEXT_HOP_GROUP_TYPE_ECMP 0 >nhop_group_id2
sai_create_next_hop_group_member 9288674231451648 $next_hop_id_out3 $nhop_group_id2 0 >nh_group_member_id_3


home
sai
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 1 0 0 > acl_tbl_id


sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT 1 65535 2471
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT 1 65535 2472

sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE 1 65535 3

sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT 1 $nhop_group_id1
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
    
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 > acl_table_group_mem
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT 1 $nhop_group_id2
back
route
sai_create_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nhop_group_id1

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
route
sai_remove_route 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]

home
sai
acl
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group
back
nexthop
sai_remove_next_hop $next_hop_id_out1
sai_remove_next_hop $next_hop_id_out2
sai_remove_next_hop $next_hop_id_out3
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_id_1 0 10.1.1.20 00:10:03:04:05:06
sai_remove_neighbor_entry 9288674231451648 $rif_id_2 0 20.1.1.20 00:20:03:04:05:06
sai_remove_neighbor_entry 9288674231451648 $rif_id_3 0 30.1.1.20 00:30:03:04:05:06
back
routerinterface
sai_remove_router_interface $ing_rif
sai_remove_router_interface $rif_id_3
sai_remove_router_interface $rif_id_2
sai_remove_router_interface $rif_id_1
back
virtualrouter
sai_remove_virtual_router $vr_id
back

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10")/scapy_t.TCP(sport=2471,dport=2472)
#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'''
'tap3':[<Ether  dst=00:30:03:04:05:06 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=40 id=1 flags= frag=0 ttl=63 proto=tcp chksum=0xf76a src=192.168.1.10 dst=192.168.2.10 |<TCP  sport=2471 dport=2472 seq=0 ack=0 dataofs=5 reserved=0 flags=S window=8192 chksum=0xf82e urgptr=0 |
    '''
}

