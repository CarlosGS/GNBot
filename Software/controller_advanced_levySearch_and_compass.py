#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# License: CC BY-SA 4.0 (Attribution-ShareAlike 4.0 International, http://creativecommons.org/licenses/by-sa/4.0/)


# Begin modules
from GNBot.GNBot import GNBot
from helper import *
import time
import datetime
from pprint import pprint
import atexit
import os

import matplotlib.pyplot as plt
import numpy as np
import scipy.signal as signal
from math import *

import random
import threading
# End modules

gb = None

gnbot_list = {}


MAClookup = {
	'\x00\x13\xa2\x00@a@\x15'	: "Robot 1",
	"\x00\x13\xa2\x00@a@'"		: "Robot 2",
	'\x00\x13\xa2\x00@a@\x18'	: "Robot 3",
	'\x00\x13\xa2\x00@a@\n'		: "Robot 4",
	"NI5"	: "Robot 5",
}

def lookupRobotName(address):
	if address in MAClookup.keys():
		return MAClookup[address]
	else:
		return repr(address)







def robot_goToAngle(address):
	global gb, gnbot_list, do_exit
	calib = gnbot_list[address]["magnetometerCalibration"]
	while not do_exit:
		y = mapVals(gnbot_list[address]["vals"]["magnetometerY"], calib["ymin"], calib["ymax"], -1, 1)
		x = mapVals(gnbot_list[address]["vals"]["magnetometerX"], calib["xmin"], calib["xmax"], -1, 1)
		heading = atan2(y, x)*180/pi # -180:180 range. 0deg is north
		
		forward = 0
		rotate = 0
		
		seconds = time.time()
		
		dstAngle = (360.*seconds/10)%360-180 # -180:180 range. 0deg is north
		
		# Convert angle ranges to 0:360
		dstAngle = (dstAngle+180)%360
		heading = (heading+180)%360
		
		angleError = dstAngle-heading
		if angleError > 180:
			angleError = 360-angleError
		if angleError < -180:
			angleError = 360+angleError
		
		rotateSpeed = int(abs(round(mapVals(angleError, -180., 180., -50, 50))))
		if rotateSpeed < 3: rotateSpeed = 3
		if rotateSpeed > 12: rotateSpeed = 12
					
		if angleError > 0:
			rotate = rotateSpeed
		else:
			rotate = -rotateSpeed
		if abs(angleError) < 3: rotate = 0
		robotSetMotors(address, forward+rotate,forward-rotate)
		#print(str(heading)+"\t: "+str(angleError)+"\t: "+str(dstAngle))
		time.sleep(0.2)


def toResistance(v_adc):
    Vc = 5.
    Rl = 10000.
    if v_adc <= 0: v_adc = 0.000001
    Vout = mapVals(v_adc,0,1023, 0.,Vc)
    Rs = Rl*(Vc-Vout)/Vout
    return Rs

def getRandomLevyDelay():
	alpha = 2
	return (pow(random.random(),(-1./alpha))-1)*10


