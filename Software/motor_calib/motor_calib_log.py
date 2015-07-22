#!/usr/bin/python

import time
from GNBot.GNBot import GNBot
from helper import *

import numpy as np

import os


gb = None

gnbot_addresses = []

data = {}
data['avgSpeed'] = []
data['avgRmotorInput'] = []

def gnbot_received_callback(address, received_data):
    global gb, gnbot_addresses, data
    if not address in gnbot_addresses:
        print("NEW GNBOT ADDED! Address: " + repr(address))
        gnbot_addresses.append(address)
    try:
        print received_data
        if len(received_data.keys()) > 0:
            avgSpeed = mapVals(received_data['avgSpeed'], 0.,65535., -50.,50.)
            avgRmotorInput = received_data['avgRmotorInput']
            data['avgSpeed'].append(avgSpeed)
            data['avgRmotorInput'].append(avgRmotorInput)
    except:
        print("Error while processing incoming packet")




gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

while len(gnbot_addresses) == 0:
    time.sleep(0.5)

while True:
    if raw_input() == "ok":
        break
    print("Current data:")
    print(data)

print(data)
saveToFile(data,"","motorCalibLog_R_.p")

os._exit(0)

