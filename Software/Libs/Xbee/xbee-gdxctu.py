#!/usr/bin/env python
# This is a program written in the Python Programming language.
#
# This program is Copyright (c) Gregory Dudek, 2009, 2010, 2011.
# License:
#   Creative Commons Attribution-Noncommercial-Share Alike 3.0 License
#
# You can use it for free and redistribute it at will
# for any non-commercial application under certain conditions including 
# retaining the attribution; see
# http://creativecommons.org/licenses/by-nc-sa/3.0/us/
# for details.
#
# Re. commands:
# Note: by downloading a specific detected firmware image, we could
#  unzip it and read the ".mxi" file
#  to extract exactly the right commands for the specific firmware (with 
#  explanations).  Another day...
#
doc = """This program is used to examine or change the configuration of a Digi Xbee module.
The functionality is meant to replicate some of that offered by XCTU.  By entering 2-letter
codes, you can examine many settings.  Some are displayed by default, hit '?' for a complete
list.  Use the command '+++' to re-enter command mode if necessary (since it is terminated if 
nothing it typed for a few seconds. 
  The first time the program is run it searches for the correct baud rate.  It
remembers the value and should be quicker subsequently.
  This program will remember the last successfully baud rate and other data in a 
defaults file called xbee-gdxctu.py.rc (the program name with '.rc' tacked on).
"""


VERSION = "2.15"
VERSIONTEXT = "Copyright Gregory Dudek (c) 2009, 2010, 2011, 2012. changeset 15"
ORIGIN  = "http://www.dudek.org/blog/180"
SELFDOWNLOAD="http://www.dudek.org/blog/Downloads/xbee-gdxctu.py"
debug=0


import os,sys,time
import glob
import re

try:
    import serial   # non-stardard serial IO module that we really need
except ImportError:
    print "ERROR: You seem to be missing the serial module."
    print "It can be downloaded for free from:"
    print "    http://sourceforge.net/projects/pyserial/files/pyserial/"
    sys.exit(1)
    
try:
    import io # for Python version 2.6 and above (never checked with 3. though)
except: pass

############################################################################
# These values can be overridden via the preferences file.
############################################################################
lastbaud = 0  # default, will be verified and maybe reset
defaultPAN = "3332"  # this can be modified in the auto-generated defaults file: xbee-gdxctu.py.rc
VERSIONCHECKDAYS=30  # check for updated version every 30 days


############################################################################
# XBee-specific information
############################################################################
# BD codes mapped to actual baud rates.
baudmap={"2":"2400 baud", "3":"9600 baud", "4":"19200 baud", "5":"38000 baud","6":"57600", 
         "7":"115200 baud (wow!)"}
