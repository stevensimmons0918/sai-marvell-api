#!/usr/bin/env python
#  xpsMac.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

#include "xpTypes.h"
import sys
import time
import os
import re
import readline
import thread

#/**********************************************************************************/
# import cmd2 package
#/**********************************************************************************/
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd

import xpShellGlobals
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *

def threaded_function(*args):
    ret = xpsMacSerdesAacsServer(int(args[0]), int(args[1]), int(args[2]))
    if ret < 0:
        print 'serdes_aacs_server Failed with return value:', ret
    else:
        print 'done with return value:', ret

#/**********************************************************************************/
# The class object for xpsMac operations
#/**********************************************************************************/

class xpsMacObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsMacConfigModeGet
    #/********************************************************************************/
    def do_mac_mode_get(self, arg):
        '''
         xpsMacConfigModeGet: Enter [ devId,startPort-endPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        macModeStr = ""
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId/startPort-endPort ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            valid_Ptr_2 = new_xpMacConfigMode_p()
            #print('Input Arguments are, devId=%d, portNum/startPort-endPort=%d' % (args[0],args[1]))
            for portNum in range(startIdx,endIdx+1):
                ret = xpsMacConfigModeGet(args[0],portNum,valid_Ptr_2)
                err = 0
                if ret != 0:
                    err = 1
                if err == 0:
                    print('============================')
                    print('Port %d:' % (portNum))
                    print('============================')
                    if(MAC_MODE_4X1GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_4X1GB"
                    elif(MAC_MODE_MIX_4_CHANNEL_10G == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_MIX_4_CHANNEL_10G"
                    elif(MAC_MODE_1GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1GB"
                    elif(MAC_MODE_4X10GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_4X10GB"
                    elif(MAC_MODE_2X40GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_2X40GB"
                    elif(MAC_MODE_1X40GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1X40GB"
                    elif(MAC_MODE_1X100GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1X100GB"
                    elif(MAC_MODE_4X25GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_4X25GB"
                    elif(MAC_MODE_2X50GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_2X50GB"
                    elif(MAC_MODE_1X10GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1X10GB"
                    elif(MAC_MODE_1000BASE_R == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1000BASE_R"
                    elif(MAC_MODE_1000BASE_X == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1000BASE_X"
                    elif(MAC_MODE_MIX_SGMII == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_MIX_SGMII"
                    elif(MAC_MODE_MIX_1000BASE_R == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_MIX_1000BASE_R"
                    elif(MAC_MODE_1X50GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_1X50GB"
                    elif(MAC_MODE_4X200GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_4X200GB"
                    elif(MAC_MODE_8X400GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_8X400GB"
                    elif(MAC_MODE_2X100GB == xpMacConfigMode_p_value(valid_Ptr_2)):
                        macModeStr = "MAC_MODE_2X100GB"
                    else:
                        macModeStr = "INVALID_MODE"
                    print('mac mode= %s' % (macModeStr))
                    print('')
                    pass
            delete_xpMacConfigMode_p(valid_Ptr_2)



    #/********************************************************************************/
    # command for xpsMacSwitchMacConfigMode
    #/********************************************************************************/
    def do_mac_switch_mac_config_mode(self, arg):
        '''
         xpsMacSwitchMacConfigMode: Enter [ devId,macNum,newMode,fecMode,enableFEC ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,macNum,newMode,fecMode,enableFEC ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, macNum=%d, newMode=%d, fecMode=%d, enableFEC =%d ' % (args[0],args[1],args[2],args[3], args[4]))
            ret = xpsMacSwitchMacConfigMode(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortGroupInit
    #/********************************************************************************/
    def do_mac_port_group_init(self, arg):
        '''
        xpsPortGroupInit: Enter [ devId, macNum, speedMode, initSerdes, fecMode, enableFEC ]
        Valid ptgNum are: 0 to 31
        Valid speedMode are: 1x100G, 1x40G, 2x40G, 2x50G, 1x50G, 4x10G, 4x25G, MIXED_MODE_10G, SGMII, 1000BASE_R, MIXED_MODE_SGMII, MIXED_MODE_1000BASE_R
        Valid FEC mode: RS, FC
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, macNum, speedMode, initSerdes, fecMode, enableFEC ]')
            print('Valid ptgNum are: 0 to 31')
            print('Valid speedMode are: 1x100G, 1x40G, 2x40G, 2x50G, 1x50G, 4x10G, 4x25G, MIXED_MODE_10G, SGMII, 1000BASE_R, MIXED_MODE_SGMII, MIXED_MODE_1000BASE_R')
            print('Valid FECMode are: RS, FC')
        else:
            args[0] = int(args[0])
            ptgNum = int(args[1])
            sgmiiSpeed = 9
            if (args[2] == '1x100G') :
                macMode = eval('MAC_MODE_1X100GB')
            elif (args[2] == '1x40G') :
                macMode = eval('MAC_MODE_1X40GB')
            elif (args[2] == '1x50G') :
                macMode = eval('MAC_MODE_1X50GB')
            elif (args[2] == '2x40G') :
                macMode = eval('MAC_MODE_2X40GB')
            elif (args[2] == '2x50G') :
                macMode = eval('MAC_MODE_2X50GB')
            elif (args[2] == '4x10G') :
                macMode = eval('MAC_MODE_4X10GB')
            elif (args[2] == '4x25G') :
                macMode = eval('MAC_MODE_4X25GB')
            elif (args[2] == 'MIXED_MODE_10G') :
                macMode = eval('MAC_MODE_MIX_4_CHANNEL_10G')
            elif (args[2] == '4x1G') :
                macMode = eval('MAC_MODE_4X1GB')
            elif (args[2] == '1000BASE_R') :
                macMode = eval('MAC_MODE_1000BASE_R')
            elif (args[2] == 'MIXED_MODE_SGMII') :
                macMode = eval('MAC_MODE_MIX_SGMII')
                sgmiiSpeed = 2
            elif (args[2] == 'MIXED_MODE_1000BASE_R') :
                macMode = eval('MAC_MODE_MIX_1000BASE_R')
            else :
                print('Invalid speedMode provided')
                return
            initSerdes = int(args[3])
            enableFEC = int(args[5])
            keepPortDown = 0
            prbsTestMode = 0
            firmwareUpload = 0

            if enableFEC :
                if (args[4] == 'RS') :
                    fecMode = eval('RS_FEC_MODE')
                elif (args[4] == 'FC') :
                    fecMode = eval('FC_FEC_MODE')
                else :
                    print('Invalid FECMode provided')
                    return
            else:
                fecMode = eval('MAX_FEC_MODE')

            ret = xpsMacPortGroupInit(args[0],ptgNum,macMode,sgmiiSpeed,initSerdes,prbsTestMode,firmwareUpload, fecMode, enableFEC, keepPortDown)
            if ret != 0:
                print('Return Value = %d for ptgNum = %d' % (ret,ptgNum))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortGroupInitWithLinkStatusControl
    #/********************************************************************************/
    def do_mac_port_group_init_with_link_status_control(self, arg):
        '''
        xpsMacPortGroupInitWithLinkStatusControl: Enter [ devId, macNum, speedMode, initSerdes, fecMode, enableFEC, keepPortDown ]
        Valid ptgNum are: 0 to 31
        Valid speedMode are: 1x100G, 1x40G, 2x40G, 2x50G, 1x50G, 4x10G, 4x25G, MIXED_MODE_10G, SGMII, 1000BASE_R, MIXED_MODE_SGMII, MIXED_MODE_1000BASE_R
        Valid FEC mode: RS, FC, RS_544_514
        keepPortDown : when 1 it will keep the port status as down upon PTG init.
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, macNum, speedMode, initSerdes, fecMode, enableFEC, keepPortDown ]')
            print('Valid ptgNum are: 0 to 31')
            print('Valid speedMode are: 1x100G, 1x40G, 2x40G, 2x50G, 1x50G, 4x10G, 4x25G, MIXED_MODE_10G, SGMII, 1000BASE_R, MIXED_MODE_SGMII, MIXED_MODE_1000BASE_R')
            print('Valid FECMode are: RS, FC')
            print('keepPortDown : when 1 it will keep the port status as down upon PTG init.')
        else:
            args[0] = int(args[0])
            ptgNum = int(args[1])
            sgmiiSpeed = 9
            if (args[2] == '1x100G') :
                macMode = eval('MAC_MODE_1X100GB')
            elif (args[2] == '1x40G') :
                macMode = eval('MAC_MODE_1X40GB')
            elif (args[2] == '1x50G') :
                macMode = eval('MAC_MODE_1X50GB')
            elif (args[2] == '2x40G') :
                macMode = eval('MAC_MODE_2X40GB')
            elif (args[2] == '2x50G') :
                macMode = eval('MAC_MODE_2X50GB')
            elif (args[2] == '4x10G') :
                macMode = eval('MAC_MODE_4X10GB')
            elif (args[2] == '4x25G') :
                macMode = eval('MAC_MODE_4X25GB')
            elif (args[2] == 'MIXED_MODE_10G') :
                macMode = eval('MAC_MODE_MIX_4_CHANNEL_10G')
            elif (args[2] == '4x1G') :
                macMode = eval('MAC_MODE_4X1GB')
            elif (args[2] == '1000BASE_R') :
                macMode = eval('MAC_MODE_1000BASE_R')
            elif (args[2] == 'MIXED_MODE_SGMII') :
                macMode = eval('MAC_MODE_MIX_SGMII')
                sgmiiSpeed = 2
            elif (args[2] == 'MIXED_MODE_1000BASE_R') :
                macMode = eval('MAC_MODE_MIX_1000BASE_R')
            else :
                print('Invalid speedMode provided')
                return
            initSerdes = int(args[3])
            enableFEC = int(args[5])
            keepPortDown = int(args[6])
            prbsTestMode = 0
            firmwareUpload = 0

            if enableFEC :
                if (args[4] == 'RS') :
                    fecMode = eval('RS_FEC_MODE')
                elif (args[4] == 'FC') :
                    fecMode = eval('FC_FEC_MODE')
                elif (args[4] == 'RS_544_514'):
                    fecMode = eval('RS_544_514_FEC_MODE')
                else :
                    print('Invalid FECMode provided')
                    return
            else:
                fecMode = eval('MAX_FEC_MODE')

            ret = xpsMacPortGroupInitWithLinkStatusControl(args[0],ptgNum,macMode,sgmiiSpeed,initSerdes,prbsTestMode,firmwareUpload, fecMode, enableFEC, keepPortDown)
            if ret != 0:
                print('Return Value = %d for ptgNum = %d' % (ret,ptgNum))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortInitWithLinkStatusControl
    #/********************************************************************************/
    def do_mac_port_init_with_link_status_control(self, arg):
        '''
         xpsMacPortInitWithLinkStatusControl : Enter [ devId, portNum, speedMode, fecMode, enableFEC, initSerdes, keepPortDown]
            Valid portNum are: 0 to 127 and 176
            Valid speedMode are: 8x400G, 4x200G, 2x100G, 1x100G, 1x50G, 1x40G, 2x40G, 2x50G, 4x10G, 4x25G, 4x1G, 1x10G, 1G
            176 is the CPU eth interface port. Valid mac mode for CPU port is 1x10G and 1G
            port 176 doesn't support FEC
            Valid FECMode are: RS, FC, RS_544_514
            keepPortDown : when 1 it will keep the port status as down upon port init
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, portNum/startPortNum-endPortNum, speedMode, fecMode, enableFEC, initSerdes, keepPortDown]')
            print('Valid portNum are: 0 to 127 and 176')
            print('176 is the CPU eth interface port. Valid mac mode for this is 1x10G and 1G')
            print('Valid speedMode are: 8x200G, 4x200G, 2x100G, 1x100G, 1x50G, 1x40G, 2x40G, 2x50G, 4x10G, 4x25G, 4x1G, 1x10G, 1G')
            print('1x10G used for the dmac port 176. port 176 does not support FEC')
            print('Valid FECMode are: RS, FC, RS_544_514')
            print('keepPortDown : when 1 it will keep the port status as down upon port init')

        else:
            args[0] = int(args[0])
            portNum = int(args[1])

            if (args[2] == '1x100G') :
                macMode = eval('MAC_MODE_1X100GB')
            elif (args[2] == '4x200G') :
                macMode = eval('MAC_MODE_4X200GB')
            elif (args[2] == '8x400G') :
                macMode = eval('MAC_MODE_8X400GB')
            elif (args[2] == '2x100G') :
                macMode = eval('MAC_MODE_2X100GB')
            elif (args[2] == '1x40G') :
                macMode = eval('MAC_MODE_1X40GB')
            elif (args[2] == '2x40G') :
                macMode = eval('MAC_MODE_2X40GB')
            elif (args[2] == '2x50G') :
                macMode = eval('MAC_MODE_2X50GB')
            elif (args[2] == '4x10G') :
                macMode = eval('MAC_MODE_4X10GB')
            elif (args[2] == '4x25G') :
                macMode = eval('MAC_MODE_4X25GB')
            elif (args[2] == '1x10G') :
                macMode = eval('MAC_MODE_1X10GB')
            elif (args[2] == '4x1G') :
                macMode = eval('MAC_MODE_4X1GB')
            elif (args[2] == '1G') :
                macMode = eval('MAC_MODE_1GB')
            elif (args[2] == '1x50G'):
                macMode = eval('MAC_MODE_1X50GB')
            #elif (args[2] == '100M') :
                #macMode = eval('MAC_MODE_100M')
            #elif (args[2] == '10M') :
                #macMode = eval('MAC_MODE_10M')
            else :
                print('Invalid speedMode provided')
                return

            enableFEC = int(args[4])
            initSerdes = int(args[5])
            keepPortDown = int(args[6])
            prbsTestMode = 0
            firmwareUpload = 0

	    if enableFEC :
	        if (args[3] == 'RS') :
		    fecMode = eval('RS_FEC_MODE')
		elif (args[3] == 'FC') :
	            fecMode = eval('FC_FEC_MODE')
	        elif (args[3] == 'RS_544_514'):
                    fecMode = eval('RS_544_514_FEC_MODE')
                else :
                    print('Invalid FECMode provided')
		    return
	    else:
		fecMode = eval('MAX_FEC_MODE')

            ret = xpsMacPortInitWithLinkStatusControl(args[0],portNum,macMode,initSerdes,prbsTestMode,firmwareUpload,fecMode,enableFEC,keepPortDown)
            if ret != 0:
                print('Return Value = %d for portNum = %d' % (ret,portNum))
            else:
                pass

#   #/********************************************************************************/
#   # command for xpsMacMixedModePortSpeedChange
#   #/********************************************************************************/
#   def do_mac_mixed_mode_port_speed_change(self, arg):
#       '''
#        xpsMacMixedModePortSpeedChange: Enter [ devId,portNum,portMode ]
#       '''
#       args = re.split(';| ',arg)
#       numArgsReq = 3
#       if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
#           print('Invalid input, Enter [ devId,portNum,portMode ]')
#       else:
#           args[0] = int(args[0])
#           args[1] = int(args[1])
#           args[2] = int(args[2])
#           #print('Input Arguments are, devId=%d, portNum=%d, portMode=%d' % (args[0],args[1],args[2]))
#           ret = xpsMacMixedModePortSpeedChange(args[0],args[1],args[2])
#           if ret != 0:
#               print('Return Value = %d' % (ret))
#           else:
#               pass

    #/********************************************************************************/
    # command for xpsMacPortDeInit
    #/********************************************************************************/
    def do_mac_port_de_init(self, arg):
        '''
         xpsMacPortDeInit: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacPortDeInit(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortGroupReInit
    #/********************************************************************************/
    def do_mac_port_group_re_init(self, arg):
        '''
         xpsMacPortGroupReInit: Enter [ devId,macNum,portMode,macConfig,speed,enable100GFEC]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,macNum,portMode,macConfig,speed,enable100GFEC]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])

            #print('Input Arguments are, devId=%d, macNum=%d' % (args[0],args[1]))
            ret = xpsMacPortGroupDeInit(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                #print('Input Arguments are, devId=%d, macNum=%d, portMode=%d, macConfig=%d, speed=%d, serdesTestOn=%d, prbsTestMode=%d, firmwareUpload=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]))
                ret = xpsMacPortGroupInit(args[0],args[1],args[2],args[3],args[4],args[5])
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    pass

    #/********************************************************************************/
    # command for xpsMacPortGroupDeInit
    #/********************************************************************************/
    def do_mac_port_group_de_init(self, arg):
        '''
         xpsMacPortDeInit: Enter [ devId,macNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,macNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, macNum=%d' % (args[0],args[1]))
            ret = xpsMacPortGroupDeInit(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIsPortInited
    #/********************************************************************************/
    def do_is_mac_port_inited(self, arg):
        '''
         xpsIsPortInited: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsIsPortInited(args[0],args[1])
            err = 0
            if ret != 0:
                print('port is not inited')
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('port inited')
                pass


    #/********************************************************************************/
    # command for xpsMacIsPortNumValid
    #/********************************************************************************/
    def do_is_mac_port_num_valid(self, arg):
        '''
         xpsMacIsPortNumValid: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            valid_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacIsPortNumValid(args[0],args[1],valid_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('valid = %d' % (uint8_tp_value(valid_Ptr_2)))
                pass
            delete_uint8_tp(valid_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacEventHandler
    #/********************************************************************************/
    def do_mac_event_handler(self, arg):
        '''
         xpsMacEventHandler: Enter [ devId,macNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,macNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, macNum=%d' % (args[0],args[1]))
            ret = xpsMacEventHandler(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortLoopbackEnable
    #/********************************************************************************/
    def do_mac_port_loopback_enable(self, arg):
        '''
         xpsMacPortLoopbackEnable: Enter [devId, portNum, enable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [devId, portNum, enable]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsMacPortLoopbackEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortLoopbackEnableGet
    #/********************************************************************************/
    def do_mac_port_loopback_enable_get(self, arg):
        '''
         xpsMacPortLoopbackEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacPortLoopbackEnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)


    #/********************************************************************************/
    # Auto completion for serdesDfeTuneMode enumerations
    #/********************************************************************************/
    def complete_mac_port_serdes_tune(self, text, line, begidx, endidx):
        tempDict = { 3 : 'serdesDfeTuneMode'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsMacPortSerdesTune
    #/********************************************************************************/
    def do_mac_port_serdes_tune(self, arg):
        '''
         xpsMacPortSerdesTune: Enter [devId, portNum/startPort-endPort, tuneMode, force]
         Valid values for tuneMode : <serdesDfeTuneMode>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [devId, portNum/startPort-endPort, tuneMode, force]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]

            tuneMode = int(args[2]) # mode 0 is coarse, mode 1 is fine
            force = int(args[3])
            portList = range(startIdx,endIdx+1)
            arrUint32 = new_arrUint32(len(portList))
            for index, value in enumerate(portList):
                arrUint32_setitem(arrUint32, index, value)
            #print('Input Arguments are, devId=%d, portNum/startPort-endPort=%d, tuneMode=%d force=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacPortSerdesTune(args[0],arrUint32,len(portList),tuneMode,force)
            if ret != 0:
                print('Error returned = %d ' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpLinkManagerPortSerdesSignalOverride
    #/********************************************************************************/
    def do_mac_port_serdes_signal_override(self, arg):
        '''
         xpLinkManagerPortSerdesSignalOverride: Enter [devId, portNum/startPort-endPort, serdesSignalVal]
         serdesSignalVal could be :
         0 (serdes signal normal operation. no override)
         1 (Invert. Not available in A0)
         2 (force signal ok to 0 going in PCS)
         3 (force signal ok to 1 going in PCS)
         How it operate :
             - In case of 100G,40G all four serdes will override
             - In case of 50G, Two serdes per port will override
             - In case of 10G,25G, Single serdes per port will override
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [devId, portNum/startPort-endPort, serdesSignalVal]')
            print('serdesSignalVal could be :')
            print('0 (serdes signal normal operation. no override)')
            print('1 (Invert. Not available in A0)')
            print('2 (force signal ok to 0 going in PCS)')
            print('3 (force signal ok to 1 going in PCS)')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]

            serdesSignalVal = int(args[2])

            #print('Input Arguments are, devId=%d, portNum/startPort-endPort=%d, tuneMode=%d force=%d' % (args[0],args[1],args[2],args[3]))
            valid_Ptr = new_uint8_tp()
            for portNum in range(startIdx,endIdx+1):
                xpsMacIsPortNumValid(args[0],portNum,valid_Ptr)
                if((uint8_tp_value(valid_Ptr))):
                    if(xpsIsPortInited(args[0],portNum) == 0):
                        ret = xpsMacPortSerdesSignalOverride(args[0],portNum,serdesSignalVal)
                        if ret != 0:
                            print('Error returned = %d for portNum = %d' % (ret,portNum))
                    else:
                        print('============================')
                        print('Port %d:' % (portNum))
                        print('============================')
                        print('portNum %d is not initialized' % (portNum))
                        print('')
                else:
                    pass

    #/********************************************************************************/
    # command for xpsMacPortFarEndLoopbackEnable
    #/********************************************************************************/
    def do_mac_port_far_end_loopback_enable(self, arg):
        '''
         xpsMacPortFarEndLoopbackEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPortFarEndLoopbackEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacRxCrcCheckEnable
    #/********************************************************************************/
    def do_mac_rx_crc_check_enable(self, arg):
        '''
         xpsMacRxCrcCheckEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRxCrcCheckEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacRxStripFcsEnable
    #/********************************************************************************/
    def do_mac_rx_strip_fcs_enable(self, arg):
        '''
         xpsMacRxStripFcsEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRxStripFcsEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacRxFlowControlDecodeEnable
    #/********************************************************************************/
    def do_mac_rx_flow_control_decode_enable(self, arg):
        '''
         xpsMacRxFlowControlDecodeEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRxFlowControlDecodeEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetRxPreambleLen
    #/********************************************************************************/
    def do_mac_set_rx_preamble_len(self, arg):
        '''
         xpsMacSetRxPreambleLen: Enter [ devId,portNum,bytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,bytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            if(args[2] == 8):
                args[2] = eval('PREAMBLE_RX_LEN_BYTES_8')
            elif(args[2] == 4):
                args[2] = eval('PREAMBLE_RX_LEN_BYTES_4')
            #print('Input Arguments are, devId=%d, portNum=%d, bytes=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetRxPreambleLen(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetRxJabberFrmSize
    #/********************************************************************************/
    def do_mac_set_rx_jabber_frm_size(self, arg):
        '''
         xpsMacSetRxJabberFrmSize: Enter [ devId,portNum,jabberFrmSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,jabberFrmSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, jabberFrmSize=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetRxJabberFrmSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetTxPriorityPauseVector
    #/********************************************************************************/
    def do_mac_set_tx_priority_pause_vector(self, arg):
        '''
         xpsMacSetTxPriorityPauseVector: Enter [ devId,portNum,txPriPauseVector ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,txPriPauseVector ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, txPriPauseVector=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxPriorityPauseVector(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetVlanTag3
    #/********************************************************************************/
    def do_mac_set_vlan_tag3(self, arg):
        '''
         xpsMacSetVlanTag3: Enter [ devId,portNum,vlanTag ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,vlanTag ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, vlanTag=%d' % (args[0],args[1],args[2]))
            #ret = xpsMacSetVlanTag3(args[0],args[1],args[2])
            #if ret != 0:
            #    print('Return Value = %d' % (ret))
            #else:
            #    pass

    #/********************************************************************************/
    # command for xpsMacSetVlanTag1
    #/********************************************************************************/
    def do_mac_set_vlan_tag1(self, arg):
        '''
         xpsMacSetVlanTag1: Enter [ devId,portNum,vlanTag ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,vlanTag ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, vlanTag=%d' % (args[0],args[1],args[2]))
            #ret = xpsMacSetVlanTag1(args[0],args[1],args[2])
            #if ret != 0:
            #    print('Return Value = %d' % (ret))
            #else:
            #    pass

    #/********************************************************************************/
    # command for xpsMacSetVlanTag2
    #/********************************************************************************/
    def do_mac_set_vlan_tag2(self, arg):
        '''
         xpsMacSetVlanTag2: Enter [ devId,portNum,vlanTag ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,vlanTag ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, vlanTag=%d' % (args[0],args[1],args[2]))
            #ret = xpsMacSetVlanTag2(args[0],args[1],args[2])
            #if ret != 0:
            #    print('Return Value = %d' % (ret))
            #else:
            #    pass

    #/********************************************************************************/
    # command for xpsMacGetPauseFrameGen
    #/********************************************************************************/
    def do_mac_get_pause_frame_gen(self, arg):
        '''
         xpsMacGetPauseFrameGen: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPauseFrameGen(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetPauseFrameGen
    #/********************************************************************************/
    def do_mac_set_pause_frame_gen(self, arg):
        '''
         xpsMacSetPauseFrameGen: Enter [ devId,portNum,status ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,status ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, status=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetPauseFrameGen(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetMacFlowCtrlFrmDestAddr
    #/********************************************************************************/
    def do_mac_set_mac_flow_ctrl_frm_dest_addr(self, arg):
        '''
         xpsMacSetMacFlowCtrlFrmDestAddr: Enter [ devId,portNum,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,macAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macAddrList)
            macAddr = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                macAddr[listLen - ix - 1] = int(macAddrList[ix], 16)
                #print('macAddrList[%d] = %02x' % (ix, macAddr[listLen - ix - 1]))
            #print('Input Arguments are, devId=%d, portNum=%d, macAddr=%s' % (args[0],args[1],args[2]))
            ret = xpsMacSetMacFlowCtrlFrmDestAddr(args[0],args[1],macAddr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsMacSetMacFlowCtrlFrmSrcAddr
    #/********************************************************************************/
    def do_mac_set_mac_flow_ctrl_frm_src_addr(self, arg):
        '''
         xpsMacSetMacFlowCtrlFrmSrcAddr: Enter [ devId,portNum,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,macAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macAddrList)
            macAddr = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                macAddr[listLen - ix - 1] = int(macAddrList[ix], 16)
                #print('macAddrList[%d] = %02x' % (ix, macAddr[listLen - ix - 1]))
            #print('Input Arguments are, devId=%d, portNum=%d, macAddr=%s' % (args[0],args[1],args[2]))
            ret = xpsMacSetMacFlowCtrlFrmSrcAddr(args[0],args[1],macAddr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetXonPauseTime
    #/********************************************************************************/
    def do_mac_set_xon_pause_time(self, arg):
        '''
         xpsMacSetXonPauseTime: Enter [ devId,portNum,pauseTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,pauseTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, pauseTime=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetXonPauseTime(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetXoffPauseTime
    #/********************************************************************************/
    def do_mac_set_xoff_pause_time(self, arg):
        '''
         xpsMacSetXoffPauseTime: Enter [ devId,portNum,pauseTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,pauseTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, pauseTime=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetXoffPauseTime(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxTimeStampValue
    #/********************************************************************************/
    def do_mac_get_tx_time_stamp_value(self, arg):
        '''
         xpsMacGetTxTimeStampValue: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            timeStampVal_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxTimeStampValue(args[0],args[1],timeStampVal_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('timeStampVal = %d' % (uint64_tp_value(timeStampVal_Ptr_2)))
                pass
            delete_uint64_tp(timeStampVal_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTimeStampValidStatus
    #/********************************************************************************/
    def do_mac_get_time_stamp_valid_status(self, arg):
        '''
         xpsMacGetTimeStampValidStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            valid_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTimeStampValidStatus(args[0],args[1],valid_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('valid = %d' % (uint8_tp_value(valid_Ptr_2)))
                pass
            delete_uint8_tp(valid_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTimeStampId
    #/********************************************************************************/
    def do_mac_get_time_stamp_id(self, arg):
        '''
         xpsMacGetTimeStampId: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            timeStampId_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTimeStampId(args[0],args[1],timeStampId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('timeStampId = %d' % (uint8_tp_value(timeStampId_Ptr_2)))
                pass
            delete_uint8_tp(timeStampId_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFlowCtrlPauseTime
    #/********************************************************************************/
    def do_mac_set_flow_ctrl_pause_time(self, arg):
        '''
         xpsMacSetFlowCtrlPauseTime: Enter [ devId,portNum,pauseTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,pauseTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, pauseTime=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetFlowCtrlPauseTime(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetRxMaxFrmLen
    #/********************************************************************************/
    def do_mac_set_rx_max_frm_len(self, arg):
        '''
         xpsMacSetRxMaxFrmLen: Enter [ devId,portNum,frameSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,frameSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, frameSize=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetRxMaxFrmLen(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetTxIfgLen
    #/********************************************************************************/
    def do_mac_set_tx_ifg_len(self, arg):
        '''
         xpsMacSetTxIfgLen: Enter [ devId,portNum,ifgLength ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,ifgLength ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, ifgLength=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxIfgLen(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetTxJabberFrmSize
    #/********************************************************************************/
    def do_mac_set_tx_jabber_frm_size(self, arg):
        '''
         xpsMacSetTxJabberFrmSize: Enter [ devId,portNum,jabberFrmSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,jabberFrmSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, jabberFrmSize=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxJabberFrmSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxIfgCtrlPerFrameEnable
    #/********************************************************************************/
    def do_mac_tx_ifg_ctrl_per_frame_enable(self, arg):
        '''
         xpsMacTxIfgCtrlPerFrameEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacTxIfgCtrlPerFrameEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxCrcCheckEnable
    #/********************************************************************************/
    def do_mac_tx_crc_check_enable(self, arg):
        '''
         xpsMacTxCrcCheckEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacTxCrcCheckEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxFcsInvertEnable
    #/********************************************************************************/
    def do_mac_tx_fcs_invert_enable(self, arg):
        '''
         xpsMacTxFcsInvertEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacTxFcsInvertEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxFlowControlEnable
    #/********************************************************************************/
    def do_mac_tx_flow_control_enable(self, arg):
        '''
         xpsMacTxFlowControlEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacTxFlowControlEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxPriorityFlowControlEnable
    #/********************************************************************************/
    def do_mac_tx_priority_flow_control_enable(self, arg):
        '''
         xpsMacTxPriorityFlowControlEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacTxPriorityFlowControlEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetTxPreambleLen
    #/********************************************************************************/
    def do_mac_set_tx_preamble_len(self, arg):
        '''
         xpsMacSetTxPreambleLen: Enter [ devId,portNum,bytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,bytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            if(args[2] == 8):
                args[2] = eval('PREAMBLE_LEN_BYTES_8')
            #print('Input Arguments are, devId=%d, portNum=%d, bytes=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxPreambleLen(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortSoftReset
    #/********************************************************************************/
    def do_mac_port_soft_reset(self, arg):
        '''
         xpsMacPortSoftReset: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPortSoftReset(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacRxtxPortEnable
    #/********************************************************************************/
    def do_mac_rxtx_port_enable(self, arg):
        '''
         xpsMacRxtxPortEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRxtxPortEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxPortEnable
    #/********************************************************************************/
    def do_mac_tx_port_enable(self, arg):
        '''
         xpsMacTxPortEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacTxPortEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacRxPortEnable
    #/********************************************************************************/
    def do_mac_rx_port_enable(self, arg):
        '''
         xpsMacRxPortEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRxPortEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacStatCounterReset
    #/********************************************************************************/
    def do_mac_stat_counter_reset(self, arg):
        '''
         xpsMacStatCounterReset: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacStatCounterReset(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortEnable
    #/********************************************************************************/
    def do_mac_port_enable(self, arg):
        '''
         xpsMacPortEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPortEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortEnableRange
    #/********************************************************************************/
    def do_mac_port_enable_range(self, arg):
        '''
         xpsMacPortEnableRange: Enter [ devId,fromPortNum,toPortNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,fromPortNum,toPortNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, fromPortNum=%d, toPortNum=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacPortEnableRange(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortReset
    #/********************************************************************************/
    def do_mac_port_reset(self, arg):
        '''
         xpsMacPortReset: Enter [ devId,enable,portNum,reset ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,portNum,reset ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, enable=%d, portNum=%d, reset=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacPortReset(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacMacLoopback
    #/********************************************************************************/
    def do_mac_mac_loopback(self, arg):
        '''
         xpsMacMacLoopback: Enter [ devId,portNum,loopback,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,loopback,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, loopback=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacMacLoopback(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacMacLoopbackGet
    #/********************************************************************************/
    def do_mac_mac_loopback_get(self, arg):
        '''
         xpsMacMacLoopbackGet: Enter [ devId,portNum,loopback ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,loopback ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, loopback=%d ' % (args[0],args[1],args[2]))
            ret = xpsMacMacLoopbackGet(args[0],args[1],args[2],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGmiiLoopbackEnableGet
    #/********************************************************************************/
    def do_mac_gmii_loopback_enable_get(self, arg):
        '''
         xpsMacGmiiLoopbackEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGmiiLoopbackEnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacConfigMdioMaster
    #/********************************************************************************/
    def do_mac_config_mdio_master(self, arg):
        '''
         xpsMacConfigMdioMaster: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacConfigMdioMaster(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacDeconfigMdioMaster
    #/********************************************************************************/
    def do_mac_deconfig_mdio_master(self, arg):
        '''
         xpsMacDeconfigMdioMaster: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacDeconfigMdioMaster(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacExtPHYRead
    #/********************************************************************************/
    def do_mac_ext_phy_read(self, arg):
        '''
         xpsMacExtPHYRead: Enter [ devId,portNum,phyAddr,phyRegAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,phyAddr,phyRegAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            value_Ptr_4 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, phyAddr=0x%x, phyRegAddr=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacExtPHYRead(args[0],args[1],args[2],args[3],value_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %s' % (hex(uint16_tp_value(value_Ptr_4)).rstrip("L")))
                pass
            delete_uint16_tp(value_Ptr_4)

    #/********************************************************************************/
    # command for xpsMacExtPHYWrite
    #/********************************************************************************/
    def do_mac_ext_phy_write(self, arg):
        '''
         xpsMacExtPHYWrite: Enter [ devId,portNum,phyAddr,phyRegAddr,value ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,phyAddr,phyRegAddr,value ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            args[4] = int(args[4], 16)
            #print('Input Arguments are, devId=%d, portNum=%d, phyAddr=0x%x, phyRegAddr=0x%x, value=0x%x' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsMacExtPHYWrite(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetPortSpeed
    #/********************************************************************************/
    def do_mac_get_port_speed(self, arg):
        '''
         xpsMacGetPortSpeed: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            speed_Ptr_2 = new_xpSpeedp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPortSpeed(args[0],args[1],speed_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('speed = %d' % (xpSpeedp_value(speed_Ptr_2)))
                pass
            delete_xpSpeedp(speed_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetPortAutoNeg
    #/********************************************************************************/
    def do_mac_set_port_auto_neg(self, arg):
        '''
         xpsMacSetPortAutoNeg: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetPortAutoNeg(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortAutoNegRestart
    #/********************************************************************************/
    def do_mac_port_auto_neg_restart(self, arg):
        '''
         xpsMacPortAutoNegRestart: Enter [ devId,portNum,restart ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,restart ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, restart=%d' % (args[0],args[1],args[2]))
            ret = xpsMacPortAutoNegRestart(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetPortAutoNegEnable
    #/********************************************************************************/
    def do_mac_get_port_auto_neg_enable(self, arg):
        '''
         xpsMacGetPortAutoNegEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPortAutoNegEnable(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPortAutoNegDone
    #/********************************************************************************/
    def do_mac_get_port_auto_neg_done(self, arg):
        '''
         xpsMacGetPortAutoNegDone: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPortAutoNegDone(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPortAutoNegSyncStatus
    #/********************************************************************************/
    def do_mac_get_port_auto_neg_sync_status(self, arg):
        '''
         xpsMacGetPortAutoNegSyncStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPortAutoNegSyncStatus(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacIsMdioMasterConf
    #/********************************************************************************/
    def do_mac_is_mdio_master_conf(self, arg):
        '''
         xpsMacIsMdioMasterConf: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacIsMdioMasterConf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetMdioClkDivisonCtrl
    #/********************************************************************************/
    def do_mac_set_mdio_clk_divison_ctrl(self, arg):
        '''
         xpsMacSetMdioClkDivisonCtrl: Enter [ devId,portNum,divCtrlRatio ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,divCtrlRatio ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, divCtrlRatio=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetMdioClkDivisonCtrl(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacEventHandlerDeRegister
    #/********************************************************************************/
    def do_mac_event_handler_de_register(self, arg):
        '''
         xpsMacEventHandlerDeRegister: Enter [ devId,portNum,eventType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,eventType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, eventType=%d' % (args[0],args[1],args[2]))
            ret = xpsMacEventHandlerDeRegister(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacCounterStatsTxPkts
    #/********************************************************************************/
    def do_mac_counter_stats_tx_pkts(self, arg):
        '''
         xpsMacCounterStatsTxPkts: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            txTotalPkts_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacCounterStatsTxPkts(args[0],args[1],txTotalPkts_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('txTotalPkts = %d' % (uint64_tp_value(txTotalPkts_Ptr_2)))
                pass
            delete_uint64_tp(txTotalPkts_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacCounterStatsRxOctets
    #/********************************************************************************/
    def do_mac_counter_stats_rx_octets(self, arg):
        '''
         xpsMacCounterStatsRxOctets: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rxTotalOctets_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacCounterStatsRxOctets(args[0],args[1],rxTotalOctets_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rxTotalOctets = %d' % (uint64_tp_value(rxTotalOctets_Ptr_2)))
                pass
            delete_uint64_tp(rxTotalOctets_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacCounterStatsTxOctets
    #/********************************************************************************/
    def do_mac_counter_stats_tx_octets(self, arg):
        '''
         xpsMacCounterStatsTxOctets: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            txTotalOctets_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacCounterStatsTxOctets(args[0],args[1],txTotalOctets_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('txTotalOctets = %d' % (uint64_tp_value(txTotalOctets_Ptr_2)))
                pass
            delete_uint64_tp(txTotalOctets_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacEventHandlerRegister
    #/********************************************************************************/
    def do_mac_event_handler_register(self, arg):
        '''
         xpsMacEventHandlerRegister: Enter [ devId,portNum,eventType,eventHandler ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,eventType,eventHandler ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, eventType=%d, eventHandler=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacEventHandlerRegister(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacCounterStatsRxPkts
    #/********************************************************************************/
    def do_mac_counter_stats_rx_pkts(self, arg):
        '''
        xpsMacCounterStatsRxPkts: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rxTotalPkts_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacCounterStatsRxPkts(args[0],args[1],rxTotalPkts_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rxTotalPkts = %d' % (uint64_tp_value(rxTotalPkts_Ptr_2)))
                pass
            delete_uint64_tp(rxTotalPkts_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetCounterStats
    #/********************************************************************************/
    def do_mac_get_counter_stats(self, arg):
        '''
         xpsMacGetCounterStats: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xp_Statistics_Ptr = new_xp_Statisticsp()
            ret = xpsMacGetCounterStats(args[0],args[1],0,89,xp_Statistics_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                xpsMacPrintPortStat(args[0],args[1],xp_Statistics_Ptr)
                pass
            delete_xp_Statisticsp(xp_Statistics_Ptr)


    #/********************************************************************************/
    # command for xpsMacGetCounterStatsDirect
    #/********************************************************************************/
    def do_mac_get_counter_stats_direct(self, arg):
        '''
         xpsMacGetCounterStatsDirect: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xp_Statistics_Ptr = new_xp_Statisticsp()
            ret = xpsMacGetCounterStatsDirect(args[0],args[1],0,31,xp_Statistics_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print("\nPort Number:: %d" % (args[1]))
                print("--------------------------------------------------------------------------")
                print('RxOk             0x%-16x      Rx65-127      0x%-16x' % (xp_Statistics_Ptr.frameRxOk,xp_Statistics_Ptr.frameRxLength65To127))
                print('RxAll            0x%-16x      Rx128-255     0x%-16x' % (xp_Statistics_Ptr.frameRxAll, xp_Statistics_Ptr.frameRxLength128To255))
                print('RxFcsErr         0x%-16x      Rx256-511     0x%-16x' % (xp_Statistics_Ptr.frameRxFcsErr,xp_Statistics_Ptr.frameRxLength256To511))
                print('RxAnyErr         0x%-16x      Rx512-1023    0x%-16x' % (xp_Statistics_Ptr.frameRxAnyErr,xp_Statistics_Ptr.frameRxLength1024To1518))
                print('RxoctetsGood     0x%-16x      Rx1024-1518   0x%-16x' % (xp_Statistics_Ptr.octetsRxGoodFrame,xp_Statistics_Ptr.frameRxLength1024To1518))
                print('Rxoctets         0x%-16x      Rx1519-Up     0x%-16x' % (xp_Statistics_Ptr.octetsRx,xp_Statistics_Ptr.frameRxLength1519AndUp))
                print('RxUnicastAddr    0x%-16x      RxFIFOFull    0x%-16x' % (xp_Statistics_Ptr.frameRxUnicastAddr,xp_Statistics_Ptr.frameDroppedFromRxFIFOFullCondition))
                print('RxMulticastAddr  0x%-16x      TxOk          0x%-16x' % (xp_Statistics_Ptr.frameRxMulticastAddr,xp_Statistics_Ptr.frameTransmittedOk))
                print('RxBroadcastAddr  0x%-16x      TxAll         0x%-16x' % (xp_Statistics_Ptr.frameRxBroadcastAddr,xp_Statistics_Ptr.frameTransmittedAll))
                print('RxTypePause      0x%-16x      TxWithErr     0x%-16x' % (xp_Statistics_Ptr.frameRxTypePause,xp_Statistics_Ptr.frameTransmittedWithErr))
                print('RxLengthErr      0x%-16x      TxTotal       0x%-16x' % (xp_Statistics_Ptr.frameRxLengthErr,xp_Statistics_Ptr.octetsTransmittedTotal))
                print('RxUndersize      0x%-16x      TxUC          0x%-16x' % (xp_Statistics_Ptr.frameRxUndersize,xp_Statistics_Ptr.framesTransmittedUnicast))
                print('RxOversize       0x%-16x      TxMC          0x%-16x' % (xp_Statistics_Ptr.frameRxOversize,xp_Statistics_Ptr.framesTransmittedMulticast))
                print('RxFragments      0x%-16x      TxBC          0x%-16x' % (xp_Statistics_Ptr.fragmentsRx,xp_Statistics_Ptr.framesTransmittedBroadcast0))
                print('RxJabFrag        0x%-16x      TxPause       0x%-16x' % (xp_Statistics_Ptr.jabberFrameRx,xp_Statistics_Ptr.framesTransmittedPause))
                print('RxLen            0x%-16x      TxWithoutErr  0x%-16x' % (xp_Statistics_Ptr.frameRxLength64,xp_Statistics_Ptr.octetsTransmittedWithoutErr))
                pass
            delete_xp_Statisticsp(xp_Statistics_Ptr)



    #/********************************************************************************/
    # command for xpsMacGetPcsFaultStatus
    #/********************************************************************************/
    def do_mac_get_pcs_fault_status(self, arg):
        '''
         xpsMacGetPcsFaultStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsFaultStatus(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacPcsDoubleSpeedEnable
    #/********************************************************************************/
    def do_mac_pcs_double_speed_enable(self, arg):
        '''
         xpsMacPcsDoubleSpeedEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsDoubleSpeedEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacEncoding10gEnable
    #/********************************************************************************/
    def do_mac_encoding10g_enable(self, arg):
        '''
         xpsMacEncoding10gEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacEncoding10gEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsInterruptMaskEnable
    #/********************************************************************************/
    def do_mac_pcs_interrupt_mask_enable(self, arg):
        '''
         xpsMacPcsInterruptMaskEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsInterruptMaskEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetPcsInterruptMaskEnable
    #/********************************************************************************/
    def do_mac_get_pcs_interrupt_mask_enable(self, arg):
        '''
         xpsMacGetPcsInterruptMaskEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsInterruptMaskEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacPcsInterruptTestEnable
    #/********************************************************************************/
    def do_mac_pcs_interrupt_test_enable(self, arg):
        '''
         xpsMacPcsInterruptTestEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsInterruptTestEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetPcsInterruptTestEnable
    #/********************************************************************************/
    def do_mac_get_pcs_interrupt_test_enable(self, arg):
        '''
         xpsMacGetPcsInterruptTestEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsInterruptTestEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)


    #/********************************************************************************/
    # command for xpsMacSetPcsTestPatternSeedA
    #/********************************************************************************/
    def do_mac_set_pcs_test_pattern_seed_a(self, arg):
        '''
         xpsMacSetPcsTestPatternSeedA: Enter [ devId,portNum,seed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, portNum=%d, seed=0x%x' % (args[0],args[1],args[2]))
            ret = xpsMacSetPcsTestPatternSeedA(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetPcsTestPatternSeedB
    #/********************************************************************************/
    def do_mac_set_pcs_test_pattern_seed_b(self, arg):
        '''
         xpsMacSetPcsTestPatternSeedB: Enter [ devId,portNum,seed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, portNum=%d, seed=0x%x' % (args[0],args[1],args[2]))
            ret = xpsMacSetPcsTestPatternSeedB(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsDataPatternSelect
    #/********************************************************************************/
    def do_mac_pcs_data_pattern_select(self, arg):
        '''
         xpsMacPcsDataPatternSelect: Enter [ devId,portNum,dataPattern ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,dataPattern ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, portNum=%d, dataPattern=0x%x' % (args[0],args[1],args[2]))
            ret = xpsMacPcsDataPatternSelect(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsTestPatternSelect
    #/********************************************************************************/
    def do_mac_pcs_test_pattern_select(self, arg):
        '''
         xpsMacPcsTestPatternSelect: Enter [ devId,portNum,testPattern ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,testPattern ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, testPattern=%d' % (args[0],args[1],args[2]))
            ret = xpsMacPcsTestPatternSelect(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetPcsRxTxTestModeEnable
    #/********************************************************************************/
    def do_mac_set_pcs_rx_tx_test_mode_enable(self, arg):
        '''
         xpsMacSetPcsRxTxTestModeEnable: Enter [ devId,portNum,testMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,testMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, testMode=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetPcsRxTxTestModeEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetPcsScrambledIdealTestPattern
    #/********************************************************************************/
    def do_mac_set_pcs_scrambled_ideal_test_pattern(self, arg):
        '''
         xpsMacSetPcsScrambledIdealTestPattern: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetPcsScrambledIdealTestPattern(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetPcsLockStatus
    #/********************************************************************************/
    def do_mac_get_pcs_lock_status(self, arg):
        '''
         xpsMacGetPcsLockStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pcsLock_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsLockStatus(args[0],args[1],pcsLock_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pcsLock = %d' % (uint8_tp_value(pcsLock_Ptr_2)))
                pass
            delete_uint8_tp(pcsLock_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsHiBerStatus
    #/********************************************************************************/
    def do_mac_get_pcs_hi_ber_status(self, arg):
        '''
         xpsMacGetPcsHiBerStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pcsHiBer_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsHiBerStatus(args[0],args[1],pcsHiBer_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pcsHiBer = %d' % (uint8_tp_value(pcsHiBer_Ptr_2)))
                pass
            delete_uint8_tp(pcsHiBer_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsBlockLockStatus
    #/********************************************************************************/
    def do_mac_get_pcs_block_lock_status(self, arg):
        '''
         xpsMacGetPcsBlockLockStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            blockLock_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsBlockLockStatus(args[0],args[1],blockLock_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('blockLock = %d' % (uint32_tp_value(blockLock_Ptr_2)))
                pass
            delete_uint32_tp(blockLock_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsAlignmentLock
    #/********************************************************************************/
    def do_mac_get_pcs_alignment_lock(self, arg):
        '''
         xpsMacGetPcsAlignmentLock: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            alignmentLock_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsAlignmentLock(args[0],args[1],status_Ptr_2,alignmentLock_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                print('alignmentLock = %d' % (uint32_tp_value(alignmentLock_Ptr_3)))
                pass
            delete_uint32_tp(alignmentLock_Ptr_3)
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsDebugDeskewOverFlow
    #/********************************************************************************/
    def do_mac_get_pcs_debug_deskew_over_flow(self, arg):
        '''
         xpsMacGetPcsDebugDeskewOverFlow: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            deskewOverFlowErr_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsDebugDeskewOverFlow(args[0],args[1],deskewOverFlowErr_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('deskewOverFlowErr = %d' % (uint32_tp_value(deskewOverFlowErr_Ptr_2)))
                pass
            delete_uint32_tp(deskewOverFlowErr_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsDebugTxGearboxFifoErr
    #/********************************************************************************/
    def do_mac_get_pcs_debug_tx_gearbox_fifo_err(self, arg):
        '''
         xpsMacGetPcsDebugTxGearboxFifoErr: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fifoErr_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsDebugTxGearboxFifoErr(args[0],args[1],fifoErr_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fifoErr = %d' % (uint8_tp_value(fifoErr_Ptr_2)))
                pass
            delete_uint8_tp(fifoErr_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxPreambleLen
    #/********************************************************************************/
    def do_mac_get_rx_preamble_len(self, arg):
        '''
         xpsMacGetRxPreambleLen: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            length_Ptr_2 = new_xpRxPreambleLenBytesp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxPreambleLen(args[0],args[1],length_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                retval = xpRxPreambleLenBytesp_value(length_Ptr_2)
                if(retval == eval('PREAMBLE_RX_LEN_BYTES_8')):
                    retval = 8
                if(retval == eval('PREAMBLE_RX_LEN_BYTES_4')):
                    retval = 4
                print('bytes = %d' % (retval))
                pass
            delete_xpRxPreambleLenBytesp(length_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsFullThreshold
    #/********************************************************************************/
    def do_mac_get_pcs_full_threshold(self, arg):
        '''
         xpsMacGetPcsFullThreshold: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            maxEntries_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsFullThreshold(args[0],args[1],maxEntries_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('maxEntries = %d' % (uint8_tp_value(maxEntries_Ptr_2)))
                pass
            delete_uint8_tp(maxEntries_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsTxTestModeEnable
    #/********************************************************************************/
    def do_mac_get_pcs_tx_test_mode_enable(self, arg):
        '''
         xpsMacGetPcsTxTestModeEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            testMode_Ptr_2 = new_xpPCSEnableTestModep()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsTxTestModeEnable(args[0],args[1],testMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('testMode = %d' % (xpPCSEnableTestModep_value(testMode_Ptr_2)))
                pass
            delete_xpPCSEnableTestModep(testMode_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsRxTestModeEnable
    #/********************************************************************************/
    def do_mac_get_pcs_rx_test_mode_enable(self, arg):
        '''
         xpsMacGetPcsRxTestModeEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            testMode_Ptr_2 = new_xpPCSEnableTestModep()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsRxTestModeEnable(args[0],args[1],testMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('testMode = %d' % (xpPCSEnableTestModep_value(testMode_Ptr_2)))
                pass
            delete_xpPCSEnableTestModep(testMode_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsTestPatternSeedA
    #/********************************************************************************/
    def do_mac_get_pcs_test_pattern_seed_a(self, arg):
        '''
         xpsMacGetPcsTestPatternSeedA: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            seed_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsTestPatternSeedA(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint64_tp_value(seed_Ptr_2)))
                pass
            delete_uint64_tp(seed_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsTestPatternSeedB
    #/********************************************************************************/
    def do_mac_get_pcs_test_pattern_seed_b(self, arg):
        '''
         xpsMacGetPcsTestPatternSeedB: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            seed_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsTestPatternSeedB(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint64_tp_value(seed_Ptr_2)))
                pass
            delete_uint64_tp(seed_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetEncoding10gEnable
    #/********************************************************************************/
    def do_mac_get_encoding10g_enable(self, arg):
        '''
         xpsMacGetEncoding10gEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetEncoding10gEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsLaneMappingStatus
    #/********************************************************************************/
    def do_mac_get_pcs_lane_mapping_status(self, arg):
        '''
         xpsMacGetPcsLaneMappingStatus: Enter [ devId,portNum,laneNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,laneNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            laneMapping_Ptr_3 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, laneNum=%d' % (args[0],args[1],args[2]))
            ret = xpsMacGetPcsLaneMappingStatus(args[0],args[1],args[2],laneMapping_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('laneMapping = %d' % (uint8_tp_value(laneMapping_Ptr_3)))
                pass
            delete_uint8_tp(laneMapping_Ptr_3)

    #/********************************************************************************/
    # command for xpsMacPcsEnable
    #/********************************************************************************/
    def do_mac_pcs_enable(self, arg):
        '''
         xpsMacPcsEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsBypassScramblerEnable
    #/********************************************************************************/
    def do_mac_pcs_bypass_scrambler_enable(self, arg):
        '''
         xpsMacPcsBypassScramblerEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsBypassScramblerEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsUseShortTimerEnable
    #/********************************************************************************/
    def do_mac_pcs_use_short_timer_enable(self, arg):
        '''
         xpsMacPcsUseShortTimerEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsUseShortTimerEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsIgnoreSignalOkEnable
    #/********************************************************************************/
    def do_mac_pcs_ignore_signal_ok_enable(self, arg):
        '''
         xpsMacPcsIgnoreSignalOkEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsIgnoreSignalOkEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsBypassTxBufEnable
    #/********************************************************************************/
    def do_mac_pcs_bypass_tx_buf_enable(self, arg):
        '''
         xpsMacPcsBypassTxBufEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsBypassTxBufEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetPcsFullThreshold
    #/********************************************************************************/
    def do_mac_set_pcs_full_threshold(self, arg):
        '''
         xpsMacSetPcsFullThreshold: Enter [ devId,portNum,maxEntries ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,maxEntries ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, maxEntries=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetPcsFullThreshold(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsLowLatencyModeEnable
    #/********************************************************************************/
    def do_mac_pcs_low_latency_mode_enable(self, arg):
        '''
         xpsMacPcsLowLatencyModeEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacPcsLowLatencyModeEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetPcsLowLatencyModeEnable
    #/********************************************************************************/
    def do_mac_get_pcs_low_latency_mode_enable(self, arg):
        '''
         xpsMacGetPcsLowLatencyModeEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsLowLatencyModeEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacFecEnable
    #/********************************************************************************/
    def do_mac_fec_enable(self, arg):
        '''
         xpsMacFecEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacFecEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFec10gAbility
    #/********************************************************************************/
    def do_mac_get_fec10g_ability(self, arg):
        '''
         xpsMacGetFec10gAbility: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecAbility(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec10gErrIndicationAbility
    #/********************************************************************************/
    def do_mac_get_fec10g_err_indication_ability(self, arg):
        '''
         xpsMacGetFec10gErrIndicationAbility: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecErrIndicationAbility(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacFec10gEnable
    #/********************************************************************************/
    def do_mac_fec10g_enable(self, arg):
        '''
         xpsMacFec10gEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacFcFecEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacFec10gErrToPcsEnable
    #/********************************************************************************/
    def do_mac_fec10g_err_to_pcs_enable(self, arg):
        '''
         xpsMacFec10gErrToPcsEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacFcFecErrToPcsEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFec10CorrectedBlockCnt
    #/********************************************************************************/
    def do_mac_get_fec10_corrected_block_cnt(self, arg):
        '''
         xpsMacGetFec10CorrectedBlockCnt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecCorrectedBlockCnt(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_2)))
                pass
            delete_uint32_tp(value_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec10UncorrectedBlockCnt
    #/********************************************************************************/
    def do_mac_get_fec10_uncorrected_block_cnt(self, arg):
        '''
         xpsMacGetFec10UncorrectedBlockCnt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecUncorrectedBlockCnt(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_2)))
                pass
            delete_uint32_tp(value_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacFec10gSoftReset
    #/********************************************************************************/
    def do_mac_fec10g_soft_reset(self, arg):
        '''
         xpsMacFec10gSoftReset: Enter [ devId,portNum,enable ]
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
            ret = xpsMacFcFecSoftReset(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFec10gSoftReset
    #/********************************************************************************/
    def do_mac_get_fec10g_soft_reset(self, arg):
        '''
         xpsMacGetFec10gSoftReset: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecSoftReset(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)



    #/********************************************************************************/
    # command for xpsMacFec100gBypassEnable
    #/********************************************************************************/
    def do_mac_fec100g_bypass_enable(self, arg):
        '''
         xpsMacFec100gBypassEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRsFecBypassEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFec100gEnaErrorIndication
    #/********************************************************************************/
    def do_mac_get_fec100g_ena_error_indication(self, arg):
        '''
         xpsMacGetFec100gEnaErrorIndication: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            error_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecEnaErrorIndication(args[0],args[1],error_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('error = %d' % (uint8_tp_value(error_Ptr_2)))
                pass
            delete_uint8_tp(error_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFec100gAbility
    #/********************************************************************************/
    def do_mac_get_rs_fec100g_ability(self, arg):
        '''
         xpsMacGetRsFec100gAbility: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecAbility(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec100gBypassCorrAbility
    #/********************************************************************************/
    def do_mac_get_fec100g_bypass_corr_ability(self, arg):
        '''
         xpsMacGetFec100gBypassCorrAbility: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecBypassCorrAbility(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec100gErrIndicationAbi
    #/********************************************************************************/
    def do_mac_get_fec100g_err_indication_abi(self, arg):
        '''
         xpsMacGetFec100gErrIndicationAbi: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            error_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecErrIndicationAbi(args[0],args[1],error_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('error = %d' % (uint8_tp_value(error_Ptr_2)))
                pass
            delete_uint8_tp(error_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec100gCorrectedBlockCnt
    #/********************************************************************************/
    def do_mac_get_fec100g_corrected_block_cnt(self, arg):
        '''
         xpsMacGetFec100gCorrectedBlockCnt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecCorrectedBlockCnt(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_2)))
                pass
            delete_uint32_tp(value_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec100gUncorrectedBlockCnt
    #/********************************************************************************/
    def do_mac_get_fec100g_uncorrected_block_cnt(self, arg):
        '''
         xpsMacGetFec100gUncorrectedBlockCnt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecUncorrectedBlockCnt(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_2)))
                pass
            delete_uint32_tp(value_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecSymbolErrCnt
    #/********************************************************************************/
    def do_mac_get_rs_fec_symbol_err_cnt(self, arg):
        '''
         xpsMacGetRsFecSymbolErrCnt: Enter [ devId,portNum,laneNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,laneNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            value_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, laneNum=%d' % (args[0],args[1],args[2]))
            ret = xpsMacGetRsFecSymbolErrCnt(args[0],args[1],args[2],value_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_3)))
                pass
            delete_uint32_tp(value_Ptr_3)

    #/********************************************************************************/
    # command for xpsMacRsFecSoftReset
    #/********************************************************************************/
    def do_mac_rs_fec_soft_reset(self, arg):
        '''
         xpsMacRsFecSoftReset: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRsFecSoftReset(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacFecModeGet
    #/********************************************************************************/
    def do_mac_fec_mode_get(self, arg):
        '''
         xpsMacFecModeGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fecMode_Ptr_2 = new_xpFecModep()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacFecModeGet(args[0],args[1],fecMode_Ptr_2)
	    err = 0
	    if ret != 0:
	        print('Return Value = %d for portNum = %d' % (ret,args[1]))
	        err = 1
	    if err == 0:
                if(RS_FEC_MODE == xpFecModep_value(fecMode_Ptr_2)):
                    fecModeStr = "RS FEC MODE is enable"
                elif(FC_FEC_MODE == xpFecModep_value(fecMode_Ptr_2)):
                    fecModeStr = "FC FEC MODE is enable"
                elif(RS_544_514_FEC_MODE == xpFecModep_value(fecMode_Ptr_2)):
                    fecModeStr = "RS 544-514 FEC MODE is enable"
                else:
                    fecModeStr = "FEC is disable"
	        print('============================')
	        print('%s ' % fecModeStr)
	        print('Port %d:' % (args[1]))
	        print('============================')
	        print('')
            delete_xpFecModep(fecMode_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacFecModeSet
    #/********************************************************************************/
    def do_mac_fec_mode_set(self, arg):
        '''
         xpsMacFecModeSet: Enter [ devId,portNum,fecMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,fecMode(RS/FC/RS_544_514) ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
           
            if (args[2] == 'RS'):
                fecMode = eval('RS_FEC_MODE')
            elif (args[2] == 'FC'):
                fecMode = eval('FC_FEC_MODE')
            elif (args[2] == 'RS_544_514'):
                fecMode = eval('RS_544_514_FEC_MODE')
            else :
                print('Invalid FECMode provided')
                return
            ret = xpsMacFecModeSet(args[0],args[1],fecMode)
	    err = 0
	    if ret != 0:
	        print('Return Value = %d for portNum = %d' % (ret,args[1]))
	        err = 1
	    if err == 0:
                fecModeStr = "FECMode is Set"
	        print('============================')
	        print('%s ' % fecModeStr)
	        print('Port %d:' % (args[1]))
	        print('============================')
	        print('')


    #/********************************************************************************/
    # command for xpsMacGetPcsUseShortTimerEnable
    #/********************************************************************************/
    def do_mac_get_pcs_use_short_timer_enable(self, arg):
        '''
         xpsMacGetPcsUseShortTimerEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsUseShortTimerEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsEnable
    #/********************************************************************************/
    def do_mac_get_pcs_enable(self, arg):
        '''
         xpsMacGetPcsEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsDoubleSpeedEnable
    #/********************************************************************************/
    def do_mac_get_pcs_double_speed_enable(self, arg):
        '''
         xpsMacGetPcsDoubleSpeedEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsDoubleSpeedEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsBypassTxBufEnable
    #/********************************************************************************/
    def do_mac_get_pcs_bypass_tx_buf_enable(self, arg):
        '''
         xpsMacGetPcsBypassTxBufEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsBypassTxBufEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsBypassScramblerEnable
    #/********************************************************************************/
    def do_mac_get_pcs_bypass_scrambler_enable(self, arg):
        '''
         xpsMacGetPcsBypassScramblerEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsBypassScramblerEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsIgnoreSignalOkEnable
    #/********************************************************************************/
    def do_mac_get_pcs_ignore_signal_ok_enable(self, arg):
        '''
         xpsMacGetPcsIgnoreSignalOkEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsIgnoreSignalOkEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsDataPatternSelect
    #/********************************************************************************/
    def do_mac_get_pcs_data_pattern_select(self, arg):
        '''
         xpsMacGetPcsDataPatternSelect: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dataPattern_Ptr_2 = new_xpPCSDataPatternp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsDataPatternSelect(args[0],args[1],dataPattern_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dataPattern = %d' % (xpPCSDataPatternp_value(dataPattern_Ptr_2)))
                pass
            delete_xpPCSDataPatternp(dataPattern_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetBackPlaneAbility
    #/********************************************************************************/
    def do_mac_set_back_plane_ability(self, arg):
        '''
         xpsMacSetBackPlaneAbility: Enter [ devId,portNum,backPlaneAbilityMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,backPlaneAbilityMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, backPlaneAbilityMode=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetBackPlaneAbility(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetBackPlaneAbility
    #/********************************************************************************/
    def do_mac_get_back_plane_ability(self, arg):
        '''
         xpsMacGetBackPlaneAbility: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            backPlaneAbilityMode_Ptr_2 = new_xpBackPlaneAbilityModesp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetBackPlaneAbility(args[0],args[1],backPlaneAbilityMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('backPlaneAbilityMode = %d' % (xpBackPlaneAbilityModesp_value(backPlaneAbilityMode_Ptr_2)))
                pass
            delete_xpBackPlaneAbilityModesp(backPlaneAbilityMode_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetResetAllChannel
    #/********************************************************************************/
    def do_mac_set_reset_all_channel(self, arg):
        '''
         xpsMacSetResetAllChannel: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetResetAllChannel(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetResetAllChannel
    #/********************************************************************************/
    def do_mac_get_reset_all_channel(self, arg):
        '''
         xpsMacGetResetAllChannel: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetResetAllChannel(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTxFifoThreshold4ch
    #/********************************************************************************/
    def do_mac_set_tx_fifo_threshold4ch(self, arg):
        '''
         xpsMacSetTxFifoThreshold4ch: Enter [ devId,portNum,thresold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,thresold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, thresold=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxFifoThreshold4ch(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFifoThreshold4ch
    #/********************************************************************************/
    def do_mac_get_tx_fifo_threshold4ch(self, arg):
        '''
         xpsMacGetTxFifoThreshold4ch: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,thresold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            thresold_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFifoThreshold4ch(args[0],args[1],thresold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('thresold = %d' % (uint8_tp_value(thresold_Ptr_2)))
                pass
            delete_uint8_tp(thresold_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTxFifoThreshold2ch
    #/********************************************************************************/
    def do_mac_set_tx_fifo_threshold2ch(self, arg):
        '''
         xpsMacSetTxFifoThreshold2ch: Enter [ devId,portNum,thresold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,thresold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, thresold=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxFifoThreshold2ch(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFifoThreshold2ch
    #/********************************************************************************/
    def do_mac_get_tx_fifo_threshold2ch(self, arg):
        '''
         xpsMacGetTxFifoThreshold2ch: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            thresold_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFifoThreshold2ch(args[0],args[1],thresold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('thresold = %d' % (uint8_tp_value(thresold_Ptr_2)))
                pass
            delete_uint8_tp(thresold_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTxFifoThreshold1ch
    #/********************************************************************************/
    def do_mac_set_tx_fifo_threshold1ch(self, arg):
        '''
         xpsMacSetTxFifoThreshold1ch: Enter [ devId,portNum,thresold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,thresold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, thresold=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTxFifoThreshold1ch(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFifoThreshold1ch
    #/********************************************************************************/
    def do_mac_get_tx_fifo_threshold1ch(self, arg):
        '''
         xpsMacGetTxFifoThreshold1ch: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            thresold_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFifoThreshold1ch(args[0],args[1],thresold_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('thresold = %d' % (uint8_tp_value(thresold_Ptr_2)))
                pass
            delete_uint8_tp(thresold_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetAnRxSelector
    #/********************************************************************************/
    def do_mac_get_an_rx_selector(self, arg):
        '''
         xpsMacGetAnRxSelector: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rxSelector_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetAnRxSelector(args[0],args[1],rxSelector_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rxSelector = %d' % (uint8_tp_value(rxSelector_Ptr_2)))
                pass
            delete_uint8_tp(rxSelector_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetAnRxNonce
    #/********************************************************************************/
    def do_mac_get_an_rx_nonce(self, arg):
        '''
         xpsMacGetAnRxNonce: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rxNonce_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetAnRxNonce(args[0],args[1],rxNonce_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rxNonce = %d' % (uint8_tp_value(rxNonce_Ptr_2)))
                pass
            delete_uint8_tp(rxNonce_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxActive
    #/********************************************************************************/
    def do_mac_get_tx_active(self, arg):
        '''
         xpsMacGetTxActive: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxActive(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxError
    #/********************************************************************************/
    def do_mac_get_tx_error(self, arg):
        '''
         xpsMacGetTxError: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxError(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxUnderrun
    #/********************************************************************************/
    def do_mac_get_tx_underrun(self, arg):
        '''
         xpsMacGetTxUnderrun: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxUnderrun(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxPause
    #/********************************************************************************/
    def do_mac_get_tx_pause(self, arg):
        '''
         xpsMacGetTxPause: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxPause(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxActive
    #/********************************************************************************/
    def do_mac_get_rx_active(self, arg):
        '''
         xpsMacGetRxActive: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxActive(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxCrcError
    #/********************************************************************************/
    def do_mac_get_rx_crc_error(self, arg):
        '''
         xpsMacGetRxCrcError: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxCrcError(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxError
    #/********************************************************************************/
    def do_mac_get_rx_error(self, arg):
        '''
         xpsMacGetRxError: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxError(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxOverflow
    #/********************************************************************************/
    def do_mac_get_rx_overflow(self, arg):
        '''
         xpsMacGetRxOverflow: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxOverflow(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxPause
    #/********************************************************************************/
    def do_mac_get_rx_pause(self, arg):
        '''
         xpsMacGetRxPause: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxPause(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTxFifoOverFlowErrorInt
    #/********************************************************************************/
    def do_mac_set_tx_fifo_over_flow_error_int(self, arg):
        '''
         xpsMacSetTxFifoOverFlowErrorInt: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetTxFifoOverFlowErrorInt(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFifoOverFlowErrorInt
    #/********************************************************************************/
    def do_mac_get_tx_fifo_over_flow_error_int(self, arg):
        '''
         xpsMacGetTxFifoOverFlowErrorInt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFifoOverFlowErrorInt(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTxFifoOverFlowErrorIntEnable
    #/********************************************************************************/
    def do_mac_set_tx_fifo_over_flow_error_int_enable(self, arg):
        '''
         xpsMacSetTxFifoOverFlowErrorIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetTxFifoOverFlowErrorIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFifoOverFlowErrorIntEnable
    #/********************************************************************************/
    def do_mac_get_tx_fifo_over_flow_error_int_enable(self, arg):
        '''
         xpsMacGetTxFifoOverFlowErrorIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFifoOverFlowErrorIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFecErrorBlockInt
    #/********************************************************************************/
    def do_mac_get_fec_error_block_int(self, arg):
        '''
         xpsMacGetFecErrorBlockInt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecErrorBlockInt(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFecUncorrectedBlockInt
    #/********************************************************************************/
    def do_mac_get_fec_uncorrected_block_int(self, arg):
        '''
         xpsMacGetFecUncorrectedBlockInt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecUncorrectedBlockInt(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFecBlockLockInt
    #/********************************************************************************/
    def do_mac_get_fec_block_lock_int(self, arg):
        '''
         xpsMacGetFecBlockLockInt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecBlockLockInt(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFecBlockLockLostInt
    #/********************************************************************************/
    def do_mac_get_fec_block_lock_lost_int(self, arg):
        '''
         xpsMacGetFecBlockLockLostInt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecBlockLockLostInt(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFecBlockLockChangedInt
    #/********************************************************************************/
    def do_mac_get_fec_block_lock_changed_int(self, arg):
        '''
         xpsMacGetFecBlockLockChangedInt: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecBlockLockChangedInt(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFecErrorBlockIntEnable
    #/********************************************************************************/
    def do_mac_set_fec_error_block_int_enable(self, arg):
        '''
         xpsMacSetFecErrorBlockIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecErrorBlockIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFecErrorBlockIntEnable
    #/********************************************************************************/
    def do_mac_get_fec_error_block_int_enable(self, arg):
        '''
         xpsMacGetFecErrorBlockIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecErrorBlockIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFecUncorrectedBlockIntEnable
    #/********************************************************************************/
    def do_mac_set_fec_uncorrected_block_int_enable(self, arg):
        '''
         xpsMacSetFecUncorrectedBlockIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecUncorrectedBlockIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFecUncorrectedBlockIntEnable
    #/********************************************************************************/
    def do_mac_get_fec_uncorrected_block_int_enable(self, arg):
        '''
         xpsMacGetFecUncorrectedBlockIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecUncorrectedBlockIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFecBlockLockIntEnable
    #/********************************************************************************/
    def do_mac_set_fec_block_lock_int_enable(self, arg):
        '''
         xpsMacSetFecBlockLockIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecBlockLockIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFecBlockLockIntEnable
    #/********************************************************************************/
    def do_mac_get_fec_block_lock_int_enable(self, arg):
        '''
         xpsMacGetFecBlockLockIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecBlockLockIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFecBlockLockLostIntEnable
    #/********************************************************************************/
    def do_mac_set_fec_block_lock_lost_int_enable(self, arg):
        '''
         xpsMacSetFecBlockLockLostIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecBlockLockLostIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFecBlockLockLostIntEnable
    #/********************************************************************************/
    def do_mac_get_fec_block_lock_lost_int_enable(self, arg):
        '''
         xpsMacGetFecBlockLockLostIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecBlockLockLostIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFecBlockLockChangedIntEnable
    #/********************************************************************************/
    def do_mac_set_fec_block_lock_changed_int_enable(self, arg):
        '''
         xpsMacSetFecBlockLockChangedIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecBlockLockChangedIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetFecBlockLockChangedIntEnable
    #/********************************************************************************/
    def do_mac_get_fec_block_lock_changed_int_enable(self, arg):
        '''
         xpsMacGetFecBlockLockChangedIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFecBlockLockChangedIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFecErrorBlockIntTest
    #/********************************************************************************/
    def do_mac_set_fec_error_block_int_test(self, arg):
        '''
         xpsMacSetFecErrorBlockIntTest: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecErrorBlockIntTest(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetFecUncorrectedBlockIntTest
    #/********************************************************************************/
    def do_mac_set_fec_uncorrected_block_int_test(self, arg):
        '''
         xpsMacSetFecUncorrectedBlockIntTest: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecUncorrectedBlockIntTest(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetFecBlockLockIntTest
    #/********************************************************************************/
    def do_mac_set_fec_block_lock_int_test(self, arg):
        '''
         xpsMacSetFecBlockLockIntTest: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecBlockLockIntTest(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetFecBlockLockLostIntTest
    #/********************************************************************************/
    def do_mac_set_fec_block_lock_lost_int_test(self, arg):
        '''
         xpsMacSetFecBlockLockLostIntTest: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecBlockLockLostIntTest(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetFecBlockLockChangedIntTest
    #/********************************************************************************/
    def do_mac_set_fec_block_lock_changed_int_test(self, arg):
        '''
         xpsMacSetFecBlockLockChangedIntTest: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetFecBlockLockChangedIntTest(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacSetSgmiiIntEnable
    #/********************************************************************************/
    def do_mac_set_sgmii_int_enable(self, arg):
        '''
         xpsMacSetSgmiiIntEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetSgmiiIntEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetSgmiiIntEnable
    #/********************************************************************************/
    def do_mac_get_sgmii_int_enable(self, arg):
        '''
         xpsMacGetSgmiiIntEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetSgmiiIntEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetSgmiiIntTest
    #/********************************************************************************/
    def do_mac_set_sgmii_int_test(self, arg):
        '''
         xpsMacSetSgmiiIntTest: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetSgmiiIntTest(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetSgmiiIntTest
    #/********************************************************************************/
    def do_mac_get_sgmii_int_test(self, arg):
        '''
         xpsMacGetSgmiiIntTest: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetSgmiiIntTest(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSgmiiSerdesLoopbackEnableGet
    #/********************************************************************************/
    def do_mac_sgmii_serdes_loopback_enable_get(self, arg):
        '''
         xpsMacSgmiiSerdesLoopbackEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacSgmiiSerdesLoopbackEnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetReadCounterClear
    #/********************************************************************************/
    def do_mac_set_read_counter_clear(self, arg):
        '''
         xpsMacSetReadCounterClear: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetReadCounterClear(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetReadCounterClear
    #/********************************************************************************/
    def do_mac_get_read_counter_clear(self, arg):
        '''
         xpsMacGetReadCounterClear: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetReadCounterClear(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetReadCounterPriority
    #/********************************************************************************/
    def do_mac_set_read_counter_priority(self, arg):
        '''
         xpsMacSetReadCounterPriority: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetReadCounterPriority(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetReadCounterPriority
    #/********************************************************************************/
    def do_mac_get_read_counter_priority(self, arg):
        '''
         xpsMacGetReadCounterPriority: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetReadCounterPriority(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTxDrainMode
    #/********************************************************************************/
    def do_mac_set_tx_drain_mode(self, arg):
        '''
         xpsMacSetTxDrainMode: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetTxDrainMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxDrainMode
    #/********************************************************************************/
    def do_mac_get_tx_drain_mode(self, arg):
        '''
         xpsMacGetTxDrainMode: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxDrainMode(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetRxEarlyEofDetection
    #/********************************************************************************/
    def do_mac_set_rx_early_eof_detection(self, arg):
        '''
         xpsMacSetRxEarlyEofDetection: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetRxEarlyEofDetection(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetRxEarlyEofDetection
    #/********************************************************************************/
    def do_mac_get_rx_early_eof_detection(self, arg):
        '''
         xpsMacGetRxEarlyEofDetection: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxEarlyEofDetection(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetRsFecDebugShortAMP
    #/********************************************************************************/
    def do_mac_set_rs_fec_debug_short_amp(self, arg):
        '''
         xpsMacSetRsFecDebugShortAMP: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetRsFecDebugShortAMP(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetRsFecDebugShortAMP
    #/********************************************************************************/
    def do_mac_get_rs_fec_debug_short_amp(self, arg):
        '''
         xpsMacGetRsFecDebugShortAMP: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecDebugShortAMP(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetRsFecDebugTestInit
    #/********************************************************************************/
    def do_mac_set_rs_fec_debug_test_init(self, arg):
        '''
         xpsMacSetRsFecDebugTestInit: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetRsFecDebugTestInit(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetRsFecDebugTestInit
    #/********************************************************************************/
    def do_mac_get_rs_fec_debug_test_init(self, arg):
        '''
         xpsMacGetRsFecDebugTestInit: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecDebugTestInit(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetRsFecCWTestStopASM
    #/********************************************************************************/
    def do_mac_set_rs_fec_cw_test_stop_asm(self, arg):
        '''
         xpsMacSetRsFecCWTestStopASM: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetRsFecCWTestStopASM(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetRsFecCWTestStopASM
    #/********************************************************************************/
    def do_mac_get_rs_fec_cw_test_stop_asm(self, arg):
        '''
         xpsMacGetRsFecCWTestStopASM: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecCWTestStopASM(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetBpanSerdesTxSel
    #/********************************************************************************/
    def do_mac_set_bpan_serdes_tx_sel(self, arg):
        '''
         xpsMacSetBpanSerdesTxSel: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetBpanSerdesTxSel(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetBpanSerdesTxSel
    #/********************************************************************************/
    def do_mac_get_bpan_serdes_tx_sel(self, arg):
        '''
         xpsMacGetBpanSerdesTxSel: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetBpanSerdesTxSel(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetBpanMode
    #/********************************************************************************/
    def do_mac_set_bpan_mode(self, arg):
        '''
         xpsMacSetBpanMode: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetBpanMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetBpanMode
    #/********************************************************************************/
    def do_mac_get_bpan_mode(self, arg):
        '''
         xpsMacGetBpanMode: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetBpanMode(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetBpanRxGearboxReSync
    #/********************************************************************************/
    def do_mac_set_bpan_rx_gearbox_re_sync(self, arg):
        '''
         xpsMacSetBpanRxGearboxReSync: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSetBpanRxGearboxReSync(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetBpanRxGearboxReSync
    #/********************************************************************************/
    def do_mac_get_bpan_rx_gearbox_re_sync(self, arg):
        '''
         xpsMacGetBpanRxGearboxReSync: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetBpanRxGearboxReSync(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecInterruptStatus
    #/********************************************************************************/
    def do_mac_get_rs_fec_interrupt_status(self, arg):
        '''
         xpsMacGetRsFecInterruptStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecInterruptStatus(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecAmcLock
    #/********************************************************************************/
    def do_mac_get_rs_fec_amc_lock(self, arg):
        '''
         xpsMacGetRsFecAmcLock: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            amcLock_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecAmcLock(args[0],args[1],amcLock_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('amcLock = %d' % (uint8_tp_value(amcLock_Ptr_2)))
                pass
            delete_uint8_tp(amcLock_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecLane0InternalMap
    #/********************************************************************************/
    def do_mac_get_rs_fec_lane0_internal_map(self, arg):
        '''
         xpsMacGetRsFecLane0InternalMap: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lanMap_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecLane0InternalMap(args[0],args[1],lanMap_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lanMap = %d' % (uint8_tp_value(lanMap_Ptr_2)))
                pass
            delete_uint8_tp(lanMap_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecLane1InternalMap
    #/********************************************************************************/
    def do_mac_get_rs_fec_lane1_internal_map(self, arg):
        '''
         xpsMacGetRsFecLane1InternalMap: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lanMap_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecLane1InternalMap(args[0],args[1],lanMap_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lanMap = %d' % (uint8_tp_value(lanMap_Ptr_2)))
                pass
            delete_uint8_tp(lanMap_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecLane2InternalMap
    #/********************************************************************************/
    def do_mac_get_rs_fec_lane2_internal_map(self, arg):
        '''
         xpsMacGetRsFecLane2InternalMap: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lanMap_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecLane2InternalMap(args[0],args[1],lanMap_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lanMap = %d' % (uint8_tp_value(lanMap_Ptr_2)))
                pass
            delete_uint8_tp(lanMap_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRsFecLane3InternalMap
    #/********************************************************************************/
    def do_mac_get_rs_fec_lane3_internal_map(self, arg):
        '''
         xpsMacGetRsFecLane3InternalMap: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lanMap_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecLane3InternalMap(args[0],args[1],lanMap_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lanMap = %d' % (uint8_tp_value(lanMap_Ptr_2)))
                pass
            delete_uint8_tp(lanMap_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacLinkStatusInterruptEnableGet
    #/********************************************************************************/
    def do_mac_link_status_interrupt_enable_get(self, arg):
        '''
         xpsMacLinkStatusInterruptEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacLinkStatusInterruptEnableGet(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacLinkStatusInterruptEnableSet
    #/********************************************************************************/
    def do_mac_link_status_interrupt_enable_set(self, arg):
        '''
         xpsMacLinkStatusInterruptEnableSet: Enter [ devId,portNum,status ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,status ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, status=%d' % (args[0],args[1],args[2]))
            ret = xpsMacLinkStatusInterruptEnableSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetLinkStatus
    #/********************************************************************************/
    def do_mac_get_link_status(self, arg):
        '''
         xpsMacGetLinkStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            status_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetLinkStatus(args[0],args[1],status_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('status = %d' % (uint8_tp_value(status_Ptr_2)))
                pass
            delete_uint8_tp(status_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacPrintAllPortStatus
    #/********************************************************************************/
    def do_mac_print_all_port_status(self, arg):
        '''
         xpsMacPrintAllPortStatus: Enter [ devId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId]')
        else:
            args[0] = int(args[0])
            xpsMacPrintAllPortStatus(args[0])

    #/********************************************************************************/
    # command for xpsMacGetPortStatus
    #/********************************************************************************/
    def do_mac_get_port_status(self, arg):
        '''
         xpsMacGetPortStatus: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            linkStatus_Ptr_2 = new_uint8_tp()
            serdesStatus_Ptr_3 = new_uint8_tp()
            faultStatus_Ptr_4 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPortStatus(args[0],args[1],linkStatus_Ptr_2,serdesStatus_Ptr_3,faultStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('linkStatus = %d' % (uint8_tp_value(linkStatus_Ptr_2)))
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_3)))
                print('faultStatus = %d' % (uint8_tp_value(faultStatus_Ptr_4)))
                pass
            delete_uint8_tp(faultStatus_Ptr_4)
            delete_uint8_tp(serdesStatus_Ptr_3)
            delete_uint8_tp(linkStatus_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetLinkStatus
    #/********************************************************************************/
    def do_mac_set_link_status(self, arg):
        '''
         xpsMacSetLinkStatus: Enter [ devId,portNum,status ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,status ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, status=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetLinkStatus(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacLinkStatusOverride
    #/********************************************************************************/
    def do_mac_link_status_override(self, arg):
        '''
         xpsMacLinkStatusOverride: Enter [ devId,portNum,status ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,status ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, status=%d' % (args[0],args[1],args[2]))
            ret = xpsMacLinkStatusOverride(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacFaultStatusOverride
    #/********************************************************************************/
    def do_mac_fault_status_override(self, arg):
        '''
         xpsMacFaultStatusOverride: Enter [ devId,portNum,status ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,status ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, status=%d' % (args[0],args[1],args[2]))
            ret = xpsMacFaultStatusOverride(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetRxCrcCheckEnable
    #/********************************************************************************/
    def do_mac_get_rx_crc_check_enable(self, arg):
        '''
         xpsMacGetRxCrcCheckEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxCrcCheckEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxStripFcsEnable
    #/********************************************************************************/
    def do_mac_get_rx_strip_fcs_enable(self, arg):
        '''
         xpsMacGetRxStripFcsEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxStripFcsEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxFlowControlDecodeEnable
    #/********************************************************************************/
    def do_mac_get_rx_flow_control_decode_enable(self, arg):
        '''
         xpsMacGetRxFlowControlDecodeEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxFlowControlDecodeEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxMaxFrmLen
    #/********************************************************************************/
    def do_mac_get_rx_max_frm_len(self, arg):
        '''
         xpsMacGetRxMaxFrmLen: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            frameSize_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxMaxFrmLen(args[0],args[1],frameSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('frameSize = %d' % (uint16_tp_value(frameSize_Ptr_2)))
                pass
            delete_uint16_tp(frameSize_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxPriorityPauseVector
    #/********************************************************************************/
    def do_mac_get_tx_priority_pause_vector(self, arg):
        '''
         xpsMacGetTxPriorityPauseVector: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            txPriPauseVector_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxPriorityPauseVector(args[0],args[1],txPriPauseVector_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('txPriPauseVector = %d' % (uint16_tp_value(txPriPauseVector_Ptr_2)))
                pass
            delete_uint16_tp(txPriPauseVector_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxIfgLen
    #/********************************************************************************/
    def do_mac_get_tx_ifg_len(self, arg):
        '''
         xpsMacGetTxIfgLen: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ifgLength_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxIfgLen(args[0],args[1],ifgLength_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ifgLength = %d' % (uint8_tp_value(ifgLength_Ptr_2)))
                pass
            delete_uint8_tp(ifgLength_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxJabberFrmSize
    #/********************************************************************************/
    def do_mac_get_tx_jabber_frm_size(self, arg):
        '''
         xpsMacGetTxJabberFrmSize: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            jabberFrmSize_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxJabberFrmSize(args[0],args[1],jabberFrmSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('jabberFrmSize = %d' % (uint16_tp_value(jabberFrmSize_Ptr_2)))
                pass
            delete_uint16_tp(jabberFrmSize_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxJabberFrmSize
    #/********************************************************************************/
    def do_mac_get_rx_jabber_frm_size(self, arg):
        '''
         xpsMacGetRxJabberFrmSize: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            jabberFrmSize_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxJabberFrmSize(args[0],args[1],jabberFrmSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('jabberFrmSize = %d' % (uint16_tp_value(jabberFrmSize_Ptr_2)))
                pass
            delete_uint16_tp(jabberFrmSize_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxIfgCtrlPerFrameEnable
    #/********************************************************************************/
    def do_mac_get_tx_ifg_ctrl_per_frame_enable(self, arg):
        '''
         xpsMacGetTxIfgCtrlPerFrameEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxIfgCtrlPerFrameEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFlowCtrlPauseTime
    #/********************************************************************************/
    def do_mac_get_flow_ctrl_pause_time(self, arg):
        '''
         xpsMacGetFlowCtrlPauseTime: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pauseTime_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFlowCtrlPauseTime(args[0],args[1],pauseTime_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pauseTime = %d' % (uint16_tp_value(pauseTime_Ptr_2)))
                pass
            delete_uint16_tp(pauseTime_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetXoffPauseTime
    #/********************************************************************************/
    def do_mac_get_xoff_pause_time(self, arg):
        '''
         xpsMacGetXoffPauseTime: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pauseTime_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetXoffPauseTime(args[0],args[1],pauseTime_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pauseTime = %d' % (uint16_tp_value(pauseTime_Ptr_2)))
                pass
            delete_uint16_tp(pauseTime_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetXonPauseTime
    #/********************************************************************************/
    def do_mac_get_xon_pause_time(self, arg):
        '''
         xpsMacGetXonPauseTime: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pauseTime_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetXonPauseTime(args[0],args[1],pauseTime_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pauseTime = %d' % (uint16_tp_value(pauseTime_Ptr_2)))
                pass
            delete_uint16_tp(pauseTime_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetTimeStampId
    #/********************************************************************************/
    def do_mac_set_time_stamp_id(self, arg):
        '''
         xpsMacSetTimeStampId: Enter [ devId,portNum,timeStampId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,timeStampId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, timeStampId=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetTimeStampId(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFcsInvertEnable
    #/********************************************************************************/
    def do_mac_get_tx_fcs_invert_enable(self, arg):
        '''
         xpsMacGetTxFcsInvertEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFcsInvertEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)


    #/********************************************************************************/
    # command for xpsMacGetMacFlowCtrlFrmSrcAddr
    #/********************************************************************************/
    def do_mac_get_mac_flow_ctrl_frm_src_addr(self, arg):
        '''
         xpsMacGetMacFlowCtrlFrmSrcAddr: Enter [ devId,portNum,macAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macAddr = macAddr_t()
            ret = xpsMacGetMacFlowCtrlFrmSrcAddr(args[0],args[1],macAddr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('FrmSrcMacAddr : '),
                for i in range(5,-1,-1):
                    print('%02x' % (macAddr[i])),
                    if i > 0:
                        sys.stdout.write(':'),
                print('')
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetMacFlowCtrlFrmDestAddr
    #/********************************************************************************/
    def do_mac_get_mac_flow_ctrl_frm_dest_addr(self, arg):
        '''
         xpsMacGetMacFlowCtrlFrmDestAddr: Enter [ devId,portNum]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macAddr = macAddr_t()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetMacFlowCtrlFrmDestAddr(args[0],args[1],macAddr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('FrmDestMacAddr : '),
                for i in range(5,-1,-1):
                    print('%02x' % (macAddr[i])),
                    if i > 0:
                        sys.stdout.write(':'),
                print('')
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetTxFlowControlEnable
    #/********************************************************************************/
    def do_mac_get_tx_flow_control_enable(self, arg):
        '''
         xpsMacGetTxFlowControlEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxFlowControlEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxPriorityFlowControlEnable
    #/********************************************************************************/
    def do_mac_get_tx_priority_flow_control_enable(self, arg):
        '''
         xpsMacGetTxPriorityFlowControlEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxPriorityFlowControlEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxCrcCheckEnable
    #/********************************************************************************/
    def do_mac_get_tx_crc_check_enable(self, arg):
        '''
         xpsMacGetTxCrcCheckEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxCrcCheckEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxPreambleLen
    #/********************************************************************************/
    def do_mac_get_tx_preamble_len(self, arg):
        '''
         xpsMacGetTxPreambleLen: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            bytes_Ptr_2 = new_xpPreambleLenBytesp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxPreambleLen(args[0],args[1],bytes_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                retval = xpPreambleLenBytesp_value(bytes_Ptr_2)
                if(retval == eval('PREAMBLE_LEN_BYTES_8')):
                    retval = 8
                print('bytes = %d' % (retval))
                pass
            delete_xpPreambleLenBytesp(bytes_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxtxPortEnable
    #/********************************************************************************/
    def do_mac_get_rxtx_port_enable(self, arg):
        '''
         xpsMacGetRxtxPortEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxtxPortEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetTxPortEnable
    #/********************************************************************************/
    def do_mac_get_tx_port_enable(self, arg):
        '''
         xpsMacGetTxPortEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetTxPortEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetRxPortEnable
    #/********************************************************************************/
    def do_mac_get_rx_port_enable(self, arg):
        '''
         xpsMacGetRxPortEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRxPortEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPortSoftReset
    #/********************************************************************************/
    def do_mac_get_port_soft_reset(self, arg):
        '''
         xpsMacGetPortSoftReset: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPortSoftReset(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPortReset
    #/********************************************************************************/
    def do_mac_get_port_reset(self, arg):
        '''
         xpsMacGetPortReset: Enter [ devId,portNum,reset ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,reset ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint8_tp()
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, reset=%d' % (args[0],args[1],args[2]))
            ret = xpsMacGetPortReset(args[0],enable_Ptr_1,args[2],args[2])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_1)))
                pass
            delete_uint8_tp(enable_Ptr_1)

    #/********************************************************************************/
    # command for xpsMacGetMdioClkDivisonCtrl
    #/********************************************************************************/
    def do_mac_get_mdio_clk_divison_ctrl(self, arg):
        '''
         xpsMacGetMdioClkDivisonCtrl: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            divCtrlRatio_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetMdioClkDivisonCtrl(args[0],args[1],divCtrlRatio_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('divCtrlRatio = %d' % (uint8_tp_value(divCtrlRatio_Ptr_2)))
                pass
            delete_uint8_tp(divCtrlRatio_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsTestPatternSelect
    #/********************************************************************************/
    def do_mac_get_pcs_test_pattern_select(self, arg):
        '''
         xpsMacGetPcsTestPatternSelect: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            testPattern_Ptr_2 = new_xpPCSTestPatternp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsTestPatternSelect(args[0],args[1],testPattern_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('testPattern = %d' % (xpPCSTestPatternp_value(testPattern_Ptr_2)))
                pass
            delete_xpPCSTestPatternp(testPattern_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetPcsScrambledIdealTestPattern
    #/********************************************************************************/
    def do_mac_get_pcs_scrambled_ideal_test_pattern(self, arg):
        '''
         xpsMacGetPcsScrambledIdealTestPattern: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetPcsScrambledIdealTestPattern(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec10gEnable
    #/********************************************************************************/
    def do_mac_get_fec10g_enable(self, arg):
        '''
         xpsMacGetFec10gEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec10gErrToPcsEnable
    #/********************************************************************************/
    def do_mac_get_fec10g_err_to_pcs_enable(self, arg):
        '''
         xpsMacGetFec10gErrToPcsEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetFcFecErrToPcsEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacGetFec100gBypassEnable
    #/********************************************************************************/
    def do_mac_get_fec100g_bypass_enable(self, arg):
        '''
         xpsMacGetFec100gBypassEnable: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecBypassEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSetFec100gEnaErrorIndication
    #/********************************************************************************/
    def do_mac_set_fec100g_ena_error_indication(self, arg):
        '''
         xpsMacSetFec100gEnaErrorIndication: Enter [ devId,portNum,error ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,error ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, error=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSetRsFecEnaErrorIndication(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacGetRsFecSoftReset
    #/********************************************************************************/
    def do_mac_get_rs_fec_soft_reset(self, arg):
        '''
         xpsMacGetRsFecSoftReset: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacGetRsFecSoftReset(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacXpMacWrite
    #/********************************************************************************/
    def do_mac_xp_mac_write(self, arg):
        '''
         xpsMacXpMacWrite: Enter [ devId,portNum,regAddr,regValue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,regAddr,regValue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            #print('Input Arguments are, devId=%d, portNum=%d, regAddr=0x%x, regValue=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacXpMacWrite(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacXpMacRead
    #/********************************************************************************/
    def do_mac_xp_mac_read(self, arg):
        '''
         xpsMacXpMacRead: Enter [ devId,portNum,regAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,regAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            regValue_Ptr_3 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, regAddr=0x%x' % (args[0],args[1],args[2]))
            ret = xpsMacXpMacRead(args[0],args[1],args[2],regValue_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('regValue = %d' % (uint16_tp_value(regValue_Ptr_3)))
                pass
            delete_uint16_tp(regValue_Ptr_3)

    #/********************************************************************************/
    # command for xpsMacRegisterPlatformSerdesInitFunc
    #/********************************************************************************/
    def do_mac_register_platform_serdes_init_func(self, arg):
        '''
         xpsMacRegisterPlatformSerdesInitFunc: Enter [ func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ func ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, func=%d' % (args[0]))
            ret = xpsMacRegisterPlatformSerdesInitFunc(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacCounterRxTxRate
    #/********************************************************************************/
    def do_get_port_rx_tx_rate(self, arg):
        '''
        getPortRxTxRate: Enter [devId, rx(0)/tx(1), portNum, time(seconds)]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        ret = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rx(0)/tx(1),portNum,time(seconds) ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            totalOctets_Ptr_2 = new_uint64_tp()
            totalPkts_Ptr_2 = new_uint64_tp()

            previousTime = int(round(time.time()))

            if args[1]:
                ret += xpsMacCounterStatsTxOctets(args[0],args[2],totalOctets_Ptr_2)
                ret += xpsMacCounterStatsTxPkts(args[0],args[2],totalPkts_Ptr_2)
            else:
                ret += xpsMacCounterStatsRxOctets(args[0],args[2],totalOctets_Ptr_2)
                ret += xpsMacCounterStatsRxPkts(args[0],args[2],totalPkts_Ptr_2)

            prevPktCount = uint64_tp_value(totalPkts_Ptr_2)
            prevOctetCount = uint64_tp_value(totalOctets_Ptr_2)

            time.sleep(args[3])

            if args[1]:
                ret += xpsMacCounterStatsTxOctets(args[0],args[2],totalOctets_Ptr_2)
                ret += xpsMacCounterStatsTxPkts(args[0],args[2],totalPkts_Ptr_2)
            else:
                ret += xpsMacCounterStatsRxOctets(args[0],args[2],totalOctets_Ptr_2)
                ret += xpsMacCounterStatsRxPkts(args[0],args[2],totalPkts_Ptr_2)

            currentTime = int(round(time.time()))

            totalOctetCount = uint64_tp_value(totalOctets_Ptr_2) - prevOctetCount
            totalPktCount = uint64_tp_value(totalPkts_Ptr_2) - prevPktCount

            dataRate = (((totalPktCount * 20) + totalOctetCount) * 8) / (currentTime - previousTime)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Total Octets = %d' % (totalOctetCount))
                print('Total Pkts = %d' % (totalPktCount))
                if args[1]:
                    print('Tx Rate = %d bits/sec' % (dataRate))
                else:
                    print('Rx Rate = %d bits/sec' % (dataRate))
                pass
            delete_uint64_tp(totalOctets_Ptr_2)
            delete_uint64_tp(totalPkts_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacRxFlowControlFilterEnable
    #/********************************************************************************/
    def do_mac_rx_flow_control_filter_enable(self, arg):
        '''
         xpsMacRxFlowControlFilterEnable: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRxFlowControlFilterEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacRxFlowControlFilterEnableGet
    #/********************************************************************************/
    def do_mac_rx_flow_control_filter_enable_get(self, arg):
        '''
         xpsMacRxFlowControlFilterEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRxFlowControlFilterEnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacDisableMacInterrupts
    #/********************************************************************************/
    def do_mac_disable_all_interrupts(self, arg):
        '''
         xpsMacDisableMacInterrupts: Enter [ devId,macNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,macNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, macNum=%d' % (args[0],args[1]))
            ret = xpsMacDisableMacInterrupts(args[0],args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass

   #/********************************************************************************/
    # command for xpsMacTxFaultGenSet
    #/********************************************************************************/
    def do_mac_tx_fault_gen_set(self, arg):
        '''
         xpsMacTxFaultGenSet: Enter [ devId,portNum,faultType,value ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,faultType,value ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3], 16)
            #print('Input Arguments are, devId=%d, portNum=%d, faultType=%d, value=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacTxFaultGenSet(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacInterruptEnable
    #/********************************************************************************/
    def do_mac_interrupt_enable(self, arg):
        '''
         xpsMacInterruptEnable: Enter [ devId,portNum,eventType,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,eventType,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, eventType=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMacInterruptEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacTxFaultGenGet
    #/********************************************************************************/
    def do_mac_tx_fault_gen_get(self, arg):
        '''
         xpsMacTxFaultGenGet: Enter [ devId,portNum,faultType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,faultType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            value_Ptr_3 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, faultType=%d' % (args[0],args[1],args[2]))
            ret = xpsMacTxFaultGenGet(args[0],args[1],args[2],value_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint16_tp_value(value_Ptr_3)))
                pass
            delete_uint16_tp(value_Ptr_3)

    #/********************************************************************************/
    # command for xpsMacPcsDecodeTrapGet
    #/********************************************************************************/
    def do_mac_pcs_decode_trap_get(self, arg):
        '''
         xpsMacPcsDecodeTrapGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            trap_Ptr_2 = new_xpDecodeTrapp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacPcsDecodeTrapGet(args[0],args[1],trap_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('trap = %d' % (xpDecodeTrapp_value(trap_Ptr_2)))
                pass
            delete_xpDecodeTrapp(trap_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacPcsDecodeTrapSet
    #/********************************************************************************/
    def do_mac_pcs_decode_trap_set(self, arg):
        '''
         xpsMacPcsDecodeTrapSet: Enter [ devId,portNum,trap ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,trap ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, trap=%d' % (args[0],args[1],args[2]))
            ret = xpsMacPcsDecodeTrapSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortSerdesSignalOverride
    #/********************************************************************************/
    def do_mac_port_serdes_signal_override(self, arg):
        '''
         xpsMacPortSerdesSignalOverride: Enter [ devId,portNum,serdesSignalVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,serdesSignalVal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, serdesSignalVal=%d' % (args[0],args[1],args[2]))
            ret = xpsMacPortSerdesSignalOverride(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortSerdesTuneConditionGet
    #/********************************************************************************/
    def do_mac_port_serdes_tune_condition_get(self, arg):
        '''
         xpsMacPortSerdesTuneConditionGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacPortSerdesTuneConditionGet(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPcsOperationalStatusGet
    #/********************************************************************************/
    def do_mac_pcs_operational_status_get(self, arg):
        '''
         xpsMacPcsOperationalStatusGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacPcsOperationalStatusGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacPcsRErrBlckAndPcsBerCntGet
    #/********************************************************************************/
    def do_mac_pcs_r_err_blck_and_ber_cnt_get(self, arg):
        '''
         xpsMacPcsRErrBlckAndPcsBerCntGet: Enter [ devId,portNum/startPort-endPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum/startPort-endPort ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            count_Ptr_2 = new_uint32_tp()
            count_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum/startPort-endPort=%d' % (args[0],args[1]))
            valid_Ptr = new_uint8_tp()
            for portNum in range(startIdx,endIdx+1):
                xpsMacIsPortNumValid(args[0],portNum,valid_Ptr)
                if((uint8_tp_value(valid_Ptr))):
                    if(xpsIsPortInited(args[0],portNum) == 0):
                        ret = xpsMacpcsRErrBlckAndPcsBerCntGet(args[0],portNum,count_Ptr_2,count_Ptr_3)
                        err = 0
                        if ret != 0:
                            print('Return Value = %d for portNum = %d' % (ret,portNum))
                            err = 1
                        if err == 0:
                            print('============================')
                            print('Port %d:' % (portNum))
                            print('============================')
                            print('pcs R error block count = %d ' % (uint32_tp_value(count_Ptr_2)))
                            print('pcs BER count value = %d ' % (uint32_tp_value(count_Ptr_3)))
                            print('')
                    else:
                        print('============================')
                        print('Port %d:' % (portNum))
                        print('============================')
                        print('portNum %d is not initialized' % (portNum))
                        print('')
                else:
                    pass
            delete_uint32_tp(count_Ptr_2)
            delete_uint32_tp(count_Ptr_3)

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
    # command for xpsMacSgmiiAutoNegSet
    #/********************************************************************************/
    def do_sgmii_auto_neg_set(self, arg):
        '''
         xpsMacSgmiiAutoNegSet: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSgmiiAutoNegSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacSgmiiAutoNegGet
    #/********************************************************************************/
    def do_sgmii_auto_neg_get(self, arg):
        '''
         xpsMacSgmiiAutoNegGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacSgmiiAutoNegGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacSgmiiAutoNegRestartSet
    #/********************************************************************************/
    def do_sgmii_auto_neg_restart_set(self, arg):
        '''
         xpsMacSetSgmiiAutoNegRestartSet: Enter [ devId,portNum,enable ]
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
            ret = xpsMacSgmiiAutoNegRestartSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacSgmiiAutoNegRestartGet
    #/********************************************************************************/
    def do_sgmii_auto_neg_restart_get(self, arg):
        '''
         xpsMacSgmiiAutoNegRestartGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacSgmiiAutoNegRestartGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacCustomAlignmentMarkerLenSet
    #/********************************************************************************/
    def do_custom_am_len_get(self, arg):
        '''
         xpsMacCustomAlignmentMarkerLenSet: Enter [ devId,portNum,value ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,frameSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, value=%d' % (args[0],args[1],args[2]))
            ret = xpsMacCustomAlignmentMarkerLenSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacCustomAlignmentMarkerLenGet
    #/********************************************************************************/
    def do_custom_am_len_get(self, arg):
        '''
         xpsMacCustomAlignmentMarkerLenGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacCustomAlignmentMarkerLenGet(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_2)))
                pass
            delete_uint32_tp(value_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacRsFecProgAmpLockLenSet
    #/********************************************************************************/
    def do_rsfec_prog_amp_lock_len_set(self, arg):
        '''
         xpsMacRsFecProgAmpLockLenSet: Enter [ devId,portNum,value ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,value ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, value=%d' % (args[0],args[1],args[2]))
            ret = xpsMacRsFecProgAmpLockLenSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacRsFecProgAmpLockLenGet
    #/********************************************************************************/
    def do_rs_fec_prog_amp_lock_len_get(self, arg):
        '''
         xpsMacRsFecProgAmpLockLenGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRsFecProgAmpLockLenGet(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint32_tp_value(value_Ptr_2)))
                pass
            delete_uint32_tp(value_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacRsFecProgCwAmpLenSet
    #/********************************************************************************/
    def do_mac_rs_fec_prog_cw_amp_len_set(self, arg):
        '''
         xpsMacRsFecProgCwAmpLenSet: Enter [ devId,portNum,value ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,value ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, value=%d' % (args[0],args[1],args[2]))
            ret = xpsMacRsFecProgCwAmpLenSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacRsFecProgCwAmpLenGet
    #/********************************************************************************/
    def do_mac_rs_fec_prog_cw_amp_len_get(self, arg):
        '''
         xpsMacRsFecProgCwAmpLenGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            value_Ptr_2 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRsFecProgCwAmpLenGet(args[0],args[1],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('value = %d' % (uint16_tp_value(value_Ptr_2)))
                pass
            delete_uint16_tp(value_Ptr_2)
    #/********************************************************************************/
    # command for xpsMacSerdesRxLaneRemapSet
    #/********************************************************************************/
    def do_mac_serdes_rx_lane_remap_set(self, arg):
        '''
         xpsMacSerdesRxLaneRemapSet: Enter [ devId,portNum,laneNum,serdesLaneNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,laneNum,serdesLaneNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, laneNum=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSerdesRxLaneRemapSet(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacSerdesRxLaneRemapGet
    #/********************************************************************************/
    def do_mac_serdes_rx_lane_remap_get(self, arg):
        '''
         xpsMacSerdesRxLaneRemapGet: Enter [ devId,portNum,laneNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,laneNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            value_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacSerdesRxLaneRemapGet(args[0],args[1],args[2],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('serdes Lane associated with PCS Rx Lane %d = %d' % args[2],(uint8_tp_value(value_Ptr_2)))
                pass
            delete_uint8_tp(value_Ptr_2)
    #/********************************************************************************/
    # command for xpsMacSerdesTxLaneRemapSet
    #/*******************************************************************************/
    def do_mac_serdes_tx_lane_remap_set(self, arg):
        '''
         xpsMacSerdesTxLaneRemapSet: Enter [ devId,portNum,laneNum,serdesLaneNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,laneNum,serdesLaneNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portNum=%d, laneNum=%d' % (args[0],args[1],args[2]))
            ret = xpsMacSerdesTxLaneRemapSet(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacSerdesTxLaneRemapGet
    #/********************************************************************************/
    def do_mac_serdes_tx_lane_remap_get(self, arg):
        '''
         xpsMacSerdesTxLaneRemapGet: Enter [ devId,portNum,laneNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,laneNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            value_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacSerdesTxLaneRemapGet(args[0],args[1],args[2],value_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('serdes Lane associated with PCS Rx Lane %d = %d' % args[2],(uint8_tp_value(value_Ptr_2)))
                pass
            delete_uint8_tp(value_Ptr_2)
    #/********************************************************************************/
    # command for xpsMacRsFecAlwaysUseClause49EnableSet
    #/********************************************************************************/
    def do_rsfec_always_use_clause_enable_set(self, arg):
        '''
         xpsMacRsFecAlwaysUseClause49EnableSet: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRsFecAlwaysUseClause49EnableSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacRsFecAlwaysUseClause49EnableGet
    #/********************************************************************************/
    def do_rsfec_always_use_clause_enable_get(self, arg):
        '''
         xpsMacRsFecAlwaysUseClause49EnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRsFecAlwaysUseClause49EnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacRsFecNeverUseClause49EnableSet
    #/********************************************************************************/
    def do_rsfec_never_use_clause_enable_set(self, arg):
        '''
         xpsMacRsFecNeverUseClause49EnableSet: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRsFecNeverUseClause49EnableSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacRsFecNeverUseClause49EnableGet
    #/********************************************************************************/
    def do_rsfec_never_use_clause_enable_get(self, arg):
        '''
         xpsMacRsFecNeverUseClause49EnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRsFecNeverUseClause49EnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacRsFecPcsScramblerEnableSet
    #/********************************************************************************/
    def do_rsfec_pcs_scrambler_enable_set(self, arg):
        '''
         xpsMacRsFecsFecPcsScramblerEnableSet: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRsFecPcsScramblerEnableSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacRsFecPcsScramblerEnableGet
    #/********************************************************************************/
    def do_rsfec_pcs_scrambler_enable_get(self, arg):
        '''
         xpsMacRsFecPcsScramblerEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRsFecPcsScramblerEnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacRsFecMlgScramblerEnableSet
    #/********************************************************************************/
    def do_rsfec_mlg_scrambler_enable_set(self, arg):
        '''
         xpsMacRsFecsFecMlgScramblerEnableSet: Enter [ devId,portNum,enable ]
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
            ret = xpsMacRsFecMlgScramblerEnableSet(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMacRsFecMlgScramblerEnableGet
    #/********************************************************************************/
    def do_rsfec_mlg_scrambler_enable_get(self, arg):
        '''
         xpsMacRsFecMlgScramblerEnableGet: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsMacRsFecMlgScramblerEnableGet(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsMacLinkStatusOverrideGet
    #/********************************************************************************/
    def do_link_status_override_get(self, arg):
        '''
         xpsMacLinkStatusOverrideGet: Enter [ devId,portNum/startPort-endPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum/startPort-endPort ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            status = new_uint8_tp()
            valid_Ptr = new_uint8_tp()
            for portNum in range(startIdx,endIdx+1):
                xpsMacIsPortNumValid(args[0],portNum,valid_Ptr)
                if((uint8_tp_value(valid_Ptr))):
                    ret = xpsMacLinkStatusOverrideGet(args[0],portNum,status)
                    if ret != 0:
                        print('Return Value = %d for portNum = %d' % (ret,portNum))
                    else:
                        print('status = %d for portNum = %d' % (uint8_tp_value(status),portNum))
                else:
                    print('Return Value = %d for portNum = %d' % (ret,portNum))
                    pass
            delete_uint8_tp(status)
            delete_uint8_tp(valid_Ptr)

    #/********************************************************************************/
    # command for xpsMacFaultStatusOverrideGet
    #/********************************************************************************/
    def do_fault_status_override_get(self, arg):
        '''
         xpsMacFaultStatusOverrideGet: Enter [ devId,portNum/startPort-endPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum/startPort-endPort ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            status = new_uint8_tp()
            valid_Ptr = new_uint8_tp()
            for portNum in range(startIdx,endIdx+1):
                xpLinkManagerIsPortNumValid(args[0],portNum,valid_Ptr)
                if((uint8_tp_value(valid_Ptr))):
                    ret = xpsMacFaultStatusOverrideGet(args[0],portNum,status)
                    if ret != 0:
                        print('Return Value = %d for portNum = %d' % (ret,portNum))
                    else:
                        print('status = %d for portNum = %d' % (uint8_tp_value(status),portNum))
                else:
                    print('Return Value = %d for portNum = %d' % (ret,portNum))
                    pass
            delete_uint8_tp(status)
            delete_uint8_tp(valid_Ptr)

    #/********************************************************************************/
    # command for xpsMacPortSerdesDfeRunning
    #/********************************************************************************/
    def do_mac_port_serdes_dfe_running_debug(self, arg):
        '''
         cpssHalMacPortSerdesIsDfeRunningDebug: Enter [ devId,PortId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq =2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,PortId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dfeRunning_Ptr_1 = new_uint8_tp()
            dfeRunning_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = cpssHalMacPortSerdesIsDfeRunningDebug(args[0],args[1],dfeRunning_Ptr_1,dfeRunning_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rxTuneStatusPtr = %d' % (uint8_tp_value(dfeRunning_Ptr_1)))
                print('txTuneStatusPtr = %d' % (uint8_tp_value(dfeRunning_Ptr_2)))
                pass
            delete_uint8_tp(dfeRunning_Ptr_2)
            delete_uint8_tp(dfeRunning_Ptr_1)


    #/********************************************************************************/
    # command for xpsMacPortSerdesTxTune
    #/********************************************************************************/
    def do_mac_port_serdes_tx_tune(self, arg):
        '''
         xpsMacPortSerdesTune: Enter [devId, portNum/startPort-endPort, force]
         Valid values for tuneMode : <serdesDfeTuneMode>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [devId, portNum/startPort-endPort, force]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]

            force = int(args[2])
            portList = range(startIdx,endIdx+1)
            arrUint32 = new_arrUint32(len(portList))
            for index, value in enumerate(portList):
                arrUint32_setitem(arrUint32, index, value)
            #print('Input Arguments are, devId=%d, portNum/startPort-endPort=%d, force=%d' % (args[0],args[1],args[2]))
            ret = xpsMacPortSerdesTxTune(args[0],arrUint32,len(portList),force)
            if ret != 0:
                print('Error returned = %d ' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMacPortInit
    #/********************************************************************************/
    def do_mac_port_init(self, arg):
        '''
         xpsMacPortCreate: Enter [ devId,portNum,macMode,fecMode,enableFec ]
         Valid speedMode are: 8x400G, 4x200G, 2x100G, 1x100G, 1x50G, 1x40G, 2x40G, 2x50G, 4x10G, 4x25G, 4x1G, 1x10G, 1G
         Valid FECMode are: RS, FC, RS_544_514
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,macMode,fecMode,enableFec ]')
            print('Valid speedMode are: 8x400G, 4x200G, 2x100G, 1x100G, 1x50G, 1x40G, 2x40G, 2x50G, 4x10G, 4x25G, 4x1G, 1x10G, 1G')
            print('Valid FECMode are: RS, FC, RS_544_514')

        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            if (args[2] == '1x100G') :
                macMode = eval('MAC_MODE_1X100GB')
            elif (args[2] == '4x200G') :
                macMode = eval('MAC_MODE_4X200GB')
            elif (args[2] == '8x400G') :
                macMode = eval('MAC_MODE_8X400GB')
            elif (args[2] == '2x100G') :
                macMode = eval('MAC_MODE_2X100GB')
            elif (args[2] == '1x40G') :
                macMode = eval('MAC_MODE_1X40GB')
            elif (args[2] == '2x40G') :
                macMode = eval('MAC_MODE_2X40GB')
            elif (args[2] == '2x50G') :
                macMode = eval('MAC_MODE_2X50GB')
            elif (args[2] == '4x10G') :
                macMode = eval('MAC_MODE_4X10GB')
            elif (args[2] == '4x25G') :
                macMode = eval('MAC_MODE_4X25GB')
            elif (args[2] == '1x10G') :
                macMode = eval('MAC_MODE_1X10GB')
            elif (args[2] == '4x1G') :
                macMode = eval('MAC_MODE_4X1GB')
            elif (args[2] == '1G') :
                macMode = eval('MAC_MODE_1GB')
            elif (args[2] == '1x50G'):
                macMode = eval('MAC_MODE_1X50GB')
            #elif (args[2] == '100M') :
                #macMode = eval('MAC_MODE_100M')
            #elif (args[2] == '10M') :
                #macMode = eval('MAC_MODE_10M')
            else :
                print('Invalid speedMode provided')
                return

            enableFEC = int(args[4])

            if enableFEC :
                if (args[3] == 'RS') :
                    fecMode = eval('RS_FEC_MODE')
                elif (args[3] == 'FC') :
                    fecMode = eval('FC_FEC_MODE')
                elif (args[3] == 'RS_544_514'):
                    fecMode = eval('RS_544_514_FEC_MODE')
                else :
                    print('Invalid FECMode provided')
                    return
            else:
                fecMode = eval('MAX_FEC_MODE')



            ret = xpsMacPortInit(args[0],args[1],macMode,fecMode,enableFEC)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

