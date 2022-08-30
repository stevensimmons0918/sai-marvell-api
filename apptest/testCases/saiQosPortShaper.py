#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosPortShaper',
    'description' : 'Verifies port shaper create,set,get & remove',
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
scheduler
sai_create_scheduler 9288674231451648 > sch_id
sai_set_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE 10000000
sai_set_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE 4096
sai_set_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE 20000000
sai_set_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE 40960
back
scheduler
sai_get_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE 1
sai_get_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE 1
sai_get_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE 1
sai_get_scheduler_attribute $sch_id SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE 1
back
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID $sch_id
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID 1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID 0
back
scheduler
sai_remove_scheduler $sch_id
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'6192449487634437'
}
