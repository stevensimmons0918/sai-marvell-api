import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L4Packet',
    'tcName' : 'saiSampleMirrorPathACLIngress',
    'description' : 'IACL sample for L4 Dst port Entry',
    'ingressPort' : ['28'],
    'egressPort' : ['29'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap1'],
    'count' : 1,   # expected data count
    'acl_counter'    : ['acl_counter_id1'] #only for acl cases
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.
tcProgramStr = '''
home
sai
mirror
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 0 SAI_IP_ADDR_FAMILY_IPV4 0 0 0 SAI_MIRROR_SESSION_TYPE_SFLOW 281474976710685 0 0 0 0 0 > mirror_session0

back
samplepacket
sai_create_samplepacket 9288674231451648 SAI_SAMPLEPACKET_MODE_SHARED SAI_SAMPLEPACKET_TYPE_MIRROR_SESSION 1 > smpl
sai_set_samplepacket_attribute $smpl SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE 1

back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group0
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_tbl_id0
sai_create_acl_counter 9288674231451648 $acl_tbl_id0 1 1 > acl_counter0
sai_create_acl_entry 9288674231451648 $acl_tbl_id0 10 > acl_entry0
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.44
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE 1 $smpl
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group0
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id0 $acl_table_group0 0 > acl_table_group_member0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
home
sai
acl
sai_get_acl_counter_attribute $acl_counter0 SAI_ACL_COUNTER_ATTR_PACKETS 1

home
sai
acl
sai_remove_acl_entry $acl_entry0
sai_remove_acl_table_group_member $acl_table_group_member0
sai_remove_acl_counter $acl_counter0
sai_remove_acl_table $acl_tbl_id0

back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL 0

back
acl
sai_remove_acl_table_group $acl_table_group0

back
samplepacket
sai_remove_samplepacket $smpl

back
mirror
sai_remove_mirror_session $mirror_session0
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.44", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
#'tap1':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.44 dst=195.169.0.18
expectedData = {
       'expect1':'''
       'tap1':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf358 src=195.169.0.44 dst=195.169.0.18 |
'''
}
