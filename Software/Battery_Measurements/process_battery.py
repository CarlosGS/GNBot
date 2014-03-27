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
axTop.plot(estimated_distance, battery_log, color='green', marker=None, ls='-', lw=0.5, markersize=1)
axTop.set_title( 'Performance' )
axTop.set_ylabel( 'Battery level [V]' )
axTop.set_xlabel( 'Estimated distance [m]' )
plt.tight_layout()
plt.show()
exit()

#for datapoint in data:
#    print(str(datapoint["timestamp"]) +" -> "+ str(datapoint["all"]))
#exit()
times_hr = []
hr = []
times_T = []
hr_period = []
hrv = []
otherPlot = []
#timeT = None
prevTime = None
for datapoint in data:
    vals = datapoint["all"]
    if len(vals) >= 4:
        time = datapoint["timestamp"]
        hr.append(vals[1])
        times_hr.append(datapoint["timestamp"])
        #if not timeT: timeT = times_hr[0]
        if not prevTime: prevTime = times_hr[0]
        timeT = time
        total = 0
        hr_period_vals = []
        times_T_vals = []
        for i in reversed(range((len(vals)-2)/2)):
            hr_period_measure = vals[2+i*2:]
            LSO = hr_period_measure[0]
            MSO = hr_period_measure[1]
            hr_period_base2 = MSO*256+LSO
            hr_period_ms = (hr_period_base2*1024.)/1000.
            hr_bpm = 60./(hr_period_ms/1000.)
            #if hr_bpm > 140: continue
            hr_period_vals.append(hr_period_ms)
            hrv.append(60000./hr_period_ms)
            times_T_vals.append(timeT)
            timeT -= timedelta(milliseconds = hr_period_ms)
            total += hr_period_ms
        hr_period.extend(reversed(hr_period_vals))
        times_T.extend(reversed(times_T_vals))
        incT = (datapoint["timestamp"]-prevTime).total_seconds()
        otherPlot.append(incT*10)
#        if incT < 0.5:
#            print("")
#            print(incT)
#            print(vals)
#            for i in range((len(vals)-2)/2):
#                hr_period_measure = vals[2+i*2:]
#                LSO = hr_period_measure[0]
#                MSO = hr_period_measure[1]
#                hr_period_base2 = MSO*256+LSO
#                hr_period_ms = (hr_period_base2*1024.)/1000.
#                print(" " + str(hr_period_ms))
        prevTime = datapoint["timestamp"]
        
        #print(str(datapoint["timestamp"]) +" -> "+ str(vals))

if len(times_hr) == 0:
	exit()

#hr_period_fft = np.fft.rfft(hr_period)
#hr_period_fft[int(len(hr_period)/50):] = 0.
#hr_period = np.fft.irfft(hr_period_fft,len(hr_period))
#hrv = np.diff(hr_period).tolist()
#hrv.insert(0,0)


print("\nEnd of data!")
print("Total datapoints: " + str(len(times_hr)))
print("Start:" + str(times_hr[0]))
print("End:" + str(times_hr[-1]))
nightLength = times_hr[-1]-times_hr[0]
print("Lenght:" + str(nightLength))
nightLengthHours = int(math.ceil(nightLength.seconds/(60.*60.)))

print("\nPlotting...")

fig = plt.figure(figsize=(40.0, 7.0))
#fig = plt.figure(figsize=(nightLengthHours*30.0/9, 7.0))
axTop = fig.add_subplot( 211 )
axTop.grid( True )
axTop.plot_date( x=times_T, y=hr_period, color='green', marker=None, ls='-', lw=0.5, markersize=1)
#axTop.plot_date( x=times_hr, y=hrv, color='cyan', marker='', ls='-', lw=0.5)
axTop.set_title( 'Heart rate over time (' + str(times_hr[0].date()) + ')' )
axTop.set_ylabel( 'RR interval [ms]' )

axBtm = fig.add_subplot( 212 )
axBtm.grid( True )
axBtm.plot_date( x=times_hr, y=hr, color='blue', marker=None, ls='-', lw=0.5)
axBtm.plot_date( x=times_hr, y=hr, color='red', marker='.', markersize=1)
#axBtm.plot_date( x=times_hr, y=otherPlot, color='yellow', marker='', ls='-', lw=0.5)
#axBtm.plot_date( x=times_T, y=hrv, color='cyan', marker='', ls='-', lw=0.5)
axBtm.set_xlabel( 'Time' )
axBtm.set_ylabel( 'Heart rate [BPM]' )

fig.autofmt_xdate()

beginDate = times_hr[0].replace(minute = 0, second = 0, microsecond = 0)
xlabels = []
for h in range(nightLengthHours+1):
	for q in range (6*2):
		m = q*5
		val = beginDate + timedelta(seconds = (h*60+m)*60)
		xlabels.append(val)
		#print val
xlabels.append(times_hr[0])
xlabels.append(times_hr[-1])
axTop.axvline(x=times_hr[0]+timedelta(seconds = (5-4)*60*60), visible=True, color='r')
axBtm.axvline(x=times_hr[0]+timedelta(seconds = (5-4)*60*60), visible=True, color='r')

axTop.xaxis.set_ticks(xlabels)
axBtm.xaxis.set_ticks(xlabels)
#axTop.set_ylim([40, 130])
axTop.set_xlim([times_hr[0], times_hr[-1]])
axBtm.set_xlim([times_hr[0], times_hr[-1]])
plt.tight_layout()
if len(sys.argv) > 2:
	plt.show()
else:
	plt.savefig("HR log " + str(times_hr[0]) + " Length: " + str(nightLength) + ".png")
print("#########################################################################################################################################################")
print("\nDone!")