def robot_levyIR(address):
	global gb, gnbot_list, do_exit
	calib = gnbot_list[address]["magnetometerCalibration"]
	while not do_exit:
		try:
			dstAngle = random.uniform(-180, 180) # -180:180 range. 0deg is north
			# Convert angle ranges to 0:360
			dstAngle = (dstAngle+180)%360
			while 1: # Go to angle
				y = mapVals(gnbot_list[address]["vals"]["magnetometerY"], calib["ymin"], calib["ymax"], -1, 1)
				x = mapVals(gnbot_list[address]["vals"]["magnetometerX"], calib["xmin"], calib["xmax"], -1, 1)
				heading = atan2(y, x)*180/pi # -180:180 range. 0deg is north
		
				forward = 0
				rotate = 0
		
				# Convert angle ranges to 0:360
				heading = (heading+180)%360
		
				angleError = dstAngle-heading
				if angleError > 180:
					angleError = 360-angleError
				if angleError < -180:
					angleError = 360+angleError
		
				rotateSpeed = int(abs(round(mapVals(angleError, -180., 180., -50, 50))))
				if rotateSpeed < 3: rotateSpeed = 3
				if rotateSpeed > 10: rotateSpeed = 10
					
				if angleError > 0:
					rotate = rotateSpeed
				else:
					rotate = -rotateSpeed
				if abs(angleError) < 5: break # Exit loop when desired angle is reached
				robotSetMotors(address, forward+rotate,forward-rotate)
				#print(str(heading)+"\t: "+str(angleError)+"\t: "+str(dstAngle))
				time.sleep(0.2)
			robotSetMotors(address, 0,0)
			time.sleep(0.5)
			timeFwd = getRandomLevyDelay()
			print("dstAngle: "+str(dstAngle)+"\ttimeFwd: "+str(timeFwd))
			if timeFwd < 0.1:
				timeFwd = 0.1
			gnbot_list[address]["record"] = True
			iniTime = time.time()
			maxIRdistance = 0
			while time.time()-iniTime < timeFwd:
				speed = 8
				speedOffset = 5
				robotSetMotors(address, 1+speed+speedOffset,speed)
				time.sleep(0.5)
				robotSetMotors(address, speed,speed+speedOffset)
				time.sleep(0.5)
				maxIRdistance = max(gnbot_list[address]["recordedVals"]["distMin"])
				minNoseResistance = toResistance(max(gnbot_list[address]["recordedVals"]["noseMax"]))
				print("Nose resistance: "+str(round(minNoseResistance))+" Ohms")
				if minNoseResistance < 10000: # ODOR SOURCE LOCATED!
					robotSetMotors(address, 0,0)
					time.sleep(0.5)
					robotSetMotors(address, -8,-8) # The robot is "backing up" (to account for detection delay)
					time.sleep(2)
					endRobot(address) # Robot positioned over the source
					return
				if maxIRdistance > 300:
					robotSetMotors(address, 0,0)
					time.sleep(0.5)
					robotSetMotors(address, -8,-8) # The robot is "backing up" (to avoid collision)
					time.sleep(2)
					break # Random rotation again
			robotSetMotors(address, 0,0)
			gnbot_list[address]["record"] = False
			time.sleep(0.25)
			gnbot_list[address]["recordedVals"] = {}
			time.sleep(0.25)
		except:
			print("Warning: Levy had some trouble")



def robot_levyIR_noCompass(address):
	global gb, gnbot_list, do_exit
	initRobot(address)
	time.sleep(0.5)
	while 1:
		minNoseResistance = toResistance(gnbot_list[address]["vals"]["noseMax"])
		if minNoseResistance > 10000:
			break
		time.sleep(1)
	while not do_exit:
		try:
			dstAngleDelay = random.uniform(0, 6.1)
			rotateSpeed = 8
			if round(random.random()) == 1:
				rotateSpeed *= -1
			values = gb.createValue("motorL", rotateSpeed)
			values += gb.createValue("motorR", -rotateSpeed)
			values += gb.createValue("delay", int(dstAngleDelay*1000))
			values += gb.createValue("motorL", 0)
			values += gb.createValue("motorR", 0)
			values += gb.createValue("sampletime", 300)
			gb.sendPUTcommand(address, values)
			time.sleep(dstAngleDelay)
			time.sleep(0.5)
			timeFwd = getRandomLevyDelay()
			print("dstAngleDelay: "+str(dstAngleDelay)+"\ttimeFwd: "+str(timeFwd))
			if timeFwd < 0.1:
				timeFwd = 0.1
			gnbot_list[address]["record"] = True
			iniTime = time.time()
			maxIRdistance = 0
			while time.time()-iniTime < timeFwd:
				speed = 8
				speedOffset = 5
				robotSetMotors(address, speed+speedOffset,speed)
				time.sleep(0.5)
				robotSetMotors(address, speed,speed+speedOffset)
				time.sleep(0.5)
				maxIRdistance = max(gnbot_list[address]["recordedVals"]["distMin"])
				minNoseResistance = toResistance(max(gnbot_list[address]["recordedVals"]["noseMax"]))
				#print("Nose resistance: "+str(round(minNoseResistance))+" Ohms")
				if minNoseResistance < 10000: # ODOR SOURCE LOCATED!
					robotSetMotors(address, 0,0)
					time.sleep(0.5)
					robotSetMotors(address, -8,-8) # The robot is "backing up" (to account for detection delay)
					time.sleep(2)
					endRobot(address) # Robot positioned over the source
					return
				if maxIRdistance > 300:
					robotSetMotors(address, 0,0)
					time.sleep(0.5)
					robotSetMotors(address, -8,-8) # The robot is "backing up" (to avoid collision)
					time.sleep(2)
					break # Random rotation again
			robotSetMotors(address, 0,0)
			gnbot_list[address]["record"] = False
			time.sleep(0.25)
			gnbot_list[address]["recordedVals"] = {}
			time.sleep(0.25)
		except:
			print("Warning: Levy had some trouble")


