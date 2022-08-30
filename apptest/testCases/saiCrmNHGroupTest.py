#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmNHGroupTest',
    'description' : 'TC to verify the CRM count for ECMP group (NH group) entries',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY 0 > max_count
back
nexthopgroup
sai_create_next_hop_group 9288674231451648 0 0 > nhgroup0
sai_create_next_hop_group 9288674231451648 0 0 > nhgroup1
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY 0
back
nexthopgroup
sai_remove_next_hop_group $nhgroup0
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY 0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
nexthopgroup
sai_remove_next_hop_group $nhgroup1
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY 0
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count -1'
}

