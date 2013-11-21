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

def turn(angle):
    robot.send("LedRGB:0,"+str(int(255.*angle/360.))+",255\n")
    robot.send("GoToAngle:"+str(angle)+"\n")
    time.sleep(2)
    robot.send("LedRGB:0,255,"+str(int(255.*angle/360.))+"\n")
    robot.send("Motor:15,15\n")
    time.sleep(2)
    robot.send("Motor:0,0\n")


# Set compass to zero
robot.send("MagSetZero\n")

turn(0)

print("Fin del giro de 90...")

#val = sys.stdin.readline()

turn(90*1)
turn(90*2)
turn(90*3)

print("Fin del programa...")

val = sys.stdin.readline()

print("Desconexion!")

exit()

