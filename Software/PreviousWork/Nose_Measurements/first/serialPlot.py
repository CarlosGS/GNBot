#! /usr/bin/python
import serial
import time
from pprint import pprint

from matplotlib import pyplot as plt

import numpy as np

PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

ser = serial.Serial(PORT, BAUD_RATE, timeout=10)

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

# check http://www.lebsanft.org/?p=48
ydata = [1] * 1000

fftlen = len(ydata)/2;
fftdata = [0] * fftlen
plt.ion()
fig = plt.figure(figsize=(15.0, 8.0))

ax2 = fig.add_subplot( 212 )
lineFFT, = plt.plot(fftdata, linewidth=0.0001)
lineFFT.set_antialiased(False)
plt.ylim([0,50000])

ax1 = fig.add_subplot( 211 )
ax1.set_yscale('log')
line, = plt.plot(ydata, linewidth=0.0001)
line.set_antialiased(False)
plt.ylim([100,1000000000])

plt.tight_layout()
plt.show()

firstRun = 1

newValsCount = 0
while True:
    while ser.inWaiting() > 0:
        val_txt = ser.readline()
        try:
            val = float(val_txt)
        except:
            continue
        ydata.append(toResistance(val))
        del ydata[0]
        
        if firstRun:
            ydata = [ydata[-1]] * len(ydata)
            firstRun = 0
        newValsCount += 1
    
    if newValsCount > 0:
        #ymin = float(min(ydata))-1
        #ymax = float(max(ydata))+1
        #plt.ylim([ymin,ymax])
        line.set_xdata(range(len(ydata)))
        line.set_ydata(ydata)  # update the data
        
        fftdata = np.abs(np.fft.rfft(ydata))[1:]
        
        ymin = float(min(fftdata))-1
        ymax = float(max(fftdata))+1
        ax2.set_ylim([ymin,ymax])
        lineFFT.set_xdata(range(len(fftdata)))
        lineFFT.set_ydata(fftdata)
        
        plt.draw() # update the plot
        newValsCount = 0
    while ser.inWaiting() == 0: time.sleep(0.01)

ser.close()

