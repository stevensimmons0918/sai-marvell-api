#!/usr/bin/env python
#  saiShellDebugcounter.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellDebugcounter.py
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
# The class object for saiShellDebugcounter operations
#/**********************************************************************************/

class saiShellDebugcounterObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_debug_counter
    #/*********************************************************/
    def do_sai_create_debug_counter(self, arg):

        ''' sai_create_debug_counter '''

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
		    ret = sai_create_debug_counter_default(int(args[0]))
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
		    ret = sai_create_debug_counter_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_debug_counter_attribute
    #/*********************************************************/
    def do_sai_set_debug_counter_attribute(self, arg):

	'''sai_set_debug_counter_attribute: Enter[ debug_counter_id, attr_id, attr_value ]'''

	varStr = 'debug_counter_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'debug_counter_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_DEBUG_COUNTER_ATTR_IN_DROP_REASON_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_debug_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_DEBUG_COUNTER_ATTR_BIND_METHOD'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_debug_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DEBUG_COUNTER_ATTR_INDEX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_debug_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_DEBUG_COUNTER_ATTR_OUT_DROP_REASON_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_debug_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_DEBUG_COUNTER_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_debug_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_debug_counter_attribute
    #/*********************************************************/
    def do_sai_get_debug_counter_attribute(self, arg):

	'''sai_get_debug_counter_attribute: Enter[  debug_counter_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' debug_counter_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'debug_counter_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_debug_counter_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_debug_counter
    #/*********************************************************/
    def do_sai_remove_debug_counter(self, arg):

	'''sai_remove_debug_counter: Enter[  debug_counter_id ]'''

	varStr = ' debug_counter_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'debug_counter_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_debug_counter(int(args[0]))
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