#!/usr/bin/env python
#  regAccess.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

#/*********************************************************************************************************/
# import cmd2 package. Ideally we want cmd2 package; while that is being installed, we use cmd2.
#/*********************************************************************************************************/
#from cmd import Cmd
import sys
import time
import os
import re
import readline
import re
import pwd

dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd

#/*********************************************************************************************************/
# import everything from buildTarget
from buildTarget import *

#pipe access
pipeAccess = 0
cpldSimAccess = 0
inface = 'PCIE'

userId = str(os.getuid())
username = 'root'
username = pwd.getpwuid( os.getuid() ).pw_name

#/*********************************************************************************************************/
# The class object for driver over pipe operations
#/*********************************************************************************************************/
class runDrvPipe(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to init the named pipe
    #/*****************************************************************************************************/
    def do_init_pipe(self, arg):
       '''
        Initialize the named pipe, Enter device-id and interface name: Enter [ devId,interfaceName ]
       '''
       global pipeAccess,username
       args = arg.split()
       if  len(args) < 2:
            print 'Invalid input, Enter its Device-Id and interface name'
       else:
            try:
                global inface
                fpaths = '/tmp/npipes' + args[0] + username
                fpathv = '/tmp/npipev' + args[0] + username
                inface = args[1]
                print ' pipe path = %s - %s' % (fpaths, fpathv)
                os.mkfifo(fpaths, 0777)
                os.mkfifo(fpathv, 0777)
                nstr = tuple(map(ord, map(str, args[1])))
                lenx = len(nstr)
                for i in range(0, lenx):
                    setPyInput(i, nstr[i]);
                devId = int(args[0])
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpSetPipeDalType(devId, cvar.pyInput)
            except OSError:
                print 'Error creating named pipe'
            # leave it zero now we do pipe access in special xpDalPipe inside xdk
            pipeAccess = 0
            print 'Done'

    # command to de-init the named pipe
    #/*****************************************************************************************************/
    def do_deinit_pipe(self, arg):
        '''
         De-Initialize the named pipe, Enter device-id: Enter [ devId ]
        '''
        global pipeAccess
  	global username
        args = arg.split()
        if  len(args) < 1:
             print 'Invalid input, Enter its Device-Id'
        else:
             try:
                 fpath = '/tmp/npipes' + args[0] + str(username)
                 os.remove(fpath)
                 fpath = '/tmp/npipev' + args[0] + str(username)
                 os.remove(fpath)
             except OSError:
                 print 'Error deleting named pipe'
             devId = int(args[0])
             xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpSetHwShDalType(devId)
             pipeAccess = 0
             print 'Done'

    # command to enable/disable pipe access
    #/*****************************************************************************************************/
    def get_cpld_sim_access(self):
        ' Gets cpld sim access '
        global cpldSimAccess
        return cpldSimAccess

    # command to enable/disable pipe access
    #/*****************************************************************************************************/
    def do_enable_pipe_access(self, arg):
        '''
         Set or reset pipeAccess, Enter devId, pipeAccess (0 or 1)  and interface name (PCIE/MDIO/I2C): Enter [ devId,pipeAccess,interfaceName ]
        '''
        global pipeAccess
        global cpldSimAccess
        global inface
        args = arg.split()
        if  len(args) < 3:
             print 'Invalid input, Enter devId, pipe Access (0 or 1) and interface name (PCIE/MDIO/I2C)'
        else:
            devId = int(args[0])
            pipeAccess = int(args[1])
            inface = args[2]

            if pipeAccess == 0:
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpSetHwShDalType(devId)
                cpldSimAccess = 0
            else:
                nstr = tuple(map(ord, map(str, args[2])))
                lenx = len(nstr)
                cpldSimAccess = 1
                print 'PipeAccess set to %d' % pipeAccess
                for i in range(0, lenx):
                    setPyInput(i, nstr[i]);
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpSetPipeDalType(devId, cvar.pyInput)
            print 'PipeAccess set to %d' % pipeAccess
            # leave it zero now we do pipe access in special xpDalPipe inside xdk
            pipeAccess = 0

    #/*****************************************************************************************************/
    # command to write to the named pipe
    #/*****************************************************************************************************/
    def do_write_pipe(self, arg):
       '''
        Write a Register from pipe driver, Enter its Device-Id, regOffset (hex), wordWidth: Enter [ devId,regOffset,wordWidth ]
       '''
       args = arg.split()
       global username
       if  len(args) < 3:
           print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth'
       else:
            devId = int(args[0])
            offset = int(args[1], 16)
            width = int(args[2])

            print('Input Pipe-In Arguments are devId=%d, regOffset=0x%x, width=%d' % (devId, offset, width))
            valArgs = raw_input('Enter the Value in %d word(s) in Hex, [word1, word2, ..., wordn] : ' % width)
            inArgs = valArgs

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").strip("]\n").split(",")
            listLen = len(postList)

            if listLen == width:
                global inface
                fpaths = '/tmp/npipes' + args[0] + str(username)
                wp = open(fpaths, 'w+')
                entry = 'W ' + inface + ' ' + args[0] + ' ' + args[1] + ' ' + args[2] + ' ' + valArgs + '\n'
                wp.write(entry)
                wp.close()
                print('Reg entry written')
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))


    #/*****************************************************************************************************/
    # command to write to the named pipe with input
    #/*****************************************************************************************************/
    def do_write_pipe_in(self, arg):
       '''
        Write a Register from pipe driver: Enter [ devId,regOffset,wordWidth,wordNum ]
       '''
       args = arg.split()
       global username
       if  len(args) < 4:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth, [word1, word2, ...wordn]'
       else:
            devId = int(args[0])
            offset = int(args[1], 16)
            width = int(args[2])
            valArgs = args[3]

            print('Input Pipe Arguments are devId=%d, regOffset=0x%x, width=%d' % (devId, offset, width))
            inArgs = valArgs
            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                global inface
                fpaths = '/tmp/npipes' + args[0] + str(username)
                wp = open(fpaths, 'w+')
                entry = 'W ' + inface + ' ' + args[0] + ' ' + args[1] + ' ' + args[2] + ' ' + valArgs
                wp.write(entry)
                wp.close()
                print('Reg entry written')

                fpathv = '/tmp/npipev' + args[0] + str(username)
                rp = open(fpathv, 'r')
                response = rp.read()
                rp.close()
                print response
                return response


            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))

    #/*****************************************************************************************************/
    # command to read the named pipe
    #/*****************************************************************************************************/
    def do_read_pipe(self, arg):
       '''
        Read a Register from pipe driver: Enter [ devId,regOffset,wordWidth ]
       '''
       args = arg.split()
       global username
       if  len(args) < 3:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth'
       else:
            global inface
            devId = int(args[0])
            offset = int(args[1], 16)
            width = int(args[2])
            print('Input Arguments are devId=%d, regOffset=0x%x, width=%d' % (devId, offset, width))

            fpaths = '/tmp/npipes' + args[0] + str(username)
            wp = open(fpaths, 'w+')
            entry = 'R ' + inface + ' ' + args[0] + ' ' + args[1] + ' ' + args[2]
            wp.write(entry)
            wp.close()

            fpathv = '/tmp/npipev' + args[0] + str(username)
            rp = open(fpathv, 'r')
            response = rp.read()
            rp.close()
            return response

    #/*****************************************************************************************************/
    # command to write a string to the named pipe
    #/*****************************************************************************************************/
    def do_write_str_pipe(self, arg):
       '''
        Write a string from pipe driver: Enter [ inputString ]
       '''
       args = arg
       global username
       if  len(args) < 1:
            print 'Invalid input, Enter input string'
       else:
            global inface
            devId = 0
            strn = tuple(map(ord, map(str, args)))
            width = len(strn)
            #print('Input Arguments are devId=%d, str=%s, len=%d' % (devId, strn, width))

            for i in range(0, width):
                setPyInput(i, strn[i]);

            xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().getDalObj().dalPut(devId, cvar.pyInput)
            # print 'Write string on pipe is Done'

    #/*****************************************************************************************************/
    # command to read a string to the named pipe
    #/*****************************************************************************************************/
    def do_read_str_pipe(self, arg):
       '''
        Read a string from pipe driver: Enter [ devId ]
       '''
       args = arg.split()
       global username
       if  len(args) < 1:
            print 'Invalid input, Enter its Device-Id'
       else:
            global inface
            devId = int(args[0])
	    #print('Input Arguments are devId=%d' % (devId))
	    response = ''
            L = [0] * 512
            lent = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().getDalObj().dalGet(devId, cvar.pyInput)
            #print 'First character len = %d is %d' % (lent,getPyInput(0))
            for i in range(0, lent):
                L[i] = getPyInput(i);
            L[lent] = 0
            response = ''.join(chr(i) for i in L)
            print 'Read string on pipe is : %s' % response
            return response


