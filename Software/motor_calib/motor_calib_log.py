#!/usr/bin/python

import time
from GNBot.GNBot import GNBot
from helper import *

import numpy as np

import os


gb = None

gnbot_addresses = []

data = {}
data['avgSpeed_L'] = []
data['avgLmotorInput'] = []
data['avgSpeed_R'] = []
data['avgRmotorInput'] = []

def gnbot_received_callback(address, received_data):
    global gb, gnbot_addresses, data
    if not address in gnbot_addresses:
        print("NEW GNBOT ADDED! Address: " + repr(address))
        gnbot_addresses.append(address)
    try:
        print received_data
        if len(received_data.keys()) > 0:
            if 'avgRmotorInput' in received_data.keys():
                avgSpeed = mapVals(received_data['avgSpeed'], 0.,65535., -10.,10.)
                avgRmotorInput = received_data['avgRmotorInput']
                data['avgSpeed_R'].append(avgSpeed)
                data['avgRmotorInput'].append(avgRmotorInput)
            else:
                avgSpeed = mapVals(received_data['avgSpeed'], 0.,65535., -10.,10.)
                avgLmotorInput = received_data['avgLmotorInput']
                data['avgSpeed_L'].append(avgSpeed)
                data['avgLmotorInput'].append(avgLmotorInput)
    except:
        print("Error while processing incoming packet")




gb = GNBot(gnbot_received_callback, '/dev/ttyUSB1', 9600)

while len(gnbot_addresses) == 0:
    time.sleep(0.5)

while True:
    if raw_input() == "ok":
        break
    print("Current data:")
    print(data)

print(data)
saveToFile(data,"","motorCalibLog.p")

os._exit(0)

