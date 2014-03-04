#! /usr/bin/python
from xbee import ZigBee
import time
import serial
from pprint import pprint

from matplotlib import pyplot as plt
import numpy as np

from helper import *






# check http://www.lebsanft.org/?p=48
WINDOW_LEN = 300
ydata1 = [0] * WINDOW_LEN
ydata2 = [0] * WINDOW_LEN

plt.ion()
fig = plt.figure(figsize=(15.0, 8.0))

ax1 = fig.add_subplot( 211 )
#ax1.set_yscale('log')
line1, = plt.plot(ydata1, linewidth=0.0001)
line1.set_antialiased(False)
plt.ylim([0,360])

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

def message_received(data):
    global updated
    if 'rf_data' in data.keys():
        rf_data = data['rf_data']
        print(rf_data)
        
        data_vals = rf_data.split()
        
        compass = data_vals[1]
        compass = float(compass[1:])
        ydata1.append(compass)
        del ydata1[0]
        
        batt_ADC = data_vals[0]
        batt_ADC = float(batt_ADC[1:])
        battV = mapVals(batt_ADC,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
        ydata2.append(battV)
        del ydata2[0]
        updated = 0

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
            
            plt.draw() # update the plot
        else: time.sleep(0.01)
    except KeyboardInterrupt:
        break

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