#/*********************************************************************************************************/
# The class object for driver wrapper operations
#/*********************************************************************************************************/
class runDrvWrapper(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to init the driver wrapper
    #/*****************************************************************************************************/
    def do_init_drv(self, arg):
       '''
        Initialize the driver wrapper, Enter device-id and one of the following bus select: Enter [ devId,busType ]
        PCIE_BUS = 0
        I2C_BUS = 1
        MDIO_BUS = 2
        I2C_OVER_LPC = 3
        MDIO_OVER_LPC = 4
       '''
       args = arg.split()
       if  len(args) < 2:
            print 'Invalid input, Enter its Device-Id, BusType'
       else:
            devId = int(args[0])
            bus = int(args[1])
            print('Input Arguments are devId=%d, busSelect=%d' % (devId, bus))
            xpDrvWrapperInit(devId)
            xpDrvWrapperBusInitWrap(devId, bus);
            print 'Done'

    #/*****************************************************************************************************/
    # command to create fifo
    #/*****************************************************************************************************/
    def create_mdio_fifo(self, arg):
        os.mkfifo('/dev/mdio-9c-1', 0777)

    #/*****************************************************************************************************/
    # command to read mdio
    #/*****************************************************************************************************/
    def read_mdio_in(self, arg):
       '''
        Read a Register from MDIO, regOffset (hex): Enter [ regOffset ]
       '''
       args = arg.split()
       if  len(args) < 1:
            print 'Invalid input, Enter regOffset (hex)'
       else:
            fpaths = '/dev/mdio-9c-1'
            wp = open(fpaths, 'w+')
            entry = 'r 0 ' + args[0]
            wp.write(entry)
            #print('Entry %s written and waiting to read:' % entry)
            var = wp.read()
            #print('Read %s' % var)
            var = var.split()
            lent = len(var)
            ret = int(var[lent-1],16)
            wp.close()
            return ret


    #/*****************************************************************************************************/
    # command to write mdio
    #/*****************************************************************************************************/
    def write_mdio_in(self, arg):
       '''
        Write a Register from MDIO, regOffset (hex) and data: Enter [ regOffset,data ]
       '''
       args = arg.split()
       if  len(args) < 2:
            print 'Invalid input, Enter regOffset (hex) and data'
       else:
            fpaths = '/dev/mdio-9c-1'
            wp = open(fpaths, 'w+')
            entry = 'w 0 ' + args[0] + ' ' + args[1]
            wp.write(entry)
            #print('Entry %s written:' % entry)
            wp.close()

    #/*****************************************************************************************************/
    # command to check mdio busy
    #/*****************************************************************************************************/
    def check_mdio_busy(self, arg):
        var = 1
        while var == 1:
            var = runDrvWrapper().read_mdio_in('0')
            var = var & 0x1

    #/*****************************************************************************************************/
    # command to read mdio
    #/*****************************************************************************************************/
    def do_read_mdio(self, arg):
       '''
        Read a Register from MDIO, regOffset (hex): Enter [ regOffset ]
       '''
       args = arg.split()
       if  len(args) < 1:
            print 'Invalid input, Enter regOffset (hex)'
       else:
           runDrvWrapper().check_mdio_busy('0')
           runDrvWrapper().write_mdio_in('0 0')
           val = int(args[0],16)
           vall = format((val & 0xffff), 'x')
           valh = format(((val >> 16) & 0xffff), 'x')
           entry = '1 0x' + vall
           runDrvWrapper().write_mdio_in(entry)
           entry = '2 0x' + valh
           runDrvWrapper().write_mdio_in(entry)
           runDrvWrapper().check_mdio_busy('0')
           vall = runDrvWrapper().read_mdio_in('3')
           valh = runDrvWrapper().read_mdio_in('4')
           print("Value of register 0x%x = 0x%04x%04x" % (val, valh, vall))

    #/*****************************************************************************************************/
    # command to write mdio
    #/*****************************************************************************************************/
    def do_write_mdio(self, arg):
       '''
        Write a Register from MDIO, regOffset (hex) and data: Enter [ regOffset,data ]
       '''
       args = arg.split()
       if  len(args) < 2:
            print 'Invalid input, Enter regOffset (hex) and data'
       else:
           runDrvWrapper().check_mdio_busy('0')
           runDrvWrapper().write_mdio_in('0 1')
           val = int(args[0],16)
           vall = format((val & 0xffff), 'x')
           valh = format(((val >> 16) & 0xffff), 'x')
           entry = '1 0x' + vall
           runDrvWrapper().write_mdio_in(entry)
           entry = '2 0x' + valh
           runDrvWrapper().write_mdio_in(entry)
           val = int(args[1],16)
           vall = format((val & 0xffff), 'x')
           valh = format(((val >> 16) & 0xffff), 'x')
           entry = '3 ' + vall
           runDrvWrapper().write_mdio_in(entry)
           entry = '4 ' + valh
           runDrvWrapper().write_mdio_in(entry)
           runDrvWrapper().check_mdio_busy('0')

    #/*****************************************************************************************************/
    # command to select the driver bus
    #/*****************************************************************************************************/
    def do_init_bus(self, arg):
       '''
        Initialize the driver bus, Enter [ devId,BusType ]
        one of the following bus select
        PCIE_BUS = 0
        I2C_BUS = 1
        MDIO_BUS = 2
        I2C_OVER_LPC = 3
        MDIO_OVER_LPC = 4
        Enter [ devId,busType ]
       '''
       args = arg.split()
       if  len(args) < 2:
            print 'Invalid input, Enter its Device-Id, BusType'
       else:
            devId = int(args[0])
            bus = int(args[1])
            print('Input Arguments are devId=%d, busSelect=%d' % (devId, bus))
            #xpDrvWrapperInit(devId)
            xpDrvWrapperBusInitWrap(devId, bus);
            print 'Done'

    #/*****************************************************************************************************/
    # command to de-init the driver wrapper
    #/*****************************************************************************************************/
    def do_deinit_drv(self, arg):
       '''
        De-Initialize the driver wrapper: Enter [ devId ]
       '''
       args = arg.split()
       if  len(args) < 1:
            print 'Invalid input, Enter its Device-Id'
       else:
            devId = int(args[0])
            #bus = int(args[1])
            print('Input Arguments are devId=%d' % (devId))
            #xpDrvWrapperBusDeInit(devId, bus);
            xpDrvWrapperDeInit(devId);
            print 'Done'

    #/*****************************************************************************************************/
    # command to write directly from driver wrapper
    #/*****************************************************************************************************/
    def do_write_drv(self, arg):
       '''
        Write a Register from driver wrapper: Enter [ devId,regOffset,wordWidth ]
       '''
       args = arg.split()
       if  len(args) < 3:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth'
       else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])

            print('Input Arguments are devId=%d, regOffset=0x%x, width=%d' % (args[0], args[1], args[2]))
            width = args[2]
            valArgs = raw_input('Enter the Value in %d word(s) in Hex, [word1, word2, ..., wordn] : ' % width)

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                arrUint32 = new_arrUint32(listLen)
                for i in range(0, listLen):
                    arrUint32_setitem(arrUint32, i, int(postList[i], 16))

                ret = xpDrvWrapperDalWrite(args[0], args[1], (width * 4), arrUint32)
                if ret == 0:
                    print('Reg entry written')
                else:
                    print("Error %d in writing" % (ret))
                delete_arrUint32(arrUint32)
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))


    #/*****************************************************************************************************/
    # command to read the register from driver wrapper
    #/*****************************************************************************************************/
    def do_read_drv(self, arg):
       '''
        Read a Register from driver wrapper: Enter [ devId,regOffset,wordWidth ]
       '''
       args = arg.split()
       if  len(args) < 3:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth'
       else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])
            width = args[2]
            print('Input Arguments are devId=%d, regOffset=0x%x, width=%d' % (args[0], args[1], args[2]))
            ret = xpDrvWrapperDalPrint(args[0], args[1], (width * 4))

