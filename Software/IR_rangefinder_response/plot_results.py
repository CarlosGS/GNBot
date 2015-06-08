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

data = loadFromFile("","distanceLog.p")



distances = np.array(data['distances'])
measurementMin = np.array(data['measurementMin'])
measurementMax = np.array(data['measurementMax'])


measurementMin = mapVals(measurementMin,0.,1023.,0.,5.)
measurementMax = mapVals(measurementMax,0.,1023.,0.,5.)

ax.plot(distances,measurementMax)
ax.plot(distances,(measurementMin+measurementMax)/2)
ax.plot(distances,measurementMin)




ax.set_ylim([0,3.5])
ax.set_xlim([0,200])

ax.set_ylabel('Sensor output [V]', fontsize=16)
ax.set_xlabel('Actual distance [cm]', fontsize=16)

ax.set_title('IR range-finder response curve', fontsize=16)


ax.legend(["max","avg","min"])

tight_layout()
savefig("IR_sensor_response_curve.pdf")
show()


