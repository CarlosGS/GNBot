#! /usr/bin/python

"""
receive_samples_async.py

By Paul Malmsten, 2010
pmalmsten@gmail.com

This example reads the serial port and asynchronously processes IO data
received from a remote XBee.
"""

from xbee import ZigBee
import time
import serial
from pprint import pprint

PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600

# Open serial port
ser = serial.Serial(PORT, BAUD_RATE)

def message_received(data):
    pprint(data)
    print("\n")
    #print(data['rf_data'])

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        time.sleep(.1)
    except KeyboardInterrupt:
        break

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

