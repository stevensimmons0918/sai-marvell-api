#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiCrmAclCounterTest',
    'description' : 'TC to check the crm counter for Acl Counter',
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
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 > acl_tbl_id
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER 0 > max_count
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id1
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER 0
sai_remove_acl_counter $acl_counter_id0
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER 0

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_counter $acl_counter_id1
sai_get_acl_table_attribute $acl_tbl_id SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER 0
sai_remove_acl_table $acl_tbl_id
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE 0

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count - 1',
}

