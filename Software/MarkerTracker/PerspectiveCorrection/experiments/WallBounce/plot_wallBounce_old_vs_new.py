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


data_file_nameA = "2014-09-25-193626"
data_file_nameB = "2015-08-07-183814"
pathsA = loadFromFile("",data_file_nameA+"_posLog.p")
pathsB = loadFromFile("",data_file_nameB+"_posLog.p")


tsA = pathsA['timestamp']
tsB = pathsB['timestamp']
N_markersA = len(pathsA.keys())-1
N_markersB = len(pathsB.keys())-1

N_pointsA = len(pathsA[0][:,0])
N_pointsB = len(pathsB[0][:,0])


map_pointsA = loadFromFile("",data_file_nameA+"map_data.p")
map_pointsA = np.vstack((map_pointsA,map_pointsA[0,:])) # Convert to closed curve
map_pointsB = loadFromFile("",data_file_nameB+"map_data.p")
map_pointsB = np.vstack((map_pointsB,map_pointsB[0,:])) # Convert to closed curve



# Low pass filtering:
for r in xrange(N_markersA):
    first = True
    for i in xrange(N_pointsA):
        if not first:
            pathsA[r][i,:] = pathsA[r][i-1,:]*0.9 + pathsA[r][i,:]*0.1
        else:
            first = False
# Move XY=(0,0) to lower left corner
Xoffset = min(map_pointsA[:,0])
Yoffset = min(map_pointsA[:,1])
offset = np.array([Xoffset,Yoffset])
for r in xrange(N_markersA):
    pathsA[r] -= offset
map_pointsA -= offset


# Low pass filtering:
for r in xrange(N_markersB):
    first = True
    for i in xrange(N_pointsB):
        if not first:
            pathsB[r][i,:] = pathsB[r][i-1,:]*0.9 + pathsB[r][i,:]*0.1
        else:
            first = False
# Move XY=(0,0) to lower left corner
Xoffset = min(map_pointsB[:,0])-max(map_pointsA[:,0])-10
Yoffset = min(map_pointsB[:,1])
offset = np.array([Xoffset,Yoffset])
for r in xrange(N_markersB):
    pathsB[r] -= offset
map_pointsB -= offset



figure(figsize=(12,7))

# Draw paths
plot(pathsA[0][:,0],pathsA[0][:,1], 'g', linewidth=1.5)
plot(pathsA[1][:,0],pathsA[1][:,1], 'k', linewidth=1.5)
plot(pathsA[2][:,0],pathsA[2][:,1], 'r', linewidth=1.5)
plot(pathsA[3][:,0],pathsA[3][:,1], 'c', linewidth=1.5)

# Draw markers
plot(pathsA[0][0,0],pathsA[0][0,1], 'g>', markersize=10)
plot(pathsA[0][-1,0],pathsA[0][-1,1], 'gs', markersize=8)

plot(pathsA[1][0,0],pathsA[1][0,1], 'k>', markersize=10)
plot(pathsA[1][-1,0],pathsA[1][-1,1], 'ks', markersize=8)

plot(pathsA[2][0,0],pathsA[2][0,1], 'r>', markersize=10)
plot(pathsA[2][-1,0],pathsA[2][-1,1], 'rs', markersize=8)

plot(pathsA[3][0,0],pathsA[3][0,1], 'c>', markersize=10)
plot(pathsA[3][-1,0],pathsA[3][-1,1], 'cs', markersize=8)


# Draw map walls
plot(map_pointsA[:,0], map_pointsA[:,1], 'b', linewidth=2)
plot(map_pointsB[:,0], map_pointsB[:,1], 'b', linewidth=2, label="Map walls")
plot(pathsB[3][:,0],pathsB[3][:,1], 'k', linewidth=1.5, label="Robot 1")
plot(pathsB[2][:,0],pathsB[2][:,1], 'c', linewidth=1.5, label="Robot 2")
plot(pathsB[0][:,0],pathsB[0][:,1], 'r', linewidth=1.5, label="Robot 3")
plot(pathsB[1][:,0],pathsB[1][:,1], 'g', linewidth=1.5, label="Robot 4")

# Draw markers
plot(pathsB[3][0,0],pathsB[3][0,1], 'k>', markersize=10)
plot(pathsB[3][-1,0],pathsB[3][-1,1], 'ks', markersize=8)

plot(pathsB[2][0,0],pathsB[2][0,1], 'c>', markersize=10)
plot(pathsB[2][-1,0],pathsB[2][-1,1], 'cs', markersize=8)

plot(pathsB[0][0,0],pathsB[0][0,1], 'r>', markersize=10)
plot(pathsB[0][-1,0],pathsB[0][-1,1], 'rs', markersize=8)

plot(pathsB[1][0,0],pathsB[1][0,1], 'g>', markersize=10)
plot(pathsB[1][-1,0],pathsB[1][-1,1], 'gs', markersize=8)


xlabel('Position in X [cm]', fontsize=14)
ylabel('Position in Y [cm]', fontsize=14)
axis('equal')

xlim([-1,max(map_pointsB[:,0])+1])
ylim([-1,max(map_pointsA[:,1])+1])

legend(ncol=5)

title("``Wall bounce'' search using the original controller (left) and the new self-calibration (right)", fontsize=18)

tight_layout()
savefig("wallBounce_oldVSnew.png")
savefig("wallBounce_oldVSnew.pdf")
show()


exit()




f, ax = subplots(1,2, figsize=(14,7))


for r in xrange(N_markersA):
    ax[0].plot(pathsA[r][:,0],pathsA[r][:,1], linewidth=2)
ax[0].plot(map_pointsA[:,0], map_pointsA[:,1], 'b')
ax[0].set_xlabel('Position in X [cm]', fontsize=16)
ax[0].set_ylabel('Position in Y [cm]', fontsize=16)
ax[0].axis('equal')
#ax[0].set_xlim([-5,400])
#ax[0].set_ylim([-5,380])


for r in xrange(N_markersB):
    ax[0].plot(pathsB[r][:,0],pathsB[r][:,1], linewidth=2)
ax[0].plot(map_pointsB[:,0], map_pointsB[:,1], 'b')
#ax[1].set_xlabel('Position in X [cm]', fontsize=16)
#ax[1].set_ylabel('Position in Y [cm]', fontsize=16)
#ax[1].axis('equal')
#ax[1].set_xlim([-5,400])
#ax[1].set_ylim([-5,380])

tight_layout()
savefig("wallBounce_oldVSnew.png")
savefig("wallBounce_oldVSnew.pdf")
show()

