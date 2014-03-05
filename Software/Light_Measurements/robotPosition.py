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

import struct


WORLD_SIZE = (600.,300.)
LANDMARK_POS = (-90.,-90.)

robotPosition = (30.,100.)
robotAngle = radians(45.)


plt.ion() # Real time plotting
fig_world = plt.figure(figsize=(15.0, 9.0))
ax_world = fig_world.add_subplot( 111 )

def redrawRobotPosition():
    global ax_world, robotPosition, robotAngle, robot_world
    robotSize = 15./2.
    x = robotPosition[0]
    y = robotPosition[1]
    dx = robotSize*cos(robotAngle)
    dy = robotSize*-sin(robotAngle)
    if robot_world:
        robot_world.remove()
        robot_world = None
    if not robot_world:
        robot_world = ax_world.arrow(x,y,dx,dy, shape='full', lw=1, length_includes_head=True, width=robotSize/2., head_width=robotSize)
    else:
        robot_world.set_x(x)
        robot_world.set_y(y)
        robot_world.set_dx(dx)
        robot_world.set_dy(dy)
        


landmark_world, = ax_world.plot(LANDMARK_POS[0],LANDMARK_POS[1], 'yo', markersize=50)
robot_world = None
redrawRobotPosition()

plt.xlim([LANDMARK_POS[0],WORLD_SIZE[0]])
plt.ylim([LANDMARK_POS[1],WORLD_SIZE[1]])
ax_world.set_aspect('equal')
plt.tight_layout()
plt.show()

# check http://www.lebsanft.org/?p=48
WINDOW_LEN = 20*60*1000/100
ydata1 = [0] * WINDOW_LEN
ydata2 = [0] * WINDOW_LEN

plt.ion()
fig = plt.figure(figsize=(15.0, 9.0))

ax1 = fig.add_subplot( 211 )
#ax1.set_yscale('log')
line1, = plt.plot(ydata1, linewidth=0.0001)
line1.set_antialiased(False)
plt.ylim([-180,180])

ax2 = fig.add_subplot( 212 )
line2, = plt.plot(ydata2, linewidth=0.0001)
line2.set_antialiased(False)
plt.ylim([0,25])

plt.tight_layout()
plt.show()




updated = 0

PORT = '/dev/ttyUSB1'
BAUD_RATE = 9600

# Open serial port
ser = serial.Serial(PORT, BAUD_RATE)

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
def message_received(data):
    global updated, robotAngle, robot_dest_addr_long
    #print(data)
    if not ('source_addr_long' in data.keys()) or not ('rf_data' in data.keys()): return
    if not robot_dest_addr_long:
        robot_dest_addr_long = data['source_addr_long']
        print("Robot address: ")
        pprint(robot_dest_addr_long)
    if 'rf_data' in data.keys():
        rf_data = data['rf_data']
        #print(rf_data)
        
        data_vals = rf_data.split()
        angle = readCompassAngle(data_vals[1],data_vals[2])
        ydata1.append(angle)
        del ydata1[0]
        
        robotAngle = radians(angle)
        redrawRobotPosition()
        
        spinSpeed = 10
        forwardSpeed = 0
        dstAngle = 90*cos(time.time()/5.)
        
        angle -= dstAngle
        
        if abs(angle) < 0.5:
            spinSpeed = 0
        elif abs(angle) < 3:
            spinSpeed = 2
        elif abs(angle) < 5:
            spinSpeed = 3
        elif abs(angle) < 20:
            spinSpeed = 4
        
        if angle < 0:
            robotSetMotors(forwardSpeed+spinSpeed,forwardSpeed-spinSpeed)
        else:
            robotSetMotors(forwardSpeed-spinSpeed,forwardSpeed+spinSpeed)
        
        batt_ADC = data_vals[0]
        batt_ADC = float(batt_ADC[1:])
        battV = mapVals(batt_ADC,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
        ydata2.append(battV)
        del ydata2[0]
        updated = 0

def robotSetMotors(L,R):
    if not robot_dest_addr_long: return
    rf_data = struct.pack('b', L) + struct.pack('b', R)
    zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=robot_dest_addr_long, data=rf_data)

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        if not updated:
            updated = 1
            #ymin = float(min(ydata))-1
            #ymax = float(max(ydata))+1
            #plt.ylim([ymin,ymax])
            line1.set_xdata(range(WINDOW_LEN))
            line1.set_ydata(ydata1)  # update the data
            
            #fftdata = np.abs(np.fft.rfft(ydata))[1:]
            
            #ymin = float(min(fftdata))-1
            #ymax = float(max(fftdata))+1
            #ax2.set_ylim([ymin,ymax])
            line2.set_xdata(range(WINDOW_LEN))
            line2.set_ydata(ydata2)
            
            fig.canvas.draw() # redraw the plot
            fig_world.canvas.draw()
        else: time.sleep(0.01)
    except KeyboardInterrupt:
        break

robotSetMotors(0,0)

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

