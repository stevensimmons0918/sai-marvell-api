#!/usr/bin/env python
#  saiShellDtel.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellDtel.py
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
# The class object for saiShellDtel operations
#/**********************************************************************************/

class saiShellDtelObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_dtel_int_session
    #/*********************************************************/
    def do_sai_create_dtel_int_session(self, arg):

        ''' sai_create_dtel_int_session '''

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
		    ret = sai_create_dtel_int_session_default(int(args[0]))
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
		    ret = sai_create_dtel_int_session_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_dtel_event
    #/*********************************************************/
    def do_sai_create_dtel_event(self, arg):

        ''' sai_create_dtel_event '''

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
		    ret = sai_create_dtel_event_default(int(args[0]))
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
		    ret = sai_create_dtel_event_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_dtel_queue_report
    #/*********************************************************/
    def do_sai_create_dtel_queue_report(self, arg):

        ''' sai_create_dtel_queue_report '''

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
		    ret = sai_create_dtel_queue_report_default(int(args[0]))
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
		    ret = sai_create_dtel_queue_report_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_dtel_report_session
    #/*********************************************************/
    def do_sai_create_dtel_report_session(self, arg):

        ''' sai_create_dtel_report_session '''

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
		    ret = sai_create_dtel_report_session_default(int(args[0]))
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
		    ret = sai_create_dtel_report_session_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_dtel
    #/*********************************************************/
    def do_sai_create_dtel(self, arg):

        ''' sai_create_dtel '''

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
		    ret = sai_create_dtel_default(int(args[0]))
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
		    ret = sai_create_dtel_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_dtel_int_session_attribute
    #/*********************************************************/
    def do_sai_set_dtel_int_session_attribute(self, arg):

	'''sai_set_dtel_int_session_attribute: Enter[ dtel_int_session_id, attr_id, attr_value ]'''

	varStr = 'dtel_int_session_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_int_session_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_int_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_INT_SESSION_ATTR_COLLECT_EGRESS_TIMESTAMP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_int_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_INT_SESSION_ATTR_COLLECT_INGRESS_TIMESTAMP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_int_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_INT_SESSION_ATTR_MAX_HOP_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_int_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_INT_SESSION_ATTR_COLLECT_QUEUE_INFO'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_int_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_int_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_dtel_event_attribute
    #/*********************************************************/
    def do_sai_set_dtel_event_attribute(self, arg):

	'''sai_set_dtel_event_attribute: Enter[ dtel_event_id, attr_id, attr_value ]'''

	varStr = 'dtel_event_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_event_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_DTEL_EVENT_ATTR_DSCP_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_EVENT_ATTR_REPORT_SESSION'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_dtel_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_EVENT_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_dtel_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_dtel_queue_report_attribute
    #/*********************************************************/
    def do_sai_set_dtel_queue_report_attribute(self, arg):

	'''sai_set_dtel_queue_report_attribute: Enter[ dtel_queue_report_id, attr_id, attr_value ]'''

	varStr = 'dtel_queue_report_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_queue_report_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_DTEL_QUEUE_REPORT_ATTR_QUEUE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_dtel_queue_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_QUEUE_REPORT_ATTR_DEPTH_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_dtel_queue_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_QUEUE_REPORT_ATTR_BREACH_QUOTA'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_dtel_queue_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_QUEUE_REPORT_ATTR_LATENCY_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_dtel_queue_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_QUEUE_REPORT_ATTR_TAIL_DROP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_queue_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_dtel_report_session_attribute
    #/*********************************************************/
    def do_sai_set_dtel_report_session_attribute(self, arg):

	'''sai_set_dtel_report_session_attribute: Enter[ dtel_report_session_id, attr_id, attr_value ]'''

	varStr = 'dtel_report_session_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_report_session_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_DTEL_REPORT_SESSION_ATTR_TRUNCATE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_dtel_report_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_REPORT_SESSION_ATTR_UDP_DST_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_dtel_report_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_REPORT_SESSION_ATTR_VIRTUAL_ROUTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_dtel_report_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_REPORT_SESSION_ATTR_SRC_IP'):
			ipaddrflag = 1
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			ipaddr_ptr.addr_family = eval(args[2])
			base = 10
			if listLen > 4:
				base = 16
			for ix in range(0, listLen, 1):
				ipaddr_ptr.addr.ip6[ix] = int(postList[ix], base)
			ret = sai_set_dtel_report_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_REPORT_SESSION_ATTR_DST_IP_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.ipaddrlist.count = listLen
			list_ptr = new_arr_sai_ip_address(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_ip_address_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.ipaddrlist.list = list_ptr
			ret = sai_set_dtel_report_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_ip_address(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_dtel_attribute
    #/*********************************************************/
    def do_sai_set_dtel_attribute(self, arg):

	'''sai_set_dtel_attribute: Enter[ dtel_id, attr_id, attr_value ]'''

	varStr = 'dtel_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_DTEL_ATTR_POSTCARD_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_DROP_REPORT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_INT_TRANSIT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_FLOW_STATE_CLEAR_CYCLE'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_INT_ENDPOINT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_INT_L4_DSCP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_LATENCY_SENSITIVITY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_QUEUE_REPORT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_SWITCH_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DTEL_ATTR_SINK_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_dtel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_dtel_int_session_attribute
    #/*********************************************************/
    def do_sai_get_dtel_int_session_attribute(self, arg):

	'''sai_get_dtel_int_session_attribute: Enter[  dtel_int_session_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' dtel_int_session_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_int_session_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_dtel_int_session_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_dtel_event_attribute
    #/*********************************************************/
    def do_sai_get_dtel_event_attribute(self, arg):

	'''sai_get_dtel_event_attribute: Enter[  dtel_event_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' dtel_event_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_event_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_dtel_event_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_dtel_queue_report_attribute
    #/*********************************************************/
    def do_sai_get_dtel_queue_report_attribute(self, arg):

	'''sai_get_dtel_queue_report_attribute: Enter[  dtel_queue_report_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' dtel_queue_report_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_queue_report_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_dtel_queue_report_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_dtel_report_session_attribute
    #/*********************************************************/
    def do_sai_get_dtel_report_session_attribute(self, arg):

	'''sai_get_dtel_report_session_attribute: Enter[  dtel_report_session_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' dtel_report_session_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_report_session_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_dtel_report_session_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_dtel_attribute
    #/*********************************************************/
    def do_sai_get_dtel_attribute(self, arg):

	'''sai_get_dtel_attribute: Enter[  dtel_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' dtel_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_dtel_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_dtel_int_session
    #/*********************************************************/
    def do_sai_remove_dtel_int_session(self, arg):

	'''sai_remove_dtel_int_session: Enter[  dtel_int_session_id ]'''

	varStr = ' dtel_int_session_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_int_session_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_dtel_int_session(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_dtel_event
    #/*********************************************************/
    def do_sai_remove_dtel_event(self, arg):

	'''sai_remove_dtel_event: Enter[  dtel_event_id ]'''

	varStr = ' dtel_event_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_event_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_dtel_event(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_dtel_queue_report
    #/*********************************************************/
    def do_sai_remove_dtel_queue_report(self, arg):

	'''sai_remove_dtel_queue_report: Enter[  dtel_queue_report_id ]'''

	varStr = ' dtel_queue_report_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_queue_report_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_dtel_queue_report(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_dtel_report_session
    #/*********************************************************/
    def do_sai_remove_dtel_report_session(self, arg):

	'''sai_remove_dtel_report_session: Enter[  dtel_report_session_id ]'''

	varStr = ' dtel_report_session_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_report_session_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_dtel_report_session(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_dtel
    #/*********************************************************/
    def do_sai_remove_dtel(self, arg):

	'''sai_remove_dtel: Enter[  dtel_id ]'''

	varStr = ' dtel_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'dtel_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_dtel(int(args[0]))
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