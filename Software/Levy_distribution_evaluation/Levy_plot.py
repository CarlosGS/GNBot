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

def getRandomLevyDistance():
	alpha = 2
	minDistance = 10
	levyVal = pow(random.random(),(-1./alpha)) * minDistance
	#if levyVal < minDistance:
	#	levyVal = minDistance
	return levyVal

N_evaluations = 1000

levy_evals = [getRandomLevyDistance() for i in range(N_evaluations)]

plt.figure()
plt.plot(levy_evals)
plt.ylabel("Levy result [cm]")

plt.figure()
plt.hist([min(val,500) for val in levy_evals], normed=1, bins=100, cumulative=False)
#plt.xlim([0,100])
plt.ylabel("Levy resulting PDF")

levy_evals_x = range(N_evaluations)
levy_evals_y = range(N_evaluations)
for i in range(N_evaluations):
	theta = random.random()*2*np.pi
	r = levy_evals[i]
	if i == 0:
		prevX = 0
		prevY = 0
	else:
		prevX = levy_evals_x[i-1]
		prevY = levy_evals_y[i-1]
	levy_evals_x[i] = prevX + r*np.cos(theta)
	levy_evals_y[i] = prevY + r*np.sin(theta)

plt.figure()
plt.plot(levy_evals_x, levy_evals_y)
plt.ylabel("Levy result [cm]")
plt.gca().set_aspect('equal')

plt.show()

