#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# License: CC BY-SA 4.0 (Attribution-ShareAlike 4.0 International, http://creativecommons.org/licenses/by-sa/4.0/)


# Begin modules
from GNBot.GNBot import GNBot
from helper import *
import time
import datetime
from pprint import pprint
import atexit
import os

import matplotlib.pyplot as plt
import numpy as np
import scipy.signal as signal
from math import *

import random
import threading
# End modules

gb = None

gnbot_list = {}


MAClookup = {
    '\x00\x13\xa2\x00@a@\x15'    : "Robot 1",
    "\x00\x13\xa2\x00@a@'"        : "Robot 2",
    '\x00\x13\xa2\x00@a@\x18'    : "Robot 3",
    '\x00\x13\xa2\x00@a@\n'        : "Robot 4",
    "NI5"    : "Robot 5",
}

def lookupRobotName(address):
    if address in MAClookup.keys():
        return MAClookup[address]
    else:
        return repr(address)









def toResistance(v_adc):
    Vc = 5.
    Rl = 10000.
    if v_adc <= 0: v_adc = 0.000001
    Vout = mapVals(v_adc,0,1023, 0.,Vc)
    Rs = Rl*(Vc-Vout)/Vout
    return Rs




def robot_log_IMU(address):
    global gb, gnbot_list, do_exit
    initRobot(address)
    time.sleep(0.5)
    
    while not do_exit:
        try:
#            values = gb.createValue("motorL", rotateSpeed)
#            values += gb.createValue("motorR", -rotateSpeed)
#            values += gb.createValue("delay", int(dstAngleDelay*1000))
#            values += gb.createValue("motorL", 0)
#            values += gb.createValue("motorR", 0)
#            values += gb.createValue("sampletime", 300)
#            gb.sendPUTcommand(address, values)
#            time.sleep(dstAngleDelay)
#            time.sleep(0.5)
            
            #robotSetMotors(address, -8,-8)
            #gnbot_list[address]["record"] = True
            for i in xrange(100):
                time.sleep(100)
            
#            robotSetMotors(address, 0,0)
#            gnbot_list[address]["record"] = False
#            time.sleep(0.25)
#            gnbot_list[address]["recordedVals"] = {}
#            time.sleep(0.25)
        except:
            print("Warning: Robot main loop had some trouble")

start_time = 0
dataLog = {}
recording = False
def gnbot_received_callback(address, received_data):
    global gb, gnbot_list, do_exit, recording, dataLog, start_time
    received_data["time"] = datetime.datetime.now()
    if not address in gnbot_list.keys():
        print("NEW GNBOT ADDED! Address: " + repr(address))
        gnbot_list[address] = {}
#        t = threading.Thread(target=robot_log_IMU, args = ([address]))
#        t.daemon = True
#        t.start()
        initRobot(address)
        robotSetMotors(address, 5,5) #5, 15, 25
        start_time = time.time()
        dataLog['yaw'] = []
        dataLog['timestamp'] = []
        recording = True
    #else: print("Received packet from: " + repr(address))
    if "record" in gnbot_list[address].keys():
        if gnbot_list[address]["record"] == True: # Append values if record=true. The values are kept if record=false
            if not "recordedVals" in gnbot_list[address]:
                gnbot_list[address]["recordedVals"] = {}
            for key in received_data.keys():
                if not key in gnbot_list[address]["recordedVals"]:
                    gnbot_list[address]["recordedVals"][key] = []
                gnbot_list[address]["recordedVals"][key].append(received_data[key])
    gnbot_list[address]["vals"] = received_data
    yaw = mapVals(received_data['IMUyaw'], 0.,65535., -180.,180.)
    pitch = mapVals(received_data['IMUpitch'], 0.,65535., -180.,180.)
    roll = mapVals(received_data['IMUroll'], 0.,65535., -180.,180.)
    #print yaw,pitch,roll
    #pprint(received_data)
    #print("")
    if recording:
        dataLog['yaw'].append(yaw)
        print(yaw)
        ts = time.time()-start_time
        dataLog['timestamp'].append(ts)
        if ts > 4:
            recording = False
            do_exit = True



