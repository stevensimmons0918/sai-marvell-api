#!/usr/bin/env python
#  xpsFdb.py
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
# The class object for xpsFdb operations
#/**********************************************************************************/

class xpsFdbObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsFdbInit
    #/********************************************************************************/
    def do_fdb_init(self, arg):
        '''
         xpsFdbInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsFdbInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbInitScope
    #/********************************************************************************/
    def do_fdb_init_scope(self, arg):
        '''
         xpsFdbInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsFdbInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbDeInit
    #/********************************************************************************/
    def do_fdb_de_init(self, arg):
        '''
         xpsFdbDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsFdbDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbDeInitScope
    #/********************************************************************************/
    def do_fdb_de_init_scope(self, arg):
        '''
         xpsFdbDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsFdbDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsFdbAddDevice
    #/********************************************************************************/
    def do_fdb_add_device(self, arg):
        '''
         xpsFdbAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsFdbAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbRemoveDevice
    #/********************************************************************************/
    def do_fdb_remove_device(self, arg):
        '''
         xpsFdbRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbTriggerAging
    #/********************************************************************************/
    def do_fdb_trigger_aging(self, arg):
        '''
         xpsFdbTriggerAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbTriggerAging(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbConfigureTableAging
    #/********************************************************************************/
    def do_fdb_configure_table_aging(self, arg):
        '''
         xpsFdbConfigureTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsFdbConfigureTableAging(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbConfigureEntryAging
    #/********************************************************************************/
    def do_fdb_configure_entry_aging(self, arg):
        '''
         xpsFdbConfigureEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsFdbConfigureEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbGetTableDepth
    #/********************************************************************************/
    def do_fdb_get_table_depth(self, arg):
        '''
         xpsFdbGetTableDepth: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            depth_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbGetTableDepth(args[0],depth_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('%d' % (uint32_tp_value(depth_Ptr_1)))
                pass
            delete_uint32_tp(depth_Ptr_1)

    #/********************************************************************************/
    # command for xpsFdbGetTableAgingStatus
    #/********************************************************************************/
    def do_fdb_get_table_aging_status(self, arg):
        '''
         xpsFdbGetTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbGetTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_1)))
                pass
            delete_uint8_tp(enable_Ptr_1)

    #/********************************************************************************/
    # command for xpsFdbGetEntryAgingStatus
    #/********************************************************************************/
    def do_fdb_get_entry_aging_status(self, arg):
        '''
         xpsFdbGetEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint8_tp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsFdbGetEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_1)))
                pass
            delete_uint8_tp(enable_Ptr_1)

    #/********************************************************************************/
    # command for xpsFdbSetAgingTime
    #/********************************************************************************/
    def do_fdb_set_aging_time(self, arg):
        '''
         xpsFdbSetAgingTime: Enter [ devId,agingExpo ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,agingExpo ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, againgExpo=%d' % (args[0],args[1]))
            ret = xpsFdbSetAgingTime(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbGetAgingTime
    #/********************************************************************************/
    def do_fdb_get_aging_time(self, arg):
        '''
         xpsFdbGetAgingTime: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbGetAgingTime(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('aging expo = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)

    #/********************************************************************************/
    # command for xpsFdbAddEntry
    #/********************************************************************************/
    def do_fdb_add_entry(self, arg):
        '''
         xpsFdbAddEntry: Enter [ devId,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 9
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
        
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId ]')
            print('fdb_add_entry X X XX:XX:XX:XX:XX:XX X X X X X X')
        else:
            args[0] = int(args[0])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[1])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)
            xpsFdbEntry_t_Ptr.pktCmd = eval(args[3])
            xpsFdbEntry_t_Ptr.isControl = int(args[4])
            xpsFdbEntry_t_Ptr.isRouter = int(args[5])
            xpsFdbEntry_t_Ptr.isStatic = int(args[6])
            xpsFdbEntry_t_Ptr.intfId = int(args[7])
            xpsFdbEntry_t_Ptr.serviceInstId = int(args[8])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()

            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s, pktCmd=%d, isControl=%d, isRouter=%d, isStatic=%d, intfId=%d, serviceInstId=%d' % (args[0],xpsFdbEntry_t_Ptr.vlanId,args[2],xpsFdbEntry_t_Ptr.pktCmd,xpsFdbEntry_t_Ptr.isControl,xpsFdbEntry_t_Ptr.isRouter,xpsFdbEntry_t_Ptr.isStatic,xpsFdbEntry_t_Ptr.intfId,xpsFdbEntry_t_Ptr.serviceInstId))
            ret = xpsFdbAddEntry(args[0],xpsFdbEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (int(args[1])))
                print('macAddr = '),
                listLen = 6
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                xpShellGlobals.cmdRetVal = xpsHashIndexList_t_Ptr.index[0]
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsFdbWriteEntry
    #/********************************************************************************/
    def do_fdb_write_entry(self, arg):
        '''
         xpsFdbWriteEntry: Enter [ devId,index,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq)  or (listLen != 6) :
            print('Invalid input, Enter [ devId,index,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId ]')
            print('fdb_write_entry X X X XX:XX:XX:XX:XX:XX X X X X X X')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[2])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)
            xpsFdbEntry_t_Ptr.pktCmd = eval(args[4])
            xpsFdbEntry_t_Ptr.isControl = int(args[5])
            xpsFdbEntry_t_Ptr.isRouter = int(args[6])
            xpsFdbEntry_t_Ptr.isStatic = int(args[7])
            xpsFdbEntry_t_Ptr.intfId = int(args[8])
            xpsFdbEntry_t_Ptr.serviceInstId = int(args[9])

            #print('Input Arguments are, devId=%d, index=%d, vlanId=%d, macAddr=%s, pktCmd=%d, isControl=%d, isRouter=%d, isStatic=%d, intfId=%d, serviceInstId=%d' % (args[0],args[1],xpsFdbEntry_t_Ptr.vlanId,args[3],xpsFdbEntry_t_Ptr.pktCmd,xpsFdbEntry_t_Ptr.isControl,xpsFdbEntry_t_Ptr.isRouter,xpsFdbEntry_t_Ptr.isStatic,xpsFdbEntry_t_Ptr.intfId,xpsFdbEntry_t_Ptr.serviceInstId))
            ret = xpsFdbWriteEntry(args[0],args[1],xpsFdbEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 1:
                print('vlanId = %d' % (int(args[2])))
                print('macAddr = '),
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                pass
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbRemoveEntry
    #/********************************************************************************/
    def do_fdb_remove_entry(self, arg):
        '''
         xpsFdbRemoveEntry: Enter [ devId,vlanId,macAddr,intfId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr,intfId ]')
            print('fdb_remove_entry X X XX:XX:XX:XX:XX:XX X')
        else:
            args[0] = int(args[0])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[1])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)
            xpsFdbEntry_t_Ptr.intfId = int(args[3])

            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s, intfId=%d' % (args[0],xpsFdbEntry_t_Ptr.vlanId,args[2],xpsFdbEntry_t_Ptr.intfId))
            ret = xpsFdbRemoveEntry(args[0],xpsFdbEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 1:
                print('vlanId = %d' % (int(args[1])))
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                pass
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbRemoveEntryByIndex
    #/********************************************************************************/
    def do_fdb_remove_entry_by_index(self, arg):
        '''
         xpsFdbRemoveEntryByIndex: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsFdbRemoveEntryByIndex(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbFindEntry
    #/********************************************************************************/
    def do_fdb_find_entry(self, arg):
        '''
         xpsFdbFindEntry: Enter [ devId,vlanId,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr ]')
            print('fdb_find_entry X X XX:XX:XX:XX:XX:XX')
        else:
            args[0] = int(args[0])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[1])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)

            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s' % (args[0],xpsFdbEntry_t_Ptr.vlanId,args[2]))
            ret = xpsFdbGetEntry(args[0],xpsFdbEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsFdbEntry_t_Ptr.vlanId))
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('\nInterface = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('Static Bit = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                pass
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbGetEntry
    #/********************************************************************************/
    def do_fdb_get_entry(self, arg):
        '''
         xpsFdbGetEntry: Enter [ devId,vlanId,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr ]')
            print('fdb_get_entry X X XX:XX:XX:XX:XX:XX')
        else:
            args[0] = int(args[0])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[1])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)

            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s' % (args[0],xpsFdbEntry_t_Ptr.vlanId,args[2]))
            ret = xpsFdbGetEntry(args[0],xpsFdbEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsFdbEntry_t_Ptr.vlanId))
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                pass
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbGetNumOfValidEntries
    #/********************************************************************************/
    def do_fdb_get_num_of_valid_entries(self, arg):
        '''
         xpsFdbGetNumOfValidEntries: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfValidEntries_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbGetNumOfValidEntries(args[0],numOfValidEntries_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_1)))
                pass
            delete_uint32_tp(numOfValidEntries_Ptr_1)

    #/********************************************************************************/
    # command for xpsFdbGetEntryByIndex
    #/********************************************************************************/
    def do_fdb_get_entry_by_index(self, arg):
        '''
         xpsFdbGetEntryByIndex: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()

            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsFdbGetEntryByIndex(args[0],args[1],xpsFdbEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsFdbEntry_t_Ptr.vlanId))
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                pass
	    delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbAddControlMacEntry
    #/********************************************************************************/
    def do_fdb_add_control_mac_entry(self, arg):
        '''
         xpsFdbAddControlMacEntry: Enter [ devId,vlanId,macAddr,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macAddrList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr,reasonCode ]')
            print('fdb_add_control_mac_entry X X XX:XX:XX:XX:XX:XX X')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macAddr = macAddr_t()
            for ix in range(0, listLen, 1):
                macAddr[ix] = int(macAddrList[ix], 16)
            args[3] = int(args[3])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s, reasonCode=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsFdbAddControlMacEntry(args[0],args[1],macAddr,args[3],xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                xpShellGlobals.cmdRetVal = xpsHashIndexList_t_Ptr.index[0]
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
    #/********************************************************************************/
    # command for xpsFdbRemoveControlMacEntry
    #/********************************************************************************/
    def do_fdb_remove_control_mac_entry(self, arg):
        '''
         xpsFdbRemoveControlMacEntry: Enter [ devId,vlanId,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        listLen = 0
        if (len(args) == numArgsReq) :
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macAddrList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr ]')
            print('fdb_remove_control_mac_entry X X XX:XX:XX:XX:XX:XX')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macAddr = macAddr_t()
            for ix in range(0, listLen, 1):
                macAddr[ix] = int(macAddrList[ix], 16)
            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s' % (args[0],args[1],args[2]))
            ret = xpsFdbRemoveControlMacEntry(args[0],args[1],macAddr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsFdbGetControlMacEntryReasonCode
    #/********************************************************************************/
    def do_fdb_get_control_mac_entry_reason_code(self, arg):
        '''
         xpsFdbGetControlMacEntryReasonCode: Enter [ devId,vlanId,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        listLen = 0
        if (len(args) == numArgsReq) :
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macAddrList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr ]')
            print('fdb_get_control_mac_entry_reason_code X X XX:XX:XX:XX:XX:XX')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macAddr = macAddr_t()
            for ix in range(0, listLen, 1):
                macAddr[ix] = int(macAddrList[ix], 16)
            reasonCode_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s' % (args[0],args[1],args[2]))
            ret = xpsFdbGetControlMacEntryReasonCode(args[0],args[1],macAddr,reasonCode_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('reasonCode = %d' % (uint32_tp_value(reasonCode_Ptr_3)))
                pass
            delete_uint32_tp(reasonCode_Ptr_3)

    #/********************************************************************************/
    # command for xpsFdbRegisterLearnHandler
    #/********************************************************************************/
    def do_fdb_register_learn_handler(self, arg):
        '''
         xpsFdbSetResetLearnHandlerFdbMgr: Enter [ 0/1 ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ 0/1 ]')
        else:
            args[0] = int(args[0])
            ret = xpsFdbSetResetLearnHdl(args[0])
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass

    #/********************************************************************************/
    # command for xpsFdbUnregisterLearnHandler
    #/********************************************************************************/
    def do_fdb_unregister_learn_handler(self, arg):
        '''
         xpsFdbUnregisterLearnHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbUnRegisterLearnHandlerFdbMgr(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbGetL2EncapType
    #/********************************************************************************/
    def do_fdb_get_l2_encap_type(self, arg):
        '''
         xpsFdbGetL2EncapType: Enter [ devId,intfId,bdId,isTagged,tagVlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,bdId,isTagged,tagVlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            encapType_Ptr_5 = new_uint32_tp()
            encapVid_Ptr_6 = new_xpVlan_tp()
            transTnl_Ptr_7 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, intfId=%d, bdId=%d, isTagged=%d, tagVlanId=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsFdbGetL2EncapType(args[0],args[1],args[2],args[3],args[4],encapType_Ptr_5,encapVid_Ptr_6,transTnl_Ptr_7)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('encapType = %d' % (uint32_tp_value(encapType_Ptr_5)))
                print('encapVid = %d' % (xpVlan_tp_value(encapVid_Ptr_6)))
                print('transTnl = %d' % (uint8_tp_value(transTnl_Ptr_7)))
                pass
            delete_uint8_tp(transTnl_Ptr_7)
            delete_xpVlan_tp(encapVid_Ptr_6)
            delete_uint32_tp(encapType_Ptr_5)

    #/********************************************************************************/
    # command for xpsFdbRegisterAgingHandler
    #/********************************************************************************/
    def do_fdb_register_aging_handler(self, arg):
        '''
         xpsFdbRegisterAgingHandler: Enter [ devId,fdbAgingHandler ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fdbAgingHandler ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, fdbAgingHandler=%d' % (args[0],args[1]))
            ret = xpsFdbRegisterAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)

    #/********************************************************************************/
    # command for xpsFdbRegisterDefaultAgingHandler
    #/********************************************************************************/
    def do_fdb_register_default_aging_handler(self, arg):
        '''
         xpsFdbRegisterDefaultAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbRegisterDefaultAgingHandler(args[0])
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass

    #/********************************************************************************/
    # command for xpsFdbUnregisterAgingHandler
    #/********************************************************************************/
    def do_fdb_unregister_aging_handler(self, arg):
        '''
         xpsFdbUnregisterAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbUnregisterAgingHandler(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbFlushEntry
    #/********************************************************************************/
    def do_fdb_flush_entry(self, arg):
        '''
         xpsFdbFlushEntry: Enter devId,entryType [ updateShadow ]
         valid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,entryType [ updateShadow ]\nvalid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            if(len(args) > numArgsReq):
                args[2] = int(args[2])
                #print('Input Arguments are, devId=%d, entryType=%d updateShadow=%d' % (args[0],args[1],args[2]))
                ret = xpsFdbFlushEntryWithShadowUpdate(args[0],args[1],args[2])
            else:
                #print('Input Arguments are, devId=%d, entryType=%d' % (args[0],args[1]))
                ret = xpsFdbFlushEntryWithShadowUpdate(args[0],args[1], 1)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbFlushEntryLoop
    #/********************************************************************************/
    def do_fdb_flush_entry_loop(self, arg):
        '''
         xpsFdbFlushEntryLoop: Enter devId,entryType [ updateShadow ]
        '''
        for i in range(0, 90000000):
            self.do_fdb_flush_entry(arg)


    #/********************************************************************************/
    # command for xpsFdbFlushEntryByIntf
    #/********************************************************************************/
    def do_fdb_flush_entry_by_intf(self, arg):
        '''
         xpsFdbFlushEntryByIntf: Enter devId,intfId,entryType [ updateShadow ]
         valid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,intfId,entryType [ updateShadow ]\nvalid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            if(len(args) > numArgsReq):
                args[3] = int(args[3])
                #print('Input Arguments are, devId=%d, intfId=%d, updateShadow=%d' % (args[0],args[1],args[2],args[3]))
                ret = xpsFdbFlushEntryByIntfWithShadowUpdate(args[0],args[1],args[2],args[3])
            else:
                #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1],args[2]))
                ret = xpsFdbFlushEntryByIntfWithShadowUpdate(args[0],args[1],args[2], 1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbFlushEntryByVlan
    #/********************************************************************************/
    def do_fdb_flush_entry_by_vlan(self, arg):
        '''
         xpsFdbFlushEntryByVlan: Enter devId,vlanId,entryType [ updateShadow ]
         valid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,vlanId,entryType [ updateShadow ]\nvalid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            if(len(args) > numArgsReq):
                args[3] = int(args[3])
                #print('Input Arguments are, devId=%d, vlanId=%d, updateShadow=%d' % (args[0],args[1],args[2],args[3]))
                ret = xpsFdbFlushEntryByVlanWithShadowUpdate(args[0],args[1],args[2],args[3])
            else:
                #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1],args[2]))
                ret = xpsFdbFlushEntryByVlanWithShadowUpdate(args[0],args[1],args[2], 1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbFlushEntryByIntfVlan
    #/********************************************************************************/
    def do_fdb_flush_entry_by_intf_vlan(self, arg):
        '''
         xpsFdbFlushEntryByIntfVlan: Enter devId,intfId,vlanId,entryType [ updateShadow ]
         valid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,intfId,vlanId,entryType [ updateShadow ]\nvalid entryType values are XP_FDB_ENTRY_TYPE_DYNAMIC,XP_FDB_ENTRY_TYPE_STATIC and XP_FDB_ENTRY_TYPE_ALL')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            if(len(args) > numArgsReq):
                args[4] = int(args[4])
                #print('Input Arguments are, devId=%d, intfId=%d, vlanId=%d, entryType=%d, updateShadow=%d' % (args[0],args[1],args[2],args[3],args[4]))
                ret = xpsFdbFlushEntryByIntfVlanWithShadowUpdate(args[0],args[1],args[2],args[3],args[4])
            else:
                #print('Input Arguments are, devId=%d, intfId=%d, vlanId=%d, entryType=%d' % (args[0],args[1],args[2],args[3]))
                ret = xpsFdbFlushEntryByIntfVlanWithShadowUpdate(args[0],args[1],args[2],args[3], 1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbSetAttribute
    #/********************************************************************************/
    def do_fdb_set_attribute(self, arg):
        '''
         xpsFdbSetAttribute: Enter [ devId,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId,field ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId,field ]')
            print('fdb_set_attribute X X XX:XX:XX:XX:XX:XX X X X X X X X')
        else:
            args[0] = int(args[0])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[1])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)
            xpsFdbEntry_t_Ptr.pktCmd = eval(args[3])
            xpsFdbEntry_t_Ptr.isControl = int(args[4])
            xpsFdbEntry_t_Ptr.isRouter = int(args[5])
            xpsFdbEntry_t_Ptr.isStatic = int(args[6])
            xpsFdbEntry_t_Ptr.intfId = int(args[7])
            xpsFdbEntry_t_Ptr.serviceInstId = int(args[8])
            args[9] = eval(args[9])
            data_Ptr_10 = new_voidp()

            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s, pktCmd=%d, isControl=%d, isRouter=%d, isStatic=%d, intfId=%d, serviceInstId=%d, field=%d' % (args[0],xpsFdbEntry_t_Ptr.vlanId,args[2],xpsFdbEntry_t_Ptr.pktCmd,xpsFdbEntry_t_Ptr.isControl,xpsFdbEntry_t_Ptr.isRouter,xpsFdbEntry_t_Ptr.isStatic,xpsFdbEntry_t_Ptr.intfId,xpsFdbEntry_t_Ptr.serviceInstId,args[9]))
            ret = xpsFdbSetAttribute(args[0],xpsFdbEntry_t_Ptr,args[9],data_Ptr_10)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsFdbEntry_t_Ptr.vlanId))
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                print('data = %d' % (voidp_value(data_Ptr_10)))
                pass
            delete_voidp(data_Ptr_10)
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbGetAttribute
    #/********************************************************************************/
    def do_fdb_get_attribute(self, arg):
        '''
         xpsFdbGetAttribute: Enter [ devId,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId,field ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        listLen = 0
        if  (len(args) == numArgsReq) :
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (listLen != 6) :
            print('Invalid input, Enter [ devId,vlanId,macAddr,pktCmd,isControl,isRouter,isStatic,intfId,serviceInstId,field ]')
            print('fdb_get_attribute X X XX:XX:XX:XX:XX:XX X X X X X X X')
        else:
            args[0] = int(args[0])
            xpsFdbEntry_t_Ptr = new_xpsFdbEntry_tp()
            xpsFdbEntry_t_Ptr.vlanId = int(args[1])

            for ix in range(0, listLen, 1):
                xpsFdbEntry_t_Ptr.macAddr[ix] = int(postList[ix], 16)
            xpsFdbEntry_t_Ptr.pktCmd = eval(args[3])
            xpsFdbEntry_t_Ptr.isControl = int(args[4])
            xpsFdbEntry_t_Ptr.isRouter = int(args[5])
            xpsFdbEntry_t_Ptr.isStatic = int(args[6])
            xpsFdbEntry_t_Ptr.intfId = int(args[7])
            xpsFdbEntry_t_Ptr.serviceInstId = int(args[8])
            args[9] = eval(args[9])
            data_Ptr_10 = new_voidp()

            #print('Input Arguments are, devId=%d, vlanId=%d, macAddr=%s, pktCmd=%d, isControl=%d, isRouter=%d, isStatic=%d, intfId=%d, serviceInstId=%d, field=%d' % (args[0],xpsFdbEntry_t_Ptr.vlanId,args[2],xpsFdbEntry_t_Ptr.pktCmd,xpsFdbEntry_t_Ptr.isControl,xpsFdbEntry_t_Ptr.isRouter,xpsFdbEntry_t_Ptr.isStatic,xpsFdbEntry_t_Ptr.intfId,xpsFdbEntry_t_Ptr.serviceInstId,args[9]))
            ret = xpsFdbGetAttribute(args[0],xpsFdbEntry_t_Ptr,args[9],data_Ptr_10)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsFdbEntry_t_Ptr.vlanId))
                listLen = 6
                print('macAddr = '),
                for ix in range(0, listLen, 1):
                    print('%02x' % (xpsFdbEntry_t_Ptr.macAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsFdbEntry_t_Ptr.pktCmd))
                print('isControl = %d' % (xpsFdbEntry_t_Ptr.isControl))
                print('isRouter = %d' % (xpsFdbEntry_t_Ptr.isRouter))
                print('isStatic = %d' % (xpsFdbEntry_t_Ptr.isStatic))
                print('intfId = %d' % (xpsFdbEntry_t_Ptr.intfId))
                print('serviceInstId = %d' % (xpsFdbEntry_t_Ptr.serviceInstId))
                print('data = %d' % (voidp_value(data_Ptr_10)))
                pass
            delete_voidp(data_Ptr_10)
            delete_xpsFdbEntry_tp(xpsFdbEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsFdbSetAttributeByIndex
    #/********************************************************************************/
    def do_fdb_set_attribute_by_index(self, arg):
        '''
         xpsFdbSetAttributeByIndex: Enter [ devId,index,field ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,field ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            data_Ptr_3 = new_voidp()
            #print('Input Arguments are, devId=%d, index=%d, field=%d' % (args[0],args[1],args[2]))
            ret = xpsFdbSetAttributeByIndex(args[0],args[1],args[2],data_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_3)))
                pass
            delete_voidp(data_Ptr_3)

    #/********************************************************************************/
    # command for xpsFdbGetAttributeByIndex
    #/********************************************************************************/
    def do_fdb_get_attribute_by_index(self, arg):
        '''
         xpsFdbGetAttributeByIndex: Enter [ devId,index,field ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,field ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            data_Ptr_3 = new_voidp()
            #print('Input Arguments are, devId=%d, index=%d, field=%d' % (args[0],args[1],args[2]))
            ret = xpsFdbGetAttributeByIndex(args[0],args[1],args[2],data_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_3)))
                pass
            delete_voidp(data_Ptr_3)
    #/********************************************************************************/
    # command for xpsFdbSetRehashLevel
    #/********************************************************************************/
    def do_fdb_set_rehash_level(self, arg):
        '''
         xpsFdbSetRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsFdbSetRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsFdbGetRehashLevel
    #/********************************************************************************/
    def do_fdb_get_rehash_level(self, arg):
        '''
         xpsFdbGetRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbGetRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

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

	#/********************************************************************************/
    # command for xpsFdbGetSourceMacHit
    #/********************************************************************************/
    def do_fdb_get_source_mac_hit(self, arg):
        '''
         xpsFdbGetSourceMacHit: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        enable_Ptr_1 = new_uint8_tp()
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsFdbGetSourceMacHit(args[0],args[1],enable_Ptr_1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('MacHitStatus= %d' % (uint8_tp_value(enable_Ptr_1)))
                pass
            delete_uint8_tp(enable_Ptr_1)

    #/********************************************************************************/
    # command for xpsFdbClearSourceMacHit
    #/********************************************************************************/
    def do_fdb_clear_source_mac_hit(self, arg):
        '''
         xpsFdbClearSourceMacHit: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsFdbClearSourceMacHit(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsFdbClearBucketState
    #/********************************************************************************/
    def do_fdb_clear_bucket_state(self, arg):
        '''
         xpsFdbClearBucketState: Enter [ devId,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tblCopyIdx=%d ' % (args[0],args[1]))
            ret = xpsFdbClearBucketState(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSetFdbMsgRateLimit
    #/********************************************************************************/
    def do_fdb_set_msg_ratelimit(self, arg):
        '''
         xpsFdbSetMsgRateLimit: Enter [ devId,msgRate,enable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,msgRate, enable]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d,msgRate=%d, enable = %d' % (args[0],args[1],args[2]))
            ret = xpsFdbSetMsgRateLimit(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
