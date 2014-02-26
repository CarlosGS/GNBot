#! /usr/bin/python

from xbee import ZigBee
import time
import serial
from pprint import pprint

PORT = '/dev/ttyUSB1'
BAUD_RATE = 9600

ser = serial.Serial(PORT, BAUD_RATE)

def message_received(data):
    pprint(data)

zb = ZigBee(ser, escaped = True, callback=message_received)

while True:
    try:
        time.sleep(1)
        #zb.send("tx", dest_addr='.\x0c', dest_addr_long='\x00\x13\xa2\x00@\x9c.\x0c', data='Hello world from root!')
        zb.send("tx", dest_addr='\x00\x00', dest_addr_long='\x00\x00\x00\x00\x00\x00\x00\x00', data='Hello world from dest!')
        time.sleep(5)
    except KeyboardInterrupt:
        break

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

