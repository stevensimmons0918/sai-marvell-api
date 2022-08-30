#!/usr/bin/env python
#  saiShellWred.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellWred.py
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
# The class object for saiShellWred operations
#/**********************************************************************************/

class saiShellWredObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_wred
    #/*********************************************************/
    def do_sai_create_wred(self, arg):

        ''' sai_create_wred '''

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
		    ret = sai_create_wred_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, red_max_threshold, yellow_drop_probability, red_drop_probability, green_drop_probability, green_enable, ecn_mark_mode, yellow_min_threshold, green_min_threshold, green_max_threshold, red_enable, ecn_color_unaware_mark_probability, yellow_max_threshold, yellow_enable, red_min_threshold'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','red_max_threshold','yellow_drop_probability','red_drop_probability','green_drop_probability','green_enable','ecn_mark_mode','yellow_min_threshold','green_min_threshold','green_max_threshold','red_enable','ecn_color_unaware_mark_probability','yellow_max_threshold','yellow_enable','red_min_threshold' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_wred_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]),int(args[4]),int(args[5]),eval(args[6]),int(args[7]),int(args[8]),int(args[9]),int(args[10]),int(args[11]),int(args[12]),int(args[13]),int(args[14]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_wred_attribute
    #/*********************************************************/
    def do_sai_set_wred_attribute(self, arg):

	'''sai_set_wred_attribute: Enter[ wred_id, attr_id, attr_value ]'''

	varStr = 'wred_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'wred_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_WRED_ATTR_ECN_YELLOW_MARK_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_GREEN_MARK_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_RED_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_COLOR_UNAWARE_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_COLOR_UNAWARE_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_GREEN_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_RED_DROP_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_GREEN_DROP_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_GREEN_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_RED_MARK_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_GREEN_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_MARK_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_RED_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_GREEN_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_RED_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_YELLOW_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_YELLOW_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_GREEN_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_RED_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_ECN_COLOR_UNAWARE_MARK_PROBABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_WEIGHT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_YELLOW_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_WRED_ATTR_RED_MIN_THRESHOLD'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_wred_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_wred_attribute
    #/*********************************************************/
    def do_sai_get_wred_attribute(self, arg):

	'''sai_get_wred_attribute: Enter[  wred_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' wred_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'wred_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_wred_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_wred
    #/*********************************************************/
    def do_sai_remove_wred(self, arg):

	'''sai_remove_wred: Enter[  wred_id ]'''

	varStr = ' wred_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'wred_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_wred(int(args[0]))
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