#
# List of known commands. These combine possibilities from different firmwares.
# A minus suffix suffix - means don't actually run it initially
commands = { 
  "ID": "Addressing: The network ID (my default "+defaultPAN+")",
  "CH": "Addressing: The channel of the module",
  "SH": "Addressing: Serial number of module (high word)",
  "SL": "Addressing: Serial number of module (low word) ",
  "MY": "16-bit address of the spefic module",
  "DH": "Addressing: destination address for wireless communication (hi word)",
  "DL": "Addressing: destination address for wireless communication (lo word)",
  "BD": "baud code (2:4800, 3:9600, 4:19200, 5:38400, 6:57600 )",
  "CE": "Coordinator Enable. Set/Read the coordinator setting. (0:end device, 1:coord)",
  "NI": "Addressing: Node ID",
  "SC-": "channels list for scans, returns bitfiels (bit0:chan 0xB, bit15:0x1a)",
  "SD-": "scan duration per channel (0-0F,exponential, biggest -> 15 MINUTES)",
  "VR-": "Firmware version",
  "VL": "Firmware version verbose",
  "RR-": "Max retries to send (above the 3 minimum)", # Newer firmware only
  "MM-": "Max mode value for 802.15.4 header",
  "WR-": "Save any changes to EEPROM",
  "NR-": "Network reset. Will lose all routing info.  With parameter 1, effects all nodes on PAN.",
  "DD-": "Device Type Identifier. Stores a device type value. Can be used to differentiate nodes",
  "DE-": "Destination Endpoint.",
  "CC-": "Character used to enter command mode, default 2B is a '+'",
  "RE-": "Restore factory default setting",
  "CN-": "Exit command mode",
  "FR-": "Software Reset. Reset module.",
  "AS-":  "Active channels scan",
  "OP-":  "Operating extended PAN ID (ZB firmware) 64-bit version of PAN ID in use",
  "OI-":  "Operating extended PAN ID (ZB firmware) short-form PAN ID in use",
  "EE-":  "Encryption enable (1:AES encryption enabled)",
  "KY-": "AES encryption key (16 bytes, cannot be read, only set)",
  "PL-":  "Power level (0-4, not useful for Japanese units)",
  "RO":  "Serial packetizing timeout",
  "AO-": "Serial API output format (0:standard, 1:explicit addressing)",
  "FT-": "Serial API flow control buffer threshold",
  "NB-": "Serial parity (0:none, 1:even, 2:odd, 3:forceHI, 4:forceLow)",
  "D6-": "DIO6 Configuration. (0:input, 1:power LED output, 4:low)", 
  "D5-": "DIO5 Configuration. (0:input, 1:power LED output, 4:low)", 
  "P0-":  "DIO10 Configuration. (0:input, 1:RSSI, 2:PWM0, 4:low, ... )",
  "P1-": "DIO11 Configuration. (0:input, 2:PWM1, 4:low, ... )",
  "RP-":  "RSSI Timer. Time (x100 ms) RSSI signal will be output after last transmission.",
  "HV-":  "Hardware version number",
  "ER-":  "RF error count (packets with bad CRC, max FFFF)",
  "TR-":  "Transmission errors (packets lost)",
  "GD-":  "Good macket count (max FFFF)",
  "RC-":  "RSSI value (DBM level on channel, 0 (best)..5E(worst), see also DB)",
  "DB":  "Received Signal Strength [in dB] of last good packet, 0[best]..5E (see RC)",
  "NT":  "Node discovery timeout, how long to look for others (for ND and DN commands)",
  "DN-": "Discover a node identifier mapping a physical address (e.g. DNfoobar).\n     Sets DL & DH and exits command mode.",
  "ND":  "Discover all other nodes (takes NT*100 ms).  MY/SH/SL/DB/NI/?/?/?/?/?",
  "SM-":  "Sleep mode: (0:nosleep, 1:pinHibernate, 4:cyclic). Uses Sleep_RQ (pin 9) ",
  "ST-": "Sleep mode: time before sleep (cyclic mode)",
  "Q -":  "Quit.",
  "AP-":  "API mode",
  "CT-":   "Command Mode Timeout. Set/Read inactivity delay to exit command mode (ms)",
  "GT-":  "Guard time (required delay after +++ to enter AT mode)",
  "%V":   "Vcc Voltage (multiply by 0.001173; thus 0x8FE represents 2.70v, AEC is 3.28v)",
  "+++":  "Special code to enter command-entry mode (wait 1 sec before hitting return)",
  }


