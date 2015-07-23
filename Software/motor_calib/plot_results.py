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

f, ax = subplots(1,3,figsize=(10,5))


suptitle('Velocity response curves for distinct wheel diameters', fontsize=18)

data = loadFromFile("","motorCalibLog_biggerLwheel.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']

ax[0].plot(L_pulse_ms,L_speed_rads,'-r')
ax[0].plot(R_pulse_ms,R_speed_rads,'-b')
ax[0].set_xlabel('Motor input pulse width [ms]', fontsize=14)
ax[0].set_ylabel('Robot rotational speed [rad/s]', fontsize=14)
#ax[0].legend(['L','R'])
ax[0].set_title('$D_L=75mm, D_R=65mm$', fontsize=14)
ax[0].set_ylim([-2.5,2.5])
ax[0].set_xlim([1050,1950])

tight_layout()


data = loadFromFile("","motorCalibLog_sameWheelDiam.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']

ax[1].plot(L_pulse_ms,L_speed_rads,'-r')
ax[1].plot(R_pulse_ms,R_speed_rads,'-b')
ax[1].set_xlabel('Motor input pulse width [ms]', fontsize=14)
#ax[1].set_ylabel('Robot rotational speed [rad/s]', fontsize=14)
#ax[1].legend(['L','R'])
ax[1].set_title('$D_L=D_R=65mm$', fontsize=14)
ax[1].set_ylim([-2.5,2.5])
ax[1].set_xlim([1050,1950])
ax[1].set_yticklabels([])

tight_layout()


data = loadFromFile("","motorCalibLog_biggerRwheel.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']

ax[2].plot(L_pulse_ms,L_speed_rads,'-r')
ax[2].plot(R_pulse_ms,R_speed_rads,'-b')
ax[2].set_xlabel('Motor input pulse width [ms]', fontsize=14)
#ax[2].set_ylabel('Robot rotational speed [rad/s]', fontsize=14)
ax[2].legend(['Left motor','Right motor'])
ax[2].set_title('$D_L=65mm, D_R=75mm$', fontsize=14)
ax[2].set_ylim([-2.5,2.5])
ax[2].set_xlim([1050,1950])
ax[2].set_yticklabels([])

tight_layout()
plt.subplots_adjust(top=0.875)
plt.subplots_adjust(wspace=0)
#plt.subplots_adjust(wspace=0.12)

savefig("motors_speed_response_curve.pdf")
savefig("motors_speed_response_curve.png")











from scipy.interpolate import interp1d

f, ax = subplots(1,figsize=(7.5,7))


data = loadFromFile("","motorCalibLog_sameWheelDiam.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']
speeds = []
pulses = []
for i in xrange(len(L_speed_rads)):
    if abs(L_speed_rads[i]) > 0.05:
        speeds.append(L_speed_rads[i])
        pulses.append(L_pulse_ms[i])
speeds = np.array(speeds)
f = interp1d(R_speed_rads, R_pulse_ms, bounds_error=False)
R_pulse_ms_linear = f(-speeds)

ax.plot(pulses,R_pulse_ms_linear,'.b',ms=4)



data = loadFromFile("","motorCalibLog_biggerLwheel.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']
speeds = []
pulses = []
for i in xrange(len(L_speed_rads)):
    if abs(L_speed_rads[i]) > 0.05:
        speeds.append(L_speed_rads[i])
        pulses.append(L_pulse_ms[i])
speeds = np.array(speeds)
f = interp1d(R_speed_rads, R_pulse_ms, bounds_error=False)
R_pulse_ms_linear = f(-speeds)

ax.plot(pulses,R_pulse_ms_linear,'.r',ms=4)



data = loadFromFile("","motorCalibLog_biggerRwheel.p")
L_speed_rads = data['avgSpeed_L']
L_pulse_ms = data['avgLmotorInput']
R_speed_rads = data['avgSpeed_R']
R_pulse_ms = data['avgRmotorInput']
speeds = []
pulses = []
for i in xrange(len(L_speed_rads)):
    if abs(L_speed_rads[i]) > 0.05:
        speeds.append(L_speed_rads[i])
        pulses.append(L_pulse_ms[i])
speeds = np.array(speeds)
f = interp1d(R_speed_rads, R_pulse_ms, bounds_error=False)
R_pulse_ms_linear = f(-speeds)

ax.plot(pulses,R_pulse_ms_linear,'.g',ms=4)






ax.legend(['$D_L=D_R$','$D_L>D_R$','$D_L<D_R$'])

ax.set_xlabel('Left motor input pulse width [ms]', fontsize=16)
ax.set_ylabel('Right motor input pulse width [ms]', fontsize=16)


ax.set_aspect('equal')
ax.set_xlim([1050,1950])
ax.set_ylim([1050,1950])

ax.set_title('Motor input values that correspond to a linear trajectory', fontsize=18,y=1.01)


#ax.annotate('Forwards motion', xy=(1400, 1800), xycoords='data', xytext=(1500, 1600), textcoords='data', arrowprops=dict(facecolor='black', shrink=0.1), horizontalalignment='left', verticalalignment='top')
ax.arrow(1525, 1575, -150, 150, head_width=2*0.05*100, head_length=2*0.1*100, fc='k', ec='k')
ax.text(1450, 1680, 'Forwards motion', fontsize=16)


ax.arrow(1575, 1525, 150, -150, head_width=2*0.05*100, head_length=2*0.1*100, fc='k', ec='k')
ax.text(1650, 1480, 'Backwards motion', fontsize=16)

tight_layout()

savefig("linear_trajectory_PWM_mapping.pdf")
savefig("linear_trajectory_PWM_mapping.png")


show()


