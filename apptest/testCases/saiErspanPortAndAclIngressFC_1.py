import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiErspanPortAndAclIngress_1',
    'description' : 'TC to verify ingress port based and acl based erspan',
    'ingressPort' : ['28'],
    'egressPort' : ['30'],
    'vlan'       : '',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
    'count' : 1,   # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.
tcProgramStr = '''
home
sai
mirror
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 2.2.2.2 SAI_IP_ADDR_FAMILY_IPV4 3.3.3.3 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 35006 4 8 > mirror_session0
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 4.4.4.4 SAI_IP_ADDR_FAMILY_IPV4 5.5.5.5 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710685 00:00:11:22:33:88 00:00:11:22:33:99 35006 4 8 > mirror_session1
back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group0
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_table0
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter0
sai_create_acl_entry 9288674231451648 $acl_table0 10 > acl_entry0
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.44
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group0
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION $mirror_session1
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_table0 $acl_table_group0 0 > acl_table_group_member0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.44", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:22:33:88 src=00:00:11:22:33:99 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=100 id=0 flags= frag=0 ttl=255 proto=gre chksum=0xa951 src=4.4.4.4 dst=5.5.5.5 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x1 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=1 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf358 src=195.169.0.44 dst=195.169.0.18
'''
}