def identification():
    """ Try to identify the device and firmware.  This is not complete and is based on 
        a lot of guesswork and unverified web lore.
    """
    global firmware, hwDeviceCode
    ser.write("ATVR\r")
    if io_support_for_universal_newlines: firmware = ser.readline()
    else: firmware = ser.readline(eol='\r')
    firmware = firmware.strip()  # e.g. 1221
    saveprefs()  # save the successful baud rate, in case of exit by control-C later
    ser.write("ATHV\r")
    
    if io_support_for_universal_newlines: hwDeviceCode = ser.readline()
    else: hwDeviceCode = ser.readline(eol='\r')
    
    hwDeviceCode = hwDeviceCode.strip().upper()   # e.g. 1941

    if hwDeviceCode[0:2] in [ "18","17" ]:
        print "XBee Series 1 device."
        
    if hwDeviceCode[0:2] in [ "1A","19" ]:
        if firmware[0]=="1": print "XBee Series 2 device."
        if firmware[0]=="2": print "XBee Zigbee (ZB) device."

    print "Firmware:",firmware
    if re.search("10[A86].",firmware):
        if firmware[1]=="0":
           print "Series 1, Non-Beacon Enabled 802.15.4 Code"
        elif firmware[1]=="1":
           print "Series 1, Beacon Enabled 802.15.4 Code"
    if re.search("1.4.",firmware):
       print "ZNet 2.5 firmware EmberZNet 2.5.4"#ftp1.digi.com/support/firmware/93009374_C.txt
    elif firmware[2]=="2":
       #if re.search("1.2.",firmware):  # Zigbee/ZB for Series 2
       print "Zigbee/ZB firmware"
    elif re.search("2.4.",firmware):
       print "ZB Firmware"
    if re.search("2.6.",firmware): #http://ftp1.digi.com/support/firmware/93009373_C.txt
       print "ZB Firmware EmberZNet 3.3.1 ZigBee-PRO stack"



def list_Key_values():
    """ Print the dictionary of keywords and their meanings, and
    execute all the ones that don't end in a minus sign.  Treat a few specailly: this
    makes for ugly code that is less generic, but more utility.
    """
    global commands, keylist
    for i in keylist:
      # skip commands we don't want to run due to danger or boredom
      if i[2:]=="-": continue
      if i=="+++": continue

      # skip commands not supported by this firmware
      if re.search("1.[A8].",firmware):  # 802.15.4 
          if i in [ "%V","NT","DB" ]: continue
      if re.search("131.",firmware):  # Router development version, Series 2, ZB or ZNet?
          if i in [ "%V","CE","DB" ]: continue
      if re.search("1.2.",firmware):  # Zigbee/ZB for Series 2  1.2.
          if i in [ "CE","DB" ]: continue
      if re.search("1.4.",firmware):  # ZNet 2.5  1.4.
          if i in [ "CE" ]: continue
      if re.search("2.4.",firmware):  # ZB http://ftp1.digi.com/support/firmware/93009373_B.txt
          if i in [ "CE" ]: continue

      # run the command and report the return value
      ser.write("AT"+i+"\r")
      print ">>", i,commands[i]
      # handle some commands specially
      if i[0:2]=="ND":
          # wait as long as it takes for double newline
          reply=""
          loops=0
          while (loops<8) and not "\n\n\n" in reply:
              s = ser.read(200).replace("\r","\n")
              if s:
                  reply = reply + s
                  print s
                  if s=="\n": 
                      break
              else: 
                  loops+=1
              #print "waiting for more.",map(ord,reply)
          reply = ""  # since we printed it already above.
      elif i[0:2]=="BD":
          
          if io_support_for_universal_newlines: reply = ser.readline()
          else: reply = ser.readline(eol='\r')
          reply = reply.replace("\r","\n").strip()
          try: reply = reply + " => " + baudmap[reply]
          except: pass
      elif i[0:2]=="%V":
          reply = ser.read(4).replace("\r","\n").strip()
          try: reply = reply + " equal to " + str(eval("0x"+reply)*0.001173) +" V"
          except: pass
      else:
          reply = ser.read(100).strip().replace("\r","\n")
      if "\n" in reply: print reply,"\n"
      elif reply: print i,reply


def  enterCommandMode():
        time.sleep(1)
        ser.flushInput()
        ser.write("+++")
        time.sleep(2)


############################################################################
# Generic preferences-file and setup methods
############################################################################
import re
inquotes = re.compile(r'''\s*(".*?"|'.*?')(.*)''')
badchars = re.compile(r'''^[^'," \[\]\(\)#]+$''')
##commented_line = re.compile(r'''\s*([^#]*)\s*(#.*)''')
paramfinder = re.compile(r'''(?:'.*?')|(?:".*?")|(?:[^'",\s][^,]*)''') 
unquoted = re.compile(r'''
    ([^\#,"'\(\)\[\]][^\#,\]\)]*)  # value
    \s*                         # whitespace - XXX not caught
    ([\#,\)\]].*)?                  # rest of the line
    $''', re.VERBOSE)
