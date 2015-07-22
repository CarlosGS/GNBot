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

data = loadFromFile("","motorCalibLog.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']

ax.plot(L_pulse_ms,L_speed_rads,'-r')
ax.plot(R_pulse_ms,R_speed_rads,'-b')

ax.set_xlabel('Motor input pulse width [ms]', fontsize=16)
ax.set_ylabel('Measured rotational speed [rad/s]', fontsize=16)

ax.legend(['L','R'])

ax.set_title('Motor response curve', fontsize=16)

tight_layout()

savefig("motors_speed_response_curve.pdf")
show()


