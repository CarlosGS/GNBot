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


from linear_speed_log1 import *
linear_speed_log1 = np.array(linear_speed_log1)

from linear_speed_log2 import *
linear_speed_log2 = np.array(linear_speed_log2)

from linear_speed_log3 import *
linear_speed_log3 = np.array(linear_speed_log3)

from linear_speed_log4 import *
linear_speed_log4 = np.array(linear_speed_log4)



linear_speed_log = linear_speed_log4

f, ax = subplots(1,figsize=(10,5))
ax.plot(linear_speed_log[:,0],linear_speed_log[:,2],'b')
ax.plot(linear_speed_log[:,0],abs(linear_speed_log[:,1]),'r')

ax.legend(['FW', 'BW'])

ax.set_title('Calibration of the linear velocity. Asymmetric FW/BW friction can be observed', fontsize=18,y=1.01)
ax.set_xlabel('Motor input $\omega$ [rad/s]', fontsize=16)
ax.set_ylabel('Velocity [cm/s]', fontsize=16)
#ax.set_xlim([0,11])
#ax.set_ylim([-65,65])
tight_layout()

savefig("linear_velocity_calibration.pdf")
savefig("linear_velocity_calibration.png")

show()

