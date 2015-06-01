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



fileName = "yawLog5"
dataLog = loadFromFile("",fileName+"_withPos.p")

videoTimestamp = np.array(dataLog['videoTimestamp'])
pos = np.array(dataLog['pos'])
yaw = np.array(dataLog['yaw'])
yawTimestamp = np.array(dataLog['timestamp'])


f, ax = subplots(2, figsize=(10,8))
ax[0].plot(pos[:,0],pos[:,1])
ax[1].plot(yawTimestamp,yaw)


ax[0].set_ylim([-520,-270])
ax[0].set_xlim([50,1600])

ax[0].set_ylabel('Actual trajectory [arb]', fontsize=16)
ax[1].set_ylabel('Angle from IMU [degrees]', fontsize=16)
ax[1].set_xlabel('Time (s)', fontsize=16)
ax[0].set_title('Deviation of a linear robot path measured with IMU', fontsize=16)

ax[1].set_ylim([10,25])

#ax[0].set_xlim([0,4])

tight_layout()
#show()
savefig(fileName+".png")

