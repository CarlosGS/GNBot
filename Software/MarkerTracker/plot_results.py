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

f, ax = subplots(2, figsize=(10,8))

#ax[0].plot([0,22],[0,0],'--k')
#ax[1].plot([0,3.5],[0,0],'--k')


real_length = 60.

labels = ["$input = 5 [arb]$","$input = 15 [arb]$","$input = 25 [arb]$"]
i = 0
for fileName in ["yawLog5","yawLog15","yawLog25"]:
    dataLog = loadFromFile("",fileName+"_withPos.p")

    videoTimestamp = np.array(dataLog['videoTimestamp'])
    pos = np.array(dataLog['pos'])
    # center starting point in 0,0
    pos[:,0] -= pos[0,0]
    pos[:,1] -= pos[0,1]
    
    pos[:,0] *= real_length/1425.
    pos[:,1] *= real_length/1425.
    
    prevX = 0
    prevY = 0
    for j in xrange(len(pos[:,0])):
        pos[j,0] = pos[j,0]*0.3 + prevX*0.7
        pos[j,1] = pos[j,1]*0.3 + prevY*0.7
        prevX = pos[j,0]
        prevY = pos[j,1]
    
    yaw = np.array(dataLog['yaw'])
    yaw -= yaw[0]
    yaw -= arange(len(yaw))*0.06
    yawTimestamp = np.array(dataLog['timestamp'])
    yawTimestamp -= 1



    ax[0].plot(pos[:,0],pos[:,1],linewidth=2,label=labels[i])
    ax[1].plot(yawTimestamp,yaw,linewidth=2)
    
    i += 1


ax[0].set_ylim([-10,10])
ax[0].axis('equal')
ax[0].set_xlim([-10,real_length+5])
ax[0].set_ylabel('Position in Y [cm]', fontsize=16)
ax[0].set_xlabel('Position in X [cm]', fontsize=16)
ax[1].set_ylabel('Yaw [deg]', fontsize=16)
ax[1].set_xlabel('Time $t$ [s]', fontsize=16)
ax[0].set_title('Observed trajectory for distinct linear velocities', fontsize=18)

ax[1].set_title('Deviation from a straight path (yaw), measured with the on-board gyroscope', fontsize=16)

ax[1].set_ylim([-5,8.5])
ax[1].set_xlim([0,3])

ax[0].legend()

ax[0].grid(True)
ax[1].grid(True)



def drawRobot(ax,wheel_type):
    dimX = 9.
    dimY = 8.
    # Chassis
    ax.add_patch(Rectangle((-dimX/2.,-dimY/2.),dimX,dimY,linewidth=1.5, fc='w', ec='k'))
    # Wheels
    Dr = 6.5
    Dl = 6.5
    if wheel_type == "larger left":
        Dl += 1
    elif wheel_type == "larger right":
        Dr += 1
    ax.add_patch(Rectangle((-dimX/2.,dimY/2.),Dr,0.6,linewidth=0, fc='k', ec='k'))
    ax.add_patch(Rectangle((-dimX/2.,-dimY/2.-0.6),Dl,0.6,linewidth=0, fc='k', ec='k'))
    
    ax.arrow(-dimX/3., 0, dimX/2., 0, head_width=dimY/2., head_length=dimX/4.,linewidth=3, fc='g', ec='c')

drawRobot(ax[0], "same")




#ax[0].set_xlim([0,4])

#ax[0].legend(["$speed=5 [arb]$","$speed=15 [arb]$","$speed=25 [arb]$"])

tight_layout()

savefig("gnbot_linear_drift.pdf")
savefig("gnbot_linear_drift.png")

#show()

