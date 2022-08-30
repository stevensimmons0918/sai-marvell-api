#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiFdbAgingIpv4_III.py',
    'description' : 'TC to verify the  aging with ipv4 entries already present',
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
fdb_set_aging_time 0 60
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

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

#1 mac2me + 1 RIF+ 1 v4 NNbr
expectedData = {
       'expect1':'$max_fdb_cnt - 3',
}


