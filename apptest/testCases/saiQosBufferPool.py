#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosBufferPool',
    'description' : 'Verifies Buffer pool create,set,get & remove',
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
buffer
sai_create_buffer_pool 9288674231451648 1000000 0 0 > bp_static_ingress
sai_create_buffer_pool 9288674231451648 1000000 1 0 > bp_dynamic_ingress
sai_create_buffer_pool 9288674231451648 1000000 0 1 > bp_static_egress
sai_create_buffer_pool 9288674231451648 1000000 1 1 > bp_dynamic_egress
sai_set_buffer_pool_attribute $bp_static_ingress SAI_BUFFER_POOL_ATTR_XOFF_SIZE 100000
sai_set_buffer_pool_attribute $bp_dynamic_ingress SAI_BUFFER_POOL_ATTR_XOFF_SIZE 100000

sai_get_buffer_pool_attribute $bp_static_ingress SAI_BUFFER_POOL_ATTR_SIZE 1
sai_get_buffer_pool_attribute $bp_dynamic_ingress SAI_BUFFER_POOL_ATTR_SIZE 1
sai_get_buffer_pool_attribute $bp_static_egress SAI_BUFFER_POOL_ATTR_SIZE 1
sai_get_buffer_pool_attribute $bp_dynamic_egress SAI_BUFFER_POOL_ATTR_SIZE 1
sai_get_buffer_pool_attribute $bp_static_ingress SAI_BUFFER_POOL_ATTR_XOFF_SIZE 1
sai_get_buffer_pool_attribute $bp_dynamic_ingress SAI_BUFFER_POOL_ATTR_XOFF_SIZE 1

'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
buffer
sai_remove_buffer_pool $bp_static_ingress
sai_remove_buffer_pool $bp_dynamic_ingress
sai_remove_buffer_pool $bp_static_egress
sai_remove_buffer_pool $bp_dynamic_egress
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'100000'
}
