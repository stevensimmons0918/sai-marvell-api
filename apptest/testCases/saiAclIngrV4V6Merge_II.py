import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L3Packet',
    'tcName'         : 'saiIngrAclV4V6Merge_II',
    'description'    : 'TC to verify IACL trap for V4/V6 Entry',
    'ingressPort'    : ['29'],
    'egressPort'     : [],
    'vlan'           : '75',
    'pktAction'      : 'TRAP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : '',
    'count'          : 1 ,             # expected data count
    'acl_counter'    : ['acl_counter_id1'] #only for acl cases
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
acl
sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id1
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6 1 255:255:255:255:255:255:255:255:255:255:255:255:255:255:255:255 02:00:00:01:00:00:00:00:00:00:00:00:00:00:00:08
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE 1 65535 34525
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS 1 281474976710685
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id1

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_entry $acl_entry_id1
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_counter $acl_counter_id1
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
sai_remove_acl_table_group $acl_table_group
home

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IPv6(src="0200:0001:0:0:0:0:0:0008",dst="0200:0001:0:0:0:0:0:0009")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}

