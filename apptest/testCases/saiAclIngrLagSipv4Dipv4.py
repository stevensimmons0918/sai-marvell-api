import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiIngrAclLagSipv4Dipv4',
    'description'    : 'TC to verify Ingress drop Acl Table Entry for Sipv4 and Dipv4',
    'ingressPort'    : ['30'],
    'egressPort'     : [],
    'pktAction'      : 'TRAP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : '',
    'count'          : 1,              # expected data count
    'acl_counter'    : ['acl_counter_id1']
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
back
lag
sai_create_lag 9288674231451648 1 > lag_id_1
sai_create_lag_member 9288674231451648 281474976710685 $lag_id_1 > lag_member_1
sai_create_lag_member 9288674231451648 281474976710686 $lag_id_1 > lag_member_2
back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_tbl_id

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id1

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255.255.255.255 192.168.0.25
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
sai_create_acl_entry 9288674231451648 $acl_tbl_id 11 > acl_entry_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_DST_IP 1 255.255.255.255 192.168.0.26
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id1
back
lag
sai_set_lag_attribute $lag_id_1 SAI_LAG_ATTR_INGRESS_ACL $acl_table_group
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 >acl_table_group_mem

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
back
lag
sai_set_lag_attribute $lag_id_1 SAI_LAG_ATTR_INGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group
back
lag
sai_remove_lag_member $lag_member_1
sai_remove_lag_member $lag_member_2
sai_remove_lag $lag_id_1
back
home

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="192.168.0.25",dst="192.168.0.26")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}