musicNotes = {"DO":262, "RE":294, "MI":330, "FA":349, "SOL":392, "LA":440, "SI":494}

def initRobot(address):
    noteDelay = 200
    noteScale = 1
    values = gb.createValue("motorL", 0)
    values += gb.createValue("motorR", 0)
    values += gb.createValue("ledR_PWM", 0)
    #values += gb.createValue("ledG_PWM", 128)
    values += gb.createValue("ledG_PWM", 0)
    values += gb.createValue("ledB_PWM", 0)
    values += gb.createValue("tone", noteScale*musicNotes["DO"])
    values += gb.createValue("delay", noteDelay)
    values += gb.createValue("notone", 10)
    values += gb.createValue("tone", noteScale*musicNotes["MI"])
    values += gb.createValue("delay", noteDelay)
    values += gb.createValue("notone", 10)
    values += gb.createValue("tone", noteScale*musicNotes["SOL"])
    values += gb.createValue("delay", noteDelay)
    values += gb.createValue("notone", 0)
    values += gb.createValue("sampletime", 200)
    gb.sendPUTcommand(address, values)

def endRobot(address):
    noteDelay = 300
    noteScale = 1
    values = gb.createValue("motorL", 0)
    values += gb.createValue("motorR", 0)
    values += gb.createValue("tone", noteScale*musicNotes["SOL"])
    values += gb.createValue("delay", noteDelay)
    values += gb.createValue("notone", 10)
    values += gb.createValue("tone", noteScale*musicNotes["MI"])
    values += gb.createValue("delay", noteDelay)
    values += gb.createValue("notone", 10)
    values += gb.createValue("tone", noteScale*musicNotes["DO"])
    values += gb.createValue("delay", noteDelay)
    values += gb.createValue("notone", 0)
    values += gb.createValue("sampletime", 1000)
    values += gb.createValue("ledR_PWM", 0)
    values += gb.createValue("ledG_PWM", 0)
    values += gb.createValue("ledB_PWM", 128)
    gb.sendPUTcommand(address, values)

def robotSetMotors(address, L,R):
    values = gb.createValue("motorL", L)
    values += gb.createValue("motorR", R)
    gb.sendPUTcommand(address, values)


random.seed() # Set random seed using system time

# Register callback for powering off the GNBot system
do_exit = False
def exitCallback():
    global gb, do_exit, dataLog
    do_exit = True
    if gb != None:
        time.sleep(0.5)
        for address in gnbot_list.keys():
            endRobot(address)
        gb.halt()
        gb = None
    saveToFile(dataLog,"","yawLog.p")
atexit.register(exitCallback)

gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

print("Waiting for packets...")

plt.ion()
fig = plt.figure(figsize=(8.0, 8.0))
ax = fig.add_subplot( 122 )
colors = ['r','g','b','c','y']

ax2 = fig.add_subplot( 121 )

while not do_exit:
    try:
        ax.clear() # clear the plot
        ax2.clear()
        index = 0
        for address in gnbot_list.keys():
            robotName = lookupRobotName(address)
            robot_battery_adc = (gnbot_list[address]["vals"]["batteryMax"]+gnbot_list[address]["vals"]["batteryMin"])/2
            robot_battery = mapVals(robot_battery_adc,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
            ax.bar(index,robot_battery,width=0.25,color=colors[index],label=robotName)
            minNoseResistance = toResistance(gnbot_list[address]["vals"]["noseMax"])
            ax2.bar(index,minNoseResistance,width=0.75,color=colors[index],label=robotName)
            index += 1
        ax.set_ylim([8,20])
        ax2.set_ylim([0,50000])
        ax.set_title("Battery levels")
        ax2.set_title("Nose sensor levels")
        ax.set_ylabel("Battery level [Volts]")
        ax2.set_ylabel("Nose sensor level [Ohms]")
        ax2.invert_yaxis()
        if index > 0:
            #ax.legend()
            ax2.legend()
        plt.draw() # update the plot
        time.sleep(0.1)
    except:
        do_exit = True
        break

exitCallback()

os._exit(0)


