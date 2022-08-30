#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmAclTableEntry',
    'description' : 'TC to check the crm counter for Acl Table Entry',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_tbl_id
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY 0 > max_count

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id1

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 192.168.0.25
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
sai_create_acl_entry 9288674231451648 $acl_tbl_id 11 > acl_entry_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 192.168.0.26
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id1
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 >acl_table_group_mem
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY 0
sai_remove_acl_entry $acl_entry_id0
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY 0

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_entry $acl_entry_id1
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_counter $acl_counter_id1
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY 0
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group
home

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count -1',
}

