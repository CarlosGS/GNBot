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


from gyro_drift_data_1 import *
from gyro_drift_data_2 import *
from gyro_drift_data_3 import *
from gyro_drift_data_4 import *


f, ax = subplots(1,figsize=(10,5))

drift1 = np.array(drift1)
drift2 = np.array(drift2)
drift3 = np.array(drift3)#
drift4 = np.array(drift4)

time = drift3[:,0]
radians = drift3[:,1]

from scipy.interpolate import interp1d

f = interp1d(time, radians, bounds_error=False)

new_time = np.arange(100000)+6000
new_radians = f(new_time)
new_radians_diff = np.diff(new_radians)*1000

new_time -= 6000



alpha = 0.0005
first = True
for i in xrange(len(new_radians_diff)):
    if first:
        first = False
    else:
        if abs(new_radians_diff[i]-new_radians_diff[i-1]) < 0.01:
            new_radians_diff[i] = new_radians_diff[i-1]*(1-alpha) + new_radians_diff[i]*alpha


new_radians_diff *= 60


ax.plot(new_time[1:]/1000.,new_radians_diff)


value = "%.3f" % new_radians_diff[-1]
ax.annotate(value+" rad/min", xy=(99, new_radians_diff[-1]-0.05), xytext=(70, -0.3),arrowprops=dict(arrowstyle="->"))

ax.set_xlim([0,100])


ax.set_title('Analysis of gyroscope drift (with static robot)', fontsize=18,y=1.01)
ax.set_xlabel('Elapsed time since gyroscope initialisation [seconds]', fontsize=16)
ax.set_ylabel('Yaw drift rate [rad/min]', fontsize=16)

tight_layout()

savefig("gyro_drift.pdf")
savefig("gyro_drift.png")

#show()
