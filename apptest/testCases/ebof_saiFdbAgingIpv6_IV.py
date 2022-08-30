#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'ebof_saiFdbAgingIpv6_IV.py',
    'description' : 'TC to verify the aging with ipv6 entries already present',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count' : 1,             # expected data count
    'sleep_time' : 120
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
sai_remove_vlan_member $memtap3
sai_remove_vlan $vlan75
home
sai
nexthop
sai_remove_next_hop $nh0
sai_remove_next_hop $nh1
back
routerinterface
sai_remove_router_interface $rif_i_prt_01
sai_remove_router_interface $rif_i_prt_02
back
virtualrouter
sai_remove_virtual_router $vr_id
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_fdb_cnt',
}

