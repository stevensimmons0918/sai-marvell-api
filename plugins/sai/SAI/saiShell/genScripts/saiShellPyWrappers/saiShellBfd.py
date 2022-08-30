#!/usr/bin/env python
#  saiShellBfd.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellBfd.py
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
# The class object for saiShellBfd operations
#/**********************************************************************************/

class saiShellBfdObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_bfd_session
    #/*********************************************************/
    def do_sai_create_bfd_session(self, arg):

        ''' sai_create_bfd_session '''

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
		    ret = sai_create_bfd_session_default(int(args[0]))
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
		    ret = sai_create_bfd_session_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_bfd_session_attribute
    #/*********************************************************/
    def do_sai_set_bfd_session_attribute(self, arg):

	'''sai_set_bfd_session_attribute: Enter[ bfd_session_id, attr_id, attr_value ]'''

	varStr = 'bfd_session_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'bfd_session_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_BFD_SESSION_ATTR_HW_LOOKUP_VALID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_LOCAL_DISCRIMINATOR'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_STATE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_VLAN_TPID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_VLAN_PRI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_SRC_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_MIN_TX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TUNNEL_TOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_LOCAL_DIAG'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TUNNEL_SRC_IP_ADDRESS'):
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
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_DST_IP_ADDRESS'):
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
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TUNNEL_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_ECHO_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_MIN_RX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_OFFLOAD_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_VLAN_HEADER_VALID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TUNNEL_DST_IP_ADDRESS'):
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
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_REMOTE_MIN_RX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_REMOTE_DIAG'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_MULTIHOP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_REMOTE_MULTIPLIER'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_CBIT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_VLAN_CFI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_VIRTUAL_ROUTER'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_MULTIPLIER'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_SRC_IP_ADDRESS'):
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
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_UDP_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_DST_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_NEGOTIATED_RX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_NEGOTIATED_TX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_BFD_ENCAPSULATION_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_IPHDR_VERSION'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_REMOTE_DISCRIMINATOR'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_BFD_SESSION_ATTR_REMOTE_MIN_TX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_bfd_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_bfd_session_attribute
    #/*********************************************************/
    def do_sai_get_bfd_session_attribute(self, arg):

	'''sai_get_bfd_session_attribute: Enter[  bfd_session_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' bfd_session_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'bfd_session_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_bfd_session_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_bfd_session_stats
    #/*********************************************************/
    def do_sai_get_bfd_session_stats(self, arg):

	'''sai_get_bfd_session_stats: Enter[  bfd_session_id, number_of_counters ]'''

	varStr = ' bfd_session_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'bfd_session_id','number_of_counters' ]
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
		ret = sai_get_bfd_session_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_bfd_session_stats
    #/*********************************************************/
    def do_sai_clear_bfd_session_stats(self, arg):

	'''sai_clear_bfd_session_stats: Enter[  bfd_session_id, number_of_counters ]'''

	varStr = ' bfd_session_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'bfd_session_id','number_of_counters' ]
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
		ret = sai_clear_bfd_session_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_bfd_session_stats_ext
    #/*********************************************************/
    def do_sai_get_bfd_session_stats_ext(self, arg):

	'''sai_get_bfd_session_stats_ext: Enter[  bfd_session_id, number_of_counters, mode ]'''

	varStr = ' bfd_session_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'bfd_session_id','number_of_counters','mode' ]
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
		ret = sai_get_bfd_session_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_remove_bfd_session
    #/*********************************************************/
    def do_sai_remove_bfd_session(self, arg):

	'''sai_remove_bfd_session: Enter[  bfd_session_id ]'''

	varStr = ' bfd_session_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'bfd_session_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_bfd_session(int(args[0]))
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