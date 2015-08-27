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





fig = figure(figsize=(10,6))


suptitle('Linear velocity calibration for two different surfaces', fontsize=18)

subplot(121)


plot([0,0.5*10],[0,5.11*10],'--m',linewidth=2)#,label='$y(x)=10.22x$')
plot([0,0.5*10],[0,4.5*10],'--g',linewidth=0.5)#,label='$v(\omega_c)=9\omega_c$')


plot([0.5,0.5],[0,20],':k',linewidth=1)
text(0.52, 2+0.25, '$\omega_c=0.5 rad/s$', fontsize=14)

arrow(0.5, 3+0.25, 0.3+0.25, 0, head_width=0.3, head_length=0.05, fc='k', ec='k')
text(0.52, 3.25+0.25, 'Nonlinear region', fontsize=14)

linear_speed_log = linear_speed_log2
plot(linear_speed_log[:,0],linear_speed_log[:,2],'b',linewidth=2,label='Forward')
plot(linear_speed_log[:,0],abs(linear_speed_log[:,1]),'r',linewidth=2,label='Backward')


plot(0,0,'--m',linewidth=2,label='$v_{wood}(\omega_c)=10.22\omega_c$')
plot(0,0,'--g',linewidth=2,label='$v_{foam}(\omega_c)=9\omega_c$')


legend(loc='upper left')

title('Surface type: Wooden table', fontsize=16,y=1.01)
xlabel('Motor input rotational velocity $\omega_c$ [rad/s]', fontsize=14)
ylabel('Velocity $v$ (towards wall) [cm/s]', fontsize=14)
xlim([0,1.5])
ylim([0,15.5])
tight_layout()






subplot(122)


plot([0,0.5*10],[0,5.11*10],'--m',linewidth=0.5,label='$y(x)=10.22x$')
plot([0,0.5*10],[0,4.5*10],'--g',linewidth=2,label='$v(\omega_c)=9\omega_c$')


plot([0.35,0.35],[0,20],':k',linewidth=1)
text(0.37, 1, '$\omega_c=0.35 rad/s$', fontsize=14)

arrow(0.35, 1+1, 0.3+0.25, 0, head_width=0.3, head_length=0.05, fc='k', ec='k')
text(0.37, 1.25+1, 'Nonlinear region', fontsize=14)

linear_speed_log = linear_speed_log3
plot(linear_speed_log[:,0],linear_speed_log[:,2],'b',linewidth=2,label='Forward')
plot(linear_speed_log[:,0],abs(linear_speed_log[:,1]),'r',linewidth=2,label='Backward')

#legend(loc='upper left')

title('Surface type: Polyurethane foam', fontsize=16,y=1.01)
xlabel('Motor input rotational velocity $\omega_c$ [rad/s]', fontsize=14)
ylabel('Velocity $v$ (towards wall) [cm/s]', fontsize=14)
xlim([0,1.5])
ylim([0,15.5])
tight_layout()

subplots_adjust(top=0.88)

savefig("linear_velocity_calibration_curves.pdf")
savefig("linear_velocity_calibration_curves.png")

#show()

