#!/usr/bin/env python
#  saiShellSegmentroute.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellSegmentroute.py
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
# The class object for saiShellSegmentroute operations
#/**********************************************************************************/

class saiShellSegmentrouteObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_segmentroute_sidlist
    #/*********************************************************/
    def do_sai_create_segmentroute_sidlist(self, arg):

        ''' sai_create_segmentroute_sidlist '''

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
		    ret = sai_create_segmentroute_sidlist_default(int(args[0]))
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
		    ret = sai_create_segmentroute_sidlist_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_segmentroute_sidlist_attribute
    #/*********************************************************/
    def do_sai_set_segmentroute_sidlist_attribute(self, arg):

	'''sai_set_segmentroute_sidlist_attribute: Enter[ segmentroute_sidlist_id, attr_id, attr_value ]'''

	varStr = 'segmentroute_sidlist_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'segmentroute_sidlist_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_SEGMENTROUTE_SIDLIST_ATTR_TLV_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.tlvlist.count = listLen
			list_ptr = new_arr_sai_tlv(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_tlv_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.tlvlist.list = list_ptr
			ret = sai_set_segmentroute_sidlist_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_tlv(list_ptr)
		elif args[1] == eval('SAI_SEGMENTROUTE_SIDLIST_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_segmentroute_sidlist_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SEGMENTROUTE_SIDLIST_ATTR_SEGMENT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.segmentlist.count = listLen
			list_ptr = new_arr_sai_segment(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_segment_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.segmentlist.list = list_ptr
			ret = sai_set_segmentroute_sidlist_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_segment(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_segmentroute_sidlist_attribute
    #/*********************************************************/
    def do_sai_get_segmentroute_sidlist_attribute(self, arg):

	'''sai_get_segmentroute_sidlist_attribute: Enter[  segmentroute_sidlist_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' segmentroute_sidlist_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'segmentroute_sidlist_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_segmentroute_sidlist_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_segmentroute_sidlist
    #/*********************************************************/
    def do_sai_remove_segmentroute_sidlist(self, arg):

	'''sai_remove_segmentroute_sidlist: Enter[  segmentroute_sidlist_id ]'''

	varStr = ' segmentroute_sidlist_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'segmentroute_sidlist_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_segmentroute_sidlist(int(args[0]))
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