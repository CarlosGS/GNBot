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


data_file_name = "2015-08-07-175946"
data = loadFromFile("",data_file_name+"_posLog_raw.p")

ts = data['videoTimestamp']
posX = data['posX']
posY = data['posY']


# Segment position data into N robot paths
N_markers = len(posX[0,:])
paths = {}
paths['timestamp'] = ts
for r in xrange(N_markers):
    paths[r] = []

indices = range(len(ts))
indices.reverse()
first = True
for i in indices:
    if ts[i] < 20: # Skip first 20 seconds
        break
    blobs = [np.array([posX[i,r],posY[i,r]]) for r in xrange(N_markers)]
    for r in xrange(N_markers):
        pos = []
        if first:
            pos = blobs[r]
        else:
            last_pos = np.array(paths[r][-1])
            distances = [np.linalg.norm(blobs[ri]-last_pos) for ri in xrange(N_markers)]
            idmin = np.argmin(distances)
            minDist = distances[idmin]
            if minDist < 10:
                pos = blobs[idmin]
            else:
                pos = last_pos
        paths[r].append(pos.tolist())
    first = False

for r in xrange(N_markers):
    path = np.array(paths[r])
    paths[r] = np.flipud(path)


map_points = loadFromFile("",data_file_name+"map_data.p")
map_points = np.vstack((map_points,map_points[0,:])) # Convert to closed curve

figure()
plot(map_points[:,0], map_points[:,1], 'b')
plot(posX,posY,'.y', ms=0.5)
for r in xrange(N_markers):
    plot(paths[r][:,0],paths[r][:,1],linewidth=0.5)
    #plot(paths[r][:,0],paths[r][:,1],'.', ms=0.1)
xlabel('Position in X [cm]', fontsize=16)
ylabel('Position in Y [cm]', fontsize=16)
axis('equal')
xlim([-10,410])
ylim([-410,10])
#tight_layout()
savefig(data_file_name+"position_log.png")

saveToFile(paths,"",data_file_name+"_posLog.p")

#figure()
#plot(ts, pos[:,0],'b')
#plot(ts, pos[:,1],'g')
#legend(["X","Y"])
#xlabel('Time [seconds]', fontsize=16)
#ylabel('Position [cm]', fontsize=16)
#tight_layout()
#savefig("position_vs_time.png")

show()

