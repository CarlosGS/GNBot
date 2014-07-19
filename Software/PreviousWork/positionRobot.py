#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import sys
import atexit
import time

import numpy as np

from helper import *

import GNBot.GNBot as GNBot
# End modules

# Begin configuration.
BT_ADDRESS = "00:11:09:18:02:34"
LDR_DF_PATH = "./data/"
# End configuration



# Stabilish the connection
robot = GNBot.GNBot(BT_ADDRESS)

while robot.connect() == -1:
    print("Connect failed. Retrying...")

# Callback to close the connection when force-quitting
def exitCallback():
    print("Closing connection...")
    robot.send("Init:ByeBye!\n") # This will reset the robot, stopping all commands, motions.. etc
    robot.disconnect()
atexit.register(exitCallback)




print("Position the robot pointing to north and press enter!")
val = sys.stdin.readline()

# Set compass to zero
robot.send("MagSetZero\n")


print("Move it and press enter to auto-center!")
val = sys.stdin.readline()

# Point robot to the origin
correct = False
while not correct:
    robot.send("GoToAngle:0\n")
    angle = robot.getMagSensorAngle()
    print("Initial angle:" + str(angle))
    correct = getValue("Is it almost zero? {1,0}",1)


# Variable to store all the data for each experiment
Position_data = {}

print(" ---- Parameters for the experiment ----")
dontAsk=True # This setting can select if the user is asked to modify/check the parameters
Position_data["description"] = getValue("Description:","Robot localization",dontAsk)

Position_data["motion"]["L"] = getValue("Left motor speed [-90,90]",0,dontAsk)
Position_data["motion"]["R"] = getValue("Right motor speed [-90,90]",0,dontAsk)

Position_data["environment"]["AmbientLight"] = getValue("Ambient light [0,100]",10.0,dontAsk)

print("Press enter to begin the experiment!")
val = sys.stdin.readline()

# Record the date and time of the beginning of the experiment
Position_data["date"]["start"] = getDate()


# Turn of the LED to avoid light interference
robot.send("LedRGB:0,0,0\n")

# Reset the timer [? TODO check and explain this!]
robot.send("Log:OFF\n")

# Set the motion
robot.send("Motor:"+str(Position_data["motion"]["L"])+","+str(-Position_data["motion"]["R"])+"\n")

# Begin logging
robot.send("Log:ON\n")

N_LDR = 4;

Position_data["LDR_raw"] = []
Position_data["MagAngle_raw"] = []
Position_data["BattVoltage_raw"] = []
Position_data["time_raw"] = []

N_circles = Position_data["Ncircles"]

angle = 0

halfTurns = 0
while 1:
    line = robot.recvLine()
    if line != -1 and len(line) > 0:
        print(line)
        i = 0
        LDR_row = [0.0]*N_LDR
        valuePairs = line.split(" ")
        if len(valuePairs) == 8:
            for valuePair in line.split(" "):
                data = valuePair.split(":")
                if len(data) != 2:
                    print("Error! Value pair len != 2")
                    exit()
                if i < N_LDR:
                    LDR_row[i] = float(data[1])
                elif i == N_LDR:
                    Position_data["MagAngle_raw"].append(float(data[1]))
                    angle = float(data[1])
                elif i == N_LDR+1:
                    Position_data["BattVoltage_raw"].append(float(data[1]))
                elif i == N_LDR+2:
                    Position_data["time_raw"].append(int(data[1]))
                i += 1
            Position_data["LDR_raw"].append(LDR_row)
            #print(LDR_row)
            #print(Position_data["MagAngle_raw"])
            #print(Position_data["BattVoltage_raw"])
            #print(Position_data["time_raw"])
            #exit()
        else:
            print("Ignoring line!")
    
    
    if abs(angle-180) < 10:
        if (halfTurns % 2) == 0:
            halfTurns += 1
            print(halfTurns)
    elif abs(angle) < 10:
        if (halfTurns % 2) == 1:
            halfTurns += 1
            print(halfTurns)
    if halfTurns >= N_circles*2:
        break

# End logging
robot.send("Log:OFF\n")

# Stop the motion
robot.send("Motor:0,0\n")

Position_data["date"]["end"] = getDate()

# Show the resulting data
for key in Position_data.keys():
    print(str(key)+":")
    print(Position_data[key])

LDR_DF = "Position_data_" + Position_data["date"]["start"].replace(" ", "_") + ".p"
print("Saving data to " + LDR_DF_PATH + LDR_DF)
saveToFile(Position_data,LDR_DF_PATH,LDR_DF)

exit()

