#
#  WFA Test Engine Source Code (DUT and Control PC) Revision 2.41 (April 4, 08)

# Release History:
# Base Release: Version 3.3.10 Qiumin Hu, Wi-Fi Alliance
#
# Revision: Version 4.0.0  Chandra Duba, Wi-Fi Alliance
#		- Added feature "CLI handling using shell scirpts"

This source code package contains a sample Linux implementation of the DUT code for 
the Wi-Fi Test Engine.  This DUT code includes the Wi-Fi Traffic Generator.  
The DUT code is written to be portable to other platforms.

Also included here is Linux source code for the Control PC component of the Wi-Fi Test Engine.
The sample code uses TCP over USB to provide connectivity between the DUT and the Control PC. 
This code will need to be modified to support alternate connectivity methods. 

The third component included is called the Test Console.  This component is a peer traffic 
Generator to the DUT's traffic generator that can run on the Control PC 

Please refer to the WFA Porting Guidelines document for detailed instructions on the porting process. 

The following instructions pertain to the direct use of this sample code in a Linux environment.

************************************************ IMPORTANT****************************************************
Before building the software, if you plan to support WPA2 as default, you can 
simply go to the step 1 "Building the software". 

If the DUT is preparing for WMM support, the lines in the file "Makefile.inc" 
for WMM is needed to be uncommented and the default setting for WPA2 should 
be commented out as well.

For WMM-PS, make sure to uncomment the line to build WMM-PS and comment out 
original build-line.

To support Voice, the build in the Makefile.inc must include WFA_WMM_EXT and 
WFA_WMM_VOICE


1. Building the software
   a. build all
      all you need is to simply type "make all".

   b. build individual modules
      cd into the appropriate directory to edit and type "make"

   c. build a specific agent 
      there are three agents in this package. They are "dut", "ca" and "tc", 
      standing for "Device Under Test", "Control Agent" and "Test Console" 
      respectively.

      To build one of them only, you need to edit the file "Makefile.inc" to
      comment rest of modules out

   d. clean modules and object files
      simply type "make clean" or cd into the module directories, 
      type "make clean"

2. Installing the software
   a. copy dut/wfa_dut to /usr/sbin (or other location) on the 
      linux DUT

   b. MUST copy scripts/getipconfig.sh to /usr/local/sbin on the Linux DUT. 

3. Running the software
   3.1 DUT -- dut/wfa_dut
   a. The current implementation is based on the open source wpa_supplicant
      (hostap.epitest.fi/wpa_supplicant). The DUT must have it installed.

   b. The supplicant can be either auto started during the system boot or 
      manually started by typing "wpa_supplicant -Dmadwifi -iath0 -c "any 
      configuration file". Before doing this, you need to have a console 
      terminal to access/login the DUT.
      Please refer the wpa_supplicant document/README and Linux setup document 
      for setting auto-start.

   c. The IP address for the control link interface (can be USB, ethernet, or 
      others with TCP/IP/aLink) needs to be set, e.g. 10.10.1.100 . This should be set for a differnt
      subnet from the traffic agent interface (Wi-Fi interface).

   d. Once you have already logged in the DUT, type:
      wfa_dut <control link interface, etc usb0> <local tcp port, e.g. 8000>  

   3.2 Control Agent (LINUX PC) -- ca/wfa_ca
   a. Before starting the control agent, first you need to make sure or set 
      the IP address for the control link interface (e.g. 10.10.1.110) that 
      matches the subnet set in the DUT. Second, you need to set two 
      environment variables. Type:
      a.1: export WFA_ENV_AGENT_IPADDR=<IP address of the DUT's control 
           interface, e.g. 10.10.1.100>
      a.2: export WFA_ENV_AGENT_PORT=<Port number of the DUT control TCP sits
           on, e.g. 8000>  

   b. Start the control agent:
      wfa_ca <local ethernet interface to communicate to others such as Test 
              Manager, CLI commander or Console with Python scripts e.g eth0> 
	      <local port to listen on, e.g. 9000> 

   c. Now you are ready.

   3.4 Console using Python Scripts.
       Read the instructions in the directory "ucc" for how to setup python if 
       it is not installed on your PC. It also has instructions for the script
       file modification.

   3.5  [REMOVED]

   3.6  [REMOVED]

   3.7 Test Console with Python Script
       see 3.4

   3.8 Running WMM-Power Save
       Before doing following, some necessary setup commands must be executed,
       such as set security, association and configure IP address, etc.

       a. sta_set_uapsd,interface,<wlan i/f, e.g. ath0>,ssid,<a test ssid>,acBE,<0 or 1>acBK,<0 or 1>,acVO,<0 or 1>,acVI,<0 or 1>
          Note: the AC value need to follow the test plan definition.
       b. traffic_agent_config,profile,uapsd,direction,receive,source<ip address of the console>,sourceport 12345,destinationport,12345
       c. traffic_agent_receive_start,streamid,<the streamid returned from the step "b"
       traffic_agent_receive_stop,streamid,<the streamid used for testing from the step "b"

    For details, please refer the WMM-Power Save Techops Manual and Test Plan 
    as well as the WMM-PS specification.
       

4. Debug Command
   There is a CLI command implemented to enable debug output. The debug levels
   include
   a. ERROR -- Default and not Changeable  (level 1)
   b. INFO  -- Can be turned on/off        (level 2)
   c. WARNING -- Can be turned on/off      (level 4)

   sta_debug_set,level,2,enable,1 
      will enable to debug INFO
   sta_debug_set,level,2,enable,0
      will disable it.


5.To enable the CLI handling using shell scripts
	1) copy the wfa_cli.txt to /etc/WfaEndpoint folder.
	2) modify the wfa_cli.txt to include the CLI's that are handled by shell script.
		- The format is <shell script>-<ResultFlag>
		
		- The ResultFlag can be either TRUE or FALSE. 
			TRUE represents that there is some return string which should be sent to UCC
			FALSE represents that there is no retrun string to send UCC
		EX: wfa_test_cli-TRUE,
	3) keep the named shell script ( should match with CLI defined in wfa_cli.txt) in /usr/bin or /usr/sbin so that the system can call.

For questions, send emails to sigmasupport@wi-fi.org or cduba@wi-fi.org or qhu@wi-fi.org

