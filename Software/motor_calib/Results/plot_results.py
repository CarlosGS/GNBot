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

ax.set_title('Motor response curve', fontsize=18)

tight_layout()

#savefig("motors_speed_response_curve.pdf")





f, ax = subplots(1)

speeds_fw = []
pulses_fw = []
speeds_bw = []
pulses_bw = []
for i in xrange(len(L_speed_rads)):
    if L_speed_rads[i] > 0.03:
        speeds_fw.append(L_speed_rads[i])
        pulses_fw.append(L_pulse_ms[i])
    if L_speed_rads[i] < -0.03:
        speeds_bw.append(L_speed_rads[i])
        pulses_bw.append(L_pulse_ms[i])

speeds_fw = np.array(speeds_fw)
speeds_bw = np.array(speeds_bw)

from scipy.interpolate import interp1d
f = interp1d(R_speed_rads, R_pulse_ms)
R_pulse_ms_linear_fw = f(-speeds_fw)
R_pulse_ms_linear_bw = f(-speeds_bw)


ax.plot(pulses_fw,R_pulse_ms_linear_fw,'ob',ms=4)
ax.plot(pulses_bw,R_pulse_ms_linear_bw,'oy',ms=4)

ax.set_xlabel('Left motor input pulse width [ms]', fontsize=16)
ax.set_ylabel('Right motor input pulse width [ms]', fontsize=16)

ax.legend(['Forwards motion','Backwards motion'])

ax.set_aspect('equal')
ax.set_xlim([1000,2000])

ax.set_title('Motor input values that correspond to a linear trajectory', fontsize=18)


tight_layout()

#savefig("linear_trajectory_PWM_mapping.pdf")



show()


