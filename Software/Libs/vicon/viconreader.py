#!/usr/bin/python

#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 3.0 of the License, or (at your option) any later version.
#
#  The library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
# (c) Shai Revzen, U Penn, 2010

import struct
import socket 
import sys

"""
viconreader is a simple library for connecting to the real-time feed emitted
by a vicon motion capture system. 

It provides two functions:

(1) As a library, it provides the ViconReader class which gives an interface for
connect()-ing to and reading data from a vicon system. ViconReader gives its
user the list of variables emitted by the vicon, and a python generator object
which yields consecutive samples from the motion capture system. stop() and
close() methods allow the user to stop streaming data and to close a TCP
connection to the vicon.

(2) As a commandline tool, viconreader.py will connect to the DEFAULT_HOST, 
currently configured for the GRASP lab vicon (*NOT* high-bay), and save two 
files: a .dcr file containing the variable names, one per line, and a .dat file
containing samples as a continuous stream of 64 bit floats. Capture is stopped 
with a ctrl-c (or ctrl-break, if you use such systems).
"""
DEFAULT_HOST = "129.31.217.168"  
DEFAULT_PORT = 800
  
class ViconReader(object):
  """ViconReader instances provide access to the real-time data stream 
  made available by vicon motion tracking systems.
  
  Instances of ViconReader can send Query, Start and Stop messages and parse 
  responses into lists of attributes and blocks of doubles (encoded as strings).
  
  ViconReader is used from the commandline to log tracking data to a file.
  
  Typical use is:
  >>> V = ViconReader()
  >>> names = V.connect() # names gets list of names from Query Response
  >>> # V.stream() is a generator returning packet payload each .next()
  >>> for pkt,_ in zip(V.stream(),xrange(100)): 
  >>>   dat = struct.unpack("%dd" % (len(pkt)/8),pkt)
  >>>   print
  >>>   for nm,val in zip(name,dat):
  >>>     print nm,val
  >>> V.stop()
  """
  QUERY = (1,0)
  INFO = (1,1)
  START = (3,0)
  STOP = (4,0)
  DATA = (2,1)
  def __init__(self):
    self.sock = None
    self.push = ''

  def _get( self, fmt ):
    "Read data from socket based on format string, and parse it accordingly"
    N = struct.calcsize(fmt)
    # Start reading from push-back buffer
    buf = self.push[:min(len(self.push),N)]
    self.push = self.push[:len(buf)]
    while len(buf)<N:
      buf += self.sock.recv(N-len(buf))
    return struct.unpack(fmt,buf)

  def _parseInfo( self ):
    "Parse an INFO packet, starting with byte after the header"
    N = self._get("1L")[0]
    lst = []
    for _ in xrange(N):
      L = self._get("1L")[0]
      lst.append(self._get("%ds" % L)[0])
    return lst
  
  def _parseData( self ):
    "Parse a DATA packet, starting with byte after the header"
    N = self._get("1L")[0]
    return self._get("%ds" % (N*8))[0]
  
  def _parse( self ):
    "Parse an incoming packet"    
    hdr = self._get("2L")
    if hdr==self.__class__.DATA:
      return (hdr, self._parseData())
    elif hdr==self.__class__.INFO:
      return (hdr, self._parseInfo())
    # Failed -- need to resync
    self.push = struct.pack("2L",*hdr)
    self._resync()
  
  def _resync(self):
    raise ValueError,"Lost synchronization on socket"

  def _cmd( self, hdr ):
    "Command encoded as a 2-tuple header"
    self.sock.send(struct.pack( "2L", *hdr ))
  
  def connect( self, host = DEFAULT_HOST, port = DEFAULT_PORT ):
    # Connect the socket
    self.sock = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.sock.connect((host,port))
    # Send a query
    self._cmd( self.__class__.QUERY )
    # Loop until a query response is received
    while True:
      hdr,names = self._parse()
      if hdr == self.__class__.INFO:
        return names
          
  def stream( self, fps=1e9 ):
    """
    Generator producing a stream of data packet payloads

    INPUTS
      fps -- expected fps, for throwing away stale data. 
        If not set, all data will be returned. Set to the expected vicon 
        frames-per-second data capture rate; typically 120
    """
    self._cmd( self.__class__.START )
    thresh = 0.5/fps
    while True:
      # Make sure we enter data reading loop at least once
      last = time.time() + 1e6
      while time.time() < thresh + last:
        last = time.time()        
        hdr,data = self._parse()
      if hdr == self.__class__.DATA:
        yield data

  def stop( self ):
    "Tell Vicon to stop streaming"
    self._cmd( self.__class__.STOP )
  
  def close( self ):
    "Close connection to Vicon"
    self.sock.close()
    self.sock = None

if __name__=="__main__":
  hosts = dict( 
    highbay = "192.168.129.65",
    grasp =  "10.66.68.1",
    dyson =  "129.31.217.168",
    DEFAULT = "146.179.206.65" 
  )
  fn = None
  host = hosts['DEFAULT']
  port = DEFAULT_PORT
  argv = []; argv[:] = sys.argv[1:]
  err = ''
  while argv:
    arg = argv.pop(0).lower()
    if arg in ['-h','--host']:
      arg = argv.pop(0)
      host = hosts.get( arg.lower(), arg )
      continue
    if arg in ['-p','--port']:
      port = int(argv.pop(0))
      continue
    if arg in ['--help','--usage']:
      err = ' '
      break
    if arg in ['-a','--aliases']:
      for k,v in hosts.iteritems():
        print "%-16s : %s" % (k,v)
      sys.exit(0)
    if arg[0] != '-':
      if fn is not None:
        err = 'Only one filename allowed on commandline; got "%s" and "%s"' % (fn,arg)
        break
      fn = arg
      continue
    err = 'Unknown commandline argument "%s"' % arg
    break
  if fn is None:
    err = "Must specify filename on the commandline"
  if err:
    sys.stderr.write("%s\n" % err)
    sys.stderr.write("""
Usage: %s {options} filename

  Creates filename.dcr with the field names and filename.dat with the raw
  data stored as doubles.
  
  Options:
   -h <hostname>, --host <hostname> 
        Specify VICON hostname where hostname is an IP address, FQDN or alias 

   -p <port>, --port <port> 
        Specify VICON port to connect to 

   -a, --aliases
        List the builtin host aliases
   
   --help, --usage
        Print this message
  """ % sys.argv[0])
    sys.exit(5)
  # Filename user requested
  print "Recording from %s:%d to %s.dcr, %s.raw" % (host,port,fn,fn)   
  V = ViconReader()
  names = V.connect(host=host)
  # Write descriptor
  dcr = open(fn+".dcr", "w")
  dcr.write("\n".join(names)+"\n")
  dcr.close()
  # Write data
  L = 0
  N = 0
  dat = []
  try: # for catching ctrl-c termination
    for pkt in V.stream():
      if not L: # First packet sets the length we'll expect 
        L = len(pkt)
      elif L != len(pkt):
        print "\nUnexpected packet of length %d instead of %d" % (len(pkt),L)
        continue
      #P dat.write(pkt)
      dat.append(pkt)
      N += 1
      if (N%50)==0:
        print "\r%5d" % N,
    # ---
  except KeyboardInterrupt, ex:
    #P dat.close()
    f = open(fn+".dat", "w")
    f.write("".join(dat))
    f.close()
    V.stop()
    print "\nStop."
    


