#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import sys
import time
import bluetooth #pybluez!
# End modules

class GNBot(object):
    # Robot constructor, takes MAC address of wireless module as (i.e. "00:11:09:18:02:34")
    def __init__(self, bt_address):
        self.BT_ADDRESS = bt_address
        self.received_buffer = ""
        self.isConnected = False

    # Connects to the robot
    def connect(self):
        print("Connecting to GNBot with address "+self.BT_ADDRESS+"...")
        
        port = 1
        
        self.BT_socket = bluetooth.BluetoothSocket( bluetooth.RFCOMM )
        
        try:
            self.BT_socket.connect((self.BT_ADDRESS, port))
        except:
            print("ERROR: Could not connect")
            self.BT_socket.close()
            return -1
        
        self.BT_socket.settimeout(0.0) # Non blocking
        
        self.isConnected = True
        
        print("Checking connection...")
        
        time.sleep(0.5)
        if self.checkConnection() != 0:
            print("ERROR: Mismatching response. Connection failed")
            self.isConnected = False
            return -1
        
        print("Connected!")
        
        self.flushRecvBuffer()
        return 0

    # Closes connection with the robot
    def disconnect(self):
        #print("Closing connection...")
        if self.isConnected:
            self.BT_socket.close()


    # Empties the reception buffer
    def flushRecvBuffer(self): # We could also use flushInput(), but this way we can see the data that is being discarded
        if not self.isConnected: return
        val = self.recvChar()
        while len(val) > 0:
            if val != '\r' and val != '\n': # This makes sense, right?
                self.received_buffer += val # append characters
            val = self.recvChar()
        if self.received_buffer != "":
            print("FLUSHING: " + str(self.received_buffer))
        self.received_buffer = ""

    # Checks the connection by sending a command with known response
    def checkConnection(self):
        if not self.isConnected: return -1
        checkStr = self.BT_ADDRESS.replace(":", "-") # Test string is the BT address without ':' character
        self.send("Init:" + checkStr + "\n")
        
        time.sleep(0.2)
        self.flushRecvBuffer()
        
        line = -1
        while line == -1 or len(line) < 1:
            line = self.recvLine()
        if line == "OK:"+checkStr:
            return 0
        else:
            return -1

    # Reads angle from compass
    # Retuns:
    #  >=0 : Measured angle
    #  <0  : Error
    def getMagSensorAngleNoWait(self):
        if not self.isConnected: return -1
        self.flushRecvBuffer()
        self.send("MagSensorAngle\n")
        time.sleep(0.1)
        line = self.recvLine()
        if line == -1 or len(line) < 1:
            return -1
        print("Received: "+line)
        vals = line.split(":")
        if len(vals) != 2:
            print("Error when reading angle")
            return -1
        return float(vals[1])
    
    # Reads angle from compass
    # NOTE: This function is blocking, will wait for a good measure
    def getMagSensorAngle(self):
        angle = self.getMagSensorAngleNoWait()
        while angle < 0:
            angle = self.getMagSensorAngleNoWait()
            print("NOTE: Re-reading angle measurement")
            time.sleep(0.3)
        return angle
    
    # Send string to the robot
    def send(self,line):
        if not self.isConnected: return
        self.flushRecvBuffer()
        print("SENDING: " + str(line))
        self.BT_socket.send(line)
    
    
    # Read character from the input buffer
    # Returns:
    #  Character read
    #  Empty string
    def recvChar(self):
        if not self.isConnected: return ""
        try:
            data = self.BT_socket.recv(1)
        except:
            return ""
        return data
    
    
    # Reads a line from the input, ignores every '\r' and removes the '\n'
    def recvLine(self):
        if not self.isConnected: return -1
        gotLine = False
        val = self.recvChar()
        while len(val) > 0:
            if val == '\n':
                gotLine = True
                break
            if val != '\r':
                self.received_buffer += val # append characters
            val = self.recvChar()
        if gotLine:
            line = self.received_buffer
            self.received_buffer = "" # Reset the input buffer
            #print("RECEIVED LINE: " + str(line))
            return line
        else:
            return -1
    