def robot_calibrateCompass(address):
	global gb, gnbot_list, do_exit
	initRobot(address)
	while not do_exit:
		if not "magnetometerCalibration" in gnbot_list[address].keys():
			gnbot_list[address]["record"] = True
			robotSetMotors(address, 12,-12)
			time.sleep(5)
			robotSetMotors(address, 0,0)
			gnbot_list[address]["record"] = False
			#pprint(gnbot_list[address])
	
			gnbot_list[address]["magnetometerCalibration"] = {}
			gnbot_list[address]["magnetometerCalibration"]["xmin"] = float(min(gnbot_list[address]["recordedVals"]["magnetometerX"]))
			gnbot_list[address]["magnetometerCalibration"]["xmax"] = float(max(gnbot_list[address]["recordedVals"]["magnetometerX"]))
			gnbot_list[address]["magnetometerCalibration"]["ymin"] = float(min(gnbot_list[address]["recordedVals"]["magnetometerY"]))
			gnbot_list[address]["magnetometerCalibration"]["ymax"] = float(max(gnbot_list[address]["recordedVals"]["magnetometerY"]))
			print(gnbot_list[address]["magnetometerCalibration"])
		
	#			fig = plt.figure(figsize=(8.0, 8.0))
	#			ax = fig.add_subplot( 111 )
	#			iniTime = gnbot_list[address]["recordedVals"]["time"][0]
	#			time = [(t-iniTime).total_seconds() for t in gnbot_list[address]["recordedVals"]["time"]]
	#			ax.plot(time, gnbot_list[address]["recordedVals"]["magnetometerX"])
	#			ax.plot(time, gnbot_list[address]["recordedVals"]["magnetometerY"])
	#			ax.plot(time, gnbot_list[address]["recordedVals"]["magnetometerZ"])
	#			ax.grid( True )
	#			plt.show()
		else:
			#t = threading.Thread(target=robot_goToAngle, args = ([address])) # Run selected robot program
			t = threading.Thread(target=robot_levyIR, args = ([address]))
			t.daemon = True
			t.start()
			return
		time.sleep(0.2)




def robot_printValues(address):
	global gb, gnbot_list, do_exit
	initRobot(address)
	time.sleep(1)
	while not do_exit:
		pprint(gnbot_list[address]["vals"])
		minNoseResistance = toResistance(gnbot_list[address]["vals"]["noseMax"])
		print("Nose resistance: "+str(round(minNoseResistance))+" Ohms")
		time.sleep(0.1)


def gnbot_received_callback(address, received_data):
	global gb, gnbot_list
	received_data["time"] = datetime.datetime.now()
	if not address in gnbot_list.keys():
		print("NEW GNBOT ADDED! Address: " + repr(address))
		gnbot_list[address] = {}
		t = threading.Thread(target=robot_levyIR_noCompass, args = ([address]))
		#t = threading.Thread(target=robot_calibrateCompass, args = ([address]))
		#t = threading.Thread(target=robot_printValues, args = ([address]))
		t.daemon = True
		t.start()
	#else: print("Received packet from: " + repr(address))
	if "record" in gnbot_list[address].keys():
		if gnbot_list[address]["record"] == True: # Append values if record=true. The values are kept if record=false
			if not "recordedVals" in gnbot_list[address]:
				gnbot_list[address]["recordedVals"] = {}
			for key in received_data.keys():
				if not key in gnbot_list[address]["recordedVals"]:
					gnbot_list[address]["recordedVals"][key] = []
				gnbot_list[address]["recordedVals"][key].append(received_data[key])
	gnbot_list[address]["vals"] = received_data
	#pprint(received_data)
	#print("")



