#!/usr/bin/env python
#  xpsAging.py
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
# The class object for xpsAging operations
#/**********************************************************************************/

class xpsAgingObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsAgeFifoHandler
    #/********************************************************************************/
    def do_age_fifo_handler(self, arg):
        '''
         xpsAgeFifoHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsAgeFifoHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsReadAgeFifoMessage
    #/********************************************************************************/
    def do_read_age_fifo_message(self, arg):
        '''
         xpsReadAgeFifoMessage: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            xpAgeFifoData_Ptr = new_xpAgeFifoData_tp()
            
            #print('Input Arguments are, devId=%d, ageMemRowAddr=0x%x, ageMemRowData=%s' % (args[0],xpAgeFifoData_Ptr.ageMemRowAddr,args[2]))
            ret = xpsReadAgeFifoMessage(args[0],xpAgeFifoData_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                listLen = 2
                print('ageMemRowAddr = %d' % (xpAgeFifoData_Ptr.ageMemRowAddr))
                print('ageMemRowData = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpAgeFifoData_Ptr.ageMemRowData[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                pass
            delete_xpAgeFifoData_tp(xpAgeFifoData_Ptr)
    #/********************************************************************************/
    # command for xpsProcessAgeFifoMessage
    #/********************************************************************************/
    def do_process_age_fifo_message(self, arg):
        '''
         xpsProcessAgeFifoMessage: Enter [ devId,ageMemRowAddr,ageMemRowData ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,ageMemRowAddr,ageMemRowData ]')
        else:
            args[0] = int(args[0])
            xpAgeFifoData_Ptr = new_xpAgeFifoData_tp()
            xpAgeFifoData_Ptr.ageMemRowAddr = int(args[1], 16)
            
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpAgeFifoData_Ptr.ageMemRowData[listLen - ix - 1] = int(postList[ix])
            
            #print('Input Arguments are, devId=%d, ageMemRowAddr=0x%x, ageMemRowData=%s' % (args[0],xpAgeFifoData_Ptr.ageMemRowAddr,args[2]))
            ret = xpsProcessAgeFifoMessage(args[0],xpAgeFifoData_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ageMemRowAddr = %d' % (xpAgeFifoData_Ptr.ageMemRowAddr))
                print('ageMemRowData = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpAgeFifoData_Ptr.ageMemRowData[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                pass
            delete_xpAgeFifoData_tp(xpAgeFifoData_Ptr)
    #/********************************************************************************/
    # command for xpsSetAgingMode
    #/********************************************************************************/
    def do_set_aging_mode(self, arg):
        '''
         xpsSetAgingMode: Enter [ devId,ageMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,ageMode ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, ageMode=%d' % (args[0],args[1]))
            ret = xpsSetAgingMode(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGetAgingMode
    #/********************************************************************************/
    def do_get_aging_mode(self, arg):
        '''
         xpsGetAgingMode: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            ageMode_Ptr_1 = new_XP_AGE_MODE_Tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsGetAgingMode(args[0],ageMode_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ageMode = %d' % (XP_AGE_MODE_Tp_value(ageMode_Ptr_1)))
                pass
            delete_XP_AGE_MODE_Tp(ageMode_Ptr_1)
    #/********************************************************************************/
    # command for xpsSetAgingCycleUnitTime
    #/********************************************************************************/
    def do_set_aging_cycle_unit_time(self, arg):
        '''
         xpsSetAgingCycleUnitTime: Enter [ devId,ageTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,ageTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, ageTime=%d' % (args[0],args[1]))
            ret = xpsSetAgingCycleUnitTime(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGetAgingCycleUnitTime
    #/********************************************************************************/
    def do_get_aging_cycle_unit_time(self, arg):
        '''
         xpsGetAgingCycleUnitTime: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            ageTime_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsGetAgingCycleUnitTime(args[0],ageTime_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ageTime = %d' % (uint32_tp_value(ageTime_Ptr_1)))
                pass
            delete_uint32_tp(ageTime_Ptr_1)
    #/********************************************************************************/
    # command for xpsDisableRefreshPause
    #/********************************************************************************/
    def do_disable_refresh_pause(self, arg):
        '''
         xpsDisableRefreshPause: Enter [ devId,disable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,disable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, disable=%d' % (args[0],args[1]))
            ret = xpsDisableRefreshPause(args[0],args[1])
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
