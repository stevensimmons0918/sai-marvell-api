#!/usr/bin/env python
#  xpShell.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

#/*********************************************************************************************************/
# import cmd2 package.
#/*********************************************************************************************************/
#from cmd import Cmd
import sys
import time
import os
import readline
import re
import getopt
import thread
import pwd
import grp
import stat

xp_root = os.getenv('XP_ROOT', "")


if not xp_root:
    dirname, filename = os.path.split(os.path.realpath(__file__))
    if not dirname.strip():
        dirname = './cli/'
    dirname += '/'

else:
    dirname = xp_root.rstrip('/') + "/cli/"

absDirname, filename = os.path.split(os.path.realpath(__file__))
sys.path.append(dirname)
sys.path.append(dirname + "../pipeline-profiles/xpDefault/cli/xps")
#path for saiImports.py
sys.path.append(dirname + "../plugins/sai/sai-src/saiShell/cli")
sys.path.append(dirname + "/platform")
sys.path.append(dirname + "/bdk")

from pkgImports import *
from xpsImports import *

if ("saiShell" in sys.modules.keys() or "ksaiShell" in sys.modules.keys()):
    from saiImports import *
else:
    pass

from LagHash import *
import xpShellGlobals
import enumDict
global redirout


#Used to assign and free Session-Id [Magic-Number]
ENTRY_POINT = 0xA0   #Used to assign the SessionId to Client when client wants to join the server and expect the new SessionId; Example:: SessionId + 0xA0
EXIT_POINT = 0xE0    #Used to free the assigned Session-Id when client wants to leave the Server; Example:: SessionID + 0xE0
NEGATIVE_FEEDBACK = 0xEE #Used to give the feedback to client when requested Session-Id is not available at Server-Side

#/*SessionId range is 0-7 & Total available Sessions are 8 */
MIN_CLIENT_ID = 0
MAX_CLIENT_ID = 7

#/*********************************************************************************************************/
# import everything from buildTarget
from buildTarget import *
from regAccess import *
from displayTables import *

xpDiagPath = dirname + "/../device/sdksrc/xp/xpDiag"
sdkPyFile = dirname + "/../dist/sdk.py"
RunningConfig = ' ' + dirname + '/runningConfig.txt'

global gquitexit
gquitexit = 0
global gexitmode
global shellmode
gexitmode = 0
isNetlinkEn = 0

# If xpDiag and svb are present in the filesystem, import corresponding python ifs


#/*********************************************************************************************************/
# Global FUnctions
#/*********************************************************************************************************/
def print_supported_clk_freq():
        print '  Core clock Frequency'
        print '========================'
        print 'XP_CORE_CLK_FREQ_400_MHZ'
        print 'XP_CORE_CLK_FREQ_500_MHZ'
        print 'XP_CORE_CLK_FREQ_550_MHZ'
        print 'XP_CORE_CLK_FREQ_600_MHZ'
        print 'XP_CORE_CLK_FREQ_633_MHZ'
        print 'XP_CORE_CLK_FREQ_650_MHZ'
        print 'XP_CORE_CLK_FREQ_683_MHZ'
        print 'XP_CORE_CLK_FREQ_700_MHZ'
        print 'XP_CORE_CLK_FREQ_733_MHZ'
        print 'XP_CORE_CLK_FREQ_750_MHZ'
        print 'XP_CORE_CLK_FREQ_783_MHZ'
        print 'XP_CORE_CLK_FREQ_800_MHZ'
        print 'XP_CORE_CLK_FREQ_833_MHZ'
        print 'XP_CORE_CLK_FREQ_850_MHZ'
        print 'XP_CORE_CLK_FREQ_900_MHZ'
        print 'XP_CORE_CLK_FREQ_950_MHZ'
        print 'XP_CORE_CLK_FREQ_1000_MHZ'
        return

def core_clk_freq_enum_to_Str(coreClkFreqEnum):
        clkFreqStr = 'Invalid value'
        if(coreClkFreqEnum == XP_CORE_CLK_FREQ_400_MHZ):
            clkFreqStr = '400 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_500_MHZ):
            clkFreqStr = '500 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_550_MHZ):
            clkFreqStr = '550 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_600_MHZ):
            clkFreqStr = '600 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_633_MHZ):
            clkFreqStr = '633 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_650_MHZ):
            clkFreqStr = '650 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_683_MHZ):
            clkFreqStr = '683 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_700_MHZ):
            clkFreqStr = '700 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_733_MHZ):
            clkFreqStr = '733 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_750_MHZ):
            clkFreqStr = '750 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_783_MHZ):
            clkFreqStr = '783 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_800_MHZ):
            clkFreqStr = '800 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_833_MHZ):
            clkFreqStr = '833 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_850_MHZ):
            clkFreqStr = '850 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_900_MHZ):
            clkFreqStr = '900 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_950_MHZ):
            clkFreqStr = '950 MHz'
        elif(coreClkFreqEnum == XP_CORE_CLK_FREQ_1000_MHZ):
            clkFreqStr = '1000 MHz'
        return clkFreqStr


