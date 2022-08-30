import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiEverflowIpv6ErspanMultipleMirrorSessionTest_1',
    'description' : 'TC to verify Ipv4 mirroring(ERSPAN) with multiple mirror session and multiple ACL Rules',
    'ingressPort' : ['28'],
    'egressPort' : ['30'],
    'vlan'       : '75',
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
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:11:11:22:22:0:0:0:0:0:0:0:2 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:12:12:12:12:0:0:0:0:0:0:0:1 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 25944 6 8 > mirror_session0
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:33:33:44:44:0:0:0:0:0:0:0:2 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:34:34:34:34:0:0:0:0:0:0:0:1 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 25944 6 8 > mirror_session1
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:55:55:66:66:0:0:0:0:0:0:0:2 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:56:56:56:56:0:0:0:0:0:0:0:1 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 25944 6 8 > mirror_session2
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:77:77:88:88:0:0:0:0:0:0:0:2 SAI_IP_ADDR_FAMILY_IPV6 20:01:DB:8:78:78:78:78:0:0:0:0:0:0:0:1 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 25944 6 8 > mirror_session3
back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group0
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_table0
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter0
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter1
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter2
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter3
sai_create_acl_entry 9288674231451648 $acl_table0 10 > acl_entry0
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.20
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0
sai_create_acl_entry 9288674231451648 $acl_table0 20 > acl_entry1
sai_set_acl_entry_attribute $acl_entry1 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.21
sai_set_acl_entry_attribute $acl_entry1 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session1
sai_create_acl_entry 9288674231451648 $acl_table0 30 > acl_entry2
sai_set_acl_entry_attribute $acl_entry2 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.22
sai_set_acl_entry_attribute $acl_entry2 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session2
sai_create_acl_entry 9288674231451648 $acl_table0 40 > acl_entry3
sai_set_acl_entry_attribute $acl_entry3 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.23
sai_set_acl_entry_attribute $acl_entry3 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session3
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group0
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_table0 $acl_table_group0 0 > acl_table_group_member0
home
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP(src="195.169.0.21", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
#[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv6 |<IPv6  version=6 tc=8 fl=0 plen=96 nh=GRE hlim=255 src=2001:db08:1111:2222::2 dst=2001:db08:1212:1212::1 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x1 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=0 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=VLAN |<Dot1Q  prio=0 id=0 vlan=75 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=2048 id=17664 flags= frag=20 ttl=0 proto=icmp chksum=0x0 src=64.0.243.111 dst=195.169.0.21

expectedData = {
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv6 |<IPv6  version=6 tc=8 fl=0 plen=64 nh=GRE hlim=255 src=2001:db08:3333:4444::2 dst=2001:db08:3434:3434::1 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=0 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x6558 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=VLAN |<Dot1Q  prio=0 id=0 vlan=75 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf36f src=195.169.0.21 dst=195.169.0.18
'''
}

