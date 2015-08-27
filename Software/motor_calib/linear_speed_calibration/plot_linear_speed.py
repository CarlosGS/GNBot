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



linear_speed_log = linear_speed_log2

f, ax = subplots(1,figsize=(9,6))


ax.plot([0,0.5*10],[0,5.11*10],'--g',linewidth=0.5)


ax.plot([0.5,0.5],[0,20],'--k',linewidth=1)
ax.text(0.52, 13.5, '$\omega_c=0.5 rad/s$', fontsize=14)

ax.arrow(0.5, 3, 0.3, 0, head_width=0.3, head_length=0.03, fc='k', ec='k')
ax.text(0.52, 3.25, 'Nonlinear region', fontsize=14)

ax.plot(linear_speed_log[:,0],linear_speed_log[:,2],'b',linewidth=2,label='Forward')
ax.plot(linear_speed_log[:,0],abs(linear_speed_log[:,1]),'r',linewidth=2,label='Backward')






#ax.grid(True)

ax.legend(loc='upper left')

ax.set_title('Effect of friction over the linear velocity (with PID yaw control)', fontsize=18,y=1.01)
ax.set_xlabel('Motor input rotational velocity $\omega_c$ [rad/s]', fontsize=16)
ax.set_ylabel('Velocity towards wall [cm/s]', fontsize=16)
ax.set_xlim([0,1.5])
ax.set_ylim([0,15.5])
tight_layout()

savefig("linear_velocity_calibration.pdf")
savefig("linear_velocity_calibration.png")

#show()

