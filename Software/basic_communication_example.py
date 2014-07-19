#!/usr/bin/python

import time
from GNBot.GNBot import GNBot

gnbot_addresses = []

def gnbot_received_callback(address, received_data):
	global gb, gnbot_addresses
	if not address in gnbot_addresses:
		print("NEW GNBOT ADDED! Address: " + repr(address))
		gnbot_addresses.append(address)
	else:
		print("Received packet from: " + repr(address))
	print(received_data)

gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

print("Waiting for packets...")

while 1:
	for address in gnbot_addresses:
		values = gb.createValue("motorL", 10)
		values += gb.createValue("motorR", 10)
		values += gb.createValue("delay", 2000)
		values += gb.createValue("motorL", 0)
		values += gb.createValue("motorR", 0)
		gb.sendPUTcommand(address, values)
	time.sleep(5)

