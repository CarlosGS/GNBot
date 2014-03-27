#! /usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
from xbee import ZigBee
import time
import serial
from pprint import pprint
from math import *
import datetime

from matplotlib import pyplot as plt
import numpy as np

from helper import *

import atexit
import struct
# End modules


PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600

# Open serial port
ser = serial.Serial(PORT, BAUD_RATE)

def robotSetMotors_single(L,R):
    if not robot_dest_addr_long: return
    rf_data = struct.pack('b', L) + struct.pack('b', R)
    zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=robot_dest_addr_long, data=rf_data)

def robotSetMotors(L,R):
    robotSetMotors_single(L,R)
    time.sleep(0.1)
    robotSetMotors_single(L,R)
    time.sleep(0.1)
    robotSetMotors_single(L,R)

robot_dest_addr_long = None
robot_battery = 999
turn_dir = 1
backing_up = False
waiting = 0
def message_received(datain):
    global robotAngle, robot_dest_addr_long, robot_battery, current_logging, data, robot_speed, turn_dir, backing_up, waiting
    if not ('source_addr_long' in datain.keys()) or not ('rf_data' in datain.keys()): return
    if not robot_dest_addr_long:
        robot_dest_addr_long = datain['source_addr_long']
        print("Robot address: ")
        pprint(robot_dest_addr_long)
        robotSetMotors(0,0)
    rf_data = datain['rf_data']
    #print(rf_data)
    
    data_vals = rf_data.split()
    
    batt_ADC = data_vals[0]
    batt_ADC = float(batt_ADC[1:])
    robot_battery = mapVals(batt_ADC,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
    
    IRdist_ADC = float(data_vals[-1])
    robot_IRdist = IRdist_ADC # here we should convert the range [0-1024] to centimeters with the proper regression
    
    if current_logging:
        data['time'].append(datetime.datetime.now())
        data['battery'].append(robot_battery)
        data['IRdist'].append(robot_IRdist)
        
        motorSpeedL = robot_speed
        motorSpeedR = robot_speed
        
        variance = 9999
        if len(data['IRdist']) > 60:
            variance = np.std(data['IRdist'][-50:-1])
        if robot_IRdist > 634: or variance < 1.8:
            backing_up = True
            waiting = 10
        if not backing_up:
            if robot_IRdist > 410: # object closer than 20cm (see datasheet)
                offset = mapVals(robot_IRdist,410.,490., 0.,robot_speed)
                if offset > robot_speed: offset = robot_speed
                #motorSpeedL += offset
                motorSpeedR -= offset
            else:
                turn_dir *= -1
            if turn_dir == 1:
                motorSpeedL_ = motorSpeedL
                motorSpeedL = motorSpeedR
                motorSpeedR = motorSpeedL_
        else:
            waiting -= 1
            motorSpeedL = -10
            motorSpeedR = -10
            if waiting <= 0:
                backing_up = False
        
        robotSetMotors_single(motorSpeedL,motorSpeedR)
        
        data['speedL'].append(motorSpeedL)
        data['speedR'].append(motorSpeedR)
        
        #print("Batt: "+str(robot_battery))
        print(robot_battery,robot_IRdist,variance)


data = {}
current_logging = False
current_dist = 0

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)


# Callback to close the connection when force-quitting
def exitCallback():
    print("Closing connection...")
    robotSetMotors(0,0)
    # halt() must be called before closing the serial
    # port in order to ensure proper thread shutdown
    zb.halt()
    ser.close()

atexit.register(exitCallback)


time.sleep(1)


robot_speed = 5


print("Let's measure the real speed of the robot. Mark the current position.")

print("Position the robot in a clear area and press enter! Battery: "+str(round(robot_battery,2))+"V")
print("Test speed: "+str(robot_speed))
val = sys.stdin.readline()

motionTime = 10.
print("Moving forward "+str(motionTime)+" seconds")

robotSetMotors(robot_speed,robot_speed)
time.sleep(motionTime)
robotSetMotors(0,0)

distance = getValue("Traveled distance? [cm]",50.)

data['realSpeed'] = 0.01*distance/motionTime #m/s

print("Speed: "+str(data['realSpeed'])+" m/s")

data['maxSpeed'] = robot_speed
data['battery'] = []
data['IRdist'] = []
data['speedL'] = []
data['speedR'] = []
data['time'] = []

print("\nPosition the robot and press enter! Battery: "+str(round(robot_battery,2))+"V")
val = sys.stdin.readline()

current_logging = True

while 1:
    if len(data['time']) > 0 and datetime.datetime.now()-data['time'][-1] > datetime.timedelta(seconds = 10):
        print("Robot not responding!")
        break
    try:
        time.sleep(0.05)
    except KeyboardInterrupt:
        break

current_logging = False

time.sleep(0.3)
robotSetMotors(0,0)

saveToFile(data,"./","logged_data_battery"+str(robot_speed)+".p")

#pprint(data)

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

os._exit(0)

