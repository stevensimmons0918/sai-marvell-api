#!/usr/bin/env python
#  xpsInt.py
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
# The class object for xpsInt operations
#/**********************************************************************************/

class xpsIntObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsIntInit
    #/********************************************************************************/
    def do_int_init(self, arg):
        '''
         xpsIntInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsIntInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIntDeInit
    #/********************************************************************************/
    def do_int_de_init(self, arg):
        '''
         xpsIntDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsIntDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIntAddDevice
    #/********************************************************************************/
    def do_int_add_device(self, arg):
        '''
         xpsIntAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsIntAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIntRemoveDevice
    #/********************************************************************************/
    def do_int_remove_device(self, arg):
        '''
         xpsIntRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIntRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIntGetQueueCongestion
    #/********************************************************************************/
    def do_int_get_queue_congestion(self, arg):
        '''
         xpsIntGetQueueCongestion: Enter [ devId,portNum,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            queueCongestion_Ptr_3 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsIntGetQueueCongestion(args[0],args[1],args[2],queueCongestion_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('queueCongestion = %d' % (uint8_tp_value(queueCongestion_Ptr_3)))
                pass
            delete_uint8_tp(queueCongestion_Ptr_3)

    #/********************************************************************************/
    # command for xpsIntDisplayTable
    #/********************************************************************************/
    def do_int_display_table(self, arg):
        '''
         xpsIntDisplayTable: Enter [ devId,startIndex,endIndex,detailFormat,silentMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,startIndex,endIndex,detailFormat,silentMode ]')
        else:
            args[0] = int(args[0])
            numOfValidEntries_Ptr_1 = new_uint32_tp()
            args[2] = int(args[2])
            args[3] = int(args[3])
            logFile_Ptr_4 = new_charp()
            args[5] = int(args[5])
            args[6] = int(args[6])
            #print('Input Arguments are, devId=%d, startIndex=%d, endIndex=%d, detailFormat=%d, silentMode=%d' % (args[0],args[2],args[3],args[5],args[6]))
            ret = xpsIntDisplayTable(args[0],numOfValidEntries_Ptr_1,args[2],args[3],logFile_Ptr_4,args[5],args[6])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_1)))
                print('logFile = %d' % (charp_value(logFile_Ptr_4)))
                pass
            delete_charp(logFile_Ptr_4)
            delete_uint32_tp(numOfValidEntries_Ptr_1)

    #/********************************************************************************/
    # command for xpsIntSetSwitchId
    #/********************************************************************************/
    def do_int_set_switch_id(self, arg):
        '''
         xpsIntSetSwitchId: Enter [ devId,switchId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,switchId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsIntSetSwitchId(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIntGetSwitchId
    #/********************************************************************************/
    def do_int_get_switch_id(self, arg):
        '''
         xpsIntGetSwitchId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            switchId_Ptr_3 = new_uint32_tp()
            ret = xpsIntGetSwitchId(args[0],switchId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Switch ID = %d' % (uint32_tp_value(switchId_Ptr_3)))
                pass
            delete_uint32_tp(switchId_Ptr_3)

    #/********************************************************************************/
    # command for xpsIntRegisterIntIntf
    #/********************************************************************************/
    def do_int_register_intf(self, arg):
        '''
         xpsIntRegisterIntIntf: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsIntRegisterIntIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIntSetAsIntCollector
    #/********************************************************************************/
    def do_int_set_as_collector(self, arg):
        '''
         xpsIntSetAsIntCollector: Enter [ devId,portNum,sinkMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,sinkMode ]')
            print('mode: To set as collector- 1 otherwise 0 ')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = bool(args[2])
            ret = xpsIntSetAsIntCollector(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
