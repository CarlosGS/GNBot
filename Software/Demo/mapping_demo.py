#!/usr/bin/python

import time
from GNBot.GNBot import GNBot
import numpy as np
import os
from helper import *
from pylab import *

gnbot_addresses = []


robot_X = 0
robot_Y = 0
robot_angle = 0

scan_data = []
done = False

time_last = time.time()

def gnbot_received_callback(address, received_data):
    global gb, gnbot_addresses, scan_data, done, time_last
    if not address in gnbot_addresses:
        print("NEW GNBOT ADDED! Address: " + repr(address))
        gnbot_addresses.append(address)
    try:
        IRdistanceCM = mapVals(received_data['IRdistanceCM'], 0.,65535., 0.,150.)
        IMUyaw = mapVals(received_data['IMUyaw'], 0.,65535., -np.pi,np.pi)
        print IMUyaw, IRdistanceCM
        scan_data.append((IMUyaw, IRdistanceCM))
        time_last = time.time()
    except:
        if len(received_data.keys()) > 0:
            print(received_data)
    if len(scan_data) > 0:
        if time.time()-time_last > 2:
            done = True

gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

print("Waiting for packets...")



distance_increment = 20

multi_scans = []

while 1:
    fail = False
    direction = raw_input()
    angle = 0
    if direction == 'l':
        angle = -90
    elif direction == 'r':
        angle = 90
    elif direction == 'f':
        angle = 0
    elif direction == 'b':
        angle = 180
    elif direction == 'q':
        break
    else:
        fail = True
    
    if not fail:
        values = gb.createValue("targetAngle", angle)
        values += gb.createValue("targetDistance", distance_increment)
        gb.sendPUTcommand(gnbot_addresses[0], values)
        
        
        scan_data = []
        time.sleep(0.1)
        done = False
        
        robot_angle -= angle
        robot_X += np.cos(robot_angle*np.pi/180.)*distance_increment
        robot_Y += np.sin(robot_angle*np.pi/180.)*distance_increment
        
        while not done:
            time.sleep(0.5)
        
        obstacles = []
        for (IMUyaw, IRdistanceCM) in scan_data:
            if IRdistanceCM > 8 and IRdistanceCM < 40:
                IRdistanceCM += 4.5 # Remove offset of the sensor within the robot
                obsX = robot_X + np.cos(-IMUyaw)*IRdistanceCM
                obsY = robot_Y + np.sin(-IMUyaw)*IRdistanceCM
                obstacles.append([obsX,obsY])
        
        scan_data = []
        
        multi_scans.append(np.array(obstacles))
        
        figure()
        plot(robot_X, robot_Y, 'gs', ms=15)
        for obs in multi_scans:
            plot(obs[:,0], obs[:,1], 'o', ms=3)
        for pos in multi_scans[-1]:
            plot([robot_X,pos[0]], [robot_Y,pos[1]], 'g', lw=0.5)
        xlabel('Position in X [cm]', fontsize=16)
        ylabel('Position in Y [cm]', fontsize=16)
        axis('equal')
        show()

os._exit(0)

