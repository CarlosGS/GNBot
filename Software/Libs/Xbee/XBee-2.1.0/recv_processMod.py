#! /usr/bin/python

"""
receive_samples_async.py

By Paul Malmsten, 2010
pmalmsten@gmail.com

This example reads the serial port and asynchronously processes IO data
received from a remote XBee.
"""

from xbee import ZigBee
import time
import serial
from pprint import pprint

from matplotlib import pyplot as plt

PORT = '/dev/ttyUSB1'
BAUD_RATE = 9600

# Open serial port
ser = serial.Serial(PORT, BAUD_RATE)

def mapf(x, in_min, in_max, out_min, out_max):
    try:
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
    except:
        return out_min

def toResistance(v_adc):
    Vc = 5.
    Rl = 10000.
    if v_adc <= 0: v_adc = 0.4
    Vout = mapf(v_adc,0,1023, 0.,Vc)
    Rs = Rl*(Vc-Vout)/Vout
    return Rs

# check http://www.lebsanft.org/?p=48
ydata = [0] * 10
plt.ion()
ax1=plt.axes()
#ax1.set_yscale('log')
line, = plt.plot(ydata)
plt.ylim([0,400000])

firstRun = 1
def message_received(data):
    global ydata,firstRun
    #pprint(data)
    #print("\n")
    #print(data['rf_data'])
    try:
        words = data['rf_data'].split()
        nose_val = [float(word) for word in words[3:]]
    except:
        return
    if len(nose_val) <= 3:
        print(data['rf_data'])
        return
    print(nose_val)
    vals = [toResistance(val) for val in nose_val]
    top_val = vals[1]
    btm_val = vals[3]
    newVals = [mapf(val,btm_val,top_val, 0.,10.) for val in vals]
    #newVals = []
    #newVals.append(mapf(vals[2],btm_val,top_val, 0.,10.))
    #newVals.append(mapf(vals[3],btm_val,top_val, 0.,10.))
    #newVals.append(mapf(vals[4],btm_val,top_val, 0.,10.))
    #newVals.append(vals[4]-vals[3])
    #newVals = [val-vals[3] for val in vals]
    #dataToPlot.append(newVals)
    vals = newVals
    slope1 = (vals[2]-vals[1])/500
    slope2 = (vals[3]-vals[2])/1000
    slope3 = (vals[4]-vals[3])/1000
    slope4 = (vals[5]-vals[4])/1000
    slope5 = (vals[6]-vals[5])/1000
    ydata.append(slope1-slope4)
    del ydata[0]
    #for val in vals:
    #    ydata.append(val)
    #    del ydata[0]
    if firstRun:
        ydata = [ydata[-1]] * len(ydata)
        firstRun = 0

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        ymin = float(min(ydata))
        ymax = float(max(ydata))
        plt.ylim([ymin,ymax])
        line.set_xdata(range(len(ydata)))
        line.set_ydata(ydata)  # update the data
        plt.draw() # update the plot
        time.sleep(.50)
    except KeyboardInterrupt:
        break

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

