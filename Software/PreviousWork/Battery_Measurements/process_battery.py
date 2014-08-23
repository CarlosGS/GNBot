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

from dateutil.parser import *
from dateutil.tz import *
from datetime import datetime, timedelta

from helper import *
# End modules

p_file = sys.argv[1]
print("Processing file: " + p_file )

data = loadFromFile("",p_file)

realSpeed = data['realSpeed']*0.6
battery_log = data['battery']
timestamp_log = data['time']


print("Real speed: "+str(realSpeed))

battery_log_fft = np.fft.rfft(battery_log)
 # Store the first coefficients of the FFT, this acts as a low-pass filter
battery_log_fft = battery_log_fft[:len(battery_log)/50]
battery_log = np.fft.irfft(battery_log_fft,len(battery_log))

timestamp_log_seconds = [(ts-timestamp_log[0]).total_seconds() for ts in timestamp_log]

estimated_distance = [ts_seconds*realSpeed for ts_seconds in timestamp_log_seconds]

fig = plt.figure()
#fig = plt.figure(figsize=(nightLengthHours*30.0/9, 7.0))
axTop = fig.add_subplot( 111 )
axTop.grid( True )
axTop.plot(timestamp_log_seconds, battery_log, color='green', marker=None, ls='-', lw=0.5, markersize=1)
axTop.set_title( 'Performance' )
axTop.set_ylabel( 'Battery level [V]' )
axTop.set_xlabel( 'Estimated distance [m]' )
plt.tight_layout()
plt.show()

