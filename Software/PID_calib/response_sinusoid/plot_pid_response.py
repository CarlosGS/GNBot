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


from pid_response1 import *
from pid_response2 import *
from pid_response3 import *
from pid_response4 import *

pid_response1 = np.array(pid_response1)
pid_response2 = np.array(pid_response2)
pid_response3 = np.array(pid_response3)
pid_response4 = np.array(pid_response4)

# Initial timestamp is T=0
pid_response1[:,0] -= pid_response1[0,0]
pid_response2[:,0] -= pid_response2[0,0]
pid_response3[:,0] -= pid_response3[0,0]
pid_response4[:,0] -= pid_response4[0,0]

f, ax = subplots(1,figsize=(10,5))
toffset = 2000
ax.plot((pid_response4[:,0]-2000)/1000.+0.05,pid_response4[:,2]*180./np.pi,'--k')
ax.plot((pid_response3[:,0]-2000)/1000.,pid_response3[:,1]*180./np.pi,'r')
ax.plot((pid_response2[:,0]-2000)/1000.,pid_response2[:,1]*180./np.pi,'b')
ax.plot((pid_response1[:,0]-2000)/1000.,pid_response1[:,1]*180./np.pi,'g')
ax.plot((pid_response4[:,0]-2000)/1000.,pid_response4[:,1]*180./np.pi,'c',linewidth=2)

#f, ax = subplots(1,figsize=(10,5))
#time_seconds = (pid_response4[:,0]-8500)/1000.
#ax.plot(time_seconds+0.05,pid_response1[:,2]*180./np.pi,'--k')
#ax.plot(time_seconds,pid_response3[:,1]*180./np.pi,'r')
#ax.plot(time_seconds,pid_response2[:,1]*180./np.pi,'b')
#ax.plot(time_seconds,pid_response1[:,1]*180./np.pi,'g')
#ax.plot(time_seconds,pid_response4[:,1]*180./np.pi,'c',linewidth=2)

ax.legend(["Target yaw (sinusoid)","$K_p=10, K_{\{i,d\}}=0$","$K_p=5, K_{\{i,d\}}=0$","$K_p=1, K_{\{i,d\}}=0$","$K_{\{p,i,d\}}=\{4.15,5.85,0.21\}$"])


ax.set_title('Comparison of a proportional yaw controller with the auto-tuned PID (II)', fontsize=18,y=1.01)
ax.set_xlabel('Time [seconds]', fontsize=16)
ax.set_ylabel('Yaw [degrees]', fontsize=16)

ax.set_ylim([-15,15])
ax.set_xlim([0,3])

tight_layout()

savefig("P_controller_vs_autotuned_PID_sinusoid.pdf")
savefig("P_controller_vs_autotuned_PID_sinusoid.png")

show()
