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

#f, ax = subplots(1,figsize=(10,5))
#ax.plot(pid_autoTune1[:,0],pid_autoTune1[:,1:])






#data = {}
#data[pid_autoTune1[:,1]] = pid_autoTune1[:,2:]

#pid_autoTune1 = pid_autoTune2

Kps = np.unique(pid_autoTune1[:,1])[::-1]

times = {}
Tu = {}
err = {}
err_peak = {}

for Kp in Kps:
    indices = pid_autoTune1[:,1] == Kp
    times[Kp] = pid_autoTune1[indices,0]/1000.
    Tu[Kp] = pid_autoTune1[indices,2]
    err[Kp] = pid_autoTune1[indices,4]
    err_peak[Kp] = pid_autoTune1[indices,3]


# Start and end signals with zero value
for Kp in Kps:
    times[Kp] = np.insert(times[Kp], 0, times[Kp][0])
    err[Kp] = np.insert(err[Kp], 0, 0)
    err_peak[Kp] = np.insert(err_peak[Kp], 0, 0)
    times[Kp] = np.append(times[Kp], times[Kp][-1])
    err[Kp] = np.append(err[Kp], 0)
    err_peak[Kp] = np.append(err_peak[Kp], 0)

# Extend signals value as zero
for i in xrange(len(Kps)-1):
    Kp = Kps[i]
    times[Kp] = np.append(times[Kp], times[Kps[i+1]][0]) # Append the fist TS of the next signal
    err[Kp] = np.append(err[Kp], 0)
    err_peak[Kp] = np.append(err_peak[Kp], 0)

#f, ax = subplots(1,figsize=(10,5))
#ax.plot(pid_autoTune2[:,0],pid_autoTune2[:,1:])

f, ax = subplots(1,figsize=(10,4.6))

ax.plot([0,11],[0,0],':k')

legends = []
i = 0
for Kp in Kps:
    #if i == 3:
    #    legends.append("$K_P="+str(Kp)+"=K_u$")
    #elif i == 6:
    #    legends.append("$K_P="+str(Kp)+"\\nearrow$")
    #else:
    legends.append("$K_p="+str(Kp)+"$")
    i += 1

i = 0
for Kp in Kps:
    ax.plot(times[Kp],err[Kp]*180./np.pi, label=legends[i])
    i += 1

for Kp in Kps:
    peaks, = ax.plot(times[Kp],err_peak[Kp]*180./np.pi,color = '0.75',linewidth=2)


l1 = ax.legend(labelspacing=0.25,fontsize = 'medium',mode='expand', loc='upper center',ncol=5)#loc='lower right')
l2 = legend([peaks], ['Measured amplitude of the error oscillation'], loc='lower right',labelspacing=0.25,fontsize = 'medium') # this removes l1 from the axes.
gca().add_artist(l1)

ax.arrow(8.15, -7, -0.06, 0, head_width=2, head_length=0.05, fc='k', ec='k')
ax.arrow(8.15, -7, 0.06, 0, head_width=2, head_length=0.05, fc='k', ec='k')
ax.text(7.95, -14, '$T_u=0.24s$', fontsize=12)

ax.text(7.35, 14, '$K_u=K_p=18.45$', fontsize=12)


ax.arrow(9.9, 2, 0.2, 10, head_width=0, head_length=0, fc='k', ec='k')
ax.text(10.1, 10+5, 'Critically', fontsize=10)
ax.text(10.15, 10, 'damped', fontsize=10)

ax.set_title('PID auto-tuning process: Empirical measure of the values $K_u$ and $T_u$', fontsize=14,y=1.01)
ax.set_xlabel('Time [seconds]', fontsize=14)
ax.set_ylabel('Yaw error [deg]', fontsize=14)
ax.set_xlim([0,11])
ax.set_ylim([-65,65])
tight_layout()




savefig("PID_auto_tuning.pdf")
savefig("PID_auto_tuning.png")

exit()






# Alignment for next plot
i = 0
for Kp in Kps:
    times[Kp] -= times[Kp][0]
    if i%2:
        err[Kp] = -err[Kp]
    else:
        err[Kp] = err[Kp]
    i += 1


f, ax = subplots(1,figsize=(10,5))
i = 0
for Kp in Kps:
    if i == 5:
        ax.plot(times[Kp],err[Kp]*180./np.pi,linewidth=2)
    elif i == 6:
        ax.plot(times[Kp],err[Kp]*180./np.pi,linewidth=2)
    else:
        ax.plot(times[Kp],err[Kp]*180./np.pi)
    i += 1

legends = []
i = 0
for Kp in Kps:
    if i == 5:
        legends.append("$K_P="+str(Kp)+"=K_u$")
    #elif i == 6:
    #    legends.append("$K_P="+str(Kp)+"\\nearrow$")
    else:
        legends.append("$K_P="+str(Kp)+"$")
    i += 1

ax.legend(legends)

ax.set_ylim([-15,50])
ax.set_xlim([0,1.5])

ax.set_xlabel('Time [seconds]', fontsize=14)
ax.set_ylabel('Yaw error [deg]', fontsize=14)


tight_layout()


show()
