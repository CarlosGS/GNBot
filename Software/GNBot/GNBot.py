#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# License: CC BY-SA 4.0 (Attribution-ShareAlike 4.0 International, http://creativecommons.org/licenses/by-sa/4.0/)

# Begin modules
from xbee import ZigBee
import serial
import struct
from pprint import pprint
# End modules

class GNBot(object):
	valueTypes = {	0	: "reserved",
					1	: "ledR_PWM",
					2	: "ledG_PWM",
					3	: "ledB_PWM",
					4	: "motorL",
					5	: "motorR",
					6	: "tone",
					7	: "noseHeater_PWM",
					8	: "noseMax",# to be read
					9	: "noseMin",# to be read
					10	: "distMax",# to be read
					11	: "distMin",# to be read
					12	: "batteryMax",# to be read
					13	: "batteryMin",# to be read
					14	: "notone",
					15	: "delay",
					16	: "toneMs",
					17	: "humidity",# to be read
					18	: "temperature",# to be read
					19	: "magnetometerX",# to be read
					20	: "magnetometerY",# to be read
					21	: "magnetometerZ",# to be read
					22	: "button",# to be read
					23	: "sampletime"}
	
	valueTypesLookup = {}
	
	gnbot_addresses = []
	
	# Constructor
	def __init__(self, callback_function, port='/dev/ttyUSB0', baud_rate=9600):
		self._callback = callback_function
		self._port = port
		self._baud_rate = baud_rate
		
		# Create reverse lookup table for value types
		print("\nGNBot value types:")
		print("CODE\t: VALUE")
		print("-----------------")
		for key in self.valueTypes.keys():
			value = self.valueTypes[key]
			self.valueTypesLookup[value] = key
			print(str(key)+"\t: "+str(value))
		print("")
		# Open serial port
		self._ser = serial.Serial(self._port, self._baud_rate)
		
		self._zb = ZigBee(self._ser, escaped = True, callback=self._processCallback)
	
	# Destructor
	def halt(self):
		self._zb.halt()
		self._ser.close()
	
	def createValue(self, valueTypeStr, val):
		if val < 0: val += 2**16
		char1 = chr(val >> 8)
		char2 = chr(val & 0x00ff)
		valTypeID = chr(self.valueTypesLookup[valueTypeStr])
		return valTypeID+char1+char2
	
	def sendPUTcommand(self, address, values):
		rf_data = chr(0) # PUT command
		rf_data += values# [struct.pack('b', v) for v in values]
		self._zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=address, data=rf_data)
	
	# Callback processing function
	def _processCallback(self, datain):
		if not ('source_addr_long' in datain.keys()) or not ('rf_data' in datain.keys()): return
		data = datain['rf_data']
		packetType = ord(data[0])
		address = datain['source_addr_long']
		
		
		if packetType == 0: # PUT
			#print("Received: PUT")
			raw_values_string = data[1:]
			
			N_values = int(len(raw_values_string)/3)
			values = {}
			for i in range(N_values):
				istart = i*3
				valType = ord(raw_values_string[istart])
				#value = ord(raw_values_string[istart+1])*256+ord(raw_values_string[istart+2])
				#value = int(ord(raw_values_string[istart+1])<<8|ord(raw_values_string[istart+2]),16)
				value = struct.unpack('h', raw_values_string[istart+2]+raw_values_string[istart+1])[0]
				if not valType in self.valueTypes.keys():
					print("WARNING! Unknown value type received: " + str(valType))
					continue
				#print("Value: "+str(value)+"\t: ("+str(self.valueTypes[valType])+")")
				values[self.valueTypes[valType]] = value
			
			self._callback(address, values)
			
		elif packetType == 1: # GET
			print("Received: GET")
			sample_period = ord(data[1])*10 # ms
			valTypes_requested = [ord(t) for t in data[2:]]
			print("IGNORING")
			
		else:
			print("WARNING! Unknown packet type received: " + str(packetType))
			print("IGNORING")
		
		
	

