#!/usr/bin/env python
#  xpsSr.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

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

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xpsSr operations
#/**********************************************************************************/

class xpsSrObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsSrInitScope
    #/********************************************************************************/
    def do_sr_init_scope(self, arg):
        '''
         xpsSrInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsSrInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrDeInitScope
    #/********************************************************************************/
    def do_sr_de_init_scope(self, arg):
        '''
         xpsSrDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsSrDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrInit
    #/********************************************************************************/
    def do_sr_init(self, arg):
        '''
         xpsSrInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSrInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrDeInit
    #/********************************************************************************/
    def do_sr_de_init(self, arg):
        '''
         xpsSrDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSrDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSrAddDevice
    #/********************************************************************************/
    def do_sr_add_device(self, arg):
        '''
         xpsSrAddDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsSrAddDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSrCreateInterface
    #/********************************************************************************/
    def do_sr_create_interface(self, arg):
        '''
         xpsSrCreateInterface: Enter [ devId,numOfSegments,segment list(comma separated) ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if (len(args) == 2) :
            args[0] = int(args[0])
            srIntfId_Ptr_0 = new_xpsInterfaceId_tp()
            xpsSrhData_t_Ptr = new_xpsSrhData_tp()
            xpsSrhData_t_Ptr.numOfSegments = int(args[1])
            ret = xpsSrCreateInterface(args[0],srIntfId_Ptr_0,xpsSrhData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('srIntfId = %d' % (xpsInterfaceId_tp_value(srIntfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(srIntfId_Ptr_0))
                pass
            delete_xpsInterfaceId_tp(srIntfId_Ptr_0)
            delete_xpsSrhData_tp(xpsSrhData_t_Ptr)


        elif (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfSegments,segment list(comma separated) ]')


        else:
            args[0] = int(args[0])
            srIntfId_Ptr_0 = new_xpsInterfaceId_tp()
            xpsSrhData_t_Ptr = new_xpsSrhData_tp()
            xpsSrhData_t_Ptr.numOfSegments = int(args[1])
            portList = args[2].split(",")
            if len(portList) != int(args[1]) :
                print('segement list provided is either more or less than the numOfSegments. Add comma separated list.')
                print('Input values are : numOfSegments = %d' % int(args[1]))
                print('Segments provided in segment list = %d' % len(portList))
                return

            for segmentList in range(len(portList)):
                ipv6AddrList = portList[segmentList].split(":")
                ipv6AddrList.reverse()
                if len(ipv6AddrList) != 16 :
                    print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                    return
                str1 = 'xpsSrhData_t_Ptr.segment' + str(segmentList) + '[addrOffset]'
                for addrOffset in range(len(ipv6AddrList)):
                    exec("%s = %d" % (str1,int(ipv6AddrList[addrOffset], 16)))
            ret = xpsSrCreateInterface(args[0],srIntfId_Ptr_0,xpsSrhData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('srIntfId = %d' % (xpsInterfaceId_tp_value(srIntfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(srIntfId_Ptr_0))
                pass
            delete_xpsInterfaceId_tp(srIntfId_Ptr_0)
            delete_xpsSrhData_tp(xpsSrhData_t_Ptr)
    #/********************************************************************************/
    # command for xpsSrInterfaceDelete
    #/********************************************************************************/
    def do_sr_interface_delete(self, arg):
        '''
         xpsSrInterfaceDelete: Enter [ devId,srIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,srIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, srIntfId=%d' % (args[0],args[1]))
            ret = xpsSrInterfaceDelete(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrOriginationEntry
    #/********************************************************************************/
    def do_sr_origination_entry(self, arg):
        '''
         xpsSrOriginationEntry: Enter [ devId,srIntId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,srIntId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, srIntId=%d' % (args[0],args[1]))
            ret = xpsSrOriginationEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrTerminationEntry
    #/********************************************************************************/
    def do_sr_termination_entry(self, arg):
        '''
         xpsSrTerminationEntry: Enter [ devId,srIntId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,srIntId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, srIntId=%d' % (args[0],args[1]))
            ret = xpsSrTerminationEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrSetNextHopData
    #/********************************************************************************/
    def do_sr_set_next_hop_data(self, arg):
        '''
         xpsSrSetNextHopData: Enter [ devId,srVifId,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,srVifId,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, srVifId=%d, nhId=%d' % (args[0],args[1],args[2]))
            ret = xpsSrSetNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSrSetLocalSid
    #/********************************************************************************/
    def do_sr_set_local_sid(self, arg):
        '''
         xpsSrSetLocalSid: Enter [ devId,localSid, endFunction, args, vrfId, nhId, ecmpEnable, flag ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,localSid, endFunction, args, vrfId, nhId, ecmpEnable, flag ]')
        else:
            sidEntry = xpSidEntry()
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            localSidList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(localSidList)
            if listLen != 16 :
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return
            for ix in range(listLen-1, -1, -1):
                sidEntry.localSid[listLen - ix - 1] = int(localSidList[ix], 16)
            sidEntry.localSidData.func = eval(args[2])
            sidEntry.localSidData.args = int(args[3])
            sidEntry.localSidData.VRFId = int(args[4])
            sidEntry.localSidData.nhId = int(args[5])
            sidEntry.localSidData.ecmpEnable = int(args[6])
            sidEntry.localSidData.flag = int(args[7])

            #for ix in range(listLen-1, -1, -1):
            #    localSid[listLen - ix - 1] = int(localSidList[ix], 16)
            #print('Input Arguments are, devId=%d, LocalSid=%s' % (args[0],args[1]))
            ret = xpsSrSetLocalSid(args[0],sidEntry)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/*****************************************************************************************************/
    # sub-commands to display tables
    #/*****************************************************************************************************/
    def do_display_tables(self, s):
        'Display Table commands'
        i = displayTableCmds()
        i.prompt = self.prompt[:-1]+':displayTables)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i
