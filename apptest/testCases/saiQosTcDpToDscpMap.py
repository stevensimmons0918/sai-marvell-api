#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosTcDpToDscpMap',
    'description' : 'Verifies Tc, dp to dscp Qos map',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'ingressTapIntf' : '',
    'egressTapIntf'  : [''],
    'count' : 1,             # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
qosmap
sai_create_qos_map 9288674231451648 [0,0,60:0,1,60:0,2,60:1,0,50:1,1,50:1,2,50:2,0,40:2,1,40:2,2,40:3,0,30:3,1,30:3,2,30:4,0,20:4,1,20:4,2,20:5,0,10:5,1,10:5,2,10:6,0,5:6,1,5:6,2,5:7,0,63:7,1,63:7,2,63] SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP > qosmap_id
sai_get_qos_map_attribute $qosmap_id SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST 24
back
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP $qosmap_id
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP 1
dump $qosmap_id
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP 0
back
qosmap
sai_remove_qos_map $qosmap_id
back
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'5629499534213121'
}
