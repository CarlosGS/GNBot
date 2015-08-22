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
plot(pathsA[0][:,0],pathsA[0][:,1], 'g', linewidth=1)
plot(pathsA[1][:,0],pathsA[1][:,1], 'k', linewidth=1)
plot(pathsA[2][:,0],pathsA[2][:,1], 'r', linewidth=1)
plot(pathsA[3][:,0],pathsA[3][:,1], 'c', linewidth=1)

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
plot(map_pointsA[:,0], map_pointsA[:,1], 'b', linewidth=1.5)
plot(map_pointsB[:,0], map_pointsB[:,1], 'b', linewidth=1.5, label="Map walls")
plot(pathsB[3][:,0],pathsB[3][:,1], 'k', linewidth=1, label="Robot 1")
plot(pathsB[2][:,0],pathsB[2][:,1], 'c', linewidth=1, label="Robot 2")
plot(pathsB[0][:,0],pathsB[0][:,1], 'r', linewidth=1, label="Robot 3")
plot(pathsB[1][:,0],pathsB[1][:,1], 'g', linewidth=1, label="Robot 4")

# Draw markers
plot(pathsB[3][0,0],pathsB[3][0,1], 'k>', markersize=10)
plot(pathsB[3][-1,0],pathsB[3][-1,1], 'ks', markersize=8)

plot(pathsB[2][0,0],pathsB[2][0,1], 'c>', markersize=10)
plot(pathsB[2][-1,0],pathsB[2][-1,1], 'cs', markersize=8)

plot(pathsB[0][0,0],pathsB[0][0,1], 'r>', markersize=10)
plot(pathsB[0][-1,0],pathsB[0][-1,1], 'rs', markersize=8)

plot(pathsB[1][0,0],pathsB[1][0,1], 'g>', markersize=10)
plot(pathsB[1][-1,0],pathsB[1][-1,1], 'gs', markersize=8)




def drawRobot(ax,wheel_type,yoffset=0):
    dimX = 9.
    dimY = 8.
    # Chassis
    ax.add_patch(Rectangle((-dimX/2.,-dimY/2.+yoffset),dimX,dimY,linewidth=0.5, fc='w', ec='k'))
    # Wheels
    Dr = 6.5
    Dl = 6.5
    if wheel_type == "larger left":
        Dl += 1
    elif wheel_type == "larger right":
        Dr += 1
    ax.add_patch(Rectangle((-dimX/2.,dimY/2.+yoffset),Dr,1,linewidth=0, fc='k', ec='k'))
    ax.add_patch(Rectangle((-dimX/2.,-dimY/2.-1+yoffset),Dl,1,linewidth=0, fc='k', ec='k'))
    
    ax.arrow(-dimX/3., yoffset, dimX/2., 0, head_width=dimY/2., head_length=dimX/4.,linewidth=0.1, fc='g', ec='c')

drawRobot(gca(), "same", yoffset=390)


#annotate("Actual size of each robot", xy=(10, 0), xytext=(40, -20),arrowprops=dict(arrowstyle="->"), fontsize=10)
text(10, 385, 'Actual size of each robot', fontsize=10)


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
#show()


