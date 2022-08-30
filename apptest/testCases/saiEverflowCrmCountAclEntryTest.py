#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiEverflowCrmCountAclEntryTest',
    'description' : 'TC to verify CRM count for ACL Entry with mirroring(ERSPAN).',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count' : 1,   # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
mirror
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 2.2.2.2 SAI_IP_ADDR_FAMILY_IPV4 3.3.3.3 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE 281474976710686 00:00:11:22:33:44 00:00:11:22:33:55 25944 4 8 > mirror_session0
back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group0
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_table0
sai_get_acl_table_attribute $acl_table0 SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY 0 > max_count
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter0
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter1
sai_create_acl_counter 9288674231451648 $acl_table0 1 1 > acl_counter2
sai_create_acl_entry 9288674231451648 $acl_table0 10 > acl_entry0
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.20
sai_set_acl_entry_attribute $acl_entry0 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0
sai_create_acl_entry 9288674231451648 $acl_table0 20 > acl_entry1
sai_set_acl_entry_attribute $acl_entry1 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.21
sai_set_acl_entry_attribute $acl_entry1 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0
sai_create_acl_entry 9288674231451648 $acl_table0 30 > acl_entry2
sai_set_acl_entry_attribute $acl_entry2 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.18
sai_set_acl_entry_attribute $acl_entry2 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS 1 $mirror_session0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group0
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_table0 $acl_table_group0 0 > acl_table_group_member0
sai_get_acl_table_attribute $acl_table0 SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY 0

'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_entry $acl_entry0
sai_remove_acl_entry $acl_entry1
sai_remove_acl_entry $acl_entry2
sai_remove_acl_table_group_member $acl_table_group_member0
sai_remove_acl_counter $acl_counter0
sai_remove_acl_counter $acl_counter1
sai_remove_acl_counter $acl_counter2
sai_remove_acl_table $acl_table0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group0
back
mirror
sai_remove_mirror_session $mirror_session0
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count -3'
}

