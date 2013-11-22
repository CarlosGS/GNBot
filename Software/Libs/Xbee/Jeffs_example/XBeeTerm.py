#!/usr/bin/env python
"""XBeeTerm.py is a XBee serial command shell for interacting with XBee radios

   This command interpretors establishes communications with XBee radios so that AT Commands can be sent to the XBee.
   The interpretors output is color coded to help distinguish user input, from XBee radio output, and from
   interpretors output. This command-line interpretor uses Python modual Cmd, and therefore, inherit bash-like history-list
   editing (e.g. Control-P or up-arrow scrolls back to the last command, Control-N or down-arrow forward to the next one,
   Control-F or right-arrow moves the cursor to the right non-destructively, Control-B or left-arrow moves the cursor
   to the left non-destructively, etc.).

   XBeeTerm is not a replacement for the Digi X-CTU program but a utility program for the Linux envirnment.  You can pipe
   scripts of XBee configration commands, making it easy to multiple radios.  Also, XBeeTerm wait for the arrival of a
   XBee data packet, print the XBee frame, and wait for the next packet, much like a packet sniffer.

   XBeeTerm Commands:
       baudrate <rate>       set the baud rate at which you will communicate with the XBee radio
       serial <device>       set the serial device that the XBee radio is attached
       watch               wait for the arrival of a XBee data packet, print it, wait for the next
       shell or !          pause the interpreter and invoke command in Linux shell
       exit or EOF         exit the XBeeTerm
       help or ?           prints out short discription of the commands (similar to the above)

   Just like the Digi X-CTU program, the syntax for the AT commands are:
       AT+ASCII_Command+Space+Optional_Parameter+Carriage_Return
       Example: ATDL 1F<CR>

   Example Session:
       baudrate 9600           # (XBeeTerm command) set the baudrate used to comm. with the XBee
       serial /dev/ttyUSB0     # (XBeeTerm command) serial device which has the XBee radio
       +++                     # (XBee command) enter AT command mode on the XBee
       ATRE                    # (XBee command) restore XBee to factory settings
       ATAP 2                  # (XBee command) enable API mode with escaped control characters
       ATCE 0                  # (XBee command) make this XBee radio an end device
       ATMY AAA1               # (XBee command) set the address of this radio to eight byte hex
       ATID B000               # (XBee command) Set the PAN ID to eight byte hex
       ATCH 0E                 # (XBee command) set the Channel ID to a four byte hex
       ATPL 0                  # (XBee command) power level at which the RF module transmits
       ATWR                    # (XBee command) write all the changes to the XBee non-volatile memory
       ATFR                    # (XBee command) reboot XBee radio
       exit                    # (XBeeTerm command) exit python shell

   Referance Materials:
       XBee 802.15.4 (Series 1) Module Product Manual (section 3: RF Module Configuration)
           ftp://ftp1.digi.com/support/documentation/90000982_A.pdf
       python-xbee Documentation: Release 2.0.0, Paul Malmsten, December 29, 2010

http://python-xbee.googlecode.com/files/XBee-2.0.0-Documentation.pdf

       cmd - Support for line-oriented command interpreters

http://docs.python.org/2/library/cmd.html

       cmd - Create line-oriented command processors

http://bip.weizmann.ac.il/course/python/PyMOTW/PyMOTW/docs/cmd/index.html

       Easy command-line applications with cmd and cmd2

http://pyvideo.org/video/306/pycon-2010--easy-command-line-applications-with-c

   Orginally Created By:
       Amit Snyderman (amit@amitsnyderman.com), on/about August 2012, and taken from

https://github.com/sensestage/xbee-tools

   Modified By:
       Jeff Irland (jeff.irland@gmail.com) in January 2013
"""

# imported modules
import os                       # portable way of using operating system dependent functionality
import sys                      # provides access to some variables used or maintained by the interpreter
import time                     # provides various time-related functions
import cmd                      # provides a simple framework for writing line-oriented command interpreters
import serial                   # encapsulates the access for the serial port
import argparse                 # provides easy to write and user-friendly command-line interfaces
from xbee import XBee           # implementation of the XBee serial communication API
from pretty import switchColor  # colored text in Python using ANSI Escape Sequences

# authorship information
__author__ = "Jeff Irland"
__copyright__ = "Copyright 2013"
__credits__ = "Amit Snyderman, Marije Baalman, Paul Malmsten"
__license__ = "GNU General Public License"
__version__ = "0.1"
__maintainer__ = "Jeff Irland"
__email__ = "jeff.irland@gmail.com"
__status__ = "Development"
__python__ = "Version 2.7.3"

# text colors to be used during terminal sessions
CMD_INPUT_TEXT = 'normal'
CMD_OUTPUT_TEXT = 'bright yellow'
XBEE_OUTPUT_TEXT = 'bright red'
SHELL_OUTPUT_TEXT = 'bright cyan'
WATCH_OUTPUT_TEXT = 'bright green'

