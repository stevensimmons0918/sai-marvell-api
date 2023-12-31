Current library contains utilities to enable traffic to application demo simulation from SmartBits simulation 
and from ethernet NIC's.


---------- HowTo ------------------

To run simulation and be able to send traffic to it:
1) run ./slanConnector_Linux -D
   (-D - optional key, means run as daemon;
    -h - for help)
2) If traffic should come to simulation by ethernet NIC's run slanEthPortLinux for every NIC, for example:
   ./slanEthPortLinux -D -n slan00 eth1
	(-D - optional key, means run as daemon;
	 -n slan00 - slan name corresponding to slan names in ini-file of simulation;
	 eth1 - name of NIC shown by /sbin/ifconfig command;
         -h - for help)
Note: For FreeBSD run slanConnector_FreeBSD with same options;
      On Linux you must have root privileges to run slanEthPortLinux, because it opens NIC devices;

3) if traffic not generated by NIC's run ./SmbSim (from SmbSim_ClientServ\Serv\linux\SmbSim)

4) run ./appDemoSim -i Puma_pss_wm_RDE.ini

5) To run SmbSim GUI SmbSim_ClientServ\Gui\linux\run_SmbSimGUI could be used;
   (There is no SmbSim GUI for FreeBSD, only server)

------- List of files ----------

slanConnector.c      - broker program connecting appDemoSim with SmbSim or Ethernet driver

slanConnector_Linux/slanConnector_FreeBSD - connectors binary (executable)

        !!!!
Note: connector must run before SmbSim server!!!!



slanEthPortFreeBsd.c - Ethernet driver for FreeBSD


slanEthPortFreeBsd   - binary (executable) of Ethernet driver for FreeBsd

slanEthPortLinux.c   - Ethernet driver for Linux

slanEthPortLinux     - binary (executable) of Ethernet driver for Linux


slanLib.c - code of slan library interface between appDemoSim, SmbSim, EthDriver's from one side and connector 
            from other side.

slanLib.h - slan library API's must be copied to:
                             
                 smbsim_clientserver/SLAN/src/linux
                             
                 smbsim_clientserver/SLAN/src/freeBSD
                             
                 cpss/mainOs/src/gtOs/linux/linuxSim
                 cpss/mainOs/src/gtOs/FreeBSD/stubs


testConnector.c - utility that checking if connector programm is running


testConnectorLinux   - exe for Linux of connector checking program



testSlan.c           - simple slan mechanizm testing program



testSlanPerf.c       - slan mechanizm performance testing program



unixSlanUtilsBuild.sh - script to build slan utilities. For FreeBSD objects and exe will be built in /tmp/objs 
                        if not defined other; for Linux they will be built in local directory.
                        (if you see problems with this script just run 'gmake all')

gtBuild              - to avoid compiling this directory with appDemo
 in case it will ever appear in pss/cpss tree
