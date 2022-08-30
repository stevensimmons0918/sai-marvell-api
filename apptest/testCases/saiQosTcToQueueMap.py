import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosTcToQueueMap',
    'description' : 'Verifies Tc to queue Qos map',
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
sai_create_qos_map 9288674231451648 [0,7:1,7:2,7:3,7:4,7:5,7:6,7:7,7] SAI_QOS_MAP_TYPE_TC_TO_QUEUE > qosmap_id
sai_get_qos_map_attribute $qosmap_id SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST 8
back
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP $qosmap_id
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP 1
dump $qosmap_id
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP 0
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
