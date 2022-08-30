#!/usr/bin/env python
#  cmdsObj.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell’s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

import sys
import time
import os
import re
import readline

dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd

#/*****************************************************************************************************/
# sub-commands for XPS layer FDB functions
#/*****************************************************************************************************/
def do_fdb(self, s):
    'XPS FDB Commands'
    i = xpsFdbObj()
    i.prompt = self.prompt[:-1]+':fdb)'
    while True:
        try:
            i.cmdloop()
            break
        except (ValueError, IndexError), e:
            print 'Invalid inputs format, check help <command> or ? for usage'
            continue

#/*********************************************************************************************************/
#/* main program */
#/*********************************************************************************************************/

if __name__ == '__main__':
    #/*****************************************************************************************************/
    #Init/addDevice for Device manager and tableManager
    # Initialize only of we are running in standalone mode
    #init and redirect logger prints to console
    #/*****************************************************************************************************/
    arg3 = sys.argv[1]
    arg2 = arg3.lower()
    arg1 = arg3.upper()


    print('')
    print('    #/*****************************************************************************************************/')
    print('    # sub-commands for XPS layer %s functions' % (arg1))
    print('    #/*****************************************************************************************************/')
    print('    def do_%s(self, s):' % (arg2))
    print('        \'XPS %s Commands\'' % (arg1))
    print('        i = xps%sObj()' % (arg3))
    print('        i.prompt = self.prompt[:-1]+\':%s)\'' % (arg2))
    print('        while True:')
    print('            try:')
    print('                i.cmdloop()')
    print('                break')
    print('            except (ValueError, IndexError), e:')
    print('                print \'Invalid inputs format, check help <command> or ? for usage\'')
    print('                continue')
    print('        del i')
    print('')

#/*********************************************************************************************************/
#/* End of main program */
#/*********************************************************************************************************/


