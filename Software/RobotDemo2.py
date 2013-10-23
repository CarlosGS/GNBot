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

# Set compass to zero
robot.send("MagSetZero\n")

def turn(angle):
    robot.send("GoToAngle:"+str(angle)+"\n")
    robot.send("LedRGB:255,0,0\n")
    robot.send("Motor:20,20\n")
    time.sleep(3)
    robot.send("Motor:0,0\n")

turn(0)
turn(90)
turn(2*90)
turn(3*90)

exit()

