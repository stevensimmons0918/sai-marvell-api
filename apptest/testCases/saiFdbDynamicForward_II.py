#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiFdbDynamicFwd_I',
    'description' : 'TC to verify the dynamic learning (part-1)',
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
fdb
sai_get_fdb_entry_attribute 9288674231451648 00:00:11:00:11:00 $vlan75 SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID 1
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
'expect1': '16325548649218077' #check the ingress bridge port
}
