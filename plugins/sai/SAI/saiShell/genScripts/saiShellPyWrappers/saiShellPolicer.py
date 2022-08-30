#!/usr/bin/env python
#  saiShellPolicer.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellPolicer.py
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
# The class object for saiShellPolicer operations
#/**********************************************************************************/

class saiShellPolicerObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_policer
    #/*********************************************************/
    def do_sai_create_policer(self, arg):

        ''' sai_create_policer '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, meter_type, mode, color_source'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','meter_type','mode','color_source' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    color_source = 'SAI_POLICER_COLOR_SOURCE_AWARE'
		    if len(args) >= totNumArgs:
			    color_source = args[3]
		    ret = sai_create_policer_default(int(args[0]),eval(args[1]),eval(args[2]),eval(color_source))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, meter_type, cir, yellow_packet_action, mode, enable_counter_packet_action_list, color_source, green_packet_action, red_packet_action, cbs, pbs, pir'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','meter_type','cir','yellow_packet_action','mode','enable_counter_packet_action_list','color_source','green_packet_action','red_packet_action','cbs','pbs','pir' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[5] = args[5].replace(".",":").replace(",",":")
		    args[5] = args[5].strip(',').strip("'")
		    if args[5] != "[]":
			    postList = args[5].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    enable_counter_packet_action_list_ptr = new_sai_s32_list_tp()
		    enable_counter_packet_action_list_ptr.count = listLen
		    enable_counter_packet_action_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(enable_counter_packet_action_list_list_ptr, ix, int(postList[ix]))
		    enable_counter_packet_action_list_ptr.list = enable_counter_packet_action_list_list_ptr
		    ret = sai_create_policer_custom(int(args[0]),eval(args[1]),int(args[2]),eval(args[3]),eval(args[4]),enable_counter_packet_action_list_ptr,eval(args[6]),eval(args[7]),eval(args[8]),int(args[9]),int(args[10]),int(args[11]))
		    delete_sai_s32_list_tp(enable_counter_packet_action_list_ptr)
		    delete_arr_sai_s32(enable_counter_packet_action_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_policer_attribute
    #/*********************************************************/
    def do_sai_set_policer_attribute(self, arg):

	'''sai_set_policer_attribute: Enter[ policer_id, attr_id, attr_value ]'''

	varStr = 'policer_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'policer_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_POLICER_ATTR_METER_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_CIR'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_YELLOW_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_ENABLE_COUNTER_PACKET_ACTION_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_COLOR_SOURCE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_GREEN_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_RED_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_CBS'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_PBS'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_POLICER_ATTR_PIR'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_policer_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_policer_attribute
    #/*********************************************************/
    def do_sai_get_policer_attribute(self, arg):

	'''sai_get_policer_attribute: Enter[  policer_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' policer_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'policer_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_policer_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_policer_stats
    #/*********************************************************/
    def do_sai_get_policer_stats(self, arg):

	'''sai_get_policer_stats: Enter[  policer_id, number_of_counters ]'''

	varStr = ' policer_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'policer_id','number_of_counters' ]
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
		ret = sai_get_policer_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_policer_stats
    #/*********************************************************/
    def do_sai_clear_policer_stats(self, arg):

	'''sai_clear_policer_stats: Enter[  policer_id, number_of_counters ]'''

	varStr = ' policer_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'policer_id','number_of_counters' ]
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
		ret = sai_clear_policer_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_policer_stats_ext
    #/*********************************************************/
    def do_sai_get_policer_stats_ext(self, arg):

	'''sai_get_policer_stats_ext: Enter[  policer_id, number_of_counters, mode ]'''

	varStr = ' policer_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'policer_id','number_of_counters','mode' ]
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
		ret = sai_get_policer_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_remove_policer
    #/*********************************************************/
    def do_sai_remove_policer(self, arg):

	'''sai_remove_policer: Enter[  policer_id ]'''

	varStr = ' policer_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'policer_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_policer(int(args[0]))
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