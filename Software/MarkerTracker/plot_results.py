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



for fileName in ["yawLog5","yawLog15","yawLog25"]:
    dataLog = loadFromFile("",fileName+"_withPos.p")

    videoTimestamp = np.array(dataLog['videoTimestamp'])
    pos = np.array(dataLog['pos'])
    # center starting point in 0,0
    pos[:,0] -= pos[0,0]
    pos[:,1] -= pos[0,1]
    
    yaw = np.array(dataLog['yaw'])
    yaw -= yaw[0]
    yawTimestamp = np.array(dataLog['timestamp'])
    yawTimestamp -= 0.5



    ax[0].plot(pos[:,0],pos[:,1])
    ax[1].plot(yawTimestamp,yaw)


    ax[0].set_ylim([-200,200])
    ax[0].set_xlim([0,1450])
    #ax[0].axis('equal')
    ax[0].set_ylabel('Position in Y [arb]', fontsize=16)
    ax[0].set_xlabel('Position in X [arb]', fontsize=16)
    ax[1].set_ylabel('$yaw(t)$ [degrees]', fontsize=16)
    ax[1].set_xlabel('Time [s]', fontsize=16)
    ax[0].set_title('Observed trajectory for distinct speeds (recorded with OpenCV)', fontsize=16)
    
    ax[1].set_title('Deviation from straight line, as measured with the on-board gyroscope', fontsize=16)

    #ax[1].set_ylim([10,25])
    ax[1].set_xlim([0.5,4])

#ax[0].set_xlim([0,4])

ax[0].legend(["$speed=5 [arb]$","$speed=15 [arb]$","$speed=25 [arb]$"])

tight_layout()
#show()
savefig("gnbot_linear_drift.pdf")

