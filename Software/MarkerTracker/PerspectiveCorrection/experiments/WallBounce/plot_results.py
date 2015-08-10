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


data_file_name = "2015-08-06-201917"
data = loadFromFile("",data_file_name+"_posLog.p")

ts = data['videoTimestamp']
pos = data['pos']

figure()
plot(pos[:,0], pos[:,1])
xlabel('Position in X [cm]', fontsize=16)
ylabel('Position in Y [cm]', fontsize=16)
axis('equal')
tight_layout()
savefig("position_log.png")


figure()
plot(ts, pos[:,0],'b')
plot(ts, pos[:,1],'g')
legend(["X","Y"])
xlabel('Time [seconds]', fontsize=16)
ylabel('Position [cm]', fontsize=16)
tight_layout()
savefig("position_vs_time.png")

show()

