#!/usr/bin/env python
#  saiShellBuffer.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellBuffer.py
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
# The class object for saiShellBuffer operations
#/**********************************************************************************/

class saiShellBufferObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_ingress_priority_group
    #/*********************************************************/
    def do_sai_create_ingress_priority_group(self, arg):

        ''' sai_create_ingress_priority_group '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, index, port'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','index','port' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_ingress_priority_group_default(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, index, port, buffer_profile'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','index','port','buffer_profile' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_ingress_priority_group_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_buffer_pool
    #/*********************************************************/
    def do_sai_create_buffer_pool(self, arg):

        ''' sai_create_buffer_pool '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, size, threshold_mode, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','size','threshold_mode','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_buffer_pool_default(int(args[0]),int(args[1]),eval(args[2]),eval(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, size, xoff_size, threshold_mode, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','size','xoff_size','threshold_mode','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_buffer_pool_custom(int(args[0]),int(args[1]),int(args[2]),eval(args[3]),eval(args[4]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_buffer_profile
    #/*********************************************************/
    def do_sai_create_buffer_profile(self, arg):

        ''' sai_create_buffer_profile '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, shared_dynamic_th, shared_static_th, reserved_buffer_size, threshold_mode, pool_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','shared_dynamic_th','shared_static_th','reserved_buffer_size','threshold_mode','pool_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_buffer_profile_default(int(args[0]),int(args[1]),int(args[2]),int(args[3]),eval(args[4]),int(args[5]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, xoff_th, shared_dynamic_th, shared_static_th, xon_th, reserved_buffer_size, xon_offset_th, threshold_mode, pool_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','xoff_th','shared_dynamic_th','shared_static_th','xon_th','reserved_buffer_size','xon_offset_th','threshold_mode','pool_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_buffer_profile_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]),int(args[4]),int(args[5]),int(args[6]),eval(args[7]),int(args[8]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_ingress_priority_group_attribute
    #/*********************************************************/
    def do_sai_set_ingress_priority_group_attribute(self, arg):

	'''sai_set_ingress_priority_group_attribute: Enter[ ingress_priority_group_id, attr_id, attr_value ]'''

	varStr = 'ingress_priority_group_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ingress_priority_group_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_INGRESS_PRIORITY_GROUP_ATTR_INDEX'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ingress_priority_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_INGRESS_PRIORITY_GROUP_ATTR_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_ingress_priority_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_INGRESS_PRIORITY_GROUP_ATTR_TAM'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_ingress_priority_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_ingress_priority_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_buffer_pool_attribute
    #/*********************************************************/
    def do_sai_set_buffer_pool_attribute(self, arg):

	'''sai_set_buffer_pool_attribute: Enter[ buffer_pool_id, attr_id, attr_value ]'''

	varStr = 'buffer_pool_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_pool_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_BUFFER_POOL_ATTR_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_POOL_ATTR_XOFF_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_POOL_ATTR_SHARED_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_POOL_ATTR_TAM'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_BUFFER_POOL_ATTR_WRED_PROFILE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_POOL_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_buffer_pool_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_buffer_profile_attribute
    #/*********************************************************/
    def do_sai_set_buffer_profile_attribute(self, arg):

	'''sai_set_buffer_profile_attribute: Enter[ buffer_profile_id, attr_id, attr_value ]'''

	varStr = 'buffer_profile_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_profile_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_BUFFER_PROFILE_ATTR_XOFF_TH'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH'):
			args[2]= int(args[2])
			attr_value_ptr.s8= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_XON_TH'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_RESERVED_BUFFER_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BUFFER_PROFILE_ATTR_POOL_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_buffer_profile_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_ingress_priority_group_attribute
    #/*********************************************************/
    def do_sai_get_ingress_priority_group_attribute(self, arg):

	'''sai_get_ingress_priority_group_attribute: Enter[  ingress_priority_group_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' ingress_priority_group_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ingress_priority_group_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_ingress_priority_group_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_buffer_pool_attribute
    #/*********************************************************/
    def do_sai_get_buffer_pool_attribute(self, arg):

	'''sai_get_buffer_pool_attribute: Enter[  buffer_pool_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' buffer_pool_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_pool_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_buffer_pool_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_buffer_profile_attribute
    #/*********************************************************/
    def do_sai_get_buffer_profile_attribute(self, arg):

	'''sai_get_buffer_profile_attribute: Enter[  buffer_profile_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' buffer_profile_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_profile_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_buffer_profile_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_ingress_priority_group_stats
    #/*********************************************************/
    def do_sai_get_ingress_priority_group_stats(self, arg):

	'''sai_get_ingress_priority_group_stats: Enter[  ingress_priority_group_id, number_of_counters ]'''

	varStr = ' ingress_priority_group_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ingress_priority_group_id','number_of_counters' ]
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
		ret = sai_get_ingress_priority_group_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_ingress_priority_group_stats
    #/*********************************************************/
    def do_sai_clear_ingress_priority_group_stats(self, arg):

	'''sai_clear_ingress_priority_group_stats: Enter[  ingress_priority_group_id, number_of_counters ]'''

	varStr = ' ingress_priority_group_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ingress_priority_group_id','number_of_counters' ]
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
		ret = sai_clear_ingress_priority_group_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_buffer_pool_stats
    #/*********************************************************/
    def do_sai_get_buffer_pool_stats(self, arg):

	'''sai_get_buffer_pool_stats: Enter[  buffer_pool_id, number_of_counters ]'''

	varStr = ' buffer_pool_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_pool_id','number_of_counters' ]
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
		ret = sai_get_buffer_pool_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_buffer_pool_stats
    #/*********************************************************/
    def do_sai_clear_buffer_pool_stats(self, arg):

	'''sai_clear_buffer_pool_stats: Enter[  buffer_pool_id, number_of_counters ]'''

	varStr = ' buffer_pool_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_pool_id','number_of_counters' ]
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
		ret = sai_clear_buffer_pool_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_ingress_priority_group_stats_ext
    #/*********************************************************/
    def do_sai_get_ingress_priority_group_stats_ext(self, arg):

	'''sai_get_ingress_priority_group_stats_ext: Enter[  ingress_priority_group_id, number_of_counters, mode ]'''

	varStr = ' ingress_priority_group_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ingress_priority_group_id','number_of_counters','mode' ]
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
		ret = sai_get_ingress_priority_group_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_get_buffer_pool_stats_ext
    #/*********************************************************/
    def do_sai_get_buffer_pool_stats_ext(self, arg):

	'''sai_get_buffer_pool_stats_ext: Enter[  buffer_pool_id, number_of_counters, mode ]'''

	varStr = ' buffer_pool_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_pool_id','number_of_counters','mode' ]
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
		ret = sai_get_buffer_pool_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_remove_ingress_priority_group
    #/*********************************************************/
    def do_sai_remove_ingress_priority_group(self, arg):

	'''sai_remove_ingress_priority_group: Enter[  ingress_priority_group_id ]'''

	varStr = ' ingress_priority_group_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ingress_priority_group_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_ingress_priority_group(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_buffer_pool
    #/*********************************************************/
    def do_sai_remove_buffer_pool(self, arg):

	'''sai_remove_buffer_pool: Enter[  buffer_pool_id ]'''

	varStr = ' buffer_pool_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_pool_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_buffer_pool(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_buffer_profile
    #/*********************************************************/
    def do_sai_remove_buffer_profile(self, arg):

	'''sai_remove_buffer_profile: Enter[  buffer_profile_id ]'''

	varStr = ' buffer_profile_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'buffer_profile_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_buffer_profile(int(args[0]))
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