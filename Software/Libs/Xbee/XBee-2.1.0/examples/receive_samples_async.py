#! /usr/bin/python

"""
receive_samples_async.py

By Paul Malmsten, 2010
pmalmsten@gmail.com

This example reads the serial port and asynchronously processes IO data
received from a remote XBee.
"""

from xbee import XBee
import time
import serial

PORT = '/dev/ttyACM1'
BAUD_RATE = 9600

# Open serial port
#ser = serial.Serial(PORT, BAUD_RATE)

ser = serial.Serial()
ser.port = PORT
ser.baudrate = BAUD_RATE
ser.timeout = 1
ser.writeTimeout = 1
ser.open()

def message_received(data):
    print data

# Create API object, which spawns a new thread
xbee = XBee(ser, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        time.sleep(.1)
    except KeyboardInterrupt:
        break

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
xbee.halt()
ser.close()