#/*********************************************************************************************************/
# The class object for test scripts commands
#/*********************************************************************************************************/
class runTestScripts(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # cmd here
    #/*****************************************************************************************************/
    def dummy_cmd(self, arg):
        'dummy command till first command is added'

    #/*****************************************************************************************************/
    # Command which can go into different xps submodules and load a respective cfg file
    #/*****************************************************************************************************/
    def do_load_feature_config_files_100g(self, s):
        #TO_DO: We can modify this func later to expect cfg file list(with proper seq.) from user to make it more dynamic
        'To verify 100G Route In /Out of VxLAN to Geneve Tunnels. (Verifies VLAN , FDB, Tunnel Termination, Route and Tunnel Origination by loading vlan.cfg, l3intf.cfg, vxlanNexthop.cfg, vxlanTunnel.cfg, geneveTunnel.cfg, vlanTunnel.cfg, fdb.cfg, l3.cfg)'

        self.init_snake_load_config_files(s)
        self.l2_snake_load_config_files(s)
        self.l3_snake_load_config_files(s)
        self.do_enable_tunneling(s)

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsVlanObj()
        print "\nLoading vlan.cfg ..."
        obj.do_load(fName + "vlan.cfg")
        del obj

        obj = xpsL3Obj()
        print "\nLoading l3intf.cfg ..."
        obj.do_load(fName + "l3intf.cfg")
        print "\nLoading vxlanNexthop.cfg ..."
        obj.do_load(fName + "vxlanNexthop.cfg")
        del obj

        obj = xpsVxlanObj()
        print "\nLoading vxlanTunnel.cfg ..."
        obj.do_load(fName + "vxlanTunnel.cfg")
        del obj

        obj = xpsGeneveObj()
        print "\nLoading geneveTunnel.cfg ..."
        obj.do_load(fName + "geneveTunnel.cfg")
        del obj

        obj = xpsVlanObj()
        print "\nLoading vlanTunnel.cfg ..."
        obj.do_load(fName + "vlanTunnel.cfg")
        del obj

        obj = xpsFdbObj()
        print "\nLoading fdb.cfg ..."
        obj.do_load(fName + "fdb.cfg")
        del obj

        obj = xpsL3Obj()
        print "\nLoading l3.cfg ..."
        obj.do_load(fName + "l3.cfg")
        del obj

        obj = xpsTunnelObj()
        print "\nLoading tunnelenable.cfg ..."
        obj.do_load(fName + "tunnelenable.cfg")
        del obj

        obj = xpsPbbObj()
        print "\nLoading pbbenable.cfg ..."
        obj.do_load(fName + "pbbenable.cfg")
        del obj

    def do_load_feature_config_files_10g(self, s):
        'To verify 10G Route In /Out of VxLAN to Geneve Tunnels. (Verifies VLAN , FDB, Tunnel Termination, Route and Tunnel Origination by loading vlan.cfg, l3intf.cfg, vxlanNexthop_10G.cfg, vxlanTunnel_10G.cfg, geneveTunnel_10G.cfg, vlanTunnel.cfg, fdb.cfg, l3_10G.cfg)'

        self.init_snake_load_config_files_10g(s)
        self.l2_snake_load_config_files_10g(s)
        self.l3_snake_load_config_files_10g(s)
        self.do_enable_tunneling(s)

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsVlanObj()
        print "\nLoading vlan.cfg ..."
        obj.do_load(fName + "vlan.cfg")
        del obj

        obj = xpsL3Obj()
        print "\nLoading l3intf.cfg ..."
        obj.do_load(fName + "l3intf.cfg")
        print "\nLoading vxlanNexthop_10G.cfg ..."
        obj.do_load(fName + "vxlanNexthop_10G.cfg")
        del obj

        obj = xpsVxlanObj()
        print "\nLoading vxlanTunnel_10G.cfg ..."
        obj.do_load(fName + "vxlanTunnel_10G.cfg")
        del obj

        obj = xpsGeneveObj()
        print "\nLoading geneveTunnel_10G.cfg ..."
        obj.do_load(fName + "geneveTunnel_10G.cfg")
        del obj

        obj = xpsVlanObj()
        print "\nLoading vlanTunnel.cfg ..."
        obj.do_load(fName + "vlanTunnel.cfg")
        del obj

        obj = xpsFdbObj()
        print "\nLoading fdb.cfg ..."
        obj.do_load(fName + "fdb.cfg")
        del obj

        obj = xpsL3Obj()
        print "\nLoading l3_10G.cfg ..."
        obj.do_load(fName + "l3_10G.cfg")
        del obj

        obj = xpsTunnelObj()
        print "\nLoading tunnelenable.cfg ..."
        obj.do_load(fName + "tunnelenable.cfg")
        del obj

        obj = xpsPbbObj()
        print "\nLoading pbb.cfg ..."
        obj.do_load(fName + "pbb.cfg")
        del obj

    def do_load_snake_test_config (self, s):
        'Snake test configuration generated for SKU ID '
        args =  s.split()
        if  len(args) < 1:
            print 'Invalid input, Enter device-id, pci device id for SKU'
        else:
            pci = 0
            if len(args) > 1:
                pci = int(args[1], 16)
            print 'Input Arguments are devId=%d, pci=0x%x' % (int(args[0]), pci)
            ret = xpDiagSnakeTestGenerateConfigs(int(args[0]), pci)
            if ret != XP_NO_ERR :
                print 'Snake test config failed for devId=%d pci=0x%x' % (int(args[0]), pci)
            else:
                # Get the absolute path of configuration file
                fName = absDirname + "/cli/config/"
                if(os.path.isdir(absDirname+"/cli/config/")):
                    fName = absDirname + "/cli/config/"
                elif(os.path.isdir(absDirname+"/config/")):
                    fName = absDirname + "/config/"
                elif(os.path.isdir(absDirname + "/../../cli/config/")):
                    fName = absDirname+"/../../cli/config/"
                else:
                    print "Error: path to cli/config/ is not detected.Assuming " + fName

                obj = xpsVlanObj()
                print "\nLoading snake_pmap_l2vlan.txt ..."
                obj.do_load(fName + "snake_pmap_l2vlan.txt")
                del obj

                obj = xpsPortObj()
                print "\nLoading snake_pmap_l2port.txt ..."
                obj.do_load(fName + "snake_pmap_l2port.txt")
                del obj

                obj = xpsQosObj()
                print "\nLoading snake_pmap_l3qostrust.txt ..."
                obj.do_load(fName + "snake_pmap_l3qostrust.txt")
                del obj

                obj = xpsFdbObj()
                print "\nLoading snake_pmap_l2fdb.txt ..."
                obj.do_load(fName + "snake_pmap_l2fdb.txt")
                del obj

                obj = xpsL3Obj()
                print "\nLoading snake_pmap_l3vlan.txt ..."
                obj.do_load(fName + "snake_pmap_l3vlan.txt")
                del obj


    def do_enable_tunneling(self, s):
        'To Enable Tunneling'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsTunnelObj()
        print "\nLoading tunnelenable.cfg ..."
        obj.do_load(fName + "tunnelenable.cfg")
        del obj

    def do_disable_tunneling(self, s):
        'To Disable Tunneling'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsTunnelObj()
        print "\nLoading tunneldisable.cfg ..."
        obj.do_load(fName + "tunneldisable.cfg")
        del obj

    def init_snake_load_config_files(self, s):
        #TO_DO: We can modify this func later to expect cfg file list(with proper seq.) from user to make it more dynamic
        'To Initialize the 100G Snake Configuration'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsVlanObj()
        print "\nLoading vlanL2Snake.txt ..."
        obj.do_load(fName + "vlanL2Snake.txt")
        del obj

        obj = xpsPortObj()
        print "\nLoading portL2Snake.txt ..."
        obj.do_load(fName + "portL2Snake.txt")
        del obj

        obj = xpsQosObj()
        print "\nLoading trustl3.txt ..."
        obj.do_load(fName + "trustl3.txt")
        del obj

    def init_snake_load_config_files_10g(self, s):
        'To Initialize the 10G Snake Configuration'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsVlanObj()
        print "\nLoading vlanL2Snake_10G.txt ..."
        obj.do_load(fName + "vlanL2Snake_10G.txt")
        del obj

        obj = xpsPortObj()
        print "\nLoading portL2Snake_10G.txt ..."
        obj.do_load(fName + "portL2Snake_10G.txt")
        del obj

        obj = xpsQosObj()
        print "\nLoading trustl3_10G.txt ..."
        obj.do_load(fName + "trustl3_10G.txt")
        del obj
    def l2_snake_load_config_files(self, s):
        #TO_DO: We can modify this func later to expect cfg file list(with proper seq.) from user to make it more dynamic
        'To Verify 100G Vlan Based L2 Bridging Snake'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsFdbObj()
        print "\nLoading fdbL2Snake.txt ..."
        obj.do_load(fName + "fdbL2Snake.txt")
        del obj

    def l2_snake_load_config_files_10g(self, s):
        #TO_DO: We can modify this func later to expect cfg file list(with proper seq.) from user to make it more dynamic
        'To Verify 10G Vlan Based L2 Bridging Snake'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsFdbObj()
        print "\nLoading fdbL2Snake_10G.txt ..."
        obj.do_load(fName + "fdbL2Snake_10G.txt")
        del obj

    def l3_snake_load_config_files(self, s):
        #TO_DO: We can modify this func later to expect cfg file list(with proper seq.) from user to make it more dynamic
        'To Verify 100G Vlan Based L3 Routing Snake'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsL3Obj()
        print "\nLoading l3VlanSnake.txt ..."
        obj.do_load(fName + "l3VlanSnake.txt")
        del obj

    def l3_snake_load_config_files_10g(self, s):
        'To Verify 10G Vlan Based L3 Routing Snake'

        # Get the absolute path of configuration file
        fName = absDirname + "/cli/config/"
        if(os.path.isdir(absDirname+"/cli/config/")):
           fName = absDirname + "/cli/config/"
        elif(os.path.isdir(absDirname+"/config/")):
           fName = absDirname + "/config/"
        elif(os.path.isdir(absDirname + "/../../cli/config/")):
           fName = absDirname+"/../../cli/config/"
        else:
           print "Error: path to cli/config/ is not detected.Assuming " + fName

        obj = xpsL3Obj()
        print "\nLoading l3VlanSnake_10G.txt ..."
        obj.do_load(fName + "l3VlanSnake_10G.txt")
        del obj

#/*********************************************************************************************************/
# The class object for L2Domain commands
#/*********************************************************************************************************/
class runl2Domain(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # cmd here
    #/*****************************************************************************************************/
    def dummy_cmd(self, arg):
        'dummy command till first command is added'

#/*********************************************************************************************************/
# The class object for Serdes commands
#/*********************************************************************************************************/
class runSerdes(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # cmd here
    #/*****************************************************************************************************/
    def dummy_cmd(self, arg):
        'dummy command till first command is added'

#/*********************************************************************************************************/
# The class object for xpSim commands
#/*********************************************************************************************************/
class runXpSim(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # cmd here
    #/*****************************************************************************************************/
    def dummy_cmd(self, arg):
        'dummy command till first command is added'

#/*********************************************************************************************************/
# The class object for system commands
#/*********************************************************************************************************/
class runSystem(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command to get device type
    #/********************************************************************************/
    def do_get_device_type(self, arg):
        '''
         getDeviceType: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            devId = int(args[0])
            #print('Input Arguments are, devId=%d ' % (devId))
            deviceType = xpDeviceMgr.instance().getDeviceObj(devId).getDeviceType()
            devTypeName = 'Invalid Value'
            devSubTypeName = ''
            if(deviceType == XP80):
                devTypeName = 'XP80'
                devSubTypeName = 'XP80A0'
            elif(deviceType == XP70):
                devTypeName = 'XP70'
            elif(deviceType == XP70A1):
                devTypeName = 'XP70A1'
            elif(deviceType == XP70A2):
                devTypeName = 'XP70A2'
            elif(deviceType == XP80B0):
                devTypeName = 'XP80'
                deviceSubType = xpDeviceMgr.instance().getDeviceObj(devId).getMinorNum()
                if(deviceSubType == XP_DEVICE_SUB_TYPE_B0):
                    devSubTypeName = 'XP80B0'
                elif(deviceSubType == XP_DEVICE_SUB_TYPE_B1):
                    devSubTypeName = 'XP80B1'
            elif(deviceType == XPT):
                devTypeName = 'XPT'
            elif(deviceType == XP60):
                devTypeName = 'XP60'
            print 'Device Type : ', devTypeName
            if(devSubTypeName != ''):
                print 'Device Sub Type : ' , devSubTypeName

    #/*****************************************************************************************************/
    # command to get xp80 temp over sbus
    #/*****************************************************************************************************/
    def do_get_xp80_temp_over_sbus(self, arg):
        'Get xp80 temp over sbus, Enter Device-Id'
        #args = tuple(map(int, arg.split()))
        args = arg.split()
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id'
        else:
            deviceId = int(args[0])
            xp80Temp = new_floatp()
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(deviceId).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.readXp80TempOverSbus(deviceId, xp80Temp)
                if ret != XP_NO_ERR :
                    print "Failed with error code %d"%(ret)
                else:
                    print  "XP80 temp (over SBUS) : " + str(floatp_value(xp80Temp))
            else:
                print "Failed : Init mgmt block first"
            delete_floatp(xp80Temp)

    #/********************************************************************************/
    # command for sending config done
    #/********************************************************************************/
    def do_send_config_done(self, arg):
        '''
         send config done to WM: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            print('Sending config done to WM devId=%d' % (args[0]))
            xpAppIpcMsgConfigDoneSend(args[0])

    #/*****************************************************************************************************/
    # sub-commands for lag operation functions
    #/*****************************************************************************************************/
    def do_lag_hash(self, s):
        'set and get hash field operation commands'
        i = runHashFieldCmds()
        i.prompt = self.prompt[:-1]+':LagHash)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/********************************************************************************/
    # Auto completion for sal enumerations
    #/********************************************************************************/
    def complete_set_sal_type(self, text, line, begidx, endidx):
        tempDict = { 1 : 'salType'}
        return  enumDict.autoCompletion(line, text, tempDict)


    #/*****************************************************************************************************/
    # command to set SAL type
    #/*****************************************************************************************************/
    def do_set_sal_type(self, arg):
        '''
        set sal type, Enter SAL-Type'
        Valid values for salType : <salType>
        '''
        args = arg.split()
        if  len(args) < 1:
            print 'Invalid input, Enter SAL-Type'
        xpSetSalType(eval(args[0]))

    #/********************************************************************************/
    # command for getSerdesFwRevision
    #/********************************************************************************/
    def do_get_serdes_fw_revision(self, arg):
        '''
         getSerdesFwRevision: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            print('Input Arguments are, devId=%d' % (args[0]))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(args[0]).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.getSerdesFwRevision(args[0])
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    print('Done')
            else:
                print "Error : Init mgmt block first"

    #/*****************************************************************************************************/
    # command to read ipc debug messages from scpu
    #/*****************************************************************************************************/
    def do_read_ipc_debug(self, arg):
        'Read debug type IPC messages, Enter Device-Id, size'
        #args = tuple(map(int, arg.split()))
        args = arg.split()
        if ((len(args) == 2) and (int(args[1]) >= 0)):
            numMessages = int(args[1]) if (int(args[1]) <> 0) else int(1)
            msgCount = 1
            while (int(numMessages)):
                if (xpDeviceMgr.instance().isDeviceValid(int(args[0]))):
                    msg = " " * 2048
                    size = new_uint16_tp()
                    ret = XP_NO_ERR
                    ret = xpIpcRecvDebugMessage(int(args[0]), msg, size)
                    if ret == XP_NO_ERR:
                        print "************** Message (%d/%d) *********************" %(int(msgCount),int(args[1]))
                        print "Data bytes received : ",uint16_tp_value(size)
                        print "Data: ",msg.split("\x00")[0]
                        print "************** End of Message (%d/%d) **************\n" %(int(msgCount),int(args[1]))
                    else:
                        print "Failed with error code %d"%(ret)
                    delete_uint16_tp(size)
                else:
                    print ('Device %d not added' % int(args[0]))
                # Zero will read all messages
                if (int(args[1]) > 0):
                    numMessages = int(numMessages) - 1
                if(ret == XP_NO_ERR):
                    msgCount = int(msgCount) + 1
        else:
            print 'Invalid input, Enter Device-Id, numMessages(0 to read all msgs)'

    #/********************************************************************************/
    # command to get XP80 Core clock freq.
    #/********************************************************************************/
    def do_get_core_clk_freq(self, arg):
        '''
         getxp80CoreClkFrequency: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            devId = int(args[0])
            clkFreq =  new_xpCoreClkFreq_tp()
            print('Input Arguments are, devId=%d ' % (devId))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(devId).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.getCoreClkFrequency(devId, clkFreq)
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    enmFreq = xpCoreClkFreq_tp_value(clkFreq)
                    freq = core_clk_freq_enum_to_Str(enmFreq)
                    print ('XP Device %d core clock frequency : %s' % (devId, freq))
                    print('Done')
            else:
                print "Error : Mgmt block is not Initialized"

    #/********************************************************************************/
    # command for set XP80 Core clock freq.
    #/********************************************************************************/
    def do_set_core_clk_freq(self, arg):
        '''
         setCoreClkFrequency: Enter [ devId , coreClockFreq]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 2:
            print('Invalid input, Enter [ devId , coreClockFreq]')
            print_supported_clk_freq()
        else:
            devId = int(args[0])
            clkFreq = eval((args[1]))
            print('Input Arguments are, devId=%d and core clk freq : %s' % (devId, args[1]))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(devId).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.setCoreClkFrequency(devId, clkFreq)
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    print('Done')
            else:
                print "Error : Mgmt block is not Initialized"

    #/********************************************************************************/
    # command for set XP80 MAC clock Frequency.
    #/********************************************************************************/
    def do_set_mac_clk_freq(self, arg):
        '''
         setMacClkFrequency: Enter [ devId , macClockFreq]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 2:
            print('Invalid input, Enter [ devId , macClockFreq]')
            print 'MAC clock Frequency'
            print '===================='
            print 'XP_MAC_CLK_FREQ_800_MHZ'
            print 'XP_MAC_CLK_FREQ_833_MHZ'
            print 'XP_MAC_CLK_FREQ_883_MHZ'
        else:
            devId = int(args[0])
            clkFreq = eval((args[1]))
            print('Input Arguments are, devId=%d and MAC clk freq : %s' % (devId, args[1]))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(devId).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.setMacClkFrequency(devId, clkFreq)
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    print('Done')
            else:
                print "Error : Init mgmt block first"

    #/********************************************************************************/
    # command for getEfuseDataStatus
    #/********************************************************************************/
    def do_get_efuse_data_status(self, arg):
        '''
         getEfuseDataStatus: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            efuseData = new_arrUint32(4)
            print('Input Arguments are, devId=%d' % (args[0]))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(args[0]).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.getEfuseDataStatus(args[0], efuseData)
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    skewDict = { 0:'nom', 1:'', 2:'ss', 3:'ff', 4:'slow', 5:'rtran', 6:'ltran', 7:'fast'}
                    voltOffsetDict = { 0:'VDD', 1:'VDD-25mv', 2:'VDD-50mv', 3:'VDD-75mv', 4:'VDD-100mv', 5:'---', 6:'---', 7:'VDD+25mv'}
                    prmoCfgDict = { 0:'SVT', 1:'LVT', 2:'HVT', 3:'TBC'}

                    hex0 = arrUint32_getitem(efuseData, 0)
                    hex1 = arrUint32_getitem(efuseData, 1)
                    hex2 = arrUint32_getitem(efuseData, 2)
                    hex3 = arrUint32_getitem(efuseData, 3)

                    hex_no = (hex3 << 96) | (hex2 << 64) | (hex1 << 32) | hex0
                    #print '%032x' % (hex_no)

                    bin_size = 128
                    in_bin = bin(hex_no)[2:].zfill(bin_size)
                    rvs_bin = in_bin[::-1]
                    #print rvs_bin

                    skew_no = int(rvs_bin[:3], 2)
                    fab_code = int(rvs_bin[5:9], 2)
                    lot_type = int(rvs_bin[9], 2)
                    lotnum0 = int(rvs_bin[10:16] + '0', 2)
                    lotnum1 = int(rvs_bin[17:23] + '0', 2)
                    lotnum2 = int(rvs_bin[24:30] + '0', 2)
                    lotnum3 = int(rvs_bin[31:37] + '0', 2)
                    wafer_num = int(rvs_bin[38:43], 2)
                    x_sign = int(rvs_bin[43], 2)
                    x_coord = int(rvs_bin[44:51], 2)
                    y_sign = int(rvs_bin[51], 2)
                    y_coord = int(rvs_bin[52:59], 2)
                    volt_offset = int(rvs_bin[110:113][::-1], 2)
                    no_offset = int(rvs_bin[113], 2)
                    prmo_meas = rvs_bin[118:126][::-1]
                    prmo_cfg = int(rvs_bin[126:128][::-1], 2)

                    skew = skewDict[skew_no]

                    if lot_type == 1:
                            lottype = "prod"
                    else:
                            lottype = "engr"

                    lotnum = chr(lotnum0) + chr(lotnum1) + chr(lotnum2) + chr(lotnum3)

                    if x_sign == 1:
                            x_coord = x_coord * -1

                    if y_sign == 1:
                            y_coord = y_coord * -1

                    if no_offset == 1:
                            pack_volt = "VDD";
                    else:
                            pack_volt = voltOffsetDict[volt_offset]

                    prmo_config = prmoCfgDict[prmo_cfg]

                    print "*---------------------------------*"
                    print "LOT\tType\tSkew\tFab\tWafer\tX\tY\tVDD offset\tPRMO data\tPRMO config\tEfuse data (for reference)\n"
                    print "%s\t%s\t%s\t%d\t%d\t%d\t%d\t%s\t\t%s\t%s\t\t%032x\n" % (lotnum, lottype, skew, fab_code, wafer_num, x_coord, y_coord, pack_volt, prmo_meas, prmo_config, hex_no)
                    print "*---------------------------------*"
            else:
                print "Error : Init mgmt block first"
            delete_arrUint32(efuseData)

    #/********************************************************************************/
    # command for getAaplRevision
    #/********************************************************************************/
    def do_get_aapl_revision(self, arg):
        '''
         getAaplRevision: Enter [ deviceId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ deviceId ]')
        else:
            deviceId = int(args[0])
            print('Input Arguments are, devId=%d' % (deviceId))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(deviceId).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.getAaplRevision()
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    print('Done')
            else:
                print "Error : Init mgmt block first"

    #/*****************************************************************************************************/
    # print descriptor info.
    #/*****************************************************************************************************/
    def do_print_all_descriptors(self, arg):
        'Print descriptor information. Enter device-id, queue-num, dmaOperation(XP_DMA_TRANSMIT, XP_DMA_RECEIVE) ]'
        args = arg.split()
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, queue-num, dmaOperation(XP_DMA_TRANSMIT, XP_DMA_RECEIVE) ]'
        else:
            print 'Input Arguments are devId=%d, queue number=%d, isRxQueue=%d' % (int(args[0]), int(args[1]), int(args[2]))
            devId = int(args[0])
            queueNum = int(args[1])
            isRxQueue = eval(args[2])
            ret = xpPacketDriverPrintAllDescriptors(devId, queueNum, isRxQueue)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # print descriptor info of TxCDP.
    #/*****************************************************************************************************/
    def do_print_tx_cdp_desc_info(self, arg):
        'Print descriptor information. Enter device-id, queue-num'
        args = arg.split()
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, queue-num'
        else:
            print 'Input Arguments are devId=%d, queue number=%d' % (int(args[0]), int(args[1]))
            devId = int(args[0])
            queueNum = int(args[1])
            xpPacketDriverPrintTxCDPWithDescriptor(devId, queueNum)

    #/*****************************************************************************************************/
    # set XpHeaderExists in parser
    #/*****************************************************************************************************/
    def do_parser_set_xph_exists(self, arg):
        'Print scpu status. Enter the device id, port, enable'
        args = arg.split()
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, port, enable'
        else:
            print 'Input Arguments are devId=%d, port=%d, enable=%d' % (int(args[0]), int(args[1]), int(args[2]))
            devId = int(args[0])
            ret = xpInterfaceMgr.instance().parserMgrSetIktXpHeaderExists(devId, int(args[1]), int(args[2]))
            if (ret != XP_NO_ERR):
                print "-------------------------------------------------------"
                print "Failed with error code : %d"%(ret)
                print "-------------------------------------------------------"
            else:
                print "Done"

#/*********************************************************************************************************/
# The class object for ACM related commands
#/*********************************************************************************************************/
class runAcm(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to print ACM register counts if any
    #/*****************************************************************************************************/
    def do_print_acm_table(self, arg):
        '''
         Prints the Non zero ACM registers, if any; Input device-id
         Loops through to read all the ACM registers of the device and
         prints them if the value is non-zero
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintAcmRegListAttr(args[0])
            print('Done')

    #/********************************************************************************/
    # command for xpAcmInit
    #/********************************************************************************/
    def do_xp_acm_init(self, arg):
        '''
         xpAcmInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpAcmInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmDeInit
    #/********************************************************************************/
    def do_xp_acm_de_init(self, arg):
        '''
         xpAcmDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpAcmDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmAddDevice
    #/********************************************************************************/
    def do_xp_acm_add_device(self, arg):
        '''
         xpAcmAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpAcmAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmRemoveDevice
    #/********************************************************************************/
    def do_xp_acm_remove_device(self, arg):
        '''
         xpAcmRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpAcmRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpGetAcmBankMgrObj
    #/********************************************************************************/
    def do_xp_get_acm_bank_mgr_obj(self, arg):
        '''
         xpGetAcmBankMgrObj: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpGetAcmBankMgrObj()
    #/********************************************************************************/
    # command for xpGetAcmRsltMgrObj
    #/********************************************************************************/
    def do_xp_get_acm_rslt_mgr_obj(self, arg):
        '''
         xpGetAcmRsltMgrObj: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpGetAcmRsltMgrObj()
    #/********************************************************************************/
    # command for xpGetAcmCntMgrObj
    #/********************************************************************************/
    def do_xp_get_acm_cnt_mgr_obj(self, arg):
        '''
         xpGetAcmCntMgrObj: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpGetAcmCntMgrObj(args[0])

	#/********************************************************************************/
	# Auto completion for acmClient enumerations
	#/********************************************************************************/
    def complete_xp_acm_get_counter_value(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text , tempDict)

    #/********************************************************************************/
    # command for xpAcmGetCounterValue
    #/********************************************************************************/
    def do_xp_acm_get_counter_value(self, arg):
        '''
         xpAcmGetCounterValue: Enter [ devId,client,countIndex ]
		  Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,countIndex ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            nPkts_Ptr_3 = new_uint64_tp()
            nBytes_Ptr_4 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, client=%d, countIndex=%d' % (args[0],args[1],args[2]))
            ret = xpAcmGetCounterValue(args[0],args[1],args[2],nPkts_Ptr_3,nBytes_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nPkts = %d' % (uint64_tp_value(nPkts_Ptr_3)))
                print('nBytes = %d' % (uint64_tp_value(nBytes_Ptr_4)))
                pass
            delete_uint64_tp(nBytes_Ptr_4)
            delete_uint64_tp(nPkts_Ptr_3)

    #/*******************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_print_counter_value(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmPrintCounterValue
    #/********************************************************************************/
    def do_xp_acm_print_counter_value(self, arg):
        '''
         xpAcmPrintCounterValue: Enter [ devId,client,countIndex,printZero ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,countIndex,printZero ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            #print('Input Arguments are, devId=%d, client=%d, countIndex=%d, printZero=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpAcmPrintCounterValue(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/*******************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_get_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmGetSamplingConfig
    #/********************************************************************************/
    def do_xp_acm_get_sampling_config(self, arg):
        '''
         xpAcmGetSamplingConfig: Enter [ devId,sde,client,index ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            nSample_Ptr_4 = new_uint32_tp()
            mBase_Ptr_5 = new_uint32_tp()
            mExpo_Ptr_6 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, index=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpAcmGetSamplingConfig(args[0],args[1],args[2],args[3],nSample_Ptr_4,mBase_Ptr_5,mExpo_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nSample = %d' % (uint32_tp_value(nSample_Ptr_4)))
                print('mBase = %d' % (uint32_tp_value(mBase_Ptr_5)))
                print('mExpo = %d' % (uint32_tp_value(mExpo_Ptr_6)))
                pass
            delete_uint32_tp(mExpo_Ptr_6)
            delete_uint32_tp(mBase_Ptr_5)
            delete_uint32_tp(nSample_Ptr_4)

    #/*******************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_set_sampling_config_2(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmSetSamplingConfig_2
    #/********************************************************************************/
    def do_xp_acm_set_sampling_config_2(self, arg):
        '''
         xpAcmSetSamplingConfig_2: Enter [ devId,client,index,nSample,mBase,mExpo ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,nSample,mBase,mExpo ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, nSample=%d, mBase=%d, mExpo=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpAcmSetSamplingConfig_2(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_set_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmSetSamplingConfig
    #/********************************************************************************/
    def do_xp_acm_set_sampling_config(self, arg):
        '''
         xpAcmSetSamplingConfig: Enter [ devId,client,index,nSample,mBase,mExpo,sde ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,nSample,mBase,mExpo,sde ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, nSample=%d, mBase=%d, mExpo=%d, sde=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpAcmSetSamplingConfig(args[0],args[1],args[2],args[3],args[4],args[5],args[6])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_get_sampling_state(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmGetSamplingState
    #/********************************************************************************/
    def do_xp_acm_get_sampling_state(self, arg):
        '''
         xpAcmGetSamplingState: Enter [ devId,sde,client,index ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            totalCnt_Ptr_4 = new_uint64_tp()
            interEventCnt_Ptr_5 = new_uint32_tp()
            interSampleStart_Ptr_6 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, index=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpAcmGetSamplingState(args[0],args[1],args[2],args[3],totalCnt_Ptr_4,interEventCnt_Ptr_5,interSampleStart_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('totalCnt = %d' % (uint64_tp_value(totalCnt_Ptr_4)))
                print('interEventCnt = %d' % (uint32_tp_value(interEventCnt_Ptr_5)))
                print('interSampleStart = %d' % (uint32_tp_value(interSampleStart_Ptr_6)))
                pass
            delete_uint32_tp(interSampleStart_Ptr_6)
            delete_uint32_tp(interEventCnt_Ptr_5)
            delete_uint64_tp(totalCnt_Ptr_4)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_set_sampling_state_2(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmSetSamplingState_2
    #/********************************************************************************/
    def do_xp_acm_set_sampling_state_2(self, arg):
        '''
         xpAcmSetSamplingState_2: Enter [ devId,client,index,totalCnt,interEventCnt,interSampleStart ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,totalCnt,interEventCnt,interSampleStart ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, totalCnt=0x%x, interEventCnt=%d, interSampleStart=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpAcmSetSamplingState_2(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_set_sampling_state(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient' }
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmSetSamplingState
    #/********************************************************************************/
    def do_xp_acm_set_sampling_state(self, arg):
        '''
         xpAcmSetSamplingState: Enter [ devId,client,index,totalCnt,interEventCnt,interSampleStart,sde ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,totalCnt,interEventCnt,interSampleStart,sde ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, totalCnt=0x%x, interEventCnt=%d, interSampleStart=%d, sde=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpAcmSetSamplingState(args[0],args[1],args[2],args[3],args[4],args[5],args[6])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmSetGlobalConfigBucketization
    #/********************************************************************************/
    def do_xp_acm_set_global_config_bucketization(self, arg):
        '''
         xpAcmSetGlobalConfigBucketization: Enter [ devId,enable,startRange,endRange,numBkts,granularity,addAddr,bktUseAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,startRange,endRange,numBkts,granularity,addAddr,bktUseAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6], 16)
            args[7] = int(args[7], 16)
            #print('Input Arguments are, devId=%d, enable=%d, startRange=0x%x, endRange=0x%x, numBkts=%d, granularity=%d, addAddr=0x%x, bktUseAddr=0x%x' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]))
            ret = xpAcmSetGlobalConfigBucketization(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmSetGlobalConfigModePol
    #/********************************************************************************/
    def do_xp_acm_set_global_config_mode_pol(self, arg):
        '''
         xpAcmSetGlobalConfigModePol: Enter [ devId,refreshEnable,unitTime,refrTimeGranularity,updateWeight,billingCntrEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,refreshEnable,unitTime,refrTimeGranularity,updateWeight,billingCntrEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, refreshEnable=%d, unitTime=%d, refrTimeGranularity=%d, updateWeight=%d, billingCntrEnable=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpAcmSetGlobalConfigModePol(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_print_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmPrintSamplingConfig
    #/********************************************************************************/
    def do_xp_acm_print_sampling_config(self, arg):
        '''
         xpAcmPrintSamplingConfig: Enter [ devId,sde,client,index ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, index=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpAcmPrintSamplingConfig(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_print_sampling_state(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient' }
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmPrintSamplingState
    #/********************************************************************************/
    def do_xp_acm_print_sampling_state(self, arg):
        '''
         xpAcmPrintSamplingState: Enter [ devId,sde,client,index ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, index=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpAcmPrintSamplingState(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_display_acm_bank_table(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient' }
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpDisplayAcmBankTable
    #/********************************************************************************/
    def do_xp_display_acm_bank_table(self, arg):
        '''
         xpDisplayAcmBankTable: Enter [ devId,sde,client,type,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 9
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,type,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = eval(args[2])
            args[3] = int(args[3])
            numOfValidEntries_Ptr_4 = new_uint32_tp()
            args[4] = int(args[4])
            args[5] = int(args[5])
            logFile_Ptr_7 = new_charp()
            args[6] = int(args[6])
            args[7] = int(args[7])
            args[8] = int(args[8])
            #print('Input Arguments are, devId=%d, sde=0x%x, client=%d, type=%d, startIndex=%d, endIndex=%d, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0],args[1],args[2],args[3],args[5],args[6],args[8],args[9],args[10]))
            ret = xpDisplayAcmBankTable(args[0],args[1],args[2],args[3],numOfValidEntries_Ptr_4,args[4],args[5],logFile_Ptr_7,args[6],args[7],args[8])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_4)))
                print('logFile = %d' % (charp_value(logFile_Ptr_7)))
                pass
            delete_charp(logFile_Ptr_7)
            delete_uint32_tp(numOfValidEntries_Ptr_4)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_set_acm_bank_range_by_client(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, 'acmClient')

    #/********************************************************************************/
    # command for xpSetAcmBankRangeByClient
    #/********************************************************************************/
    def do_xp_set_acm_bank_range_by_client(self, arg):
        '''
         xpSetAcmBankRangeByClient: Enter [ devId,client,type,sde,bankStart,bankEnd ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type,sde,bankStart,bankEnd ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, type=%d, sde=0x%x, bankStart=%d, bankEnd=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpSetAcmBankRangeByClient(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpSetAcmBankRangeByClientInterface
    #/********************************************************************************/
    def do_xp_set_acm_bank_range_by_client_interface(self, arg):
        '''
         xpSetAcmBankRangeByClientInterface: Enter [ devId,profile,clientIntf,sde,bankStart,bankEnd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profile,clientIntf,sde,bankStart,bankEnd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, profile=0x%x, clientIntf=%d, sde=0x%x, bankStart=%d, bankEnd=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpSetAcmBankRangeByClientInterface(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpGetAcmBankRange
    #/********************************************************************************/
    def do_xp_get_acm_bank_range(self, arg):
        '''
         xpGetAcmBankRange: Enter [ devId,sde,profile,clientIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,profile,clientIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            bankStart_Ptr_4 = new_uint32_tp()
            bankEnd_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, sde=%d, profile=%d, clientIntf=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpGetAcmBankRange(args[0],args[1],args[2],args[3],bankStart_Ptr_4,bankEnd_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bankStart = %d' % (uint32_tp_value(bankStart_Ptr_4)))
                print('bankEnd = %d' % (uint32_tp_value(bankEnd_Ptr_5)))
                pass
            delete_uint32_tp(bankEnd_Ptr_5)
            delete_uint32_tp(bankStart_Ptr_4)
    #/********************************************************************************/
    # command for xpAcmGetClientInterfaceEnableByClientIntfId
    #/********************************************************************************/
    def do_xp_acm_get_client_interface_enable_by_client_intf_id(self, arg):
        '''
         xpAcmGetClientInterfaceEnableByClientIntfId: Enter [ devId,sde,clientIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,clientIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, sde=0x%x, clientIntf=%d' % (args[0],args[1],args[2]))
            ret = xpAcmGetClientInterfaceEnableByClientIntfId(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_get_acm_client_interface_mask(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpGetAcmClientInterfaceMask
    #/********************************************************************************/
    def do_xp_get_acm_client_interface_mask(self, arg):
        '''
         xpGetAcmClientInterfaceMask: Enter [ devId,client,type ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            clientIntfMask_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, client=%d, type=%d' % (args[0],args[1],args[2]))
            ret = xpGetAcmClientInterfaceMask(args[0],args[1],args[2],clientIntfMask_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('clientIntfMask = %d' % (uint64_tp_value(clientIntfMask_Ptr_3)))
                pass
            delete_uint64_tp(clientIntfMask_Ptr_3)
    #/********************************************************************************/
    # command for xpDisplayAcmRsltTable
    #/********************************************************************************/
    def do_xp_display_acm_rslt_table(self, arg):
        '''
         xpDisplayAcmRsltTable: Enter [ devId,startIndex,endIndex,detailFormat,silentMode ]
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
            ret = xpDisplayAcmRsltTable(args[0],numOfValidEntries_Ptr_1,args[2],args[3],logFile_Ptr_4,args[5],args[6])
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

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_set_client_interface_enable_by_client(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text , tempDict)

    #/********************************************************************************/
    # command for xpAcmSetClientInterfaceEnableByClient
    #/********************************************************************************/
    def do_xp_acm_set_client_interface_enable_by_client(self, arg):
        '''
         xpAcmSetClientInterfaceEnableByClient: Enter [ devId,client,type,enable ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, client=%d, type=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpAcmSetClientInterfaceEnableByClient(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmSetClientInterfaceEnableByClientIntfId
    #/********************************************************************************/
    def do_xp_acm_set_client_interface_enable_by_client_intf_id(self, arg):
        '''
         xpAcmSetClientInterfaceEnableByClientIntfId: Enter [ devId,clientIntf,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,clientIntf,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, clientIntf=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpAcmSetClientInterfaceEnableByClientIntfId(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpAcmSetAllClientInterfaceEnable
    #/********************************************************************************/
    def do_xp_acm_set_all_client_interface_enable(self, arg):
        '''
         xpAcmSetAllClientInterfaceEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpAcmSetAllClientInterfaceEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_acm_xp_set_count_mode(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAcmXpSetCountMode
    #/********************************************************************************/
    def do_xp_acm_xp_set_count_mode(self, arg):
        '''
         xpAcmXpSetCountMode: Enter [ devId,client,type,bankMode,clearOnRead,wrapAround,countOffset ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type,bankMode,clearOnRead,wrapAround,countOffset ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4], 16)
            args[5] = int(args[5], 16)
            args[6] = int(args[6], 16)
            #print('Input Arguments are, devId=%d, client=%d, type=%d, bankMode=0x%x, clearOnRead=0x%x, wrapAround=0x%x, countOffset=0x%x' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpAcmXpSetCountMode(args[0],args[1],args[2],args[3],args[4],args[5],args[6])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpPolicerInit
    #/********************************************************************************/
    def do_xp_policer_init(self, arg):
        '''
         xpPolicerInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpPolicerInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerDeInit
    #/********************************************************************************/
    def do_xp_policer_de_init(self, arg):
        '''
         xpPolicerDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpPolicerDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerAddDevice
    #/********************************************************************************/
    def do_xp_policer_add_device(self, arg):
        '''
         xpPolicerAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpPolicerAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerRemoveDevice
    #/********************************************************************************/
    def do_xp_policer_remove_device(self, arg):
        '''
         xpPolicerRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpPolicerRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerEnablePortPolicing
    #/********************************************************************************/
    def do_xp_policer_enable_port_policing(self, arg):
        '''
         xpPolicerEnablePortPolicing: Enter [ devId,portId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpPolicerEnablePortPolicing(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerGetPortPolicingStatus
    #/********************************************************************************/
    def do_xp_policer_get_port_policing_status(self, arg):
        '''
         xpPolicerGetPortPolicingStatus: Enter [ devId,portId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portId=%d' % (args[0],args[1]))
            ret = xpPolicerGetPortPolicingStatus(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_add_policer_entry_2(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAddPolicerEntry_2
    #/********************************************************************************/
    def do_xp_add_policer_entry_2(self, arg):
        '''
         xpAddPolicerEntry_2: Enter [ devId,client,index,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 19
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            xpPolicerEntry_t_Ptr = new_xpPolicerEntry_tp()
            xpPolicerEntry_t_Ptr.cbsBase = int(args[3])
            xpPolicerEntry_t_Ptr.cbsExpo = int(args[4])
            xpPolicerEntry_t_Ptr.pbsBase = int(args[5])
            xpPolicerEntry_t_Ptr.pbsExpo = int(args[6])
            xpPolicerEntry_t_Ptr.cirBase = int(args[7])
            xpPolicerEntry_t_Ptr.cirExpo = int(args[8])
            xpPolicerEntry_t_Ptr.cirTimeGranExpo = int(args[9])
            xpPolicerEntry_t_Ptr.pirBase = int(args[10])
            xpPolicerEntry_t_Ptr.pirExpo = int(args[11])
            xpPolicerEntry_t_Ptr.pirTimeGranExpo = int(args[12])
            xpPolicerEntry_t_Ptr.colorAware = int(args[13])
            xpPolicerEntry_t_Ptr.dropRed = int(args[14])
            xpPolicerEntry_t_Ptr.dropYellow = int(args[15])
            xpPolicerEntry_t_Ptr.remarkGreen = int(args[16])
            xpPolicerEntry_t_Ptr.remarkYellow = int(args[17])
            xpPolicerEntry_t_Ptr.remarkRed = int(args[18])
            xpPolicerEntry_t_Ptr.polResult = int(args[19])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, cbsBase=%d, cbsExpo=%d, pbsBase=%d, pbsExpo=%d, cirBase=%d, cirExpo=%d, cirTimeGranExpo=%d, pirBase=%d, pirExpo=%d, pirTimeGranExpo=%d, colorAware=%d, dropRed=%d, dropYellow=%d, remarkGreen=%d, remarkYellow=%d, remarkRed=%d, polResult=%d' % (args[0],args[1],args[2],xpPolicerEntry_t_Ptr.cbsBase,xpPolicerEntry_t_Ptr.cbsExpo,xpPolicerEntry_t_Ptr.pbsBase,xpPolicerEntry_t_Ptr.pbsExpo,xpPolicerEntry_t_Ptr.cirBase,xpPolicerEntry_t_Ptr.cirExpo,xpPolicerEntry_t_Ptr.cirTimeGranExpo,xpPolicerEntry_t_Ptr.pirBase,xpPolicerEntry_t_Ptr.pirExpo,xpPolicerEntry_t_Ptr.pirTimeGranExpo,xpPolicerEntry_t_Ptr.colorAware,xpPolicerEntry_t_Ptr.dropRed,xpPolicerEntry_t_Ptr.dropYellow,xpPolicerEntry_t_Ptr.remarkGreen,xpPolicerEntry_t_Ptr.remarkYellow,xpPolicerEntry_t_Ptr.remarkRed,xpPolicerEntry_t_Ptr.polResult))
            ret = xpAddPolicerEntry_2(args[0],args[1],args[2],xpPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cbsBase = %d' % (xpPolicerEntry_t_Ptr.cbsBase))
                print('cbsExpo = %d' % (xpPolicerEntry_t_Ptr.cbsExpo))
                print('pbsBase = %d' % (xpPolicerEntry_t_Ptr.pbsBase))
                print('pbsExpo = %d' % (xpPolicerEntry_t_Ptr.pbsExpo))
                print('cirBase = %d' % (xpPolicerEntry_t_Ptr.cirBase))
                print('cirExpo = %d' % (xpPolicerEntry_t_Ptr.cirExpo))
                print('cirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.cirTimeGranExpo))
                print('pirBase = %d' % (xpPolicerEntry_t_Ptr.pirBase))
                print('pirExpo = %d' % (xpPolicerEntry_t_Ptr.pirExpo))
                print('pirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.pirTimeGranExpo))
                print('colorAware = %d' % (xpPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpPolicerEntry_t_Ptr.dropYellow))
                print('remarkGreen = %d' % (xpPolicerEntry_t_Ptr.remarkGreen))
                print('remarkYellow = %d' % (xpPolicerEntry_t_Ptr.remarkYellow))
                print('remarkRed = %d' % (xpPolicerEntry_t_Ptr.remarkRed))
                print('polResult = %d' % (xpPolicerEntry_t_Ptr.polResult))
                pass
            delete_xpPolicerEntry_tp(xpPolicerEntry_t_Ptr)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_add_policer_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAddPolicerEntry
    #/********************************************************************************/
    def do_xp_add_policer_entry(self, arg):
        '''
         xpAddPolicerEntry: Enter [ devId,client,index,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult,sde ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 20
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult,sde ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            xpPolicerEntry_t_Ptr = new_xpPolicerEntry_tp()
            xpPolicerEntry_t_Ptr.cbsBase = int(args[3])
            xpPolicerEntry_t_Ptr.cbsExpo = int(args[4])
            xpPolicerEntry_t_Ptr.pbsBase = int(args[5])
            xpPolicerEntry_t_Ptr.pbsExpo = int(args[6])
            xpPolicerEntry_t_Ptr.cirBase = int(args[7])
            xpPolicerEntry_t_Ptr.cirExpo = int(args[8])
            xpPolicerEntry_t_Ptr.cirTimeGranExpo = int(args[9])
            xpPolicerEntry_t_Ptr.pirBase = int(args[10])
            xpPolicerEntry_t_Ptr.pirExpo = int(args[11])
            xpPolicerEntry_t_Ptr.pirTimeGranExpo = int(args[12])
            xpPolicerEntry_t_Ptr.colorAware = int(args[13])
            xpPolicerEntry_t_Ptr.dropRed = int(args[14])
            xpPolicerEntry_t_Ptr.dropYellow = int(args[15])
            xpPolicerEntry_t_Ptr.remarkGreen = int(args[16])
            xpPolicerEntry_t_Ptr.remarkYellow = int(args[17])
            xpPolicerEntry_t_Ptr.remarkRed = int(args[18])
            xpPolicerEntry_t_Ptr.polResult = int(args[19])
            args[20] = int(args[20])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, cbsBase=%d, cbsExpo=%d, pbsBase=%d, pbsExpo=%d, cirBase=%d, cirExpo=%d, cirTimeGranExpo=%d, pirBase=%d, pirExpo=%d, pirTimeGranExpo=%d, colorAware=%d, dropRed=%d, dropYellow=%d, remarkGreen=%d, remarkYellow=%d, remarkRed=%d, polResult=%d, sde=%d' % (args[0],args[1],args[2],xpPolicerEntry_t_Ptr.cbsBase,xpPolicerEntry_t_Ptr.cbsExpo,xpPolicerEntry_t_Ptr.pbsBase,xpPolicerEntry_t_Ptr.pbsExpo,xpPolicerEntry_t_Ptr.cirBase,xpPolicerEntry_t_Ptr.cirExpo,xpPolicerEntry_t_Ptr.cirTimeGranExpo,xpPolicerEntry_t_Ptr.pirBase,xpPolicerEntry_t_Ptr.pirExpo,xpPolicerEntry_t_Ptr.pirTimeGranExpo,xpPolicerEntry_t_Ptr.colorAware,xpPolicerEntry_t_Ptr.dropRed,xpPolicerEntry_t_Ptr.dropYellow,xpPolicerEntry_t_Ptr.remarkGreen,xpPolicerEntry_t_Ptr.remarkYellow,xpPolicerEntry_t_Ptr.remarkRed,xpPolicerEntry_t_Ptr.polResult,args[20]))
            ret = xpAddPolicerEntry(args[0],args[1],args[2],xpPolicerEntry_t_Ptr,args[20])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cbsBase = %d' % (xpPolicerEntry_t_Ptr.cbsBase))
                print('cbsExpo = %d' % (xpPolicerEntry_t_Ptr.cbsExpo))
                print('pbsBase = %d' % (xpPolicerEntry_t_Ptr.pbsBase))
                print('pbsExpo = %d' % (xpPolicerEntry_t_Ptr.pbsExpo))
                print('cirBase = %d' % (xpPolicerEntry_t_Ptr.cirBase))
                print('cirExpo = %d' % (xpPolicerEntry_t_Ptr.cirExpo))
                print('cirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.cirTimeGranExpo))
                print('pirBase = %d' % (xpPolicerEntry_t_Ptr.pirBase))
                print('pirExpo = %d' % (xpPolicerEntry_t_Ptr.pirExpo))
                print('pirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.pirTimeGranExpo))
                print('colorAware = %d' % (xpPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpPolicerEntry_t_Ptr.dropYellow))
                print('remarkGreen = %d' % (xpPolicerEntry_t_Ptr.remarkGreen))
                print('remarkYellow = %d' % (xpPolicerEntry_t_Ptr.remarkYellow))
                print('remarkRed = %d' % (xpPolicerEntry_t_Ptr.remarkRed))
                print('polResult = %d' % (xpPolicerEntry_t_Ptr.polResult))
                pass
            delete_xpPolicerEntry_tp(xpPolicerEntry_t_Ptr)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_remove_policer_entry_2(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpRemovePolicerEntry_2
    #/********************************************************************************/
    def do_xp_remove_policer_entry_2(self, arg):
        '''
         xpRemovePolicerEntry_2: Enter [ devId,client,index ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpRemovePolicerEntry_2(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_remove_policer_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpRemovePolicerEntry
    #/********************************************************************************/
    def do_xp_remove_policer_entry(self, arg):
        '''
         xpRemovePolicerEntry: Enter [ devId,client,index,sde ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,sde ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, sde=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpRemovePolicerEntry(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_get_policer_entry(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpGetPolicerEntry
    #/********************************************************************************/
    def do_xp_get_policer_entry(self, arg):
        '''
         xpGetPolicerEntry: Enter [ devId,sde,client,index,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 20
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,index,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpPolicerEntry_t_Ptr = new_xpPolicerEntry_tp()
            xpPolicerEntry_t_Ptr.cbsBase = int(args[4])
            xpPolicerEntry_t_Ptr.cbsExpo = int(args[5])
            xpPolicerEntry_t_Ptr.pbsBase = int(args[6])
            xpPolicerEntry_t_Ptr.pbsExpo = int(args[7])
            xpPolicerEntry_t_Ptr.cirBase = int(args[8])
            xpPolicerEntry_t_Ptr.cirExpo = int(args[9])
            xpPolicerEntry_t_Ptr.cirTimeGranExpo = int(args[10])
            xpPolicerEntry_t_Ptr.pirBase = int(args[11])
            xpPolicerEntry_t_Ptr.pirExpo = int(args[12])
            xpPolicerEntry_t_Ptr.pirTimeGranExpo = int(args[13])
            xpPolicerEntry_t_Ptr.colorAware = int(args[14])
            xpPolicerEntry_t_Ptr.dropRed = int(args[15])
            xpPolicerEntry_t_Ptr.dropYellow = int(args[16])
            xpPolicerEntry_t_Ptr.remarkGreen = int(args[17])
            xpPolicerEntry_t_Ptr.remarkYellow = int(args[18])
            xpPolicerEntry_t_Ptr.remarkRed = int(args[19])
            xpPolicerEntry_t_Ptr.polResult = int(args[20])
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, index=%d, cbsBase=%d, cbsExpo=%d, pbsBase=%d, pbsExpo=%d, cirBase=%d, cirExpo=%d, cirTimeGranExpo=%d, pirBase=%d, pirExpo=%d, pirTimeGranExpo=%d, colorAware=%d, dropRed=%d, dropYellow=%d, remarkGreen=%d, remarkYellow=%d, remarkRed=%d, polResult=%d' % (args[0],args[1],args[2],args[3],xpPolicerEntry_t_Ptr.cbsBase,xpPolicerEntry_t_Ptr.cbsExpo,xpPolicerEntry_t_Ptr.pbsBase,xpPolicerEntry_t_Ptr.pbsExpo,xpPolicerEntry_t_Ptr.cirBase,xpPolicerEntry_t_Ptr.cirExpo,xpPolicerEntry_t_Ptr.cirTimeGranExpo,xpPolicerEntry_t_Ptr.pirBase,xpPolicerEntry_t_Ptr.pirExpo,xpPolicerEntry_t_Ptr.pirTimeGranExpo,xpPolicerEntry_t_Ptr.colorAware,xpPolicerEntry_t_Ptr.dropRed,xpPolicerEntry_t_Ptr.dropYellow,xpPolicerEntry_t_Ptr.remarkGreen,xpPolicerEntry_t_Ptr.remarkYellow,xpPolicerEntry_t_Ptr.remarkRed,xpPolicerEntry_t_Ptr.polResult))
            ret = xpGetPolicerEntry(args[0],args[1],args[2],args[3],xpPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cbsBase = %d' % (xpPolicerEntry_t_Ptr.cbsBase))
                print('cbsExpo = %d' % (xpPolicerEntry_t_Ptr.cbsExpo))
                print('pbsBase = %d' % (xpPolicerEntry_t_Ptr.pbsBase))
                print('pbsExpo = %d' % (xpPolicerEntry_t_Ptr.pbsExpo))
                print('cirBase = %d' % (xpPolicerEntry_t_Ptr.cirBase))
                print('cirExpo = %d' % (xpPolicerEntry_t_Ptr.cirExpo))
                print('cirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.cirTimeGranExpo))
                print('pirBase = %d' % (xpPolicerEntry_t_Ptr.pirBase))
                print('pirExpo = %d' % (xpPolicerEntry_t_Ptr.pirExpo))
                print('pirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.pirTimeGranExpo))
                print('colorAware = %d' % (xpPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpPolicerEntry_t_Ptr.dropYellow))
                print('remarkGreen = %d' % (xpPolicerEntry_t_Ptr.remarkGreen))
                print('remarkYellow = %d' % (xpPolicerEntry_t_Ptr.remarkYellow))
                print('remarkRed = %d' % (xpPolicerEntry_t_Ptr.remarkRed))
                print('polResult = %d' % (xpPolicerEntry_t_Ptr.polResult))
                pass
            delete_xpPolicerEntry_tp(xpPolicerEntry_t_Ptr)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_set_policer_attribute_2(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpSetPolicerAttribute_2
    #/********************************************************************************/
    def do_xp_set_policer_attribute_2(self, arg):
        '''
         xpSetPolicerAttribute_2: Enter [ devId,client,index,field ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,field ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            data_Ptr_4 = new_voidp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d, field=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpSetPolicerAttribute_2(args[0],args[1],args[2],args[3],data_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_4)))
                pass
            delete_voidp(data_Ptr_4)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_set_policer_attribute(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpSetPolicerAttribute
    #/********************************************************************************/
    def do_xp_set_policer_attribute(self, arg):
        '''
         xpSetPolicerAttribute: Enter [ devId,client,index,field,sde ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,field,sde ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            data_Ptr_4 = new_voidp()
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, field=%d, sde=%d' % (args[0],args[1],args[2],args[3],args[5]))
            ret = xpSetPolicerAttribute(args[0],args[1],args[2],args[3],data_Ptr_4,args[5])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_4)))
                pass
            delete_voidp(data_Ptr_4)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_get_policer_attribute(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return  enumDict.autoCompletion(line, text , tempDict)

    #/********************************************************************************/
    # command for xpGetPolicerAttribute
    #/********************************************************************************/
    def do_xp_get_policer_attribute(self, arg):
        '''
         xpGetPolicerAttribute: Enter [ devId,sde,client,index,field ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,index,field ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            args[4] = eval(args[4])
            data_Ptr_5 = new_voidp()
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, index=%d, field=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpGetPolicerAttribute(args[0],args[1],args[2],args[3],args[4],data_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_5)))
                pass
            delete_voidp(data_Ptr_5)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_add_port_policer_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpAddPortPolicerEntry
    #/********************************************************************************/
    def do_xp_add_port_policer_entry(self, arg):
        '''
         xpAddPortPolicerEntry: Enter [ devId,client,portId,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 19
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,portId,cbsBase,cbsExpo,pbsBase,pbsExpo,cirBase,cirExpo,cirTimeGranExpo,pirBase,pirExpo,pirTimeGranExpo,colorAware,dropRed,dropYellow,remarkGreen,remarkYellow,remarkRed,polResult ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            xpPolicerEntry_t_Ptr = new_xpPolicerEntry_tp()
            xpPolicerEntry_t_Ptr.cbsBase = int(args[3])
            xpPolicerEntry_t_Ptr.cbsExpo = int(args[4])
            xpPolicerEntry_t_Ptr.pbsBase = int(args[5])
            xpPolicerEntry_t_Ptr.pbsExpo = int(args[6])
            xpPolicerEntry_t_Ptr.cirBase = int(args[7])
            xpPolicerEntry_t_Ptr.cirExpo = int(args[8])
            xpPolicerEntry_t_Ptr.cirTimeGranExpo = int(args[9])
            xpPolicerEntry_t_Ptr.pirBase = int(args[10])
            xpPolicerEntry_t_Ptr.pirExpo = int(args[11])
            xpPolicerEntry_t_Ptr.pirTimeGranExpo = int(args[12])
            xpPolicerEntry_t_Ptr.colorAware = int(args[13])
            xpPolicerEntry_t_Ptr.dropRed = int(args[14])
            xpPolicerEntry_t_Ptr.dropYellow = int(args[15])
            xpPolicerEntry_t_Ptr.remarkGreen = int(args[16])
            xpPolicerEntry_t_Ptr.remarkYellow = int(args[17])
            xpPolicerEntry_t_Ptr.remarkRed = int(args[18])
            xpPolicerEntry_t_Ptr.polResult = int(args[19])
            #print('Input Arguments are, devId=%d, client=%d, portId=%d, cbsBase=%d, cbsExpo=%d, pbsBase=%d, pbsExpo=%d, cirBase=%d, cirExpo=%d, cirTimeGranExpo=%d, pirBase=%d, pirExpo=%d, pirTimeGranExpo=%d, colorAware=%d, dropRed=%d, dropYellow=%d, remarkGreen=%d, remarkYellow=%d, remarkRed=%d, polResult=%d' % (args[0],args[1],args[2],xpPolicerEntry_t_Ptr.cbsBase,xpPolicerEntry_t_Ptr.cbsExpo,xpPolicerEntry_t_Ptr.pbsBase,xpPolicerEntry_t_Ptr.pbsExpo,xpPolicerEntry_t_Ptr.cirBase,xpPolicerEntry_t_Ptr.cirExpo,xpPolicerEntry_t_Ptr.cirTimeGranExpo,xpPolicerEntry_t_Ptr.pirBase,xpPolicerEntry_t_Ptr.pirExpo,xpPolicerEntry_t_Ptr.pirTimeGranExpo,xpPolicerEntry_t_Ptr.colorAware,xpPolicerEntry_t_Ptr.dropRed,xpPolicerEntry_t_Ptr.dropYellow,xpPolicerEntry_t_Ptr.remarkGreen,xpPolicerEntry_t_Ptr.remarkYellow,xpPolicerEntry_t_Ptr.remarkRed,xpPolicerEntry_t_Ptr.polResult))
            ret = xpAddPortPolicerEntry(args[0],args[1],args[2],xpPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cbsBase = %d' % (xpPolicerEntry_t_Ptr.cbsBase))
                print('cbsExpo = %d' % (xpPolicerEntry_t_Ptr.cbsExpo))
                print('pbsBase = %d' % (xpPolicerEntry_t_Ptr.pbsBase))
                print('pbsExpo = %d' % (xpPolicerEntry_t_Ptr.pbsExpo))
                print('cirBase = %d' % (xpPolicerEntry_t_Ptr.cirBase))
                print('cirExpo = %d' % (xpPolicerEntry_t_Ptr.cirExpo))
                print('cirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.cirTimeGranExpo))
                print('pirBase = %d' % (xpPolicerEntry_t_Ptr.pirBase))
                print('pirExpo = %d' % (xpPolicerEntry_t_Ptr.pirExpo))
                print('pirTimeGranExpo = %d' % (xpPolicerEntry_t_Ptr.pirTimeGranExpo))
                print('colorAware = %d' % (xpPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpPolicerEntry_t_Ptr.dropYellow))
                print('remarkGreen = %d' % (xpPolicerEntry_t_Ptr.remarkGreen))
                print('remarkYellow = %d' % (xpPolicerEntry_t_Ptr.remarkYellow))
                print('remarkRed = %d' % (xpPolicerEntry_t_Ptr.remarkRed))
                print('polResult = %d' % (xpPolicerEntry_t_Ptr.polResult))
                pass
            delete_xpPolicerEntry_tp(xpPolicerEntry_t_Ptr)

    #*********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_xp_remove_port_policer_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpRemovePortPolicerEntry
    #/********************************************************************************/
    def do_xp_remove_port_policer_entry(self, arg):
        '''
         xpRemovePortPolicerEntry: Enter [ devId,client,portId ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,portId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, client=%d, portId=%d' % (args[0],args[1],args[2]))
            ret = xpRemovePortPolicerEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerSetPolicingResultByType
    #/********************************************************************************/
    def do_xp_policer_set_policing_result_by_type(self, arg):
        '''
         xpPolicerSetPolicingResultByType: Enter [ devId,paramIndex,resultType,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,paramIndex,resultType,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xpPolicerResult_t_Ptr = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr.dp = int(args[3])
            xpPolicerResult_t_Ptr.tc = int(args[4])
            xpPolicerResult_t_Ptr.pcp = int(args[5])
            xpPolicerResult_t_Ptr.dei = int(args[6])
            xpPolicerResult_t_Ptr.dscp = int(args[7])
            xpPolicerResult_t_Ptr.exp = int(args[8])
            xpPolicerResult_t_Ptr = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr.dp = int(args[9])
            xpPolicerResult_t_Ptr.tc = int(args[10])
            #print('Input Arguments are, devId=%d, paramIndex=%d, resultType=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d' % (args[0],args[1],args[2],xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp,xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp,xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp))
            ret = xpPolicerSetPolicingResultByType_2(args[0],args[1],args[2],xpPolicerResult_t_Ptr,xpPolicerResult_t_Ptr,xpPolicerResult_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                pass
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
    #/********************************************************************************/
    # command for xpPolicerGetPolicingResultByType
    #/********************************************************************************/
    def do_xp_policer_get_policing_result_by_type(self, arg):
        '''
         xpPolicerGetPolicingResultByType: Enter [ devId,paramIndex,resultType,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,paramIndex,resultType,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xpPolicerResult_t_Ptr = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr.dp = int(args[3])
            xpPolicerResult_t_Ptr.tc = int(args[4])
            xpPolicerResult_t_Ptr.pcp = int(args[5])
            xpPolicerResult_t_Ptr.dei = int(args[6])
            xpPolicerResult_t_Ptr.dscp = int(args[7])
            xpPolicerResult_t_Ptr.exp = int(args[8])
            xpPolicerResult_t_Ptr = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr.dp = int(args[9])
            xpPolicerResult_t_Ptr.tc = int(args[10])
            #print('Input Arguments are, devId=%d, paramIndex=%d, resultType=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d' % (args[0],args[1],args[2],xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp,xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp,xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp))
            ret = xpPolicerGetPolicingResultByType_2(args[0],args[1],args[2],xpPolicerResult_t_Ptr,xpPolicerResult_t_Ptr,xpPolicerResult_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                pass
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
    #/********************************************************************************/
    # command for xpPolicerSetPolicingResultByColor
    #/********************************************************************************/
    def do_xp_policer_set_policing_result_by_color(self, arg):
        '''
         xpPolicerSetPolicingResultByColor: Enter [ devId,paramIndex,resultType,color,dp,tc,pcp,dei,dscp,exp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 9
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,paramIndex,resultType,color,dp,tc,pcp,dei,dscp,exp ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            xpPolicerResult_t_Ptr = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr.dp = int(args[4])
            xpPolicerResult_t_Ptr.tc = int(args[5])
            xpPolicerResult_t_Ptr.pcp = int(args[6])
            xpPolicerResult_t_Ptr.dei = int(args[7])
            xpPolicerResult_t_Ptr.dscp = int(args[8])
            xpPolicerResult_t_Ptr.exp = int(args[9])
            #print('Input Arguments are, devId=%d, paramIndex=%d, resultType=%d, color=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d' % (args[0],args[1],args[2],args[3],xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp))
            ret = xpPolicerSetPolicingResultByColor_2(args[0],args[1],args[2],args[3],xpPolicerResult_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                pass
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)
    #/********************************************************************************/
    # command for xpPolicerEnablePolicingCounter
    #/********************************************************************************/
    def do_xp_policer_enable_policing_counter(self, arg):
        '''
         xpPolicerEnablePolicingCounter: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpPolicerEnablePolicingCounter(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpPolicerSetGlobalUnitTime
    #/********************************************************************************/
    def do_xp_policer_set_global_unit_time(self, arg):
        '''
         xpPolicerSetGlobalUnitTime: Enter [ devId,unitTime ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,unitTime ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, unitTime=%d' % (args[0],args[1]))
            ret = xpPolicerSetGlobalUnitTime(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass


#/**********************************************************************************/
# The class object for xpSflow operations
#/**********************************************************************************/

class xpSflowObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpSflowInit
    #/********************************************************************************/
    def do_xp_sflow_init(self, arg):
        '''
         xpSflowInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpSflowInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpSflowDeInit
    #/********************************************************************************/
    def do_xp_sflow_de_init(self, arg):
        '''
         xpSflowDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpSflowDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpSflowAddDevice
    #/********************************************************************************/
    def do_xp_sflow_add_device(self, arg):
        '''
         xpSflowAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpSflowAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpSflowRemoveDevice
    #/********************************************************************************/
    def do_xp_sflow_remove_device(self, arg):
        '''
         xpSflowRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpSflowRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpSetSflowMirrorEnable
    #/********************************************************************************/
    def do_xp_set_sflow_mirror_enable(self, arg):
        '''
         xpSetSflowMirrorEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpSetSflowMirrorEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpSflowSetPortSamplingConfig
    #/********************************************************************************/
    def do_xp_sflow_set_port_sampling_config(self, arg):
        '''
         xpSflowSetPortSamplingConfig: Enter [ devId,client,portId,nSample,mBase,mExpo ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,portId,nSample,mBase,mExpo ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, portId=%d, nSample=%d, mBase=%d, mExpo=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpSflowSetPortSamplingConfig(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpSflowGetPortSamplingConfig
    #/********************************************************************************/
    def do_xp_sflow_get_port_sampling_config(self, arg):
        '''
         xpSflowGetPortSamplingConfig: Enter [ devId,sde,client,portId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,portId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            nSample_Ptr_4 = new_uint32_tp()
            mBase_Ptr_5 = new_uint32_tp()
            mExpo_Ptr_6 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, sde=%d, client=%d, portId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpSflowGetPortSamplingConfig(args[0],args[1],args[2],args[3],nSample_Ptr_4,mBase_Ptr_5,mExpo_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nSample = %d' % (uint32_tp_value(nSample_Ptr_4)))
                print('mBase = %d' % (uint32_tp_value(mBase_Ptr_5)))
                print('mExpo = %d' % (uint32_tp_value(mExpo_Ptr_6)))
                pass
            delete_uint32_tp(mExpo_Ptr_6)
            delete_uint32_tp(mBase_Ptr_5)
            delete_uint32_tp(nSample_Ptr_4)
    #/********************************************************************************/
    # command for xpSflowEnablePortSampling
    #/********************************************************************************/
    def do_xp_sflow_enable_port_sampling(self, arg):
        '''
         xpSflowEnablePortSampling: Enter [ devId,portId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpSflowEnablePortSampling(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpSflowGetPortSamplingEn
    #/********************************************************************************/
    def do_xp_sflow_get_port_sampling_en(self, arg):
        '''
         xpSflowGetPortSamplingEn: Enter [ devId,portId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portId=%d' % (args[0],args[1]))
            ret = xpSflowGetPortSamplingEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpSetSflowPktCmd
    #/********************************************************************************/
    def do_xp_set_sflow_pkt_cmd(self, arg):
        '''
         xpSetSflowPktCmd: Enter [ devId,pktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, pktCmd=%d' % (args[0],args[1]))
            ret = xpSetSflowPktCmd(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpGetSflowPktCmd
    #/********************************************************************************/
    def do_xp_get_sflow_pkt_cmd(self, arg):
        '''
         xpGetSflowPktCmd: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            pktCmd_Ptr_1 = new_xpPktCmd_ep()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpGetSflowPktCmd(args[0],pktCmd_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpPktCmd_ep_value(pktCmd_Ptr_1)))
                pass
            delete_xpPktCmd_ep(pktCmd_Ptr_1)
    #/********************************************************************************/
    # command for xpSetSflowEgressVif
    #/********************************************************************************/
    def do_xp_set_sflow_egress_vif(self, arg):
        '''
         xpSetSflowEgressVif: Enter [ devId,eVif ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,eVif ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, eVif=%d' % (args[0],args[1]))
            ret = xpSetSflowEgressVif(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpGetSflowEgressVif
    #/********************************************************************************/
    def do_xp_get_sflow_egress_vif(self, arg):
        '''
         xpGetSflowEgressVif: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            eVif_Ptr_1 = new_xpVif_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpGetSflowEgressVif(args[0],eVif_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('eVif = %d' % (xpVif_tp_value(eVif_Ptr_1)))
                pass
            delete_xpVif_tp(eVif_Ptr_1)

    #/********************************************************************************/
    # command for xpSflowDisplayAcmBankTable
    #/********************************************************************************/
    def do_xp_sflow_display_acm_bank_table(self, arg):
        '''
         xpSflowDisplayAcmBankTable: Enter [ devId,sde,client,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sde,client,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = eval(args[2])
            numOfValidEntries_Ptr_3 = new_uint32_tp()
            args[4] = int(args[4])
            args[5] = int(args[5])
            logFile_Ptr_6 = new_charp()
            args[7] = int(args[7])
            args[8] = int(args[8])
            args[9] = int(args[9])
            #print('Input Arguments are, devId=%d, sde=0x%x, client=%d, startIndex=%d, endIndex=%d, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0],args[1],args[2],args[4],args[5],args[7],args[8],args[9]))
            ret = xpSflowDisplayAcmBankTable(args[0],args[1],args[2],numOfValidEntries_Ptr_3,args[4],args[5],logFile_Ptr_6,args[7],args[8],args[9])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_3)))
                print('logFile = %d' % (charp_value(logFile_Ptr_6)))
                pass
            delete_charp(logFile_Ptr_6)
            delete_uint32_tp(numOfValidEntries_Ptr_3)
    #/********************************************************************************/
    # command for xpSflowDisplayMdtTable
    #/********************************************************************************/
    def do_xp_sflow_display_mdt_table(self, arg):
        '''
         xpSflowDisplayMdtTable: Enter [ devId,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            numOfValidEntries_Ptr_1 = new_uint32_tp()
            args[2] = int(args[2])
            args[3] = int(args[3])
            logFile_Ptr_4 = new_charp()
            args[5] = int(args[5])
            args[6] = int(args[6])
            args[7] = int(args[7])
            #print('Input Arguments are, devId=%d, startIndex=%d, endIndex=%d, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0],args[2],args[3],args[5],args[6],args[7]))
            ret = xpSflowDisplayMdtTable(args[0],numOfValidEntries_Ptr_1,args[2],args[3],logFile_Ptr_4,args[5],args[6],args[7])
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
    # command for xpSflowDisplayPortCfgTable
    #/********************************************************************************/
    def do_xp_sflow_display_port_cfg_table(self, arg):
        '''
         xpSflowDisplayPortCfgTable: Enter [ devId,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,startIndex,endIndex,detailFormat,silentMode,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            numOfValidEntries_Ptr_1 = new_uint32_tp()
            args[2] = int(args[2])
            args[3] = int(args[3])
            logFile_Ptr_4 = new_charp()
            args[5] = int(args[5])
            args[6] = int(args[6])
            args[7] = int(args[7])
            #print('Input Arguments are, devId=%d, startIndex=%d, endIndex=%d, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0],args[2],args[3],args[5],args[6],args[7]))
            ret = xpSflowDisplayPortCfgTable(args[0],numOfValidEntries_Ptr_1,args[2],args[3],logFile_Ptr_4,args[5],args[6],args[7])
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

#/*********************************************************************************************************/
# The class object for Diags commands
#/*********************************************************************************************************/
class runDiags(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # cmd here
    #/*****************************************************************************************************/
    def dummy_cmd(self, arg):
        'dummy command till first command is added'

#/*********************************************************************************************************/
# The class object for Data Path debug commands
#/*********************************************************************************************************/
class runDataPathDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to data path debug information
    #/*****************************************************************************************************/
    def do_get_data_path_debug_info(self, arg):
        '''
        Get the data path block debug information. Input device-id
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetDataPathDebugInfo(args[0])
            print('Done')

#/*********************************************************************************************************/
# The class object for Parser debug commands
#/*********************************************************************************************************/
class runParserDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to parser block debug information
    #/*****************************************************************************************************/
    def do_get_parser_debug_info(self, arg):
        '''
        Get the parser block debug information. Input device-id
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetParserDebugInfo(args[0])
            print('Done')
        return

    #/********************************************************************************/
    # command for getHdrSzInPtg
    #/********************************************************************************/
    def do_get_hdr_sz_in_ptg(self, arg):
        '''
         getHdrSzInPtg: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            hdrSz_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getDpMgr().getHdrSzInPtg(args[0],args[1],hdrSz_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('hdrSz = %d' % (uint32_tp_value(hdrSz_Ptr_2)))
                pass
            delete_uint32_tp(hdrSz_Ptr_2)

    #/********************************************************************************/
    # command for getCutThroughMode
    #/********************************************************************************/
    def do_get_cut_through_mode(self, arg):
        '''
         getCutThroughMode: Enter [ deviceId,portNumber ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,portNumber ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            cutThrough_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, portNumber=%d' % (args[0],args[1]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getParser().getCutThroughMode(args[0],args[1],cutThrough_Ptr_2)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('cutThrough = %d' % (uint32_tp_value(cutThrough_Ptr_2)))
                pass
            delete_uint32_tp(cutThrough_Ptr_2)

    #/********************************************************************************/
    # command for setCutThroughMode
    #/********************************************************************************/
    def do_set_cut_through_mode(self, arg):
        '''
         setCutThroughMode: Enter [ deviceId,portNumber,enable(1)/disbale(0) ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,portNumber,enable(1)/disbale(0) ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, portNumber=%d, enable=%d' % (args[0],args[1], args[2]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getParser().setCutThroughMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('cutThrough = %d' % args[2])
                pass

    #/********************************************************************************/
    # command for setHdrSzInPtg
    #/********************************************************************************/
    def do_set_hdr_sz_in_ptg(self, arg):
        '''
         setHdrSzInPtg: Enter [ devId,portNum,hdrSz ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,hdrSz ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, hdrSz=%d' % (args[0],args[1],args[2]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getDpMgr().setHdrSzInPtg(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # command to enable parser block debug information
    #/*****************************************************************************************************/
    def do_enable_parser_debug_info(self, arg):
        '''
        Enable the parser block debug information. Input device-id, enable-1 or 0
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, enable (1 or 0)'
        else:
            print('Input Arguments are devId=%d, %s ' % (args[0], 'enabled' if args[1] is not 0 else 'disabled'))
            xpEnableParserDebugInfo(args[0], args[1])
            print('Done')
        return

#/*********************************************************************************************************/
# The class object for MRE debug commands
#/*********************************************************************************************************/
class runMreDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to MRE debug information
    #/*****************************************************************************************************/
    def do_get_mre_debug_info(self, arg):
        '''
        Get the MRE debug information. Input device-id
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetMreDebugInfo(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to map traffic class with MRE queue
    #/*****************************************************************************************************/
    def do_map_traffic_class_to_mre_queue(self, arg):
        '''
        Map traffic class to MRE queue. Input device-id, trafficClass, queue
        Valid ranges:
            devId: 0-63
            traffficClass: 0-3
            queue: 0-3
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, trafficClass, queue'
        else:
            print('Input Arguments are devId=%d trafficClass = %d, queue = %d' % (args[0], args[1], args[2]))
            xpMapTrafficClassToMreQueue(args[0], args[1], args[2])
            print('Done')

    #/*****************************************************************************************************/
    # command to configure MRE queue threshold
    #/*****************************************************************************************************/
    def do_config_mre_queue_threshold(self, arg):
        '''
        Config MRE queue threshold. Input device-id, queue, threshold
        Valid ranges:
            devId: 0-63
            queue: 0-3
            thresold: 0-511
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, queue,threshold'
        else:
            print('Input Arguments are devId=%d queue = %d, threshold = %d' % (args[0], args[1], args[2]))
            xpConfigMreQueueThreshold(args[0], args[1], args[2])
            print('Done')

    #/*****************************************************************************************************/
    # command to configure MRE queue bank threshold
    #/*****************************************************************************************************/
    def do_config_mre_queue_bank_threshold(self, arg):
        '''
        Config MRE queue bank threshold. Input device-id, threshold.
        Valid ranges:
            devId: 0-63
            thresold: 0-511
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, threshold'
        else:
            print('Input Arguments are devId=%d threshold = %d' % (args[0], args[1]))
            xpConfigMreQueueBankThreshold(args[0], args[1])
            print('Done')

    #/*****************************************************************************************************/
    # command to configure MRE queue WRR weight
    #/*****************************************************************************************************/
    def do_config_mre_queue_wrr_weight(self, arg):
        '''
        Config MRE queue WRR weight. Input device-id, queue, weight.
        Valid ranges:
            devId: 0-63
            queue: 0-3
            weight: 0-511
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, queue, weight'
        else:
            print('Input Arguments are devId=%d queue = %d weight = %d' % (args[0], args[1], args[2]))
            xpConfigMreQueueWrrWeight(args[0], args[1], args[2])
            print('Done')

    #/*****************************************************************************************************/
    # command to configure MRE queue WRR bypass
    #/*****************************************************************************************************/
    def do_config_mre_queue_wrr_bypass(self, arg):
        '''
        Config MRE queue WRR bypass. Input device-id, queue, enable.
        Valid ranges:
            devId: 0-63
            queue: 0-3
            enable: 1 = enable, 0 = disable
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, queue, enable'
        else:
            print('Input Arguments are devId=%d queue = %d enable = %d' % (args[0], args[1], args[2]))
            xpConfigMreQueueWrrBypass(args[0], args[1], args[2])
            print('Done')


#/*********************************************************************************************************/
# The class object for LDE debug commands
#/*********************************************************************************************************/
class runLdeDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to print LDE debug information
    #/*****************************************************************************************************/
    def do_get_lde_debug_info(self, arg):
        '''
        Get the LDE block debug information. Input device-id,ldeId
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id , ldeId'
        else:
            print('Input Arguments are devId=%d, lde = %d' % (args[0], args[1]))
            xpGetLdeDebugInfo(args[0], args[1])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to enable lde block debug information
    #/*****************************************************************************************************/
    def do_enable_lde_debug_info(self, arg):
        '''
        Enable the lde block debug information. Input device-id, enable-1 or 0
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, enable (1 or 0)'
        else:
            print('Input Arguments are devId=%d, %s ' % (args[0], 'enabled' if args[1] is not 0 else 'disabled'))
            xpEnableLdeDebugInfo(args[0], args[1])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to get lde token dump
    #/*****************************************************************************************************/
    def do_get_lde_token_dump(self, arg):
        '''
        Get the LDE token dump. Input device-id,ldeId
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id , ldeId'
        else:
            print('Input Arguments are devId=%d, lde = %d' % (args[0], args[1]))
            xpGetLdeTokenDump(args[0], args[1])
            print('Done')
        return

#/*********************************************************************************************************/
# The class object for SE debug commands
#/*********************************************************************************************************/
class runSeDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to print SE debug info
    #/*****************************************************************************************************/
    def do_se_debug_info(self, arg):
        '''
        Get the SE block debug information.
        Enter devId,  profileId, key, reqId, cmdEn
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 5:
            print 'Invalid input, Enter devId,  profileId, key, reqId, cmdEn'
        else:
            print('Input Arguments are devId=%d, profileId=%d, key=%d, reqId=%d, cmdEn=%d' % (args[0],
                                                                    args[1], args[2], args[3], args[4]))
            xpGetSeDebugInfo(args[0], args[1], args[2], args[3], args[4])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to enable se block debug information
    #/*****************************************************************************************************/
    def do_enable_se_debug_info(self, arg):
        '''
        Enable the se block debug information. Input device-id, enable-1 or 0
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, enable (1 or 0)'
        else:
            print('Input Arguments are devId=%d, %s ' % (args[0], 'enabled' if args[1] is not 0 else 'disabled'))
            xpEnableSeDebugInfo(args[0], args[1])
            print('Done')
        return

#/*********************************************************************************************************/
# The class object for URW debug commands
#/*********************************************************************************************************/
class runUrwDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to print URW debug information
    #/*****************************************************************************************************/
    def do_get_urw_debug_info(self, arg):
        '''
        Get the URW debug information. Input device-id and cmdEn
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, cmdEn'
        else:
            print('Input Arguments are devId=%d, cmdEn=%d' % (args[0], args[1]))
            xpGetUrwDebugInfo(args[0], args[1])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to enable urw block debug information
    #/*****************************************************************************************************/
    def do_enable_urw_debug_info(self, arg):
        '''
        Enable the urw block debug information. Input device-id, enable-1 or 0
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, enable (1 or 0)'
        else:
            print('Input Arguments are devId=%d, %s ' % (args[0], 'enabled' if args[1] is not 0 else 'disabled'))
            xpEnableUrwDebugInfo(args[0], args[1])
            print('Done')
        return

#/*********************************************************************************************************/
# The class object for IBUFFER debug commands
#/*********************************************************************************************************/
class runIbufferDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpDebugGetIbufferDropCounterBmFull
    #/********************************************************************************/
    def do_get_ibuffer_drop_counter_bm_full(self, arg):
        '''
         xpDebugGetIbufferDropCounterBmFull: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferDropCounterBmFull(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferDropCounterBufFull
    #/********************************************************************************/
    def do_get_ibuffer_drop_counter_buf_full(self, arg):
        '''
         xpDebugGetIbufferDropCounterBufFull: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferDropCounterBufFull(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferTermCounterBmFull
    #/********************************************************************************/
    def do_get_ibuffer_term_counter_bm_full(self, arg):
        '''
         xpDebugGetIbufferTermCounterBmFull: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferTermCounterBmFull(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferTermCounterBufFull
    #/********************************************************************************/
    def do_get_ibuffer_term_counter_buf_full(self, arg):
        '''
         xpDebugGetIbufferTermCounterBufFull: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferTermCounterBufFull(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferDropCounterMisc
    #/********************************************************************************/
    def do_get_ibuffer_drop_counter_misc(self, arg):
        '''
         xpDebugGetIbufferDropCounterMisc: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferDropCounterMisc(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferFifoOccupancy
    #/********************************************************************************/
    def do_get_ibuffer_fifo_occupancy(self, arg):
        '''
         xpDebugGetIbufferFifoOccupancy: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferFifoOccupancy(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferFifoOccupancyLossless
    #/********************************************************************************/
    def do_get_ibuffer_fifo_occupancy_lossless(self, arg):
        '''
         xpDebugGetIbufferFifoOccupancyLossless: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            count_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpDebugGetIbufferFifoOccupancyLossless(args[0],args[1],count_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_2)))
                pass
            delete_uint64_tp(count_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetIbufferFifoOccupancyLossy
    #/********************************************************************************/
    def do_get_ibuffer_fifo_occupancy_lossy(self, arg):
        '''
         xpDebugGetIbufferFifoOccupancyLossy: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            count_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpDebugGetIbufferFifoOccupancyLossy(args[0],args[1],count_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_2)))
                pass
            delete_uint64_tp(count_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetIbufferPfcEventCount
    #/********************************************************************************/
    def do_get_ibuffer_pfc_event_count(self, arg):
        '''
         xpDebugGetIbufferPfcEventCount: Enter [ devId,portNum,priority ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,priority ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            count_Ptr_3 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d, priority=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetIbufferPfcEventCount(args[0],args[1],args[2],count_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_3)))
                pass
            delete_uint64_tp(count_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetIbufferPauseEventCount
    #/********************************************************************************/
    def do_get_ibuffer_pause_event_count(self, arg):
        '''
         xpDebugGetIbufferPauseEventCount: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            count_Ptr_2 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpDebugGetIbufferPauseEventCount(args[0],args[1],count_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint64_tp_value(count_Ptr_2)))
                pass
            delete_uint64_tp(count_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugEnableTerminationPerPortPriority
    #/********************************************************************************/
    def do_enable_termination_per_port_priority(self, arg):
        '''
         xpDebugEnableTerminationPerPortPriority: Enter [ devId,portNum,priority,enable ]
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
            ret = xpDebugEnableTerminationPerPortPriority(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpDebugGetTerminationEnablePerPortPriority
    #/********************************************************************************/
    def do_get_termination_enable_per_port_priority(self, arg):
        '''
         xpDebugGetTerminationEnablePerPortPriority: Enter [ devId,portNum,priority ]
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
            ret = xpDebugGetTerminationEnablePerPortPriority(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
#/*********************************************************************************************************/
# The class object for PFC debug commands
#/*********************************************************************************************************/
class runPfcDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpDebugGetPfcDynamicPortTcXonThreshold
    #/********************************************************************************/
    def do_get_pfc_dynamic_port_tc_xon_threshold(self, arg):
        '''
         xpDebugGetPfcDynamicPortTcXonThreshold: Enter [ devId,devPort,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xonThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetPfcDynamicPortTcXonThreshold(args[0],args[1],args[2],xonThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xonThreshold = %d' % (uint32_tp_value(xonThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xonThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetPfcDynamicPortTcXoffThreshold
    #/********************************************************************************/
    def do_get_pfc_dynamic_port_tc_xoff_threshold(self, arg):
        '''
         xpDebugGetPfcDynamicPortTcXoffThreshold: Enter [ devId,devPort,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xoffThreshold_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetPfcDynamicPortTcXoffThreshold(args[0],args[1],args[2],xoffThreshold_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('xoffThreshold = %d' % (uint32_tp_value(xoffThreshold_Ptr_3)))
                pass
            delete_uint32_tp(xoffThreshold_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetPfcGlobalCounter
    #/********************************************************************************/
    def do_get_pfc_global_counter(self, arg):
        '''
         xpDebugGetPfcGlobalCounter: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            counter_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpDebugGetPfcGlobalCounter(args[0],counter_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_1)))
                pass
            delete_uint32_tp(counter_Ptr_1)
    #/********************************************************************************/
    # command for xpDebugGetPfcPortCounter
    #/********************************************************************************/
    def do_get_pfc_port_counter(self, arg):
        '''
         xpDebugGetPfcPortCounter: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpDebugGetPfcPortCounter(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetPfcPgCounter
    #/********************************************************************************/
    def do_get_pfc_pg_counter(self, arg):
        '''
         xpDebugGetPfcPgCounter: Enter [ devId,portGroupId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portGroupId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portGroupId=%d' % (args[0],args[1]))
            ret = xpDebugGetPfcPgCounter(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetPfcTcCounter
    #/********************************************************************************/
    def do_get_pfc_tc_counter(self, arg):
        '''
         xpDebugGetPfcTcCounter: Enter [ devId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, trafficClass=%d' % (args[0],args[1]))
            ret = xpDebugGetPfcTcCounter(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetPfcPortTcCounter
    #/********************************************************************************/
    def do_get_pfc_port_tc_counter(self, arg):
        '''
         xpDebugGetPfcPortTcCounter: Enter [ devId,devPort,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            counter_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetPfcPortTcCounter(args[0],args[1],args[2],counter_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_3)))
                pass
            delete_uint32_tp(counter_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetPfcPgTcCounter
    #/********************************************************************************/
    def do_get_pfc_pg_tc_counter(self, arg):
        '''
         xpDebugGetPfcPgTcCounter: Enter [ devId,portGroupId,trafficClass ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portGroupId,trafficClass ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            counter_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portGroupId=%d, trafficClass=%d' % (args[0],args[1],args[2]))
            ret = xpDebugGetPfcPgTcCounter(args[0],args[1],args[2],counter_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_3)))
                pass
            delete_uint32_tp(counter_Ptr_3)
    #/********************************************************************************/
    # command for xpDebugGetFcGlobalCounter
    #/********************************************************************************/
    def do_get_fc_global_counter(self, arg):
        '''
         xpDebugGetFcGlobalCounter: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            counter_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpDebugGetFcGlobalCounter(args[0],counter_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_1)))
                pass
            delete_uint32_tp(counter_Ptr_1)
    #/********************************************************************************/
    # command for xpDebugGetFcPgCounter
    #/********************************************************************************/
    def do_get_fc_pg_counter(self, arg):
        '''
         xpDebugGetFcPgCounter: Enter [ devId,portGroupId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portGroupId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portGroupId=%d' % (args[0],args[1]))
            ret = xpDebugGetFcPgCounter(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetFcPortCounter
    #/********************************************************************************/
    def do_get_fc_port_counter(self, arg):
        '''
         xpDebugGetFcPortCounter: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counter_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpDebugGetFcPortCounter(args[0],args[1],counter_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_2)))
                pass
            delete_uint32_tp(counter_Ptr_2)
    #/********************************************************************************/
    # command for xpDebugGetFcPoolCounter
    #/********************************************************************************/
    def do_get_fc_pool_counter(self, arg):
        '''
         xpDebugGetFcPoolCounter: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            counter_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpDebugGetFcPoolCounter(args[0],counter_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counter = %d' % (uint32_tp_value(counter_Ptr_1)))
                pass
            delete_uint32_tp(counter_Ptr_1)
#/*********************************************************************************************************/
# The class object for Txq debug commands
#/*********************************************************************************************************/
class runTxqDebug(Cmd):
    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to dump the last aqm query for a pipe
    #/*****************************************************************************************************/
    def do_get_last_aqm_query(self, arg):
        '''
        Get the last AQM Query made in Txq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-id'
        else:
            xpDebugGetAqmLastQuery(args[0], args[1])
        return

    #/*****************************************************************************************************/
    # command to dump the last aqm reply
    #/*****************************************************************************************************/
    def do_get_last_aqm_reply(self, arg):
        '''
        Get the last AQM Reply made in Txq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-id'
        else:
            xpDebugGetAqmLastReply(args[0], args[1])
        return

    #/*****************************************************************************************************/
    # command to dump the last qmap index hit
    #/*****************************************************************************************************/
    def do_get_last_qmap_index(self, arg):
        '''
        Get the last QMAP Query result in Txq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-id'
        else:
            xpDebugGetQmapLastIndex(args[0], args[1])
        return

    #/*****************************************************************************************************/
    # command to dump the last eq input token for a pipe
    #/*****************************************************************************************************/
    def do_get_last_eq_input_token(self, arg):
        '''
        Get the last EQ Input Token in Txq for a pipe
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-id'
        else:
            xpDebugGetEqLastInputToken(args[0], args[1])
        return

    #/*****************************************************************************************************/
    # command to print the queue to port path
    #/*****************************************************************************************************/
    def do_display_queue_to_port_path(self, arg):
        '''
        Display the path from queue to port
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter device-id, port-num, queue-num'
        else:
            xpDebugDisplayQueueToPortPath(args[0], args[1], args[2])
        return
    #/********************************************************************************/
    # command for ptf mode update
    #/********************************************************************************/
    def do_ptg_mode_update(self, arg):
        '''
         xpDebugPtgModeUpdate: Enter [ devId,ptgNum,numChannels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,ptgNum,numChannels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, ptgNum=%d, numChannels=%d' % (args[0],args[1],args[2]))
            ret = xpDebugPtgModeUpdate(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpDebugDisplayPortCalendar
    #/********************************************************************************/
    def do_display_port_calendar(self, arg):
        '''
         xpDebugDisplayPortCalendar: Enter [ devId,pipeNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pipeNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, pipeNum=%d' % (args[0],args[1]))
            ret = xpDebugDisplayPortCalendar(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/*****************************************************************************************************/
    # command to get TxQ AQM drop reasons mask
    #/*****************************************************************************************************/
    def do_get_aqm_drop_reason_mask(self, arg):
        '''
        Get the AQM drop reasons mask
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq):
            print 'Invalid input, Enter [device-id]'
        else:
            args[0] = int(args[0])
            ret = xpDebugGetTxqAqmDropReasonMask(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # command to set TxQ AQM drop reasons mask
    #/*****************************************************************************************************/
    def do_set_aqm_drop_reason_mask(self, arg):
        '''
        Set the TxQ AQM drop reasons mask
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq):
            print 'Invalid input, Enter [device-id, drop-type, mask]'
            print '   drop-type :'
            print '     0 => mask_port_len_drop'
            print '     1 => mask_h2_len_drop'
            print '     2 => mask_h1_len_drop'
            print '     3 => mask_q_len_drop'
            print '     4 => mask_port_pkt_cnt_drop'
            print '     5 => mask_h2_pkt_cnt_drop'
            print '     6 => mask_h1_pkt_cnt_drop'
            print '     7 => mask_q_pkt_cnt_drop'
            print '     8 => mask_global_drop'
            print '     9 => mask_link_up_drop'
            print '   mask      : 0 or 1'
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            ret = xpDebugSetTxqAqmDropReasonMask(args[0], args[1], args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # command to break at txq input
    #/*****************************************************************************************************/
    def do_break_txq_input(self, arg):
        '''
        Break txq at input
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-num'
        else:
            ret = xpDebugBreakTxqInput(args[0], args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/*****************************************************************************************************/
    # command to step at txq input
    #/*****************************************************************************************************/
    def do_step_txq_input(self, arg):
        '''
        Step txq at input
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter device-id, pipe-num, num-iterations'
        else:
            ret = xpDebugStepTxqInput(args[0], args[1], args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # command to continue at txq input
    #/*****************************************************************************************************/
    def do_continue_txq_input(self, arg):
        '''
        Continue txq at input
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-num'
        else:
            ret = xpDebugContinueTxqInput(args[0], args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/*****************************************************************************************************/
    # command to break at txq eq
    #/*****************************************************************************************************/
    def do_break_txq_eq(self, arg):
        '''
        Break txq at eq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-num'
        else:
            ret = xpDebugBreakTxqEq(args[0], args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/*****************************************************************************************************/
    # command to step at txq eq
    #/*****************************************************************************************************/
    def do_step_txq_eq(self, arg):
        '''
        Step txq at eq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter device-id, pipe-num, num-iterations'
        else:
            ret = xpDebugStepTxqEq(args[0], args[1], args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # command to continue at txq eq
    #/*****************************************************************************************************/
    def do_continue_txq_eq(self, arg):
        '''
        Continue txq at eq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-num'
        else:
            ret = xpDebugContinueTxqEq(args[0], args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/*****************************************************************************************************/
    # command to break at txq dq
    #/*****************************************************************************************************/
    def do_break_txq_dq(self, arg):
        '''
        Break txq at dq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-num'
        else:
            ret = xpDebugBreakTxqDq(args[0], args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/*****************************************************************************************************/
    # command to step at txq dq
    #/*****************************************************************************************************/
    def do_step_txq_dq(self, arg):
        '''
        Step txq at dq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter device-id, pipe-num, num-iterations'
        else:
            ret = xpDebugStepTxqDq(args[0], args[1], args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/*****************************************************************************************************/
    # command to continue at txq dq
    #/*****************************************************************************************************/
    def do_continue_txq_dq(self, arg):
        '''
        Continue txq at dq
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter device-id, pipe-num'
        else:
            ret = xpDebugContinueTxqDq(args[0], args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpDebugGetDynamicPoolPageUsedCount
    #/********************************************************************************/
    def do_get_dynamic_pool_page_used_count(self, arg):
        '''
         getDynamicPoolPageUsedCount: Enter [ devId,dynPoolId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dynPoolId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            count_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, dynPoolId=%d' % (args[0],args[1]))
            ret = xpDebugGetDynamicPoolPageUsedCount(args[0],args[1],count_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('count = %d' % (uint32_tp_value(count_Ptr_2)))
                pass
            delete_uint32_tp(count_Ptr_2)

#/*********************************************************************************************************/
# The class object for Debug commands
#/*********************************************************************************************************/
class runDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to set mgmt local reset done
    #/*****************************************************************************************************/
    def do_mgmt_local_reset(self, arg):
        'Set Management local reset bit'
        args = arg.split()
        if  len(args) < 2 :
            print('Invlid input, Enter devId and the Mgmt local Reset value')
        else:
            xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().setMgmtLocalReset(int(args[1]))
            print('Done')

    #/*****************************************************************************************************/
    # command to setBmCfgStatIniHook
    #/*****************************************************************************************************/
    def do_set_bm_cfg_stat_ini_hook(self, arg):
        'command to setBmCfgStatIniHook'
        args = arg.split()
        if  len(args) < 1 :
            print('Invlid input, Enter devId')
        else:
            xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().setBmCfgStatIniHook(int(args[0]))
            print('Done')

    #/********************************************************************************/
    # command for enableDropDebug
    #/********************************************************************************/
    def do_enable_drop_debug(self, arg):
        '''
         enableDropDebug: Enter [ devId,enable ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 2:
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpDebugMgr.instance().enableDropDebug(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Done')

    #/********************************************************************************/
    # command for enablePortDropDebug
    #/********************************************************************************/
    def do_enable_port_drop_debug(self, arg):
        '''
         enablePortDropDebug: Enter [ devId,portId,enable ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 3:
            print('Invalid input, Enter [ devId,portId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            print('Input Arguments are, devId=%d, portId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpDebugMgr.instance().enablePortDropDebug(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Done')
    #/********************************************************************************/
    # command for xpDebugDisplayReasonCodeCounters
    #/********************************************************************************/
    def do_debug_display_reason_code_counters(self, arg):
        '''
         xpDebugDisplayReasonCodeCounters: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpDebugDisplayReasonCodeCounters(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/*****************************************************************************************************/
    # command to print VIFs (Ingress & Egress)
    #/*****************************************************************************************************/
    def do_print_vif(self, arg):
        '''
         Prints the VIF entries.
         Enter device-id, vif, direction (XP_EGRESS, XP_INGRESS)
        '''
        #args = tuple(map(int, arg.split()))
        args = arg.split()
        if  len(args) < 3:
            print 'Invalid input, Enter device-id, vif, direction (XP_EGRESS, XP_INGRESS)'
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            print('Input Arguments are devId=%d, vif=%d, direction=%d' % (args[0], args[1], args[2]))
            if (args[2] == XP_EGRESS):
                print("\nEgress VIF\n")
                ret = xpInterfaceMgr.instance().dumpVifEntry(args[0], args[1], args[2])
            else:
                if (args[2] == XP_INGRESS):
                    print("\nIngress VIF\n")
                    ret = xpInterfaceMgr.instance().dumpVifEntry(args[0], args[1], args[2])
                else:
                    ret = 1
            if (ret == XP_NO_ERR):
                print ('Done')
            else:
                print ('Failed = %d' % ret)

    #/*****************************************************************************************************/
    # command to print drop register counts if any
    #/*****************************************************************************************************/
    def do_print_drops(self, arg):
        '''
         Prints the Non zero drop registers, if any; Input device-id
         Loops through to read all the drop registers of the device and
         prints them if the value is non-zero
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintDropRegListAttr(args[0])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to print global debug info
    #/*****************************************************************************************************/
    def do_get_debug_info(self, arg):
        '''
         Prints the debug info for all the blocks; Input device-id
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetDebugInfo(args[0])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to enable or disable debugging across all blocks
    #/*****************************************************************************************************/
    def do_enable_debug_info(self, arg):
        '''
         Enables or Disables the debug info for all the blocks; Input device-id, enable-1 or 0
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, enable (1 or 0)'
        else:
            print('Input Arguments are devId=%d, %s ' % (args[0], 'enabled' if args[1] is not 0 else 'disabled'))
            xpEnableDebugInfo(args[0], args[1])
            print('Done')
        return

    #/*****************************************************************************************************/
    # command to print PCIe Status registers
    #/*****************************************************************************************************/
    def do_print_pcie_status_registers(self, arg):
        '''
         Input device-id
         read all the PCIe Status registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintPCIeStatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print PCIe Config registers
    #/*****************************************************************************************************/
    def do_print_pcie_config_registers(self, arg):
        '''
         Input device-id
         read all the PCIe Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintPCIeConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print SCPU Status registers
    #/*****************************************************************************************************/
    def do_print_scpu_status_registers(self, arg):
        '''
         Input device-id
         read all the SCPU Status registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintScpuStatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print SCPU Config registers
    #/*****************************************************************************************************/
    def do_print_scpu_config_registers(self, arg):
        '''
         Input device-id
         read all the SCPU Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintScpuConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print DMA0 Status registers
    #/*****************************************************************************************************/
    def do_print_dma0_status_registers(self, arg):
        '''
         Input device-id
         read all the DMA0 Status registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintDMA0StatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print DMA0 Config registers
    #/*****************************************************************************************************/
    def do_print_dma0_config_registers(self, arg):
        '''
         Input device-id
         read all the DMA0 Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintDMA0ConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print DMA1 Status registers
    #/*****************************************************************************************************/
    def do_print_dma1_status_registers(self, arg):
        '''
         Input device-id
         read all the DMA1 Status registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintDMA1StatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print DMA1 Config registers
    #/*****************************************************************************************************/
    def do_print_dma1_config_registers(self, arg):
        '''
         Input device-id
         read all the DMA1 Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintDMA1ConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print SBUS Status registers
    #/*****************************************************************************************************/
    def do_print_sbus_status_registers(self, arg):
        '''
         Input device-id
         read all the SBUS Status registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintSBusStatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print SBUS Config registers
    #/*****************************************************************************************************/
    def do_print_sbus_config_registers(self, arg):
        '''
         Input device-id
         read all the SBUS Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintSBusConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print I2C-S/M Clk Config registers
    #/*****************************************************************************************************/
    def do_print_i2c_config_registers(self, arg):
        '''
         Input device-id
         read all the I2C-S/M Clk Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintI2cConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print GPIO Status/Config registers
    #/*****************************************************************************************************/
    def do_print_gpio_sts_cfg_registers(self, arg):
        '''
         Input device-id
         read all the GPIO Status/Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintGpioStsCfgRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print PLL Status/Config registers
    #/*****************************************************************************************************/
    def do_print_pll_sts_cfg_registers(self, arg):
        '''
         Input device-id
         read all the PLL Status/Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintPLLStsCfgRegListAttr(args[0])
            print('Done')

    #/********************************************************************************/
    # command for printPLLStatusReg
    #/********************************************************************************/
    def do_print_pll_status_reg(self, arg):
        '''
         printPLLStatusReg: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            print('Input Arguments are, devId=%d' % (args[0]))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(args[0]).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.printPLLStatusReg(args[0])
                if ret != 0:
                    print('Error returned = %d' % (ret))
                else:
                    print('Done')
            else:
                print "Error : Init mgmt block first"

    #/*****************************************************************************************************/
    # command to print RESET Config registers
    #/*****************************************************************************************************/
    def do_print_reset_config_registers(self, arg):
        '''
         Input device-id
         read all the RESET Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintResetConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print Mgmt Interrupts Status registers
    #/*****************************************************************************************************/
    def do_print_mgmt_int_status_registers(self, arg):
        '''
         Input device-id
         read all the Mgmt Interrupts Status registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintMgmtIntStatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print Mgmt Interrupts Config registers
    #/*****************************************************************************************************/
    def do_print_mgmt_int_config_registers(self, arg):
        '''
         Input device-id
         read all the Mgmt Interrupts Config registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintMgmtIntConfigRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to print All Mgmt module registers
    #/*****************************************************************************************************/
    def do_print_all_mgmt_registers(self, arg):
        '''
         Input device-id
         read all the Mgmt registers of the device and
         prints them
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpPrintMgmtRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # sub-commands for data path debug operations
    #/*****************************************************************************************************/
    def do_datapath(self, s):
        'data path debug command operations'
        i = runDataPathDebug()
        i.prompt = self.prompt[:-1]+':datapath)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for parser debug operations
    #/*****************************************************************************************************/
    def do_parser(self, s):
        'parser debug command operations'
        i = runParserDebug()
        i.prompt = self.prompt[:-1]+':parser)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for MRE debug operations
    #/*****************************************************************************************************/
    def do_mre(self, s):
        'MRE debug command operations'
        i = runMreDebug()
        i.prompt = self.prompt[:-1]+':mre)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for LDE debug operations
    #/*****************************************************************************************************/
    def do_lde(self, s):
        'LDE debug command operations'
        i = runLdeDebug()
        i.prompt = self.prompt[:-1]+':lde)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SE debug operations
    #/*****************************************************************************************************/
    def do_se(self, s):
        'SE debug command operations'
        i = runSeDebug()
        i.prompt = self.prompt[:-1]+':se)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for URW debug operations
    #/*****************************************************************************************************/
    def do_urw(self, s):
        'URW debug command operations'
        i = runUrwDebug()
        i.prompt = self.prompt[:-1]+':urw)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for TXQ debug operations
    #/*****************************************************************************************************/
    def do_txq(self, s):
        'TXQ debug command operations'
        i = runTxqDebug()
        i.prompt = self.prompt[:-1]+':txq)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for iBuffer debug operations
    #/*****************************************************************************************************/
    def do_ibuffer(self, s):
        'iBuffer debug command operations'
        i = runIbufferDebug()
        i.prompt = self.prompt[:-1]+':ibuffer)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for pfc debug operations
    #/*****************************************************************************************************/
    def do_pfc(self, s):
        'PFC debug command operations'
        i = runPfcDebug()
        i.prompt = self.prompt[:-1]+':pfc)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # command to print status register if non zero
    #/*****************************************************************************************************/
    def do_print_status(self, arg):
        '''
         Prints the Non zero Status registers, if any; Input device-id
         Loops through to read all the status registers of the device and
         prints them if the value is non-zero
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintStatusRegListAttr(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to read PCIe GPIO bus
    #/*****************************************************************************************************/
    def do_read_pcie_gpio_bus(self, arg):
        '''
         Input device-id
         reads PCIe GPIO bus
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpReadPCIeGPIOBus(args[0])
            print('Done')

    #/*****************************************************************************************************/
    # command to set the pcie core bar 0 to 32 bit addressing mode
    #/*****************************************************************************************************/
    def do_set_32b_pcie_core_addressing(self, arg):
        '''
         Input device-id
         sets the pcie core bar 0 to 32 bit addressing mode
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpSet32bPCIeCoreAddressing(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to read the REI done pass status reported by mgmt
    #/*****************************************************************************************************/
    def do_get_rei_done_pass(self, arg):
        '''
         Input device-id
         gets the REI done pass status reported by mgmt
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetREIDonePass(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to set PCIe in one-lane mode
    #/*****************************************************************************************************/
    def do_set_pcie_in_gen1_mode(self, arg):
        '''
         Input device-id
         set PCIe in Gen-1 mode (requires pcie core reset)
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpSetPCIeInGen1Mode(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to set PCIe in one-lane mode
    #/*****************************************************************************************************/
    def do_set_pcie_in_gen2_mode(self, arg):
        '''
         Input device-id
         set PCIe in Gen-2 mode (requires pcie core reset)
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpSetPCIeInGen2Mode(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to set PCIe in one-lane mode
    #/*****************************************************************************************************/
    def do_set_pcie_in_one_lane_mode(self, arg):
        '''
         Input device-id
         set PCIe in one-lane mode (requires pcie core reset)
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpSetPCIeInOneLaneMode(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to set PCIe in four-lane mode
    #/*****************************************************************************************************/
    def do_set_pcie_in_four_lane_mode(self, arg):
        '''
         Input device-id
         set PCIe in four-lane mode
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpSetPCIeInFourLaneMode(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to reset the PCIe core
    #/*****************************************************************************************************/
    def do_reset_pcie_core(self, arg):
        '''
         Input device-id
         command to reset the PCIe core
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpResetPCIeCore(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to unreset the PCIe core
    #/*****************************************************************************************************/
    def do_unreset_pcie_core(self, arg):
        '''
         Input device-id
         command to unreset the PCIe core
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpUnresetPCIeCore(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to get Mgmt's PCIe related info
    #/*****************************************************************************************************/
    def do_get_pcie_mgmt_info(self, arg):
        '''
         Input device-id
         get Mgmt's PCIe related info
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetPCIeMgmtInfo(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to get PCIe's Core related info
    #/*****************************************************************************************************/
    def do_get_pcie_core_info(self, arg):
        '''
         Input device-id
         get PCIe's Core related info
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter device-id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpGetPCIeCoreInfo(args[0])
            print('Done')
    #/*****************************************************************************************************/
    # command to Set debug level for all sbus clients
    #/*****************************************************************************************************/
    def do_debug_level_all_sbus_clients(self, arg):
        '''
         Input device-id , debugLevel
         sets debug level for all sbus clients
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter device-id, debugLevel'
        else:
            print('Input Arguments are devId=%d, debugLevel=%d' % (args[0], args[1]))
            xpLinkManager.instance().setSerdesDebugLevelAll(args[0], args[1])
            print('Done')
    #/*****************************************************************************************************/
    # command to Set dump token level for each module in whitemodel
    #/*****************************************************************************************************/
    def do_whitemodel_dump_token(self, arg):
        '''
         Input device-id , module-id, dumpInput(enable-1/disable-0), dumpOutput(enable-1/disable-0)
         sets dump token level for each module in whitemodel
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 4 or args[1] > 4:
            print 'Invalid input, Enter device-id, module-id, dumpInput(enable-1/0), dumpOutput(enable-1/0)'
            print 'Usage: whitemodel_dump_token 0 0 0 1'
            print ' '
            print 'module-id   module-name'
            print ' 0          PARSER'
            print ' 1          LDE'
            print ' 2          URW'
            print ' 3          MRE'
            print ' 4          MME'
        else:
            print('Input Arguments are devId=%d, module=%d, dumpInput=%s, dumpOutput=%s' % (args[0], args[1], 'enabled' if args[2] is not 0 else 'disabled', 'enabled' if args[3] is not 0 else 'disabled'))
            xpAppTokenDumpInfo(args[0], args[1], args[2], args[3])
            print('Done')
    #/*****************************************************************************************************/
    # command to Get Buffer Statistics Per Port
    #/*****************************************************************************************************/
    def do_get_buffer_stat_per_port(self, arg):
        '''
         Input device-id , port number
         get buffer congestion related info
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 2:
            print 'Invalid input, Enter [device-id, portNum]'
        else:
            print('Input Arguments are devId=%d, portNum=%d' % (args[0], args[1]))
            totalBuf = new_uint32_tp()
            currentBuf = new_uint32_tp()
            ret = xpBufMgrGetBufCountPerPort(args[0], args[1], totalBuf, currentBuf)
            if(ret == XP_NO_ERR):
                print('Total Buffer Space of Port-%d is %d' % (args[1], uint32_tp_value(totalBuf)))
                print('Current Utilized Buffer Space of Port-%d is %d' % (args[1], uint32_tp_value(currentBuf)))
            else:
                print 'Error in fatching buffer statistics'
            delete_uint32_tp(totalBuf)
            delete_uint32_tp(currentBuf)
    #/*****************************************************************************************************/
    # command to Get Buffer Statistics of device
    #/*****************************************************************************************************/
    def do_get_buffer_stat_of_device(self, arg):
        '''
         Input device-id
         get buffer congestion related info
        '''
        args = tuple(map(int, arg.split()))
        if len(args) < 1:
            print 'Invalid input, Enter [device-id]'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            totalBuf = new_uint32_tp()
            currentBuf = new_uint32_tp()
            ret = xpBufMgrGetBufCountTotal(args[0], totalBuf, currentBuf)
            if(ret == XP_NO_ERR):
                print('Total Buffer Space is %d' % (uint32_tp_value(totalBuf)))
                print('Current Utilized Buffer Space is %d' % (uint32_tp_value(currentBuf)))
            else:
                print 'Error in fatching buffer statistics'
            delete_uint32_tp(totalBuf)
            delete_uint32_tp(currentBuf)


#/*********************************************************************************************************/
# The class object for dal Debug commands
#/*********************************************************************************************************/
class runDalDebug(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to set dal debug type
    #/*****************************************************************************************************/
    def do_set_dal_type(self, arg):
        'Set the dal type especially for debug operations at DAL layer; Input devId, (dalDbg), dal Type, mode (mode is relavent only for HW)'
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            runDalDebug().printDalHelp(arg)
        else:
            if xpShellGlobals.disableLog == 0:
                print('Input Arguments are devId=%d, dalType=%d, mode=%d' % (args[0], args[2], args[3]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpSetDalType(args[0], args[2], args[3])
            if xpShellGlobals.disableLog == 0:
                print 'Dal Type is Set'

    #/*****************************************************************************************************/
    # command to print dal cmd2 help
    #/*****************************************************************************************************/
    def printDalHelp(self, arg):
        print 'Invalid input, Enter devId, (dalDbg), dalType, mode (mode is relavent only for HW)'
        print '   Dal-Type                     Dal-Mode'
        print '   ========                    ========='
        print 'XP_HARDWARE = 0                PCIE_BUS = 0'
        print 'XP_SHADOW_HARDWARE = 1         I2C_BUS = 1'
        print 'XP_SHADOW_REMOTEWM = 2         MDIO_BUS = 2'
        print 'XP_SHADOW = 3                  I2C_OVER_LPC = 3'
        print 'XP_SHADOW_EMULATOR = 4         MDIO_OVER_LPC = 4'
        print 'XP_DAL_DEBUG = 5'
        print 'XP_DAL_I2C = 6'
        print 'XP_DAL_MDIO = 7'
        print 'XP_DAL_PIPE = 8'
        print 'XP_DAL_DNGL = 9'
        print 'XP_DAL_DUMMY = 10'
        print 'XP_DAL_RDR = 11'
        print 'XP_DAL_DEFAULT = 12'
        print 'XP_DAL_FIFO = 13'

    #/*****************************************************************************************************/
    # command to set dal config
    #/*****************************************************************************************************/
    def do_set_dal_config(self, arg):
        '''
        Set the dal type in configuration;
        Input devId, dalDbg, dalType, mode (mode is relavent only for HW)
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            runDalDebug().printDalHelp(arg)
        else:
            print('Input Arguments are devId=%d, dalDbg=%d, dalType=%d, mode=%d' % (args[0], args[1], args[2], args[3]))
            cvar.devDefaultConfig.dalConfig.dalDbg = args[1]
            cvar.devDefaultConfig.dalType = args[2]
            #cvar.devDefaultConfig.dalMode = args[3]
            print 'Dal Type Config is Set'

    #/*****************************************************************************************************/
    # command to set dal debug config
    #/*****************************************************************************************************/
    def do_set_dal_dbg_config(self, arg):
        '''
        Set the dal type in configuration;
        Input devId, dalDbg, dalType, dalMode, regOff(hex), regId,
        instance, repeat, offset, wrMask(hex), rdMask(hex)
        Example : set_dal_dbg_config 0 1 5 0 0 0 0 0 0 0x3 0x3
        If you dont want to watch on regOff, pass 0xffffffff;
        similarly -1 can be used to ignore other arguments
        from watching. For wrMask and rdMask, following operations
        can be ORed
        XP_DAL_NUM_TX_RX_ACCESS       (0x1)
        XP_DAL_REG_WRITE_TRACE        (0x2)
        '''
        args = arg.split()
        if  len(args) < 11:
            print('Enter devId, dalDbg, dalType, dalMode, regOff(hex), regId, instance, repeat, offset, wrMask(hex), rdMask(hex)')
            runDalDebug().printDalHelp(arg)
        else:
            print('Input Arguments are devId=%d, dalDbg=%d, dalType=%d, mode=%d' % (int(args[0]), int(args[1]), int(args[2]), int(args[3])))
            print('Input Arguments are regOffset=0x%x, regId=%d, instance=%d, repeat=%d' % (int(args[4],16), int(args[5]), int(args[6]), int(args[7])))
            print('Input Arguments are offset=%d, txDbgMask=0x%x, rxDbgMask=0x%x' % (int(args[8]), int(args[9],16), int(args[10],16)))

            cvar.devDefaultConfig.dalConfig.dalDbg = int(args[1])
            cvar.devDefaultConfig.dalType = int(args[2])
            #cvar.devDefaultConfig.dalMode = int(args[3])
            cvar.devDefaultConfig.dalDbgConfig.regOffset = int(args[4],16)
            cvar.devDefaultConfig.dalDbgConfig.regId = int(args[5])
            cvar.devDefaultConfig.dalDbgConfig.instance = int(args[6])
            cvar.devDefaultConfig.dalDbgConfig.repeat = int(args[7])
            cvar.devDefaultConfig.dalDbgConfig.offset = int(args[8])
            cvar.devDefaultConfig.dalDbgConfig.txDbgMask = int(args[9],16)
            cvar.devDefaultConfig.dalDbgConfig.rxDbgMask = int(args[10],16)
            print 'Dal Debug Config is Set'

    #/*****************************************************************************************************/
    # command to redirect xpDal
    #/*****************************************************************************************************/
    def do_dal_shadow_redirect(self, arg):
        'redirect the read/write to shadow'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print('Invalid input. Enter DeviceId')
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().dalRedirect()
            print 'Dal is redirected to shadow'

    #/*****************************************************************************************************/
    # command to restore xpDal
    #/*****************************************************************************************************/
    def do_dal_shadow_restore(self, arg):
        'restore the read/write to original dal'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print('Invalid input. Enter DeviceId')
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().dalRestore()
            print 'Dal is restored'

    #/*****************************************************************************************************/
    # command to set HW Dal Mode
    #/*****************************************************************************************************/
    def do_set_hw_dal_mode(self, arg):
        'Set the dal HW type and mode; Input devId, dal Type, mode (mode is relavent only for HW)'
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            runDalDebug().printDalHelp(arg)
        else:
            print('Input Arguments are devId=%d, dalType=%d, mode=%d' % (args[0], args[1], args[2]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpSetHwDalMode(args[0], args[1], args[2])
            print 'Dal Type is Set'

    #/*****************************************************************************************************/
    # command to show dal debug type
    #/*****************************************************************************************************/
    def do_show_dal_type(self, arg):
        'Show the current dal type; Input devId'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter devId'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().showDalType(args[0])


    #/*****************************************************************************************************/
    # command to set dal debug ON
    #/*****************************************************************************************************/
    def do_set_debug_on(self, arg):
        'Set the dal debug ON; Input devId, regOffset(hex), regId, instance, repeat, offset (last 3 can be -1)'
        args = arg.split()
        if  len(args) < 6:
            print 'Invalid input, Enter devId, regOffset(hex), regId, instance, repeat, offset (last 3 can be -1)'
        else:
            print('Input Arguments are devId=%d, regOffset=0x%x, regId=%d, inst=%d, repeat=%d, off=%d' %
                    (int(args[0]), int(args[1],16), int(args[2]), int(args[3]), int(args[4]), int(args[5])))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().getDalObj().setDebugOn(int(args[0]),
                                        int(args[1],16), int(args[2]), int(args[3]), int(args[4]), int(args[5]))
            print 'Dal Debug is Set'

    #/*****************************************************************************************************/
    # command to set dal debug OFF
    #/*****************************************************************************************************/
    def do_set_debug_off(self, arg):
        'Set the dal debug OFF; Input devId'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter devId'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().getDalObj().setDebugOff(args[0])
            print 'Dal Debug is turned OFF'

    #/*****************************************************************************************************/
    # command to show dal debug status
    #/*****************************************************************************************************/
    def do_show_debug_status(self, arg):
        'Show the dal debug Status; Input devId'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter devId'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().getDalObj().showDebugStatus(args[0])

    #/*****************************************************************************************************/
    # command to set num read access
    #/*****************************************************************************************************/
    def do_num_read_access(self, arg):
        'Debug Num reads. Input devId, enable(1)/disable(0)'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter devId, enable(1)/disable(0)'
        else:
            print('Input Arguments are devId=%d, enable=%d' % (args[0], args[1]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().getDalObj().xpSetNumRead(args[0], args[1])
            print 'Debug num read access is configured'


    #/*****************************************************************************************************/
    # command to set num write access
    #/*****************************************************************************************************/
    def do_num_write_access(self, arg):
        'Debug Num writes. Input devId, enable(1)/disable(0)'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter devId, enable(1)/disable(0)'
        else:
            print('Input Arguments are devId=%d, enable=%d' % (args[0], args[1]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().getDalObj().xpSetNumWrite(args[0], args[1])
            print 'Debug num write access is configured'

    #/*****************************************************************************************************/
    # command to show the read/write access counts
    #/*****************************************************************************************************/
    def do_show_read_write_access(self, arg):
        'show num read/writes. Input devId, clearOnRead'

        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter devId, clearOnRead'
        else:
            print('Input Arguments are devId=%d, clearOnRead=%d' % (args[0], args[1]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().getDalObj().showRxTxAccessCount(args[0], args[1])


#/*********************************************************************************************************/
# The class object for utitlity functions
#/*********************************************************************************************************/
class utiltityCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # execute
    #/*****************************************************************************************************/
    def do_exec(self, arg):
        'Exec example'
        try:
            exec(arg, globals())     #, locals())
        except:
            print("Syntax error in: %s" % arg)

    #/*****************************************************************************************************/
    # echo command
    #/*****************************************************************************************************/
    def do_echo(self, arg):
        "Print the input, replacing '$out' with the output of the last shell command"
        # Obviously not robust
        print arg.replace('$out', self.last_output)

    #/*****************************************************************************************************/
    # execute shell command
    #/*****************************************************************************************************/
    def do_shell(self, arg):
        "Run a shell command"
        print "running shell command:", arg
        output = os.popen(arg).read()
        print output
        self.last_output = output

#/*********************************************************************************************************/
# The class object for QoS functions
#/*********************************************************************************************************/
class runQosCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpShaperEnablePortShaping
    #/********************************************************************************/
    def do_shaper_enable_port_shaping(self, arg):
        '''
         xpShaperEnablePortShaping: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpShaperEnablePortShaping(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpShaperIsPortShapingEnabled
    #/********************************************************************************/
    def do_shaper_is_port_shaping_enabled(self, arg):
        '''
         xpShaperIsPortShapingEnabled: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpShaperIsPortShapingEnabled(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)

    #/********************************************************************************/
    # command for xpSetPortShaperMtu
    #/********************************************************************************/
    def do_set_port_shaper_mtu(self, arg):
        '''
         xpSetPortShaperMtu: Enter [ devId,mtuInBytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,mtuInBytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mtuInBytes=%d' % (args[0],args[1]))
            ret = xpSetPortShaperMtu(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpGetPortShaperMtu
    #/********************************************************************************/
    def do_get_port_shaper_mtu(self, arg):
        '''
         xpGetPortShaperMtu: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            mtuInBytes_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpGetPortShaperMtu(args[0],mtuInBytes_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuInBytes = %d' % (uint32_tp_value(mtuInBytes_Ptr_1)))
                pass
            delete_uint32_tp(mtuInBytes_Ptr_1)

    #/********************************************************************************/
    # command for xpConfigurePortShaper
    #/********************************************************************************/
    def do_configure_port_shaper(self, arg):
        '''
         xpConfigurePortShaper: Enter [ devId,devPort,rateKbps,maxBurstByteSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,rateKbps,maxBurstByteSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, rateKbps=0x%x, maxBurstByteSize=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpConfigurePortShaper(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpGetPortShaperConfiguration
    #/********************************************************************************/
    def do_get_port_shaper_configuration(self, arg):
        '''
         xpGetPortShaperConfiguration: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rateKbps_Ptr_2 = new_uint64_tp()
            maxBurstByteSize_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpGetPortShaperConfiguration(args[0],args[1],rateKbps_Ptr_2,maxBurstByteSize_Ptr_3)
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
    # command for xpSetPortShaperEnable
    #/********************************************************************************/
    def do_set_port_shaper_enable(self, arg):
        '''
         xpSetPortShaperEnable: Enter [ devId,devPort,enableShaper ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,enableShaper ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, devPort=%d, enableShaper=%d' % (args[0],args[1],args[2]))
            ret = xpSetPortShaperEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpGetPortShaperEnable
    #/********************************************************************************/
    def do_get_port_shaper_enable(self, arg):
        '''
         xpGetPortShaperEnable: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enableShaper_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpGetPortShaperEnable(args[0],args[1],enableShaper_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableShaper = %d' % (uint32_tp_value(enableShaper_Ptr_2)))
                pass
            delete_uint32_tp(enableShaper_Ptr_2)

    #/********************************************************************************/
    # command for xpGetPortShaperTableIndex
    #/********************************************************************************/
    def do_get_port_shaper_table_index(self, arg):
        '''
         xpGetPortShaperTableIndex: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            index_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d' % (args[0],args[1]))
            ret = xpGetPortShaperTableIndex(args[0],args[1],index_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (uint32_tp_value(index_Ptr_2)))
                pass
            delete_uint32_tp(index_Ptr_2)

    #/********************************************************************************/
    # command for xpShaperEnableQueueShaping
    #/********************************************************************************/
    def do_shaper_enable_queue_shaping(self, arg):
        '''
         xpShaperEnableQueueShaping: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpShaperEnableQueueShaping(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpShaperIsQueueShapingEnabled
    #/********************************************************************************/
    def do_shaper_is_queue_shaping_enabled(self, arg):
        '''
         xpShaperIsQueueShapingEnabled: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpShaperIsQueueShapingEnabled(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)

    #/********************************************************************************/
    # command for xpSetQueueShaperMtu
    #/********************************************************************************/
    def do_set_queue_shaper_mtu(self, arg):
        '''
         xpSetQueueShaperMtu: Enter [ devId,mtuInBytes ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,mtuInBytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mtuInBytes=%d' % (args[0],args[1]))
            ret = xpSetQueueShaperMtu(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpGetQueueShaperMtu
    #/********************************************************************************/
    def do_get_queue_shaper_mtu(self, arg):
        '''
         xpGetQueueShaperMtu: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            mtuInBytes_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpGetQueueShaperMtu(args[0],mtuInBytes_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuInBytes = %d' % (uint32_tp_value(mtuInBytes_Ptr_1)))
                pass
            delete_uint32_tp(mtuInBytes_Ptr_1)

    #/********************************************************************************/
    # command for xpConfigureQueueSlowShaper
    #/********************************************************************************/
    def do_configure_queue_slow_shaper(self, arg):
        '''
         xpConfigureQueueSlowShaper: Enter [ devId,devPort,queueNum,rateKbps,maxBurstByteSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,rateKbps,maxBurstByteSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, rateKbps=0x%x, maxBurstByteSize=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpConfigureQueueSlowShaper(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpGetQueueSlowShaperConfiguation
    #/********************************************************************************/
    def do_get_queue_slow_shaper_configuation(self, arg):
        '''
         xpGetQueueSlowShaperConfiguation: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            rateKbps_Ptr_3 = new_uint64_tp()
            maxBurstByteSize_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpGetQueueSlowShaperConfiguation(args[0],args[1],args[2],rateKbps_Ptr_3,maxBurstByteSize_Ptr_4)
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
    # command for xpSetQueueSlowShaperEnable
    #/********************************************************************************/
    def do_set_queue_slow_shaper_enable(self, arg):
        '''
         xpSetQueueSlowShaperEnable: Enter [ devId,devPort,queueNum,enableShaper ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum,enableShaper ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d, enableShaper=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpSetQueueSlowShaperEnable(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpGetQueueSlowShaperEnable
    #/********************************************************************************/
    def do_get_queue_slow_shaper_enable(self, arg):
        '''
         xpGetQueueSlowShaperEnable: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            enableShaper_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpGetQueueSlowShaperEnable(args[0],args[1],args[2],enableShaper_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enableShaper = %d' % (uint32_tp_value(enableShaper_Ptr_3)))
                pass
            delete_uint32_tp(enableShaper_Ptr_3)

    #/********************************************************************************/
    # command for xpGetQueueSlowShaperTableIndex
    #/********************************************************************************/
    def do_get_queue_slow_shaper_table_index(self, arg):
        '''
         xpGetQueueSlowShaperTableIndex: Enter [ devId,devPort,queueNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,queueNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            index_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, devPort=%d, queueNum=%d' % (args[0],args[1],args[2]))
            ret = xpGetQueueSlowShaperTableIndex(args[0],args[1],args[2],index_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (uint32_tp_value(index_Ptr_3)))
                pass
            delete_uint32_tp(index_Ptr_3)

    #/*****************************************************************************************************/
    # command to set AQM profile to port/queue
    #/*****************************************************************************************************/
    def do_set_aqm_profile_to_port_q(self, arg):
        'Set AQM Profile to Port / Queue: Enter Device-Id, portNumber, queueNum, profileType (0 to 6), profileId'
        args = tuple(map(int, arg.split()))
        if  len(args) < 5:
            print 'Invalid input, Enter Device-Id, portNumber, queueNum, profileType (0 to 6), profileId'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(int(args[0]))):
                print('Input Arguments are devId=%d, port=%d, queue=%d, profileType=%d and profileId=%d' %
                                          (args[0], args[1], args[2], args[3], args[4]))
                xpAqmMgr.instance().setAqmProfileToPortQ(args[0], args[1], args[2], args[3], args[4])
                print('Set Aqm Profile to port queue success !!')
            else:
                print ('Device %d not added' %args[0])


    #/*****************************************************************************************************/
    # command to set AQM Queue profile to port/queue
    #/*****************************************************************************************************/
    def do_set_aqm_q_profile_to_port_q(self, arg):
        'St AQM Q Profile to Port / Queue: Enter Device-Id, portNumber, queueNum, profileId'
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            print 'Invalid input, Enter Device-Id, portNumber, queueNum, profileId'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(int(args[0]))):
                print('Input Arguments are devId=%d, port=%d, queue=%d and profileId=%d' %
                                          (args[0], args[1], args[2], args[3]))
                xpAqmMgr.instance().setAqmQProfileToPortQ(args[0], args[1], args[2], args[3])
                print('Set Aqm Queue Profile to port queue success !!')
            else:
                print ('Device %d not added' %args[0])


    #/*****************************************************************************************************/
    # command to set AQM profile field
    #/*****************************************************************************************************/
    def do_set_aqm_profile_field(self, arg):
        'Set field in AQM Profile: Enter Device-Id, profileId, profileField, field Value'
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            print 'Invalid input, Enter Device-Id, profileId, profileField, field Value'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(int(args[0]))):
                print('Input Arguments are devId=%d, profileId=%d, field=%d and value=%d' %
                                                    (args[0], args[1], args[2], args[3]))
                xpAqmMgr.instance().setAqmProfileField(args[0], args[1], args[2], args[3])
                print('Set Aqm Profile field success !!')
            else:
                print ('Device %d not added' %args[0])


    #/*****************************************************************************************************/
    # command to set AQM Queue profile field
    #/*****************************************************************************************************/
    def do_set_aqm_q_profile_field(self, arg):
        'Set field in AQM Q Profile: Enter Device-Id, profileId, profileField, field Value'
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            print 'Invalid input, Enter Device-Id, profileId, profileField, field Value'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(int(args[0]))):
                print('Input Arguments are devId=%d, profileId=%d, field=%d and value=%d' %
                                                    (args[0], args[1], args[2], args[3]))
                xpAqmMgr.instance().setAqmQProfileField(args[0], args[1], args[2], args[3])
                print('Set Aqm Queue Profile field success !!')
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to show queue forward count for a given port and queue
    #/*****************************************************************************************************/
    def do_get_q_fwd_cnt_for_port(self, arg):
        'Retrieve the Port TC (Q) Forward Clear-On-Read Counter: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint64_tp()
                wrap = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getQueueFwdPacketCountForPort(args[0], args[1], args[2], cnt, wrap)
                cntVal  = uint64_tp_value(cnt)
                wrapVal = uint32_tp_value(wrap)

                print 'Device Id    Device Port        Queue       Count(Packets)      Wrap'
                print '--------------------------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d            %-1d' % (args[0], args[1], args[2], cntVal, wrapVal)
                delete_uint64_tp(cnt)
                delete_uint32_tp(wrap)
            else:
                print 'Device %d not added' % (args[0])

    #/*****************************************************************************************************/
    # command to show queue drop count for a given port and queue
    #/*****************************************************************************************************/
    def do_get_q_drop_cnt_for_port(self, arg):
        'Retrieve the Port TC (Q) Drop Clear-On-Read Counter: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint64_tp()
                wrap = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getQueueDropPacketCountForPort(args[0], args[1], args[2], cnt, wrap)
                cntVal  = uint64_tp_value(cnt)
                wrapVal = uint32_tp_value(wrap)

                print 'Device Id    Device Port        Queue       Count(Packets)      Wrap'
                print '--------------------------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d            %-1d' % (args[0], args[1], args[2], cntVal, wrapVal)
                delete_uint64_tp(cnt)
                delete_uint32_tp(wrap)
            else:
                print 'Device %d not added' % (args[0])

    #/*****************************************************************************************************/
    # command to show queue forward length count in pages for a given port and queue
    #/*****************************************************************************************************/
    def do_get_q_fwd_len_cnt_for_port(self, arg):
        'Retrieve the Port TC (Q) Forward Length Clear-On-Read Counter: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint64_tp()
                wrap = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getQueueFwdPageCountForPort(args[0], args[1], args[2], cnt, wrap)
                cntVal  = uint64_tp_value(cnt)
                wrapVal = uint32_tp_value(wrap)

                print 'Device Id    Device Port        Queue       Count(Pages)        Wrap'
                print '--------------------------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d            %-1d' % (args[0], args[1], args[2], cntVal, wrapVal)
                delete_uint64_tp(cnt)
                delete_uint32_tp(wrap)
            else:
                print 'Device %d not added' % (args[0])


    #/*****************************************************************************************************/
    # command to show queue drop count length in pages for a given port and queue
    #/*****************************************************************************************************/
    def do_get_q_drop_len_cnt_for_port(self, arg):
        'Retrieve the Port TC (Q) Drop Length Clear-On-Read Counter: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint64_tp()
                wrap = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getQueueDropPageCountForPort(args[0], args[1], args[2], cnt, wrap)
                cntVal  = uint64_tp_value(cnt)
                wrapVal = uint32_tp_value(wrap)

                print 'Device Id    Device Port        Queue       Count(Pages)        Wrap'
                print '--------------------------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d            %-1d' % (args[0], args[1], args[2], cntVal, wrapVal)
                delete_uint64_tp(cnt)
                delete_uint64_tp(wrap)
            else:
                print 'Device %d not added' % (args[0])


    #/*****************************************************************************************************/
    # command to show current queue packet depth
    #/*****************************************************************************************************/
    def do_get_q_current_pkt_depth(self, arg):
        'Retrieve the Current depth of a Queue associated with a port: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getCurrentQueuePacketDepth(args[0], args[1], args[2], cnt)
                cntVal  = uint32_tp_value(cnt)

                print 'Device Id    Device Port        Queue       Count(Packets)'
                print '----------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d        ' % (args[0], args[1], args[2], cntVal)
                delete_uint32_tp(cnt)
            else:
                print 'Device %d not added' % (args[0])

    #/*****************************************************************************************************/
    # command to show current queue packet length depth
    #/*****************************************************************************************************/
    def do_get_q_current_len_depth(self, arg):
        'Retrieve the Current depth of a Queue in pages associated with a port: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getCurrentQueuePageDepth(args[0], args[1], args[2], cnt)
                cntVal  = uint32_tp_value(cnt)

                print 'Device Id    Device Port        Queue       Count(Pages)  '
                print '----------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d        ' % (args[0], args[1], args[2], cntVal)
                delete_uint32_tp(cnt)
            else:
                print 'Device %d not added' % (args[0])

    #/*****************************************************************************************************/
    # command to show average queue packet length
    #/*****************************************************************************************************/
    def do_get_q_average_len(self, arg):
        'Retrieve the Average depth of a Queue in pages associated with a port: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getQueueAveragePageLength(args[0], args[1], args[2], cnt)
                cntVal  = uint32_tp_value(cnt)

                print 'Device Id    Device Port        Queue       Count(Pages)  '
                print '----------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d        ' % (args[0], args[1], args[2], cntVal)
                delete_uint32_tp(cnt)
            else:
                print 'Device %d not added' % (args[0])

    #/*****************************************************************************************************/
    # command to show old queue packet length
    #/*****************************************************************************************************/
    def do_get_q_old_len(self, arg):
        'Retrieve the Old depth of a Queue in pages associated with a port: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getQueueOldPageLength(args[0], args[1], args[2], cnt)
                cntVal  = uint32_tp_value(cnt)

                print 'Device Id    Device Port        Queue       Count(Pages)  '
                print '----------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d        ' % (args[0], args[1], args[2], cntVal)
                delete_uint32_tp(cnt)
            else:
                print 'Device %d not added' % (args[0])


    #/*****************************************************************************************************/
    # command to show pfc count
    #/*****************************************************************************************************/
    def do_get_pfc_cnt(self, arg):
        'Retrieve the PFC Count of a Queue in pages associated with a port: Enter Device-Id, Device-Port, Queue'
        args = tuple(map(int, arg.split()))
        if len(args) < 3:
            print 'Invalid input, Enter Device-Id, Device-Port, Queue'
        else:
            if xpDeviceMgr.instance().isDeviceValid(int(args[0])):
                cnt  = new_uint32_tp()
                print 'Input Arguments are devId=%d, devPort=%d, q=%d' % (args[0], args[1], args[2])

                xpQueueCounterMgr.instance().getPfcPageCount(args[0], args[1], args[2], cnt)
                cntVal  = uint32_tp_value(cnt)

                print 'Device Id    Device Port        Queue       Count(Pages)  '
                print '----------------------------------------------------------'
                print '%-2d           %-3d                %-5d       %-8d        ' % (args[0], args[1], args[2], cntVal)
                delete_uint32_tp(cnt)
            else:
                print 'Device %d not added' % (args[0])

#/*********************************************************************************************************/
# The definition for displaying help for logging
#/*********************************************************************************************************/
def displayLogHelp(a):
    '''
    Prints error message and display the module name/ log-level
    name and correspoding ID
    '''
    if a == 1 or a == 3:
        i = xpLogMgr.instance().getNumModules()
        print ' '
        print 'ID     MODULE NAME'
        print '----------------------'
        for x in range(0, i):
            print ('%02d      %s' %(x, xpLogMgr.instance().getModuleName(x)))

    if a == 2 or  a == 3:
        i = xpLogMgr.instance().getNumLogLevels()
        print ' '
        print 'NUM    LEVEL   '
        print '----------------------'
        for x in range(0, i):
            print ('%02d      %s ' %(x, xpLogMgr.instance().getLevelName(x)))
    del i
    del x

#/*********************************************************************************************************/
# The definition for displaying help for subModule logging
#/*********************************************************************************************************/
def displaySubModuleLogHelp(arg):
    print ' '
    print 'ID    SUB MODULE NAME'
    print '----------------------'
    j = xpLogMgr.instance().getNumSubModule(arg)
    for x in range(0, j):
        print '%02d    %s' %(x,xpLogMgr.instance().getSubmoduleName(arg, x))
    print '%02d    ALL' %(j)

#/*********************************************************************************************************/
# The class object for LOG related commands
#/*********************************************************************************************************/
class runLog(Cmd):
    #/*****************************************************************************************************/
    # command to enable Log
    #/*****************************************************************************************************/
    def do_enable_log(self, arg):
        '''
        Enable log.
        '''
        consoleLog = xpLogMgr.instance().find(XP_CONSOLE_LOG, XP_DEFAULT_CONSOLE_NAME);
        if  (consoleLog is None):
            consoleLog = xpLogMgr.instance().create(XP_CONSOLE_LOG, XP_DEFAULT_CONSOLE_NAME, 0, False);
        xpLogMgr.instance().connectAll(consoleLog);

        if xpShellGlobals.sessionFlag != 0:
            print ' Logging Enabled'

    #/*****************************************************************************************************/
    # command to disable Log
    #/*****************************************************************************************************/
    def do_disable_log(self, arg):
        '''
        Disable log.
        '''
        xpLogMgr.instance().disconnectAll();
        print ' Logging Disabled'

    #/*****************************************************************************************************/
    # command to Set Log Level for modules
    #/*****************************************************************************************************/
    def do_set_log_level(self, arg):
        '''
        Set log-level , Input log-level num
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Log-Level number'
	    print '0-DEBUG   1-TRACE  2-DEFAULT  3-WARNING  4-ERROR  5-CRITICAL'
        else :
            print(' LogLevel is set to %d' % (args[0]))
            y = xpsGetNumLogLevels()
            if args[0] > y-1:
               print 'Invalid Log-Level'
            else:
               xpsSetLogLevel(args[0])
            del y

    #/*****************************************************************************************************/
    # command to print Loglevel for each module
    #/*****************************************************************************************************/
    def do_get_log_level(self, arg):
        '''
        Get log_level 
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) >= 1:
            print 'Invalid input\n'
            print 'Give Enter after the CLI command '
        else:
            print '----------------'
            l = xpsGetLogLevel()
	    name = xpsGetLogLevelName(l)
            print '%d  -  %s' %(l, name)
	    print '----------------'
	    del l
	    del name

    #/*****************************************************************************************************/
    # command to enable submodule
    #/*****************************************************************************************************/
    def do_enable_submodule(self, arg):
        '''
         Enable logging for submodules
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Module-Id, SubModule-Id'
            displayLogHelp(1)
        elif len(args) < 2:
            print 'Invalid input,Enter Module-Id SubModule-Id'
            i = xpLogMgr.instance().getNumModules()
            if  args[0] > i-1:
                print ''
                print 'Invalid Module-id'
                displayLogHelp(1)
            else:
                displaySubModuleLogHelp(args[0])
        else:
            i = xpLogMgr.instance().getNumModules()
            j = xpLogMgr.instance().getNumSubModule(args[0])
            if  args[0] > i-1:
                print ''
                print 'Invalid Module-id'
                displayLogHelp(1)
            elif args[1] > j:
                print 'Invalid SubModule-id'
                displaySubModuleLogHelp(args[0])
            else:
                xpLogMgr.instance().setSubModuleLog(args[0], args[1], True);
                print('Input Arguments are Module: %s, SubModule: %s' %(xpLogMgr.instance().getModuleName(args[0]), xpLogMgr.instance().getSubmoduleName(args[0], args[1])))

    #/*****************************************************************************************************/
    # command to disable submodule
    #/*****************************************************************************************************/
    def do_disable_submodule(self, arg):
        '''
         Disable logging for submodule
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Module-Id, SubModule-Id'
            displayLogHelp(1)
        elif len(args) < 2:
            print 'Invalid input,Enter Module-Id SubModule-Id'
            i = xpLogMgr.instance().getNumModules()
            if  args[0] > i-1:
                print ''
                print 'Invalid Module-id'
                displayLogHelp(1)
            else:
                displaySubModuleLogHelp(args[0])
        else:
            i = xpLogMgr.instance().getNumModules()
            j = xpLogMgr.instance().getNumSubModule(args[0])
            if  args[0] > i-1:
                print ''
                print 'Invalid Module-id'
                displayLogHelp(1)
            elif args[1] > j:
                print 'Invalid SubModule-id'
                displaySubModuleLogHelp(args[0])
            else:
                xpLogMgr.instance().setSubModuleLog(args[0], args[1], False);
                print('Input Arguments are Module: %s, SubModule: %s' %(xpLogMgr.instance().getModuleName(args[0]), xpLogMgr.instance().getSubmoduleName(args[0], args[1])))

    #/*****************************************************************************************************/
    # command to enable file logging
    #/*****************************************************************************************************/
    def do_log_file(self, arg):
        '''
         enable file logging
        '''
        #args = tuple(map(int, arg.split()))
        args = re.split(';| ',arg)
        if  args[0] == '':
            print '  Invalid Input'
            print '   Enter default for log file "/var/log/xp/xdk.log" '
            print '   Enter exact path for a perticular log file'
            print '   Enter exact path and module ID for a perticular module'
            print '   Example:'
            print '     1. log_file default'
            print '     2. log_file /home/peter/xdk.log'
            print '     3. log_file /home/peter/xdk.log 9'
            print ''
        elif args[0] != '' and len(args) == 1:
            if args[0] == "default" :
                args[0] = "/var/log/xp/xdk.log"

            if not os.path.exists(os.path.dirname(args[0])):
                os.makedirs(os.path.dirname(args[0]))

            if not os.path.exists(args[0]):
                 open(args[0], 'w').close()

            ret = xpLogMgr.instance().find(XP_FILE_LOG, args[0])
            ret1 = ret
            if ret == None :
                ret1 = xpLogMgr.instance().create(XP_FILE_LOG, args[0], 0, False)
            if ret1 == None:
                print('  FAILED  for file :%s' % (args[0]))
            else:
                xpLogMgr.instance().connectAll(ret1)
                #if xpShellGlobals.sessionFlag != 0:
                #    print('  %s  Enabled' % args[0])
        elif args[1] != '':
            args[1] = eval(args[1])
            print('Input Arguments are ModuleId=%02d' % (args[1]))
            i = xpLogMgr.instance().getNumModules()
            if args[1] > i-1:
               print 'Invalid Module-id'
               displayLogHelp(1)
            else :
               if not os.path.exists(os.path.dirname(args[0])):
                  os.makedirs(os.path.dirname(args[0]))
               ret = xpLogMgr.instance().find(XP_FILE_LOG, args[0])
               ret1 = ret
               if ret == None :
                  ret1 = xpLogMgr.instance().create(XP_FILE_LOG, args[0], 0, False)

               if ret1 == None:
                   print('  FAILED  for  file :%s' % (args[0]))
               else:
                   xpLogMgr.instance().connect(args[1], ret1)
                   if xpShellGlobals.sessionFlag != 0:
                       print('  %s    Enabled' % args[0])

#/*********************************************************************************************************/
# the command loop of xpShell.
#/*********************************************************************************************************/
class xpShellObj(Cmd):
    prompt = '(xpShell) '
    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    # ----- basic xpShell commands -----

    #/*****************************************************************************************************/
    # command to print version information of XDK
    #/*****************************************************************************************************/
    def do_show_version(self, arg):
        'Print version information of SDK.'
        cpssHalPrintVersion()


    #/*****************************************************************************************************/
    # command to reset Chip
    #/*****************************************************************************************************/
    def do_reset_device(self, arg):
        '''
         Resets the chip if not already in reset; Input device-id
         Toggles both dyn and cfg reset
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, needs devId, Assuming devId zero')
            devId = 0
        else:
            devId = int(args[0])

        ret = xpDeviceMgr.instance().resetDevice(devId)
        if ret != 0:
            print('Return Value = %d' % (ret))
        else:
            pass

    #/*****************************************************************************************************/
    # command to print initialization type
    #/*****************************************************************************************************/
    def do_show_init_type(self, arg):
        'Print initialization type. Enter [ devId ]'
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, needs devId, Assuming devId zero')
            devId = 0
        else:
            devId = int(args[0])

        initType = xpDeviceMgr.instance().getInitType(devId)
        initDone = xpDeviceMgr.instance().isDevInitDone(devId)

        str1 = 'Device ' + str(devId) + ' is initialized with init type : '
        str2 = 'INIT_UNKNOWN'

        if initDone == 0:
            str1 = 'Device ' + str(devId) + ' initialization is in progress with init type : '

        if initType == INIT_COLD:
            str2 = 'INIT_COLD'

        if initType == INIT_WARM:
            str2 = 'INIT_WARM'

        if initType == INIT_WARM_SYNC_TO_HW:
            str2 = 'INIT_WARM_SYNC_TO_HW'

        if initType == INIT_WARM_SYNC_TO_SHADOW:
            str2 = 'INIT_WARM_SYNC_TO_SHADOW'

        print('%s%s' % (str1, str2))

    #/*****************************************************************************************************/
    # command to display boot time analysis
    #/*****************************************************************************************************/
    def do_display_boot_time_analysis(self, arg):
        '''
         Display boot time analysis.
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input. Need device Id.')
        else:
            devId = int(args[0])
            xpsPrintBootTimeAnalysis(devId)
            pass
    #/*****************************************************************************************************/

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

    #/*****************************************************************************************************/
    # sub-commands for utility functions
    #/*****************************************************************************************************/
    def do_utils(self, s):
         'Utility commands'
         i = utiltityCmds()
         i.prompt = self.prompt[:-1]+':utils)'
         while True:
             try:
                 i.cmdloop()
                 break
             except (ValueError, IndexError), e:
                 print 'Invalid input format, check help <command> or ? for usage'
                 continue
         del i

    def do_xp_logo(self,s):
        'Just print the xpliant logo'
        class color:
            RED = '\033[91m'
            BOLD = '\033[1m'
            END = '\033[0m'

        if (xpShellGlobals.sessionFlag != 0):
            return

        print('                                                                                ')
        print(color.BOLD + color.RED + '                    #,#.#       ###. ,###*  #*#####*   ' + color.END)
        print(color.BOLD + color.RED + '                    #.#.*#     ##*  ###,  #####*       ' + color.END)
        print(color.BOLD + color.RED + '                    .# # #.  *## .##*  ,####.          ' + color.END)
        print(color.BOLD + color.RED + '                     *### #.## .##. ,###,              ' + color.END)
        print(color.BOLD + color.RED + '                      #,## ## ##. ###*                 ' + color.END)
        print(color.BOLD + color.RED + '                       #*,#,,## *##  ##                ' + color.END)
        print(color.BOLD + color.RED + '                        .#.,#. ##.  ## *#              ' + color.END)
        print(color.BOLD + color.RED + '                          *# ###  ### ## #*            ' + color.END)
        print(color.BOLD + color.RED + '                           .##, .##. ## ##.#           ' + color.END)
        print(color.BOLD + color.RED + '                            * *##. *#*.##,*##          ' + color.END)
        print(color.BOLD + color.RED + '                           ,###. *## ,### #,#*         ' + color.END)
        print(color.BOLD + color.RED + '                        *###,  ###  ##  ,# #,#         ' + color.END)
        print(color.BOLD + color.RED + '                    *####.  *##* .##,     *.#.#        ' + color.END)
        print(color.BOLD + color.RED + '                *#####.  *###. .##,       #.#*#,       ' + color.END)
        print(color.BOLD + color.RED + '           ,#####*.   ####.  *##.         #,#.#       ' + color.END)
        print('                                                                          ')


    #/*****************************************************************************************************/
    # sub-commands for register/table operation functions
    #/*****************************************************************************************************/
    def do_reg_access(self, s):
        'Register and Table operation commands'
        i = regTableOperationCmds()
        i.prompt = self.prompt[:-1]+':regAccess)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer functions
    #/*****************************************************************************************************/
    def do_xps(self, s):
        'XPS layer APIs'
        i = xpsLayerOperationCmds()
        i.prompt = self.prompt[:-1]+':xps)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue

    #/*****************************************************************************************************/
    # sub-commands for SAI layer functions
    #/*****************************************************************************************************/
    if ("saiShell" in sys.modules.keys() or "ksaiShell" in sys.modules.keys()):
        def do_sai(self, s):
            'SAI layer APIs'
            i = saiLayerOperationCmds()
            i.prompt = self.prompt[:-1]+':sai)'
            while True:
                try:
                    i.cmdloop()
                    break
                except (ValueError, IndexError), e:
                    print 'Invalid inputs format, check help <command> or ? for usage'
                    continue


    #/*****************************************************************************************************/
    # sub-commands for DEVICE layer functions
    #/*****************************************************************************************************/
    def do_device(self, s):
        'PL layer APIs'
        i = xpPlOperationCmds()
        i.prompt = self.prompt[:-1]+':device)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue

    #/*****************************************************************************************************/
    # sub-commands for platform functions
    #/*****************************************************************************************************/
    def do_bdk_access(self, s):
        'BDK APIs'
        i = xpBdkObj()
        i.prompt = self.prompt[:-1]+':bdk)'
        if (xpShellGlobals.sessionFlag == 0):
            xpDeviceMgr.instance().getDeviceObj(0).getHwAccessMgr().xpCacheLookUpInit(0)
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue

    #/*****************************************************************************************************/
    # sub-commands for dal debug operations
    #/*****************************************************************************************************/
    def do_dal_debug(self, s):
        'DAL debug command operations'
        i = runDalDebug()
        i.prompt = self.prompt[:-1]+':dal)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for system debug operations
    #/*****************************************************************************************************/
    def do_system(self, s):
        'system command operations'
        i = runSystem()
        i.prompt = self.prompt[:-1]+':system)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for dal debug operations
    #/*****************************************************************************************************/
    def do_debug(self, s):
        'debug command operations'
        i = runDebug()
        i.prompt = self.prompt[:-1]+':debug)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    if os.path.isdir(xpDiagPath) and (os.path.isfile(sdkPyFile) or os.path.isfile(ksdkPyFile) or os.path.isfile(ksaiShellPyFile)):
        #/*****************************************************************************************************/
        # sub-commands for xpDiags operations
        #/*****************************************************************************************************/
        def do_xpdiags(self, s):
            'xpDiag command operations'
            i = runXpDiags()
            i.prompt = self.prompt[:-1]+':xpdiags)'
            while True:
                try:
                    i.cmdloop()
                    break
                except (ValueError, IndexError), e:
                    print 'Invalid input format, check help <command> or ? for usage'
                    continue
            del i

        #/*****************************************************************************************************/
        # sub-commands for svbInterface
        #/*****************************************************************************************************/
        def do_svb_interface(self, s):
            'svb interface operations'
            i = xpSvbInterface()
            i.prompt = self.prompt[:-1]+':svb_interface)'
            while True:
                try:
                    i.cmdloop()
                    break
                except (ValueError, IndexError), e:
                    print 'Invalid input format, check help <command> or ? for usage'
                    continue
            del i

        #/*****************************************************************************************************/
        # sub-commands for crb1
        #/*****************************************************************************************************/
        def do_crb_interface(self, s):
            'crb interface operations'
            i = runCrbCmd()
            i.prompt = self.prompt[:-1]+':crb)'
            while True:
                try:
                    i.cmdloop()
                    break
                except (ValueError, IndexError), e:
                    print 'Invalid input format, check help <command> or ? for usage'
                    continue
            del i

    #/*****************************************************************************************************/
    # sub-commands for link manager operations
    #/*****************************************************************************************************/
    def do_link_mgr(self, s):
        'link manager command operations'
        print 'Link manager operations are currently not supported'
        return
        i = xpLinkMgrObj()
        i.prompt = self.prompt[:-1]+':linkMgr)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for interrupt handler operations
    #/*****************************************************************************************************/
    def do_interrupt_mgr(self, s):
        'interrupt handler command operations'
        i = xpInterruptMgrObj()
        i.prompt = self.prompt[:-1]+':interruptMgr)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for serdes operations
    #/*****************************************************************************************************/
    def serdes(self, s):
        'Serdes command operations'
        i = runSerdes()
        i.prompt = self.prompt[:-1]+':serdes)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for ACM  Sflow operations. Hidden now
    #/*****************************************************************************************************/
    def sflow(self, s):
        'ACM sflow command operations'
        i = xpSflowObj()
        i.prompt = self.prompt[:-1]+':sflow)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for ACM operations. Hidden now
    #/*****************************************************************************************************/
    def do_acm(self, s):
        'ACM command operations'
        i = runAcm()
        i.prompt = self.prompt[:-1]+':acm)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for test scripts operations
    #/*****************************************************************************************************/
    def do_test_scripts(self, s):
        'Test Scripts command operations'
        i = runTestScripts()
        i.prompt = self.prompt[:-1]+':testScripts)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for L2Domain operations
    #/*****************************************************************************************************/
    def l2_domain(self, s):
        'L2 Domain command operations'
        i = runl2Domain()
        i.prompt = self.prompt[:-1]+':l2Domain)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for Logging operations
    #/*****************************************************************************************************/
    def do_log(self, s):
        'Logging Level command operations'
        i = runLog()
        i.prompt = self.prompt[:-1]+':log)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for register/table operation functions
    #/*****************************************************************************************************/
    def do_qos(self, s):
        'QoS and TxQ operation commands'
        i = runQosCmds()
        i.prompt = self.prompt[:-1]+':qos)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # command loop and check for exception
    #/*****************************************************************************************************/
    def cmdloop(self, configFile):
        global gexitmode
        global shellmode
        if configFile != 'None':
            if os.path.exists(configFile) == 0:
                #print('configFile = %s' % configFile)
                tmpConfFile = '/tmp/tmpXpShell.txt'
                if ('xpShell/' in configFile):
                    if os.path.isfile(tmpConfFile):
                        os.remove(tmpConfFile)
                    fpaths = tmpConfFile
                    wp = open(fpaths, 'w+')
                    confCmds = re.split('/',configFile)
                    nums = configFile.count('/') - 1
                    width = len(confCmds)
                    for i in range(1, width):
                        entry = confCmds[i]
                        entry = entry.lower() + '\n'
                        wp.write(entry)
                    entry = 'back' + '\n'
                    for i in range(0, nums):
                        wp.write(entry)
                    entry = 'exit 0' + '\n'
                    for i in range(0, 2):
                        wp.write(entry)
                    wp.close()
                    configFile = tmpConfFile
                else:
                    if os.path.isfile(tmpConfFile):
                        os.remove(tmpConfFile)
                    fpaths = tmpConfFile
                    wp = open(fpaths, 'w+')
                    entry = 'exit 0' + '\n'
                    for i in range(0, 2):
                        wp.write(entry)
                    wp.close()
                    if 'xpShellCmdChain' not in configFile:
                        configFile = tmpConfFile

        if configFile != 'None':
            if (xpDeviceMgr.instance().isDevInitDone(0) == 0):
                print('xpShell waiting for device Initialization...')
                time.sleep(30)
                while (xpDeviceMgr.instance().isDevInitDone(0) == 0):
                    time.sleep(5)
                print('xpShell waiting for device Initialization...')
                time.sleep(10)

            configFile = configFile.replace(" ", "")
            #filter and restore configuration file if <configfile>.tmp exist
            if(os.path.isfile(configFile + '.tmp')):
                print "Filtering Configuration file [%s]" % configFile
                Cmd.filter_runningconfig(self, configFile + '.tmp', configFile)
            runConfig = RunningConfig.strip()
            if (configFile == runConfig):
                if os.path.exists(configFile):
                    print('Loading Configuration file [%s]' % configFile)
                else:
                    configobj = open(configFile, "w+")
                    configobj.close()
                    if os.path.exists(configFile):
                        print('Create and Load Configuration file [%s]' % configFile)
                    else:
                        print ('Configuration file [%s] is not created exit the shell' % configFile)
                        xpShellExit(3)
            else:
                print('Loading Configuration file [%s]' % configFile)
            #time.sleep(2)
            if gexitmode == 1:
                arg = '1'
            else:
                arg = '0'
            Cmd.add_exit(self, arg)
            try:
                if os.path.exists(configFile):
                    Cmd.loader(self, configFile)
            except KeyboardInterrupt as e:
                print('xpShell terminating, Enter exit command')
                xpShellExit(3)
                return True
            except (ValueError, IndexError, AttributeError), e:
                #print 'Invalid input format or device not initialized, check help <command> or ? for usage'
                pass
            arg = '0'
            Cmd.add_exit(self, arg)
            if gexitmode == 1:
                return True
        while True:
            try:
                return Cmd.cmdloop(self)
            except KeyboardInterrupt as e:
                print('xpShell terminating, Enter exit command')
                xpShellExit(3)
                return True
            except (ValueError, IndexError, AttributeError), e:
                #print 'Invalid input format or device not initialized, check help <command> or ? for usage'
                continue

    #/*****************************************************************************************************/
    # preloop hook
    #/*****************************************************************************************************/
    def preloop(self):
        print

    #/*****************************************************************************************************/
    # postloop hook
    #/*****************************************************************************************************/
    def postloop(self):
        print

    #/*****************************************************************************************************/
    # parse the input arguments
    #/*****************************************************************************************************/
    def parseline(self, arg):
        #print 'parseline(%s) =>' % arg,
        ret = Cmd.parseline(self, arg)
        #print ret
        return ret

    #/*****************************************************************************************************/
    # one command
    #/*****************************************************************************************************/
    def onecmd(self, s):
        #print 'onecmd2(%s)' % s
        return Cmd.onecmd(self, s)

    #/*****************************************************************************************************/
    # empty line command
    #/*****************************************************************************************************/
    def emptyline(self):
        #print 'emptyline()'
        return Cmd.emptyline(self)

    #/*****************************************************************************************************/
    # pre command
    #/*****************************************************************************************************/
    def precmd(self, line):
        #print 'precmd2(%s)' % line
        return Cmd.precmd(self, line)

    #/*****************************************************************************************************/
    # post command
    #/*****************************************************************************************************/
    def postcmd(self, stop, arg):
        #print 'postcmd2(%s, %s)' % (stop, arg)
        return Cmd.postcmd(self, stop, arg)

    #/*****************************************************************************************************/
    # parse command
    #/*****************************************************************************************************/
    def parse(arg):
        'Convert a series of zero or more numbers to an argument tuple'
        return tuple(map(int, arg.split()))


    #/********************************************************************************/
    # command for print spc details
    #/********************************************************************************/
    def do_show_soc_details(self, arg):
        '''
        show_soc_details: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            print('Input Arguments are, devId=%d' % (args[0]))
            mgmtBlockMgr = xpDeviceMgr.instance().getDeviceObj(args[0]).getMgmtBlockMgr()
            xp80MgmtBlockMgr = getXp80MgmtBlockMgrPtr(mgmtBlockMgr)
            if xp80MgmtBlockMgr != None:
                ret = xp80MgmtBlockMgr.getEfuseDataStatus(args[0])
                if ret != 0:
                    print('getEfuseDataStatus Failed with error code = %d' % (ret))
                ret = xp80MgmtBlockMgr.getSerdesFwRevision(args[0])
                if ret != 0:
                    print('getSerdesFwRevision Failed with error code = %d' % (ret))
                xp80Temp = new_floatp()
                ret = xp80MgmtBlockMgr.readXp80TempOverSbus(args[0], xp80Temp)
                if ret != XP_NO_ERR :
                    print "readXp80TempOverSbus Failed with error code %d"%(ret)
                else:
                    print  "XP80 temp (over SBUS) : " + str(floatp_value(xp80Temp)) + " degree celsius"
                delete_floatp(xp80Temp)
            else:
                print "Error : Init mgmt block first"

    #/*****************************************************************************************************/
    # command to print dal cmd2 help
    #/*****************************************************************************************************/
    def printDalHelp(self, arg):
        print 'Invalid input, Enter devId, devType, pipeMode, dalType, mode (mode is relavent only for HW)'
        print '   Dal-Type                     Dal-Mode'
        print '   ========                    ========='
        print 'XP_HARDWARE = 0                PCIE_BUS = 0'
        print 'XP_SHADOW_HARDWARE = 1         I2C_BUS = 1'
        print 'XP_SHADOW_REMOTEWM = 2         MDIO_BUS = 2'
        print 'XP_SHADOW = 3                  I2C_OVER_LPC = 3'
        print 'XP_SHADOW_EMULATOR = 4         MDIO_OVER_LPC = 4'
        print 'XP_DAL_DEBUG = 5'
        print 'XP_DAL_I2C = 6'
        print 'XP_DAL_MDIO = 7'
        print 'XP_DAL_PIPE = 8'
        print 'XP_DAL_DNGL = 9'
        print 'XP_DAL_DUMMY = 10'
        print 'XP_DAL_RDR = 11'
        print 'XP_DAL_DEFAULT = 12'
        print 'XP_DAL_FIFO = 13'

    #/*****************************************************************************************************/
    # sub-commands for xpSim operations
    #/*****************************************************************************************************/
    def xp_sim(self, s):
        'xpSim WhiteModel command operations'
        i = runXpSim()
        i.prompt = self.prompt[:-1]+':xpSim)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # command to load the configuration
    #/*****************************************************************************************************/
    def do_load_config(self, arg):
        'load the configuration file'
        args = arg.split()
        if (len(args) == 0):
            print('Invlaid input, Enter the configuration file')
        else:
            Cmd.loader(self, arg)

    #/*****************************************************************************************************/
    # command to load the configuration
    #/*****************************************************************************************************/
    def do_back(self, arg):
        'xpShell back is a no-operation command'
        #print('Cannot go back from the xpShell root')

    #/*****************************************************************************************************/
    # command to create the configuration
    #/*****************************************************************************************************/
    def do_create_config(self, arg):
        '''
         creates the configuration file from the xpShell output dump;
         Enter the xpShell output file; New file with cmd_ prefix will be created
        '''
        prompt = '(xpShell) '
        args = arg.split()
        if (len(args) == 0):
            print('Invalid input. Enter the xpShell output file; New file with \'.cmd\' postfix will be created')
        else:
            oarg = arg + '.cmd'
            if os.path.isfile(arg):
                ofx = sys.stdout
                fx = open(oarg,'a');
                sys.stdout = fx
                print('#Commands file generated using create_config')
                with open(arg) as f:
                    for line in f:
                        #print ('IN : %s' % (line))
                        if prompt not in line:
                            continue
                        if 'exit ' in line:
                            continue
                        if 'shell vi ' in line:
                            continue
                        line = line.replace(prompt, "")
                        if line in ['\n', '\r\n']:
                            continue
                        if 'load_' in line or 'restore' in line:
                            continue
                        print ('%s' % (line)),
                f.close()
                fx.close()
                sys.stdout = ofx
                print('Commands file %s is generated.' % (oarg))
                print('It can be loaded using \'load_config %s\'' % (arg + '.cmd'))
            else:
                print('File %s not found' % (arg))

    #/*****************************************************************************************************/
    # command to save the configuration
    #/*****************************************************************************************************/
    def do_save_config(self, arg):
        'save the configuration file; default file name is command.txt'
        args = arg.split()
        if (len(args) == 0):
            print('Using default file name, command.txt')
            arg = 'command.txt'
        Cmd.save(self, arg)

    #/*****************************************************************************************************/
    # command to save the running configuration
    #/*****************************************************************************************************/
    def do_save_running_config(self, arg):
        'start/stop : To start/stop running config session'
        '1/0 : 1 to overwrite in running config file\t0 to append in running config file'
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 1:
            print('Invalid input, needs start/stop, 1/0 (overwrite/append)')
            print('start/stop : To start/stop running config session')
            print('1/0 : 1 to overwrite in running config file\t0 to append in running config file')

        else:
            if len(args) < 2:
                # By default, open the runningConfig file in append mode
                arg += " 0 "
            arg += RunningConfig
            Cmd.save_runningConfig(self, arg)

    #/*****************************************************************************************************/
    # restore the loading of configuration
    #/*****************************************************************************************************/
    def do_restore(self, arg=None):
        'restore the loading of configuration file'
        Cmd.restored(self, arg)

    #/*****************************************************************************************************/
    # command to add a device
    #/*****************************************************************************************************/
    def do_add_device(self, arg):
        'Add Device, Enter Device-Id, devType, pipeMode, dalType, dalMode,clkFreq'
        args = arg.split()
        initType = INIT_WARM
        if  len(args) >= 6 :
            devId = int(args[0])
            print 'Initialize the XDK for device %d' % (devId)
            devType = eval(args[1])
            clkFreq = eval(args[5])
            cvar.devDefaultConfig.pipeLineMode = eval(args[2])
            if(devType ==  XP70):
                cvar.devDefaultConfig.mode = SKU_72X10
                cvar.devDefaultConfig.profileType = XP_DEFAULT_SINGLE_PIPE_PROFILE
            else:
                cvar.devDefaultConfig.mode = SKU_128X10 #eval(SKU_128X10)
            cvar.devDefaultConfig.dalType = eval(args[3])
            dalType = eval(args[3])
            xpSetSalType(XP_SAL_WM_TYPE)
            xpSalInit(devType)
            #if dalType == XP_DAL_DUMMY:
            #    initType = INIT_WARM
            #cvar.devDefaultConfig.dalMode = int(args[4])
            cvar.devDefaultConfig.clkFreq = XP_CORE_CLK_FREQ_550_MHZ
            modNames = xpInitMgr.instance().xpLoggerInit();
            ret = xpInitMgr.instance().xpLogToConsole();
            ret = xpAllocatorMgr.instance(0).init(cvar.xpDefRangeProfile1Pipe)
            ret = xpInitMgr.instance().init();
            ret = xpAllocatorMgr.instance(0).initAllocators(cvar.xpDefRangeProfile1Pipe)
            ret = xpInterfaceMgr.instance().init(cvar.xpDefRangeProfile1Pipe);
            ret = xpEgressFilterMgr().init();
            ret = xpStpMgr.instance().init();
            ret = xpL2DomainMgr.instance().init();
            ret = xpLagMgr.instance().init();
            ret = xpL2LearnMgr.instance().init();
            ret = xpIpHostMgr.instance().init();
            ret = xpIpv4RouteMgr.instance().init();
            ret = xpIpNhMgr.instance().init();
            ret = xpCtrlMacMgr.instance().init();
            ret = xpMtuProfileMgr.instance().init();
            ret = xpMirrorMgr.instance().init();
            ret = xpMulticastMgr.instance().init();
            ret = xpTunnelMgr.instance().init();
            ret = xpMplsRouteMgr.instance().init();
            ret = xpAcmMgr.instance().init();
            ret = xpAclMgr.instance().init();
            ret = xpEaclMgr().init();
            ret = xpControlPlanePolicingMgr.instance().init();
            ret = xpPolicerMgr.instance().init();
            ret = xpAqmMgr.instance().init();
            ret = xpNatMgr.instance().init();
            ret = xpOfMgr.instance().init();
            ret = xpGlobalSwitchControlMgr.instance().init();
            ret = xpCpuStormControlMgr.instance().init();
            ret = xpShaperMgr.instance().init();
            ret = xpSchedulerMgr.instance().init();
            ret = xpQueueCounterMgr.instance().init();
            ret = xpSal.instance().getBufferManager().init();
            ret = xpFlowControlMgr.instance().init();
            ret = xpPortIngressQosMgr.instance().init();

            print 'XDK Init Done'

            cvar.devDefaultConfig.simulation = 1
            ret = xpInitMgr.instance().addDevice(devId, initType, cvar.devDefaultConfig);
            xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().setMgmtLocalReset(1)
            print 'XDK Device added'

        else:
            print 'Invalid Input, Enter Device-Id, devType, pipeMode, dalType, dalMode, clkFreq'
            print 'devType               Pipe Mode                  Core clock Frequency'
            print '====================  ====================       ===================='
            print 'XP80 - 0, XP80B0 - 1  XP_SINGLE_PIPE             XP_CORE_CLK_FREQ_400_MHZ'
            print 'XP70 - 2              XP_TWO_PIPE                XP_CORE_CLK_FREQ_500_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_550_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_600_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_633_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_650_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_683_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_700_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_733_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_750_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_783_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_800_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_833_MHZ'
            print '                                                 XP_CORE_CLK_FREQ_850_MHZ'
            self.printDalHelp(arg)

    #/*****************************************************************************************************/
    # command to remove a device
    #/*****************************************************************************************************/
    def do_remove_device(self, arg):
        'Remove device, Enter Device-Id'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1 :
            print 'Invalid input, Enter dev-id'
        else:
            xpInitMgr.instance().removeDevice(args[0]);
            print 'Device %d Removed' % (args[0])

    #/*****************************************************************************************************/
    # quit
    #/*****************************************************************************************************/
    def quit(self, arg):
        #'Close the xpShell window, and exit'
	if isNetlinkEn == 1:
	 	xpshellNlSockDeInit()
		#print('\nDeInit xpshellNL ')

	print('xpShell terminating, Enter exit command')
        global gquitexit
        gquitexit = 1
        xpShellExit(1)
        return True

    #/*****************************************************************************************************/
    # exit 0 / 1
    #/*****************************************************************************************************/
    def do_exit(self, arg):
        'Close the xpShell window, and exit; if arg zero it is re-launched else die'
        args = tuple(map(int, arg.split()))
        global gquitexit
        gquitexit = 1

        if(xpShellGlobals.adminFlag == 1):
            xpConfigPipeSidRd = '/tmp/npiper' + xpShellGlobals.rootUser + str(0)
            wp = open(xpConfigPipeSidRd, 'w+')
            wp.write(str(EXIT_POINT + int(xpShellGlobals.sessionId)) + '\n')
            wp.flush()
            wp.close()
            #print "Client/Admin exit with session id ::" , xpShellGlobals.sessionId

        if  len(args) < 1:
           #print 'Invalid input, if arg zero it is re-launched else die'
           self.quit(arg)
           return True
        else:
            if xpShellGlobals.disableLog == 0:
                print('xpShell Exit')
            global redirout

            if xpShellGlobals.sessionFlag == 1:
                wp = sys.stdout
                sys.stdout = redirout
                wp.close()
                xpShellGlobals.sessionFlag = 0
                obj = runLog()
                runLog().do_enable_log('console')
                del obj
                pyDalWrite(xpShellGlobals.sessionId, 1)
            xpShellExit(args[0])
            return True

    #/*****************************************************************************************************/
    # switch to CPSS console
    #/*****************************************************************************************************/
    def do_lua_cli(self, arg):
        'Launch the LUA Cli for CPSS'
        os.system('telnet 127.0.0.1 12345')
        pass

#/*********************************************************************************************************/
# the command loop of xpShell for xpSim.
#/*********************************************************************************************************/
class xpSimObj(Cmd):

    # banner and prompt for the xpShell
    intro = 'Launching xpShell for xpSim. Type help or ? to list commands.\n'
    prompt = '(xpShell) '
    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    # ----- basic xpShell commands -----

    #/*****************************************************************************************************/
    # command to print version information of XDK
    #/*****************************************************************************************************/
    def do_show_version(self, arg):
        xpShellObj().do_show_version(arg)

    #/*****************************************************************************************************/
    # sub-commands for xpSim operations
    #/*****************************************************************************************************/
    def do_xp_sim(self, s):
        xpShellObj().xp_sim(s)

    #/*****************************************************************************************************/
    # sub-commands for utility functions
    #/*****************************************************************************************************/
    def do_utils(self, s):
        xpShellObj().do_utils(s)

    #/*****************************************************************************************************/
    # sub-commands for register/table operation functions
    #/*****************************************************************************************************/
    def do_reg_access(self, s):
        xpShellObj().do_reg_access(s)

    #/*****************************************************************************************************/
    # sub-commands for dal debug operations
    #/*****************************************************************************************************/
    def do_dal_debug(self, s):
        xpShellObj().do_dal_debug(s)

    #/*****************************************************************************************************/
    # sub-commands for dal debug operations
    #/*****************************************************************************************************/
    def do_debug(self, s):
        xpShellObj().do_debug(s)

    #/*****************************************************************************************************/
    # sub-commands for diags operations
    #/*****************************************************************************************************/
    def do_diags(self, s):
        xpShellObj().do_diags(s)

    #/*****************************************************************************************************/
    # sub-commands for test scripts operations
    #/*****************************************************************************************************/
    def do_test_scripts(self, s):
        xpShellObj().do_test_scripts(s)


    #/*****************************************************************************************************/
    # sub-commands for test scripts operations
    #/*****************************************************************************************************/
    def do_test_scripts(self, s):
        xpShellObj().do_test_scripts(s)

    #/*****************************************************************************************************/
    # command loop and check for exception
    #/*****************************************************************************************************/
    def cmdloop(self, intro=None):
        #print 'cmdloop(%s)' % intro
        while True:
            try:
                return Cmd.cmdloop(self)
            except KeyboardInterrupt as e:
                print('xpShell terminating, Enter exit command')
                xpShellExit(3)
                return True
            except (ValueError, IndexError, AttributeError), e:
                #print 'Invalid input format or device not initialized, check help <command> or ? for usage'
                continue

    #/*****************************************************************************************************/
    # preloop hook
    #/*****************************************************************************************************/
    def preloop(self):
        print

    #/*****************************************************************************************************/
    # postloop hook
    #/*****************************************************************************************************/
    def postloop(self):
        print

    #/*****************************************************************************************************/
    # parse the input arguments
    #/*****************************************************************************************************/
    def parseline(self, arg):
        #print 'parseline(%s) =>' % arg,
        ret = Cmd.parseline(self, arg)
        #print ret
        return ret

    #/*****************************************************************************************************/
    # one command
    #/*****************************************************************************************************/
    def onecmd(self, s):
        #print 'onecmd2(%s)' % s
        return Cmd.onecmd(self, s)

    #/*****************************************************************************************************/
    # empty line command
    #/*****************************************************************************************************/
    def emptyline(self):
        #print 'emptyline()'
        return Cmd.emptyline(self)

    #/*****************************************************************************************************/
    # pre command
    #/*****************************************************************************************************/
    def precmd(self, line):
        #print 'precmd2(%s)' % line
        return Cmd.precmd(self, line)

    #/*****************************************************************************************************/
    # post command
    #/*****************************************************************************************************/
    def postcmd(self, stop, arg):
        #print 'postcmd2(%s, %s)' % (stop, arg)
        return Cmd.postcmd(self, stop, arg)

    #/*****************************************************************************************************/
    # parse command
    #/*****************************************************************************************************/
    def parse(arg):
        'Convert a series of zero or more numbers to an argument tuple'
        return tuple(map(int, arg.split()))

    #/*****************************************************************************************************/
    # command to read statistics for given port
    #/*****************************************************************************************************/
    def do_get_stats(self, arg):
        xpShellObj().do_get_stats(arg)

    #/*****************************************************************************************************/
    # quit
    #/*****************************************************************************************************/
    def quit(self, arg):
        #'Close the xpShell window, and exit'
        print('xpShell terminating, Enter exit command')
        xpShellExit(1)
        return True

    #/*****************************************************************************************************/
    # exit 0 / 1
    #/*****************************************************************************************************/
    def do_exit(self, arg):
        'Close the xpShell window, and exit; if arg zero it is re-launched else die'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            #print 'Invalid input, if arg zero it is re-launched else die'
            self.quit(arg)
            return True
        else:
            print('xpShell terminating, Enter exit command')
            xpShellExit(args[0])
            return True

def usage():
    print '\n##### USAGE #####'
    print '1. mode'
    print 'User can run any of this mode at a time ["--standalone", "--kernel", "--inmode", "--connect", "--withwm", "--xpSim", "--default", "--verification"]'
    print 'e.g python cli/xpShell.py --standalone'
    print '2. IPC ports'
    print 'Use -z and -y to pass socket ports, these options are only valid for withwm mode'
    print 'e.g python cli/xpShell.py --withwm -z 1025 -y 1026'
    print '3. xpShell config file'
    print 'Use -n and path of configuration file to load with xpShell'
    print 'If exit command is placed at end of config file, xpShell will exit after configuration is done'
    print '5. Client session-id'
    print 'Use -s and number for client session-id'
    print '6. Device type'
    print 'Use -g and device type | this option is supported for standalone mode, inmode mode and kernel mode'
    print 'e.g python cli/xpShell.py --standalone -g xp80'
    print '7. Pipeline Mode'
    print 'Use -p and Pipeline mode | this option is supported for standalone'
    print 'e.g. python cli/xpShell.py --standalone -p XP_SINGLE_PIPE'
    print '8. Core clock frequency'
    print 'Use -f and core clock frequency | this option is supported for standalone'
    print 'e.g. python cli/xpShell.py --standalone -f XP_CORE_CLK_FREQ_550_MHZ'
    print '9. xpApp arguments'
    print 'Use -x and pass xpApp arguments in double quotes | this option is supported for inmode, kernel & stanalone mode'
    print 'e.g python cli/xpShell.py --kernel -x "-t crb1-rev2 -c config.txt -m 4x10G -M INTR -s XP_DEFAULT_SINGLE_PIPE_PROFILE"'
    print '10. Verification mode Arguments'
    print 'Use -d option for Run xpShell Automation for xpDiags and Use -a option to Generate all commands for xpShell Automated with --verification mode'
    print 'e.g python cli/xpShell.py --verification'
    print 'Use -x and pass xpApp arguments in double quotes | this option is supported --standalone only'
    print 'e.g python cli/xpShell.py --standalone -x "-t crb1-rev2 -c config.txt -m 4x10G -M INTR -s XP_DEFAULT_SINGLE_PIPE_PROFILE"'
    print '11. Help'
    print 'use -h or --help for usage'
    sys.exit()

# Parse command line
def parseCmdLine(argv):
    try:
        (opts, args) = getopt.getopt(argv, 'hz:y:n:s:g:p:f:x:a:d', ["help", "standalone", "inmode", "kernel", "connect", "withwm", "xpSim", "default", "verification"])
    except getopt.GetoptError:
        usage()

    if(len(argv) > 0 and len(opts) == 0):
        print("argv = %s, %d, %d" % (argv, len(argv), len(opts)))
        usage()
    mode = ""
    global isDiagMode
    isDiagMode = 0
    socketPort = None
    interruptPort = None
    configFile = 'None'
    sId = None
    deviceType = 'XP70'
    pipeMode = 'XP_SINGLE_PIPE'
    clkFreq = 'XP_CORE_CLK_FREQ_550_MHZ'
    deviceTypeList = ['XP80B0', 'XP80', 'XP70']
    pipeModesList = ['XP_SINGLE_PIPE','XP_TWO_PIPE']
    coreClkFreqList = ['XP_CORE_CLK_FREQ_400_MHZ','XP_CORE_CLK_FREQ_500_MHZ','XP_CORE_CLK_FREQ_550_MHZ','XP_CORE_CLK_FREQ_600_MHZ','XP_CORE_CLK_FREQ_633_MHZ','XP_CORE_CLK_FREQ_650_MHZ','XP_CORE_CLK_FREQ_683_MHZ','XP_CORE_CLK_FREQ_700_MHZ','XP_CORE_CLK_FREQ_733_MHZ','XP_CORE_CLK_FREQ_750_MHZ','XP_CORE_CLK_FREQ_783_MHZ','XP_CORE_CLK_FREQ_800_MHZ','XP_CORE_CLK_FREQ_833_MHZ','XP_CORE_CLK_FREQ_850_MHZ']
    xpAppArgs = ""

    for (opt, arg) in opts:
        if opt in ('-h', '--help'):
            usage()
        elif opt in '-a':
            print "Generating Commands for Verification"
            os.system('cd cli/AutoxpShell/;python genAllCmd.py ../../cli;python genAllCmd.py ../../pipeline-profiles/xpDefault/cli/xps/;python genAllCmd.py ../../device/sdksrc/xp/xpDiag/;cd -')
        elif opt in '-z':
            options = dict(opts)
            if options.has_key('--withwm') == False:
                print '-z option is supported for --withwm mode only'
                usage()
            else :
                socketPort = int(arg)
        elif opt in '-y':
            if options.has_key('--withwm') == False:
                print '-y option is supported for --withwm mode only'
                usage()
            else :
                interruptPort = int(arg)
        elif opt in '-n':
            configFile = arg.strip()
        elif (opt in '-g'):
            options = dict(opts)
            if options.has_key('--standalone') == False and options.has_key('--withwm') == False and options.has_key('--kernel') == False and options.has_key('--connect') == False:
                print '-g option is only supported for --standalone, --withwm and --kernel mode'
                usage()
            else :
                if str(arg).upper() in deviceTypeList:
                    deviceType = str(arg).upper()
                    print "device Type : ",deviceType
                else:
                    print 'WARN : Invalid device type passed. Continuing with default device type : ', deviceType
        elif opt in '-s':
            sId = int(arg)
        elif (opt.strip() in ("--standalone", "--inmode", "--kernel", "--connect", "--withwm", "--xpSim", "--default", "--verification")):
            mode = opt.strip("-")
            options = dict(opts)
        elif opt in '-d':
            if mode == "verification":
                isDiagMode = 1
                os.system('echo "load_config cli/AutoxpShell/xpShellCmds/xpDiagsCmd.txt" > tmp')
        elif opt in '-p':
            options = dict(opts)
            if options.has_key('--standalone') == False :
                print '-p option is only supported for --standalone'
                usage()
            else:
                if(str(arg) in pipeModesList):
                    pipeMode = str(arg)
                else:
                    print 'WARN : Invalid Pipe mode. Continuing with default frequency: ', pipeMode
        elif opt in '-f':
            options = dict(opts)
            if options.has_key('--standalone') == False :
                print '-f option is only supported for --standalone'
                usage()
            else:
                if(str(arg) in coreClkFreqList):
                    clkFreq = str(arg)
                else:
                    print 'WARN : Invalid core clock frequency. Continuing with default frequency: ', clkFreq
        elif opt in '-x':
            options = dict(opts)
            if ((options.has_key("--kernel") == False) and (options.has_key("--inmode") == False) and (options.has_key("--standalone") == False)):
                print '-x is supported for --kernel, --inmode  & --standalone only'
                usage()
            xpAppArgs = arg
    return mode, socketPort, interruptPort, configFile, sId, deviceType, pipeMode, clkFreq, xpAppArgs

def receiveTrapPacket():
    while True:
        xpAppReceiveTrapPacket()
        time.sleep(1)

def getRpType():

    rpType = RANGE_PROFILE_MAX
    if((cvar.xpAppConf.pipeLineNum == 2) and ((cvar.xpAppConf.profileNum == XP_DEFAULT_TWO_PIPE_PROFILE) or (cvar.xpAppConf.profileNum == XP_FDB_128K_TWO_PIPE_PROFILE))):
        rpType = RANGE_FULL_FEATURED_2
    elif(cvar.xpAppConf.pipeLineNum == 1):
        if((cvar.xpAppConf.profileNum == XP_DEFAULT_SINGLE_PIPE_PROFILE) or (cvar.xpAppConf.profileNum == XP_OPENFLOW_HYBRID_SINGLE_PIPE_PROFILE)):
            rpType = RANGE_FULL_FEATURED
        elif ((cvar.xpAppConf.profileNum == XP_DEFAULT_TWO_PIPE_PROFILE) or (cvar.xpAppConf.profileNum == XP_FDB_128K_TWO_PIPE_PROFILE)):
            rpType = RANGE_FULL_FEATURED_2
        elif (cvar.xpAppConf.profileNum == XP_TUNNEL_CENTRIC_SINGLE_PIPE_PROFILE):
            rpType = RANGE_TUNNEL_CENTRIC
        elif (cvar.xpAppConf.profileNum == XP_SEGMENT_ROUTE_SINGLE_PIPE_PROFILE):
            rpType = RANGE_SEGMENT_ROUTE
        else:
            rpType = RANGE_ROUTE_CENTRIC
    return rpType


#/*********************************************************************************************************/
#/* main program */
#/*********************************************************************************************************/

if __name__ == '__main__':
    #/*****************************************************************************************************/
    #Init/addDevice for Device manager and tableManager
    # Initialize only of we are running in standalone mode
    #init and redirect logger prints to console
    #/*****************************************************************************************************/
    mode, socketPort, interruptPort, configFile, sId, devType, pipeMode, clkFreq, xpAppArgs = parseCmdLine(sys.argv[1:])
    obj = xpShellObj()
    gexitmode = 0
    global shellmode
    global redirout,isDiagMode
    shellmode = 'None'

    userId = str(os.getuid())
    username = 'root'

    global redirout
    checkFlag = 0
    userName = ''
    xpShellGlobals.globalInit()

    userId = str(os.getuid())
    userName = pwd.getpwuid( os.getuid() ).pw_name

    try:
        if (userName == xpShellGlobals.adminUser):
            xpShellGlobals.adminFlag = 1
            userName = xpShellGlobals.rootUser
            for currSessionId in range(8):
                xpConfigPipeSidRd = '/tmp/npiper' + userName + str(0)
                xpConfigPipeSidWr = '/tmp/npipew' + userName + str(currSessionId)
                if os.access(xpConfigPipeSidRd, os.R_OK):
                    st = os.stat(xpConfigPipeSidRd)
                    if ((st.st_mode & stat.S_IWGRP) and (st.st_mode & stat.S_IRGRP) and (st.st_mode & stat.S_IXGRP)):                
                        wp = open(xpConfigPipeSidRd, 'w+')
                        wp.write(str(ENTRY_POINT + currSessionId) + '\n')
                        wp.flush()
                    else:
                        print "\nSORRY, SERVER IS NOT STARTED PLEASE TRY AFTER SOMETIME\n"
                        quit(0)
                else:
                    print "\nSORRY, SERVER IS NOT STARTED PLEASE TRY AFTER SOMETIME\n"
                    quit(0)
                if os.access(xpConfigPipeSidWr, os.R_OK):
                        st = os.stat(xpConfigPipeSidWr)
                        if ((st.st_mode & stat.S_IWGRP) and (st.st_mode & stat.S_IRGRP) and (st.st_mode & stat.S_IXGRP)):
                            wp = open(xpConfigPipeSidWr, 'r')
                            var = wp.read()
                            wp.close()
                        else:
                            print "\nSORRY, SERVER IS NOT STARTED PLEASE TRY AFTER SOMETIME\n"
                            quit(0)
                else:
                    print "\nSORRY, SERVER IS NOT STARTED PLEASE TRY AFTER SOMETIME\n"
                    quit(0)
                var = int(var.split('\n')[0])
                if ((var == NEGATIVE_FEEDBACK) & ( currSessionId < 7)):
                    continue
                if((var >= (ENTRY_POINT + MIN_CLIENT_ID)) & (var <= (ENTRY_POINT + MAX_CLIENT_ID))):
                    sId = var - ENTRY_POINT
                    xpShellGlobals.sessionId = sId
                    checkFlag = 1
                    break
                if(currSessionId == MAX_CLIENT_ID):
                    print "\nSORRY, MAXIMUM CLIENTS ARE CONNECTED TO THE SERVER; PLEASE TRY AFTER SOMETIME\n"
                    quit(0)
    except KeyboardInterrupt:
        print "\nSORRY, Exit the Shell due to KeyboardInterrupt\n"
        exit(1)
    except ValueError:
        print "\nSORRY, Exit the Shell due to Error\n"
        if (checkFlag == 1):
            exit(1)
        else:
            quit(0)

    obj.do_restore()
    xpShellGlobals.sessionId = sId
    if ((xpShellGlobals.sessionId != None) or ('xpConflg' in configFile)):
        if (xpShellGlobals.sessionId == None):
            xpShellGlobals.sessionFlag = 1
            tmpstr = '/tmp/xpConflg' + userName
            xpShellGlobals.sessionId = int(configFile.replace(tmpstr, ''))
            #print("New sessionId = %d" % (xpShellGlobals.sessionId))
        else:
            xpShellGlobals.sessionFlag = 2
        xpShellGlobals.xpConfigFileRd = '/tmp/xpConflg' + userName + str(xpShellGlobals.sessionId)
        xpShellGlobals.xpConfigFileWr = '/tmp/xpConflg' + userName + str(xpShellGlobals.sessionId) + 'w'

        if (xpShellGlobals.sessionFlag == 1):
            objx = runLog()
            #print('Now enabling the file log %s' % (xpShellGlobals.xpConfigFileWr))
            objx.do_log_file(xpShellGlobals.xpConfigFileWr)
            del objx
            redirout = sys.stdout

            if (userName == xpShellGlobals.adminUser):
                os.chmod(xpShellGlobals.xpConfigFileWr, 0666)
                os.chown(xpShellGlobals.xpConfigFileWr, pwd.getpwnam(xpShellGlobals.adminUser).pw_uid, grp.getgrnam(xpShellGlobals.adminUser).gr_gid)
            if os.path.exists(xpShellGlobals.xpConfigFileWr):
                with open(xpShellGlobals.xpConfigFileWr, "w"):
                    pass
            else:
                os.create(xpShellGlobals.xpConfigFileWr)
            # Open file in a+ mode with unbuffered mode to overcome output truncation issue
            sys.stdout = open(xpShellGlobals.xpConfigFileWr, 'a+',0)
    else:
        xpShellGlobals.xpConfigFileRd = ''
        xpShellGlobals.xpConfigFileWr = ''
        xpShellGlobals.sessionFlag = 0
        xpShellGlobals.sessionId = None
        xpShellGlobals.noPrompt = 0

    #print('configFile = %s' % configFile)
    if  mode != "" :
        if mode == 'standalone':
            if(('ksdk' in sys.modules.keys()) or (('sdk' not in sys.modules.keys())  and ('xpsApp' not in sys.modules.keys()))):
                print '\n --standalone mode requires to import sdk or xpsApp module\n'
                sys.exit()

            redirout = sys.stdout
            configFlag = 0
            lFile = '/tmp/xpTmpLogFile'
            if configFile != 'None':
                cvar.xpAppConf.disableLog = 1
                xpShellGlobals.disableLog = 1
                configFlag = 1
                if not os.path.exists(os.path.dirname(lFile)):
                    os.makedirs(os.path.dirname(lFile))
                if not os.path.exists(lFile):
                    open(lFile, 'w').close()
            else:
                print 'Initialize the XDK with xpShell Standalone '

            if('xpsApp' in sys.modules.keys()):
                # print "\n=====> Running with xpsApp_py so -x option will override -p and -f options, if used. <=====\n"
                xpSetSalType(XP_SAL_WM_TYPE)
                xpSalInit(eval(devType))
                xpLoggerInit()
                cvar.xpAppConf.packetInterface = XP_DMA
                cvar.xpAppConf.withHw = None
                xpAppArgs = "xpApp " + str(xpAppArgs)
                xpAppArgs = xpAppArgs.split()
                getOptValues(len(xpAppArgs), xpAppArgs, None)
                xpInitXpLoggerConnect()
                xpsAppInit(getRpType(), cvar.xpAppConf.initType, INIT_WARM)
                xpAppDevDefaultConfigSet()
                cvar.devDefaultConfig.dalType = XP_DAL_DUMMY
                xpsAppInitDevice(0, cvar.xpAppConf.initType, cvar.devDefaultConfig)
            else:
                #Default: as before
                #print "\n=====> Running with sdk_py, so ignoring -x option & its corresponding parameters <=====\n"
                obj.do_add_device('0 %s %s XP_DAL_DUMMY 0 %s'%(devType, pipeMode, clkFreq))

            if configFile == 'None':
                print('XDK initiated.')
            runDalDebug().do_set_dal_type('0 0 0 0')

            sys.argv = ['xpShell.py']
            if configFile == 'None':
                print('\nLaunching xpShell. Type help or ? to list commands.')
                obj.do_xp_logo(0)
            else:
                xpShellGlobals.noPrompt = 1
            obj.cmdloop(configFile)

            sys.stdout = redirout
            xpShellGlobals.noPrompt = 0

        elif mode == 'verification':
            print "Initialize the xpShell for Verification of All commands"
            if isDiagMode == 1:
                os.system('python cli/AutoxpShell/validation.py tmp')
            else:
                os.system('python cli/AutoxpShell/validation.py cli/AutoxpShell/xpShellCmds/xpShellLoadCmds.txt')
            sys.exit()
        elif mode == 'kernel' or mode == 'inmode':
            # XP_STATUS xpAppWarmInit(int devType, int redirect, int configFile, int argc, char* argv[], char *logFile)
            redirout = sys.stdout
            configFlag = 0
            lFile = '/tmp/xpTmpLogFile'
            shellmode = mode
            if configFile != 'None':
                if mode == 'kernel':
                    cvar.xpAppConf.disableLog = 1
                xpShellGlobals.disableLog = 1
                configFlag = 1
                if not os.path.exists(os.path.dirname(lFile)):
                    os.makedirs(os.path.dirname(lFile))
                if not os.path.exists(lFile):
                    open(lFile, 'w').close()
            xpAppArgs = "xpApp " + str(xpAppArgs)
            xpAppArgs = xpAppArgs.split()

            if mode == 'kernel':
                if configFile == 'None':
                    print 'Initialize the XDK with xpShellnetlink '
                if ('ksdk' in sys.modules.keys()):
                    xpAppWarmInit(eval(devType), len(xpAppArgs), xpAppArgs)
                xpshellNlSockInit();
            if configFile == 'None' and mode == 'kernel':
                print("xpshellNetlink initiated\n");

            if mode == 'kernel':
                isNetlinkEn = 1
            sys.argv = ['xpShell.py']

            lobj = runLog()
            if configFile == 'None':
                print('\nLaunching xpShell. Type help or ? to list commands.')
                obj.do_xp_logo(0)
            else:
                #lobj.do_log_file(lFile)
                #sys.stdout = open(lFile, 'w+')
                xpShellGlobals.noPrompt = 1
            obj.cmdloop(configFile)

            #if configFile != 'None':
            #    lobj.do_enable_log('console')
            #    sys.stdout.close()
            sys.stdout = redirout
            xpShellGlobals.noPrompt = 0
            del lobj

            #if configFile != 'None':
            #    with open(lFile) as f:
            #        for line in f:
            #            print line,
        else:
            if mode == 'connect':
                obj.do_add_device('0 %s XP_SINGLE_PIPE XP_SHADOW 0 XP_CORE_CLK_FREQ_550_MHZ'%(devType))
                runDalDebug().do_set_dal_type('0 0 3 0')
                sys.argv = ['xpShell.py']
                print('\nLaunching xpShell. Type help or ? to list commands.')
                obj.do_xp_logo(0)
                obj.cmdloop(configFile)
            else:
                if mode == 'withwm':
                    if('xpsApp' not in sys.modules.keys()):
                        print '\n --withwm mode requires to import xpsApp module\n'
                        sys.exit()
                    #xpLoggerInit()
                    #xpInitXpLoggerConnect()
                    #xpAppXdkInit(cvar.xpAppConf.pipeLineNum, cvar.xpAppConf.profileNum)
                    # sdkSetupSignals()
                    if socketPort != None:
                        cvar.xpAppConf.socketPort = socketPort
                    if interruptPort != None:
                        cvar.xpAppConf.interruptPort = interruptPort
            	    xpSetSalType(XP_SAL_WM_TYPE)
                    xpSalInit(eval(devType))
                    xpsSdkLoggerInit()
                    xpsSdkLogConsole()
                    xpsAppInit(0, 1)
                    print('XDK initiated.')
                    xpAppIpcInit(cvar.xpAppConf.socketPort, cvar.xpAppConf.interruptPort)
                    print('Waiting for xpSim to start...')
                    xpAppIpcWait();
                    print('xpSim connected!')
                    thread.start_new_thread (receiveTrapPacket, ())
                    sys.argv = ['xpShell.py']
                    print('\nLaunching xpShell. Type help or ? to list commands.')
                    obj.do_xp_logo(0)
                    obj.cmdloop(configFile)
                else:
                    # check if launched from xpSim
                    if mode == 'xpSim':
                        sys.argv = ['xpShell.py']
                        print('\nLaunching xpShell. Type help or ? to list commands.')
                        obj.do_xp_logo(0)
                        obj.cmdloop(configFile)
                    else :
                       sys.argv = ['xpShell.py']
                       print('\nLaunching xpShell.   Type help or ? to list commands.')
                       obj.do_xp_logo(0)
                       if mode == 'default':
                           gexitmode = 1
                       obj.cmdloop(configFile)
    else :
        print('\nLaunching xpShell. Type help or ? to list commands.')
        obj.do_xp_logo(0)
        obj.cmdloop(configFile)

    quitexit = gquitexit
    while quitexit == 0:
        quitexit = gquitexit
        obj.cmdloop('None')
        quitexit = gquitexit
    del obj
    # done with the shell
    if xpShellGlobals.sessionFlag != 0:
        print '\n+++++++++++ xpShell Exit ++++++++++++\n'

#/*********************************************************************************************************/
#/* End of main program */
#/*********************************************************************************************************/