def elem_quote(member, nonquote=True, stringify=False, encoding=None):
    """
    Simple method to add the most appropriate quote to an element - either single 
    quotes or double quotes.
    If ``nonquote`` is set to ``True`` (the default), then if member contains none 
    of ``'," []()#;`` then it isn't quoted at all.
    If ``stringify`` is set to ``True`` (the default is ``False``) then non string 
    (unicode or byte-string) values will be first converted to strings using the 
    ``str`` function. Otherwise elem_quote raises a ``TypeError``.
    """
    if not isinstance(member, basestring):
        if stringify:
            member = str(member)
        else:
            # FIXME: is this the appropriate error message ?
            raise TypeError('prefs file - Can only quote strings. "%s"' % str(member))
    if encoding and isinstance(member, str):
        # from string to unicode
        member = unicode(member, encoding)
    if '\n' in member:
        raise QuoteError('Multiline values can\'t be quoted.\n"%s"' % str(member))
    #
    if nonquote and badchars.match(member) is not None:
        return member
    # this ordering of tests determines which quote character will be used in 
    # preference - here we have \" first...
    elif member.find('"') == -1:
        return '"%s"' % member
    # but we will use either... which may not suit some people
    elif member.find("'") == -1:
        return "'%s'" % member
    else:
        raise QuoteError('Value can\'t be quoted : "%s"' % member)


def putpref(f,var,comment=""):
  """ Save a single variable in the preferences file."""
  try:
    if type(eval(var))==type("a"):
        f.write(var+" = ")
        f.write(elem_quote(eval(var),nonquote=0))
        if comment: f.write('  #  '+comment)
        f.write("\n")
    else:
        f.write(var+" = "+str(eval(var)))
        if comment: f.write('#  '+comment)
        f.write("\n")
  except: pass

def saveprefs():
    """Save stuff in the preferences file."""
    f=open(PREFS,"w")
    f.write("# Preferences file for version "+VERSION+"\n")
    putpref(f,"ORIGIN"," # where to look for an updated version")
    putpref(f,"lastbaud"," # last baud rate successfully use or manually specificied")
    putpref(f,"firmware")
    putpref(f,"defaultPAN")
    putpref(f,"next_version_check"," # when to check again")
    putpref(f,"VERSIONCHECKDAYS"," # how often to check the version (days)")
    f.close()

def versioncheck():
    """See if there is a more recent version of the program available.
    This can be disabled in the preferences file.
    """
    global next_version_check
    if VERSIONCHECKDAYS<0: return
    try:
       # Check latest version for update (non-fatal if it doesn't work)
       if debug or next_version_check < time.time():
           import urllib
           next_version_check = time.time() + 60*60*24*2  # wait 2 days if failure or killed.
           print "Automatic check for updated version of this program.  I hope that's cool."
           latest=urllib.URLopener().open(SELFDOWNLOAD+"/getProperty?id=version").read()
           if len(latest)>1 and latest != VERSION:
              print "** NOTE: version ("+latest+") of this program seems to be available."
              print "** You are using version",VERSION
              print "**     ",VERSIONTEXT
              try:
                  s = urllib.URLopener().open(SELFDOWNLOAD+"/getProperty?id=whatsnew").read()
                  print "The new version features:",s
              except: pass
              print "If you wish, you can get the latest version at",ORIGIN
           next_version_check = time.time() + 60*60*24*VERSIONCHECKDAYS  # every 7 days?
    except: 
        next_version_check = time.time() + 60*60*24*2  # wait 2 days if failure
        pass

#
############################################################################
# main code, at last!
############################################################################
#


############################################################################
# Initialization code
############################################################################

