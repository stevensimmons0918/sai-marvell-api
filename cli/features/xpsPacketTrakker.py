#!/usr/bin/env python
#  xpsPacketTrakker.py
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
# The class object for xpsPacketTrakker operations
#/**********************************************************************************/

class xpsPacketTrakkerObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsPacketTrakkerInit
    #/********************************************************************************/
    def do_packet_trakker_init(self, arg):
        '''
         xpsPacketTrakkerInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPacketTrakkerInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerAddDevice
    #/********************************************************************************/
    def do_packet_trakker_add_device(self, arg):
        '''
         xpsPacketTrakkerAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerRemoveDevice
    #/********************************************************************************/
    def do_packet_trakker_remove_device(self, arg):
        '''
         xpsPacketTrakkerRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerDeInit
    #/********************************************************************************/
    def do_packet_trakker_de_init(self, arg):
        '''
         xpsPacketTrakkerDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPacketTrakkerDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerReadMaxQueueDepth
    #/********************************************************************************/
    def do_packet_trakker_read_max_queue_depth(self, arg):
        '''
         xpsPacketTrakkerReadMaxQueueDepth: Enter [ deviceId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            depth_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerReadMaxQueueDepth(args[0],args[1],args[2],depth_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('depth = %d' % (uint32_tp_value(depth_Ptr_3)))
                pass
            delete_uint32_tp(depth_Ptr_3)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnableDeflection
    #/********************************************************************************/
    def do_packet_trakker_enable_deflection(self, arg):
        '''
         xpsPacketTrakkerEnableDeflection: Enter [ deviceId,port,queue,newPort,newQueue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,port,queue,newPort,newQueue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, deviceId=%d, port=%d, queue=%d, newPort=%d, newQueue=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsPacketTrakkerEnableDeflection(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerDisableDeflection
    #/********************************************************************************/
    def do_packet_trakker_disable_deflection(self, arg):
        '''
         xpsPacketTrakkerDisableDeflection: Enter [ deviceId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerDisableDeflection(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerConfigureCookieSession
    #/********************************************************************************/
    def do_packet_trakker_configure_cookie_session(self, arg):
        '''
         xpsPacketTrakkerConfigureCookieSession: Enter [ deviceId,port,queue,signature ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,port,queue,signature ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            sessionId_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, port=%d, queue=%d, signature=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPacketTrakkerConfigureCookieSession(args[0],args[1],args[2],args[3],sessionId_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sessionId = %d' % (uint32_tp_value(sessionId_Ptr_4)))
                pass
            delete_uint32_tp(sessionId_Ptr_4)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnableCookieSession
    #/********************************************************************************/
    def do_packet_trakker_enable_cookie_session(self, arg):
        '''
         xpsPacketTrakkerEnableCookieSession: Enter [ deviceId,sessionId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,sessionId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, deviceId=%d, sessionId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerEnableCookieSession(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerDisableCookieSession
    #/********************************************************************************/
    def do_packet_trakker_disable_cookie_session(self, arg):
        '''
         xpsPacketTrakkerDisableCookieSession: Enter [ deviceId,sessionId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,sessionId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, deviceId=%d, sessionId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerDisableCookieSession(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerTotalCirculatedCookies
    #/********************************************************************************/
    def do_packet_trakker_total_circulated_cookies(self, arg):
        '''
         xpsPacketTrakkerTotalCirculatedCookies: Enter [ deviceId,sessionId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,sessionId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            cookieCnt_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, sessionId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerTotalCirculatedCookies(args[0],args[1],cookieCnt_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cookieCnt = %d' % (uint32_tp_value(cookieCnt_Ptr_2)))
                pass
            delete_uint32_tp(cookieCnt_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerClearCirculatedCookieCount
    #/********************************************************************************/
    def do_packet_trakker_clear_circulated_cookie_count(self, arg):
        '''
         xpsPacketTrakkerClearCirculatedCookieCount: Enter [ deviceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, deviceId=%d' % (args[0]))
            ret = xpsPacketTrakkerClearCirculatedCookieCount(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerReadDelay
    #/********************************************************************************/
    def do_packet_trakker_read_delay(self, arg):
        '''
         xpsPacketTrakkerReadDelay: Enter [ deviceId,sessionId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,sessionId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            delay_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, deviceId=%d, sessionId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerReadDelay(args[0],args[1],delay_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('delay = %d' % (uint64_tp_value(delay_Ptr_2)))
                pass
            delete_uint64_tp(delay_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerReadEvent
    #/********************************************************************************/
    def do_packet_trakker_read_event(self, arg):
        '''
         xpsPacketTrakkerReadEvent: Enter [ deviceId,eventType,queueNum,timestamp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,eventType,queueNum,timestamp ]')
        else:
            args[0] = int(args[0])
            xpsPacketTrakkerEvent_Ptr = new_xpsPacketTrakkerEventp()
            xpsPacketTrakkerEvent_Ptr.eventType = eval(args[1])
            xpsPacketTrakkerEvent_Ptr.queueNum = eval(args[2])
            xpsPacketTrakkerEvent_Ptr.timestamp = int(args[3], 16)
            #print('Input Arguments are, deviceId=%d, eventType=%d, queueNum=%d, timestamp=0x%x' % (args[0],xpsPacketTrakkerEvent_Ptr.eventType,xpsPacketTrakkerEvent_Ptr.queueNum,xpsPacketTrakkerEvent_Ptr.timestamp))
            ret = xpsPacketTrakkerReadEvent(args[0],xpsPacketTrakkerEvent_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('eventType = %d' % (xpsPacketTrakkerEvent_Ptr.eventType))
                print('queueNum = %d' % (xpsPacketTrakkerEvent_Ptr.queueNum))
                print('timestamp = %d' % (xpsPacketTrakkerEvent_Ptr.timestamp))
                pass
            delete_xpsPacketTrakkerEventp(xpsPacketTrakkerEvent_Ptr)
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetEventAlmostFullThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_event_almost_full_threshold(self, arg):
        '''
         xpsPacketTrakkerGetEventAlmostFullThreshold: Enter [ deviceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetEventAlmostFullThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetEventAlmostFullThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_event_almost_full_threshold(self, arg):
        '''
         xpsPacketTrakkerSetEventAlmostFullThreshold: Enter [ deviceId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, deviceId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerSetEventAlmostFullThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetQueueWmThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_queue_wm_threshold(self, arg):
        '''
         xpsPacketTrakkerSetQueueWmThreshold: Enter [ deviceId,profileId,lowWm,highWm,maxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,profileId,lowWm,highWm,maxThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, deviceId=%d, profileId=%d, lowWm=%d, highWm=%d, maxThreshold=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsPacketTrakkerSetQueueWmThreshold(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetQueueWmThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_queue_wm_threshold(self, arg):
        '''
         xpsPacketTrakkerGetQueueWmThreshold: Enter [ deviceId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            lowWm_Ptr_2 = new_uint32_tp()
            highWm_Ptr_3 = new_uint32_tp()
            maxThreshold_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetQueueWmThreshold(args[0],args[1],lowWm_Ptr_2,highWm_Ptr_3,maxThreshold_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lowWm = %d' % (uint32_tp_value(lowWm_Ptr_2)))
                print('highWm = %d' % (uint32_tp_value(highWm_Ptr_3)))
                print('maxThreshold = %d' % (uint32_tp_value(maxThreshold_Ptr_4)))
                pass
            delete_uint32_tp(maxThreshold_Ptr_4)
            delete_uint32_tp(highWm_Ptr_3)
            delete_uint32_tp(lowWm_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerBindPacketTrakkerProfileToQueue
    #/********************************************************************************/
    def do_packet_trakker_bind_packet_trakker_profile_to_queue(self, arg):
        '''
         xpsPacketTrakkerBindPacketTrakkerProfileToQueue: Enter [ deviceId,port,queue,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,port,queue,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, deviceId=%d, port=%d, queue=%d, profileId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPacketTrakkerBindPacketTrakkerProfileToQueue(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPacketTrakkerProfileForQueue
    #/********************************************************************************/
    def do_packet_trakker_get_packet_trakker_profile_for_queue(self, arg):
        '''
         xpsPacketTrakkerGetPacketTrakkerProfileForQueue: Enter [ deviceId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            profileId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerGetPacketTrakkerProfileForQueue(args[0],args[1],args[2],profileId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_3)))
                pass
            delete_uint32_tp(profileId_Ptr_3)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnableQueueLengthTracking
    #/********************************************************************************/
    def do_packet_trakker_enable_queue_length_tracking(self, arg):
        '''
         xpsPacketTrakkerEnableQueueLengthTracking: Enter [ devId,profileId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerEnableQueueLengthTracking(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnablePacketTrakker
    #/********************************************************************************/
    def do_packet_trakker_enable_packet_trakker(self, arg):
        '''
         xpsPacketTrakkerEnablePacketTrakker: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerEnablePacketTrakker(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPacketTrakkerEnable
    #/********************************************************************************/
    def do_packet_trakker_get_packet_trakker_enable(self, arg):
        '''
         xpsPacketTrakkerGetPacketTrakkerEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetPacketTrakkerEnable(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnableGlobalStateMirrorMode
    #/********************************************************************************/
    def do_packet_trakker_enable_global_state_mirror_mode(self, arg):
        '''
         xpsPacketTrakkerEnableGlobalStateMirrorMode: Enter [ devId,state,mirrorMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,state,mirrorMode ]')
	    print('Global States: ') 	
	    print(' 0 - Shared Pool WM  ') 	
	    print(' 1 - Global Threshold ') 	
	    print(' 2 - Global MC Threshold ') 	
	    print('Mirror Modes: ') 	
	    print(' 0 - No Mirroring ') 	
	    print(' 1 - State Change ') 	
	    print(' 2 - Level Change ') 	
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, state=%d, mirrorMode=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerEnableGlobalStateMirrorMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetGlobalStateMirrorMode
    #/********************************************************************************/
    def do_packet_trakker_get_global_state_mirror_mode(self, arg):
        '''
         xpsPacketTrakkerGetGlobalStateMirrorMode: Enter [ devId,state ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,state ]')
	    print('Global States: ') 	
	    print(' 0 - Shared Pool WM  ') 	
	    print(' 1 - Global Threshold ') 	
	    print(' 2 - Global MC Threshold ') 	
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            mirrorMode_Ptr_2 = new_xpPtMirrorModes_ep()
            #print('Input Arguments are, devId=%d, state=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetGlobalStateMirrorMode(args[0],args[1],mirrorMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mirrorMode = %d' % (xpPtMirrorModes_ep_value(mirrorMode_Ptr_2)))
                pass
            delete_xpPtMirrorModes_ep(mirrorMode_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnablePortStateMirrorMode
    #/********************************************************************************/
    def do_packet_trakker_enable_port_state_mirror_mode(self, arg):
        '''
         xpsPacketTrakkerEnablePortStateMirrorMode: Enter [ devId,portNum,state,mirrorMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,state,mirrorMode ]')
	    print('Port States: ') 	
	    print(' 0 - Port Max Threshold  ') 	
	    print(' 1 - Port-Queue Dynamic Threshold ') 	
	    print(' 2 - Port-Queue Length WM ') 	
	    print(' 3 - Port-Queue Avg Length WM ') 	
	    print('Mirror Modes: ') 	
	    print(' 0 - No Mirroring ') 	
	    print(' 1 - State Change ') 	
	    print(' 2 - Level Change ') 	
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, state=%d, mirrorMode=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPacketTrakkerEnablePortStateMirrorMode(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPortStateMirrorMode
    #/********************************************************************************/
    def do_packet_trakker_get_port_state_mirror_mode(self, arg):
        '''
         xpsPacketTrakkerGetPortStateMirrorMode: Enter [ devId,portNum,state ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,state ]')
	    print('Port States: ') 	
	    print(' 0 - Port Max Threshold  ') 	
	    print(' 1 - Port-Queue Dynamic Threshold ') 	
	    print(' 2 - Port-Queue Length WM ') 	
	    print(' 3 - Port-Queue Avg Length WM ') 	
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            mirrorMode_Ptr_3 = new_xpPtMirrorModes_ep()
            #print('Input Arguments are, devId=%d, portNum=%d, state=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerGetPortStateMirrorMode(args[0],args[1],args[2],mirrorMode_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mirrorMode = %d' % (xpPtMirrorModes_ep_value(mirrorMode_Ptr_3)))
                pass
            delete_xpPtMirrorModes_ep(mirrorMode_Ptr_3)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnablePeriodicMirror
    #/********************************************************************************/
    def do_packet_trakker_enable_periodic_mirror(self, arg):
        '''
         xpsPacketTrakkerEnablePeriodicMirror: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerEnablePeriodicMirror(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPeriodicMirrorEnable
    #/********************************************************************************/
    def do_packet_trakker_get_periodic_mirror_enable(self, arg):
        '''
         xpsPacketTrakkerGetPeriodicMirrorEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetPeriodicMirrorEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerConfigureMirrorPeriod
    #/********************************************************************************/
    def do_packet_trakker_configure_mirror_period(self, arg):
        '''
         xpsPacketTrakkerConfigureMirrorPeriod: Enter [ devId,portNum,mirrorPeriod ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,mirrorPeriod ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, mirrorPeriod=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerConfigureMirrorPeriod(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetMirrorPeriod
    #/********************************************************************************/
    def do_packet_trakker_get_mirror_period(self, arg):
        '''
         xpsPacketTrakkerGetMirrorPeriod: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            mirrorPeriod_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetMirrorPeriod(args[0],args[1],mirrorPeriod_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mirrorPeriod = %d' % (uint32_tp_value(mirrorPeriod_Ptr_2)))
                pass
            delete_uint32_tp(mirrorPeriod_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnableGlobalShaper
    #/********************************************************************************/
    def do_packet_trakker_enable_global_shaper(self, arg):
        '''
         xpsPacketTrakkerEnableGlobalShaper: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerEnableGlobalShaper(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetGlobalShaperEnable
    #/********************************************************************************/
    def do_packet_trakker_get_global_shaper_enable(self, arg):
        '''
         xpsPacketTrakkerGetGlobalShaperEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetGlobalShaperEnable(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnablePortShaper
    #/********************************************************************************/
    def do_packet_trakker_enable_port_shaper(self, arg):
        '''
         xpsPacketTrakkerEnablePortShaper: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerEnablePortShaper(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPortShaperEnable
    #/********************************************************************************/
    def do_packet_trakker_get_port_shaper_enable(self, arg):
        '''
         xpsPacketTrakkerGetPortShaperEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetPortShaperEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerConfigureGlobalShaper
    #/********************************************************************************/
    def do_packet_trakker_configure_global_shaper(self, arg):
        '''
         xpsPacketTrakkerConfigureGlobalShaper: Enter [ devId,rateKpps,maxBucketSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rateKpps,maxBucketSize ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, rateKpps=%d, maxBucketSize=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerConfigureGlobalShaper(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetGlobalShaperConfiguration
    #/********************************************************************************/
    def do_packet_trakker_get_global_shaper_configuration(self, arg):
        '''
         xpsPacketTrakkerGetGlobalShaperConfiguration: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            rateKpps_Ptr_1 = new_uint32_tp()
            maxBucketSize_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetGlobalShaperConfiguration(args[0],rateKpps_Ptr_1,maxBucketSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rateKpps = %d' % (uint32_tp_value(rateKpps_Ptr_1)))
                print('maxBucketSize = %d' % (uint32_tp_value(maxBucketSize_Ptr_2)))
                pass
            delete_uint32_tp(maxBucketSize_Ptr_2)
            delete_uint32_tp(rateKpps_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerConfigurePortShaper
    #/********************************************************************************/
    def do_packet_trakker_configure_port_shaper(self, arg):
        '''
         xpsPacketTrakkerConfigurePortShaper: Enter [ devId,portNum,rateKpps,maxBucketSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,rateKpps,maxBucketSize ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, rateKpps=%d, maxBucketSize=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPacketTrakkerConfigurePortShaper(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPortShaperConfiguration
    #/********************************************************************************/
    def do_packet_trakker_get_port_shaper_configuration(self, arg):
        '''
         xpsPacketTrakkerGetPortShaperConfiguration: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            rateKpps_Ptr_2 = new_uint32_tp()
            maxBucketSize_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetPortShaperConfiguration(args[0],args[1],rateKpps_Ptr_2,maxBucketSize_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rateKpps = %d' % (uint32_tp_value(rateKpps_Ptr_2)))
                print('maxBucketSize = %d' % (uint32_tp_value(maxBucketSize_Ptr_3)))
                pass
            delete_uint32_tp(maxBucketSize_Ptr_3)
            delete_uint32_tp(rateKpps_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerEnableGlobalAging
    #/********************************************************************************/
    def do_packet_trakker_enable_global_aging(self, arg):
        '''
         xpsPacketTrakkerEnableGlobalAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerEnableGlobalAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetGlobalAgingEnable
    #/********************************************************************************/
    def do_packet_trakker_get_global_aging_enable(self, arg):
        '''
         xpsPacketTrakkerGetGlobalAgingEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetGlobalAgingEnable(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerConfigureGlobalAgingPeriod
    #/********************************************************************************/
    def do_packet_trakker_configure_global_aging_period(self, arg):
        '''
         xpsPacketTrakkerConfigureGlobalAgingPeriod: Enter [ devId,agingPeriod,agingLimit ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,agingPeriod,agingLimit ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, agingPeriod=%d, agingLimit=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerConfigureGlobalAgingPeriod(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetGlobalAgingPeriod
    #/********************************************************************************/
    def do_packet_trakker_get_global_aging_period(self, arg):
        '''
         xpsPacketTrakkerGetGlobalAgingPeriod: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            agingPeriod_Ptr_1 = new_uint32_tp()
            agingLimit_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetGlobalAgingPeriod(args[0],agingPeriod_Ptr_1,agingLimit_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('agingPeriod = %d' % (uint32_tp_value(agingPeriod_Ptr_1)))
                print('agingLimit = %d' % (uint32_tp_value(agingLimit_Ptr_2)))
                pass
            delete_uint32_tp(agingLimit_Ptr_2)
            delete_uint32_tp(agingPeriod_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetPortMaxThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_port_max_threshold(self, arg):
        '''
         xpsPacketTrakkerSetPortMaxThreshold: Enter [ devId,portNum,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, threshold=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerSetPortMaxThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPortMaxThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_port_max_threshold(self, arg):
        '''
         xpsPacketTrakkerGetPortMaxThreshold: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            threshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetPortMaxThreshold(args[0],args[1],threshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_2)))
                pass
            delete_uint32_tp(threshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetPoolWmThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_pool_wm_threshold(self, arg):
        '''
         xpsPacketTrakkerSetPoolWmThreshold: Enter [ devId,sharedPoolId,lowWmThreshold,highWmThreshold,maxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sharedPoolId,lowWmThreshold,highWmThreshold,maxThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, devId=%d, sharedPoolId=%d, lowWmThreshold=%d, highWmThreshold=%d, maxThreshold=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsPacketTrakkerSetPoolWmThreshold(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPoolWmThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_pool_wm_threshold(self, arg):
        '''
         xpsPacketTrakkerGetPoolWmThreshold: Enter [ devId,sharedPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sharedPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            lowWmThreshold_Ptr_2 = new_uint32_tp()
            highWmThreshold_Ptr_3 = new_uint32_tp()
            maxThreshold_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, sharedPoolId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetPoolWmThreshold(args[0],args[1],lowWmThreshold_Ptr_2,highWmThreshold_Ptr_3,maxThreshold_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lowWmThreshold = %d' % (uint32_tp_value(lowWmThreshold_Ptr_2)))
                print('highWmThreshold = %d' % (uint32_tp_value(highWmThreshold_Ptr_3)))
                print('maxThreshold = %d' % (uint32_tp_value(maxThreshold_Ptr_4)))
                pass
            delete_uint32_tp(maxThreshold_Ptr_4)
            delete_uint32_tp(highWmThreshold_Ptr_3)
            delete_uint32_tp(lowWmThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetGlobalPageThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_global_page_threshold(self, arg):
        '''
         xpsPacketTrakkerSetGlobalPageThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerSetGlobalPageThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetGlobalPageThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_global_page_threshold(self, arg):
        '''
         xpsPacketTrakkerGetGlobalPageThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetGlobalPageThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetMulticastPageThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_multicast_page_threshold(self, arg):
        '''
         xpsPacketTrakkerSetMulticastPageThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerSetMulticastPageThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetMulticastPageThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_multicast_page_threshold(self, arg):
        '''
         xpsPacketTrakkerGetMulticastPageThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetMulticastPageThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerCreateAqmQProfile
    #/********************************************************************************/
    def do_packet_trakker_create_aqm_q_profile(self, arg):
        '''
         xpsPacketTrakkerCreateAqmQProfile: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            profileId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerCreateAqmQProfile(args[0],profileId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_1)))
                pass
            delete_uint32_tp(profileId_Ptr_1)
    #/********************************************************************************/
    # command for xpsPacketTrakkerBindAqmQProfileToQueue
    #/********************************************************************************/
    def do_packet_trakker_bind_aqm_q_profile_to_queue(self, arg):
        '''
         xpsPacketTrakkerBindAqmQProfileToQueue: Enter [ devId,portNum,queueNum,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d, profileId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPacketTrakkerBindAqmQProfileToQueue(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetAqmQProfileForQueue
    #/********************************************************************************/
    def do_packet_trakker_get_aqm_q_profile_for_queue(self, arg):
        '''
         xpsPacketTrakkerGetAqmQProfileForQueue: Enter [ devId,portNum,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            profileId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerGetAqmQProfileForQueue(args[0],args[1],args[2],profileId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_3)))
                pass
            delete_uint32_tp(profileId_Ptr_3)
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetAvgQueueWmThreshold
    #/********************************************************************************/
    def do_packet_trakker_set_avg_queue_wm_threshold(self, arg):
        '''
         xpsPacketTrakkerSetAvgQueueWmThreshold: Enter [ devId,profileId,minThreshold,maxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,minThreshold,maxThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, profileId=%d, minThreshold=%d, maxThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPacketTrakkerSetAvgQueueWmThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetAvgQueueWmThreshold
    #/********************************************************************************/
    def do_packet_trakker_get_avg_queue_wm_threshold(self, arg):
        '''
         xpsPacketTrakkerGetAvgQueueWmThreshold: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            minThreshold_Ptr_2 = new_uint32_tp()
            maxThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetAvgQueueWmThreshold(args[0],args[1],minThreshold_Ptr_2,maxThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('minThreshold = %d' % (uint32_tp_value(minThreshold_Ptr_2)))
                print('maxThreshold = %d' % (uint32_tp_value(maxThreshold_Ptr_3)))
                pass
            delete_uint32_tp(maxThreshold_Ptr_3)
            delete_uint32_tp(minThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerUpdatePktTrakkerStartQueueNum
    #/********************************************************************************/
    def do_packet_trakker_update_pkt_trakker_start_queue_num(self, arg):
        '''
         xpsPacketTrakkerUpdatePktTrakkerStartQueueNum: Enter [ devId,portNum,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsPacketTrakkerUpdatePktTrakkerStartQueueNum(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetPktTrakkerStartQueueNum
    #/********************************************************************************/
    def do_packet_trakker_get_pkt_trakker_start_queue_num(self, arg):
        '''
         xpsPacketTrakkerGetPktTrakkerStartQueueNum: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            queueNum_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerGetPktTrakkerStartQueueNum(args[0],args[1],queueNum_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('queueNum = %d' % (uint32_tp_value(queueNum_Ptr_2)))
                pass
            delete_uint32_tp(queueNum_Ptr_2)
    #/********************************************************************************/
    # command for xpsPacketTrakkerAddAnalyzerInterface
    #/********************************************************************************/
    def do_packet_trakker_add_analyzer_interface(self, arg):
        '''
         xpsPacketTrakkerAddAnalyzerInterface: Enter [ devId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, analyzerIntfId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerAddAnalyzerInterface(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerRemoveAnalyzerInterface
    #/********************************************************************************/
    def do_packet_trakker_remove_analyzer_interface(self, arg):
        '''
         xpsPacketTrakkerRemoveAnalyzerInterface: Enter [ devId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, analyzerIntfId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerRemoveAnalyzerInterface(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerSetSwitchIdentifier
    #/********************************************************************************/
    def do_packet_trakker_set_switch_identifier(self, arg):
        '''
         xpsPacketTrakkerSetSwitchIdentifier: Enter [ devId,switchId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,switchId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, switchId=%d' % (args[0],args[1]))
            ret = xpsPacketTrakkerSetSwitchIdentifier(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPacketTrakkerGetSwitchIdentifier
    #/********************************************************************************/
    def do_packet_trakker_get_switch_identifier(self, arg):
        '''
         xpsPacketTrakkerGetSwitchIdentifier: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            switchId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPacketTrakkerGetSwitchIdentifier(args[0],switchId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('switchId = %d' % (uint32_tp_value(switchId_Ptr_1)))
                pass
            delete_uint32_tp(switchId_Ptr_1)
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
