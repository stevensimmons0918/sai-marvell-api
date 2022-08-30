#!/usr/bin/env python
#  saiShellPort.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellPort.py
#

import sys
import time
import os
import re
import readline

#/**********************************************************************************/
# import cmd2 package
#/**********************************************************************************/
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd
import xpShellGlobals
import saiShellGlobals
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for saiShellPort operations
#/**********************************************************************************/

class saiShellPortObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_port_serdes
    #/*********************************************************/
    def do_sai_create_port_serdes(self, arg):

        ''' sai_create_port_serdes '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_port_serdes_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_port_serdes_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_port_connector
    #/*********************************************************/
    def do_sai_create_port_connector(self, arg):

        ''' sai_create_port_connector '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_port_connector_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_port_connector_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_port
    #/*********************************************************/
    def do_sai_create_port(self, arg):

        ''' sai_create_port '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, hw_lane_list, full_duplex_mode, speed'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','hw_lane_list','full_duplex_mode','speed' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[1] = args[1].replace(".",":").replace(",",":")
		    args[1] = args[1].strip(',').strip("'")
		    if args[1] != "[]":
			    postList = args[1].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    hw_lane_list_ptr = new_sai_u32_list_tp()
		    hw_lane_list_ptr.count = listLen
		    hw_lane_list_list_ptr = new_arr_sai_u32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_u32_setitem(hw_lane_list_list_ptr, ix, int(postList[ix]))
		    hw_lane_list_ptr.list = hw_lane_list_list_ptr
		    ret = sai_create_port_default(int(args[0]),hw_lane_list_ptr,int(args[2]) ,int(args[3]))
		    delete_sai_u32_list_tp(hw_lane_list_ptr)
		    delete_arr_sai_u32(hw_lane_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, qos_default_tc, ingress_mirror_session, media_type, ingress_samplepacket_enable, admin_state, interface_type, egress_acl, egress_samplepacket_enable, priority_flow_control_mode, hw_lane_list, mtu, priority_flow_control_tx, qos_dscp_to_tc_map, internal_loopback_mode, qos_pfc_priority_to_queue_map, auto_neg_mode, advertised_fec_mode, port_vlan_id, qos_dot1p_to_color_map, broadcast_storm_control_policer_id, ingress_acl, qos_tc_and_color_to_dot1p_map, qos_tc_to_queue_map, qos_tc_and_color_to_dscp_map, multicast_storm_control_policer_id, priority_flow_control_rx, qos_ingress_buffer_profile_list, full_duplex_mode, qos_pfc_priority_to_priority_group_map, qos_dot1p_to_tc_map, egress_mirror_session, speed, pkt_tx_enable, qos_scheduler_profile_id, fec_mode, priority_flow_control, policer_id, qos_tc_to_priority_group_map, qos_egress_buffer_profile_list, flood_storm_control_policer_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','qos_default_tc','ingress_mirror_session','media_type','ingress_samplepacket_enable','admin_state','interface_type','egress_acl','egress_samplepacket_enable','priority_flow_control_mode','hw_lane_list','mtu','priority_flow_control_tx','qos_dscp_to_tc_map','internal_loopback_mode','qos_pfc_priority_to_queue_map','auto_neg_mode','advertised_fec_mode','port_vlan_id','qos_dot1p_to_color_map','broadcast_storm_control_policer_id','ingress_acl','qos_tc_and_color_to_dot1p_map','qos_tc_to_queue_map','qos_tc_and_color_to_dscp_map','multicast_storm_control_policer_id','priority_flow_control_rx','qos_ingress_buffer_profile_list','full_duplex_mode','qos_pfc_priority_to_priority_group_map','qos_dot1p_to_tc_map','egress_mirror_session','speed','pkt_tx_enable','qos_scheduler_profile_id','fec_mode','priority_flow_control','policer_id','qos_tc_to_priority_group_map','qos_egress_buffer_profile_list','flood_storm_control_policer_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[2] = args[2].replace(".",":").replace(",",":")
		    args[2] = args[2].strip(',').strip("'")
		    if args[2] != "[]":
			    postList = args[2].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    ingress_mirror_session_ptr = new_sai_object_list_tp()
		    ingress_mirror_session_ptr.count = listLen
		    ingress_mirror_session_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(ingress_mirror_session_list_ptr, ix, int(postList[ix]))
		    ingress_mirror_session_ptr.list = ingress_mirror_session_list_ptr
		    listLen = 0
		    args[10] = args[10].replace(".",":").replace(",",":")
		    args[10] = args[10].strip(',').strip("'")
		    if args[10] != "[]":
			    postList = args[10].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    hw_lane_list_ptr = new_sai_u32_list_tp()
		    hw_lane_list_ptr.count = listLen
		    hw_lane_list_list_ptr = new_arr_sai_u32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_u32_setitem(hw_lane_list_list_ptr, ix, int(postList[ix]))
		    hw_lane_list_ptr.list = hw_lane_list_list_ptr
		    listLen = 0
		    args[17] = args[17].replace(".",":").replace(",",":")
		    args[17] = args[17].strip(',').strip("'")
		    if args[17] != "[]":
			    postList = args[17].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    advertised_fec_mode_ptr = new_sai_s32_list_tp()
		    advertised_fec_mode_ptr.count = listLen
		    advertised_fec_mode_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(advertised_fec_mode_list_ptr, ix, int(postList[ix]))
		    advertised_fec_mode_ptr.list = advertised_fec_mode_list_ptr
		    listLen = 0
		    args[27] = args[27].replace(".",":").replace(",",":")
		    args[27] = args[27].strip(',').strip("'")
		    if args[27] != "[]":
			    postList = args[27].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    qos_ingress_buffer_profile_list_ptr = new_sai_object_list_tp()
		    qos_ingress_buffer_profile_list_ptr.count = listLen
		    qos_ingress_buffer_profile_list_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(qos_ingress_buffer_profile_list_list_ptr, ix, int(postList[ix]))
		    qos_ingress_buffer_profile_list_ptr.list = qos_ingress_buffer_profile_list_list_ptr
		    listLen = 0
		    args[31] = args[31].replace(".",":").replace(",",":")
		    args[31] = args[31].strip(',').strip("'")
		    if args[31] != "[]":
			    postList = args[31].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    egress_mirror_session_ptr = new_sai_object_list_tp()
		    egress_mirror_session_ptr.count = listLen
		    egress_mirror_session_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(egress_mirror_session_list_ptr, ix, int(postList[ix]))
		    egress_mirror_session_ptr.list = egress_mirror_session_list_ptr
		    listLen = 0
		    args[39] = args[39].replace(".",":").replace(",",":")
		    args[39] = args[39].strip(',').strip("'")
		    if args[39] != "[]":
			    postList = args[39].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    qos_egress_buffer_profile_list_ptr = new_sai_object_list_tp()
		    qos_egress_buffer_profile_list_ptr.count = listLen
		    qos_egress_buffer_profile_list_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(qos_egress_buffer_profile_list_list_ptr, ix, int(postList[ix]))
		    qos_egress_buffer_profile_list_ptr.list = qos_egress_buffer_profile_list_list_ptr
		    ret = sai_create_port_custom(int(args[0]),int(args[1]),ingress_mirror_session_ptr,eval(args[3]),int(args[4]),int(args[5]),eval(args[6]),int(args[7]),int(args[8]),eval(args[9]),hw_lane_list_ptr,int(args[11]),int(args[12]),int(args[13]),eval(args[14]),int(args[15]),int(args[16]),advertised_fec_mode_ptr,int(args[18]),int(args[19]),int(args[20]),int(args[21]),int(args[22]),int(args[23]),int(args[24]),int(args[25]),int(args[26]),qos_ingress_buffer_profile_list_ptr,int(args[28]),int(args[29]),int(args[30]),egress_mirror_session_ptr,int(args[32]),int(args[33]),int(args[34]),eval(args[35]),int(args[36]),int(args[37]),int(args[38]),qos_egress_buffer_profile_list_ptr,int(args[40]))
		    delete_sai_object_list_tp(ingress_mirror_session_ptr)
		    delete_arr_sai_object(ingress_mirror_session_list_ptr)
		    delete_sai_u32_list_tp(hw_lane_list_ptr)
		    delete_arr_sai_u32(hw_lane_list_list_ptr)
		    delete_sai_s32_list_tp(advertised_fec_mode_ptr)
		    delete_arr_sai_s32(advertised_fec_mode_list_ptr)
		    delete_sai_object_list_tp(qos_ingress_buffer_profile_list_ptr)
		    delete_arr_sai_object(qos_ingress_buffer_profile_list_list_ptr)
		    delete_sai_object_list_tp(egress_mirror_session_ptr)
		    delete_arr_sai_object(egress_mirror_session_list_ptr)
		    delete_sai_object_list_tp(qos_egress_buffer_profile_list_ptr)
		    delete_arr_sai_object(qos_egress_buffer_profile_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_port_pool
    #/*********************************************************/
    def do_sai_create_port_pool(self, arg):

        ''' sai_create_port_pool '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_port_pool_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_port_pool_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_port_serdes_attribute
    #/*********************************************************/
    def do_sai_set_port_serdes_attribute(self, arg):

	'''sai_set_port_serdes_attribute: Enter[ port_serdes_id, attr_id, attr_value ]'''

	varStr = 'port_serdes_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_serdes_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_POST3'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_POST2'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_POST1'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_ATTN'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_MAIN'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_IPREDRIVER'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_PREEMPHASIS'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_PRE1'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_PRE3'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_TX_FIR_PRE2'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_SERDES_ATTR_IDRIVER'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_serdes_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_port_connector_attribute
    #/*********************************************************/
    def do_sai_set_port_connector_attribute(self, arg):

	'''sai_set_port_connector_attribute: Enter[ port_connector_id, attr_id, attr_value ]'''

	varStr = 'port_connector_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_connector_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_PORT_CONNECTOR_ATTR_FAILOVER_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_connector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_CONNECTOR_ATTR_SYSTEM_SIDE_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_connector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_CONNECTOR_ATTR_LINE_SIDE_FAILOVER_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_connector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_CONNECTOR_ATTR_SYSTEM_SIDE_FAILOVER_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_connector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_CONNECTOR_ATTR_LINE_SIDE_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_connector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_port_attribute
    #/*********************************************************/
    def do_sai_set_port_attribute(self, arg):

	'''sai_set_port_attribute: Enter[ port_id, attr_id, attr_value ]'''

	varStr = 'port_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_LINK_TRAINING_RX_STATUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EEE_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_TPID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SERDES_IPREDRIVER'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_LINK_TRAINING_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_MPLS_EXP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_AUTO_NEG_STATUS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_LINK_TRAINING_FAILURE_STATUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_MACSEC_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_DEFAULT_TC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INGRESS_MIRROR_SESSION'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EEE_WAKE_TIME'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_DROP_UNTAGGED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_DISABLE_DECREMENT_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_TAM_OBJECT'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_MEDIA_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PTP_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FABRIC_ATTACHED_SWITCH_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADMIN_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FABRIC_ATTACHED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EEE_IDLE_TIME'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INTERFACE_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_UPDATE_DSCP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_SPEED'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_MEDIA_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FABRIC_ATTACHED_PORT_INDEX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SERDES_IDRIVER'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRBS_LOCK_STATUS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SYSTEM_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_HW_LANE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ERR_STATUS_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.porterror.count = listLen
			list_ptr = new_arr_sai_port_err_status(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_port_err_status_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.porterror.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_port_err_status(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_MTU'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_SPEED'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_ASYMMETRIC_PAUSE_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_MPLS_EXP_TO_TC_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EGRESS_MACSEC_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_TX'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_OPER_SPEED'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRBS_CONFIG'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PORT_SERDES_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EYE_VALUES'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.porteyevalues.count = listLen
			list_ptr = new_arr_sai_port_eye_values(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_port_eye_values_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.porteyevalues.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_port_eye_values(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_AUTO_NEG_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_FEC_MODE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ISOLATION_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PORT_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRBS_LOCK_LOSS_STATUS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_META_DATA'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_HW_PROFILE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_FEC_MODE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_INTERFACE_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_OUI_CODE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EGRESS_SAMPLE_MIRROR_SESSION'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_RX'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FULL_DUPLEX_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FABRIC_ATTACHED_SWITCH_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_OUI_CODE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRBS_POLYNOMIAL'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_MEDIA_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REFERENCE_CLOCK'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PORT_POOL_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_QUEUE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_EGRESS_MIRROR_SESSION'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_MEDIA_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_HALF_DUPLEX_SPEED'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INGRESS_SAMPLE_MIRROR_SESSION'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SPEED'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FABRIC_REACHABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.reachability= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_FEC_MODE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SERDES_PREEMPHASIS'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PKT_TX_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_MAXIMUM_HEADROOM_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FEC_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_INGRESS_MACSEC_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_OPER_STATUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_MPLS_EXP_TO_COLOR_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_POLICER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_PRBS_RX_STATUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_DROP_TAGGED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_SUPPORTED_AUTO_NEG_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_port_pool_attribute
    #/*********************************************************/
    def do_sai_set_port_pool_attribute(self, arg):

	'''sai_set_port_pool_attribute: Enter[ port_pool_id, attr_id, attr_value ]'''

	varStr = 'port_pool_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_pool_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_PORT_POOL_ATTR_BUFFER_POOL_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_POOL_ATTR_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_port_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_port_serdes_attribute
    #/*********************************************************/
    def do_sai_get_port_serdes_attribute(self, arg):

	'''sai_get_port_serdes_attribute: Enter[  port_serdes_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' port_serdes_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_serdes_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_port_serdes_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_port_connector_attribute
    #/*********************************************************/
    def do_sai_get_port_connector_attribute(self, arg):

	'''sai_get_port_connector_attribute: Enter[  port_connector_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' port_connector_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_connector_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_port_connector_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_port_attribute
    #/*********************************************************/
    def do_sai_get_port_attribute(self, arg):

	'''sai_get_port_attribute: Enter[  port_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' port_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_port_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_port_pool_attribute
    #/*********************************************************/
    def do_sai_get_port_pool_attribute(self, arg):

	'''sai_get_port_pool_attribute: Enter[  port_pool_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' port_pool_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_pool_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_port_pool_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_port_stats
    #/*********************************************************/
    def do_sai_get_port_stats(self, arg):

	'''sai_get_port_stats: Enter[  port_id, number_of_counters ]'''

	varStr = ' port_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 1)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_port_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 1):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)
    #/*********************************************************/
    # command for sai_clear_port_stats
    #/*********************************************************/
    def do_sai_clear_port_stats(self, arg):

	'''sai_clear_port_stats: Enter[  port_id, number_of_counters ]'''

	varStr = ' port_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_clear_port_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_clear_port_all_stats
    #/*********************************************************/
    def do_sai_clear_port_all_stats(self, arg):

	'''sai_clear_port_all_stats: Enter[ port_id ]'''

	argsList = re.split(',| ',arg)
	args = argsList
	if (argsList[0] == ''):
		print('Invalid input, Enter [ port_id ]')
	else:
		ret = sai_clear_port_all_stats(int(args[0]))
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
    #/*********************************************************/
    # command for sai_get_port_pool_stats
    #/*********************************************************/
    def do_sai_get_port_pool_stats(self, arg):

	'''sai_get_port_pool_stats: Enter[  port_pool_id, number_of_counters ]'''

	varStr = ' port_pool_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_pool_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 1)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_port_pool_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 1):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)
    #/*********************************************************/
    # command for sai_clear_port_pool_stats
    #/*********************************************************/
    def do_sai_clear_port_pool_stats(self, arg):

	'''sai_clear_port_pool_stats: Enter[  port_pool_id, number_of_counters ]'''

	varStr = ' port_pool_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_pool_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_clear_port_pool_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_port_stats_ext
    #/*********************************************************/
    def do_sai_get_port_stats_ext(self, arg):

	'''sai_get_port_stats_ext: Enter[  port_id, number_of_counters, mode ]'''

	varStr = ' port_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_id','number_of_counters','mode' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(r'[;,\s]\s*',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 2)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 2)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 2):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_port_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 2):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)
    #/*********************************************************/
    # command for sai_get_port_pool_stats_ext
    #/*********************************************************/
    def do_sai_get_port_pool_stats_ext(self, arg):

	'''sai_get_port_pool_stats_ext: Enter[  port_pool_id, number_of_counters, mode ]'''

	varStr = ' port_pool_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_pool_id','number_of_counters','mode' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(r'[;,\s]\s*',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 2)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 2)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 2):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_port_pool_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 2):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)

    #/*********************************************************/
    # command for sai_remove_port_serdes
    #/*********************************************************/
    def do_sai_remove_port_serdes(self, arg):

	'''sai_remove_port_serdes: Enter[  port_serdes_id ]'''

	varStr = ' port_serdes_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_serdes_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_port_serdes(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_port_connector
    #/*********************************************************/
    def do_sai_remove_port_connector(self, arg):

	'''sai_remove_port_connector: Enter[  port_connector_id ]'''

	varStr = ' port_connector_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_connector_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_port_connector(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_port
    #/*********************************************************/
    def do_sai_remove_port(self, arg):

	'''sai_remove_port: Enter[  port_id ]'''

	varStr = ' port_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_port(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_port_pool
    #/*********************************************************/
    def do_sai_remove_port_pool(self, arg):

	'''sai_remove_port_pool: Enter[  port_pool_id ]'''

	varStr = ' port_pool_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'port_pool_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_port_pool(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for alter_create_mode
    #/*********************************************************/
    def do_alter_create_mode(self, arg):

        '''alter_create_mode: Enter [ 'create_mode' ] '''

        varStr = 'create_mode'
        varList = varStr.replace(' ', '').split(',')
        optArg = [ 'create_mode' ]
        args = ['0']*len(varList)
        totNumArgs = len(varList)

        argsList = re.split(',| ',arg)
        args = argsList
        if (totNumArgs > 0 and argsList[0] == ''):
            print('Invalid input, Enter [ %s ]' % varStr)
        else:
            xpShellGlobals.cmdRetVal = long(int(args[0]))
            saiShellGlobals.createMode = int(args[0])