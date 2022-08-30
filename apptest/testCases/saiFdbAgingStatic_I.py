#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiFdbAgingStatic_I.py',
    'description' : 'TC to verify the aging with static entries already present',
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
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218076 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap2
home
sai
fdb
sai_create_fdb_entry 9288674231451648 [00:00:11:00:21:23] $vlan75 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218076 SAI_FDB_ENTRY_TYPE_STATIC
sai_create_fdb_entry 9288674231451648 [00:00:11:00:21:24] $vlan75 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218078 SAI_FDB_ENTRY_TYPE_STATIC
home
xps
fdb
fdb_set_aging_time 0 60
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''

'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_fdb_cnt - 2',
}



