#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlosgs (http://carlosgs.es)
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
    robot.send("Init:ByeBye!\n") # This will stop the robot
    robot.disconnect()

atexit.register(exitCallback)

DistancesList = [40, 45, 50, 60, 70, 80, 90, 100, 150, 200, 250, 300, 400, 450, 500, 600]

for lightDistanceVal in DistancesList:
    print("Position the robot at D=" + str(lightDistanceVal) + "cm and press enter!")
    val = sys.stdin.readline()
    
    # Set compass to zero
    robot.send("MagSetZero\n")


    # Variable to store all the data for each experiment
    LDR_data = {}

    print(" ---- Parameters for the experiment ----")
    dontAsk=True
    LDR_data["description"] = getValue("Description:","Light intensity vs distance",dontAsk)

    LDR_data["Ncircles"] = getValue("Number of circles",1,dontAsk)

    LDR_data["Speed"] = getValue("Spinning speed [-90,90]",8,dontAsk)

    LDR_data["AmbientLight"] = getValue("Ambient light [0,100]",10.0,dontAsk)


    Nlights = getValue("Number of lights",1,dontAsk)

    LDR_data["lights"] = []

    for i in range(Nlights):
        light = {}
        #print("Angle of light "+str(i+1)+". Point the robot towards it and press enter...")
        #val = sys.stdin.readline()
        #angle = robot.getMagSensorAngle()

        light["angle"] = getValue("Angle of light "+str(i+1)+" [0,360]",0,dontAsk)#float(angle),dontAsk)
        light["distance"] = getValue("Distance of light "+str(i+1)+" [cm]",lightDistanceVal,dontAsk)
        light["intensity"] = getValue("Intensity of light "+str(i+1)+" [0,100]",50.0,dontAsk)
        LDR_data["lights"].append(light)


    #LDR_data["LDR_real_pos"] = []
    #LDR_data["LDR_real_pos"].append(45)
    #LDR_data["LDR_real_pos"].append(45*2)
    #LDR_data["LDR_real_pos"].append(45*3)
    #LDR_data["LDR_real_pos"].append(45*4)


    # Point robot to the origin
    correct = False
    while not correct:
        robot.send("GoToAngle:0\n")
        angle = robot.getMagSensorAngle()
        print("Initial angle:" + str(angle))
        correct = getValue("Is it almost zero? {1,0}",1)


    #print("Press enter to begin the experiment!")
    #val = sys.stdin.readline()


    LDR_data["beginDate"] = getDate()


    # Turn of the LED to avoid light interference
    robot.send("LedRGB:0,0,0\n")

    # Set compass to zero
    #robot.send("MagSetZero\n")

    # Reset the timer
    robot.send("Log:OFF\n")

    # Set the motion
    robot.send("Motor:"+str(LDR_data["Speed"])+","+str(-LDR_data["Speed"])+"\n")

    # Begin logging
    robot.send("Log:ON\n")

    N_LDR = 4;

    LDR_data["LDR_raw"] = []
    LDR_data["MagAngle_raw"] = []
    LDR_data["BattVoltage_raw"] = []
    LDR_data["time_raw"] = []

    N_circles = LDR_data["Ncircles"]

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
                        LDR_data["MagAngle_raw"].append(float(data[1]))
                        angle = float(data[1])
                    elif i == N_LDR+1:
                        LDR_data["BattVoltage_raw"].append(float(data[1]))
                    elif i == N_LDR+2:
                        LDR_data["time_raw"].append(int(data[1]))
                    i += 1
                LDR_data["LDR_raw"].append(LDR_row)
                #print(LDR_row)
                #print(LDR_data["MagAngle_raw"])
                #print(LDR_data["BattVoltage_raw"])
                #print(LDR_data["time_raw"])
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

    LDR_data["endDate"] = getDate()

    for key in LDR_data.keys():
        print(str(key)+":")
        print(LDR_data[key])

    LDR_DF = "LDR_data_" + LDR_data["beginDate"].replace(" ", "_") + "_Nlights:" + str(Nlights) + ".p"
    print("Saving data to " + LDR_DF_PATH + LDR_DF)
    saveToFile(LDR_data,LDR_DF_PATH,LDR_DF)

exit()

