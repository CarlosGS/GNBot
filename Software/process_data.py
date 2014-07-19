#!/usr/bin/python
# encoding: utf-8

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import sys
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import math

import numpy as np
import scipy.signal as signal

from dateutil.parser import *
from dateutil.tz import *
from datetime import datetime, timedelta

from helper import *
# End modules





def mapf(x, in_min, in_max, out_min, out_max):
    try:
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
    except:
        return out_min

def toResistance(v_adc):
    Vc = 5.
    Rl = 10000.
    if v_adc <= 0: v_adc = 0.000001
    Vout = mapf(v_adc,0,1023, 0.,Vc)
    Rs = Rl*(Vc-Vout)/Vout
    return Rs







# https://klassenresearch.orbs.com/Plotting+with+Python
from matplotlib import rc
# Make use of TeX﻿
rc('text',usetex=True)
# Change all fonts to 'Computer Modern'
rc('font',**{'family':'serif','serif':['Computer Modern']})

fig1 = plt.figure(figsize=(8.0, 8.0))
ax1 = fig1.add_subplot( 211 )
ax1.grid( True )

p_file = sys.argv[1]
print("Processing file: " + p_file )

data = loadFromFile("",p_file)

data_log = data['minNoseResistance'] # [toResistance(val) for val in data['noseValMax']]
timestamp_log = data['time']

# http://oceanpython.org/2013/03/11/signal-filtering-butterworth-filter/
# First, design the Buterworth filter
N  = 2    # Filter order
Wn = 0.03 # Cutoff frequency
B, A = signal.butter(N, Wn, output='ba')
# Second, apply the filter
data_log_filtered = signal.filtfilt(B,A, data_log)

timestamp_log_seconds = [(ts-timestamp_log[0]).total_seconds() for ts in timestamp_log]

timestamp_log_minutes = [ts_seconds/60. for ts_seconds in timestamp_log_seconds]

#noseValMin_filt = signal.filtfilt(B,A, data['noseValMin'])
#noseValMax_filt = signal.filtfilt(B,A, data['noseValMax'])


#noseResistance_Min = [toResistance(val) for val in noseValMin_filt]
#noseResistance_Max = [toResistance(val) for val in noseValMax_filt]

####ax1.plot(timestamp_log_minutes, noseResistance_Min, color='r', lw=1)
####ax1.plot(timestamp_log_minutes, noseResistance_Max, color='b', lw=1)

#ax1.plot(timestamp_log_minutes, data['noseValMin'], color='r', lw=1)
#ax1.plot(timestamp_log_minutes, data['noseValMax'], color='b', lw=1)
#ax1.plot(timestamp_log_minutes, [(data['noseValMin'][i]+data['noseValMax'][i])/2 for i in range(len(data['noseValMax']))], color='g', lw=1)
#ax1.plot(timestamp_log_minutes, [(noseResistance_Min[i]+noseResistance_Max[i])/2 for i in range(len(noseResistance_Min))], color='g', lw=1)
ax1.plot(timestamp_log_minutes, data_log, color='r', lw=1)
####ax1.plot(timestamp_log_minutes, data_log_filtered, color='k', lw=1.5)

ax1.set_ylim([0,60000])

#ax1.set_ylim([10.5,18.5])
#ax1.set_xlim([0,180])
ax1.set_title('Nose measurement', fontsize=16)
ax1.set_ylabel('Level [Ohms]', fontsize=15)
ax1.set_xlabel('Time [Minutes]', fontsize=15)
#ax1.legend(prop={'size':14},loc='center right')




robotPosX = data['robotPosX']
robotPosY = [-val-0 for val in data['robotPosY']]

odorPosY = robotPosY[-1]
odorPosX = robotPosX[-1]


distanceToSource = np.zeros(len(robotPosX))
for i in range(len(robotPosX)):
    distanceToSource[i] = np.sqrt((robotPosX[i]-odorPosX)**2+(robotPosY[i]-odorPosY)**2)

ax1 = fig1.add_subplot( 212 )
ax1.grid( True )
ax1.plot(timestamp_log_minutes, distanceToSource, color='b', lw=1)
#ax1.plot(timestamp_log_minutes, data['temperature'], color='b', lw=1)
#ax1.plot(timestamp_log_minutes, data['humidity'], color='b', lw=1)

#ax1.plot(timestamp_log_minutes, data['battery'], color='b', lw=1)
#ax1.plot(timestamp_log_minutes, data['IRdistMin'], color='b', lw=1)

ax1.set_title('Distance to source', fontsize=16)
ax1.set_ylabel('Distance to source [cm]', fontsize=15)
ax1.set_xlabel('Time [Minutes]', fontsize=15)


plt.rcParams['ytick.major.pad'] = 10
plt.rcParams['ytick.minor.pad'] = 10
plt.rcParams['xtick.major.pad'] = 10
plt.rcParams['xtick.minor.pad'] = 10

plt.tight_layout()
#plt.savefig("plot.eps")




fig2 = plt.figure(figsize=(8.0, 8.0))
ax2 = fig2.add_subplot( 111 )
ax2.grid( True )


ax2.plot(robotPosX, robotPosY, color='r', lw=1)
ax2.plot(odorPosX, odorPosY, 'yo', markersize=10)
ax2.plot(robotPosX[0], robotPosY[0], 'go', markersize=10)
ax2.set_aspect('equal')
#ax2.set_ylim([-300,300])
#ax2.set_xlim([-150,150])
#ax2.set_ylim([-115,115])
#ax2.set_xlim([-20,20])
ax2.set_ylabel('Y [cm]', fontsize=15)
ax2.set_xlabel('X [cm]', fontsize=15)
ax2.set_title('Trajectory', fontsize=16)
#ax2.xaxis.set_ticks([-10,0,10])
plt.tight_layout()
#plt.savefig("trajectory.png")

fig3 = plt.figure(figsize=(8.0, 8.0))
ax3 = fig3.add_subplot( 111 )
ax3.plot(data_log_filtered,distanceToSource, 'bx')
ax3.grid( True )



plt.show()

