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


data_file_name = "2015-08-07-183501"
data = loadFromFile("",data_file_name+"_posLog_raw.p")

ts = data['videoTimestamp']
posX = data['posX']
posY = data['posY']


map_points = loadFromFile("",data_file_name+"map_data.p")
map_points = np.vstack((map_points,map_points[0,:])) # Convert to closed curve

figure()
plot(map_points[:,0], map_points[:,1], 'b')
plot(posX, posY,'.g', ms=0.1)
xlabel('Position in X [cm]', fontsize=16)
ylabel('Position in Y [cm]', fontsize=16)
axis('equal')
xlim([-10,410])
ylim([-410,10])
#tight_layout()
savefig(data_file_name+"position_log.png")


#figure()
#plot(ts, pos[:,0],'b')
#plot(ts, pos[:,1],'g')
#legend(["X","Y"])
#xlabel('Time [seconds]', fontsize=16)
#ylabel('Position [cm]', fontsize=16)
#tight_layout()
#savefig("position_vs_time.png")

show()

