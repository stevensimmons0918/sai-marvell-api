#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiFdbCreateGetPktAction',
    'description' : 'TC to get the packet action after creating the fdb entry',
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
vlan
sai_create_vlan 9288674231451648 100 > vlan100
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218053 > vlanmem40
back
fdb
sai_create_fdb_entry 9288674231451648 [00:00:00:00:00:49] $vlan100 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218053 SAI_FDB_ENTRY_TYPE_STATIC
sai_get_fdb_entry_attribute 9288674231451648 [00:00:00:00:00:49] $vlan100 SAI_FDB_ENTRY_ATTR_PACKET_ACTION 1
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
fdb
sai_remove_fdb_entry 9288674231451648 [00:00:00:00:00:49] $vlan100
back
vlan
sai_remove_vlan_member $vlanmem40
sai_remove_vlan $vlan100
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'1',
}

