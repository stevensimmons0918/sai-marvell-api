#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmFdbStaticTest',
    'description' : 'TC to check the crm counter after Static FDB entry',
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
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0 > max_count
back
vlan
sai_create_vlan 9288674231451648 100 > vlan100
sai_create_vlan 9288674231451648 101 > vlan101
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218048 > vlan100_mem0
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218056 > vlan100_mem1
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218064 > vlan100_mem2
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218072 > vlan100_mem3
sai_create_vlan_member 9288674231451648 $vlan101 16325548649218048 > vlan101_mem0
sai_create_vlan_member 9288674231451648 $vlan101 16325548649218056 > vlan101_mem1
sai_create_vlan_member 9288674231451648 $vlan101 16325548649218064 > vlan101_mem2
sai_create_vlan_member 9288674231451648 $vlan101 16325548649218072 > vlan101_mem3
back
fdb
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:22] $vlan100 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218072 SAI_FDB_ENTRY_TYPE_STATIC
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:23] $vlan100 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218072 SAI_FDB_ENTRY_TYPE_STATIC
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:24] $vlan100 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218072 SAI_FDB_ENTRY_TYPE_STATIC
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:22] $vlan101 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218072 SAI_FDB_ENTRY_TYPE_STATIC
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:23] $vlan101 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218072 SAI_FDB_ENTRY_TYPE_STATIC
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:24] $vlan101 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218072 SAI_FDB_ENTRY_TYPE_STATIC
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
back
fdb
sai_remove_fdb_entry 9288674231451648 00:00:00:00:00:22 $vlan101
back
switch
#sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
#back
#fdb
#sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_BV_ID,$vlan100
#back
#switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
fdb
#sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_BV_ID,$vlan101
sai_remove_fdb_entry 9288674231451648 00:00:00:00:00:22 $vlan100
sai_remove_fdb_entry 9288674231451648 00:00:00:00:00:23 $vlan100
sai_remove_fdb_entry 9288674231451648 00:00:00:00:00:24 $vlan100
sai_remove_fdb_entry 9288674231451648 00:00:00:00:00:23 $vlan101
sai_remove_fdb_entry 9288674231451648 00:00:00:00:00:24 $vlan101
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
back
vlan
sai_remove_vlan_member $vlan100_mem0
sai_remove_vlan_member $vlan100_mem1
sai_remove_vlan_member $vlan100_mem2
sai_remove_vlan_member $vlan100_mem3
sai_remove_vlan_member $vlan101_mem0
sai_remove_vlan_member $vlan101_mem1
sai_remove_vlan_member $vlan101_mem2
sai_remove_vlan_member $vlan101_mem3
sai_remove_vlan $vlan100
sai_remove_vlan $vlan101

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count - 5',
}

