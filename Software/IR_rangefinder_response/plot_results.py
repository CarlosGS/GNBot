#!/usr/bin/python
# encoding: utf-8

from helper import *

from pylab import *

import numpy as np

from matplotlib import gridspec


# https://klassenresearch.orbs.com/Plotting+with+Python
from matplotlib import rc
# Make use of TeX﻿
rc('text',usetex=True)
# Change all fonts to 'Computer Modern'
rc('font',**{'family':'serif','serif':['Computer Modern']})

#f, ax = subplots(1,2,figsize=(11,5),width_ratios=[2, 1])

f = plt.figure(figsize=(11, 5)) 


suptitle('IR rangefinder response curve (II)', fontsize=18)


ax = []
gs = gridspec.GridSpec(1, 2, width_ratios=[2, 1]) 
ax.append(plt.subplot(gs[0]))
ax.append(plt.subplot(gs[1]))

data_files = ['distanceLog.p', 'distanceLog_robot1.p', 'distanceLog_robot2.p', 'distanceLog_robot4.p']

sensordata = []
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
    #ax.plot(distances,measurementAvg,'-o',linewidth=0.5)
    sensordata.append(measurementAvg)
    #ax.plot(distances,measurementMin)

#ax.plot(distances,measurementAvg[0],'-o',linewidth=0.5)
ax[0].plot(distances,sensordata[1],'-ob',linewidth=0.5)
ax[0].plot(distances,sensordata[2],'-og',linewidth=0.5)
ax[0].plot(distances,sensordata[3],'-or',linewidth=0.5)

x1 = 10.
y1 = 2.32

x2 = 40.
y2 = 0.7

Kv = x1*(y1-y2) / (1.-x1/x2)
Cv = y2-Kv/x2
fit = Kv*(1./distances)+Cv
print("Voltage fitting:")
print("Kv="+str(Kv))
print("Cv="+str(Cv))




print("ADC fitting:")
K = mapVals(Kv,0.,5.,0.,1023.)
C = mapVals(Cv,0.,5.,0.,1023.)
print("K="+str(K))
print("C="+str(C))



ax[0].set_title('Exponential curve fitting', fontsize=14)

ax[0].plot(distances,fit,'-c',linewidth=2)

ax[0].set_ylabel('Sensor output [V]', fontsize=16)
ax[0].set_xlabel('Actual distance $d$ from sensor to wall [cm]', fontsize=16)




ax[0].set_ylim([0,3.5])
ax[0].set_xlim([0,100])



ax[0].plot([16,50],[Kv*(1./16)+Cv,1.8],'-c',linewidth=0.5)
ax[0].text(50.5, 1.7, '$V(d)='+str(round(Kv,2))+' \\frac{\\displaystyle 1}{\\displaystyle d}+'+str(round(Cv,2))+'$', fontsize=14)

ax[0].plot([10,10],[0,3.5],'k--',linewidth=1.5)
ax[0].plot([40,40],[0,3.5],'k--',linewidth=1.5)


ax[0].arrow(10+2*2.5, 3.1, -2.5, 0, head_width=0.05, head_length=2.5, fc='k', ec='k')
ax[0].arrow(15, 3.1, 25-2.5, 0, head_width=0.05, head_length=2.5, fc='k', ec='k')

ax[0].text(15, 2.9, 'Linear fit region', fontsize=12)



ax[0].legend(['Sensor 1 (avgd.)', 'Sensor 2 (avgd.)', 'Sensor 3 (avgd.)','Fitted curve'])

tight_layout()



ax[1].set_title('Linearity evaluation', fontsize=14)

ax[1].plot(distances,Kv/(sensordata[1]-Cv),'-ob',linewidth=0.5)
ax[1].plot(distances,Kv/(sensordata[2]-Cv),'-og',linewidth=0.5)
ax[1].plot(distances,Kv/(sensordata[3]-Cv),'-or',linewidth=0.5)
ax[1].plot([0,50],[0,50],"c",linewidth=2)


#ax[1].plot([0,10],[50,10],'k--',linewidth=1.5)
#ax[1].plot([0,40],[50,40],'k--',linewidth=1.5)
#ax[1].plot([0,10],[50,10],'k--',linewidth=1.5)
#ax[1].plot([0,10],[50,10],'k--',linewidth=1.5)


ax[1].plot(10,10,'+k',markersize=20,markeredgewidth=1.5)
ax[1].plot(40,40,'+k',markersize=20,markeredgewidth=1.5)

ax[1].text(5, 35, '$d(V)=\\frac{\\displaystyle '+str(round(Kv,2))+'}{\\displaystyle V-'+str(round(Cv,2))+'}$', fontsize=14)


ax[1].set_ylabel('$d$ [cm]', fontsize=16)
ax[1].set_xlabel('$d$ [cm]', fontsize=16)

#ax[1].legend(['Sensor 1 (avgd.)', 'Sensor 2 (avgd.)', 'Sensor 3 (avgd.)','Fitted curve'])


ax[1].set_ylim([0,50])
ax[1].set_xlim([0,50])


tight_layout()
plt.subplots_adjust(top=0.875)

savefig("IR_sensor_response_curve.pdf")
savefig("IR_sensor_response_curve.png")