print "\nxbee-gdxctu XBee configuration tool.", VERSION,VERSIONTEXT
PREFS = sys.argv[0]+".rc"

next_version_check = -1
# Load preferences file, if possible.
v=VERSION
try:
    prefs=open(PREFS).read()
except: prefs=""
exec(prefs)
VERSION=v
versioncheck()

keylist = commands.keys()
keylist.sort()

# command line argument processing.
while len(sys.argv)>1:
    if sys.argv[1] in ["-h","-help","--help","-?"]:
        print "Inspect and edit Xbee configuration."
        print "Usage: ",sys.argv[0],"[-h] [--version] [--baud 9600]"
        print doc
        print "Options: --version  Print program version and, if possible, lastest available version."
        print "Options: -b N       specify baud rate (else auto detect)."
        print ""
        del sys.argv[1]
    elif sys.argv[1] == "-b" or sys.argv[1]=="--baud":
        lastbaud =  int(sys.argv[2])
        del sys.argv[1]
        del sys.argv[1]
    elif sys.argv[1] in ["-version","-v","-V","--version"]:
        del sys.argv[1]
        print "This is version",VERSION,"of this program."
        print "The latest released version is",
        sys.stdout.flush()
        try:
            import urllib
            print urllib.URLopener().open(SELFDOWNLOAD+"/getProperty?id=version").read()
        except: print "at ",ORIGIN
    elif sys.argv[1][0] != "-":
        # not a flag. This is a device name
        break
    else:
        print "Unknown option",sys.argv[1],"try -h for a list of options."
        sys.exit(1)
saveprefs()


# Try to auto detect serial port, or use the command-line option.
if len(sys.argv)>1:
    devname = sys.argv[1]
else: 
    usbs = glob.glob("/dev/tty.usbserial-*")
    if len(usbs)>1: 
        print "**** Warning: multiple USB devices. ***"
        print usbs
    if len(usbs)<1:
        print "*** ERROR: No serial device specified and no device of form /dev/tty.usbserial-*"
        print "You can specify the device on the command line."
        sys.exit(1)
    devname = usbs[0]
print sys.argv[0], "using", devname


# Search for the correct baud rate.
if lastbaud<=0:
    print "Searching for correct baud rate (if found, it will be tried first next time)."
    lastbaud = 9600

s=""
usebaud=0
# clear accumulated crud
base_ser = serial.Serial(devname, 115200, timeout=1)
try: 
    ser = io.TextIOWrapper(io.BufferedRWPair(base_ser, base_ser)) # python 2.6+
    io_support_for_universal_newlines = 1
except: 
    ser = base_ser  # try and support older python's just as happily.
    io_support_for_universal_newlines = 0
    

ser.flushInput()  # flush any crud, side effect is waiting until timeout (1 sec, I hope).

# Code to access bootloader. Doesn't work (yet).
# If we can do this, we can implement firmware uploading which is the last big
# missing feature.
"""
ser = serial.Serial(devname, 115200, timeout=2)
ser.setDTR(0)
ser.setRTS(1)
ser.sendBreak(20*4) # it's in units of 1/4 seconds
#ser.setBreak(1)  # causes error
ser.setDTR(0)
ser.setRTS(1)
print "Reset module, hit return."
sys.stdin.readline()
ser.sendBreak(1) # it's in units of 1/4 seconds
time.sleep(0.3)
#ser.setBreak(0)
ser.write("\r")
print ser.read(100)
print ser.read(100)
ser.write("\n")
print ser.read(100)
print ser.read(100)
print ser.read(100)
ser.close()
"""

