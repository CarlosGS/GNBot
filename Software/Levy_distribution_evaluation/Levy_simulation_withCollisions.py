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
robotSpeed = 6 # cm/s
totalSimulatedDistance = robotSpeed * 60 * 10 # cm
deltaPos = 1 # cm # The minimum simulation step (~robot sampling rate)
angleDriftErrorRange = 0.1 # degrees/deltaPos # The drift when performing linear trajectories
backupDistance = 5 # cm # The distance the robot will back up when an obstacle is detected
distanceSensorRange = 10 # cm # object detection threshold

worldRadius = 300/2 # cm

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

walkType = 1 # 1 Levy, 2 Wall bounce (simple), 3 Wall bounce, 4 Spiral

N_robots = 5

for plots in range(5):
	pos_x = range(N_robots)
	pos_y = range(N_robots)
	remainingDistance = range(N_robots)
	x = range(N_robots)
	y = range(N_robots)
	driftError = range(N_robots)
	theta = range(N_robots)
	bounceCount = range(N_robots)
	timeCounter = range(N_robots)

	for robot_i in range(N_robots):
		pos_x[robot_i] = []
		pos_y[robot_i] = []
		remainingDistance[robot_i] = totalSimulatedDistance
		x[robot_i] = 0
		y[robot_i] = 0
	
		driftError[robot_i] = getRandomDriftError()
	
		theta[robot_i] = 0
		bounceCount[robot_i] = 0
		timeCounter[robot_i] = 0


	def distanceSensorTriggered(robot_ID):
		posX = x[robot_ID]
		posY = y[robot_ID]
		posTheta = theta[robot_ID]
		measuredDist = deltaPos
	
		while measuredDist < distanceSensorRange:
			posX = posX + deltaPos*np.cos(posTheta)
			posX = posX + deltaPos*np.sin(posTheta)
			if (np.sqrt(posX**2+posY**2)) > worldRadius:
				return measuredDist
			for robot_i in range(N_robots):
				if robot_i != robot_ID:
					if (np.sqrt((x[robot_i]-posX)**2+(y[robot_i]-posY)**2)) < 5: # cm
						return measuredDist
			measuredDist += deltaPos # cm
		return 0


	isActive = True
	while isActive == True:
		isActive = False
		for robot_i in range(N_robots):
			while remainingDistance[robot_i] > 0:
				isActive = True
				if walkType == 1: # Levy wals
					theta[robot_i] = random.random()*2*np.pi
					r = getRandomLevyDistance()
					while r > 0 and remainingDistance[robot_i] > 0:
						theta[robot_i] += driftError[robot_i]
						x[robot_i] = x[robot_i] + deltaPos*np.cos(theta[robot_i])
						y[robot_i] = y[robot_i] + deltaPos*np.sin(theta[robot_i])
						remainingDistance[robot_i] -= deltaPos
						#if (np.sqrt(x[robot_i]**2+y[robot_i]**2)) > worldRadius-distanceSensorRange:
						if distanceSensorTriggered(robot_i):
							theta[robot_i] -= driftError[robot_i]
							x[robot_i] = x[robot_i] - backupDistance*np.cos(theta[robot_i])
							y[robot_i] = y[robot_i] - backupDistance*np.sin(theta[robot_i])
							remainingDistance[robot_i] -= backupDistance
							pos_x[robot_i].append(x[robot_i])
							pos_y[robot_i].append(y[robot_i])
							break
						pos_x[robot_i].append(x[robot_i])
						pos_y[robot_i].append(y[robot_i])
						r -= deltaPos
				elif walkType == 2: # Wall bounce (simple)
					if theta[robot_i] == 0:
						theta[robot_i] = random.random()*2*np.pi
					if bounceCount[robot_i] == 0:
						bounceCount[robot_i] = round(random.random())
					while True and remainingDistance[robot_i] > 0:
						offsetDistance = minDistance
						theta[robot_i] += driftError[robot_i]
						x[robot_i] = x[robot_i] + deltaPos*np.cos(theta[robot_i])
						y[robot_i] = y[robot_i] + deltaPos*np.sin(theta[robot_i])
						remainingDistance[robot_i] -= deltaPos
						#if (np.sqrt(x[robot_i]**2+y[robot_i]**2)) > worldRadius-distanceSensorRange:
						if distanceSensorTriggered(robot_i):
							theta[robot_i] -= driftError[robot_i]
							x[robot_i] = x[robot_i] - backupDistance*np.cos(theta[robot_i])
							y[robot_i] = y[robot_i] - backupDistance*np.sin(theta[robot_i])
							remainingDistance[robot_i] -= backupDistance
							pos_x[robot_i].append(x[robot_i])
							pos_y[robot_i].append(y[robot_i])
							theta[robot_i] += (45+90)*pi/180.
					
							bounceCount[robot_i] += 1
							break
						pos_x[robot_i].append(x[robot_i])
						pos_y[robot_i].append(y[robot_i])
				elif walkType == 3: # Wall bounce
					if theta[robot_i] == 0:
						theta[robot_i] = random.random()*2*np.pi
					if bounceCount[robot_i] == 0:
						bounceCount[robot_i] = round(random.random())
					while True and remainingDistance[robot_i] > 0:
						offsetDistance = minDistance
						theta[robot_i] += driftError[robot_i]
						x[robot_i] = x[robot_i] + deltaPos*np.cos(theta[robot_i])
						y[robot_i] = y[robot_i] + deltaPos*np.sin(theta[robot_i])
						remainingDistance[robot_i] -= deltaPos
						#if (np.sqrt(x[robot_i]**2+y[robot_i]**2)) > worldRadius-distanceSensorRange:
						if distanceSensorTriggered(robot_i):
							theta[robot_i] -= driftError[robot_i]
							x[robot_i] = x[robot_i] - backupDistance*np.cos(theta[robot_i])
							y[robot_i] = y[robot_i] - backupDistance*np.sin(theta[robot_i])
							remainingDistance[robot_i] -= backupDistance
							pos_x[robot_i].append(x[robot_i])
							pos_y[robot_i].append(y[robot_i])
					
							if bounceCount[robot_i]%2:
								theta[robot_i] += 90*pi/180.
							else:
								theta[robot_i] -= 90*pi/180.
							x[robot_i] = x[robot_i] + offsetDistance*np.cos(theta[robot_i])
							y[robot_i] = y[robot_i] + offsetDistance*np.sin(theta[robot_i])
							remainingDistance[robot_i] -= offsetDistance
							pos_x[robot_i].append(x[robot_i])
							pos_y[robot_i].append(y[robot_i])
					
							#if (np.sqrt(x[robot_i]**2+y[robot_i]**2)) > worldRadius-distanceSensorRange:
							if distanceSensorTriggered(robot_i):
								x[robot_i] = x[robot_i] - 2*offsetDistance*np.cos(theta[robot_i])
								y[robot_i] = y[robot_i] - 2*offsetDistance*np.sin(theta[robot_i])
								remainingDistance[robot_i] -= 2*offsetDistance
								pos_x[robot_i].append(x[robot_i])
								pos_y[robot_i].append(y[robot_i])
					
							if bounceCount[robot_i]%2:
								theta[robot_i] += 90*pi/180.
							else:
								theta[robot_i] -= 90*pi/180.
					
							bounceCount[robot_i] += 1
							break
						pos_x[robot_i].append(x[robot_i])
						pos_y[robot_i].append(y[robot_i])
				elif walkType == 4: # Spiral
					if theta[robot_i] == 0:
						theta[robot_i] = random.random()*2*np.pi
					while True and remainingDistance[robot_i] > 0:
						theta[robot_i] += driftError[robot_i]
						theta[robot_i] += (90*pi/180.)/(sqrt(N_robots*minDistance*0.7595*timeCounter[robot_i])+1)
						#theta[robot_i] += (90*pi/180.)/(sqrt(minDistance*0.7595*timeCounter[robot_i])+1)
						x[robot_i] = x[robot_i] + deltaPos*np.cos(theta[robot_i])
						y[robot_i] = y[robot_i] + deltaPos*np.sin(theta[robot_i])
						remainingDistance[robot_i] -= deltaPos
						timeCounter[robot_i] += deltaPos
						#if (np.sqrt(x[robot_i]**2+y[robot_i]**2)) > worldRadius-distanceSensorRange:
						if distanceSensorTriggered(robot_i):
							theta[robot_i] -= driftError[robot_i]
							x[robot_i] = x[robot_i] - backupDistance*np.cos(theta[robot_i])
							y[robot_i] = y[robot_i] - backupDistance*np.sin(theta[robot_i])
							remainingDistance[robot_i] -= backupDistance
							pos_x[robot_i].append(x[robot_i])
							pos_y[robot_i].append(y[robot_i])
							theta[robot_i] += 90*pi/180.
							break
						pos_x[robot_i].append(x[robot_i])
						pos_y[robot_i].append(y[robot_i])

	for robot_i in range(N_robots):
		plt.plot(pos_x[robot_i], pos_y[robot_i])
		allPosX += pos_x[robot_i]
		allPosY += pos_y[robot_i]

# Plot the walls
N_points = 40
circle_x = worldRadius*np.cos(np.linspace(0,2*pi, N_points))
circle_y = worldRadius*np.sin(np.linspace(0,2*pi, N_points))
plt.plot(circle_x, circle_y)

plt.ylabel("Levy result [cm]")
plt.gca().set_aspect('equal')
plt.xlim([-worldRadius, worldRadius])
plt.ylim([-worldRadius, worldRadius])

plt.savefig(str(walkType) + " paths.png")

# Density plot
H, xedges, yedges = np.histogram2d(allPosX, allPosY, bins=30, range=[[-worldRadius, worldRadius], [-worldRadius, worldRadius]])
#H, xedges, yedges = np.histogram2d(allPosX, allPosY, range=[[-worldRadius, worldRadius], [-worldRadius, worldRadius]])
#plt.contour(H)

#H, xedges, yedges = np.histogram2d(allPosX, allPosY, range=[[293.,1454.0], [464.,1896.0]], bins=(50, 50))

plt.figure()
im = plt.imshow(np.rot90(np.fliplr(H)), interpolation='nearest', origin='low', extent=[xedges[0], xedges[-1], yedges[0], yedges[-1]])



plt.savefig(str(walkType) + " histogram.png")
#plt.show()

