#!/usr/bin/env python
#  xpsQos.py
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
# The class object for xpsQos operations
#/**********************************************************************************/

class xpsQosObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsQosInitScope
    #/********************************************************************************/
    def do_qos_init_scope(self, arg):
        '''
         xpsQosInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsQosInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosDeInitScope
    #/********************************************************************************/
    def do_qos_de_init_scope(self, arg):
        '''
         xpsQosDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsQosDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetRehashLevel
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_rehash_level(self, arg):
        '''
         xpsQosEgressQosMapGetRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosEgressQosMapGetRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # command for xpsQosEgressQosMapSetRehashLevel
    #/********************************************************************************/
    def do_qos_egress_qos_map_set_rehash_level(self, arg):
        '''
         xpsQosEgressQosMapSetRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapSetRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsQosEgressQosMapSetRehashLevel
    #/********************************************************************************/
    def do_qos_egress_qos_map_set_rehash_level(self, arg):
        '''
         xpsQosEgressQosMapSetRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapSetRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetRehashLevel
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_rehash_level(self, arg):
        '''
         xpsQosEgressQosMapGetRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosEgressQosMapGetRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAddDevice
    #/********************************************************************************/
    def do_qos_add_device(self, arg):
        '''
         xpsQosAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsQosAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosRemoveDevice
    #/********************************************************************************/
    def do_qos_remove_device(self, arg):
        '''
         xpsQosRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmCreateAqmQProfile
    #/********************************************************************************/
    def do_qos_aqm_create_aqm_q_profile(self, arg):
        '''
         xpsQosAqmCreateAqmQProfile: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            profileId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmCreateAqmQProfile(args[0],profileId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_1)))
                pass
            delete_uint32_tp(profileId_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmDeleteAqmQProfile
    #/********************************************************************************/
    def do_qos_aqm_delete_aqm_q_profile(self, arg):
        '''
         xpsQosAqmDeleteAqmQProfile: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDeleteAqmQProfile(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmCreateAqmProfile
    #/********************************************************************************/
    def do_qos_aqm_create_aqm_profile(self, arg):
        '''
         xpsQosAqmCreateAqmProfile: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            profileId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmCreateAqmProfile(args[0],profileId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_1)))
                pass
            delete_uint32_tp(profileId_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmDeleteAqmProfile
    #/********************************************************************************/
    def do_qos_aqm_delete_aqm_profile(self, arg):
        '''
         xpsQosAqmDeleteAqmProfile: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDeleteAqmProfile(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmSetEnqueueEnable
    #/********************************************************************************/
    def do_qos_aqm_set_enqueue_enable(self, arg):
        '''
         xpsQosAqmSetEnqueueEnable: Enter [ devId,devPort,queueNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmSetEnqueueEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetEnqueueEnable
    #/********************************************************************************/
    def do_qos_aqm_get_enqueue_enable(self, arg):
        '''
         xpsQosAqmGetEnqueueEnable: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetEnqueueEnable(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmBindAqmQProfileToQueue
    #/********************************************************************************/
    def do_qos_aqm_bind_aqm_q_profile_to_queue(self, arg):
        '''
         xpsQosAqmBindAqmQProfileToQueue: Enter [ devId,devPort,queueNum,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, profileId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmBindAqmQProfileToQueue(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmBindAqmQProfileToColorAwareQueue
    #/********************************************************************************/
    def do_qos_aqm_bind_aqm_q_profile_to_color_aware_queue(self, arg):
        '''
         xpsQosAqmBindAqmQProfileToColorAwareQueue: Enter [ devId,devPort,queueNum,color,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,color,profileId ]')
            print('Color:')
            print('  GREEN  = 0')
            print('  YELLOW = 1')
            print('  RED    = 2')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, color=%d, profileId=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosAqmBindAqmQProfileToColorAwareQueue(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmBindAqmProfileToPort
    #/********************************************************************************/
    def do_qos_aqm_bind_aqm_profile_to_port(self, arg):
        '''
         xpsQosAqmBindAqmProfileToPort: Enter [ devId,devPort,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, profileId=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmBindAqmProfileToPort(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetAqmQProfileForQueue
    #/********************************************************************************/
    def do_qos_aqm_get_aqm_q_profile_for_queue(self, arg):
        '''
         xpsQosAqmGetAqmQProfileForQueue: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            profileId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetAqmQProfileForQueue(args[0],args[1],args[2],profileId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_3)))
                pass
            delete_uint32_tp(profileId_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmGetAqmQProfileForColorAwareQueue
    #/********************************************************************************/
    def do_qos_aqm_get_aqm_q_profile_for_color_aware_queue(self, arg):
        '''
         xpsQosAqmGetAqmQProfileForColorAwareQueue: Enter [ devId,devPort,queueNum,color ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,color ]')
            print('Color:')
            print('  GREEN  = 0')
            print('  YELLOW = 1')
            print('  RED    = 2')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            profileId_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, color=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmGetAqmQProfileForColorAwareQueue(args[0],args[1],args[2],args[3],profileId_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_4)))
                pass
            delete_uint32_tp(profileId_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosAqmGetAqmProfileForPort
    #/********************************************************************************/
    def do_qos_aqm_get_aqm_profile_for_port(self, arg):
        '''
         xpsQosAqmGetAqmProfileForPort: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            profileId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetAqmProfileForPort(args[0],args[1],args[2],profileId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profileId = %d' % (uint32_tp_value(profileId_Ptr_3)))
                pass
            delete_uint32_tp(profileId_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmConfigureWred
    #/********************************************************************************/
    def do_qos_aqm_configure_wred(self, arg):
        '''
         xpsQosAqmConfigureWred: Enter [ devId,profileId,minThreshold,maxThreshold,weight,maxDropProb ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,minThreshold,maxThreshold,weight,maxDropProb ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, profileId=%d, minThreshold=%d, maxThreshold=%d, weight=%d, maxDropProb=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosAqmConfigureWred(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetWredConfiguration
    #/********************************************************************************/
    def do_qos_aqm_get_wred_configuration(self, arg):
        '''
         xpsQosAqmGetWredConfiguration: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            minThreshold_Ptr_2 = new_uint32_tp()
            maxThreshold_Ptr_3 = new_uint32_tp()
            avgQLengthWeight_Ptr_4 = new_uint32_tp()
            maxDropProb_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetWredConfiguration(args[0],args[1],minThreshold_Ptr_2,maxThreshold_Ptr_3,avgQLengthWeight_Ptr_4,maxDropProb_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('minThreshold = %d' % (uint32_tp_value(minThreshold_Ptr_2)))
                print('maxThreshold = %d' % (uint32_tp_value(maxThreshold_Ptr_3)))
                print('avgQLengthWeight = %d' % (uint32_tp_value(avgQLengthWeight_Ptr_4)))
                print('maxDropProb = %d' % (uint32_tp_value(maxDropProb_Ptr_5)))
                pass
            delete_uint32_tp(maxDropProb_Ptr_5)
            delete_uint32_tp(avgQLengthWeight_Ptr_4)
            delete_uint32_tp(maxThreshold_Ptr_3)
            delete_uint32_tp(minThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmConfigureWredMode
    #/********************************************************************************/
    def do_qos_aqm_configure_wred_mode(self, arg):
        '''
         xpsQosAqmConfigureWredMode: Enter [ devId,profileId,wredMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,wredMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, wredMode=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmConfigureWredMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetWredMode
    #/********************************************************************************/
    def do_qos_aqm_get_wred_mode(self, arg):
        '''
         xpsQosAqmGetWredMode: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            wredMode_Ptr_2 = new_xpWredMode_ep()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetWredMode(args[0],args[1],wredMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('wredMode = %d' % (xpWredMode_ep_value(wredMode_Ptr_2)))
                pass
            delete_xpWredMode_ep(wredMode_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayWredParams
    #/********************************************************************************/
    def do_qos_aqm_display_wred_params(self, arg):
        '''
         xpsQosAqmDisplayWredParams: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayWredParams(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmConfigureQueueDctcpMarkThreshold
    #/********************************************************************************/
    def do_qos_aqm_configure_queue_dctcp_mark_threshold(self, arg):
        '''
         xpsQosAqmConfigureQueueDctcpMarkThreshold: Enter [ devId,profileId,markThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,markThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, markThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmConfigureQueueDctcpMarkThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueDctcpMarkThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_queue_dctcp_mark_threshold(self, arg):
        '''
         xpsQosAqmGetQueueDctcpMarkThreshold: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            markThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetQueueDctcpMarkThreshold(args[0],args[1],markThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('markThreshold = %d' % (uint32_tp_value(markThreshold_Ptr_2)))
                pass
            delete_uint32_tp(markThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmSetQueueDctcpEnable
    #/********************************************************************************/
    def do_qos_aqm_set_queue_dctcp_enable(self, arg):
        '''
         xpsQosAqmSetQueueDctcpEnable: Enter [ devId,profileId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmSetQueueDctcpEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueDctcpEnable
    #/********************************************************************************/
    def do_qos_aqm_get_queue_dctcp_enable(self, arg):
        '''
         xpsQosAqmGetQueueDctcpEnable: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetQueueDctcpEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayQueueDctcpParams
    #/********************************************************************************/
    def do_qos_aqm_display_queue_dctcp_params(self, arg):
        '''
         xpsQosAqmDisplayQueueDctcpParams: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayQueueDctcpParams(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmConfigurePortDctcpMarkThreshold
    #/********************************************************************************/
    def do_qos_aqm_configure_port_dctcp_mark_threshold(self, arg):
        '''
         xpsQosAqmConfigurePortDctcpMarkThreshold: Enter [ devId,profileId,markThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,markThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, markThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmConfigurePortDctcpMarkThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPortDctcpMarkThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_port_dctcp_mark_threshold(self, arg):
        '''
         xpsQosAqmGetPortDctcpMarkThreshold: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            markThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPortDctcpMarkThreshold(args[0],args[1],markThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('markThreshold = %d' % (uint32_tp_value(markThreshold_Ptr_2)))
                pass
            delete_uint32_tp(markThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmSetPortDctcpEnable
    #/********************************************************************************/
    def do_qos_aqm_set_port_dctcp_enable(self, arg):
        '''
         xpsQosAqmSetPortDctcpEnable: Enter [ devId,profileId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmSetPortDctcpEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPortDctcpEnable
    #/********************************************************************************/
    def do_qos_aqm_get_port_dctcp_enable(self, arg):
        '''
         xpsQosAqmGetPortDctcpEnable: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPortDctcpEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayPortDctcpParams
    #/********************************************************************************/
    def do_qos_aqm_display_port_dctcp_params(self, arg):
        '''
         xpsQosAqmDisplayPortDctcpParams: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayPortDctcpParams(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmConfigureQueuePageTailDropThresholdForProfile
    #/********************************************************************************/
    def do_qos_aqm_configure_queue_page_tail_drop_threshold_for_profile(self, arg):
        '''
         xpsQosAqmConfigureQueuePageTailDropThresholdForProfile: Enter [ devId,profileId,lengthMaxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,lengthMaxThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, lengthMaxThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmConfigureQueuePageTailDropThresholdForProfile(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmConfigureQueuePageTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_configure_queue_page_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmConfigureQueuePageTailDropThreshold: Enter [ devId,devPort,queueNum,lengthMaxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,lengthMaxThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, lengthMaxThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmConfigureQueuePageTailDropThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmConfigureQueuePageTailDropThresholdForColor
    #/********************************************************************************/
    def do_qos_aqm_configure_queue_page_tail_drop_threshold_for_color(self, arg):
        '''
         xpsQosAqmConfigureQueuePageTailDropThresholdForColor: Enter [ devId,devPort,queueNum,color,lengthMaxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,color,lengthMaxThreshold ]')
            print('Color:')
            print('  GREEN  = 0')
            print('  YELLOW = 1')
            print('  RED    = 2')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, color=%d, lengthMaxThreshold=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosAqmConfigureQueuePageTailDropThresholdForColor(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueuePageTailDropThresholdForProfile
    #/********************************************************************************/
    def do_qos_aqm_get_queue_page_tail_drop_threshold_for_profile(self, arg):
        '''
         xpsQosAqmGetQueuePageTailDropThresholdForProfile: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lengthMaxThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetQueuePageTailDropThresholdForProfile(args[0],args[1],lengthMaxThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lengthMaxThreshold = %d' % (uint32_tp_value(lengthMaxThreshold_Ptr_2)))
                pass
            delete_uint32_tp(lengthMaxThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmGetQueuePageTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_queue_page_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmGetQueuePageTailDropThreshold: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            lengthMaxThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetQueuePageTailDropThreshold(args[0],args[1],args[2],lengthMaxThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lengthMaxThreshold = %d' % (uint32_tp_value(lengthMaxThreshold_Ptr_3)))
                pass
            delete_uint32_tp(lengthMaxThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmGetQueuePageTailDropThresholdForColor
    #/********************************************************************************/
    def do_qos_aqm_get_queue_page_tail_drop_threshold_for_color(self, arg):
        '''
         xpsQosAqmGetQueuePageTailDropThresholdForColor: Enter [ devId,devPort,queueNum,color ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,color ]')
            print('Color:')
            print('  GREEN  = 0')
            print('  YELLOW = 1')
            print('  RED    = 2')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            lengthMaxThreshold_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, color=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmGetQueuePageTailDropThresholdForColor(args[0],args[1],args[2],args[3],lengthMaxThreshold_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lengthMaxThreshold = %d' % (uint32_tp_value(lengthMaxThreshold_Ptr_4)))
                pass
            delete_uint32_tp(lengthMaxThreshold_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosAqmUseProfileBasedTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_use_profile_based_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmUseProfileBasedTailDropThreshold: Enter [ devId,profileId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmUseProfileBasedTailDropThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetProfileBasedTailDropThresholdEnable
    #/********************************************************************************/
    def do_qos_aqm_get_profile_based_tail_drop_threshold_enable(self, arg):
        '''
         xpsQosAqmGetProfileBasedTailDropThresholdEnable: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetProfileBasedTailDropThresholdEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayQueueTailDropParams
    #/********************************************************************************/
    def do_qos_aqm_display_queue_tail_drop_params(self, arg):
        '''
         xpsQosAqmDisplayQueueTailDropParams: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayQueueTailDropParams(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmConfigurePortPageTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_configure_port_page_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmConfigurePortPageTailDropThreshold: Enter [ devId,profileId,lengthMaxThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,lengthMaxThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, lengthMaxThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmConfigurePortPageTailDropThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPortPageTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_port_page_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmGetPortPageTailDropThreshold: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lengthMaxThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPortPageTailDropThreshold(args[0],args[1],lengthMaxThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lengthMaxThreshold = %d' % (uint32_tp_value(lengthMaxThreshold_Ptr_2)))
                pass
            delete_uint32_tp(lengthMaxThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayPortTailDropParams
    #/********************************************************************************/
    def do_qos_aqm_display_port_tail_drop_params(self, arg):
        '''
         xpsQosAqmDisplayPortTailDropParams: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayPortTailDropParams(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmSetPhantomQEnable
    #/********************************************************************************/
    def do_qos_aqm_set_phantom_q_enable(self, arg):
        '''
         xpsQosAqmSetPhantomQEnable: Enter [ devId,profileId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmSetPhantomQEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPhantomQEnable
    #/********************************************************************************/
    def do_qos_aqm_get_phantom_q_enable(self, arg):
        '''
         xpsQosAqmGetPhantomQEnable: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPhantomQEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayPhantomQParams
    #/********************************************************************************/
    def do_qos_aqm_display_phantom_q_params(self, arg):
        '''
         xpsQosAqmDisplayPhantomQParams: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayPhantomQParams(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmSetQueueDynamicPoolEnable
    #/********************************************************************************/
    def do_qos_aqm_set_queue_dynamic_pool_enable(self, arg):
        '''
         xpsQosAqmSetQueueDynamicPoolEnable: Enter [ devId,devPort,queueNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmSetQueueDynamicPoolEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueDynamicPoolEnable
    #/********************************************************************************/
    def do_qos_aqm_get_queue_dynamic_pool_enable(self, arg):
        '''
         xpsQosAqmGetQueueDynamicPoolEnable: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetQueueDynamicPoolEnable(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmBindQueueToDynamicPool
    #/********************************************************************************/
    def do_qos_aqm_bind_queue_to_dynamic_pool(self, arg):
        '''
         xpsQosAqmBindQueueToDynamicPool: Enter [ devId,devPort,queueNum,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, dynPoolId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmBindQueueToDynamicPool(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueToDynamicPoolBinding
    #/********************************************************************************/
    def do_qos_aqm_get_queue_to_dynamic_pool_binding(self, arg):
        '''
         xpsQosAqmGetQueueToDynamicPoolBinding: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            dynPoolId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetQueueToDynamicPoolBinding(args[0],args[1],args[2],dynPoolId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dynPoolId = %d' % (uint32_tp_value(dynPoolId_Ptr_3)))
                pass
            delete_uint32_tp(dynPoolId_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmSetPortDynamicPoolEnable
    #/********************************************************************************/
    def do_qos_aqm_set_port_dynamic_pool_enable(self, arg):
        '''
         xpsQosAqmSetPortDynamicPoolEnable: Enter [ devId,devPort,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmSetPortDynamicPoolEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPortDynamicPoolEnable
    #/********************************************************************************/
    def do_qos_aqm_get_port_dynamic_pool_enable(self, arg):
        '''
         xpsQosAqmGetPortDynamicPoolEnable: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPortDynamicPoolEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmBindPortToDynamicPool
    #/********************************************************************************/
    def do_qos_aqm_bind_port_to_dynamic_pool(self, arg):
        '''
         xpsQosAqmBindPortToDynamicPool: Enter [ devId,devPort,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, dynPoolId=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmBindPortToDynamicPool(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPortToDynamicPoolBinding
    #/********************************************************************************/
    def do_qos_aqm_get_port_to_dynamic_pool_binding(self, arg):
        '''
         xpsQosAqmGetPortToDynamicPoolBinding: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dynPoolId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPortToDynamicPoolBinding(args[0],args[1],dynPoolId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dynPoolId = %d' % (uint32_tp_value(dynPoolId_Ptr_2)))
                pass
            delete_uint32_tp(dynPoolId_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmSetDynamicPoolThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_dynamic_pool_threshold(self, arg):
        '''
         xpsQosAqmSetDynamicPoolThreshold: Enter [ devId,dynPoolId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, threshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmSetDynamicPoolThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetDynamicPoolThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_dynamic_pool_threshold(self, arg):
        '''
         xpsQosAqmGetDynamicPoolThreshold: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            threshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetDynamicPoolThreshold(args[0],args[1],threshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_2)))
                pass
            delete_uint32_tp(threshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmSetDynamicPoolTotalThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_dynamic_pool_total_threshold(self, arg):
        '''
         xpsQosAqmSetDynamicPoolTotalThreshold: Enter [ devId,dynPoolId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, threshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmSetDynamicPoolTotalThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetDynamicPoolTotalThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_dynamic_pool_total_threshold(self, arg):
        '''
         xpsQosAqmGetDynamicPoolTotalThreshold: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            threshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetDynamicPoolTotalThreshold(args[0],args[1],threshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_2)))
                pass
            delete_uint32_tp(threshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmConfigureDynamicPoolAlpha
    #/********************************************************************************/
    def do_qos_aqm_configure_dynamic_pool_alpha(self, arg):
        '''
         xpsQosAqmConfigureDynamicPoolAlpha: Enter [ devId,dynPoolId,fraction,coefficient ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,fraction,coefficient ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, fraction=%d, coefficient=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmConfigureDynamicPoolAlpha(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetDynamicPoolAlpha
    #/********************************************************************************/
    def do_qos_aqm_get_dynamic_pool_alpha(self, arg):
        '''
         xpsQosAqmGetDynamicPoolAlpha: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fraction_Ptr_2 = new_xpDynThldFraction_ep()
            coefficient_Ptr_3 = new_xpDynThldCoeff_ep()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetDynamicPoolAlpha(args[0],args[1],fraction_Ptr_2,coefficient_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fraction = %d' % (xpDynThldFraction_ep_value(fraction_Ptr_2)))
                print('coefficient = %d' % (xpDynThldCoeff_ep_value(coefficient_Ptr_3)))
                pass
            delete_xpDynThldCoeff_ep(coefficient_Ptr_3)
            delete_xpDynThldFraction_ep(fraction_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmConfigureDynamicPoolAlphaForColor
    #/********************************************************************************/
    def do_qos_aqm_configure_dynamic_pool_alpha_for_color(self, arg):
        '''
         xpsQosAqmConfigureDynamicPoolAlphaForColor: Enter [ devId,dynPoolId,color,fraction,coefficient ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,color,fraction,coefficient ]')
            print('Color:')
            print('  GREEN  = 0')
            print('  YELLOW = 1')
            print('  RED    = 2')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, color=%d, fraction=%d, coefficient=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosAqmConfigureDynamicPoolAlphaForColor(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetDynamicPoolAlphaForColor
    #/********************************************************************************/
    def do_qos_aqm_get_dynamic_pool_alpha_for_color(self, arg):
        '''
         xpsQosAqmGetDynamicPoolAlphaForColor: Enter [ devId,dynPoolId,color ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,color ]')
            print('Color:')
            print('  GREEN  = 0')
            print('  YELLOW = 1')
            print('  RED    = 2')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            fraction_Ptr_3 = new_xpDynThldFraction_ep()
            coefficient_Ptr_4 = new_xpDynThldCoeff_ep()
            #print('Input Arguments are, devId=%d, dynPoolId=%d, color=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetDynamicPoolAlphaForColor(args[0],args[1],args[2],fraction_Ptr_3,coefficient_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fraction = %d' % (xpDynThldFraction_ep_value(fraction_Ptr_3)))
                print('coefficient = %d' % (xpDynThldCoeff_ep_value(coefficient_Ptr_4)))
                pass
            delete_xpDynThldCoeff_ep(coefficient_Ptr_4)
            delete_xpDynThldFraction_ep(fraction_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmGetDynamicPoolCurrentSharedUsed
    #/********************************************************************************/
    def do_qos_aqm_get_dynamic_pool_current_shared_used(self, arg):
        '''
         xpsQosAqmGetDynamicPoolCurrentSharedUsed: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetDynamicPoolCurrentSharedUsed(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmGetDynamicPoolCurrentTotalUsed
    #/********************************************************************************/
    def do_qos_aqm_get_dynamic_pool_current_total_used(self, arg):
        '''
         xpsQosAqmGetDynamicPoolCurrentTotalUsed: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetDynamicPoolCurrentTotalUsed(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmSetGlobalPacketThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_global_packet_threshold(self, arg):
        '''
         xpsQosAqmSetGlobalPacketThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetGlobalPacketThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetGlobalPacketThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_global_packet_threshold(self, arg):
        '''
         xpsQosAqmGetGlobalPacketThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetGlobalPacketThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmSetGlobalPageThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_global_page_threshold(self, arg):
        '''
         xpsQosAqmSetGlobalPageThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetGlobalPageThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetGlobalPageThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_global_page_threshold(self, arg):
        '''
         xpsQosAqmGetGlobalPageThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetGlobalPageThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmSetMulticastPacketThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_multicast_packet_threshold(self, arg):
        '''
         xpsQosAqmSetMulticastPacketThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetMulticastPacketThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetMulticastPacketThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_multicast_packet_threshold(self, arg):
        '''
         xpsQosAqmGetMulticastPacketThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetMulticastPacketThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmSetMulticastPageThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_multicast_page_threshold(self, arg):
        '''
         xpsQosAqmSetMulticastPageThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetMulticastPageThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetMulticastPageThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_multicast_page_threshold(self, arg):
        '''
         xpsQosAqmGetMulticastPageThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetMulticastPageThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmSetGlobalEcnThresholdEnable
    #/********************************************************************************/
    def do_qos_aqm_set_global_ecn_threshold_enable(self, arg):
        '''
         xpsQosAqmSetGlobalEcnThresholdEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetGlobalEcnThresholdEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetGlobalEcnThresholdEnable
    #/********************************************************************************/
    def do_qos_aqm_get_global_ecn_threshold_enable(self, arg):
        '''
         xpsQosAqmGetGlobalEcnThresholdEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetGlobalEcnThresholdEnable(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmSetGlobalEcnThreshold
    #/********************************************************************************/
    def do_qos_aqm_set_global_ecn_threshold(self, arg):
        '''
         xpsQosAqmSetGlobalEcnThreshold: Enter [ devId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, threshold=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetGlobalEcnThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetGlobalEcnThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_global_ecn_threshold(self, arg):
        '''
         xpsQosAqmGetGlobalEcnThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            threshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetGlobalEcnThreshold(args[0],threshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_1)))
                pass
            delete_uint32_tp(threshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmConfigureQueuePacketTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_configure_queue_packet_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmConfigureQueuePacketTailDropThreshold: Enter [ devId,devPort,queueNum,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, threshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmConfigureQueuePacketTailDropThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueuePacketTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_queue_packet_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmGetQueuePacketTailDropThreshold: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            threshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetQueuePacketTailDropThreshold(args[0],args[1],args[2],threshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_3)))
                pass
            delete_uint32_tp(threshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmConfigurePortPacketTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_configure_port_packet_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmConfigurePortPacketTailDropThreshold: Enter [ devId,devPort,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, threshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmConfigurePortPacketTailDropThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetPortPacketTailDropThreshold
    #/********************************************************************************/
    def do_qos_aqm_get_port_packet_tail_drop_threshold(self, arg):
        '''
         xpsQosAqmGetPortPacketTailDropThreshold: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            threshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetPortPacketTailDropThreshold(args[0],args[1],threshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_2)))
                pass
            delete_uint32_tp(threshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmGetQmapTableIndex
    #/********************************************************************************/
    def do_qos_aqm_get_qmap_table_index(self, arg):
        '''
         xpsQosAqmGetQmapTableIndex: Enter [ devId,destPort,srcPort,mc,txqSelVec,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,destPort,srcPort,mc,txqSelVec,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            qmapIdx_Ptr_6 = new_uint32_tp()
            queueLoc_Ptr_7 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, destPort=%d, srcPort=%d, mc=%d, txqSelVec=%d, tc=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosAqmGetQmapTableIndex(args[0],args[1],args[2],args[3],args[4],args[5],qmapIdx_Ptr_6,queueLoc_Ptr_7)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('qmapIdx = %d' % (uint32_tp_value(qmapIdx_Ptr_6)))
                print('queueLoc = %d' % (uint32_tp_value(queueLoc_Ptr_7)))
                pass
            delete_uint32_tp(queueLoc_Ptr_7)
            delete_uint32_tp(qmapIdx_Ptr_6)
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueAbsoluteNumber
    #/********************************************************************************/
    def do_qos_aqm_get_queue_absolute_number(self, arg):
        '''
         xpsQosAqmGetQueueAbsoluteNumber: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            absQNum_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetQueueAbsoluteNumber(args[0],args[1],args[2],absQNum_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('absQNum = %d' % (uint32_tp_value(absQNum_Ptr_3)))
                pass
            delete_uint32_tp(absQNum_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueAtQmapIndex
    #/********************************************************************************/
    def do_qos_aqm_get_queue_at_qmap_index(self, arg):
        '''
         xpsQosAqmGetQueueAtQmapIndex: Enter [ devId,qmapIdx,queueLoc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,qmapIdx,queueLoc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            absQNum_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, qmapIdx=%d, queueLoc=%d' % (args[0],args[1],args[2]))
            ret = xpsQosAqmGetQueueAtQmapIndex(args[0],args[1],args[2],absQNum_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('absQNum = %d' % (uint32_tp_value(absQNum_Ptr_3)))
                pass
            delete_uint32_tp(absQNum_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosAqmSetQueueAtQmapIndex
    #/********************************************************************************/
    def do_qos_aqm_set_queue_at_qmap_index(self, arg):
        '''
         xpsQosAqmSetQueueAtQmapIndex: Enter [ devId,qmapIdx,queueLoc,absQNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,qmapIdx,queueLoc,absQNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, qmapIdx=%d, queueLoc=%d, absQNum=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosAqmSetQueueAtQmapIndex(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetAqmQTotalProfiles
    #/********************************************************************************/
    def do_qos_aqm_get_aqm_q_total_profiles(self, arg):
        '''
         xpsQosAqmGetAqmQTotalProfiles: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numProfiles_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetAqmQTotalProfiles(args[0],numProfiles_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numProfiles = %d' % (uint32_tp_value(numProfiles_Ptr_1)))
                pass
            delete_uint32_tp(numProfiles_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmGetAqmPortTotalProfiles
    #/********************************************************************************/
    def do_qos_aqm_get_aqm_port_total_profiles(self, arg):
        '''
         xpsQosAqmGetAqmPortTotalProfiles: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numProfiles_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetAqmPortTotalProfiles(args[0],numProfiles_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numProfiles = %d' % (uint32_tp_value(numProfiles_Ptr_1)))
                pass
            delete_uint32_tp(numProfiles_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmGetNumberOfQueuesPerPort
    #/********************************************************************************/
    def do_qos_aqm_get_number_of_queues_per_port(self, arg):
        '''
         xpsQosAqmGetNumberOfQueuesPerPort: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            numQs_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetNumberOfQueuesPerPort(args[0],args[1],numQs_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numQs = %d' % (uint32_tp_value(numQs_Ptr_2)))
                pass
            delete_uint32_tp(numQs_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmGetNumberOfUnicastQueuesPerPort
    #/********************************************************************************/
    def do_qos_aqm_get_number_of_unicast_queues_per_port(self, arg):
        '''
         xpsQosAqmGetNumberOfUnicastQueuesPerPort: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            numUcQs_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetNumberOfUnicastQueuesPerPort(args[0],args[1],numUcQs_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numUcQs = %d' % (uint32_tp_value(numUcQs_Ptr_2)))
                pass
            delete_uint32_tp(numUcQs_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmGetNumberOfMulticastQueuesPerPort
    #/********************************************************************************/
    def do_qos_aqm_get_number_of_multicast_queues_per_port(self, arg):
        '''
         xpsQosAqmGetNumberOfMulticastQueuesPerPort: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            numMcQs_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosAqmGetNumberOfMulticastQueuesPerPort(args[0],args[1],numMcQs_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numMcQs = %d' % (uint32_tp_value(numMcQs_Ptr_2)))
                pass
            delete_uint32_tp(numMcQs_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosAqmSetQueueGuarThresholdGranularity
    #/********************************************************************************/
    def do_qos_aqm_set_queue_guar_threshold_granularity(self, arg):
        '''
         xpsQosAqmSetQueueGuarThresholdGranularity: Enter [ devId,granularity ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,granularity ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, granularity=%d' % (args[0],args[1]))
            ret = xpsQosAqmSetQueueGuarThresholdGranularity(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmGetQueueGuarThresholdGranularity
    #/********************************************************************************/
    def do_qos_aqm_get_queue_guar_threshold_granularity(self, arg):
        '''
         xpsQosAqmGetQueueGuarThresholdGranularity: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            granularity_Ptr_1 = new_xpQGuarThresholdGranularity_ep()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosAqmGetQueueGuarThresholdGranularity(args[0],granularity_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('granularity = %d' % (xpQGuarThresholdGranularity_ep_value(granularity_Ptr_1)))
                pass
            delete_xpQGuarThresholdGranularity_ep(granularity_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosAqmDisplayAqmProfile
    #/********************************************************************************/
    def do_qos_aqm_display_aqm_profile(self, arg):
        '''
         xpsQosAqmDisplayAqmProfile: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayAqmProfile(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosAqmDisplayAqmQProfile
    #/********************************************************************************/
    def do_qos_aqm_display_aqm_q_profile(self, arg):
        '''
         xpsQosAqmDisplayAqmQProfile: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosAqmDisplayAqmQProfile(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScSetReasonCodeTableEntry
    #/********************************************************************************/
    def do_qos_cpu_sc_set_reason_code_table_entry(self, arg):
        '''
         xpsQosCpuScSetReasonCodeTableEntry: Enter [ devId,index,destPort,tc,enableTruncation,enableMod ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,destPort,tc,enableTruncation,enableMod ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, index=%d, destPort=%d, tc=%d, enableTruncation=%d, enableMod=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosCpuScSetReasonCodeTableEntry(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScSetRctDestPort
    #/********************************************************************************/
    def do_qos_cpu_sc_set_rct_dest_port(self, arg):
        '''
         xpsQosCpuScSetRctDestPort: Enter [ devId,index,destPort ]
         Pass (greater or equal) 1024 as index, if wants to set the same destPort against all the reason codes
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,destPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, destPort=%d' % (args[0],args[1],args[2]))
            if args[1] < 1024:
                #print('Input Arguments are, devId=%d, index=%d, destPort=%d' % (args[0],args[1],args[2]))
                ret = xpsQosCpuScSetRctDestPort(args[0],args[1],args[2])
            else:
                print "Reset All\n"
                for rsnCode in range(0, 1024):
                    ret = xpsQosCpuScSetRctDestPort(args[0],rsnCode,args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScSetRctTc
    #/********************************************************************************/
    def do_qos_cpu_sc_set_rct_tc(self, arg):
        '''
         xpsQosCpuScSetRctTc: Enter [ devId,index,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, tc=%d' % (args[0],args[1],args[2]))
            ret = xpsQosCpuScSetRctTc(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScSetRctTruncation
    #/********************************************************************************/
    def do_qos_cpu_sc_set_rct_truncation(self, arg):
        '''
         xpsQosCpuScSetRctTruncation: Enter [ devId,index,enableTruncation ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,enableTruncation ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, enableTruncation=%d' % (args[0],args[1],args[2]))
            ret = xpsQosCpuScSetRctTruncation(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScSetRctEnableMod
    #/********************************************************************************/
    def do_qos_cpu_sc_set_rct_enable_mod(self, arg):
        '''
         xpsQosCpuScSetRctEnableMod: Enter [ devId,index,enableMod ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,enableMod ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, enableMod=%d' % (args[0],args[1],args[2]))
            ret = xpsQosCpuScSetRctEnableMod(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScGetReasonCodeTableEntry
    #/********************************************************************************/
    def do_qos_cpu_sc_get_reason_code_table_entry(self, arg):
        '''
         xpsQosCpuScGetReasonCodeTableEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            destPort_Ptr_2 = new_uint32_tp()
            tc_Ptr_3 = new_uint32_tp()
            enableTruncation_Ptr_4 = new_uint32_tp()
            enableMod_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosCpuScGetReasonCodeTableEntry(args[0],args[1],destPort_Ptr_2,tc_Ptr_3,enableTruncation_Ptr_4,enableMod_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('destPort = %d' % (uint32_tp_value(destPort_Ptr_2)))
                print('tc = %d' % (uint32_tp_value(tc_Ptr_3)))
                print('enableTruncation = %d' % (uint32_tp_value(enableTruncation_Ptr_4)))
                print('enableMod = %d' % (uint32_tp_value(enableMod_Ptr_5)))
                pass
            delete_uint32_tp(enableMod_Ptr_5)
            delete_uint32_tp(enableTruncation_Ptr_4)
            delete_uint32_tp(tc_Ptr_3)
            delete_uint32_tp(destPort_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScGetRctDestPort
    #/********************************************************************************/
    def do_qos_cpu_sc_get_rct_dest_port(self, arg):
        '''
         xpsQosCpuScGetRctDestPort: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            destPort_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosCpuScGetRctDestPort(args[0],args[1],destPort_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('destPort = %d' % (uint32_tp_value(destPort_Ptr_2)))
                pass
            delete_uint32_tp(destPort_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScGetRctTc
    #/********************************************************************************/
    def do_qos_cpu_sc_get_rct_tc(self, arg):
        '''
         xpsQosCpuScGetRctTc: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tc_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosCpuScGetRctTc(args[0],args[1],tc_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tc = %d' % (uint32_tp_value(tc_Ptr_2)))
                pass
            delete_uint32_tp(tc_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScGetRctTruncation
    #/********************************************************************************/
    def do_qos_cpu_sc_get_rct_truncation(self, arg):
        '''
         xpsQosCpuScGetRctTruncation: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enableTruncation_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosCpuScGetRctTruncation(args[0],args[1],enableTruncation_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableTruncation = %d' % (uint32_tp_value(enableTruncation_Ptr_2)))
                pass
            delete_uint32_tp(enableTruncation_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScGetRctEnableMod
    #/********************************************************************************/
    def do_qos_cpu_sc_get_rct_enable_mod(self, arg):
        '''
         xpsQosCpuScGetRctEnableMod: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enableMod_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosCpuScGetRctEnableMod(args[0],args[1],enableMod_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableMod = %d' % (uint32_tp_value(enableMod_Ptr_2)))
                pass
            delete_uint32_tp(enableMod_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScGetQueueGroupId
    #/********************************************************************************/
    def do_qos_cpu_sc_get_queue_group_id(self, arg):
        '''
         xpsQosCpuScGetQueueGroupId: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            queueGroupId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosCpuScGetQueueGroupId(args[0],args[1],args[2],queueGroupId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('queueGroupId = %d' % (uint32_tp_value(queueGroupId_Ptr_3)))
                pass
            delete_uint32_tp(queueGroupId_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosCpuScGetDmaQueueGroupId
    #/********************************************************************************/
    def do_qos_cpu_sc_get_dma_queue_group_id(self, arg):
        '''
         xpsQosCpuScGetDmaQueueGroupId: Enter [ devId,dmaQNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dmaQNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dmaQueueGroupId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dmaQNum=%d' % (args[0],args[1]))
            ret = xpsQosCpuScGetDmaQueueGroupId(args[0],args[1],dmaQueueGroupId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dmaQueueGroupId = %d' % (uint32_tp_value(dmaQueueGroupId_Ptr_2)))
                pass
            delete_uint32_tp(dmaQueueGroupId_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScCpuQGrpToDmaQGrpMapAdd
    #/********************************************************************************/
    def do_qos_cpu_sc_cpu_q_grp_to_dma_q_grp_map_add(self, arg):
        '''
         xpsQosCpuScCpuQGrpToDmaQGrpMapAdd: Enter [ devId,cpuQueueGroupNum,dmaQueueGroupNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,cpuQueueGroupNum,dmaQueueGroupNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, cpuQueueGroupNum=%d, dmaQueueGroupNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosCpuScCpuQGrpToDmaQGrpMapAdd(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScCpuQGrpToDmaQGrpMapGet
    #/********************************************************************************/
    def do_qos_cpu_sc_cpu_q_grp_to_dma_q_grp_map_get(self, arg):
        '''
         xpsQosCpuScCpuQGrpToDmaQGrpMapGet: Enter [ devId,cpuQueueGroupNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,cpuQueueGroupNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dmaQueueGroupNum_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, cpuQueueGroupNum=%d' % (args[0],args[1]))
            ret = xpsQosCpuScCpuQGrpToDmaQGrpMapGet(args[0],args[1],dmaQueueGroupNum_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dmaQueueGroupNum = %d' % (uint32_tp_value(dmaQueueGroupNum_Ptr_2)))
                pass
            delete_uint32_tp(dmaQueueGroupNum_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosCpuScCpuQGrpToDmaQGrpMapRemove
    #/********************************************************************************/
    def do_qos_cpu_sc_cpu_q_grp_to_dma_q_grp_map_remove(self, arg):
        '''
         xpsQosCpuScCpuQGrpToDmaQGrpMapRemove: Enter [ devId,cpuQueueGroupNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,cpuQueueGroupNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, cpuQueueGroupNum=%d' % (args[0],args[1]))
            ret = xpsQosCpuScCpuQGrpToDmaQGrpMapRemove(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScAddReasonCodeToCpuQueueMap
    #/********************************************************************************/
    def do_qos_cpu_sc_add_reason_code_to_cpu_queue_map(self, arg):
        '''
         xpsQosCpuScAddReasonCodeToCpuQueueMap: Enter [ devId,cpuType,reasonCode,cpuQueueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,cpuType,reasonCode,cpuQueueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, cpuType=%d, reasonCode=%d, cpuQueueNum=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosCpuScAddReasonCodeToCpuQueueMap(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosCpuScGetReasonCodeToCpuQueueMap
    #/********************************************************************************/
    def do_qos_cpu_sc_get_reason_code_to_cpu_queue_map(self, arg):
        '''
         xpsQosCpuScGetReasonCodeToCpuQueueMap: Enter [ devId,cpuType,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,cpuType,reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            queueNum_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, cpuType=%d, reasonCode=%d' % (args[0],args[1],args[2]))
            ret = xpsQosCpuScGetReasonCodeToCpuQueueMap(args[0],args[1],args[2],queueNum_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('queueNum = %d' % (uint32_tp_value(queueNum_Ptr_3)))
                pass
            delete_uint32_tp(queueNum_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPfcRcvEn
    #/********************************************************************************/
    def do_qos_fc_set_pfc_rcv_en(self, arg):
        '''
         xpsQosFcSetPfcRcvEn: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosFcSetPfcRcvEn(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcRcvEn
    #/********************************************************************************/
    def do_qos_fc_get_pfc_rcv_en(self, arg):
        '''
         xpsQosFcGetPfcRcvEn: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetPfcRcvEn(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcSetPfcPriority
    #/********************************************************************************/
    def do_qos_fc_set_pfc_priority(self, arg):
        '''
         xpsQosFcSetPfcPriority: Enter [ devId,devPort,queueNum,prio ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,prio ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, prio=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPfcPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcPriority
    #/********************************************************************************/
    def do_qos_fc_get_pfc_priority(self, arg):
        '''
         xpsQosFcGetPfcPriority: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            priority_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPfcPriority(args[0],args[1],args[2],priority_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_3)))
                pass
            delete_uint32_tp(priority_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcGetNumPfcPrioritiesSupported
    #/********************************************************************************/
    def do_qos_fc_get_num_pfc_priorities_supported(self, arg):
        '''
         xpsQosFcGetNumPfcPrioritiesSupported: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            totalSupportedPrio_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetNumPfcPrioritiesSupported(args[0],totalSupportedPrio_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('totalSupportedPrio = %d' % (uint32_tp_value(totalSupportedPrio_Ptr_1)))
                pass
            delete_uint32_tp(totalSupportedPrio_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcBindPfcPriorityToTc
    #/********************************************************************************/
    def do_qos_fc_bind_pfc_priority_to_tc(self, arg):
        '''
         xpsQosFcBindPfcPriorityToTc: Enter [ devId,priority,pfcTrafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,priority,pfcTrafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tc=%d, prio=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcBindPfcPriorityToTc(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcPriorityToTcBinding
    #/********************************************************************************/
    def do_qos_fc_get_pfc_priority_to_tc_binding(self, arg):
        '''
         xpsQosFcGetPfcPriorityToTcBinding: Enter [ devId,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pfcTrafficClass_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, tc=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcPriorityToTcBinding(args[0],args[1],pfcTrafficClass_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pfcTrafficClass = %d' % (uint32_tp_value(pfcTrafficClass_Ptr_2)))
                pass
            delete_uint32_tp(pfcTrafficClass_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcBindPortToPfcGroupStaticProfile
    #/********************************************************************************/
    def do_qos_fc_bind_port_to_pfc_group_static_profile(self, arg):
        '''
         xpsQosFcBindPortToPfcGroupStaticProfile: Enter [ devId,portId,portPfcGroupPflId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId,portPfcGroupPflId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portId=%d, portPfcGroupPflId=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcBindPortToPfcGroupStaticProfile(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortToPfcGroupStaticProfileBinding
    #/********************************************************************************/
    def do_qos_fc_get_port_to_pfc_group_static_profile_binding(self, arg):
        '''
         xpsQosFcGetPortToPfcGroupStaticProfileBinding: Enter [ devId,portId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            portPfcGroupPflId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortToPfcGroupStaticProfileBinding(args[0],args[1],portPfcGroupPflId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('portPfcGroupPflId = %d' % (uint32_tp_value(portPfcGroupPflId_Ptr_2)))
                pass
            delete_uint32_tp(portPfcGroupPflId_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortPfcGroupStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_pfc_group_static_xon_threshold(self, arg):
        '''
         xpsQosFcSetPortPfcGroupStaticXonThreshold: Enter [ devId,portPfcGroupPflId,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d, xonThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortPfcGroupStaticXonThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortPfcGroupStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_pfc_group_static_xon_threshold(self, arg):
        '''
         xpsQosFcGetPortPfcGroupStaticXonThreshold: Enter [ devId,portPfcGroupPflId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xonThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortPfcGroupStaticXonThreshold(args[0],args[1],xonThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortPfcGroupStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_pfc_group_static_xoff_threshold(self, arg):
        '''
         xpsQosFcSetPortPfcGroupStaticXoffThreshold: Enter [ devId,portPfcGroupPflId,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d, xoffThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortPfcGroupStaticXoffThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortPfcGroupStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_pfc_group_static_xoff_threshold(self, arg):
        '''
         xpsQosFcGetPortPfcGroupStaticXoffThreshold: Enter [ devId,portPfcGroupPflId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xoffThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortPfcGroupStaticXoffThreshold(args[0],args[1],xoffThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalTcPfcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_tc_pfc_static_xon_threshold(self, arg):
        '''
         xpsQosFcSetGlobalTcPfcStaticXonThreshold: Enter [ devId,trafficClass,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,trafficClass,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, trafficClass=%d, xonThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetGlobalTcPfcStaticXonThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalTcPfcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_tc_pfc_static_xon_threshold(self, arg):
        '''
         xpsQosFcGetGlobalTcPfcStaticXonThreshold: Enter [ devId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xonThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, trafficClass=%d' % (args[0],args[1]))
            ret = xpsQosFcGetGlobalTcPfcStaticXonThreshold(args[0],args[1],xonThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalTcPfcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_tc_pfc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcSetGlobalTcPfcStaticXoffThreshold: Enter [ devId,trafficClass,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,trafficClass,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, trafficClass=%d, xoffThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetGlobalTcPfcStaticXoffThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalTcPfcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_tc_pfc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcGetGlobalTcPfcStaticXoffThreshold: Enter [ devId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xoffThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, trafficClass=%d' % (args[0],args[1]))
            ret = xpsQosFcGetGlobalTcPfcStaticXoffThreshold(args[0],args[1],xoffThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcBindPortToPortTcPfcStaticProfile
    #/********************************************************************************/
    def do_qos_fc_bind_port_to_port_tc_pfc_static_profile(self, arg):
        '''
         xpsQosFcBindPortToPortTcPfcStaticProfile: Enter [ devId,portId,pfcProfileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId,pfcProfileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portId=%d, pfcProfileId=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcBindPortToPortTcPfcStaticProfile(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortToPortTcPfcStaticProfile
    #/********************************************************************************/
    def do_qos_fc_get_port_to_port_tc_pfc_static_profile(self, arg):
        '''
         xpsQosFcGetPortToPortTcPfcStaticProfile: Enter [ devId,portId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pfcProfileId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortToPortTcPfcStaticProfile(args[0],args[1],pfcProfileId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pfcProfileId = %d' % (uint32_tp_value(pfcProfileId_Ptr_2)))
                pass
            delete_uint32_tp(pfcProfileId_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortTcPfcStaticXonThresholdForProfile
    #/********************************************************************************/
    def do_qos_fc_set_port_tc_pfc_static_xon_threshold_for_profile(self, arg):
        '''
         xpsQosFcSetPortTcPfcStaticXonThresholdForProfile: Enter [ devId,pfcProfileId,trafficClass,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcProfileId,trafficClass,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, pfcProfileId=%d, trafficClass=%d, xonThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPortTcPfcStaticXonThresholdForProfile(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortTcPfcStaticXonThresholdForProfile
    #/********************************************************************************/
    def do_qos_fc_get_port_tc_pfc_static_xon_threshold_for_profile(self, arg):
        '''
         xpsQosFcGetPortTcPfcStaticXonThresholdForProfile: Enter [ devId,pfcProfileId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcProfileId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xonThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, pfcProfileId=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPortTcPfcStaticXonThresholdForProfile(args[0],args[1],args[2],xonThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile
    #/********************************************************************************/
    def do_qos_fc_set_port_tc_pfc_static_xoff_threshold_for_profile(self, arg):
        '''
         xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile: Enter [ devId,pfcProfileId,trafficClass,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcProfileId,trafficClass,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, pfcProfileId=%d, trafficClass=%d, xoffThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortTcPfcStaticXoffThresholdForProfile
    #/********************************************************************************/
    def do_qos_fc_get_port_tc_pfc_static_xoff_threshold_for_profile(self, arg):
        '''
         xpsQosFcGetPortTcPfcStaticXoffThresholdForProfile: Enter [ devId,pfcProfileId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcProfileId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xoffThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, pfcProfileId=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPortTcPfcStaticXoffThresholdForProfile(args[0],args[1],args[2],xoffThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPortTcPfcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_tc_pfc_static_xon_threshold(self, arg):
        '''
         xpsQosFcSetPortTcPfcStaticXonThreshold: Enter [ devId,devPort,pfcTrafficClass,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pfcTrafficClass,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, pfcTrafficClass=%d, xonThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPortTcPfcStaticXonThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortTcPfcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_tc_pfc_static_xon_threshold(self, arg):
        '''
         xpsQosFcGetPortTcPfcStaticXonThreshold: Enter [ devId,devPort,pfcTrafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pfcTrafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xonThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, pfcTrafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPortTcPfcStaticXonThreshold(args[0],args[1],args[2],xonThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPortTcPfcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_tc_pfc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcSetPortTcPfcStaticXoffThreshold: Enter [ devId,devPort,pfcTrafficClass,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pfcTrafficClass,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, pfcTrafficClass=%d, xoffThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPortTcPfcStaticXoffThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortTcPfcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_tc_pfc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcGetPortTcPfcStaticXoffThreshold: Enter [ devId,devPort,pfcTrafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pfcTrafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xoffThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, pfcTrafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPortTcPfcStaticXoffThreshold(args[0],args[1],args[2],xoffThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalPfcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_pfc_static_xon_threshold(self, arg):
        '''
         xpsQosFcSetGlobalPfcStaticXonThreshold: Enter [ devId,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, xonThreshold=%d' % (args[0],args[1]))
            ret = xpsQosFcSetGlobalPfcStaticXonThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalPfcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_pfc_static_xon_threshold(self, arg):
        '''
         xpsQosFcGetGlobalPfcStaticXonThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            xonThreshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetGlobalPfcStaticXonThreshold(args[0],xonThreshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_1)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalPfcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_pfc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcSetGlobalPfcStaticXoffThreshold: Enter [ devId,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, xoffThreshold=%d' % (args[0],args[1]))
            ret = xpsQosFcSetGlobalPfcStaticXoffThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalPfcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_pfc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcGetGlobalPfcStaticXoffThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            xoffThreshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetGlobalPfcStaticXoffThreshold(args[0],xoffThreshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_1)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcSetPortPfcGroupTcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_pfc_group_tc_static_xon_threshold(self, arg):
        '''
         xpsQosFcSetPortPfcGroupTcStaticXonThreshold: Enter [ devId,portPfcGroupPflId,trafficClass,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId,trafficClass,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d, trafficClass=%d, xonThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPortPfcGroupTcStaticXonThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortPfcGroupTcStaticXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_pfc_group_tc_static_xon_threshold(self, arg):
        '''
         xpsQosFcGetPortPfcGroupTcStaticXonThreshold: Enter [ devId,portPfcGroupPflId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xonThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPortPfcGroupTcStaticXonThreshold(args[0],args[1],args[2],xonThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPortPfcGroupTcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_pfc_group_tc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcSetPortPfcGroupTcStaticXoffThreshold: Enter [ devId,portPfcGroupPflId,trafficClass,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId,trafficClass,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d, trafficClass=%d, xoffThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPortPfcGroupTcStaticXoffThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortPfcGroupTcStaticXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_pfc_group_tc_static_xoff_threshold(self, arg):
        '''
         xpsQosFcGetPortPfcGroupTcStaticXoffThreshold: Enter [ devId,portPfcGroupPflId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portPfcGroupPflId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xoffThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portPfcGroupPflId=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPortPfcGroupTcStaticXoffThreshold(args[0],args[1],args[2],xoffThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPortPfcXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_pfc_xon_threshold(self, arg):
        '''
         xpsQosFcSetPortPfcXonThreshold: Enter [ devId,port,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, xonThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortPfcXonThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortPfcXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_pfc_xon_threshold(self, arg):
        '''
         xpsQosFcGetPortPfcXonThreshold: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xonThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortPfcXonThreshold(args[0],args[1],xonThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortPfcXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_pfc_xoff_threshold(self, arg):
        '''
         xpsQosFcSetPortPfcXoffThreshold: Enter [ devId,port,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, xoffThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortPfcXoffThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortPfcXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_pfc_xoff_threshold(self, arg):
        '''
         xpsQosFcGetPortPfcXoffThreshold: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xoffThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortPfcXoffThreshold(args[0],args[1],xoffThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcEnablePfcDynamicThresholding
    #/********************************************************************************/
    def do_qos_fc_enable_pfc_dynamic_thresholding(self, arg):
        '''
         xpsQosFcEnablePfcDynamicThresholding: Enter [ devId,devPort,trafficClass,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,trafficClass,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, trafficClass=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcEnablePfcDynamicThresholding(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcDynamicThresholdingEnable
    #/********************************************************************************/
    def do_qos_fc_get_pfc_dynamic_thresholding_enable(self, arg):
        '''
         xpsQosFcGetPfcDynamicThresholdingEnable: Enter [ devId,devPort,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPfcDynamicThresholdingEnable(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcBindPfcTcToDynamicPool
    #/********************************************************************************/
    def do_qos_fc_bind_pfc_tc_to_dynamic_pool(self, arg):
        '''
         xpsQosFcBindPfcTcToDynamicPool: Enter [ devId,pfcTrafficClass,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcTrafficClass,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, pfcTrafficClass=%d, dynPoolId=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcBindPfcTcToDynamicPool(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcToDynamicPoolBinding
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_to_dynamic_pool_binding(self, arg):
        '''
         xpsQosFcGetPfcTcToDynamicPoolBinding: Enter [ devId,pfcTrafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcTrafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dynPoolId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, pfcTrafficClass=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcTcToDynamicPoolBinding(args[0],args[1],dynPoolId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dynPoolId = %d' % (uint32_tp_value(dynPoolId_Ptr_2)))
                pass
            delete_uint32_tp(dynPoolId_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcUnbindPfcTcFromDynamicPool
    #/********************************************************************************/
    def do_qos_fc_unbind_pfc_tc_from_dynamic_pool(self, arg):
        '''
         xpsQosFcUnbindPfcTcFromDynamicPool: Enter [ devId,pfcTrafficClass,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pfcTrafficClass,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, pfcTrafficClass=%d, dynPoolId=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcUnbindPfcTcFromDynamicPool(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcSetPfcTcDynamicPoolTotalSize
    #/********************************************************************************/
    def do_qos_fc_set_pfc_tc_dynamic_pool_total_size(self, arg):
        '''
         xpsQosFcSetPfcTcDynamicPoolTotalSize: Enter [ devId,dynPoolId,poolSizeInPages ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,poolSizeInPages ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, poolSizeInPages=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPfcTcDynamicPoolTotalSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcDynamicPoolTotalSize
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_dynamic_pool_total_size(self, arg):
        '''
         xpsQosFcGetPfcTcDynamicPoolTotalSize: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            poolSizeInPages_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcTcDynamicPoolTotalSize(args[0],args[1],poolSizeInPages_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('poolSizeInPages = %d' % (uint32_tp_value(poolSizeInPages_Ptr_2)))
                pass
            delete_uint32_tp(poolSizeInPages_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPfcTcDynamicPoolSharedSize
    #/********************************************************************************/
    def do_qos_fc_set_pfc_tc_dynamic_pool_shared_size(self, arg):
        '''
         xpsQosFcSetPfcTcDynamicPoolSharedSize: Enter [ devId,dynPoolId,poolSizeInPages ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,poolSizeInPages ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, poolSizeInPages=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPfcTcDynamicPoolSharedSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcDynamicPoolSharedSize
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_dynamic_pool_shared_size(self, arg):
        '''
         xpsQosFcGetPfcTcDynamicPoolSharedSize: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            poolSizeInPages_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcTcDynamicPoolSharedSize(args[0],args[1],poolSizeInPages_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('poolSizeInPages = %d' % (uint32_tp_value(poolSizeInPages_Ptr_2)))
                pass
            delete_uint32_tp(poolSizeInPages_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPfcTcDynamicPoolGuaranteedSize
    #/********************************************************************************/
    def do_qos_fc_set_pfc_tc_dynamic_pool_guaranteed_size(self, arg):
        '''
         xpsQosFcSetPfcTcDynamicPoolGuaranteedSize: Enter [ devId,dynPoolId,poolSizeInPages ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,poolSizeInPages ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, poolSizeInPages=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPfcTcDynamicPoolGuaranteedSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcDynamicPoolGuaranteedSize
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_dynamic_pool_guaranteed_size(self, arg):
        '''
         xpsQosFcGetPfcTcDynamicPoolGuaranteedSize: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            poolSizeInPages_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcTcDynamicPoolGuaranteedSize(args[0],args[1],poolSizeInPages_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('poolSizeInPages = %d' % (uint32_tp_value(poolSizeInPages_Ptr_2)))
                pass
            delete_uint32_tp(poolSizeInPages_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcConfigurePfcTcDynamicPoolAlpha
    #/********************************************************************************/
    def do_qos_fc_configure_pfc_tc_dynamic_pool_alpha(self, arg):
        '''
         xpsQosFcConfigurePfcTcDynamicPoolAlpha: Enter [ devId,dynPoolId,fraction,coefficient ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,fraction,coefficient ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, fraction=%d, coefficient=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcConfigurePfcTcDynamicPoolAlpha(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcDynamicPoolAlpha
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_dynamic_pool_alpha(self, arg):
        '''
         xpsQosFcGetPfcTcDynamicPoolAlpha: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fraction_Ptr_2 = new_xpDynThldFraction_ep()
            coefficient_Ptr_3 = new_xpDynThldCoeff_ep()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcTcDynamicPoolAlpha(args[0],args[1],fraction_Ptr_2,coefficient_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fraction = %d' % (xpDynThldFraction_ep_value(fraction_Ptr_2)))
                print('coefficient = %d' % (xpDynThldCoeff_ep_value(coefficient_Ptr_3)))
                pass
            delete_xpDynThldCoeff_ep(coefficient_Ptr_3)
            delete_xpDynThldFraction_ep(fraction_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPfcTcDynamicXonCalculationEnable
    #/********************************************************************************/
    def do_qos_fc_set_pfc_tc_dynamic_xon_calculation_enable(self, arg):
        '''
         xpsQosFcSetPfcTcDynamicXonCalculationEnable: Enter [ devId,devPort,pfcTrafficClass,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pfcTrafficClass,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, pfcTrafficClass=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcSetPfcTcDynamicXonCalculationEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcDynamicXonCalculationEnable
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_dynamic_xon_calculation_enable(self, arg):
        '''
         xpsQosFcGetPfcTcDynamicXonCalculationEnable: Enter [ devId,devPort,pfcTrafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pfcTrafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, pfcTrafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPfcTcDynamicXonCalculationEnable(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio
    #/********************************************************************************/
    def do_qos_fc_configure_pfc_tc_dynamic_pool_xoff_to_xon_ratio(self, arg):
        '''
         xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio: Enter [ devId,dynPoolId,operation,betaCoeff,gammaCoeff ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,operation,betaCoeff,gammaCoeff ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, devId=%d, dynPoolId=%d, operation=%d, betaCoeff=%d, gammaCoeff=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosFcConfigurePfcTcDynamicPoolXoffToXonRatio(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPfcTcDynamicPoolXoffToXonRatio
    #/********************************************************************************/
    def do_qos_fc_get_pfc_tc_dynamic_pool_xoff_to_xon_ratio(self, arg):
        '''
         xpsQosFcGetPfcTcDynamicPoolXoffToXonRatio: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            operation_Ptr_2 = new_xpDynThldOperator_ep()
            betaCoeff_Ptr_3 = new_xpDynThldCoeff_ep()
            gammaCoeff_Ptr_4 = new_xpDynThldCoeff_ep()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcTcDynamicPoolXoffToXonRatio(args[0],args[1],operation_Ptr_2,betaCoeff_Ptr_3,gammaCoeff_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('operation = %d' % (xpDynThldOperator_ep_value(operation_Ptr_2)))
                print('betaCoeff = %d' % (xpDynThldCoeff_ep_value(betaCoeff_Ptr_3)))
                print('gammaCoeff = %d' % (xpDynThldCoeff_ep_value(gammaCoeff_Ptr_4)))
                pass
            delete_xpDynThldCoeff_ep(gammaCoeff_Ptr_4)
            delete_xpDynThldCoeff_ep(betaCoeff_Ptr_3)
            delete_xpDynThldOperator_ep(operation_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcGetPfcDynamicPoolCurrentSharedUsed
    #/********************************************************************************/
    def do_qos_fc_get_pfc_dynamic_pool_current_shared_used(self, arg):
        '''
         xpsQosFcGetPfcDynamicPoolCurrentSharedUsed: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPfcDynamicPoolCurrentSharedUsed(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcGetPfcDynamicPoolMaxSharedUsed
    #/********************************************************************************/
    def do_qos_fc_get_pfc_dynamic_pool_max_shared_used(self, arg):
        '''
         xpsQosFcGetPfcDynamicPoolMaxSharedUsed: Enter [ devId,dynPoolId,clearOnRead ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId,clearOnRead ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            counter_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d, clearOnRead=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPfcDynamicPoolMaxSharedUsed(args[0],args[1],args[2],counter_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_3)))
                pass
            delete_uint32_tp(counter_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcSetPortFcXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_fc_xon_threshold(self, arg):
        '''
         xpsQosFcSetPortFcXonThreshold: Enter [ devId,port,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, xonThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortFcXonThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortFcXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_fc_xon_threshold(self, arg):
        '''
         xpsQosFcGetPortFcXonThreshold: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xonThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortFcXonThreshold(args[0],args[1],xonThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortFcXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_fc_xoff_threshold(self, arg):
        '''
         xpsQosFcSetPortFcXoffThreshold: Enter [ devId,port,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, xoffThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortFcXoffThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortFcXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_fc_xoff_threshold(self, arg):
        '''
         xpsQosFcGetPortFcXoffThreshold: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xoffThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortFcXoffThreshold(args[0],args[1],xoffThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortFcFullThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_fc_full_threshold(self, arg):
        '''
         xpsQosFcSetPortFcFullThreshold: Enter [ devId,port,fullThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,fullThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, fullThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortFcFullThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortFcFullThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_fc_full_threshold(self, arg):
        '''
         xpsQosFcGetPortFcFullThreshold: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fullThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortFcFullThreshold(args[0],args[1],fullThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fullThreshold = %d' % (uint32_tp_value(fullThreshold_Ptr_2)))
                pass
            delete_uint32_tp(fullThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcBindPortToPortFcGroup
    #/********************************************************************************/
    def do_qos_fc_bind_port_to_port_fc_group(self, arg):
        '''
         xpsQosFcBindPortToPortFcGroup: Enter [ devId,port,portFcGroup ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,portFcGroup ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, portFcGroup=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcBindPortToPortFcGroup(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortToPortFcGroupBinding
    #/********************************************************************************/
    def do_qos_fc_get_port_to_port_fc_group_binding(self, arg):
        '''
         xpsQosFcGetPortToPortFcGroupBinding: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            portFcGroup_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortToPortFcGroupBinding(args[0],args[1],portFcGroup_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('portFcGroup = %d' % (uint32_tp_value(portFcGroup_Ptr_2)))
                pass
            delete_uint32_tp(portFcGroup_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortFcGroupXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_fc_group_xon_threshold(self, arg):
        '''
         xpsQosFcSetPortFcGroupXonThreshold: Enter [ devId,portFcGroup,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portFcGroup,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portFcGroup=%d, xonThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortFcGroupXonThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortFcGroupXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_fc_group_xon_threshold(self, arg):
        '''
         xpsQosFcGetPortFcGroupXonThreshold: Enter [ devId,portFcGroup ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portFcGroup ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xonThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portFcGroup=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortFcGroupXonThreshold(args[0],args[1],xonThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortFcGroupXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_fc_group_xoff_threshold(self, arg):
        '''
         xpsQosFcSetPortFcGroupXoffThreshold: Enter [ devId,portFcGroup,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portFcGroup,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portFcGroup=%d, xoffThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortFcGroupXoffThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortFcGroupXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_fc_group_xoff_threshold(self, arg):
        '''
         xpsQosFcGetPortFcGroupXoffThreshold: Enter [ devId,portFcGroup ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portFcGroup ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xoffThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portFcGroup=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortFcGroupXoffThreshold(args[0],args[1],xoffThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetPortFcGroupFullThreshold
    #/********************************************************************************/
    def do_qos_fc_set_port_fc_group_full_threshold(self, arg):
        '''
         xpsQosFcSetPortFcGroupFullThreshold: Enter [ devId,portFcGroup,fullThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portFcGroup,fullThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portFcGroup=%d, fullThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcSetPortFcGroupFullThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetPortFcGroupFullThreshold
    #/********************************************************************************/
    def do_qos_fc_get_port_fc_group_full_threshold(self, arg):
        '''
         xpsQosFcGetPortFcGroupFullThreshold: Enter [ devId,portFcGroup ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portFcGroup ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fullThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portFcGroup=%d' % (args[0],args[1]))
            ret = xpsQosFcGetPortFcGroupFullThreshold(args[0],args[1],fullThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fullThreshold = %d' % (uint32_tp_value(fullThreshold_Ptr_2)))
                pass
            delete_uint32_tp(fullThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalFcXonThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_fc_xon_threshold(self, arg):
        '''
         xpsQosFcSetGlobalFcXonThreshold: Enter [ devId,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, xonThreshold=%d' % (args[0],args[1]))
            ret = xpsQosFcSetGlobalFcXonThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalFcXonThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_fc_xon_threshold(self, arg):
        '''
         xpsQosFcGetGlobalFcXonThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            xonThreshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetGlobalFcXonThreshold(args[0],xonThreshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_1)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalFcXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_fc_xoff_threshold(self, arg):
        '''
         xpsQosFcSetGlobalFcXoffThreshold: Enter [ devId,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, xoffThreshold=%d' % (args[0],args[1]))
            ret = xpsQosFcSetGlobalFcXoffThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalFcXoffThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_fc_xoff_threshold(self, arg):
        '''
         xpsQosFcGetGlobalFcXoffThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            xoffThreshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetGlobalFcXoffThreshold(args[0],xoffThreshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_1)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcSetGlobalFcFullThreshold
    #/********************************************************************************/
    def do_qos_fc_set_global_fc_full_threshold(self, arg):
        '''
         xpsQosFcSetGlobalFcFullThreshold: Enter [ devId,fullThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fullThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, fullThreshold=%d' % (args[0],args[1]))
            ret = xpsQosFcSetGlobalFcFullThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetGlobalFcFullThreshold
    #/********************************************************************************/
    def do_qos_fc_get_global_fc_full_threshold(self, arg):
        '''
         xpsQosFcGetGlobalFcFullThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            fullThreshold_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosFcGetGlobalFcFullThreshold(args[0],fullThreshold_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fullThreshold = %d' % (uint32_tp_value(fullThreshold_Ptr_1)))
                pass
            delete_uint32_tp(fullThreshold_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosFcGetPfcPortTcCounter
    #/********************************************************************************/
    def do_qos_fc_get_pfc_port_tc_counter(self, arg):
        '''
         xpsQosFcGetPfcPortTcCounter: Enter [ devId,portNum,pfcTrafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,pfcTrafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            counter_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, pfcTrafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetPfcPortTcCounter(args[0],args[1],args[2],counter_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_3)))
                pass
            delete_uint32_tp(counter_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosFcGetHeadroomCount
    #/********************************************************************************/
    def do_qos_fc_get_headroom_count(self, arg):
        '''
         xpsQosFcGetHeadroomCount: Enter [ devId,portNum,priority,clearOnRead ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority,clearOnRead ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            count_Ptr_4 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d, clearOnRead=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcGetHeadroomCount(args[0],args[1],args[2],args[3],count_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_4)))
                pass
            delete_uint64_tp(count_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosFcGetMaxHeadroomCount
    #/********************************************************************************/
    def do_qos_fc_get_max_headroom_count(self, arg):
        '''
         xpsQosFcGetMaxHeadroomCount: Enter [ devId,portNum,priority,clearOnRead ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority,clearOnRead ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            count_Ptr_4 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d, clearOnRead=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcGetMaxHeadroomCount(args[0],args[1],args[2],args[3],count_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_4)))
                pass
            delete_uint64_tp(count_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosFcConfigureMaxHeadroomThreshold
    #/********************************************************************************/
    def do_qos_fc_configure_max_headroom_threshold(self, arg):
        '''
         xpsQosFcConfigureMaxHeadroomThreshold: Enter [ devId,portNum,priority,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d, threshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosFcConfigureMaxHeadroomThreshold(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosFcGetMaxHeadroomThreshold
    #/********************************************************************************/
    def do_qos_fc_get_max_headroom_threshold(self, arg):
        '''
         xpsQosFcGetMaxHeadroomThreshold: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            threshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosFcGetMaxHeadroomThreshold(args[0],args[1],args[2],threshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('threshold = %d' % (uint32_tp_value(threshold_Ptr_3)))
                pass
            delete_uint32_tp(threshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosIbConfigureGlobalMacDaForPriorityAssignment
    #/********************************************************************************/
    def do_qos_ib_configure_global_mac_da_for_priority_assignment(self, arg):
        '''
         xpsQosIbConfigureGlobalMacDaForPriorityAssignment: Enter [ devId,profileId,macDa,macDaMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,macDa,macDaMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macDaList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macDaList)
            macDa = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                macDa[listLen - ix - 1] = int(macDaList[ix], 16)
            args[3] = args[3].replace(".", ":").replace(",", ":")
            macDaMaskList = args[3].strip("'").strip("]").strip("[").split(":")
            listLen = len(macDaMaskList)
            macDaMask = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                macDaMask[listLen - ix - 1] = int(macDaMaskList[ix], 16)
            #print('Input Arguments are, devId=%d, profileId=%d, macDa=%s, macDaMask=%s' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosIbConfigureGlobalMacDaForPriorityAssignment(args[0],args[1],macDa,macDaMask)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetGlobalMacDaForPriorityAssignment
    #/********************************************************************************/
    def do_qos_ib_get_global_mac_da_for_priority_assignment(self, arg):
        '''
         xpsQosIbGetGlobalMacDaForPriorityAssignment: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macDa_Ptr_2 = macAddr_t()
            macDaMask_Ptr_3 = macAddr_t()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosIbGetGlobalMacDaForPriorityAssignment(args[0],args[1],macDa_Ptr_2,macDaMask_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                listLen = 6
                print('macDa = '),
                for ix in range(listLen-1, -1, -1):
                    print('%02x' % (macDa_Ptr_2[ix])),
                    if ix > 0:
                        sys.stdout.write(':'),
                print('')
                print('macDaMask = '),
                for ix in range(listLen-1, -1, -1):
                    print('%02x' % (macDaMask_Ptr_3[ix])),
                    if ix > 0:
                        sys.stdout.write(':'),
                print('')
                pass
    #/********************************************************************************/
    # command for xpsQosIbConfigureGlobalMacDaPriorityEnable
    #/********************************************************************************/
    def do_qos_ib_configure_global_mac_da_priority_enable(self, arg):
        '''
         xpsQosIbConfigureGlobalMacDaPriorityEnable: Enter [ devId,profileId,priorityEn ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,priorityEn ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, priorityEn=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigureGlobalMacDaPriorityEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetGlobalMacDaPriorityEnable
    #/********************************************************************************/
    def do_qos_ib_get_global_mac_da_priority_enable(self, arg):
        '''
         xpsQosIbGetGlobalMacDaPriorityEnable: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            priorityEn_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosIbGetGlobalMacDaPriorityEnable(args[0],args[1],priorityEn_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priorityEn = %d' % (uint32_tp_value(priorityEn_Ptr_2)))
                pass
            delete_uint32_tp(priorityEn_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigureGlobalMacDaPriority
    #/********************************************************************************/
    def do_qos_ib_configure_global_mac_da_priority(self, arg):
        '''
         xpsQosIbConfigureGlobalMacDaPriority: Enter [ devId,profileId,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigureGlobalMacDaPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetGlobalMacDaPriority
    #/********************************************************************************/
    def do_qos_ib_get_global_mac_da_priority(self, arg):
        '''
         xpsQosIbGetGlobalMacDaPriority: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            priority_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosIbGetGlobalMacDaPriority(args[0],args[1],priority_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_2)))
                pass
            delete_uint32_tp(priority_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigureGlobalEthertypeForPriorityAssignment
    #/********************************************************************************/
    def do_qos_ib_configure_global_ethertype_for_priority_assignment(self, arg):
        '''
         xpsQosIbConfigureGlobalEthertypeForPriorityAssignment: Enter [ devId,profileId,etherType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,etherType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, etherType=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigureGlobalEthertypeForPriorityAssignment(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetGlobalEthertypeForPriorityAssignment
    #/********************************************************************************/
    def do_qos_ib_get_global_ethertype_for_priority_assignment(self, arg):
        '''
         xpsQosIbGetGlobalEthertypeForPriorityAssignment: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            etherType_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosIbGetGlobalEthertypeForPriorityAssignment(args[0],args[1],etherType_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('etherType = %d' % (uint32_tp_value(etherType_Ptr_2)))
                pass
            delete_uint32_tp(etherType_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigureGlobalEthertypePriorityEnable
    #/********************************************************************************/
    def do_qos_ib_configure_global_ethertype_priority_enable(self, arg):
        '''
         xpsQosIbConfigureGlobalEthertypePriorityEnable: Enter [ devId,profileId,priorityEn ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,priorityEn ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, priorityEn=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigureGlobalEthertypePriorityEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetGlobalEthertypePriorityEnable
    #/********************************************************************************/
    def do_qos_ib_get_global_ethertype_priority_enable(self, arg):
        '''
         xpsQosIbGetGlobalEthertypePriorityEnable: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            priorityEn_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosIbGetGlobalEthertypePriorityEnable(args[0],args[1],priorityEn_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priorityEn = %d' % (uint32_tp_value(priorityEn_Ptr_2)))
                pass
            delete_uint32_tp(priorityEn_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigureGlobalEthertypePriority
    #/********************************************************************************/
    def do_qos_ib_configure_global_ethertype_priority(self, arg):
        '''
         xpsQosIbConfigureGlobalEthertypePriority: Enter [ devId,profileId,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, profileId=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigureGlobalEthertypePriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetGlobalEthertypePriority
    #/********************************************************************************/
    def do_qos_ib_get_global_ethertype_priority(self, arg):
        '''
         xpsQosIbGetGlobalEthertypePriority: Enter [ devId,profileId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            priority_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileId=%d' % (args[0],args[1]))
            ret = xpsQosIbGetGlobalEthertypePriority(args[0],args[1],priority_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_2)))
                pass
            delete_uint32_tp(priority_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortMplsPriorityEnable
    #/********************************************************************************/
    def do_qos_ib_configure_port_mpls_priority_enable(self, arg):
        '''
         xpsQosIbConfigurePortMplsPriorityEnable: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortMplsPriorityEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortMplsPriorityEnable
    #/********************************************************************************/
    def do_qos_ib_get_port_mpls_priority_enable(self, arg):
        '''
         xpsQosIbGetPortMplsPriorityEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortMplsPriorityEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortMplsPriority
    #/********************************************************************************/
    def do_qos_ib_configure_port_mpls_priority(self, arg):
        '''
         xpsQosIbConfigurePortMplsPriority: Enter [ devId,portNum,exp,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,exp,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, exp=%d, priority=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosIbConfigurePortMplsPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortMplsPriority
    #/********************************************************************************/
    def do_qos_ib_get_port_mpls_priority(self, arg):
        '''
         xpsQosIbGetPortMplsPriority: Enter [ devId,portNum,exp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,exp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            priority_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, exp=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbGetPortMplsPriority(args[0],args[1],args[2],priority_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_3)))
                pass
            delete_uint32_tp(priority_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortL3PriorityEnable
    #/********************************************************************************/
    def do_qos_ib_configure_port_l3_priority_enable(self, arg):
        '''
         xpsQosIbConfigurePortL3PriorityEnable: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortL3PriorityEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortL3PriorityEnable
    #/********************************************************************************/
    def do_qos_ib_get_port_l3_priority_enable(self, arg):
        '''
         xpsQosIbGetPortL3PriorityEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortL3PriorityEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortL3Priority
    #/********************************************************************************/
    def do_qos_ib_configure_port_l3_priority(self, arg):
        '''
         xpsQosIbConfigurePortL3Priority: Enter [ devId,portNum,dscp,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,dscp,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, dscp=%d, priority=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosIbConfigurePortL3Priority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortL3Priority
    #/********************************************************************************/
    def do_qos_ib_get_port_l3_priority(self, arg):
        '''
         xpsQosIbGetPortL3Priority: Enter [ devId,portNum,dscp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,dscp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            priority_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, dscp=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbGetPortL3Priority(args[0],args[1],args[2],priority_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_3)))
                pass
            delete_uint32_tp(priority_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortL2PriorityEnable
    #/********************************************************************************/
    def do_qos_ib_configure_port_l2_priority_enable(self, arg):
        '''
         xpsQosIbConfigurePortL2PriorityEnable: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortL2PriorityEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortL2PriorityEnable
    #/********************************************************************************/
    def do_qos_ib_get_port_l2_priority_enable(self, arg):
        '''
         xpsQosIbGetPortL2PriorityEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortL2PriorityEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortL2Priority
    #/********************************************************************************/
    def do_qos_ib_configure_port_l2_priority(self, arg):
        '''
         xpsQosIbConfigurePortL2Priority: Enter [ devId,portNum,pcp,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,pcp,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, pcp=%d, priority=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosIbConfigurePortL2Priority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortL2Priority
    #/********************************************************************************/
    def do_qos_ib_get_port_l2_priority(self, arg):
        '''
         xpsQosIbGetPortL2Priority: Enter [ devId,portNum,pcp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,pcp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            priority_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, pcp=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbGetPortL2Priority(args[0],args[1],args[2],priority_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_3)))
                pass
            delete_uint32_tp(priority_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortDefaultPriority
    #/********************************************************************************/
    def do_qos_ib_configure_port_default_priority(self, arg):
        '''
         xpsQosIbConfigurePortDefaultPriority: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortDefaultPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortDefaultPriority
    #/********************************************************************************/
    def do_qos_ib_get_port_default_priority(self, arg):
        '''
         xpsQosIbGetPortDefaultPriority: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            priority_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortDefaultPriority(args[0],args[1],priority_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_2)))
                pass
            delete_uint32_tp(priority_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortLosslessXonThreshold
    #/********************************************************************************/
    def do_qos_ib_configure_port_lossless_xon_threshold(self, arg):
        '''
         xpsQosIbConfigurePortLosslessXonThreshold: Enter [ devId,portNum,xonThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,xonThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, xonThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortLosslessXonThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortLosslessXonThreshold
    #/********************************************************************************/
    def do_qos_ib_get_port_lossless_xon_threshold(self, arg):
        '''
         xpsQosIbGetPortLosslessXonThreshold: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xonThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortLosslessXonThreshold(args[0],args[1],xonThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortLosslessXoffThreshold
    #/********************************************************************************/
    def do_qos_ib_configure_port_lossless_xoff_threshold(self, arg):
        '''
         xpsQosIbConfigurePortLosslessXoffThreshold: Enter [ devId,portNum,xoffThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,xoffThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, xoffThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortLosslessXoffThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortLosslessXoffThreshold
    #/********************************************************************************/
    def do_qos_ib_get_port_lossless_xoff_threshold(self, arg):
        '''
         xpsQosIbGetPortLosslessXoffThreshold: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xoffThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortLosslessXoffThreshold(args[0],args[1],xoffThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_2)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortLosslessDropThreshold
    #/********************************************************************************/
    def do_qos_ib_configure_port_lossless_drop_threshold(self, arg):
        '''
         xpsQosIbConfigurePortLosslessDropThreshold: Enter [ devId,portNum,dropThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,dropThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, dropThreshold=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbConfigurePortLosslessDropThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortLosslessDropThreshold
    #/********************************************************************************/
    def do_qos_ib_get_port_lossless_drop_threshold(self, arg):
        '''
         xpsQosIbGetPortLosslessDropThreshold: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dropThreshold_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosIbGetPortLosslessDropThreshold(args[0],args[1],dropThreshold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dropThreshold = %d' % (uint32_tp_value(dropThreshold_Ptr_2)))
                pass
            delete_uint32_tp(dropThreshold_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosIbConfigurePortLossyDropThresholdPerPriority
    #/********************************************************************************/
    def do_qos_ib_configure_port_lossy_drop_threshold_per_priority(self, arg):
        '''
         xpsQosIbConfigurePortLossyDropThresholdPerPriority: Enter [ devId,portNum,priority,dropThreshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority,dropThreshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d, dropThreshold=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosIbConfigurePortLossyDropThresholdPerPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetPortLossyDropThresholdPerPriority
    #/********************************************************************************/
    def do_qos_ib_get_port_lossy_drop_threshold_per_priority(self, arg):
        '''
         xpsQosIbGetPortLossyDropThresholdPerPriority: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            dropThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbGetPortLossyDropThresholdPerPriority(args[0],args[1],args[2],dropThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dropThreshold = %d' % (uint32_tp_value(dropThreshold_Ptr_3)))
                pass
            delete_uint32_tp(dropThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosIbEnableLosslessPerPortPriority
    #/********************************************************************************/
    def do_qos_ib_enable_lossless_per_port_priority(self, arg):
        '''
         xpsQosIbEnableLosslessPerPortPriority: Enter [ devId,portNum,priority,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosIbEnableLosslessPerPortPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosIbGetLosslessConfigPerPortPriority
    #/********************************************************************************/
    def do_qos_ib_get_lossless_config_per_port_priority(self, arg):
        '''
         xpsQosIbGetLosslessConfigPerPortPriority: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosIbGetLosslessConfigPerPortPriority(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetTrafficClassForL2QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_set_traffic_class_for_l2_qos_profile(self, arg):
        '''
         xpsQosPortIngressSetTrafficClassForL2QosProfile: Enter [ devId,profile,pcpVal,deiVal,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,pcpVal,deiVal,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, profile=%d, pcpVal=%d, deiVal=%d, tc=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosPortIngressSetTrafficClassForL2QosProfile(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetTrafficClassForL2QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_traffic_class_for_l2_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetTrafficClassForL2QosProfile: Enter [ devId,profile,pcpVal,deiVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,pcpVal,deiVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            tc_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, pcpVal=%d, deiVal=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressGetTrafficClassForL2QosProfile(args[0],args[1],args[2],args[3],tc_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tc = %d' % (uint32_tp_value(tc_Ptr_4)))
                pass
            delete_uint32_tp(tc_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetTrafficClassForL3QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_set_traffic_class_for_l3_qos_profile(self, arg):
        '''
         xpsQosPortIngressSetTrafficClassForL3QosProfile: Enter [ devId,profile,dscpVal,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,dscpVal,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, profile=%d, dscpVal=%d, tc=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetTrafficClassForL3QosProfile(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetTrafficClassForL3QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_traffic_class_for_l3_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetTrafficClassForL3QosProfile: Enter [ devId,profile,dscpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,dscpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            tc_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, dscpVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressGetTrafficClassForL3QosProfile(args[0],args[1],args[2],tc_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tc = %d' % (uint32_tp_value(tc_Ptr_3)))
                pass
            delete_uint32_tp(tc_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetTrafficClassForMplsQosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_set_traffic_class_for_mpls_qos_profile(self, arg):
        '''
         xpsQosPortIngressSetTrafficClassForMplsQosProfile: Enter [ devId,profile,expVal,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,expVal,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, profile=%d, expVal=%d, tc=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetTrafficClassForMplsQosProfile(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetTrafficClassForMplsQosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_traffic_class_for_mpls_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetTrafficClassForMplsQosProfile: Enter [ devId,profile,expVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,expVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            tc_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, expVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressGetTrafficClassForMplsQosProfile(args[0],args[1],args[2],tc_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tc = %d' % (uint32_tp_value(tc_Ptr_3)))
                pass
            delete_uint32_tp(tc_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetDropPrecedenceForL2QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_set_drop_precedence_for_l2_qos_profile(self, arg):
        '''
         xpsQosPortIngressSetDropPrecedenceForL2QosProfile: Enter [ devId,profile,pcpVal,deiVal,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,pcpVal,deiVal,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, profile=%d, pcpVal=%d, deiVal=%d, dp=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosPortIngressSetDropPrecedenceForL2QosProfile(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetDropPrecedenceForL2QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_drop_precedence_for_l2_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetDropPrecedenceForL2QosProfile: Enter [ devId,profile,pcpVal,deiVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,pcpVal,deiVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            dp_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, pcpVal=%d, deiVal=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressGetDropPrecedenceForL2QosProfile(args[0],args[1],args[2],args[3],dp_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (uint32_tp_value(dp_Ptr_4)))
                pass
            delete_uint32_tp(dp_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetDropPrecedenceForL3QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_set_drop_precedence_for_l3_qos_profile(self, arg):
        '''
         xpsQosPortIngressSetDropPrecedenceForL3QosProfile: Enter [ devId,profile,dscpVal,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,dscpVal,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, profile=%d, dscpVal=%d, dp=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetDropPrecedenceForL3QosProfile(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetDropPrecedenceForL3QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_drop_precedence_for_l3_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetDropPrecedenceForL3QosProfile: Enter [ devId,profile,dscpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,dscpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            dp_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, dscpVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressGetDropPrecedenceForL3QosProfile(args[0],args[1],args[2],dp_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (uint32_tp_value(dp_Ptr_3)))
                pass
            delete_uint32_tp(dp_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetDropPrecedenceForMplsQosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_set_drop_precedence_for_mpls_qos_profile(self, arg):
        '''
         xpsQosPortIngressSetDropPrecedenceForMplsQosProfile: Enter [ devId,profile,expVal,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,expVal,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, profile=%d, expVal=%d, dp=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetDropPrecedenceForMplsQosProfile(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetDropPrecedenceForMplsQosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_drop_precedence_for_mpls_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetDropPrecedenceForMplsQosProfile: Enter [ devId,profile,expVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,expVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            dp_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, expVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressGetDropPrecedenceForMplsQosProfile(args[0],args[1],args[2],dp_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (uint32_tp_value(dp_Ptr_3)))
                pass
            delete_uint32_tp(dp_Ptr_3)

    #/********************************************************************************/
    # command for xpsQosPortIngressRemapPriorityForL2QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_remap_priority_for_l2_qos_profile(self, arg):
        '''
         xpsQosPortIngressRemapPriorityForL2QosProfile: Enter [ devId,profile,pcpVal,deiVal,remapPcpVal,remapDeiVal,remapDscpVal,remapExpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,pcpVal,deiVal,remapPcpVal,remapDeiVal,remapDscpVal,remapExpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            args[7] = int(args[7])
            #print('Input Arguments are, devId=%d, profile=%d, pcpVal=%d, deiVal=%d, remapPcpVal=%d, remapDeiVal=%d, remapDscpVal=%d, remapExpVal=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpsQosPortIngressRemapPriorityForL2QosProfile(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsQosPortIngressGetRemappedPriorityForL2QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_remapped_priority_for_l2_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetRemappedPriorityForL2QosProfile: Enter [ devId,profile,pcpVal,deiVal]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,pcpVal,deiVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            remapPcpVal_Ptr_4 = new_uint32_tp()
            remapDeiVal_Ptr_5 = new_uint32_tp()
            remapDscpVal_Ptr_6 = new_uint32_tp()
            remapExpVal_Ptr_7 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, pcpVal=%d, deiVal=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressGetRemappedPriorityForL2QosProfile(args[0],args[1],args[2],args[3],remapPcpVal_Ptr_4,remapDeiVal_Ptr_5,remapDscpVal_Ptr_6,remapExpVal_Ptr_7)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('remapPcpVal = %d' % (uint32_tp_value(remapPcpVal_Ptr_4)))
                print('remapDeiVal = %d' % (uint32_tp_value(remapDeiVal_Ptr_5)))
                print('remapDscpVal = %d' % (uint32_tp_value(remapDscpVal_Ptr_6)))
                print('remapExpVal = %d' % (uint32_tp_value(remapExpVal_Ptr_7)))
                pass
            delete_uint32_tp(remapExpVal_Ptr_7)
            delete_uint32_tp(remapDscpVal_Ptr_6)
            delete_uint32_tp(remapDeiVal_Ptr_5)
            delete_uint32_tp(remapPcpVal_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosPortIngressRemapPriorityForL3QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_remap_priority_for_l3_qos_profile(self, arg):
        '''
         xpsQosPortIngressRemapPriorityForL3QosProfile: Enter [ devId,profile,dscpVal,remapPcpVal,remapDeiVal,remapDscpVal,remapExpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,dscpVal,remapPcpVal,remapDeiVal,remapDscpVal,remapExpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            #print('Input Arguments are, devId=%d, profile=%d, dscpVal=%d, remapPcpVal=%d, remapDeiVal=%d, remapDscpVal=%d, remapExpVal=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpsQosPortIngressRemapPriorityForL3QosProfile(args[0],args[1],args[2],args[3],args[4],args[5],args[6])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetRemappedPriorityForL3QosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_remapped_priority_for_l3_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetRemappedPriorityForL3QosProfile: Enter [ devId,profile,dscpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,dscpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            remapPcpVal_Ptr_3 = new_uint32_tp()
            remapDeiVal_Ptr_4 = new_uint32_tp()
            remapDscpVal_Ptr_5 = new_uint32_tp()
            remapExpVal_Ptr_6 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, dscpVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressGetRemappedPriorityForL3QosProfile(args[0],args[1],args[2],remapPcpVal_Ptr_3,remapDeiVal_Ptr_4,remapDscpVal_Ptr_5,remapExpVal_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('remapPcpVal = %d' % (uint32_tp_value(remapPcpVal_Ptr_3)))
                print('remapDeiVal = %d' % (uint32_tp_value(remapDeiVal_Ptr_4)))
                print('remapDscpVal = %d' % (uint32_tp_value(remapDscpVal_Ptr_5)))
                print('remapExpVal = %d' % (uint32_tp_value(remapExpVal_Ptr_6)))
                pass
            delete_uint32_tp(remapExpVal_Ptr_6)
            delete_uint32_tp(remapDscpVal_Ptr_5)
            delete_uint32_tp(remapDeiVal_Ptr_4)
            delete_uint32_tp(remapPcpVal_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosPortIngressRemapPriorityForMplsQosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_remap_priority_for_mpls_qos_profile(self, arg):
        '''
         xpsQosPortIngressRemapPriorityForMplsQosProfile: Enter [ devId,profile,expVal,remapPcpVal,remapDeiVal,remapDscpVal,remapExpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,expVal,remapPcpVal,remapDeiVal,remapDscpVal,remapExpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            #print('Input Arguments are, devId=%d, profile=%d, expVal=%d, remapPcpVal=%d, remapDeiVal=%d, remapDscpVal=%d, remapExpVal=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpsQosPortIngressRemapPriorityForMplsQosProfile(args[0],args[1],args[2],args[3],args[4],args[5],args[6])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetRemappedPriorityForMplsQosProfile
    #/********************************************************************************/
    def do_qos_port_ingress_get_remapped_priority_for_mpls_qos_profile(self, arg):
        '''
         xpsQosPortIngressGetRemappedPriorityForMplsQosProfile: Enter [ devId,profile,expVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,expVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            remapPcpVal_Ptr_3 = new_uint32_tp()
            remapDeiVal_Ptr_4 = new_uint32_tp()
            remapDscpVal_Ptr_5 = new_uint32_tp()
            remapExpVal_Ptr_6 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profile=%d, expVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressGetRemappedPriorityForMplsQosProfile(args[0],args[1],args[2],remapPcpVal_Ptr_3,remapDeiVal_Ptr_4,remapDscpVal_Ptr_5,remapExpVal_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('remapPcpVal = %d' % (uint32_tp_value(remapPcpVal_Ptr_3)))
                print('remapDeiVal = %d' % (uint32_tp_value(remapDeiVal_Ptr_4)))
                print('remapDscpVal = %d' % (uint32_tp_value(remapDscpVal_Ptr_5)))
                print('remapExpVal = %d' % (uint32_tp_value(remapExpVal_Ptr_6)))
                pass
            delete_uint32_tp(remapExpVal_Ptr_6)
            delete_uint32_tp(remapDscpVal_Ptr_5)
            delete_uint32_tp(remapDeiVal_Ptr_4)
            delete_uint32_tp(remapPcpVal_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetPortDefaultL2QosPriority
    #/********************************************************************************/
    def do_qos_port_ingress_set_port_default_l2_qos_priority(self, arg):
        '''
         xpsQosPortIngressSetPortDefaultL2QosPriority: Enter [ devId,devPort,pcpVal,deiVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,pcpVal,deiVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, pcpVal=%d, deiVal=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetPortDefaultL2QosPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetPortDefaultL2QosPriority
    #/********************************************************************************/
    def do_qos_port_ingress_get_port_default_l2_qos_priority(self, arg):
        '''
         xpsQosPortIngressGetPortDefaultL2QosPriority: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pcpVal_Ptr_2 = new_uint32_tp()
            deiVal_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetPortDefaultL2QosPriority(args[0],args[1],pcpVal_Ptr_2,deiVal_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pcpVal = %d' % (uint32_tp_value(pcpVal_Ptr_2)))
                print('deiVal = %d' % (uint32_tp_value(deiVal_Ptr_3)))
                pass
            delete_uint32_tp(deiVal_Ptr_3)
            delete_uint32_tp(pcpVal_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetPortDefaultL3QosPriority
    #/********************************************************************************/
    def do_qos_port_ingress_set_port_default_l3_qos_priority(self, arg):
        '''
         xpsQosPortIngressSetPortDefaultL3QosPriority: Enter [ devId,devPort,dscpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,dscpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, dscpVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressSetPortDefaultL3QosPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetPortDefaultL3QosPriority
    #/********************************************************************************/
    def do_qos_port_ingress_get_port_default_l3_qos_priority(self, arg):
        '''
         xpsQosPortIngressGetPortDefaultL3QosPriority: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dscpVal_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetPortDefaultL3QosPriority(args[0],args[1],dscpVal_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dscpVal = %d' % (uint32_tp_value(dscpVal_Ptr_2)))
                pass
            delete_uint32_tp(dscpVal_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetPortDefaultMplsQosPriority
    #/********************************************************************************/
    def do_qos_port_ingress_set_port_default_mpls_qos_priority(self, arg):
        '''
         xpsQosPortIngressSetPortDefaultMplsQosPriority: Enter [ devId,devPort,expVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,expVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, expVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressSetPortDefaultMplsQosPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetPortDefaultMplsQosPriority
    #/********************************************************************************/
    def do_qos_port_ingress_get_port_default_mpls_qos_priority(self, arg):
        '''
         xpsQosPortIngressGetPortDefaultMplsQosPriority: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            expVal_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetPortDefaultMplsQosPriority(args[0],args[1],expVal_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('expVal = %d' % (uint32_tp_value(expVal_Ptr_2)))
                pass
            delete_uint32_tp(expVal_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetPortDefaultTrafficClass
    #/********************************************************************************/
    def do_qos_port_ingress_set_port_default_traffic_class(self, arg):
        '''
         xpsQosPortIngressSetPortDefaultTrafficClass: Enter [ devId,devPort,tc ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,tc ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, tc=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressSetPortDefaultTrafficClass(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetPortDefaultTrafficClass
    #/********************************************************************************/
    def do_qos_port_ingress_get_port_default_traffic_class(self, arg):
        '''
         xpsQosPortIngressGetPortDefaultTrafficClass: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tc_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetPortDefaultTrafficClass(args[0],args[1],tc_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tc = %d' % (uint32_tp_value(tc_Ptr_2)))
                pass
            delete_uint32_tp(tc_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetPortDefaultDropPrecedence
    #/********************************************************************************/
    def do_qos_port_ingress_set_port_default_drop_precedence(self, arg):
        '''
         xpsQosPortIngressSetPortDefaultDropPrecedence: Enter [ devId,devPort,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, dp=%d' % (args[0],args[1],args[2]))
            ret = xpsQosPortIngressSetPortDefaultDropPrecedence(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetPortDefaultDropPrecedence
    #/********************************************************************************/
    def do_qos_port_ingress_get_port_default_drop_precedence(self, arg):
        '''
         xpsQosPortIngressGetPortDefaultDropPrecedence: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dp_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetPortDefaultDropPrecedence(args[0],args[1],dp_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (uint32_tp_value(dp_Ptr_2)))
                pass
            delete_uint32_tp(dp_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetL2QosProfileForPort
    #/********************************************************************************/
    def do_qos_port_ingress_set_l2_qos_profile_for_port(self, arg):
        '''
         xpsQosPortIngressSetL2QosProfileForPort: Enter [ devId,devPort,profile,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,profile,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, profile=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetL2QosProfileForPort(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetL2QosProfileForPort
    #/********************************************************************************/
    def do_qos_port_ingress_get_l2_qos_profile_for_port(self, arg):
        '''
         xpsQosPortIngressGetL2QosProfileForPort: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            profile_Ptr_2 = new_xpQosMapPfl_tp()
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetL2QosProfileForPort(args[0],args[1],profile_Ptr_2,enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profile = %d' % (xpQosMapPfl_tp_value(profile_Ptr_2)))
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
            delete_xpQosMapPfl_tp(profile_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetL3QosProfileForPort
    #/********************************************************************************/
    def do_qos_port_ingress_set_l3_qos_profile_for_port(self, arg):
        '''
         xpsQosPortIngressSetL3QosProfileForPort: Enter [ devId,devPort,profile,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,profile,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, profile=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetL3QosProfileForPort(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetL3QosProfileForPort
    #/********************************************************************************/
    def do_qos_port_ingress_get_l3_qos_profile_for_port(self, arg):
        '''
         xpsQosPortIngressGetL3QosProfileForPort: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            profile_Ptr_2 = new_xpQosMapPfl_tp()
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetL3QosProfileForPort(args[0],args[1],profile_Ptr_2,enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profile = %d' % (xpQosMapPfl_tp_value(profile_Ptr_2)))
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
            delete_xpQosMapPfl_tp(profile_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetMplsQosProfileForPort
    #/********************************************************************************/
    def do_qos_port_ingress_set_mpls_qos_profile_for_port(self, arg):
        '''
         xpsQosPortIngressSetMplsQosProfileForPort: Enter [ devId,devPort,profile,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,profile,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, profile=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosPortIngressSetMplsQosProfileForPort(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressGetMplsQosProfileForPort
    #/********************************************************************************/
    def do_qos_port_ingress_get_mpls_qos_profile_for_port(self, arg):
        '''
         xpsQosPortIngressGetMplsQosProfileForPort: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            profile_Ptr_2 = new_xpQosMapPfl_tp()
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressGetMplsQosProfileForPort(args[0],args[1],profile_Ptr_2,enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('profile = %d' % (xpQosMapPfl_tp_value(profile_Ptr_2)))
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
            delete_xpQosMapPfl_tp(profile_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosPortIngressSetTrustL2ForPort
    #/********************************************************************************/
    def do_qos_port_ingress_set_trust_l2_for_port(self, arg):
        '''
         xpsQosPortIngressSetTrustL2ForPort: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressSetTrustL2ForPort(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressSetTrustL3ForPort
    #/********************************************************************************/
    def do_qos_port_ingress_set_trust_l3_for_port(self, arg):
        '''
         xpsQosPortIngressSetTrustL3ForPort: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressSetTrustL3ForPort(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosPortIngressSetTrustMplsForPort
    #/********************************************************************************/
    def do_qos_port_ingress_set_trust_mpls_for_port(self, arg):
        '''
         xpsQosPortIngressSetTrustMplsForPort: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosPortIngressSetTrustMplsForPort(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosSetPcpDeiRemarkEn
    #/********************************************************************************/
    def do_qos_set_pcp_dei_remark_en(self, arg):
        '''
         xpsQosSetPcpDeiRemarkEn: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosSetPcpDeiRemarkEn(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetPcpDeiRemarkEnable
    #/********************************************************************************/
    def do_qos_get_pcp_dei_remark_enable(self, arg):
        '''
         xpsQosGetPcpDeiRemarkEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosGetPcpDeiRemarkEnable(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosSetDscpRemarkEn
    #/********************************************************************************/
    def do_qos_set_dscp_remark_en(self, arg):
        '''
         xpsQosSetDscpRemarkEn: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosSetDscpRemarkEn(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetDscpRemarkEnable
    #/********************************************************************************/
    def do_qos_get_dscp_remark_enable(self, arg):
        '''
         xpsQosGetDscpRemarkEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosGetDscpRemarkEnable(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosQcGetQueueFwdPacketCountForPort
    #/********************************************************************************/
    def do_qos_qc_get_queue_fwd_packet_count_for_port(self, arg):
        '''
         xpsQosQcGetQueueFwdPacketCountForPort: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            wrap_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetQueueFwdPacketCountForPort(args[0],args[1],args[2],count_Ptr_3,wrap_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                print('wrap = %d' % (uint32_tp_value(wrap_Ptr_4)))
                pass
            delete_uint32_tp(wrap_Ptr_4)
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetQueueDropPacketCountForPort
    #/********************************************************************************/
    def do_qos_qc_get_queue_drop_packet_count_for_port(self, arg):
        '''
         xpsQosQcGetQueueDropPacketCountForPort: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            wrap_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetQueueDropPacketCountForPort(args[0],args[1],args[2],count_Ptr_3,wrap_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                print('wrap = %d' % (uint32_tp_value(wrap_Ptr_4)))
                pass
            delete_uint32_tp(wrap_Ptr_4)
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetQueueFwdByteCountForPort
    #/********************************************************************************/
    def do_qos_qc_get_queue_fwd_byte_count_for_port(self, arg):
        '''
         xpsQosQcGetQueueFwdByteCountForPort: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            wrap_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetQueueFwdByteCountForPort(args[0],args[1],args[2],count_Ptr_3,wrap_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                print('wrap = %d' % (uint32_tp_value(wrap_Ptr_4)))
                pass
            delete_uint32_tp(wrap_Ptr_4)
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetQueueDropByteCountForPort
    #/********************************************************************************/
    def do_qos_qc_get_queue_drop_byte_count_for_port(self, arg):
        '''
         xpsQosQcGetQueueDropByteCountForPort: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            wrap_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetQueueDropByteCountForPort(args[0],args[1],args[2],count_Ptr_3,wrap_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                print('wrap = %d' % (uint32_tp_value(wrap_Ptr_4)))
                pass
            delete_uint32_tp(wrap_Ptr_4)
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetCurrentQueuePacketDepth
    #/********************************************************************************/
    def do_qos_qc_get_current_queue_packet_depth(self, arg):
        '''
         xpsQosQcGetCurrentQueuePacketDepth: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetCurrentQueuePacketDepth(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_3)))
                pass
            delete_uint32_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetCurrentQueuePageDepth
    #/********************************************************************************/
    def do_qos_qc_get_current_queue_page_depth(self, arg):
        '''
         xpsQosQcGetCurrentQueuePageDepth: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetCurrentQueuePageDepth(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_3)))
                pass
            delete_uint32_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetCurrentPortPageDepth
    #/********************************************************************************/
    def do_qos_qc_get_current_port_page_depth(self, arg):
        '''
         xpsQosQcGetCurrentPortPageDepth: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            count_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsQosQcGetCurrentPortPageDepth(args[0],args[1],count_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_2)))
                pass
            delete_uint32_tp(count_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosQcGetQueueAveragePageLength
    #/********************************************************************************/
    def do_qos_qc_get_queue_average_page_length(self, arg):
        '''
         xpsQosQcGetQueueAveragePageLength: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetQueueAveragePageLength(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_3)))
                pass
            delete_uint32_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetQueueOldPageLength
    #/********************************************************************************/
    def do_qos_qc_get_queue_old_page_length(self, arg):
        '''
         xpsQosQcGetQueueOldPageLength: Enter [ devId,port,queue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,queue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, queue=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetQueueOldPageLength(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_3)))
                pass
            delete_uint32_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcEnableFwdPktCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_enable_fwd_pkt_count_clear_on_read(self, arg):
        '''
         xpsQosQcEnableFwdPktCountClearOnRead: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosQcEnableFwdPktCountClearOnRead(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosQcIsEnableFwdPktCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_is_enable_fwd_pkt_count_clear_on_read(self, arg):
        '''
         xpsQosQcIsEnableFwdPktCountClearOnRead: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosQcIsEnableFwdPktCountClearOnRead(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosQcEnableFwdPktByteCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_enable_fwd_pkt_byte_count_clear_on_read(self, arg):
        '''
         xpsQosQcEnableFwdPktByteCountClearOnRead: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosQcEnableFwdPktByteCountClearOnRead(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosQcIsEnableFwdPktByteCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_is_enable_fwd_pkt_byte_count_clear_on_read(self, arg):
        '''
         xpsQosQcIsEnableFwdPktByteCountClearOnRead: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosQcIsEnableFwdPktByteCountClearOnRead(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosQcEnableDropPktCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_enable_drop_pkt_count_clear_on_read(self, arg):
        '''
         xpsQosQcEnableDropPktCountClearOnRead: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosQcEnableDropPktCountClearOnRead(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosQcIsEnableDropPktCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_is_enable_drop_pkt_count_clear_on_read(self, arg):
        '''
         xpsQosQcIsEnableDropPktCountClearOnRead: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosQcIsEnableDropPktCountClearOnRead(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosQcEnableDropPktByteCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_enable_drop_pkt_byte_count_clear_on_read(self, arg):
        '''
         xpsQosQcEnableDropPktByteCountClearOnRead: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosQcEnableDropPktByteCountClearOnRead(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosQcIsEnableDropPktByteCountClearOnRead
    #/********************************************************************************/
    def do_qos_qc_is_enable_drop_pkt_byte_count_clear_on_read(self, arg):
        '''
         xpsQosQcIsEnableDropPktByteCountClearOnRead: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosQcIsEnableDropPktByteCountClearOnRead(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosQcGetPfcPageCount
    #/********************************************************************************/
    def do_qos_qc_get_pfc_page_count(self, arg):
        '''
         xpsQosQcGetPfcPageCount: Enter [ devId,port,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosQcGetPfcPageCount(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_3)))
                pass
            delete_uint32_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosQcGetGlobalPageCount
    #/********************************************************************************/
    def do_qos_qc_get_global_page_count(self, arg):
        '''
         xpsQosQcGetGlobalPageCount: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            count_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosQcGetGlobalPageCount(args[0],count_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_1)))
                pass
            delete_uint32_tp(count_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosQcGetGlobalMulticastPageCount
    #/********************************************************************************/
    def do_qos_qc_get_global_multicast_page_count(self, arg):
        '''
         xpsQosQcGetGlobalMulticastPageCount: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            count_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosQcGetGlobalMulticastPageCount(args[0],count_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_1)))
                pass
            delete_uint32_tp(count_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperEnablePortShaping
    #/********************************************************************************/
    def do_qos_shaper_enable_port_shaping(self, arg):
        '''
         xpsQosShaperEnablePortShaping: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosShaperEnablePortShaping(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperIsPortShapingEnabled
    #/********************************************************************************/
    def do_qos_shaper_is_port_shaping_enabled(self, arg):
        '''
         xpsQosShaperIsPortShapingEnabled: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosShaperIsPortShapingEnabled(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperSetPortShaperMtu
    #/********************************************************************************/
    def do_qos_shaper_set_port_shaper_mtu(self, arg):
        '''
         xpsQosShaperSetPortShaperMtu: Enter [ devId,mtuInBytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mtuInBytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mtuInBytes=%d' % (args[0],args[1]))
            ret = xpsQosShaperSetPortShaperMtu(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetPortShaperMtu
    #/********************************************************************************/
    def do_qos_shaper_get_port_shaper_mtu(self, arg):
        '''
         xpsQosShaperGetPortShaperMtu: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            mtuInBytes_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosShaperGetPortShaperMtu(args[0],mtuInBytes_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuInBytes = %d' % (uint32_tp_value(mtuInBytes_Ptr_1)))
                pass
            delete_uint32_tp(mtuInBytes_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperSetPortShaperEmptyThreshold
    #/********************************************************************************/
    def do_qos_shaper_set_port_shaper_empty_threshold(self, arg):
        '''
         xpsQosShaperSetPortShaperEmptyThreshold: Enter [ devId,emptyThld ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,emptyThld ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, emptyThld=%d' % (args[0],args[1]))
            ret = xpsQosShaperSetPortShaperEmptyThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetPortShaperEmptyThreshold
    #/********************************************************************************/
    def do_qos_shaper_get_port_shaper_empty_threshold(self, arg):
        '''
         xpsQosShaperGetPortShaperEmptyThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            emptyThld_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosShaperGetPortShaperEmptyThreshold(args[0],emptyThld_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('emptyThld = %d' % (uint32_tp_value(emptyThld_Ptr_1)))
                pass
            delete_uint32_tp(emptyThld_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperConfigurePortShaper
    #/********************************************************************************/
    def do_qos_shaper_configure_port_shaper(self, arg):
        '''
         xpsQosShaperConfigurePortShaper: Enter [ devId,devPort,rateKbps,maxBurstByteSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,rateKbps,maxBurstByteSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, rateKbps=0x%x, maxBurstByteSize=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosShaperConfigurePortShaper(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetPortShaperConfiguration
    #/********************************************************************************/
    def do_qos_shaper_get_port_shaper_configuration(self, arg):
        '''
         xpsQosShaperGetPortShaperConfiguration: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rateKbps_Ptr_2 = new_uint64_tp()
            maxBurstByteSize_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosShaperGetPortShaperConfiguration(args[0],args[1],rateKbps_Ptr_2,maxBurstByteSize_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rateKbps = %d' % (uint64_tp_value(rateKbps_Ptr_2)))
                print('maxBurstByteSize = %d' % (uint32_tp_value(maxBurstByteSize_Ptr_3)))
                pass
            delete_uint32_tp(maxBurstByteSize_Ptr_3)
            delete_uint64_tp(rateKbps_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosShaperSetPortShaperEnable
    #/********************************************************************************/
    def do_qos_shaper_set_port_shaper_enable(self, arg):
        '''
         xpsQosShaperSetPortShaperEnable: Enter [ devId,devPort,enableShaper ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,enableShaper ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, enableShaper=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperSetPortShaperEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetPortShaperEnable
    #/********************************************************************************/
    def do_qos_shaper_get_port_shaper_enable(self, arg):
        '''
         xpsQosShaperGetPortShaperEnable: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enableShaper_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosShaperGetPortShaperEnable(args[0],args[1],enableShaper_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableShaper = %d' % (uint32_tp_value(enableShaper_Ptr_2)))
                pass
            delete_uint32_tp(enableShaper_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosShaperGetPortShaperTableIndex
    #/********************************************************************************/
    def do_qos_shaper_get_port_shaper_table_index(self, arg):
        '''
         xpsQosShaperGetPortShaperTableIndex: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            index_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpsQosShaperGetPortShaperTableIndex(args[0],args[1],index_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (uint32_tp_value(index_Ptr_2)))
                pass
            delete_uint32_tp(index_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosShaperEnableQueueShaping
    #/********************************************************************************/
    def do_qos_shaper_enable_queue_shaping(self, arg):
        '''
         xpsQosShaperEnableQueueShaping: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosShaperEnableQueueShaping(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperIsQueueShapingEnabled
    #/********************************************************************************/
    def do_qos_shaper_is_queue_shaping_enabled(self, arg):
        '''
         xpsQosShaperIsQueueShapingEnabled: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosShaperIsQueueShapingEnabled(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperSetQueueShaperMtu
    #/********************************************************************************/
    def do_qos_shaper_set_queue_shaper_mtu(self, arg):
        '''
         xpsQosShaperSetQueueShaperMtu: Enter [ devId,mtuInBytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mtuInBytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mtuInBytes=%d' % (args[0],args[1]))
            ret = xpsQosShaperSetQueueShaperMtu(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueShaperMtu
    #/********************************************************************************/
    def do_qos_shaper_get_queue_shaper_mtu(self, arg):
        '''
         xpsQosShaperGetQueueShaperMtu: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            mtuInBytes_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosShaperGetQueueShaperMtu(args[0],mtuInBytes_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuInBytes = %d' % (uint32_tp_value(mtuInBytes_Ptr_1)))
                pass
            delete_uint32_tp(mtuInBytes_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperSetQueueShaperEmptyThreshold
    #/********************************************************************************/
    def do_qos_shaper_set_queue_shaper_empty_threshold(self, arg):
        '''
         xpsQosShaperSetQueueShaperEmptyThreshold: Enter [ devId,emptyThld ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,emptyThld ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, emptyThld=%d' % (args[0],args[1]))
            ret = xpsQosShaperSetQueueShaperEmptyThreshold(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueShaperEmptyThreshold
    #/********************************************************************************/
    def do_qos_shaper_get_queue_shaper_empty_threshold(self, arg):
        '''
         xpsQosShaperGetQueueShaperEmptyThreshold: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            emptyThld_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosShaperGetQueueShaperEmptyThreshold(args[0],emptyThld_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('emptyThld = %d' % (uint32_tp_value(emptyThld_Ptr_1)))
                pass
            delete_uint32_tp(emptyThld_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosShaperConfigureQueueSlowShaper
    #/********************************************************************************/
    def do_qos_shaper_configure_queue_slow_shaper(self, arg):
        '''
         xpsQosShaperConfigureQueueSlowShaper: Enter [ devId,devPort,queueNum,rateKbps,maxBurstByteSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,rateKbps,maxBurstByteSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, rateKbps=0x%x, maxBurstByteSize=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosShaperConfigureQueueSlowShaper(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueSlowShaperConfiguation
    #/********************************************************************************/
    def do_qos_shaper_get_queue_slow_shaper_configuation(self, arg):
        '''
         xpsQosShaperGetQueueSlowShaperConfiguation: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            rateKbps_Ptr_3 = new_uint64_tp()
            maxBurstByteSize_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperGetQueueSlowShaperConfiguation(args[0],args[1],args[2],rateKbps_Ptr_3,maxBurstByteSize_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rateKbps = %d' % (uint64_tp_value(rateKbps_Ptr_3)))
                print('maxBurstByteSize = %d' % (uint32_tp_value(maxBurstByteSize_Ptr_4)))
                pass
            delete_uint32_tp(maxBurstByteSize_Ptr_4)
            delete_uint64_tp(rateKbps_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosShaperSetQueueSlowShaperEnable
    #/********************************************************************************/
    def do_qos_shaper_set_queue_slow_shaper_enable(self, arg):
        '''
         xpsQosShaperSetQueueSlowShaperEnable: Enter [ devId,devPort,queueNum,enableShaper ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,enableShaper ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, enableShaper=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosShaperSetQueueSlowShaperEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueSlowShaperEnable
    #/********************************************************************************/
    def do_qos_shaper_get_queue_slow_shaper_enable(self, arg):
        '''
         xpsQosShaperGetQueueSlowShaperEnable: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enableShaper_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperGetQueueSlowShaperEnable(args[0],args[1],args[2],enableShaper_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableShaper = %d' % (uint32_tp_value(enableShaper_Ptr_3)))
                pass
            delete_uint32_tp(enableShaper_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueSlowShaperTableIndex
    #/********************************************************************************/
    def do_qos_shaper_get_queue_slow_shaper_table_index(self, arg):
        '''
         xpsQosShaperGetQueueSlowShaperTableIndex: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            index_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperGetQueueSlowShaperTableIndex(args[0],args[1],args[2],index_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (uint32_tp_value(index_Ptr_3)))
                pass
            delete_uint32_tp(index_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosShaperBindQueueToFastShaper
    #/********************************************************************************/
    def do_qos_shaper_bind_queue_to_fast_shaper(self, arg):
        '''
         xpsQosShaperBindQueueToFastShaper: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            fastShaperNum_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperBindQueueToFastShaper(args[0],args[1],args[2],fastShaperNum_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fastShaperNum = %d' % (uint32_tp_value(fastShaperNum_Ptr_3)))
                pass
            delete_uint32_tp(fastShaperNum_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosShaperConfigureQueueFastShaper
    #/********************************************************************************/
    def do_qos_shaper_configure_queue_fast_shaper(self, arg):
        '''
         xpsQosShaperConfigureQueueFastShaper: Enter [ devId,fastShaperNum,rateKbps,maxBurstByteSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fastShaperNum,rateKbps,maxBurstByteSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, fastShaperNum=%d, rateKbps=0x%x, maxBurstByteSize=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosShaperConfigureQueueFastShaper(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueFastShaperConfiguation
    #/********************************************************************************/
    def do_qos_shaper_get_queue_fast_shaper_configuation(self, arg):
        '''
         xpsQosShaperGetQueueFastShaperConfiguation: Enter [ devId,fastShaperNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fastShaperNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rateKbps_Ptr_2 = new_uint64_tp()
            maxBurstSize_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, fastShaperNum=%d' % (args[0],args[1]))
            ret = xpsQosShaperGetQueueFastShaperConfiguation(args[0],args[1],rateKbps_Ptr_2,maxBurstSize_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rateKbps = %d' % (uint64_tp_value(rateKbps_Ptr_2)))
                print('maxBurstSize = %d' % (uint32_tp_value(maxBurstSize_Ptr_3)))
                pass
            delete_uint32_tp(maxBurstSize_Ptr_3)
            delete_uint64_tp(rateKbps_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosShaperSetQueueFastShaperEnable
    #/********************************************************************************/
    def do_qos_shaper_set_queue_fast_shaper_enable(self, arg):
        '''
         xpsQosShaperSetQueueFastShaperEnable: Enter [ devId,fastShaperNum,enableShaper ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fastShaperNum,enableShaper ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, fastShaperNum=%d, enableShaper=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperSetQueueFastShaperEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueFastShaperEnable
    #/********************************************************************************/
    def do_qos_shaper_get_queue_fast_shaper_enable(self, arg):
        '''
         xpsQosShaperGetQueueFastShaperEnable: Enter [ devId,fastShaperNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fastShaperNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enableShaper_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, fastShaperNum=%d' % (args[0],args[1]))
            ret = xpsQosShaperGetQueueFastShaperEnable(args[0],args[1],enableShaper_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableShaper = %d' % (uint32_tp_value(enableShaper_Ptr_2)))
                pass
            delete_uint32_tp(enableShaper_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosShaperGetQueueFastShaperTableIndex
    #/********************************************************************************/
    def do_qos_shaper_get_queue_fast_shaper_table_index(self, arg):
        '''
         xpsQosShaperGetQueueFastShaperTableIndex: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            index_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosShaperGetQueueFastShaperTableIndex(args[0],args[1],args[2],index_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (uint32_tp_value(index_Ptr_3)))
                pass
            delete_uint32_tp(index_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetQueueSchedulerDWRR
    #/********************************************************************************/
    def do_qos_set_queue_scheduler_dwrr(self, arg):
        '''
         xpsQosSetQueueSchedulerDWRR: Enter [ devId,devPort,queueNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosSetQueueSchedulerDWRR(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueSchedulerDWRR
    #/********************************************************************************/
    def do_qos_get_queue_scheduler_dwrr(self, arg):
        '''
         xpsQosGetQueueSchedulerDWRR: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosGetQueueSchedulerDWRR(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetQueueSchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_set_queue_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosSetQueueSchedulerDWRRWeight: Enter [ devId,devPort,queueNum,weight ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,weight ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, weight=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosSetQueueSchedulerDWRRWeight(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueSchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_get_queue_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosGetQueueSchedulerDWRRWeight: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            weight_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosGetQueueSchedulerDWRRWeight(args[0],args[1],args[2],weight_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('weight = %d' % (uint32_tp_value(weight_Ptr_3)))
                pass
            delete_uint32_tp(weight_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetQueueSchedulerDWRRMtu
    #/********************************************************************************/
    def do_qos_set_queue_scheduler_dwrr_mtu(self, arg):
        '''
         xpsQosSetQueueSchedulerDWRRMtu: Enter [ devId,mtuInBytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mtuInBytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mtuInBytes=%d' % (args[0],args[1]))
            ret = xpsQosSetQueueSchedulerDWRRMtu(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueSchedulerDWRRMtu
    #/********************************************************************************/
    def do_qos_get_queue_scheduler_dwrr_mtu(self, arg):
        '''
         xpsQosGetQueueSchedulerDWRRMtu: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            mtuInBytes_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosGetQueueSchedulerDWRRMtu(args[0],mtuInBytes_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuInBytes = %d' % (uint32_tp_value(mtuInBytes_Ptr_1)))
                pass
            delete_uint32_tp(mtuInBytes_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosSetQueueSchedulerSP
    #/********************************************************************************/
    def do_qos_set_queue_scheduler_sp(self, arg):
        '''
         xpsQosSetQueueSchedulerSP: Enter [ devId,devPort,queueNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosSetQueueSchedulerSP(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueSchedulerSP
    #/********************************************************************************/
    def do_qos_get_queue_scheduler_sp(self, arg):
        '''
         xpsQosGetQueueSchedulerSP: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosGetQueueSchedulerSP(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetQueueSchedulerSPPriority
    #/********************************************************************************/
    def do_qos_set_queue_scheduler_sp_priority(self, arg):
        '''
         xpsQosSetQueueSchedulerSPPriority: Enter [ devId,devPort,queueNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, priority=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosSetQueueSchedulerSPPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueSchedulerSPPriority
    #/********************************************************************************/
    def do_qos_get_queue_scheduler_sp_priority(self, arg):
        '''
         xpsQosGetQueueSchedulerSPPriority: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            priority_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosGetQueueSchedulerSPPriority(args[0],args[1],args[2],priority_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_3)))
                pass
            delete_uint32_tp(priority_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetQueueDequeueEnable
    #/********************************************************************************/
    def do_qos_set_queue_dequeue_enable(self, arg):
        '''
         xpsQosSetQueueDequeueEnable: Enter [ devId,portNum,queueNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosSetQueueDequeueEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueDequeueEnable
    #/********************************************************************************/
    def do_qos_get_queue_dequeue_enable(self, arg):
        '''
         xpsQosGetQueueDequeueEnable: Enter [ devId,portNum,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosGetQueueDequeueEnable(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetH1SchedulerDWRR
    #/********************************************************************************/
    def do_qos_set_h1_scheduler_dwrr(self, arg):
        '''
         xpsQosSetH1SchedulerDWRR: Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, enable=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH1SchedulerDWRR(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH1SchedulerDWRR
    #/********************************************************************************/
    def do_qos_get_h1_scheduler_dwrr(self, arg):
        '''
         xpsQosGetH1SchedulerDWRR: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH1SchedulerDWRR(args[0],xpTxqQueuePathToPort_t_Ptr,enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH1SchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_set_h1_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosSetH1SchedulerDWRRWeight: Enter [ devId,queueNum,h1Num,h2Num,portNum,weight ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,weight ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, weight=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH1SchedulerDWRRWeight(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH1SchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_get_h1_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosGetH1SchedulerDWRRWeight: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            weight_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH1SchedulerDWRRWeight(args[0],xpTxqQueuePathToPort_t_Ptr,weight_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('weight = %d' % (uint32_tp_value(weight_Ptr_2)))
                pass
            delete_uint32_tp(weight_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH1SchedulerSP
    #/********************************************************************************/
    def do_qos_set_h1_scheduler_sp(self, arg):
        '''
         xpsQosSetH1SchedulerSP: Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, enable=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH1SchedulerSP(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH1SchedulerSP
    #/********************************************************************************/
    def do_qos_get_h1_scheduler_sp(self, arg):
        '''
         xpsQosGetH1SchedulerSP: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH1SchedulerSP(args[0],xpTxqQueuePathToPort_t_Ptr,enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH1SchedulerSPPriority
    #/********************************************************************************/
    def do_qos_set_h1_scheduler_sp_priority(self, arg):
        '''
         xpsQosSetH1SchedulerSPPriority: Enter [ devId,queueNum,h1Num,h2Num,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,priority ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, priority=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH1SchedulerSPPriority(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH1SchedulerSPPriority
    #/********************************************************************************/
    def do_qos_get_h1_scheduler_sp_priority(self, arg):
        '''
         xpsQosGetH1SchedulerSPPriority: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            priority_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH1SchedulerSPPriority(args[0],xpTxqQueuePathToPort_t_Ptr,priority_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('weight = %d' % (uint32_tp_value(priority_Ptr_2)))
                pass
            delete_uint32_tp(priority_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH2SchedulerDWRR
    #/********************************************************************************/
    def do_qos_set_h2_scheduler_dwrr(self, arg):
        '''
         xpsQosSetH2SchedulerDWRR: Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, enable=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH2SchedulerDWRR(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH2SchedulerDWRR
    #/********************************************************************************/
    def do_qos_get_h2_scheduler_dwrr(self, arg):
        '''
         xpsQosGetH2SchedulerDWRR: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH2SchedulerDWRR(args[0],xpTxqQueuePathToPort_t_Ptr,enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH2SchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_set_h2_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosSetH2SchedulerDWRRWeight: Enter [ devId,queueNum,h1Num,h2Num,portNum,weight ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,weight ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, weight=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH2SchedulerDWRRWeight(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH2SchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_get_h2_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosGetH2SchedulerDWRRWeight: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            weight_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH2SchedulerDWRRWeight(args[0],xpTxqQueuePathToPort_t_Ptr,weight_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('weight = %d' % (uint32_tp_value(weight_Ptr_2)))
                pass
            delete_uint32_tp(weight_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH2SchedulerSP
    #/********************************************************************************/
    def do_qos_set_h2_scheduler_sp(self, arg):
        '''
         xpsQosSetH2SchedulerSP: Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,enable ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, enable=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH2SchedulerSP(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH2SchedulerSP
    #/********************************************************************************/
    def do_qos_get_h2_scheduler_sp(self, arg):
        '''
         xpsQosGetH2SchedulerSP: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH2SchedulerSP(args[0],xpTxqQueuePathToPort_t_Ptr,enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetH2SchedulerSPPriority
    #/********************************************************************************/
    def do_qos_set_h2_scheduler_sp_priority(self, arg):
        '''
         xpsQosSetH2SchedulerSPPriority: Enter [ devId,queueNum,h1Num,h2Num,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum,priority ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d, priority=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsQosSetH2SchedulerSPPriority(args[0],xpTxqQueuePathToPort_t_Ptr,args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetH2SchedulerSPPriority
    #/********************************************************************************/
    def do_qos_get_h2_scheduler_sp_priority(self, arg):
        '''
         xpsQosGetH2SchedulerSPPriority: Enter [ devId,queueNum,h1Num,h2Num,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,queueNum,h1Num,h2Num,portNum ]')
        else:
            args[0] = int(args[0])
            xpTxqQueuePathToPort_t_Ptr = xpTxqQueuePathToPort_t()
            xpTxqQueuePathToPort_t_Ptr.queueNum = int(args[1])
            xpTxqQueuePathToPort_t_Ptr.h1Num = int(args[2])
            xpTxqQueuePathToPort_t_Ptr.h2Num = int(args[3])
            xpTxqQueuePathToPort_t_Ptr.portNum = int(args[4])
            priority_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, queueNum=%d, h1Num = %d, h2Num = %d, portNum = %d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsQosGetH2SchedulerSPPriority(args[0],xpTxqQueuePathToPort_t_Ptr,priority_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('weight = %d' % (uint32_tp_value(priority_Ptr_2)))
                pass
            delete_uint32_tp(priority_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetPortSchedulerDWRR
    #/********************************************************************************/
    def do_qos_set_port_scheduler_dwrr(self, arg):
        '''
         xpsQosSetPortSchedulerDWRR: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosSetPortSchedulerDWRR(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetPortSchedulerDWRR
    #/********************************************************************************/
    def do_qos_get_port_scheduler_dwrr(self, arg):
        '''
         xpsQosGetPortSchedulerDWRR: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosGetPortSchedulerDWRR(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetPortSchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_set_port_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosSetPortSchedulerDWRRWeight: Enter [ devId,portNum,weight ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,weight ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, weight=%d' % (args[0],args[1],args[2]))
            ret = xpsQosSetPortSchedulerDWRRWeight(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetPortSchedulerDWRRWeight
    #/********************************************************************************/
    def do_qos_get_port_scheduler_dwrr_weight(self, arg):
        '''
         xpsQosGetPortSchedulerDWRRWeight: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            weight_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosGetPortSchedulerDWRRWeight(args[0],args[1],weight_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('weight = %d' % (uint32_tp_value(weight_Ptr_2)))
                pass
            delete_uint32_tp(weight_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetPortSchedulerSP
    #/********************************************************************************/
    def do_qos_set_port_scheduler_sp(self, arg):
        '''
         xpsQosSetPortSchedulerSP: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosSetPortSchedulerSP(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetPortSchedulerSP
    #/********************************************************************************/
    def do_qos_get_port_scheduler_sp(self, arg):
        '''
         xpsQosGetPortSchedulerSP: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosGetPortSchedulerSP(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSetPortSchedulerSPPriority
    #/********************************************************************************/
    def do_qos_set_port_scheduler_sp_priority(self, arg):
        '''
         xpsQosSetPortSchedulerSPPriority: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpsQosSetPortSchedulerSPPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetPortSchedulerSPPriority
    #/********************************************************************************/
    def do_qos_get_port_scheduler_sp_priority(self, arg):
        '''
         xpsQosGetPortSchedulerSPPriority: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            priority_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosGetPortSchedulerSPPriority(args[0],args[1],priority_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('priority = %d' % (uint32_tp_value(priority_Ptr_2)))
                pass
            delete_uint32_tp(priority_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosGetQueueToPortPath
    #/********************************************************************************/
    def do_qos_get_queue_to_port_path(self, arg):
        '''
         xpsQosGetQueueToPortPath: Enter [ devId,portNum,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            path_Ptr_3 = new_xpTxqQueuePathToPort_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpsQosGetQueueToPortPath(args[0],args[1],args[2],path_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('queueNum = %d' % (path_Ptr_3.queueNum))
                print('h1Num = %d' % (path_Ptr_3.h1Num))
                print('h2Num = %d' % (path_Ptr_3.h2Num))
                print('portNum = %d' % (path_Ptr_3.portNum))
                pass
            delete_xpTxqQueuePathToPort_tp(path_Ptr_3)
    #/********************************************************************************/
    # command for xpsQosSetQueueDequeueEnable
    #/********************************************************************************/
    def do_qos_set_port_dequeue_enable(self, arg):
        '''
         xpsQosSetPortDequeueEnable: Enter [ devId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosSetPortDequeueEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosGetQueueDequeueEnable
    #/********************************************************************************/
    def do_qos_get_port_dequeue_enable(self, arg):
        '''
         xpsQosGetPortDequeueEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsQosGetPortDequeueEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSchedulerSetPortPacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_set_port_pacing_interval(self, arg):
        '''
         xpsQosSchedulerSetPortPacingInterval: Enter [ devId,speed,numCycles ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,speed,numCycles ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, speed=%d, numCycles=%d' % (args[0],args[1],args[2]))
            ret = xpsQosSchedulerSetPortPacingInterval(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosSchedulerGetPortPacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_get_port_pacing_interval(self, arg):
        '''
         xpsQosSchedulerGetPortPacingInterval: Enter [ devId,speed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,speed ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numCycles_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, speed=%d' % (args[0],args[1]))
            ret = xpsQosSchedulerGetPortPacingInterval(args[0],args[1],numCycles_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numCycles = %d' % (uint32_tp_value(numCycles_Ptr_2)))
                pass
            delete_uint32_tp(numCycles_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosSchedulerSetH2PacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_set_h2_pacing_interval(self, arg):
        '''
         xpsQosSchedulerSetH2PacingInterval: Enter [ devId,numCycles ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numCycles ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numCycles=%d' % (args[0],args[1]))
            ret = xpsQosSchedulerSetH2PacingInterval(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosSchedulerGetH2PacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_get_h2_pacing_interval(self, arg):
        '''
         xpsQosSchedulerGetH2PacingInterval: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numCycles_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosSchedulerGetH2PacingInterval(args[0],numCycles_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numCycles = %d' % (uint32_tp_value(numCycles_Ptr_1)))
                pass
            delete_uint32_tp(numCycles_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosSchedulerSetH1PacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_set_h1_pacing_interval(self, arg):
        '''
         xpsQosSchedulerSetH1PacingInterval: Enter [ devId,numCycles ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numCycles ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numCycles=%d' % (args[0],args[1]))
            ret = xpsQosSchedulerSetH1PacingInterval(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosSchedulerGetH1PacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_get_h1_pacing_interval(self, arg):
        '''
         xpsQosSchedulerGetH1PacingInterval: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numCycles_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosSchedulerGetH1PacingInterval(args[0],numCycles_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numCycles = %d' % (uint32_tp_value(numCycles_Ptr_1)))
                pass
            delete_uint32_tp(numCycles_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosSchedulerSetQueuePacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_set_queue_pacing_interval(self, arg):
        '''
         xpsQosSchedulerSetQueuePacingInterval: Enter [ devId,numCycles ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numCycles ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numCycles=%d' % (args[0],args[1]))
            ret = xpsQosSchedulerSetQueuePacingInterval(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosSchedulerGetQueuePacingInterval
    #/********************************************************************************/
    def do_qos_scheduler_get_queue_pacing_interval(self, arg):
        '''
         xpsQosSchedulerGetQueuePacingInterval: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numCycles_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsQosSchedulerGetQueuePacingInterval(args[0],numCycles_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numCycles = %d' % (uint32_tp_value(numCycles_Ptr_1)))
                pass
            delete_uint32_tp(numCycles_Ptr_1)
    #/********************************************************************************/
    # command for xpsQosSetEgressQosMapEn
    #/********************************************************************************/
    def do_qos_set_egress_qos_map_en(self, arg):
        '''
         xpsQosSetEgressQosMapEn: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsQosSetEgressQosMapEn(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapAdd
    #/********************************************************************************/
    def do_qos_egress_qos_map_add(self, arg):
        '''
         xpsQosEgressQosMapAdd: Enter [ devId,eVif,tc,dp,l2QosEn,l3QosEn,pcp,dei,dscp]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,eVif,tc,dp,l2QosEn,l3QosEn,pcp,dei,dscp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            xpEgressCosMapData_t_Ptr = xpEgressCosMapData_t()
            xpEgressCosMapData_t_Ptr.l2QosEn = int(args[4])
            xpEgressCosMapData_t_Ptr.l3QosEn = int(args[5])
            xpEgressCosMapData_t_Ptr.pcp = int(args[6])
            xpEgressCosMapData_t_Ptr.dei = int(args[7])
            xpEgressCosMapData_t_Ptr.dscp = int(args[8])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, eVif=%d, tc=%d, dp=%d, l2QosEn=%d, l3QosEn=%d, pcp=%d, dei=%d, dscp=%d' % (args[0],args[1],args[2],args[3],xpEgressCosMapData_t_Ptr.l2QosEn,xpEgressCosMapData_t_Ptr.l3QosEn,xpEgressCosMapData_t_Ptr.pcp,xpEgressCosMapData_t_Ptr.dei,xpEgressCosMapData_t_Ptr.dscp))
            ret = xpsQosEgressQosMapAdd(args[0],args[1],args[2],args[3],xpEgressCosMapData_t_Ptr,xpsHashIndexList_t_Ptr)
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
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)

    #/********************************************************************************/
    # command for xpsQosEgressQosMapUpdate
    #/********************************************************************************/
    def do_qos_egress_qos_map_update(self, arg):
        '''
         xpsQosEgressQosMapUpdate: Enter [ devId,index,l2QosEn,l3QosEn,pcp,dei,dscp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,l2QosEn,l3QosEn,pcp,dei,dscp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpEgressCosMapData_t_Ptr = xpEgressCosMapData_t()
            xpEgressCosMapData_t_Ptr.l2QosEn = int(args[2])
            xpEgressCosMapData_t_Ptr.l3QosEn = int(args[3])
            xpEgressCosMapData_t_Ptr.pcp = int(args[4])
            xpEgressCosMapData_t_Ptr.dei = int(args[5])
            xpEgressCosMapData_t_Ptr.dscp = int(args[6])
            #print('Input Arguments are, devId=%d, index=%d, l2QosEn=%d, l3QosEn=%d, pcp=%d, dei=%d, dscp=%d' % (args[0],args[1],xpEgressCosMapData_t_Ptr.l2QosEn,xpEgressCosMapData_t_Ptr.l3QosEn,xpEgressCosMapData_t_Ptr.pcp,xpEgressCosMapData_t_Ptr.dei,xpEgressCosMapData_t_Ptr.dscp))
            ret = xpsQosEgressQosMapUpdate(args[0],args[1],xpEgressCosMapData_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetIndex
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_index(self, arg):
        '''
         xpsQosEgressQosMapGetIndex: Enter [ devId,eVif,tc,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,eVif,tc,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            index_Ptr_4 = new_intp()
            #print('Input Arguments are, devId=%d, eVif=%d, tc=%d, dp=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosEgressQosMapGetIndex(args[0],args[1],args[2],args[3],index_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (intp_value(index_Ptr_4)))
                pass
            delete_intp(index_Ptr_4)
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetEntry
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_entry(self, arg):
        '''
         xpsQosEgressQosMapGetEntry: Enter [ devId,eVif,tc,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,eVif,tc,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            xpEgressCosMapData_t_Ptr = new_xpEgressCosMapData_tp()
            #print('Input Arguments are, devId=%d, eVif=%d, tc=%d, dp=%d, l2QosEn=%d, l3QosEn=%d, pcp=%d, dei=%d, dscp=%d' % (args[0],args[1],args[2],args[3],xpEgressCosMapData_t_Ptr.l2QosEn,xpEgressCosMapData_t_Ptr.l3QosEn,xpEgressCosMapData_t_Ptr.pcp,xpEgressCosMapData_t_Ptr.dei,xpEgressCosMapData_t_Ptr.dscp))
            ret = xpsQosEgressQosMapGetEntry(args[0],args[1],args[2],args[3],xpEgressCosMapData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l2QosEn = %d' % (xpEgressCosMapData_t_Ptr.l2QosEn))
                print('l3QosEn = %d' % (xpEgressCosMapData_t_Ptr.l3QosEn))
                print('pcp = %d' % (xpEgressCosMapData_t_Ptr.pcp))
                print('dei = %d' % (xpEgressCosMapData_t_Ptr.dei))
                print('dscp = %d' % (xpEgressCosMapData_t_Ptr.dscp))
                pass
            delete_xpEgressCosMapData_tp(xpEgressCosMapData_t_Ptr)

    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetEntryByIndex
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_entry_by_index(self, arg):
        '''
         xpsQosEgressQosMapGetEntryByIndex: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpEgressCosMapData_t_Ptr = new_xpEgressCosMapData_tp()
            #print('Input Arguments are, devId=%d, index=%d, l2QosEn=%d, l3QosEn=%d, pcp=%d, dei=%d, dscp=%d' % (args[0],args[1],xpEgressCosMapData_t_Ptr.l2QosEn,xpEgressCosMapData_t_Ptr.l3QosEn,xpEgressCosMapData_t_Ptr.pcp,xpEgressCosMapData_t_Ptr.dei,xpEgressCosMapData_t_Ptr.dscp))
            ret = xpsQosEgressQosMapGetEntryByIndex(args[0],args[1],xpEgressCosMapData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l2QosEn = %d' % (xpEgressCosMapData_t_Ptr.l2QosEn))
                print('l3QosEn = %d' % (xpEgressCosMapData_t_Ptr.l3QosEn))
                print('pcp = %d' % (xpEgressCosMapData_t_Ptr.pcp))
                print('dei = %d' % (xpEgressCosMapData_t_Ptr.dei))
                print('dscp = %d' % (xpEgressCosMapData_t_Ptr.dscp))
                pass
            delete_xpEgressCosMapData_tp(xpEgressCosMapData_t_Ptr)

    #/********************************************************************************/
    # command for xpsQosEgressQosMapRemoveEntry
    #/********************************************************************************/
    def do_qos_egress_qos_map_remove_entry(self, arg):
        '''
         xpsQosEgressQosMapRemoveEntry: Enter [ devId,eVif,tc,dp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,eVif,tc,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, eVif=%d, tc=%d, dp=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosEgressQosMapRemoveEntry(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapRemoveEntryByIndex
    #/********************************************************************************/
    def do_qos_egress_qos_map_remove_entry_by_index(self, arg):
        '''
         xpsQosEgressQosMapRemoveEntryByIndex: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapRemoveEntryByIndex(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapSetL2QosPriority
    #/********************************************************************************/
    def do_qos_egress_qos_map_set_l2_qos_priority(self, arg):
        '''
         xpsQosEgressQosMapSetL2QosPriority: Enter [ devId,index,pcpVal,deiVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,pcpVal,deiVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, index=%d, pcpVal=%d, deiVal=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsQosEgressQosMapSetL2QosPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapSetL3QosPriority
    #/********************************************************************************/
    def do_qos_egress_qos_map_set_l3_qos_priority(self, arg):
        '''
         xpsQosEgressQosMapSetL3QosPriority: Enter [ devId,index,dscpVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,dscpVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, dscpVal=%d' % (args[0],args[1],args[2]))
            ret = xpsQosEgressQosMapSetL3QosPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapSetL2QosPriorityEn
    #/********************************************************************************/
    def do_qos_egress_qos_map_set_l2_qos_priority_en(self, arg):
        '''
         xpsQosEgressQosMapSetL2QosPriorityEn: Enter [ devId,index,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosEgressQosMapSetL2QosPriorityEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapSetL3QosPriorityEn
    #/********************************************************************************/
    def do_qos_egress_qos_map_set_l3_qos_priority_en(self, arg):
        '''
         xpsQosEgressQosMapSetL3QosPriorityEn: Enter [ devId,index,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsQosEgressQosMapSetL3QosPriorityEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetL2QosPriority
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_l2_qos_priority(self, arg):
        '''
         xpsQosEgressQosMapGetL2QosPriority: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pcpVal_Ptr_2 = new_uint32_tp()
            deiVal_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapGetL2QosPriority(args[0],args[1],pcpVal_Ptr_2,deiVal_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pcpVal = %d' % (uint32_tp_value(pcpVal_Ptr_2)))
                print('deiVal = %d' % (uint32_tp_value(deiVal_Ptr_3)))
                pass
            delete_uint32_tp(deiVal_Ptr_3)
            delete_uint32_tp(pcpVal_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetL3QosPriority
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_l3_qos_priority(self, arg):
        '''
         xpsQosEgressQosMapGetL3QosPriority: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dscpVal_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapGetL3QosPriority(args[0],args[1],dscpVal_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dscpVal = %d' % (uint32_tp_value(dscpVal_Ptr_2)))
                pass
            delete_uint32_tp(dscpVal_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetL2QosPriorityEn
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_l2_qos_priority_en(self, arg):
        '''
         xpsQosEgressQosMapGetL2QosPriorityEn: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapGetL2QosPriorityEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosEgressQosMapGetL3QosPriorityEn
    #/********************************************************************************/
    def do_qos_egress_qos_map_get_l3_qos_priority_en(self, arg):
        '''
         xpsQosEgressQosMapGetL3QosPriorityEn: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsQosEgressQosMapGetL3QosPriorityEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsQosEgressQosMapFlushEntryByEgressVif
    #/********************************************************************************/
    def do_qos_egress_qos_map_flush_entry_by_egress_vif(self, arg):
        '''
         xpsQosEgressQosMapFlushEntryByEgressVif: Enter devId,vif [ updateShadow ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,vif [ updateShadow ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            if(len(args) > numArgsReq):
                args[2] = int(args[2])
                #print('Input Arguments are, devId=%d, vif=%d, updateShadow=%d' % (args[0],args[1],args[2]))
                ret = xpsQosEgressQosMapFlushEntryByEgressVifWithShadowUpdate(args[0],args[1],args[2])
            else:
                #print('Input Arguments are, devId=%d, vif=%d' % (args[0],args[1]))
                ret = xpsQosEgressQosMapFlushEntryByEgressVifWithShadowUpdate(args[0],args[1], 1)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsQosEgressQosMapClearBucketState
    #/********************************************************************************/
    def do_qos_egress_qos_map_clear_bucket_state(self, arg):
        '''
         xpsQosEgressQosMapClearBucketState: Enter [ devId,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsQosEgressQosMapClearBucketState(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsQosInit
    #/********************************************************************************/
    def do_qos_init(self, arg):
        '''
         xpsQosInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsQosInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsQosDeInit
    #/********************************************************************************/
    def do_qos_de_init(self, arg):
        '''
         xpsQosDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsQosDeInit()
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
