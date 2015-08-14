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


data_file_name = "2014-09-25-193626"
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
connected_points=[[array([ 102.37797247,  -92.67834793]), array([ 134.91864831,  -45.11889862])], [array([ 134.91864831,  -45.11889862]), array([ 102.37797247,  -92.67834793])], [array([  62.95369212, -172.77847309]), array([  57.32165207, -154.63078849])], [array([  57.32165207, -154.63078849]), array([  62.95369212, -172.77847309])], [array([  84.85607009, -219.71214018]), array([  92.99123905, -181.53942428])], [array([  92.99123905, -181.53942428]), array([  84.85607009, -219.71214018])], [array([  79.22403004, -113.32916145]), array([ 105.5068836 ,  -74.53066333])], [array([ 105.5068836 ,  -74.53066333]), array([  79.22403004, -113.32916145])], [array([ 104.25531915,  -70.15018773]), array([ 120.52565707,  -62.01501877])], [array([ 120.52565707,  -62.01501877]), array([ 104.25531915,  -70.15018773])], [array([ 358.94868586, -128.34793492]), array([ 323.9048811 ,  -90.17521902])], [array([ 323.9048811 ,  -90.17521902]), array([ 358.94868586, -128.34793492])], [array([ 168.71088861, -326.72090113]), array([ 199.37421777, -274.15519399])], [array([ 199.37421777, -274.15519399]), array([ 168.71088861, -326.72090113])], [array([ 196.24530663, -229.72465582]), array([ 197.49687109, -226.59574468])], [array([ 197.49687109, -226.59574468]), array([ 196.24530663, -229.72465582])], [array([  62.95369212, -172.15269086]), array([  56.69586984, -155.25657071])], [array([  56.69586984, -155.25657071]), array([  62.95369212, -172.15269086])], [array([ 325.15644556,  -90.80100125]), array([ 357.6971214 , -127.72215269])], [array([ 357.6971214 , -127.72215269]), array([ 325.15644556,  -90.80100125])], [array([ 259.44931164, -283.54192741]), array([ 301.3767209 , -238.48560701])], [array([ 301.3767209 , -238.48560701]), array([ 259.44931164, -283.54192741])], [array([ 268.21026283, -251.00125156]), array([ 272.59073842, -249.74968711])], [array([ 272.59073842, -249.74968711]), array([ 268.21026283, -251.00125156])], [array([ 156.19524406, -131.47684606]), array([ 156.19524406, -130.2252816 ])], [array([ 156.19524406, -130.2252816 ]), array([ 156.19524406, -131.47684606])], [array([ 119.89987484,  -62.01501877]), array([ 103.62953692,  -70.15018773])], [array([ 103.62953692,  -70.15018773]), array([ 119.89987484,  -62.01501877])], [array([ 214.39299124, -261.01376721]), array([ 257.57196496, -252.87859825])], [array([ 257.57196496, -252.87859825]), array([ 214.39299124, -261.01376721])], [array([ 194.99374218, -225.34418023]), array([ 198.74843554, -222.21526909])], [array([ 198.74843554, -222.21526909]), array([ 194.99374218, -225.34418023])], [array([ 285.10638298, -271.65206508]), array([ 318.27284105, -266.64580726])], [array([ 318.27284105, -266.64580726]), array([ 285.10638298, -271.65206508])], [array([ 64.20525657, -97.68460576]), array([ 76.0951189, -96.4330413])], [array([ 76.0951189, -96.4330413]), array([ 64.20525657, -97.68460576])], [array([  66.08260325, -100.18773467]), array([ 76.0951189 , -97.68460576])], [array([ 76.0951189 , -97.68460576]), array([  66.08260325, -100.18773467])], [array([  62.95369212, -100.18773467]), array([ 76.0951189, -96.4330413])], [array([ 76.0951189, -96.4330413]), array([  62.95369212, -100.18773467])], [array([  67.95994994, -102.69086358]), array([ 77.34668335, -95.80725907])], [array([ 77.34668335, -95.80725907]), array([  67.95994994, -102.69086358])], [array([  62.32790989, -103.31664581]), array([ 75.46933667, -97.05882353])], [array([ 75.46933667, -97.05882353]), array([  62.32790989, -103.31664581])], [array([  66.70838548, -100.18773467]), array([ 76.0951189 , -98.31038798])], [array([ 76.0951189 , -98.31038798]), array([  66.70838548, -100.18773467])], [array([ 213.14142678, -261.63954944]), array([ 270.08760951, -247.87234043])], [array([ 270.08760951, -247.87234043]), array([ 213.14142678, -261.63954944])]]
#connected_points=[]
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

