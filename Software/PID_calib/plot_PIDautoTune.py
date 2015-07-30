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


from PID_autoTune_data1 import *
from PID_autoTune_data2 import *




pid_autoTune1 = np.array(pid_autoTune1)
pid_autoTune2 = np.array(pid_autoTune2)

# Initial timestamp is T=0
pid_autoTune1[:,0] -= pid_autoTune1[0,0]
pid_autoTune2[:,0] -= pid_autoTune2[0,0]

f, ax = subplots(1,figsize=(10,5))
ax.plot(pid_autoTune1[:,0],pid_autoTune1[:,1:])



f, ax = subplots(1,figsize=(10,5))
ax.plot(pid_autoTune2[:,0],pid_autoTune2[:,1:])

#ax.plot(pid_autoTune2)

#ax.set_title('Analysis of gyroscope drift (with static robot)', fontsize=18,y=1.01)
#ax.set_xlabel('Elapsed time since gyroscope initialisation [seconds]', fontsize=16)
#ax.set_ylabel('Yaw drift rate [deg/min]', fontsize=16)

tight_layout()

#savefig("gyro_drift.pdf")
#savefig("gyro_drift.png")

show()
