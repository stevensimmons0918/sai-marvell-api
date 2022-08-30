import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiIngrAclParallelLookup',
    'description'    : 'TC to verify Ingress drop Acl Table Entry for Parallel lookup',
    'ingressPort'    : ['29'],
    'egressPort'     : ['30'],
    'vlan'           : '75',
    'pktAction'      : 'TRAP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : [''],
    'count'          : 1,             # expected data count
    'acl_counter'    : ['acl_counter_id0','acl_counter_id1','acl_counter_id2','acl_counter_id3'] #only for acl cases
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 > acl_tbl_id0

sai_create_acl_counter 9288674231451648 $acl_tbl_id0 1 1 > acl_counter_id0

sai_create_acl_entry 9288674231451648 $acl_tbl_id0 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT 1 65535 2456
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0

sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 > acl_tbl_id1

sai_create_acl_counter 9288674231451648 $acl_tbl_id1 1 1 > acl_counter_id1

sai_create_acl_entry 9288674231451648 $acl_tbl_id1 11 > acl_entry_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT 1 65535 2458
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id1

sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_tbl_id2

sai_create_acl_counter 9288674231451648 $acl_tbl_id2 1 1 > acl_counter_id2

sai_create_acl_entry 9288674231451648 $acl_tbl_id2 12 > acl_entry_id2
sai_set_acl_entry_attribute $acl_entry_id2 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255.255.255.255 100.1.1.1
sai_set_acl_entry_attribute $acl_entry_id2 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id2 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id2

sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 > acl_tbl_id3

sai_create_acl_counter 9288674231451648 $acl_tbl_id3 1 1 > acl_counter_id3

sai_create_acl_entry 9288674231451648 $acl_tbl_id3 11 > acl_entry_id3
sai_set_acl_entry_attribute $acl_entry_id3 SAI_ACL_ENTRY_ATTR_FIELD_DST_IP 1 255.255.255.255 100.1.1.2
sai_set_acl_entry_attribute $acl_entry_id3 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id3 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id3

back
lag
sai_create_lag 928867423145164 1 > lag0
sai_create_lag_member 928867423145164 281474976710684 $lag0 >lag_member_0
sai_create_lag_member 928867423145164 281474976710685 $lag0 >lag_member_1
sai_set_lag_attribute $lag0 SAI_LAG_ATTR_INGRESS_ACL $acl_table_group
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id0 $acl_table_group 0 >acl_table_group_mem1
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id1 $acl_table_group 0 >acl_table_group_mem2
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id2 $acl_table_group 0 >acl_table_group_mem3
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id3 $acl_table_group 0 >acl_table_group_mem4

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_entry $acl_entry_id1
sai_remove_acl_counter $acl_counter_id1
sai_remove_acl_entry $acl_entry_id2
sai_remove_acl_counter $acl_counter_id2
sai_remove_acl_entry $acl_entry_id3
sai_remove_acl_counter $acl_counter_id3
sai_remove_acl_table_group_member $acl_table_group_mem1
sai_remove_acl_table_group_member $acl_table_group_mem2
sai_remove_acl_table_group_member $acl_table_group_mem3
sai_remove_acl_table_group_member $acl_table_group_mem4
sai_remove_acl_table $acl_tbl_id0
sai_remove_acl_table $acl_tbl_id1
sai_remove_acl_table $acl_tbl_id2
sai_remove_acl_table $acl_tbl_id3
back
lag
sai_set_lag_attribute $lag0 SAI_LAG_ATTR_INGRESS_ACL 0
sai_remove_lag_member $lag_member_0
sai_remove_lag_member $lag_member_1
back
acl
sai_remove_acl_table_group $acl_table_group
home

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP(src="100.1.1.1",dst="100.1.1.2")/scapy_t.TCP(sport=2456,dport=2458)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'',
}

