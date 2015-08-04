#!/usr/bin/python
# encoding: utf-8

from helper import *

from pylab import *

import numpy as np


# https://klassenresearch.orbs.com/Plotting+with+Python
from matplotlib import rc
# Make use of TeX﻿
rc('text',usetex=True)
# Change all fonts to 'Computer Modern'
rc('font',**{'family':'serif','serif':['Computer Modern']})

f, ax = subplots(1)

data_files = ['distanceLog.p','distanceLog_robot1.p', 'distanceLog_robot2.p', 'distanceLog_robot4.p']

for filename in data_files:
    data = loadFromFile("",filename)
    print(data)

    distances = np.array(data['distances'])
    measurementMin = np.array(data['measurementMin'])
    measurementMax = np.array(data['measurementMax'])

    measurementMin = mapVals(measurementMin,0.,1023.,0.,5.)
    measurementMax = mapVals(measurementMax,0.,1023.,0.,5.)
    measurementAvg = (measurementMin+measurementMax)/2

    #ax.plot(distances,measurementMax)
    ax.plot(distances,measurementAvg)
    #ax.plot(distances,measurementMin)



x1 = 8.
y1 = 2.763

x2 = 29.
y2 = 0.917

K = x1*(y1-y2) / (1.-x1/x2)
C = y2-K/x2
fit = K*(1./distances)+C
print("Voltage fitting:")
print("K="+str(K))
print("C="+str(C))

print("ADC fitting:")
K = mapVals(K,0.,5.,0.,1023.)
C = mapVals(C,0.,5.,0.,1023.)
print("K="+str(K))
print("C="+str(C))

ax.plot(distances,fit,linewidth=2)

ax.set_ylim([0,3.5])
ax.set_xlim([0,100])

ax.legend(['Original','Robot 1', 'Robot 2', 'Robot 4','Fit'])

savefig("IR_sensor_response_curve.pdf")

show()
exit()

# Fitting to exponential curve

#x1 = 9.
#y1 = float(measurementAvg[6])

x1 = 12.
y1 = float(measurementAvg[7])

#x2 = 21.
#y2 = float(measurementAvg[9])

x2 = 37.
y2 = float(measurementAvg[11])

#x2 = 86.
#y2 = float(measurementAvg[14])

#x2 = 114.
#y2 = float(measurementAvg[15])




#D = x1
#A = y1
#B = np.e
#C = 0.1

print x1, y1, x2, y2

K = x1*(y1-y2) / (1.-x1/x2)
C = y2-K/x2
fit = K*(1./distances)+C
print("K="+str(K))
print("C="+str(C))

#K = 14.6358265503
#C = -0.173084289376

#a = (np.log(K)-np.log(C+y3))/np.log(x3)
#K = (x1**a)*(y1-y2) / (1.-(x1**a)/(x2**a))
#C = y2-K/(x2**a)
#fit = K*(1./(distances**a))+C
#print a, K, C

#ax.plot(distances,fit)


ax.plot(K/(measurementAvg-C),measurementAvg,"o")





ax.set_ylim([0,3.5])
ax.set_xlim([0,200])

ax.set_ylabel('Sensor output [V]', fontsize=16)
ax.set_xlabel('Actual distance [cm]', fontsize=16)

ax.set_title('IR range-finder response curve', fontsize=16)


ax.legend(["max","avg","min","fit"])

tight_layout()




f, ax = subplots(1)

ax.plot(distances,K/(measurementAvg-C),"r")
ax.plot(distances,distances,"b")

#savefig("IR_sensor_response_curve.pdf")
show()


