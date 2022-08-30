#!/usr/bin/env python
#  xpsPtp.py
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
# The class object for xpsPtp operations
#/**********************************************************************************/

class xpsPtpObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsPtpInit
    #/********************************************************************************/
    def do_ptp_init(self, arg):
        '''
         xpsPtpInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPtpInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpFetchTimeStamp
    #/********************************************************************************/
    def do_ptp_fetch_time_stamp(self, arg):
        '''
         xpsPtpFetchTimeStamp: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            timeStamp_Ptr_1 = new_uint64_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPtpFetchTimeStamp(args[0],timeStamp_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('timeStamp = %d' % (uint64_tp_value(timeStamp_Ptr_1)))
                pass
            delete_uint64_tp(timeStamp_Ptr_1)
    #/********************************************************************************/
    # command for xpsPtpAddDevice
    #/********************************************************************************/
    def do_ptp_add_device(self, arg):
        '''
         xpsPtpAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsPtpAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpRemoveDevice
    #/********************************************************************************/
    def do_ptp_remove_device(self, arg):
        '''
         xpsPtpRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPtpRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpDeInit
    #/********************************************************************************/
    def do_ptp_de_init(self, arg):
        '''
         xpsPtpDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPtpDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpLoadCurrentTime
    #/********************************************************************************/
    def do_ptp_load_current_time(self, arg):
        '''
         xpsPtpLoadCurrentTime: Enter [ devId,currentTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,currentTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, currentTime=0x%x' % (args[0],args[1]))
            ret = xpsPtpLoadCurrentTime(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetCurrentTime
    #/********************************************************************************/
    def do_ptp_get_current_time(self, arg):
        '''
         xpsPtpGetCurrentTime: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            currentTime_Ptr_1 = new_uint64_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPtpGetCurrentTime(args[0],currentTime_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('currentTime = %d' % (uint64_tp_value(currentTime_Ptr_1)))
                pass
            delete_uint64_tp(currentTime_Ptr_1)
    #/********************************************************************************/
    # command for xpsPtpAdjustCurrentTime
    #/********************************************************************************/
    def do_ptp_adjust_current_time(self, arg):
        '''
         xpsPtpAdjustCurrentTime: Enter [ devId,deltaTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,deltaTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, deltaTime=%d' % (args[0],args[1]))
            ret = xpsPtpAdjustCurrentTime(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetAdjustCurrentTime
    #/********************************************************************************/
    def do_ptp_get_adjust_current_time(self, arg):
        '''
         xpsPtpGetAdjustCurrentTime: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            deltaTime_Ptr_1 = new_uint64_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPtpGetAdjustCurrentTime(args[0],deltaTime_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('deltaTime = %d' % (uint64_tp_value(deltaTime_Ptr_1)))
                pass
            delete_uint64_tp(deltaTime_Ptr_1)
    #/********************************************************************************/
    # command for xpsPtpLoadClockInc
    #/********************************************************************************/
    def do_ptp_load_clock_inc(self, arg):
        '''
         xpsPtpLoadClockInc: Enter [ devId,nsInc,fnsInc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nsInc,fnsInc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, nsInc=%d, fnsInc=%d' % (args[0],args[1],args[2]))
            ret = xpsPtpLoadClockInc(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetClockInc
    #/********************************************************************************/
    def do_ptp_get_clock_inc(self, arg):
        '''
         xpsPtpGetClockInc: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            nsInc_Ptr_1 = new_uint32_tp()
            fnsInc_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPtpGetClockInc(args[0],nsInc_Ptr_1,fnsInc_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nsInc = %d' % (uint32_tp_value(nsInc_Ptr_1)))
                print('fnsInc = %d' % (uint32_tp_value(fnsInc_Ptr_2)))
                pass
            delete_uint32_tp(fnsInc_Ptr_2)
            delete_uint32_tp(nsInc_Ptr_1)
    #/********************************************************************************/
    # command for xpsPtpSetOutputClock
    #/********************************************************************************/
    def do_ptp_set_output_clock(self, arg):
        '''
         xpsPtpSetOutputClock: Enter [ devId,outputNum,startTime,pulseWidth,nsInc,fsnInc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,outputNum,startTime,pulseWidth,nsInc,fsnInc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, outputNum=%d, startTime=0x%x, pulseWidth=0x%x, nsInc=%d, fsnInc=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsPtpSetOutputClock(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetOutputClock
    #/********************************************************************************/
    def do_ptp_get_output_clock(self, arg):
        '''
         xpsPtpGetOutputClock: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            startTime_Ptr_1 = new_uint64_tp()
            ns_Ptr_2 = new_uint32_tp()
            fsn_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPtpGetOutputClock(args[0],startTime_Ptr_1,ns_Ptr_2,fsn_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('startTime = %d' % (uint64_tp_value(startTime_Ptr_1)))
                print('ns = %d' % (uint32_tp_value(ns_Ptr_2)))
                print('fsn = %d' % (uint32_tp_value(fsn_Ptr_3)))
                pass
            delete_uint32_tp(fsn_Ptr_3)
            delete_uint32_tp(ns_Ptr_2)
            delete_uint64_tp(startTime_Ptr_1)
    #/********************************************************************************/
    # command for xpsPtpSetSyncDir
    #/********************************************************************************/
    def do_ptp_set_sync_dir(self, arg):
        '''
         xpsPtpSetSyncDir: Enter [ devId,pinNum,direction ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pinNum,direction ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, pinNum=%d, direction=%d' % (args[0],args[1],args[2]))
            ret = xpsPtpSetSyncDir(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetSyncDir
    #/********************************************************************************/
    def do_ptp_get_sync_dir(self, arg):
        '''
         xpsPtpGetSyncDir: Enter [ devId,pinNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pinNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            direction_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, pinNum=%d' % (args[0],args[1]))
            ret = xpsPtpGetSyncDir(args[0],args[1],direction_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('direction = %d' % (uint32_tp_value(direction_Ptr_2)))
                pass
            delete_uint32_tp(direction_Ptr_2)
    #/********************************************************************************/
    # command for xpsPtpSetupPulseAction
    #/********************************************************************************/
    def do_ptp_setup_pulse_action(self, arg):
        '''
         xpsPtpSetupPulseAction: Enter [ devId,inputNum,action ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,inputNum,action ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, inputNum=%d, action=%d' % (args[0],args[1],args[2]))
            ret = xpsPtpSetupPulseAction(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetPulseAction
    #/********************************************************************************/
    def do_ptp_get_pulse_action(self, arg):
        '''
         xpsPtpGetPulseAction: Enter [ devId,inputNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,inputNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            action_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, inputNum=%d' % (args[0],args[1]))
            ret = xpsPtpGetPulseAction(args[0],args[1],action_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('action = %d' % (uint32_tp_value(action_Ptr_2)))
                pass
            delete_uint32_tp(action_Ptr_2)
    #/********************************************************************************/
    # command for xpsPtpSetCaptureTime
    #/********************************************************************************/
    def do_ptp_set_capture_time(self, arg):
        '''
         xpsPtpSetCaptureTime: Enter [ devId,slotNum,nsTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,slotNum,nsTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, slotNum=%d, nsTime=0x%x' % (args[0],args[1],args[2]))
            ret = xpsPtpSetCaptureTime(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPtpGetPulseTime
    #/********************************************************************************/
    def do_ptp_get_pulse_time(self, arg):
        '''
         xpsPtpGetPulseTime: Enter [ devId,inputNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,inputNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            arrivalTime_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, inputNum=%d' % (args[0],args[1]))
            ret = xpsPtpGetPulseTime(args[0],args[1],arrivalTime_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('arrivalTime = %d' % (uint64_tp_value(arrivalTime_Ptr_2)))
                pass
            delete_uint64_tp(arrivalTime_Ptr_2)
    #/********************************************************************************/
    # command for xpsPtpGetEgressTimeStamp
    #/********************************************************************************/
    def do_ptp_get_egress_time_stamp(self, arg):
        '''
         xpsPtpGetEgressTimeStamp: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            egressTs_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPtpGetEgressTimeStamp(args[0],args[1],egressTs_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('egressTs = %d' % (uint64_tp_value(egressTs_Ptr_2)))
                pass
            delete_uint64_tp(egressTs_Ptr_2)
  
    #/*****************************************************************************************************/
    # command setIktXpHeaderExists
    #/*****************************************************************************************************/
    def do_set_ikt_xpheader_exists(self, arg):
        '''
         setIktXpHeaderExists: Enter [ devId,sde,pe,pIndex,xpHdrExists ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 5:
            print 'Invalid input, Enter [ devId,sde,pe,pIndex,xpHdrExists ]'
        else:
            deviceId = int(args[0])
            sde = int(args[1])
            pe = int(args[2])
            pIndex = int(args[3])
            xpHdrExists = int(args[4])

            parserMgr = xpDeviceMgr.instance().getDeviceObj(deviceId).getParser()
            xp80ParserMgr = getParserMgrPtr(parserMgr)
            if xp80ParserMgr != None:
                ret = xp80ParserMgr.setIktXpHeaderExists(deviceId, sde, pe, pIndex, xpHdrExists)
                if ret != XP_NO_ERR :
                    print "Failed with error code %d"%(ret)
            else:
                print "Failed : Init parser block first"

