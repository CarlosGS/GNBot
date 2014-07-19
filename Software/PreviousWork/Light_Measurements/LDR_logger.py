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

magnetometerCalibration = {'xmax': -620.0, 'xmin': -1153.0, 'ymax': 391.0, 'ymin': -152.0} # Manually calibrated
CALIBRATE_MAGNETOMETER = False
def readCompassAngle(x,y):
    global magnetometerCalibration
    compassX = float(x)
    compassY = float(y)
    if CALIBRATE_MAGNETOMETER:
        if magnetometerCalibration['xmin'] > compassX: magnetometerCalibration['xmin'] = compassX
        if magnetometerCalibration['xmax'] < compassX: magnetometerCalibration['xmax'] = compassX
        if magnetometerCalibration['ymin'] > compassY: magnetometerCalibration['ymin'] = compassY
        if magnetometerCalibration['ymax'] < compassY: magnetometerCalibration['ymax'] = compassY
        pprint(magnetometerCalibration)
    compassX = mapVals(compassX, magnetometerCalibration['xmin'],magnetometerCalibration['xmax'], -1.,1.)
    compassY = mapVals(compassY, magnetometerCalibration['ymin'],magnetometerCalibration['ymax'], -1.,1.)
    angle = degrees(atan2(compassY,compassX))
    return angle

robot_dest_addr_long = None
robot_battery = 999
robot_angle = 999
def message_received(datain):
    global robotAngle, robot_dest_addr_long, robot_battery, robot_angle, current_logging, current_dist, current_halfTurns, data
    #pprint(datain)
    if not ('source_addr_long' in datain.keys()) or not ('rf_data' in datain.keys()): return
    if not robot_dest_addr_long:
        robot_dest_addr_long = datain['source_addr_long']
        print("Robot address: ")
        pprint(robot_dest_addr_long)
        robotSetMotors(0,0)
    rf_data = datain['rf_data']
    #print(rf_data)
    
    data_vals = rf_data.split()
    robot_angle = readCompassAngle(data_vals[1],data_vals[2])
    correctedAngle = (robot_angle-current_initialAngle)%360.
    #print(correctedAngle)
    
    batt_ADC = data_vals[0]
    batt_ADC = float(batt_ADC[1:])
    robot_battery = mapVals(batt_ADC,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
    
    if current_logging:
        robot_LDR_vals = {}
        for sensor_i in range(4):
           data_vals_i = sensor_i + 4 # items 4,5,6,7
           robot_LDR_vals[sensor_i] = float(data_vals[data_vals_i])
           data[current_dist]['vals'][sensor_i].append(robot_LDR_vals[sensor_i])
           sensorAngle = (LDR_real_pos[sensor_i]+correctedAngle)%360.
           data[current_dist]['angles'][sensor_i].append(sensorAngle)
    
        print("Batt: "+str(robot_battery)+" Angle: "+str(correctedAngle)+" LDRs: "+str(robot_LDR_vals))
        
        if abs(correctedAngle-180) < 10:
            if (current_halfTurns % 2) == 0:
                current_halfTurns += 1
                print(current_halfTurns)
        elif abs(correctedAngle) < 10:
            if (current_halfTurns % 2) == 1:
                current_halfTurns += 1
                print(current_halfTurns)


offset = 45
LDR_real_pos = []
LDR_real_pos.append(offset+90*2)
LDR_real_pos.append(offset+90*3)
LDR_real_pos.append(offset+90*0)
LDR_real_pos.append(offset+90*1)




data = {}
current_logging = False
current_dist = 0
current_halfTurns = 0
current_initialAngle = 0

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)


# Callback to close the connection when force-quitting
def exitCallback():
    print("Closing connection...")
    robotSetMotors(0,0)
    # halt() must be called before closing the serial
    # port in order to ensure proper thread shutdown
    #zb.halt()
    #ser.close()

atexit.register(exitCallback)


DIST_NEAR = 100
DIST_MIDDLE = 200
DIST_FAR = 500
DistancesList = [DIST_NEAR, DIST_MIDDLE, DIST_FAR]
for dist in DistancesList:
    print("Position the robot at D=" + str(dist) + "cm, pointing to the lamp, and press enter! Battery: "+str(round(robot_battery,2))+"V")
    val = sys.stdin.readline()
    data[dist] = {}
    data[dist]['vals'] = {}
    data[dist]['angles'] = {}
    
    for sensor_i in range(4):
        data[dist]['vals'][sensor_i] = []
        data[dist]['angles'][sensor_i] = []
    
    # Point robot to the origin
    correct = 0
    while not correct:
        angle = 0.
        window = 10
        for i in range(window): # average angle reading
            print(robot_angle)
            angle += robot_angle
            time.sleep(0.1)
        angle /= float(window)
        print("Initial angle:" + str(angle))
        correct = getValue("Is it reasonable? {1,0}",1)
    
    current_initialAngle = angle
    data[dist]['iniAngle'] = angle
    
    spinSpeed = 5
    current_dist = dist
    current_halfTurns = 0
    current_logging = True
    
    robotSetMotors(spinSpeed,-spinSpeed)
    while current_halfTurns < 2:
        time.sleep(0.05)
    current_logging = False
    robotSetMotors(0,0)
    print("Done!!")

robotSetMotors(0,0)

saveToFile(data,"./","logged_data_LDR.p")

pprint(data)

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

exit()

