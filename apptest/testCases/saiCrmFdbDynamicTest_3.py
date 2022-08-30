#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmFdbDynamicTest_3',
    'description' : 'TC to verify CRM count after flush of dynamic learned entries',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
xps
fdb
fdb_get_table_depth 0 > max_fdb_cnt
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
back
fdb
sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_ENTRY_TYPE,0
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0

'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan_member $memtap2
sai_remove_vlan $vlan75
back
fdb
sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_BV_ID,$vlan75
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_fdb_cnt',
}
