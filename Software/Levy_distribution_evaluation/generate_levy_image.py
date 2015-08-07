#!/usr/bin/python


# Begin modules
import numpy as np
import cv2
# End modules

np.random.seed() # Set random seed using system time


margin = 100

size_X = 1440
size_Y = 900
#N_evaluations = 10000


img = np.ones((size_Y,size_X,3), np.uint8)
img *= 128

centreX = size_X/2
centreY = size_Y/2




def tuple_lerp(start,end,l):
	start = np.array(start)
	end = np.array(end)
	res = start*(1-l) + end*l
	return tuple(res)

for color in [(0,0,0), (255,0,0), (0,255,0), (0,0,255), (255,255,255)]:
	remaining_distance = size_X*10
	alpha = 1.5 #+ np.random.normal(0,scale=0.5)
	minDistance = 1

	#levy_evals = np.power(np.random.random(N_evaluations),(-1./alpha)) * minDistance
	
	N_evaluations = 0
	levy_evals = []
	while remaining_distance > 0:
		l = np.power(np.random.random(),(-1./alpha)) * minDistance
		levy_evals.append(l)
		N_evaluations += 1
		remaining_distance -= l
	
	newX = centreX
	newY = centreY
	prevX = newX
	prevY = newY

	theta = 0
	#theta_trend = 0
	for i in range(N_evaluations):
		progress = float(i)/float(N_evaluations)
		r = levy_evals[i]
		if (prevX > margin) and (prevX < size_X-margin) and (prevY > margin) and (prevY < size_Y-margin):
			theta += np.random.random()*np.pi
			#if r > 50:
			#theta = np.random.normal(theta_trend, scale=np.pi/2)
		else:
			theta = np.random.normal(loc=np.arctan2(centreY-prevY,centreX-prevX), scale=np.pi/2)
	
		theta %= 2.*np.pi
		
		#if r > 20:
		#theta_trend = theta_trend*0.9 + theta*0.1
		
		newX = prevX + r*np.cos(theta)
		newY = prevY + r*np.sin(theta)
	
		cv2.line(img, (int(prevX),int(prevY)), (int(newX),int(newY)), tuple_lerp((128,128,128),color,progress))
		prevX = newX
		prevY = newY


import datetime
ts = str(datetime.datetime.now())

bgfile = './'+ts+'.png'
cv2.imwrite(bgfile,img)





#from gi.repository import Gio

#SCHEMA = 'org.gnome.desktop.background'
#KEY = 'picture-uri'

#def change_background(filename):
#    gsettings = Gio.Settings.new(SCHEMA)
#    gsettings.set_string(KEY, 'file://' + filename)
#    gsettings.apply()

#change_background(bgfile)



