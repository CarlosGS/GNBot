#!/usr/bin/python

# Begin modules
import sys
import time
import bluetooth #pybluez!
# End modules

class GNBot(object):
    def __init__(self, bt_address):
        self.BT_ADDRESS = bt_address
        self.received_buffer = ""
        self.isConnected = False


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


    def disconnect(self):
        #print("Closing connection...")
        if self.isConnected:
            self.BT_socket.close()



    def flushRecvBuffer(self): # We could also use flushInput(), but showing the data that is being discarded is useful for debugging
        if not self.isConnected: return
        val = self.recvChar()
        while len(val) > 0:
            if val != '\r' and val != '\n':
                self.received_buffer += val # append characters
            val = self.recvChar()
        if self.received_buffer != "":
            print("FLUSHING: " + str(self.received_buffer))
        self.received_buffer = ""


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


    def getMagSensorAngle(self):
        if not self.isConnected: return -1
        time.sleep(0.3)
        self.flushRecvBuffer()
        self.send("MagSensorAngle\n")
        
        time.sleep(0.1)
        self.flushRecvBuffer()
        
        line = -1
        while line == -1 or len(line) < 1:
            line = self.recvLine()
        print("Received: "+line)
        vals = line.split(":")
        if len(vals) != 2:
            print("Error when reading angle")
            return -1
        return float(vals[1])
        


    def send(self,line):
        if not self.isConnected: return
        #print("SENDING: " + str(line))
        self.BT_socket.send(line)


    def recvChar(self):
        if not self.isConnected: return ""
        try:
            data = self.BT_socket.recv(1)
        except:
            return ""
        return data


    def recvLine(self): # Reads a line from the input, ignores every '\r' and removes the '\n'
        if not self.isConnected: return ""
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


