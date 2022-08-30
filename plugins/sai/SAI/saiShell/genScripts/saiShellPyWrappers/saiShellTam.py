#!/usr/bin/env python
#  saiShellTam.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellTam.py
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
# The class object for saiShellTam operations
#/**********************************************************************************/

class saiShellTamObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_tam_event_threshold
    #/*********************************************************/
    def do_sai_create_tam_event_threshold(self, arg):

        ''' sai_create_tam_event_threshold '''

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
		    ret = sai_create_tam_event_threshold_default(int(args[0]))
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
		    ret = sai_create_tam_event_threshold_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_report
    #/*********************************************************/
    def do_sai_create_tam_report(self, arg):

        ''' sai_create_tam_report '''

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
		    ret = sai_create_tam_report_default(int(args[0]))
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
		    ret = sai_create_tam_report_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_event_action
    #/*********************************************************/
    def do_sai_create_tam_event_action(self, arg):

        ''' sai_create_tam_event_action '''

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
		    ret = sai_create_tam_event_action_default(int(args[0]))
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
		    ret = sai_create_tam_event_action_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_telemetry
    #/*********************************************************/
    def do_sai_create_tam_telemetry(self, arg):

        ''' sai_create_tam_telemetry '''

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
		    ret = sai_create_tam_telemetry_default(int(args[0]))
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
		    ret = sai_create_tam_telemetry_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_tel_type
    #/*********************************************************/
    def do_sai_create_tam_tel_type(self, arg):

        ''' sai_create_tam_tel_type '''

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
		    ret = sai_create_tam_tel_type_default(int(args[0]))
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
		    ret = sai_create_tam_tel_type_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_event
    #/*********************************************************/
    def do_sai_create_tam_event(self, arg):

        ''' sai_create_tam_event '''

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
		    ret = sai_create_tam_event_default(int(args[0]))
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
		    ret = sai_create_tam_event_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_int
    #/*********************************************************/
    def do_sai_create_tam_int(self, arg):

        ''' sai_create_tam_int '''

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
		    ret = sai_create_tam_int_default(int(args[0]))
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
		    ret = sai_create_tam_int_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_transport
    #/*********************************************************/
    def do_sai_create_tam_transport(self, arg):

        ''' sai_create_tam_transport '''

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
		    ret = sai_create_tam_transport_default(int(args[0]))
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
		    ret = sai_create_tam_transport_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam
    #/*********************************************************/
    def do_sai_create_tam(self, arg):

        ''' sai_create_tam '''

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
		    ret = sai_create_tam_default(int(args[0]))
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
		    ret = sai_create_tam_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_collector
    #/*********************************************************/
    def do_sai_create_tam_collector(self, arg):

        ''' sai_create_tam_collector '''

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
		    ret = sai_create_tam_collector_default(int(args[0]))
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
		    ret = sai_create_tam_collector_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tam_math_func
    #/*********************************************************/
    def do_sai_create_tam_math_func(self, arg):

        ''' sai_create_tam_math_func '''

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
		    ret = sai_create_tam_math_func_default(int(args[0]))
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
		    ret = sai_create_tam_math_func_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_tam_event_threshold_attribute
    #/*********************************************************/
    def do_sai_set_tam_event_threshold_attribute(self, arg):

	'''sai_set_tam_event_threshold_attribute: Enter[ tam_event_threshold_id, attr_id, attr_value ]'''

	varStr = 'tam_event_threshold_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_threshold_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_EVENT_THRESHOLD_ATTR_LOW_WATERMARK'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_event_threshold_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_THRESHOLD_ATTR_LATENCY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_event_threshold_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_THRESHOLD_ATTR_RATE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_event_threshold_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_THRESHOLD_ATTR_HIGH_WATERMARK'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_event_threshold_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_THRESHOLD_ATTR_UNIT'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_event_threshold_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_THRESHOLD_ATTR_ABS_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_event_threshold_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_report_attribute
    #/*********************************************************/
    def do_sai_set_tam_report_attribute(self, arg):

	'''sai_set_tam_report_attribute: Enter[ tam_report_id, attr_id, attr_value ]'''

	varStr = 'tam_report_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_report_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_REPORT_ATTR_QUOTA'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_REPORT_ATTR_REPORT_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_REPORT_ATTR_REPORT_INTERVAL'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_REPORT_ATTR_ENTERPRISE_NUMBER'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_REPORT_ATTR_HISTOGRAM_NUMBER_OF_BINS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_REPORT_ATTR_HISTOGRAM_BIN_BOUNDARY'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_TAM_REPORT_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_report_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_event_action_attribute
    #/*********************************************************/
    def do_sai_set_tam_event_action_attribute(self, arg):

	'''sai_set_tam_event_action_attribute: Enter[ tam_event_action_id, attr_id, attr_value ]'''

	varStr = 'tam_event_action_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_action_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_EVENT_ACTION_ATTR_QOS_ACTION_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_event_action_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_ACTION_ATTR_REPORT_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_event_action_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_telemetry_attribute
    #/*********************************************************/
    def do_sai_set_tam_telemetry_attribute(self, arg):

	'''sai_set_tam_telemetry_attribute: Enter[ tam_telemetry_id, attr_id, attr_value ]'''

	varStr = 'tam_telemetry_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_telemetry_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_TELEMETRY_ATTR_REPORTING_INTERVAL'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_telemetry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TELEMETRY_ATTR_TAM_TYPE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_telemetry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TAM_TELEMETRY_ATTR_TAM_REPORTING_UNIT'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_telemetry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TELEMETRY_ATTR_COLLECTOR_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_telemetry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_tel_type_attribute
    #/*********************************************************/
    def do_sai_set_tam_tel_type_attribute(self, arg):

	'''sai_set_tam_tel_type_attribute: Enter[ tam_tel_type_id, attr_id, attr_value ]'''

	varStr = 'tam_tel_type_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_tel_type_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_DSCP_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_INT_SWITCH_IDENTIFIER'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_FABRIC_Q'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_PORT_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_MATH_FUNC'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_TAM_TELEMETRY_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_PORT_STATS_INGRESS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_NE_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_MMU_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_FABRIC_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_PORT_STATS_EGRESS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_VIRTUAL_QUEUE_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_RESOURCE_UTILIZATION_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_OUTPUT_QUEUE_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_SWITCH_ENABLE_FILTER_STATS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TEL_TYPE_ATTR_REPORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_tel_type_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_event_attribute
    #/*********************************************************/
    def do_sai_set_tam_event_attribute(self, arg):

	'''sai_set_tam_event_attribute: Enter[ tam_event_id, attr_id, attr_value ]'''

	varStr = 'tam_event_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_EVENT_ATTR_ACTION_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_ATTR_DSCP_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_ATTR_COLLECTOR_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TAM_EVENT_ATTR_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_event_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_int_attribute
    #/*********************************************************/
    def do_sai_set_tam_int_attribute(self, arg):

	'''sai_set_tam_int_attribute: Enter[ tam_int_id, attr_id, attr_value ]'''

	varStr = 'tam_int_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_int_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_INT_ATTR_INGRESS_SAMPLEPACKET_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_INT_PRESENCE_PB1'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_FLOW_LIVENESS_PERIOD'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_ACL_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_REPORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_MATH_FUNC'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_MAX_HOP_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_REPORT_ALL_PACKETS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_METADATA_CHECKSUM_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_INT_PRESENCE_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_NAME_SPACE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_MAX_LENGTH'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_INT_PRESENCE_L3_PROTOCOL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_COLLECTOR_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_NAME_SPACE_ID_GLOBAL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_P4_INT_INSTRUCTION_BITMAP'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_TRACE_VECTOR'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_DEVICE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_LATENCY_SENSITIVITY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_ACTION_VECTOR'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_METADATA_FRAGMENT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_INLINE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_INT_PRESENCE_PB2'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_INT_PRESENCE_DSCP_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_INT_ATTR_IOAM_TRACE_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_int_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_transport_attribute
    #/*********************************************************/
    def do_sai_set_tam_transport_attribute(self, arg):

	'''sai_set_tam_transport_attribute: Enter[ tam_transport_id, attr_id, attr_value ]'''

	varStr = 'tam_transport_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_transport_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_TRANSPORT_ATTR_TRANSPORT_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_transport_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TRANSPORT_ATTR_TRANSPORT_AUTH_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_transport_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TRANSPORT_ATTR_MTU'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_transport_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TRANSPORT_ATTR_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_transport_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_TRANSPORT_ATTR_DST_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tam_transport_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_attribute
    #/*********************************************************/
    def do_sai_set_tam_attribute(self, arg):

	'''sai_set_tam_attribute: Enter[ tam_id, attr_id, attr_value ]'''

	varStr = 'tam_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_ATTR_EVENT_OBJECTS_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TAM_ATTR_TELEMETRY_OBJECTS_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TAM_ATTR_TAM_BIND_POINT_TYPE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_tam_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_TAM_ATTR_INT_OBJECTS_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tam_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_collector_attribute
    #/*********************************************************/
    def do_sai_set_tam_collector_attribute(self, arg):

	'''sai_set_tam_collector_attribute: Enter[ tam_collector_id, attr_id, attr_value ]'''

	varStr = 'tam_collector_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_collector_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_COLLECTOR_ATTR_DSCP_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_COLLECTOR_ATTR_SRC_IP'):
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
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_COLLECTOR_ATTR_DST_IP'):
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
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_COLLECTOR_ATTR_TRANSPORT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_COLLECTOR_ATTR_TRUNCATE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_COLLECTOR_ATTR_VIRTUAL_ROUTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TAM_COLLECTOR_ATTR_LOCALHOST'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tam_collector_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tam_math_func_attribute
    #/*********************************************************/
    def do_sai_set_tam_math_func_attribute(self, arg):

	'''sai_set_tam_math_func_attribute: Enter[ tam_math_func_id, attr_id, attr_value ]'''

	varStr = 'tam_math_func_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_math_func_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TAM_MATH_FUNC_ATTR_TAM_TEL_MATH_FUNC_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tam_math_func_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_tam_event_threshold_attribute
    #/*********************************************************/
    def do_sai_get_tam_event_threshold_attribute(self, arg):

	'''sai_get_tam_event_threshold_attribute: Enter[  tam_event_threshold_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_event_threshold_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_threshold_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_event_threshold_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_report_attribute
    #/*********************************************************/
    def do_sai_get_tam_report_attribute(self, arg):

	'''sai_get_tam_report_attribute: Enter[  tam_report_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_report_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_report_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_report_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_event_action_attribute
    #/*********************************************************/
    def do_sai_get_tam_event_action_attribute(self, arg):

	'''sai_get_tam_event_action_attribute: Enter[  tam_event_action_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_event_action_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_action_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_event_action_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_telemetry_attribute
    #/*********************************************************/
    def do_sai_get_tam_telemetry_attribute(self, arg):

	'''sai_get_tam_telemetry_attribute: Enter[  tam_telemetry_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_telemetry_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_telemetry_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_telemetry_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_tel_type_attribute
    #/*********************************************************/
    def do_sai_get_tam_tel_type_attribute(self, arg):

	'''sai_get_tam_tel_type_attribute: Enter[  tam_tel_type_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_tel_type_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_tel_type_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_tel_type_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_event_attribute
    #/*********************************************************/
    def do_sai_get_tam_event_attribute(self, arg):

	'''sai_get_tam_event_attribute: Enter[  tam_event_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_event_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_event_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_int_attribute
    #/*********************************************************/
    def do_sai_get_tam_int_attribute(self, arg):

	'''sai_get_tam_int_attribute: Enter[  tam_int_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_int_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_int_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_int_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_transport_attribute
    #/*********************************************************/
    def do_sai_get_tam_transport_attribute(self, arg):

	'''sai_get_tam_transport_attribute: Enter[  tam_transport_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_transport_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_transport_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_transport_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_attribute
    #/*********************************************************/
    def do_sai_get_tam_attribute(self, arg):

	'''sai_get_tam_attribute: Enter[  tam_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_collector_attribute
    #/*********************************************************/
    def do_sai_get_tam_collector_attribute(self, arg):

	'''sai_get_tam_collector_attribute: Enter[  tam_collector_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_collector_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_collector_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_collector_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tam_math_func_attribute
    #/*********************************************************/
    def do_sai_get_tam_math_func_attribute(self, arg):

	'''sai_get_tam_math_func_attribute: Enter[  tam_math_func_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tam_math_func_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_math_func_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tam_math_func_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_event_threshold
    #/*********************************************************/
    def do_sai_remove_tam_event_threshold(self, arg):

	'''sai_remove_tam_event_threshold: Enter[  tam_event_threshold_id ]'''

	varStr = ' tam_event_threshold_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_threshold_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_event_threshold(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_report
    #/*********************************************************/
    def do_sai_remove_tam_report(self, arg):

	'''sai_remove_tam_report: Enter[  tam_report_id ]'''

	varStr = ' tam_report_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_report_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_report(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_event_action
    #/*********************************************************/
    def do_sai_remove_tam_event_action(self, arg):

	'''sai_remove_tam_event_action: Enter[  tam_event_action_id ]'''

	varStr = ' tam_event_action_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_action_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_event_action(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_telemetry
    #/*********************************************************/
    def do_sai_remove_tam_telemetry(self, arg):

	'''sai_remove_tam_telemetry: Enter[  tam_telemetry_id ]'''

	varStr = ' tam_telemetry_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_telemetry_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_telemetry(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_tel_type
    #/*********************************************************/
    def do_sai_remove_tam_tel_type(self, arg):

	'''sai_remove_tam_tel_type: Enter[  tam_tel_type_id ]'''

	varStr = ' tam_tel_type_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_tel_type_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_tel_type(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_event
    #/*********************************************************/
    def do_sai_remove_tam_event(self, arg):

	'''sai_remove_tam_event: Enter[  tam_event_id ]'''

	varStr = ' tam_event_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_event_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_event(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_int
    #/*********************************************************/
    def do_sai_remove_tam_int(self, arg):

	'''sai_remove_tam_int: Enter[  tam_int_id ]'''

	varStr = ' tam_int_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_int_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_int(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_transport
    #/*********************************************************/
    def do_sai_remove_tam_transport(self, arg):

	'''sai_remove_tam_transport: Enter[  tam_transport_id ]'''

	varStr = ' tam_transport_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_transport_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_transport(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam
    #/*********************************************************/
    def do_sai_remove_tam(self, arg):

	'''sai_remove_tam: Enter[  tam_id ]'''

	varStr = ' tam_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_collector
    #/*********************************************************/
    def do_sai_remove_tam_collector(self, arg):

	'''sai_remove_tam_collector: Enter[  tam_collector_id ]'''

	varStr = ' tam_collector_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_collector_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_collector(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tam_math_func
    #/*********************************************************/
    def do_sai_remove_tam_math_func(self, arg):

	'''sai_remove_tam_math_func: Enter[  tam_math_func_id ]'''

	varStr = ' tam_math_func_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tam_math_func_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tam_math_func(int(args[0]))
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