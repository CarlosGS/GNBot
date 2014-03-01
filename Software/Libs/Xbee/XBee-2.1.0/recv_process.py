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

PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600

# Open serial port
ser = serial.Serial(PORT, BAUD_RATE)

def mapf(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

# check http://www.lebsanft.org/?p=48
ydata = [1] * 50
plt.ion()
ax1=plt.axes()
#ax1.set_yscale('log')
line, = plt.plot(ydata)
plt.ylim([0,400000])

def message_received(data):
    global ydata,line,plt
    #pprint(data)
    #print("\n")
    #print(data['rf_data'])
    try:
        words = data['rf_data'].split()
        heater_val = int(words[1])
        battery_val = int(words[5])
        nose_val = float(words[3])
    except:
        return
    VheatMax = 5.
    Vheat = mapf(heater_val,0,255, 0.,VheatMax)
    
    
    VbattMax = 21.1765
    Vbatt = mapf(battery_val,0,1023, 0.,VbattMax)
    
    
    Vc = 5.
    Rl = 10000.
    if nose_val <= 0: nose_val = 0.4
    Vout = mapf(nose_val,0,1023, 0.,Vc)
    Rs = Rl*(Vc-Vout)/Vout
    print("Vbatt = "+str(Vbatt)+"V  Vheat = "+str(Vheat)+"V  Vout = "+str(Vout)+"V (Rs = "+str(Rs)+" Ohm)")
    ydata.append(Rs)
    del ydata[0]

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        #ymin = float(min(ydata))-10
        #ymax = float(max(ydata))+10
        #plt.ylim([ymin,ymax])
        line.set_xdata(range(len(ydata)))
        line.set_ydata(ydata)  # update the data
        plt.draw() # update the plot
        time.sleep(.10)
    except KeyboardInterrupt:
        break

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

