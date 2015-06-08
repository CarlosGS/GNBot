#!/usr/bin/python

import time
from GNBot.GNBot import GNBot
from helper import *

import numpy as np

import os


gb = None

gnbot_addresses = []

minMeasurement = -1
maxMeasurement = -1

def gnbot_received_callback(address, received_data):
    global gb, gnbot_addresses, minMeasurement, maxMeasurement
    if not address in gnbot_addresses:
        print("NEW GNBOT ADDED! Address: " + repr(address))
        gnbot_addresses.append(address)
        values = gb.createValue("sampletime", 300)
        gb.sendPUTcommand(address, values)
    #else:
    #    print("Received packet from: " + repr(address))
    #print(received_data)
    minMeasurement = received_data['distMin']
    maxMeasurement = received_data['distMax']




gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

while len(gnbot_addresses) == 0:
    time.sleep(0.5)




#print("Waiting for packets...")

distances = np.logspace(0,2.3,20).astype(int)
distances = np.unique(distances)

data = {}
data['distances'] = []
data['measurementMin'] = []
data['measurementMax'] = []

for distance in distances:
    print("Place robot at distance = "+str(distance)+", then press enter")
    raw_input()
    data['distances'].append(distance)
    data['measurementMin'].append(minMeasurement)
    data['measurementMax'].append(maxMeasurement)
    print distance, minMeasurement, maxMeasurement

print(data)
saveToFile(data,"","distanceLog.p")

os._exit(0)

