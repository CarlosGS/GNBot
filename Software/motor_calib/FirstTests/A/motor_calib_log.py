#!/usr/bin/python

import time
from GNBot.GNBot import GNBot
from helper import *

import numpy as np

import os


gb = None

gnbot_addresses = []

avgSpeed = -1
avgLmotorInput = -1
avgRmotorInput = -1

def gnbot_received_callback(address, received_data):
    global gb, gnbot_addresses, avgSpeed, avgLmotorInput, avgRmotorInput
    if not address in gnbot_addresses:
        print("NEW GNBOT ADDED! Address: " + repr(address))
        gnbot_addresses.append(address)
        #values = gb.createValue("sampletime", 300)
        #gb.sendPUTcommand(address, values)
    #else:
    #    print("Received packet from: " + repr(address))
    #print(received_data)
    try:
        avgSpeed = mapVals(received_data['avgSpeed'], 0.,65535., -100.,100.)
        avgLmotorInput = received_data['avgLmotorInput']
        avgRmotorInput = received_data['avgRmotorInput']
    except:
        print("error! :D")




gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

while len(gnbot_addresses) == 0:
    time.sleep(0.5)


data = {}
data['avgSpeed'] = []
data['avgLmotorInput'] = []
data['avgRmotorInput'] = []

for i in range(4):
    print("Sample "+str(i)+". Press enter.")
    while raw_input() != "ok":
        print avgSpeed, avgLmotorInput, avgRmotorInput
    data['avgSpeed'].append(avgSpeed)
    data['avgLmotorInput'].append(avgLmotorInput)
    data['avgRmotorInput'].append(avgRmotorInput)

print(data)
saveToFile(data,"","motorCalibLog.p")

os._exit(0)

