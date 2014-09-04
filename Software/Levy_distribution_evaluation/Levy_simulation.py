#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# License: CC BY-SA 4.0 (Attribution-ShareAlike 4.0 International, http://creativecommons.org/licenses/by-sa/4.0/)


# Begin modules
import matplotlib.pyplot as plt
import numpy as np
import scipy.signal as signal
from math import *
import random
# End modules

random.seed() # Set random seed using system time

# Simulation parameters
#N_evaluations = 100

robotSpeed = 6 # cm/s

totalSimulatedDistance = robotSpeed * 60 * 10 # cm
worldRadius = 300/2 # cm
deltaPos = 1 # cm # The minimum simulation step (~robot sampling rate)
backupDistance = 5 # The distance the robot will back up when an obstacle is detected
angleDriftErrorRange = 0.5 # degrees
distanceSensorRange = 10 # cm # object detection threshold

# Levy parameters
alpha = 1.5 # alpha tunes the short/long steps ratio
minDistance = 10 # cm

def getRandomLevyDistance():
	levyVal = pow(random.random(),(-1./alpha)) * minDistance
	return levyVal

def getRandomDriftError():
	return (pi/180.)*random.uniform(-angleDriftErrorRange/2, angleDriftErrorRange/2)

#levy_evals = [getRandomLevyDistance() for i in range(N_evaluations)]

#plt.figure()
#plt.plot(levy_evals)
#plt.ylabel("Levy result [cm]")

#plt.figure()
#plt.hist([min(val,500) for val in levy_evals], normed=1, bins=100, cumulative=False)
#plt.xlim([0,100])
#plt.ylabel("Levy resulting PDF")

plt.figure()


allPosX = []
allPosY = []

for plots in range(5):
	pos_x = []
	pos_y = []
	remainingDistance = totalSimulatedDistance
	x = 0
	y = 0
	#for i in range(N_evaluations):
	while remainingDistance > 0:
		theta = random.random()*2*np.pi
		r = getRandomLevyDistance()
	
		while r > 0:
			theta += getRandomDriftError()
			x = x + deltaPos*np.cos(theta)
			y = y + deltaPos*np.sin(theta)
			remainingDistance -= deltaPos
			if (np.sqrt(x**2+y**2)) > worldRadius-distanceSensorRange:
				theta += getRandomDriftError()
				x = x - backupDistance*np.cos(theta)
				y = y - backupDistance*np.sin(theta)
				remainingDistance -= deltaPos
				break
			pos_x.append(x)
			pos_y.append(y)
			r -= deltaPos
	plt.plot(pos_x, pos_y)
	allPosX += pos_x
	allPosY += pos_y

# Plot the walls
N_points = 40
circle_x = worldRadius*np.cos(np.linspace(0,2*pi, N_points))
circle_y = worldRadius*np.sin(np.linspace(0,2*pi, N_points))
plt.plot(circle_x, circle_y)

plt.ylabel("Levy result [cm]")
plt.gca().set_aspect('equal')
plt.xlim([-worldRadius, worldRadius])
plt.ylim([-worldRadius, worldRadius])

# Density plot
H, xedges, yedges = np.histogram2d(allPosX, allPosY, bins=30, range=[[-worldRadius, worldRadius], [-worldRadius, worldRadius]])
#H, xedges, yedges = np.histogram2d(allPosX, allPosY, range=[[-worldRadius, worldRadius], [-worldRadius, worldRadius]])
#plt.contour(H)

#H, xedges, yedges = np.histogram2d(allPosX, allPosY, range=[[293.,1454.0], [464.,1896.0]], bins=(50, 50))

plt.figure()
im = plt.imshow(np.rot90(np.fliplr(H)), interpolation='nearest', origin='low', extent=[xedges[0], xedges[-1], yedges[0], yedges[-1]])




plt.show()