#/*********************************************************************************************************/
# The class object for Register / tables read, write and print functions
#/*********************************************************************************************************/
class regTableOperationCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to print the register
    #/*****************************************************************************************************/
    def do_print_reg(self, arg):
        '''
         Print a Register: Enter [ devId,registerId,instanceId,repeatId,offset,endOffset ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 5:
            print 'Invalid input, Enter Device-Id, registerId, instanceId, repeatId, offset, [endOffset]'
        else:
            print('Input Arguments are devId=%d, registerId=%d, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
            if len(args) > 5:
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], args[2], args[1], args[3], args[4], args[5])
            else:
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], args[2], args[1], args[3], args[4])

    #/********************************************************************************/
    # command for xpCompareHwSh to compare a register
    #/********************************************************************************/
    def do_reg_compare_hw_sh(self, arg):
        '''
         Compare register between hardware and shadow: Enter [ devId,registerId ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 2:
            print('Invalid input, Enter [ devId,registerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            print('Input Arguments are, devId=%d, registerId=%d' % (args[0],args[1]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpCompareHwSh(args[0],args[1],1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Command Success')

    #/********************************************************************************/
    # command for xpCompareHwSh
    #/********************************************************************************/
    def do_table_compare_hw_sh(self, arg):
        '''
         Compare table between hardware and shadow: Enter [ devId,tableId,type (SRAM=1, TCAM=2, TCAM-MASK=3) ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 3:
            print('Invalid input, Enter [ devId,tableId,type (SRAM=1, TCAM=2, TCAM-MASK=3) ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            print('Input Arguments are, devId=%d, tableId=%d, type=%d' % (args[0],args[1],args[2]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpCompareHwSh(args[0],args[1],args[2],1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Command Success')

    #/********************************************************************************/
    # command for complete comparision of hw and shadow
    #/********************************************************************************/
    def do_compare_hw_sh(self, arg):
        '''
         xpCompareHwSh: Enter [ devId ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpCompareHwSh(args[0],1)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Command Success')


    #/*****************************************************************************************************/
    # command to print the register shadow poinrter
    #/*****************************************************************************************************/
    def do_print_reg_ptr(self, arg):
        '''
         Print a Register shadow pointer: Enter [ devId,registerId,instanceId,repeatId,offset ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            print 'Invalid input, Enter Device-Id, registerId, instanceId, repeatId, offset'
        else:
            print('Input Arguments are devId=%d, registerId=%d, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintRegPtr(args[0], args[2], args[1], args[3], args[4])


    #/*****************************************************************************************************/
    # command to print the file for given memory tile
    #/*****************************************************************************************************/
    def do_print_mem_to_file(self, arg):
        '''
         Print a memory file for given table/tile: Enter [ devId,regId,instance,repeat,beginOff,endOff ]
        '''
        try:
            args = tuple(map(int, arg.split()))
            if  len(args) < 6:
                print 'Invalid input, Enter devId, regId, instance, repeat, beginOff, endOff (if offsets unknown then enter -1)'
            else:
                print('Input Arguments are devId=%d, registerId=%d, intanceId=%d, repeatId=%d, off1=%d, off2=%d' %
                      (args[0], args[1], args[2], args[3], args[4], args[5]))
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintCmdFile(args[0], args[2], args[1], args[3], args[4], args[5])
        except (ValueError, IndexError), e:
            print 'Invalid input format, Enter devId, regId, instance, repeat, beginOff, endOff (if offsets unknown then enter -1)'


    #/*****************************************************************************************************/
    # command to read the command file and write it on to the registers/memory
    # nop XP_TXQ_QMAP_CONFIG_SRC_PORT_QMAP_TBL_CFG
    # // XP_TXQ_QMAP_CONFIG_SRC_PORT_QMAP_TBL_CFG
    # writeReg 0 952 0 0 8
    #[0x0,0x0,0xffffffff,0xffffffff,0x0,0x0,0xffffffff,0xffffffff]
    #/*****************************************************************************************************/
    def do_write_from_file(self, arg):
        'Perform all the write operations from the file'
        args = raw_input('Enter the fileName : ')
        print('Input FileName = %s' % (args))

        lineNum = -1
        regId = -1
        offset = 0
        with open(args) as f:
            for line in f:
                #print line,
                lineNum = lineNum + 1
                if line in ['\n', '\r\n']:
                    continue

                if '//' in line:
                    continue

                if '#' in line:
                    continue

                if 'nop' in line:
                        continue

                if 'write_reg' in line:
                    postList = line.split()
                    beginOff = 0
                    endOff = 0
                    depth = 0
                    if  len(postList) < 6:
                        print 'Invalid input, lineNum=%d, line=%s' % (lineNum, line)
                        regId = -1
                        continue
                    else:
                        devId = int(postList[1])
                        regId = int(postList[2])
                        instance = int(postList[3])
                        repeat = int(postList[4])

                        if  len(postList) == 6:
                            beginOff = int(postList[5])
                            endOff = beginOff

                        if  len(postList) > 6:
                            width = int(postList[5])
                            depth = int(postList[6])
                            endOff = beginOff

                    if  len(postList) == 8:
                        beginOff = int(postList[7])
                        endOff = beginOff

                    if  len(postList) > 8:
                        beginOff = int(postList[7])
                        endOff = int(postList[8])

                    offset = beginOff

                if '[' in line:
                    if regId == -1:
                        print 'Invalid input without regId, lineNum=%d, line=%s' % (lineNum, line)
                        continue
                    #print 'devId=%d, regId=%d, instance=%d, repeat=%d, beginOff=%d, endOff=%d' % (devId, regId, instance, repeat, beginOff, endOff)
                    postList = line.strip("'").strip("'").strip("]").strip("[").strip("]\n").split(",")
                    #print 'postList = %s' % postList

                    listLen = len(postList)

                    width = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpGetRegWordWidth(regId);

                    if listLen == width:
                        arrUint32 = new_arrUint32(listLen)
                        for i in range(0, listLen):
                            arrUint32_setitem(arrUint32, i, int(postList[i], 16))
                        if len(args) > 6:
                            ret = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpWriteReg(devId, instance, regId, repeat, beginOff, endOff, (width * 4), arrUint32)
                        else:
                            ret = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpWriteReg(devId, instance, regId, repeat, beginOff, (width * 4), arrUint32)
                        if ret != 0:
                            print("Error %d in writing at offset=%d" % (ret, offset))
                        delete_arrUint32(arrUint32)
                    else:
                        print('Invalid input, expected words %d, given %d' % (width, listLen))

                    regId = -1
                    width = -1
                    offset = offset + 1

                #if 'str' in line:
                #    break

        print('Write from file is done')

    #/*****************************************************************************************************/
    # command to read the hex file and write it on to the tile
    #/*****************************************************************************************************/
    def do_load_scpu_firmware(self, arg):
        'Load SCPU firmware from the file, Enter fileName and numFiles'
        arg = raw_input('Enter the fileName and numFiles: ')
        args = arg.split()
        n = 1

        if  len(args) >= 2:
            n = int(args[1])

        print('Input FileName = %s, numFiles = %d' % (args[0], n))

        devId = 0
        lineNum = -1
        regId = 0
        instance = 0
        repeat = 0
        width = 1
        offset = 0
        print 'devId=%d, regId=%d, instance=%d, repeat=%d, width=%d' % (devId, regId, instance, repeat, width)
        for x in range(0, n):
            if n == 1:
                fname = args[0]
            else:
                fname = args[0] + str(x) + '.hex'
            print 'file=%s' % (fname)
            with open(fname) as f:
               for line in f:
                   #print line,
                   lineNum = lineNum + 1
                   if line in ['\n', '\r\n', '//']:
                       continue

                   #if 'str' in line:
                   #    break

                   if 'xxxxxxxx' in line:
                       kval1 = 0
                       kval2 = 0
                       offset = offset + 1
                       continue

                   if '@' in line:
                       if regId == -1:
                           print 'Invalid input without regId, lineNum=%d, line=%s' % (lineNum, line)
                           continue

                       argz = line.split()
                       nline = argz[1]
                       listLen = 2
                       arrUint32 = new_arrUint32(listLen)

                       line_offset = int(argz[0].split('@')[1],16)
                       sram_offset = x * 0x2000
                       offset = sram_offset + line_offset

                       n = 2
                       nval = [nline[i:i+n] for i in range(0, len(nline), n)]
                       #print ('nval=%2x%2x%2x%2x%2x' % (int(nval[0],16), int(nval[1],16), int(nval[2],16), int(nval[3],16), int(nval[4],16)))

                       #kval = (int(nval[1],16) << 24) | (int(nval[2],16) << 16) | (int(nval[3],16) << 8) | (int(nval[4],16))
                       #csum = int(nval[0],16) & 0xff

                       #kval1 = (csum << 24) | (kval >> 8)
                       #kval2 = ((kval & 0xff) << 24)
                       kval1 = (int(nval[1],16) << 24) | (int(nval[2],16) << 16) | (int(nval[3],16) << 8) | (int(nval[4],16))
                       kval2 = 0

                       #print '@%x  %08x%08x' % (offset, kval1, kval2)

                       arrUint32_setitem(arrUint32, 0, kval1)
                       arrUint32_setitem(arrUint32, 1, kval2)

                       ret = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpWriteReg(devId, instance, regId, repeat, offset, (((listLen - 1) * 4)), arrUint32)
                       if ret != 0:
                           print("Error %d in writing at offset=%d" % (ret, offset))
                       else:
                         if kval1 != 0:
                           print("Done writing %d  at offset=%d sram_offset=%d line_offset= %d kval=%x" % (ret, offset,sram_offset,line_offset,kval1))
                       #offset = offset + 1
                       delete_arrUint32(arrUint32)

        print('Firmware is loaded successfully')


    #/*****************************************************************************************************/
    # command to read the hex file and write it on to the tile
    #/*****************************************************************************************************/
    def do_write_from_hex_file(self, arg):
        'Write Memory tile, Enter fileName'
        args = raw_input('Enter the fileName : ')
        print('Input FileName = %s' % (args))
        print('Writing from the file...')

        lineNum = -1
        regId = -1
        offset = 0
        with open(args) as f:
           for line in f:
               #print line,
               lineNum = lineNum + 1
               if line in ['\n', '\r\n']:
                   continue

               if '//' in line:
                   if 'Args' in line:
                       argz = line.split()
                       postList = argz[2].strip("'").strip("'").strip("]").strip("[").split(",")
                       beginOff = 0
                       if  len(postList) < 6:
                           print 'Invalid input, lineNum=%d, line=%s' % (lineNum, line)
                           regId = -1
                           continue
                       else:
                           devId = int(postList[0])
                           regId = int(postList[1])
                           instance = int(postList[2])
                           repeat = int(postList[3])
                           width = int(postList[4])
                           depth = int(postList[5])
                           endOff = depth
                       if  len(postList) == 6:
                           beginOff = int(postList[6])
                           endOff = depth

                       if  len(postList) > 6:
                               beginOff = int(postList[6])
                               endOff = int(postList[7])

                       offset = beginOff
                       print 'devId=%d, regId=%d, instance=%d, repeat=%d, width=%d, depth=%d, beginOff=%d, endOff=%d' % (devId, regId, instance, repeat, width, depth, beginOff, endOff)
                   else:
                       continue

               if '@' in line:
                   if regId == -1:
                       print 'Invalid input without regId, lineNum=%d, line=%s' % (lineNum, line)
                       continue

                   argz = line.split()
                   line_offset = int(argz[0].split('@')[1],16)
                   nline = argz[1]
                   nbytes = ((width * 8) - 1)
                   n = 8
                   linez = [nline[i:i+n] for i in range(0, len(nline), n)]
                   listLen = len(linez)
                   arrUint32 = new_arrUint32(listLen)
                   for i in range(0, listLen):
                       j = ((listLen-1) - i)
                       arrUint32_setitem(arrUint32, j, int(linez[i], 16))
                   ret = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpRegUnAccessible(regId)
                   if ret == 0:
                       ret = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpWriteReg(devId, instance, regId, repeat, line_offset, (listLen * 4), arrUint32)
                   else:
                       print('Skip UnAccessible')
                       ret = 0
                   if ret != 0:
                       print("Error %d in writing at offset=%d" % (ret, line_offset))
                   offset = offset + 1
                   delete_arrUint32(arrUint32)

               if offset > endOff:
                   break

               #if 'str' in line:
               #   break
        print('Write from file is done')

    #/*****************************************************************************************************/
    # command to print the hex file for given memory tile
    #/*****************************************************************************************************/
    def do_print_mem_to_hex_file(self, arg):
        '''
         Print a memory file for given table/tile: Enter [ devId,regId,instance,repeat,beginOff,endOff,printZeros ]
        '''
        try:
            args = tuple(map(int, arg.split()))
            if  len(args) < 7:
                print 'Invalid input, Enter devId, regId, instance, repeat, beginOff, endOff (if offsets unknown then enter -1), printZeros'
            else:
                print('Input Arguments are devId=%d, registerId=%d, intanceId=%d, repeatId=%d, off1=%d, off2=%d, printZeros=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintMemFile(args[0], args[2], args[1], args[3], args[4], args[5], args[6])
        except (ValueError, IndexError), e:
            print 'Invalid input format, Enter devId, regId, instance, repeat, beginOff, endOff (if offsets unknown then enter -1)'

    #/*****************************************************************************************************/
    # command to print the shadow register pointer
    #/*****************************************************************************************************/
    def do_print_shadow_reg_ptr(self, arg):
        '''
         Print shadow register pointer: Enter [ devId,regId,instance,repeat,offset ]
        '''
        try:
            args = tuple(map(int, arg.split()))
            if  len(args) < 5:
                print 'Invalid input, Enter devId, regId, instance, repeat, offset (zero for non-table)'
            else:
                print('Input Arguments are devId=%d, registerId=%d, intanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintShadowRegPointer(args[0], args[2], args[1], args[3], args[4])
        except (ValueError, IndexError), e:
            print 'Invalid input format, Enter devId, regId, instance, repeat, offset (zero for non-table)'



    #/*****************************************************************************************************/
    # command to print the shadow memory pointer
    #/*****************************************************************************************************/
    def do_print_shadow_mem_ptr(self, arg):
        '''
         Print shadow memory pointer: Enter [ devId,regId,tableId,poolId,tileId,offset,memType ]
        '''
        try:
            args = tuple(map(int, arg.split()))
            if  len(args) < 6:
                print 'Invalid input, Enter devId, regId, tableId, poolId, tileId, offset, memType (SRAM=1, TCAM=2, TCAM-MASK=3)'
            else:
                print('Input Arguments are devId=%d, registerId=%d, intanceId=%d, repeatId=%d, offset=%d, type=%d' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintShadowMemPointer(args[0], args[2], args[1], args[3], args[4])
        except (ValueError, IndexError), e:
            print 'Invalid input format, Enter devId, regId, tableId, poolId, tileId, offset, memType (SRAM=1, TCAM=2, TCAM-MASK=3)'



    #/*****************************************************************************************************/
    # command to print the file for complete device
    #/*****************************************************************************************************/
    def do_print_complete_mem_to_file(self, arg):
        '''
         Print a memory file for a complete device: Enter [ devId,type,printZeroes ]
        '''
        try:
            args = tuple(map(int, arg.split()))
            if  len(args) < 3:
                print 'Invalid input, Enter devId, type (XP_REG_ONLY,XP_TABLE_ONLY,XP_REG_AND_TABLE), printZeroes(1=print, 0 = dont print'
            else:
                print('Input Arguments are devId=%d, type=%d, zeros=%d' % (args[0], args[1], args[2]))
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintCompleteMemToFile(args[0], args[1], args[2])
        except (ValueError, IndexError), e:
            print 'Invalid input format, Enter devId and type (XP_REG_ONLY,XP_TABLE_ONLY,XP_REG_AND_TABLE)'

    #/*****************************************************************************************************/
    # command to print the register attributes at given offset
    #/*****************************************************************************************************/
    def do_print_reg_attr_at_offset(self, arg):
        '''
         Print a Register Attributes at offset: Enter [ devId,registerOffset ]
        '''
        args = arg.split()
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, registerOffset'
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)

            print('Input Arguments are devId=%d, registerOffset=0x%x' % (args[0], args[1]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintRegAttrAtOffset(args[0], args[1])

    #/*****************************************************************************************************/
    # command to print the table entry
    #/*****************************************************************************************************/
    def do_print_table_entry(self, arg):
        '''
         Print a Static Table Entry: Enter [ devId,tableId,poolId,tileId,offset,endOffset,memType ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 7:
            print 'Invalid input, Enter Device-Id, tableId, poolId, tileId, offset, endOffset and memType (SRAM=1, TCAM=2, TCAM-MASK=3)'
        else:
           print('Input Arguments are devId=%d, tableId=%d, poolId=%d, tileId=%d, offset=%d, endOffset=%d, memType=%d' %
                                                                                                (args[0], args[1], args[2],
                                                                                                args[3], args[4], args[5], arg[6]))
           xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().dalMemTablePrintEntry(args[0], args[1], args[2],
                                                                                               args[3], args[4], args[5], arg[6])

    #/*****************************************************************************************************/
    # command to print the table entry
    #/*****************************************************************************************************/
    def do_write_table_entry(self, arg):
        '''
         Print a Static Table Entry: Enter [ devId,tableId,poolId,tileId,offset,endOffset,memType,value ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 6:
            print 'Invalid input, Enter Device-Id, tableId, poolId, tileId, offset, endOffset and memType (SRAM=1, TCAM=2, TCAM-MASK=3), value in bytes'
        else:
            print('Input Arguments are devId=%d, tableId=%d, poolId=%d, tileId=%d, offset=%d, endOffset=%d, memType=%d' %
                                                                                                    (args[0], args[1], args[2],
                                                                                                     args[3], args[4], args[5], args[6]))
        width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetTableWordWidth(args[1], args[6]);
        valArgs = raw_input('Enter the Value in %d bytes : ' % width)
        postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
        listLen = len(postList)

        if listLen == width:
            arrUint32 = new_arrUint32(listLen)
            for i in range(0, listLen):
                arrUint32_setitem(arrUint32, i, int(postList[i], 16))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().dalMemTableWriteEntry(args[0], args[1], args[2],
                                                                                                args[3], args[4], args[5], args[6], arrUint32)
            print('Table entry written')
            delete_arrUint32(arrUint32)
        else:
            print('Invalid input, expected words %d, given %d' % (width, listLen))

    #/********************************************************************************/
    # command for setForceHwRead
    #/********************************************************************************/
    def do_set_force_hw_read(self, arg):
        '''
         setForceHwRead: Enter [ devId, value ]
        '''
        args = re.split(';| |,',arg)
        if  len(args) < 2:
            print('Invalid input, Enter [ devId, value ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            print('Input Arguments are, devId=%d, value=%d' % (args[0], args[1]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().setForceHwRead(args[1])
            print('Done')

    #/*****************************************************************************************************/
    # command to print attributes of the register
    #/*****************************************************************************************************/
    def do_print_attr_of_reg(self, arg):
        '''
         Print Attributes and Value of a Register: Enter [ devId,registerId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, registerId, [brief(0/1)]'
        else:
            brief = 1
            if len(args) > 2:
                brief = args[2]
                print('Input Arguments are devId=%d, registerId=%d, brief=%d' % (args[0], args[1], brief))
            else:
                print('Input Arguments are devId=%d, registerId=%d' % (args[0], args[1]))
            orgVal = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().setBriefOutput(brief)
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpListRegAttributes(args[0], args[1])
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().setBriefOutput(orgVal)

    #/*****************************************************************************************************/
    # command to print register field informations
    #/*****************************************************************************************************/
    def do_print_reg_field_info(self, arg):
        '''
         Print register field information: Enter [ devId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id'
        else:
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintRegFieldInfo(args[0])

    #/*****************************************************************************************************/
    # command to get last two digits of a string
    #/*****************************************************************************************************/
    def reg_enum_value(self, devId, arg):
        regname = arg
        lasttwo = arg[-2:]
        lastnine = arg[-9:]
        if '_E' not in lasttwo:
            regname = arg + '_E'

        #print('regname = %s' % regname)
        if (arg.isdigit()):
            return (int(arg))
        regId = -1
        try:
            regId = eval(regname)
            #print('regId is GOT it from here %d' % regId)
            #get device specific regId for COMMON registers
            if '_COMMON_E' in lastnine:
                regId = xpDeviceMgr.instance().getDeviceObj(devId).getRegObj().getDevRegId(regId)
                #print('regId updated here %d' % regId)
        except (NameError, ValueError):
            if '_COMMON_E' in lastnine:
                regName = tuple(map(ord, map(str, regname[:-9])))
            else:
                regName = tuple(map(ord, map(str, regname[:-2])))
            for i in range(0, len(regName)):
                setPyInput(i, regName[i]);
            regId = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpGetRegIdWithName(devId, cvar.pyInput)
            #print('regId is from here %d' % regId)

        return regId

    #/*****************************************************************************************************/
    # command to print regId by name
    #/*****************************************************************************************************/
    def do_print_reg_id_by_name(self, arg):
        '''
         Print regId of given register name: Enter [ devId,registerName ]
        '''
        args = arg.split()
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, register Name'
        else:
            regId = self.reg_enum_value(int(args[0]), args[1])
            print('RegId for reg %s is : %d' % (args[1], regId))


    #/*****************************************************************************************************/
    # command to print attributes of the register name
    #/*****************************************************************************************************/
    def do_print_attr_of_reg_name(self, arg):
        '''
         Print Attributes and Value of a Register: Enter [ devId,registerName ]
        '''
        args = arg.split()
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, register Name, [brief(0/1)]'
        else:
            devId = int(args[0])
            regId = self.reg_enum_value(devId, args[1])
            #print('Input Arguments are devId=%d, registerId=%d' % (devId, regId))
            if (regId >= 0):
                brief = 1
                if len(args) > 2:
                    brief = int(args[2])
                orgVal = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().setBriefOutput(brief)
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpListRegAttributes(devId, regId)
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().setBriefOutput(orgVal)
            else:
                print 'Invalid register name %s' % args[1]

    #/*****************************************************************************************************/
    # command to check if register is volatile
    #/*****************************************************************************************************/
    def do_check_reg_volatile(self, arg):
        '''
         Check if register is volatile: Enter [ devId,registerId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, registerId'
        else:
            print('Input Arguments are devId=%d, registerId=%d' % (args[0], args[1]))
            vol = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().isRegVolatile(args[1])
            print('Volatile bit of given register is : %d' % vol)


    #/*****************************************************************************************************/
    # command to print attributes of all the registers
    #/*****************************************************************************************************/
    def do_print_attr_of_all_regs(self, arg):
        '''
         Print Attributes and Values of all Registers: Enter [ devId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [brief(0/1)]'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            brief = 1
            if len(args) > 1:
                brief = args[1]
            orgVal = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().setBriefOutput(brief)
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpListRegAttributes(args[0])
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().setBriefOutput(orgVal)

    #/*****************************************************************************************************/
    # command to print attributes table of all the registers
    #/*****************************************************************************************************/
    def do_print_attr_table_of_all_regs(self, arg):
        '''
         Print Attributes Table of all registers: Enter [ devId ]
        '''
        input = arg.split()
        if  len(input) < 1:
            print 'Invalid input, Enter Device-Id, [grep string]'
        else:
            devId = int(input[0])
            print('Input Arguments are devId=%d' % (devId))
            if len(input) > 1:
                print 'grepStr = %s' % input[1]
                grepStr = tuple(map(ord, map(str, input[1])))
                lenx = len(grepStr)
                for i in range(0, lenx):
                    setPyInput(i, grepStr[i]);
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpListRegAttrTable(devId, cvar.pyInput)
            else:
                xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpListRegAttributeTable(devId)

    #/*****************************************************************************************************/
    # command to print volatile registers
    #/*****************************************************************************************************/
    def do_print_volatile_regs(self, arg):
        '''
         Print all volatile registers and tables: Enter [ devId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id'
        else:
            print('Input Arguments are devId=%d' % (args[0]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpListVolatileRegs(args[0])


    #/*****************************************************************************************************/
    # command to print attributes of the table
    #/*****************************************************************************************************/
    def do_print_attr_of_static_table(self, arg):
        '''
         Print Attributes of a static table: Enter [ devId,tableId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, tableId'
        else:
            print('Input Arguments are devId=%d, tableId=%d' % (args[0], args[1]))
            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintStaticTableInfo(args[0], args[1])


    #/*****************************************************************************************************/
    # command to read the register
    #/*****************************************************************************************************/
    def do_read_reg(self, arg):
       '''
        To Read a Register: Enter [ devId,registerId,instanceId,repeatId,offset ]
       '''
       args = tuple(map(int, arg.split()))
       if  len(args) < 5:
           print 'Invalid input, Enter its Device-Id, registerId, instanceId, repeatId, offset [endOffset]'
       else:
           print('Input Arguments are devId=%d, registerId=%d, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))

           if pipeAccess == 1:
               reg=args[1]
               width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(args[1]);
               regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])
               finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width)
               response = runDrvPipe().do_read_pipe(finalInput)
               print "For Dev=%d, offset=0x%x, width=%d, Value is :" % (args[0], regOff, width)
               print "[%s]" % (response)
           else:
               if len(args) > 5:
                   xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], args[2], args[1], args[3], args[4], args[5])
               else:
                   xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], args[2], args[1], args[3], args[4])

    #/*****************************************************************************************************/
    # command to read the register fields
    #/*****************************************************************************************************/
    def do_read_reg_field(self, arg):
       '''
        To Read a Register: Enter [ devId,registerId,instanceId,repeatId,offset,fieldPos,fieldLen ]
       '''
       args = arg.split()
       if  len(args) < 7:
            print 'Invalid input, Enter its Device-Id, registerId, instanceId, repeatId, offset, fieldPos, fieldLen'
       else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])

            print('Input Arguments are devId=%d, registerId=%d, instanceId=%d, repeatId=%d' % (args[0], args[1], args[2], args[3]))
            print('offset=%d, fieldOff=%d, fieldLen=%d' % (args[4], args[5], args[6]))

            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintRegField(args[0], args[2], args[1], args[3], args[4], args[5], args[6])
            print('Done')


    #/*****************************************************************************************************/
    # command to write the register fields
    #/*****************************************************************************************************/
    def do_write_reg_field(self, arg):
       '''
        To Write a Register: Enter [ devId,registerId,instanceId,repeatId,offset,fieldPos,fieldLen,fieldValue,endOffset ]
       '''
       args = arg.split()
       if  len(args) < 8:
            print 'Invalid input, Enter its Device-Id, registerId, instanceId, repeatId, offset, fieldPos, fieldLen, fieldValue(hex), endOffset'
       else:
            args[0] = int(args[0])
            regId = self.reg_enum_value(args[0], args[1])
            if (int(regId) == -1):
                print 'regId = %d not supported' % regId
                return
            args[1] = int(regId)
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            args[7] = int(args[7], 16)
            endOffset = args[4]
            if len(args) > 8:
                args[8] = int(args[8])
                endOffset = args[8]

            print('Input Arguments are devId=%d, registerId=%d, instanceId=%d, repeatId=%d, offset=%d, endOffset = %d' % (args[0], args[1], args[2], args[3], args[4],endOffset))
            print('fieldOff=%d, fieldLen=%d, fieldVal=0x%x' % (args[5], args[6], args[7]))

            xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteRegField(args[0], args[2], args[1], args[3], args[4], args[5], args[6], args[7], endOffset)
            print('Reg entry field written')

    #/*****************************************************************************************************/
    # command to write the register
    #/*****************************************************************************************************/
    def do_write_reg(self, arg):
       '''
        To Write a Register: Enter [ devId,registerId,instanceId,repeatId, offset ]
       '''
       args = tuple(map(int, arg.split()))
       if  len(args) < 5:
            print 'Invalid input, Enter its Device-Id, registerId, instanceId, repeatId, offset, [endOffset]'
       else:
            print('Input Arguments are devId=%d, registerId=%d, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
            width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(args[1]);
            valArgs = raw_input('Enter the Value in %d word(s) in Hex, [word1, word2, ..., wordn] : ' % width)

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                arrUint32 = new_arrUint32(listLen)
                for i in range(0, listLen):
                    arrUint32_setitem(arrUint32, i, int(postList[i], 16))

                if pipeAccess == 1:
                    reg=args[1]
                    regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])
                    finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                    runDrvPipe().do_write_pipe_in(finalInput)
                else:
                    if len(args) > 5:
                        ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteReg(args[0], args[2], args[1], args[3], args[4], args[5], (width * 4), arrUint32)
                    else:
                        ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteReg(args[0], args[2], args[1], args[3], args[4], (width * 4), arrUint32)
                    if ret == 0:
                        print('Reg entry written')
                    else:
                        print("Error %d in writing" % (ret))
                delete_arrUint32(arrUint32)
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))

    #/*****************************************************************************************************/
    # command to write the MAC register
    #/*****************************************************************************************************/
    def do_write_mac_reg(self, arg):
        '''
         To Write a MAC Register: Enter [ devId,macNum,regAddr,value ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 4:
            print('Invalid input, Enter its Device-Id, macNum, regAddr(in hex), value(in hex)')
        else:
            regAddr = int(args[2], 16)
            value = int(args[3], 16)
            devId = int(args[0])
            print('Input Arguments are Device-Id : %d macNum : %d regAddr : %s value : %s' % (int(args[0]), int(args[1]), args[2], args[3]))
            #if int(args[1]) >= 16:
            #    args[1] = 47 - int(args[1])

            regname = 'XP_PTG_SLAVE_APB_ADDR_E'
            regName = tuple(map(ord, map(str, regname[:-2])))
            for i in range(0, len(regName)):
                setPyInput(i, regName[i]);
            ptgAddr = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpGetRegIdWithName(devId, cvar.pyInput)

            regname = 'XP_PTG_SLAVE_APB_DATA_E'
            regName = tuple(map(ord, map(str, regname[:-2])))
            for i in range(0, len(regName)):
                setPyInput(i, regName[i]);
            ptgData = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpGetRegIdWithName(devId, cvar.pyInput)

            width = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegWordWidth(ptgAddr);

            arrUint32 = new_arrUint32(1)
            arrUint32_setitem(arrUint32, 0, regAddr)

            reg = ptgAddr
            if pipeAccess == 1:
                regOff = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegOffset(int(args[0]), reg, int(args[1]), 0, 0)
                finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                runDrvPipe().do_write_pipe_in(finalInput)
            else:
                ret = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpWriteReg(int(args[0]), int(args[1]), reg, 0, 0, (width * 4), arrUint32)
                if ret != 0:
                    print("Error %d in writing" % (ret))
                else:
                    print('Reg entry written')

                    width = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegWordWidth(ptgData);

                    dataPtr = new_arrUint32(1)
                    arrUint32_setitem(dataPtr, 0, value)
                    reg = ptgData
                    if pipeAccess == 1:
                        regOff = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegOffset(int(args[0]), reg, int(args[1]), 0, 0)
                        finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                        runDrvPipe().do_write_pipe_in(finalInput)
                    else:
                        ret = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpWriteReg(int(args[0]), int(args[1]), reg, 0, 0, (width * 4), dataPtr)
                    if ret == 0:
                        print('Reg entry written')
                    else:
                        print("Error %d in writing" % (ret))
                    delete_arrUint32(dataPtr)
            delete_arrUint32(arrUint32)

    #/*****************************************************************************************************/
    # command to read the MAC register
    #/*****************************************************************************************************/
    def do_read_mac_reg(self, arg):
        '''
         To read a MAC Register: Enter [ devId,macNum,regAddr ]
        '''
        args = re.split(';| |,',arg)
        if '' in args:
            args.remove('')
        if  len(args) < 3:
            print('Invalid input, Enter its Device-Id, macNum, regAddr(in hex)')
        else:
            regAddr = int(args[2], 16)
            devId = int(args[0])
            print('Input Arguments are Device-Id : %d macNum : %d regAddr : %s' % (int(args[0]), int(args[1]), args[2]))
            #if int(args[1]) >= 16:
            #    args[1] = 47 - int(args[1])
            regname = 'XP_PTG_SLAVE_APB_ADDR_E'
            regName = tuple(map(ord, map(str, regname[:-2])))
            for i in range(0, len(regName)):
                setPyInput(i, regName[i]);
            ptgAddr = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpGetRegIdWithName(devId, cvar.pyInput)

            regname = 'XP_PTG_SLAVE_APB_DATA_E'
            regName = tuple(map(ord, map(str, regname[:-2])))
            for i in range(0, len(regName)):
                setPyInput(i, regName[i]);
            ptgData = xpDeviceMgr.instance().getDeviceObj(devId).getHwAccessMgr().xpGetRegIdWithName(devId, cvar.pyInput)

            width = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegWordWidth(ptgAddr);

            arrUint32 = new_arrUint32(1)
            arrUint32_setitem(arrUint32, 0, regAddr)

            reg = ptgAddr
            if pipeAccess == 1:
                regOff = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegOffset(int(args[0]), reg, int(args[1]), 0, 0)
                finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                runDrvPipe().do_write_pipe_in(finalInput)
            else:
                ret = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpWriteReg(int(args[0]), int(args[1]), reg, 0, 0, (width * 4), arrUint32)
                if ret != 0:
                    print("Error %d in writing" % (ret))
                else:
                    print('Reg entry written')

                    reg = ptgData
                    if pipeAccess == 1:
                        width = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegWordWidth(reg);
                        regOff = xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpGetRegOffset(int(args[0]), reg, int(args[1]), 0, 0)
                        finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width)
                        response = runDrvPipe().do_read_pipe(finalInput)
                        print "For Dev=%d, offset=0x%x, width=%d, Value is :" % (args[0], regOff, width)
                        print "[%s]" % (response)
                    else:
                        xpDeviceMgr.instance().getDeviceObj(int(args[0])).getHwAccessMgr().xpPrintReg(int(args[0]), int(args[1]), reg, 0, 0)
            delete_arrUint32(arrUint32)

    #/*****************************************************************************************************/
    # sub-commands for driver wrapper operations
    #/*****************************************************************************************************/
    def do_drv_wrapper(self, s):
        'driver wrapper command operations'
        i = runDrvWrapper()
        i.prompt = self.prompt[:-1]+':driverWrapper)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for driver pipe operations
    #/*****************************************************************************************************/
    def do_drv_pipe(self, s):
        'Read write over named pipe command operations'
        i = runDrvPipe()
        i.prompt = self.prompt[:-1]+':driverPipe)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for driver usb dongle operations
    #/*****************************************************************************************************/
    def do_drv_usb(self, s):
        'Read write over USB dongle  command operations'
        i = runBoardDiags()
        i.prompt = self.prompt[:-1]+':usb)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # command to write the offset with input
    #/*****************************************************************************************************/
    def do_write_reg_off_in(self, arg):
       '''
        Write a Register: Enter [ devId,regOffset,wordWidth,wordNum ]
       '''
       global pipeAccess
       args = arg.split()
       if  len(args) < 4:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth, [word1,word2,...wordn]'
       else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])

            #print('Input Arguments are devId=%d, regOffset=0x%x, width=%d' % (args[0], args[1], args[2]))
            width = args[2]
            valArgs = args[3]

            if pipeAccess == 1:
                argsval = arg
                #print ('argsval = %s' % argsval)
                runDrvPipe().do_write_pipe_in(argsval)
            else:
                postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
                listLen = len(postList)

                if listLen == width:
                    arrUint32 = new_arrUint32(listLen)
                    for i in range(0, listLen):
                        arrUint32_setitem(arrUint32, i, int(postList[i], 16))
                    ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().dalInWrite(args[0], args[1], (width * 4), arrUint32)
                    if ret == 0:
                        print('Reg entry written')
                    else:
                        print("Error %d in writing" % (ret))
                    delete_arrUint32(arrUint32)
                else:
                    print('Invalid input, expected words %d, given %d' % (width, listLen))


    #/*****************************************************************************************************/
    # command to write the offset
    #/*****************************************************************************************************/
    def do_write_reg_off(self, arg):
       '''
        Write a Register, Enter [ devId,regOffset,wordWidth ]
       '''
       global pipeAccess
       args = arg.split()
       if  len(args) < 3:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth'
       else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])

            print('Input Arguments are devId=%d, regOffset=0x%x, width=%d' % (args[0], args[1], args[2]))
            width = args[2]
            valArgs = raw_input('Enter the Value in %d word(s) in Hex, [word1, word2, ..., wordn] : ' % width)

            argsval = arg + ' ' + valArgs
            self.do_write_reg_off_in(argsval)

    # command to enable/disable pipe access
    #/*****************************************************************************************************/
    def do_enable_pipe_access(self, arg):
        '''
         Set or reset pipeAccess: Enter [ pipeAccess,flag ]
        '''
        runDrvPipe().do_enable_pipe_access(arg)

    #/*****************************************************************************************************/
    # command to read the register offset
    #/*****************************************************************************************************/
    def do_read_reg_off(self, arg):
       '''
        Read a Register: Enter [ devId,regOffset,wordWidth ]
       '''
       global pipeAccess
       args = arg.split()
       if  len(args) < 3:
            print 'Invalid input, Enter its Device-Id, regOffset (hex), wordWidth'
       else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = int(args[2])
            width = args[2]
            if pipeAccess == 1:
                response = runDrvPipe().do_read_pipe(arg)
                print "For Dev=%d, offset=0x%x, width=%d, Value is :" % (args[0], args[1], width)
                print "[%s]" % (response)
            else:
                print('Input Arguments are devId=%d, regOffset=0x%x, width=%d' % (args[0], args[1], args[2]))
                xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().dalInPrint(args[0], args[1], (width * 4))

    #/*****************************************************************************************************/
    # command to read the register using its name
    #/*****************************************************************************************************/
    def do_read_reg_name(self, arg):
       '''
        To Read a Register: Enter [ devId,registerName,instanceId,repeatId,offset ]
       '''
       args = arg.split()
       if  len(args) < 5:
           print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset [endOffset]'
       else:
           args[0] = int(args[0])
           args[2] = int(args[2])
           args[3] = int(args[3])
           args[4] = int(args[4])
           print('Input Arguments are devId=%d, register=%s, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
           regId = self.reg_enum_value(args[0], args[1])
           print 'regId = %d' % regId

           if (regId >= 0):
               if pipeAccess == 1:
                   reg=regId
                   width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(reg);
                   regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])
                   finalInput = str(args[0]) + ' ' + str(format(regOff,'08x')).rstrip("L") + ' ' + str(width)
                   response = runDrvPipe().do_read_pipe(finalInput)
                   print "For Dev=%d, offset=0x%x, width=%d, Value is :" % (args[0], regOff, width)
                   print "[%s]" % (response)
               else:
                   if len(args) > 5:
                       args[5] = int(args[5])
                       xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], args[2], regId, args[3], args[4], args[5])
                   else:
                       xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], args[2], regId, args[3], args[4])
           else:
               print 'Invalid register name %s' % args[1]

    #/*****************************************************************************************************/
    # command to print PCIe offset of the register using its name
    #/*****************************************************************************************************/
    def do_print_pcie_offset_reg_name(self, arg):
       '''
        To Read a Register: Enter [ devId,registerName,instanceId,repeatId,offset ]
       '''
       args = arg.split()
       if  len(args) < 5:
           print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset [endOffset]'
       else:
           args[0] = int(args[0])
           args[2] = int(args[2])
           args[3] = int(args[3])
           args[4] = int(args[4])
           print('Input Arguments are devId=%d, register=%s, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
           regId = self.reg_enum_value(args[0], args[1])
           print 'regId = %d' % regId

           if (regId >= 0):
                   reg=regId
                   width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(reg);
                   regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])
                   print(" ***********      Register: %s        PCIe_offset: 0x%x      ********** " % (args[1], regOff))
           else:
               print 'Invalid register name %s' % args[1]

    #/*****************************************************************************************************/
    # command to read the all tx-dma0 reg using its name
    #/*****************************************************************************************************/
    def do_print_all_txdma0_regs(self, arg):
       '''
        To Read a Register: Enter [ devId,queue ]
       '''
       args = arg.split()
       if  len(args) < 2:
           print 'Invalid input, Enter its Device-Id, queue'
       else:
           args[0] = int(args[0])
           args[1] = int(args[1])
           regIdList = [
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_CDP_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_CMD_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_CLR_ERR_CNT_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_CHAIN_LEN_ERR_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_CPU_OWN_DESC_ERR_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_ZERO_BUF_LEN_ERR_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_PCIE_ERR_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_DMA_INTF_ERR_REG",
                   "XP_MGMT_TX_DMA0_CFG_REG_DMA0_TX_PKT_DROP",
                   "XP_MGMT_TX_DMA0_CFG_REG_XP_MGMT_TX_DMA0_CFG_REGLOCKREG",
                   "XP_MGMT_TX_DMA0_CFG_REG_SCRATCHPAD",
                   ]
           print('Input Arguments are devId=%d, queue=%d' % (args[0], args[1]))
           for regName in regIdList:
               regId = self.reg_enum_value(args[0], regName)
               print 'regId = %d' % regId
               xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], 0, regId, args[1], 0, 0)

    #/*****************************************************************************************************/
    # command to read the all rx-dma0 reg using its name
    #/*****************************************************************************************************/
    def do_print_all_rxdma0_regs(self, arg):
       '''
        To Read a Register: Enter [ devId,queue ]
       '''
       args = arg.split()
       if  len(args) < 2:
           print 'Invalid input, Enter its Device-Id, queue'
       else:
           args[0] = int(args[0])
           args[1] = int(args[1])
           regIdList = [
                   "XP_MGMT_RX_DMA0_CFG_REG_DMA0_RX_CDP_REG",
                   "XP_MGMT_RX_DMA0_CFG_REG_DMA0_RX_CMD_REG",
                   "XP_MGMT_RX_DMA0_CFG_REG_DMA0_RX_CHAIN_LEN_ERR_REG",
                   "XP_MGMT_RX_DMA0_CFG_REG_DMA0_RX_CPU_OWN_DESC_ERR_REG",
                   "XP_MGMT_RX_DMA0_CFG_REG_DMA0_RX_ZERO_BUF_LEN_ERR_REG",
                   "XP_MGMT_RX_DMA0_CFG_REG_DMA0_RX_PCIE_ERR_REG",
                   "XP_MGMT_RX_DMA0_CFG_REG_XP_MGMT_RX_DMA0_CFG_REGLOCKREG",
                   "XP_MGMT_RX_DMA0_CFG_REG_SCRATCHPAD",
                   ]
           print('Input Arguments are devId=%d, queue=%d' % (args[0], args[1]))
           for regName in regIdList:
               regId = self.reg_enum_value(args[0], regName)
               print 'regId = %d' % regId
               xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintReg(args[0], 0, regId, args[1], 0, 0)

    #/*****************************************************************************************************/
    # command to write the register with name and input in same line
    #/*****************************************************************************************************/
    def do_write_reg_name_in(self, arg):
       '''
        To Write a Register: Enter [ devId,registerName,instanceId,repeatId,offset,endOffset,wordNum ]
       '''
       args = arg.split()
       if  len(args) < 6:
            print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset, [endOffset], [word1, word2... wordn]'
       else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            regId = self.reg_enum_value(args[0], args[1])
            #print 'regId = %d' % regId

            if (regId < 0):
                print 'Invalid register name %s, can crash' % args[1]

            width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(regId);
            valArgs = args[len(args) - 1]

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                arrUint32 = new_arrUint32(listLen)
                for i in range(0, listLen):
                    arrUint32_setitem(arrUint32, i, int(postList[i], 16))
                if pipeAccess == 1:
                    reg=regId
                    regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])
                    finalInput = str(args[0]) + ' ' + str(format(regOff,'08x')).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                    #finalInput = str(args[0]) + ' ' + str(regOff).rjust(12,'0') + ' ' + str(width) + ' ' + valArgs
                    runDrvPipe().do_write_pipe_in(finalInput)
                else:
                    if len(args) > 6:
                        ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteReg(args[0], args[2], regId, args[3], args[4], args[5], (width * 4), arrUint32)
                    else:
                        ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteReg(args[0], args[2], regId, args[3], args[4], (width * 4), arrUint32)
                    if ret == 0:
                        print('Reg entry written')
                    else:
                        print("Error %d in writing" % (ret))
                delete_arrUint32(arrUint32)
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))


    #/*****************************************************************************************************/
    # command to write the register with name
    #/*****************************************************************************************************/
    def do_write_reg_name(self, arg):
       '''
        To Write a Register: Enter [ devId,registerName,instanceId,repeatId,offset,endOffset ]
       '''
       args = arg.split()
       if  len(args) < 5:
            print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset, [endOffset]'
       else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            regId = self.reg_enum_value(args[0], args[1])
            #print 'regId = %d' % regId

            if (regId < 0):
                print 'Invalid register name %s, can crash' % args[1]

            width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(regId);
            valArgs = raw_input('Enter the Value in %d word(s) in Hex, [word1, word2, ..., wordn] : ' % width)

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                arrUint32 = new_arrUint32(listLen)
                for i in range(0, listLen):
                    arrUint32_setitem(arrUint32, i, int(postList[i], 16))
                if pipeAccess == 1:
                    reg=regId
                    regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])
                    finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                    runDrvPipe().do_write_pipe_in(finalInput)
                else:
                    if len(args) > 5:
                        ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteReg(args[0], args[2], regId, args[3], args[4], args[5], (width * 4), arrUint32)
                    else:
                        ret = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpWriteReg(args[0], args[2], regId, args[3], args[4], (width * 4), arrUint32)
                    if ret == 0:
                        print('Reg entry written')
                    else:
                        print("Error %d in writing" % (ret))
                delete_arrUint32(arrUint32)
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))

    #/*****************************************************************************************************/
    # command to read the register offset using its name
    #/*****************************************************************************************************/
    def do_read_reg_off_name(self, arg):
       '''
        To Read a Register: Enter [ devId,registerName,instanceId,repeatId,offset ]
       '''
       args = arg.split()
       if  len(args) < 5:
           print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset [endOffset]'
       else:
           args[0] = int(args[0])
           args[2] = int(args[2])
           args[3] = int(args[3])
           args[4] = int(args[4])
           #print('Input Arguments are devId=%d, register=%s, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
           regId = self.reg_enum_value(args[0], args[1])

           if (regId < 0):
               print 'Invalid register name %s, can crash' % args[1]
           reg = regId

           width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(regId);
           regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], reg, args[2], args[3], args[4])

           #print 'regId = %d, width=%d, regOff=0x%x' % (regId), (width), (regOff)
           finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width)
           self.do_read_reg_off(finalInput)

    #/*****************************************************************************************************/
    # command to write the register offset with name and input in same line
    #/*****************************************************************************************************/
    def do_write_reg_off_name_in(self, arg):
       '''
        To Write a Register: Enter [ devId,registerName,instanceId,repeatId,offset,wordNum ]
       '''
       args = arg.split()
       if  len(args) < 6:
            print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset, [word0, word1, ...wordn]'
       else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            print('Input Arguments are devId=%d, register=%s, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
            regId = self.reg_enum_value(args[0], args[1])

            if (regId < 0):
                print 'Invalid register name %s, can crash' % args[1]

            width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(regId);
            regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], regId, args[2], args[3], args[4])
            print 'regId = %d, width=%d, regOff=0x%x' % (regId, width, regOff)

            valArgs = args[5]

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                self.do_write_reg_off_in(finalInput)
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))


    #/*****************************************************************************************************/
    # command to write the register offset with name
    #/*****************************************************************************************************/
    def do_write_reg_off_name(self, arg):
       '''
        To Write a Register: Enter [ devId,registerName,instanceId,repeatId,offset ]
       '''
       args = arg.split()
       if  len(args) < 5:
            print 'Invalid input, Enter its Device-Id, register name, instanceId, repeatId, offset'
       else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            print('Input Arguments are devId=%d, register=%s, instanceId=%d, repeatId=%d, offset=%d' % (args[0], args[1], args[2], args[3], args[4]))
            regId = self.reg_enum_value(args[0], args[1])

            if (regId < 0):
                print 'Invalid register name %s, can crash' % args[1]

            width = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegWordWidth(regId);
            regOff = xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpGetRegOffset(args[0], regId, args[2], args[3], args[4])
            print 'regId = %d, width=%d, regOff=0x%x' % (regId, width, regOff)

            valArgs = raw_input('Enter the Value in %d word(s) in Hex, [word1, word2, ..., wordn] : ' % width)

            postList = valArgs.strip("'").strip("'").strip("]").strip("[").split(",")
            listLen = len(postList)

            if listLen == width:
                finalInput = str(args[0]) + ' ' + str(hex(regOff)).rstrip("L") + ' ' + str(width) + ' ' + valArgs
                self.do_write_reg_off_in(finalInput)
            else:
                print('Invalid input, expected words %d, given %d' % (width, listLen))

    #/*****************************************************************************************************/
    # command to print attributes of all the tables
    #/*****************************************************************************************************/
    def do_print_attr_of_all_static_tables(self, arg):
      '''
        Print Attributes and Values of all Tables: Enter [ devId ]
      '''
      args = tuple(map(int, arg.split()))
      if  len(args) < 1:
          print 'Invalid input, Enter Device-Id'
      else:
          print('Input Arguments are devId=%d' % (args[0]))
          xpDeviceMgr.instance().getDeviceObj(args[0]).getHwAccessMgr().xpPrintStaticTableInfo(args[0], -1)

    #/*****************************************************************************************************/
    # command to select user interface to PCIe driver to access register space
    #/*****************************************************************************************************/
    def do_set_reg_access_mode(self, arg):
        '''
         To set reg access mode of pcie(ioctl/mmap): Enter  [ devId,interface ]'
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter devId, IOCTL(0)/MMAP(1)'
        else:
            if(args[1] == 1 or args[1] == 0):
                if(args[1] == 0):
                    interface = "IOCTL"
                else:
                    interface = "MMAP"
                print('Input Arguments are devId=%d, interface = %s' % (args[0], interface))
                xpDrvWrapperSetRegAccessMode(args[0], args[1])
            else:
                print 'Invalid input, Enter devId, IOCTL(0)/MMAP(1)'

    #/*****************************************************************************************************/
    # command to get user interface to PCIe driver to access register space
    #/*****************************************************************************************************/
    def do_get_reg_access_mode(self, arg):
        '''
         To get reg access mode of pcie(ioctl/mmap): Enter  [ devId ]
        '''
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter devId'
        else:
            mode = xpDrvWrapperGetRegAccessMode(args[0])
            if(mode == 0):
                print('devId=%d, mode = IOCTL' % (args[0]))
            elif(mode == 1):
                print('devId=%d, mode = MMAP' % (args[0]))