musicNotes = {"DO":262, "RE":294, "MI":330, "FA":349, "SOL":392, "LA":440, "SI":494}

def initRobot(address):
	noteDelay = 200
	noteScale = 1
	values = gb.createValue("motorL", 0)
	values += gb.createValue("motorR", 0)
	values += gb.createValue("tone", noteScale*musicNotes["DO"])
	values += gb.createValue("delay", noteDelay)
	values += gb.createValue("notone", 10)
	values += gb.createValue("tone", noteScale*musicNotes["MI"])
	values += gb.createValue("delay", noteDelay)
	values += gb.createValue("notone", 10)
	values += gb.createValue("tone", noteScale*musicNotes["SOL"])
	values += gb.createValue("delay", noteDelay)
	values += gb.createValue("notone", 0)
	values += gb.createValue("sampletime", 300)
	values += gb.createValue("ledR_PWM", 0)
	values += gb.createValue("ledG_PWM", 128)
	values += gb.createValue("ledB_PWM", 0)
	gb.sendPUTcommand(address, values)

def endRobot(address):
	noteDelay = 300
	noteScale = 1
	values = gb.createValue("motorL", 0)
	values += gb.createValue("motorR", 0)
	values += gb.createValue("tone", noteScale*musicNotes["SOL"])
	values += gb.createValue("delay", noteDelay)
	values += gb.createValue("notone", 10)
	values += gb.createValue("tone", noteScale*musicNotes["MI"])
	values += gb.createValue("delay", noteDelay)
	values += gb.createValue("notone", 10)
	values += gb.createValue("tone", noteScale*musicNotes["DO"])
	values += gb.createValue("delay", noteDelay)
	values += gb.createValue("notone", 0)
	values += gb.createValue("sampletime", 1000)
	values += gb.createValue("ledR_PWM", 0)
	values += gb.createValue("ledG_PWM", 0)
	values += gb.createValue("ledB_PWM", 128)
	gb.sendPUTcommand(address, values)

def robotSetMotors(address, L,R):
	values = gb.createValue("motorL", L)
	values += gb.createValue("motorR", R)
	gb.sendPUTcommand(address, values)


random.seed() # Set random seed using system time

# Register callback for powering off the GNBot system
do_exit = False
def exitCallback():
	global gb, do_exit
	do_exit = True
	if gb != None:
		time.sleep(0.5)
		for address in gnbot_list.keys():
			endRobot(address)
		gb.halt()
		gb = None
atexit.register(exitCallback)

gb = GNBot(gnbot_received_callback, '/dev/ttyUSB0', 9600)

print("Waiting for packets...")

plt.ion()
fig = plt.figure(figsize=(8.0, 8.0))
ax = fig.add_subplot( 122 )
colors = ['r','g','b','c','y']

ax2 = fig.add_subplot( 121 )

while not do_exit:
	try:
		ax.clear() # clear the plot
		ax2.clear()
		index = 0
		for address in gnbot_list.keys():
			robotName = lookupRobotName(address)
			robot_battery_adc = (gnbot_list[address]["vals"]["batteryMax"]+gnbot_list[address]["vals"]["batteryMin"])/2
			robot_battery = mapVals(robot_battery_adc,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
			ax.bar(index,robot_battery,width=0.25,color=colors[index],label=robotName)
			minNoseResistance = toResistance(gnbot_list[address]["vals"]["noseMax"])
			ax2.bar(index,minNoseResistance,width=0.75,color=colors[index],label=robotName)
			index += 1
		ax.set_ylim([8,20])
		ax2.set_ylim([0,50000])
		ax.set_title("Battery levels")
		ax2.set_title("Nose sensor levels")
		ax.set_ylabel("Battery level [Volts]")
		ax2.set_ylabel("Nose sensor level [Ohms]")
		ax2.invert_yaxis()
		if index > 0:
			#ax.legend()
			ax2.legend()
		plt.draw() # update the plot
		time.sleep(0.1)
	except:
		do_exit = True
		break

exitCallback()

os._exit(0)

