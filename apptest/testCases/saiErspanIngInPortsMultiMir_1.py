import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L3Packet',
    'tcName'         : 'saiErspanEgrOutPortsMultiMir_1',
    'description'    : 'TC to verify IACL trap for inports Entry',
    'ingressPort'    : ['28'],
    'egressPort'     : ['30'],
    'vlan'           : '75',
    'pktAction'      : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
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
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218076 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1

home
sai
mirror
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 2.2.2.2 SAI_IP_ADDR_FAMILY_IPV4 3.3.3.3 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 35006 4 8 > mirror_session0
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 4.4.4.4 SAI_IP_ADDR_FAMILY_IPV4 5.5.5.5 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:88 00:00:11:22:33:99 35006 4 8 > mirror_session1

back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 1 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 > acl_tbl_id
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 > acl_table_group_mem
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id1

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6 1 255:255:255:255:255:255:255:255:255:255:255:255:255:255:255:255 02:00:00:01:00:00:00:00:00:00:00:00:00:00:00:08
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS 1 281474976710684
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0

sai_create_acl_entry 9288674231451648 $acl_tbl_id 11 > acl_entry_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6 1 255:255:255:255:255:255:255:255:255:255:255:255:255:255:255:255 03:00:00:01:00:00:00:00:00:00:00:00:00:00:00:08
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS 1 281474976710684
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session1
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IPv6(src="0200:0001:0:0:0:0:0:0008",dst="0200:0001:0:0:0:0:0:0009")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}
expectedData = {
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=100 id=0 flags= frag=0 ttl=255 proto=gre chksum=0xb159 src=2.2.2.2 dst=3.3.3.3 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x1 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=0 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=VLAN |<Dot1Q  prio=0 id=0 vlan=75 type=IPv6 |<IPv6  version=6 tc=0 fl=0 plen=0 nh=No Next Header hlim=64 src=200:1::8 dst=200:1::9
'''
}

