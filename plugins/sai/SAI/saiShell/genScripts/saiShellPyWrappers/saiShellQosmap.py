#!/usr/bin/env python
#  saiShellQosmap.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellQosmap.py
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
# The class object for saiShellQosmap operations
#/**********************************************************************************/

class saiShellQosmapObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_qos_map
    #/*********************************************************/
    def do_sai_create_qos_map(self, arg):

        ''' sai_create_qos_map '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, map_to_value_list, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','map_to_value_list','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    postList = []
		    nameList = []
		    count = 0
		    if args[1] != "[]":
			    postList = args[1].strip(']').strip('[').split(":")
			    listLen = len(postList)
		    map_to_value_list_ptr = new_sai_qos_map_list_tp()
		    map_to_value_list_ptr.count = listLen
		    list_ptr_arr = new_arr_sai_qos_map(listLen)
		    if ("SAI_QOS_MAP_TYPE" and "TO") not in args[2]:
			    print " %s is Invalid argument" % (args[2])
			    return
		    nameList = args[2].replace("SAI_QOS_MAP_TYPE_","").replace("_","").lower().split("to")
		    nameDict = {'queue':'queue_index', 'pfcpriority':'prio', 'prioritygroup':'pg'}
		    for ix in range(0,len(nameList),1):
			    nameList[ix] = nameList[ix].split('and')
		    for ix in range(0,len(postList),1):
			    postList[ix] = postList[ix].split(',')
		    for ix in range(0,listLen,1):
			    if(len(nameList[0] + nameList[1]) != len(postList[ix])):
				    print " %s is Invalid argument" % (args[1])
				    return
			    list_ptr = arr_sai_qos_map_getitem(list_ptr_arr, ix)
			    for ix1 in range(0,len(postList[ix]),1):
				    if ix1 < len(nameList[0]):
					    for key,value in nameDict.items():
						    if nameList[0][ix1] == key:
							    nameList[0][ix1] = value
					    exec("list_ptr.key." + nameList[0][ix1] + " = "+postList[ix][ix1])
					    count += 1
				    else:
					    for key,value in nameDict.items():
						    if nameList[1][ix1 - count] == key:
							    nameList[1][ix1 - count] = value
					    exec("list_ptr.value." + nameList[1][ix1 - count] + " = "+postList[ix][ix1])
			    count = 0
			    arr_sai_qos_map_setitem(list_ptr_arr, ix, list_ptr)
		    map_to_value_list_ptr.list = list_ptr_arr
		    ret = sai_create_qos_map_default(int(args[0]),map_to_value_list_ptr,eval(args[2]))
		    delete_sai_qos_map_list_tp(map_to_value_list_ptr)
		    delete_arr_sai_qos_map(list_ptr_arr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, map_to_value_list, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','map_to_value_list','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    postList = []
		    nameList = []
		    count = 0
		    if args[1] != "[]":
			    postList = args[1].strip(']').strip('[').split(":")
			    listLen = len(postList)
		    map_to_value_list_ptr = new_sai_qos_map_list_tp()
		    map_to_value_list_ptr.count = listLen
		    list_ptr_arr = new_arr_sai_qos_map(listLen)
		    if ("SAI_QOS_MAP_TYPE" and "TO") not in args[2]:
			    print " %s is Invalid argument" % (args[2])
			    return
		    nameList = args[2].replace("SAI_QOS_MAP_TYPE_","").replace("_","").lower().split("to")
		    nameDict = {'queue':'queue_index', 'pfcpriority':'prio', 'prioritygroup':'pg'}
		    for ix in range(0,len(nameList),1):
			    nameList[ix] = nameList[ix].split('and')
		    for ix in range(0,len(postList),1):
			    postList[ix] = postList[ix].split(',')
		    for ix in range(0,listLen,1):
			    if(len(nameList[0] + nameList[1]) != len(postList[ix])):
				    print " %s is Invalid argument" % (args[1])
				    return
			    list_ptr = arr_sai_qos_map_getitem(list_ptr_arr, ix)
			    for ix1 in range(0,len(postList[ix]),1):
				    if ix1 < len(nameList[0]):
					    for key,value in nameDict.items():
						    if nameList[0][ix1] == key:
							    nameList[0][ix1] = value
					    exec("list_ptr.key." + nameList[0][ix1] + " = "+postList[ix][ix1])
					    count += 1
				    else:
					    for key,value in nameDict.items():
						    if nameList[1][ix1 - count] == key:
							    nameList[1][ix1 - count] = value
					    exec("list_ptr.value." + nameList[1][ix1 - count] + " = "+postList[ix][ix1])
			    count = 0
			    arr_sai_qos_map_setitem(list_ptr_arr, ix, list_ptr)
		    map_to_value_list_ptr.list = list_ptr_arr
		    ret = sai_create_qos_map_custom(int(args[0]),map_to_value_list_ptr,eval(args[2]))
		    delete_sai_qos_map_list_tp(map_to_value_list_ptr)
		    delete_arr_sai_qos_map(list_ptr_arr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_qos_map_attribute
    #/*********************************************************/
    def do_sai_set_qos_map_attribute(self, arg):

	'''sai_set_qos_map_attribute: Enter[ qos_map_id, attr_id, qos_map_type, attr_value ]'''

	varStr = 'qos_map_id, attr_id, qos_map_type, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'qos_map_id','attr_id','qos_map_type','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)
	argsList = re.split(' ',arg)
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
		if args[1] == eval('SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST'):
			postList = []
			count = 0
			if args[3] != "[]":
				postList = args[3].strip(']').strip('[').split(":")
				listLen = len(postList)
			map_to_value_list_ptr = new_sai_qos_map_list_tp()
			map_to_value_list_ptr.count = listLen
			list_ptr_arr = new_arr_sai_qos_map(listLen)
			if ("SAI_QOS_MAP_TYPE" and "TO") not in args[2]:
				print " %s is Invalid argument" % (args[2])
				return
			nameList = args[2]
			nameList = nameList.replace("SAI_QOS_MAP_TYPE_","").replace("_","").lower().split("to")
			nameDict = {'queue':'queue_index', 'pfcpriority':'prio', 'prioritygroup':'pg'}
			for ix in range(0,len(nameList)):
				nameList[ix] = nameList[ix].split('and')
			for ix in range(0,len(postList)):
				postList[ix] = postList[ix].split(',')
			for ix in range(0,listLen):
				if(len(nameList[0] + nameList[1]) != len(postList[ix])):
					print " %s is Invalid argument" % (args[2])
					return
				list_ptr = arr_sai_qos_map_getitem(list_ptr_arr, ix)
				for ix1 in range(0,len(postList[ix]),1):
					if ix1 < len(nameList[0]):
						for key,value in nameDict.items():
							if nameList[0][ix1] == key:
								nameList[0][ix1] = value
						exec("list_ptr.key." + nameList[0][ix1] + " = "+postList[ix][ix1])
						count += 1
					else:
						for key,value in nameDict.items():
							if nameList[1][ix1 - count] == key:
								nameList[1][ix1 - count] = value
						exec("list_ptr.value." + nameList[1][ix1 - count] + " = "+postList[ix][ix1])
				count = 0
				arr_sai_qos_map_setitem(list_ptr_arr, ix, list_ptr)
			map_to_value_list_ptr.list = list_ptr_arr
			attr_value_ptr.qosmap = map_to_value_list_ptr
			ret = sai_set_qos_map_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_sai_qos_map_list_tp(map_to_value_list_ptr)
			delete_arr_sai_qos_map(list_ptr_arr)
		elif args[1] == eval('SAI_QOS_MAP_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_qos_map_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_qos_map_attribute
    #/*********************************************************/
    def do_sai_get_qos_map_attribute(self, arg):

	'''sai_get_qos_map_attribute: Enter[  qos_map_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' qos_map_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'qos_map_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_qos_map_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_qos_map
    #/*********************************************************/
    def do_sai_remove_qos_map(self, arg):

	'''sai_remove_qos_map: Enter[  qos_map_id ]'''

	varStr = ' qos_map_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'qos_map_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_qos_map(int(args[0]))
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