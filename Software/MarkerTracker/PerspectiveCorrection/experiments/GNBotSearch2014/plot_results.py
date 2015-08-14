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


data_file_name = "2014-09-25-191650"
data = loadFromFile("",data_file_name+"_posLog_raw.p")

ts = data['videoTimestamp']
posX = data['posX']
posY = data['posY']


#figure()
#N_markers = len(posX[0,:])
#plot(posX[-1,:],posY[-1,:], '.', ms=10)
#show()



cmap = get_cmap('jet')

fig = figure()
N_points = len(posX)
for i in xrange(N_points):
    t = float(ts[i])/float(ts[-1])
    plot(posX[i],posY[i], '.', ms=1, c=cmap(t))
axis('equal')

global connected_points
global initial

# For 2014-09-25-191650.webm
#connected_points=[[array([ 176.84605757,  -39.48685857]), array([ 186.23279099,  -43.24155194])], [array([ 186.23279099,  -43.24155194]), array([ 176.84605757,  -39.48685857])], [array([ 241.92740926,  -18.83604506]), array([ 240.05006258,  -15.70713392])], [array([ 240.05006258,  -15.70713392]), array([ 241.92740926,  -18.83604506])], [array([ 227.53441802, -209.69962453]), array([ 233.16645807, -222.21526909])], [array([ 233.16645807, -222.21526909]), array([ 227.53441802, -209.69962453])], [array([  81.10137672, -206.57071339]), array([  96.74593242, -207.19649562])], [array([  96.74593242, -207.19649562]), array([  81.10137672, -206.57071339])], [array([ 121.77722153, -274.78097622]), array([ 156.82102628, -292.92866083])], [array([ 156.82102628, -292.92866083]), array([ 121.77722153, -274.78097622])], [array([ 161.20150188, -190.30037547]), array([ 173.71714643, -155.88235294])], [array([ 173.71714643, -155.88235294]), array([ 161.20150188, -190.30037547])], [array([  58.57321652, -213.4543179 ]), array([  76.0951189 , -247.87234043])], [array([  76.0951189 , -247.87234043]), array([  58.57321652, -213.4543179 ])], [array([ 175.59449312, -146.49561952]), array([ 161.20150188, -189.67459324])], [array([ 161.20150188, -189.67459324]), array([ 175.59449312, -146.49561952])]]

connected_points=[]
initial = []
def onMouseUp(event):
    global initial, connected_points
    xy = np.array([event.xdata,event.ydata])
    if len(initial) > 0:
        connected_points.append([initial,xy])
        connected_points.append([xy,initial])
        initial = []
    else:
        initial = xy
    print(event)
    print(xy)

fig.canvas.mpl_connect('button_release_event', onMouseUp)

show()



N_connected_points = len(connected_points)
print("connected_points=")
print(connected_points)


# Segment position data into N robot paths
N_markers = len(posX[0,:])
paths = {}
paths['timestamp'] = ts
for r in xrange(N_markers):
    paths[r] = []

indices = range(len(ts))
indices.reverse()
first = True
#speeds = [np.array([0.,0.]) for r in xrange(N_markers)]
#accels = [np.array([0.,0.]) for r in xrange(N_markers)]
#last_speeds = [np.array([0.,0.]) for r in xrange(N_markers)]
for i in indices:
    if ts[i] < 15: # Skip first seconds
        break
    blobs = [np.array([posX[i,r],posY[i,r]]) for r in xrange(N_markers)]
    for r in xrange(N_markers):
        pos = []
        if first:
            pos = blobs[r]
        else:
            last_pos = np.array(paths[r][-1])
            distancesA = [np.linalg.norm(blobs[ri]-last_pos) for ri in xrange(N_markers)]
            idminA = np.argmin(distancesA)
            minDistA = distancesA[idminA]
            
            distancesB = [np.linalg.norm(connected_points[ri][0]-last_pos) for ri in xrange(N_connected_points)]
            idminB = np.argmin(distancesB)
            minDistB = distancesB[idminB]
            
            distancesC = [np.linalg.norm(blobs[ri]-connected_points[idminB][1]) for ri in xrange(N_markers)]
            idminC = np.argmin(distancesC)
            minDistC = distancesC[idminC]
            
            
            if minDistA < 3:
                pos = blobs[idminA]
                #speeds[r] = speeds[r]*0.99 + (pos-last_pos)*0.01
                #accels[r] = accels[r]*0.9 + (speeds[r]-last_speeds[r])*0.1
            elif minDistB < 3 and minDistC < 3:
                pos = blobs[idminC]
            else:
                pos = last_pos
                #pos = last_pos + speeds[r]
                #speeds[r] -= accels[r]
            #last_speeds[r] = speeds[r]
        paths[r].append(pos.tolist())
    first = False

for r in xrange(N_markers):
    path = np.array(paths[r])
    paths[r] = np.flipud(path)


map_points = loadFromFile("",data_file_name+"map_data.p")
map_points = np.vstack((map_points,map_points[0,:])) # Convert to closed curve

figure()
plot(map_points[:,0], map_points[:,1], 'b')
plot(posX,posY,'.y', ms=2)
for r in xrange(N_markers):
    plot(paths[r][:,0],paths[r][:,1],linewidth=0.5)
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