class ArgsParser():
   """Within this object class you should load all the command-line switches, parameters, and arguments to operate this utility"""
   def __init__(self):
       self.parser = argparse.ArgumentParser(description="This command interpretors establishes communications with XBee radios so that AT Commands can be sent to the XBee.  It can be used to configure or query the XBee radio", epilog="This utility is primarily intended to change the AT Command parameter values, but could be used to query for the parameter values.")
       self.optSwitches()
       self.reqSwitches()
       self.optParameters()
       self.reqParameters()
       self.optArguments()
       self.reqArguments()
   def optSwitches(self):
       """optonal switches for the command-line"""
       self.parser.add_argument("--version", action="version", version=__version__, help="print version number on stdout and exit")
       self.parser.add_argument("-v", "--verbose", action="count", help="produce verbose output for debugging")
   def reqSwitches(self):
       """required switches for the command-line"""
       pass
   def optParameters(self):
       """optonal parameters for the command-line"""
       pass
   def reqParameters(self):
       """required parameters for the command-line"""
       pass
   def optArguments(self):
       """optonal arguments for the command-line"""
       self.parser.add_argument(nargs="*", action="store", dest="inputs", default=None, help="XBeeTerm script file with AT Commands to be executed")
   def reqArguments(self):
       """required arguments for the command-line"""
       pass
   def args(self):
       """return a object containing the command-line switches, parameters, and arguments"""
       return self.parser.parse_args()

class XBeeShell(cmd.Cmd):
   def __init__(self, inputFile=None):
       """Called when the objects instance is created"""
       cmd.Cmd.__init__(self)
       self.serial = serial.Serial()
       if inputFile is None:
           self.intro = "Command-Line Interpreter for Configuring XBee Radios"
           self.prompt = "xbee% "
       else:
           self.intro = "Configuring XBee Radios via command file"
           self.prompt = ""            # Do not show a prompt after each command read
           sys.stdin = inputFile

   def default(self, p):
       """Command is assumed to be an AT Commands for the XBee radio"""
       if not self.serial.isOpen():
           print "You must set a serial port first."
       else:
           print 'Sending: %s\r' % p
           if p == '+++':
               self.serial.write('+++')
               time.sleep(2)
           else:
               self.serial.write('%s\r' % p)
               time.sleep(0.5)

           output = ''
           while self.serial.inWaiting():
               output += self.serial.read()
           if output == '' :
               print 'XBee timed out, so reissue "+++". (Or maybe XBee doesn\'t understand "%s".)' % p
           else:
               switchColor(XBEE_OUTPUT_TEXT)
               print output.replace('\r', '\n').rstrip()

   def emptyline(self):
       """method called when an empty line is entered in response to the prompt"""
       return None     # do not repeat the last nonempty command entered

   def precmd(self, p):
       """executed just before the command line line is interpreted"""
       switchColor(CMD_OUTPUT_TEXT)
       return cmd.Cmd.precmd(self, p)

   def postcmd(self, stop, p):
       """executed just after a command dispatch is finished"""
       switchColor(CMD_INPUT_TEXT)
       return cmd.Cmd.postcmd(self, stop, p)

   def do_baudrate(self, p):
       """Set the baud rate used to communicate with the XBee"""
       self.serial.baudrate = p
       print 'baudrate set to %s' % self.serial.baudrate

   def do_serial(self, p):
       """Linux serial device path to the XBee radio (e.g. /dev/ttyUSB0)"""
       try:
           self.serial.port = p
           self.serial.open()
           print 'Successfully opened serial port %s' % p
       except Exception, e:
           print 'Unable to open serial port %s' % p

   def do_shell(self, p):
       """Pause the interpreter and invoke command in Linux shell"""
       print "running shell command: ", p
       switchColor(SHELL_OUTPUT_TEXT)
       print os.popen(p).read()

   def do_watch(self, p):
       """Wait for the arrival of a XBee data packet, print it when it arrives, wait for the next"""
       if not self.serial.isOpen():
           print "You must set a serial port first."
       else:
           print "Entering watch mode..."
           switchColor(WATCH_OUTPUT_TEXT)
           while 1:
               packet = xbee.find_packet(self.serial)
               if packet:
                   xb = xbee(packet)
                   print xb

   def do_exit(self, p):
       """Exits from the XBee serial terminal"""
       self.serial.close()
       print "Exiting", os.path.basename(__file__)
       return True

   def do_EOF(self, p):
       """EOF (end-of-file) or Ctrl-D will return True and drops out of the interpreter"""
       self.serial.close()
       print "Exiting", os.path.basename(__file__)
       return True

   def help_help(self) :
       """Print help messages for command arguments"""
       print 'help\t\t', self.help_help.__doc__
       print 'shell <cmd>\t', self.do_shell.__doc__
       print 'EOF or Ctrl-D\t', self.do_EOF.__doc__
       print 'exit\t\t', self.do_exit.__doc__
       print 'watch\t\t', self.do_watch.__doc__
       print 'serial <dev>\t', self.do_serial.__doc__
       print 'baudrate <rate>', self.do_baudrate.__doc__

# Enter into XBee command-line processor
if __name__ == '__main__':
   # parse the command-line for switches, parameters, and arguments
   parser = ArgsParser()                           # create parser object for the command-line
   args = parser.args()                            # get list of command line arguments, parameters, and switches

   if args.verbose > 0:                         # print what is on the command-line
       print os.path.basename(__file__), "command-line arguments =", args.__dict__

   # process the command-line arguments (i.e. script file) and start the command shell
   if len(args.inputs) == 0:   # there is no script file
       shell= XBeeShell()
       shell.cmdloop()
   else:                           # there is a script file on the command-line
       if len(args.inputs) > 1:
           print os.path.basename(__file__), "will process only the first command-line argument."
       if os.path.exists(args.inputs[0]) :
           inputFile = open(args.inputs[0], 'rt')
           shell = XBeeShell(inputFile)
           shell.cmdloop()
       else:
           print 'File "%s" doesn\'t exist. Program terminated.' % args.inputs[0]

