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


data_file_name = "2015-08-07-125531"


dataA = loadFromFile("","backwards/2015-08-07-125531_posLog.p")
dataB = loadFromFile("","forwards/2015-08-07-123434_posLog.p")

tsA = dataA['videoTimestamp']
posA = dataA['pos']

tsB = dataB['videoTimestamp']
posB = dataB['pos']


# Move initial point to (0,0)
posA[:,0] -= posA[0,0]
posA[:,1] -= posA[0,1]

posB[:,0] -= posB[0,0]
posB[:,1] -= posB[0,1]

# Low pass filter
last = np.array([0,0])
for i in xrange(len(posA[:,0])):
    posA[i,:] = posA[i,:]*0.1 + last*0.9
    last = posA[i,:]

# Low pass filter
last = np.array([0,0])
for i in xrange(len(posB[:,0])):
    posB[i,:] = posB[i,:]*0.1 + last*0.9
    last = posB[i,:]

sep = 30
posA[:,0] += sep
posB[:,0] -= sep


figure(figsize=(6,6.5))

plot([-sep*2,sep*2], [100,100], '--k')
plot([-sep*2,sep*2], [0,0], '--k')

plot([sep,sep], [0,100], '--k', linewidth=2)
plot([-sep,-sep], [0,100], '--k', linewidth=2)

#plot(posA[:,0], posA[:,1], 'g', label="Backward motion")
#plot(posB[:,0], posB[:,1], 'b', label="Forward motion")




first = True
flag = False
N = len(posB[:,0])
for i in xrange(N):
    if not first:
        color = (0.7,0,0,1-np.sqrt(float(i)/float(N)))
        x1 = posB[i-1,0]
        y1 = posB[i-1,1]
        x2 = posB[i,0]
        y2 = posB[i,1]
        if flag:
            plot([x1,x2], [y1,y2], c=color, linewidth=1.5, label="Forward")
        else:
            plot([x1,x2], [y1,y2], c=color, linewidth=1.5)
        flag = False
    else:
        first = False
        flag = True





first = True
flag = False
N = len(posA[:,0])
for i in xrange(N):
    if not first:
        color = (0,0.5,0,1-np.sqrt(float(i)/float(N)))
        x1 = posA[i-1,0]
        y1 = posA[i-1,1]
        x2 = posA[i,0]
        y2 = posA[i,1]
        if flag:
            plot([x1,x2], [y1,y2], c=color, linewidth=1.5, label="Backward")
        else:
            plot([x1,x2], [y1,y2], c=color, linewidth=1.5)
        flag = False
    else:
        first = False
        flag = True




def drawRobotUP(ax,wheel_type,xoffset=0):
    dimX = 9.
    dimY = 8.
    # Chassis
    ax.add_patch(Rectangle((-dimY/2.+xoffset,-dimX/2.),dimY,dimX,linewidth=0.5, fc='w', ec='k'))
    # Wheels
    Dr = 6.5
    Dl = 6.5
    if wheel_type == "larger left":
        Dl += 1
    elif wheel_type == "larger right":
        Dr += 1
    ax.add_patch(Rectangle((-dimY/2.-1+xoffset,-dimX/2.),1,Dr,linewidth=0, fc='k', ec='k'))
    ax.add_patch(Rectangle((dimY/2.+xoffset,-dimX/2.),1,Dl,linewidth=0, fc='k', ec='k'))
    
    ax.arrow(xoffset, -dimX/3., 0, dimX/2., head_width=dimY/2., head_length=dimX/4.,linewidth=1, fc='g', ec='c')
    ax.arrow(xoffset-10, 5, 0, 10, head_width=dimY/4., head_length=dimX/2.,fc='k', ec='k')

def drawRobotDOWN(ax,wheel_type,xoffset=0):
    dimX = 9.
    dimY = 8.
    # Chassis
    ax.add_patch(Rectangle((-dimY/2.+xoffset,-dimX/2.),dimY,dimX,linewidth=0.5, fc='w', ec='k'))
    # Wheels
    Dr = 6.5
    Dl = 6.5
    if wheel_type == "larger left":
        Dl += 1
    elif wheel_type == "larger right":
        Dr += 1
    ax.add_patch(Rectangle((-dimY/2.-1+xoffset,dimX/2.),1,-Dr,linewidth=0, fc='k', ec='k'))
    ax.add_patch(Rectangle((dimY/2.+xoffset,dimX/2.),1,-Dl,linewidth=0, fc='k', ec='k'))
    
    ax.arrow(xoffset, dimX/3., 0, -dimX/2., head_width=dimY/2., head_length=dimX/4.,linewidth=1, fc='g', ec='c')
    ax.arrow(xoffset-10, 5, 0, 10, head_width=dimY/4., head_length=dimX/2.,fc='k', ec='k')

drawRobotUP(gca(), "same", xoffset=-sep)
drawRobotDOWN(gca(), "same", xoffset=sep)




legend(loc="upper center", ncol=2)

ylim([-10,120])

xlabel('Position in X [cm]', fontsize=16)
ylabel('Position in Y [cm]', fontsize=16)
axis('equal')
title('Cumulative yaw error due to gyro drift', fontsize=18)
tight_layout()
savefig("cumulative_yaw_error.png")
savefig("cumulative_yaw_error.pdf")

#show()