# OK, now we try each baud rate in turn. 
# Start with the one known to work last time.
#
for baud in [lastbaud, 9600,19200,4800,57600,115200,38400,300, 1200, 2400]:
 for tries in [ 1,2 ]:
    s=""
    print "Try",baud,"baud ",
    sys.stdout.flush()
    ser = serial.Serial(devname, baud, timeout=1) # timeout > 1 sec is important for +++ handler
    enterCommandMode()
    
    if io_support_for_universal_newlines: s = ser.readline()
    else: s = ser.readline(eol='\r')
    
    if len(s)<1: 
        # maybe we are already in command mode, huh?
        ser.write("\r")
        
        if io_support_for_universal_newlines: s = ser.readline()
        else: s = ser.readline(eol='\r')
    
        ser.write("AT\r")
        if io_support_for_universal_newlines: s = ser.readline()
        else: s = ser.readline(eol='\r')
        s= s.strip()
    if len(s)>1: 
        if ord(s[0])>ord("Z"): s = s[1:]
        print "got: '"+s.strip()+"' " # ,len(s),"characters. ",map(ord,s)
        if (s=="OK") or (s[:2]=="OK") or (s.strip()=="OK"): 
            print "Connected."
            usebaud = baud
            break
    else: 
        print "[no OK response]",s
    ser.close()
 if usebaud > 0: break
 if (s=="OK"): break
print  "---"
lastbaud=usebaud


identification()

list_Key_values()
print "\nNow you can set any of the values. Hit '?' to see more options (not all of "
print "    them are useful with your firmware)."

#
#  Loop where we handle user commands to query or change settings.
#
changes = 0
while 1:
    print "Command and value (e.g. DL1234), 'q' to quit, 'keys', or ? >",
    cmd = sys.stdin.readline().strip()

    # Process a few commands specially, otherwise via table lookup.
    if cmd.lower() in [ "re", "nr", "cc" ]:
       # If we have this command in out table, print the explanation
       try: print commands[cmd[0:2].upper()]
       except: pass
       try: print commands[cmd[0:2].upper()+"-"]
       except: pass
       # Special dangerous commands that need confirmation
       if cmd.lower() in [ "re", "nr" ]:
           print "WARNING: All custom settings will be returned to factory default."
       while 1:
           print "Are you sure (yes, no)?"
           s=sys.stdin.readline().strip().lower()
           if s =="yes": break
           if s in ["n", "no", "0", "negative"]: 
               cmd = ""
               break
       if s !="yes": continue
    elif cmd == "?":
       # Special help command
       for i in keylist:
         print i[0:2]," \t",commands[i]
       continue
    elif cmd == "+++" or cmd == "+":
       # Special wakeup command to XBee
        enterCommandMode()
        if io_support_for_universal_newlines: s = ser.readline()
        else: s = ser.readline(eol='\r')
        s = s.strip()
        continue
    elif cmd == "keys":
       # Special command to quary specific default settings (what we ran at startup)
        list_Key_values()
        continue
    elif cmd == "q" or cmd == "'q'":
       # We're finished
       break
    elif cmd == "meter":
       # signal strength meter
         while 1:
             ser.write("ATDB\r")
             reply = ser.read(4).strip()
             print reply
    else:
         # Run any other XBee command, print the table-based info first if available.
         # make sure we didn't timeout from command mode
         ser.write("AT\r")
         reply = ser.read(4).strip()
         if reply != "OK": 
             print "re-enter command mode..."
             enterCommandMode()

         # If we have this command in out table, print the explanation
         try: print commands[cmd[0:2].upper()]
         except: pass
         try: print commands[cmd[0:2].upper()+"-"]
         except: pass
         # Does this command change the system state (in which case a WR prompt will needed)
         if len(cmd)>2: changes=1
         # Do the command
         ser.write("AT"+cmd+"\r")
         # Print the feedback
         print "AT"+cmd+"\n"+ser.read(100).strip().replace("\r","\n")
         if cmd.lower() == "wr": changes = 0
    # before going back for more, print this reminder
    if changes: print "YOU MUST USE THE COMMAND 'WR' TO SAVE YOUR CHANGES."

# Time to go home
# exit command mode
ser.write("ATCN\r")
saveprefs()
if changes: print """Warning: you seem to have quit without issuing a 'WR' command \
to commit changes to permanent memory.  If you power down the module \
before sending a WR, then any changes you made will be lost."""
